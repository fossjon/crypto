<?php
	function srnd($size=5, $chrs="0123456789")
	{
		$clen = strlen($chrs);
		$outp = "";
		for ($x = 0; $x < $size; $x++) { $outp .= $chrs[rand(0, $clen - 1)]; }
		return $outp;
	}
	
	function safe($stri="", $chrs="0123456789")
	{
		$slen = strlen($stri);
		$outp = "";
		for ($x = 0; $x < $slen; $x++)
		{
			if (strpos($chrs, $stri[$x]) !== false) { $outp .= $stri[$x]; }
		}
		return $outp;
	}
	
	$serv = "x";
	$user = "x";
	$pass = "x";
	$dbnm = "x";
	
	$conn = new mysqli($serv, $user, $pass, $dbnm);
	if ($conn->connect_error) { die("conn failed"); }
	
	if (1)
	{
		$sqlq = ("CREATE DATABASE ".$dbname);
		if ($conn->query($sqlq) === true) { /* no-op */ }
		else { /* no-op */ }
		
		$sqlq = "CREATE TABLE usrinfo (idnum INT AUTO_INCREMENT PRIMARY KEY, user VARCHAR(512), pass VARCHAR(512), ctime INT)";
		if ($conn->query($sqlq) === true) { /* no-op */ }
		else { /* no-op */ }
		
		$sqlq = "CREATE TABLE sesinfo (uidn INT PRIMARY KEY, rand VARCHAR(512), ctime INT, utime INT)";
		if ($conn->query($sqlq) === true) { /* no-op */ }
		else { /* no-op */ }
		
		$sqlq = "CREATE TABLE msginfo (mnum VARCHAR(512), sidn INT, didn INT, data VARCHAR(1024), ctime INT)";
		if ($conn->query($sqlq) === true) { /* no-op */ }
		else { /* no-op */ }
	}
	
	$nums = ("0123456789");
	$hexs = ($nums."abcdef");
	$alls = ($hexs."ghijklmnopqrstuvwxyz"."ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	$exts = (":,");
	
	if ($_POST["mode"] == "join")
	{
		$user = safe($_POST["user"], $alls);
		$arnd = srnd(4, $alls); $brnd = srnd(4, $alls);
		$hash = hash("sha256", $arnd.$_POST["pass"].$brnd);
		
		$sqlq = ("SELECT * FROM usrinfo WHERE user = '".$user."'");
		$rows = $conn->query($sqlq);
		if ($rows->num_rows < 1)
		{
			$sqlq = ("INSERT INTO usrinfo VALUES (0, '".$user."', '".$arnd.":".$hash.":".$brnd."', ".time().")");
			if ($conn->query($sqlq) === true)
			{
				$uidn = $conn->insert_id;
				$sqlq = ("INSERT INTO sesinfo VALUES (".$uidn.", '*', ".time().", 0)");
				if ($conn->query($sqlq) === true) { print("USER:GOOD\n"); }
				else { /* no-op */ }
			}
			else { /* no-op */ }
		}
	}
	
	if ($_POST["mode"] == "auth")
	{
		$user = safe($_POST["user"], $alls);
		
		$sqlq = ("SELECT * FROM usrinfo WHERE user = '".$user."'");
		$rows = $conn->query($sqlq);
		if ($rows->num_rows > 0)
		{
			while ($urow = $rows->fetch_assoc())
			{
				$rnds = explode(":", $urow["pass"]);
				$hash = hash("sha256", $rnds[0].$_POST["pass"].$rnds[2]);
				if ($rnds[1] == $hash)
				{
					$tokn = srnd(256, $alls);
					$sqlq = ("UPDATE sesinfo SET rand = '".$tokn."', utime = '".time()."' WHERE uidn = ".$urow["idnum"]);
					if ($conn->query($sqlq) === true) { print("GOOD:".$tokn."\n"); }
					else { /* no-op */ }
					break;
				}
			}
		}
	}
	
	if ($_POST["mode"] == "stat")
	{
		$user = safe($_POST["user"], $alls);
		$tokn = safe($_POST["auth"], $alls);
		
		$stat = 0;
		$sqlq = ("SELECT COUNT(*) AS n FROM msginfo JOIN sesinfo ON sesinfo.uidn = msginfo.didn JOIN usrinfo ON usrinfo.idnum = msginfo.didn WHERE user = '".$user."' AND rand = '".$tokn."'");
		$rows = $conn->query($sqlq);
		if ($rows->num_rows > 0)
		{
			$srow = $rows->fetch_assoc();
			$stat = $srow["n"];
		}
		
		print("STAT:".$stat."\n");
	}
	
	if ($_POST["mode"] == "mesg")
	{
		$user = safe($_POST["user"], $alls);
		$tokn = safe($_POST["auth"], $alls);
		$dusr = safe($_POST["rcpt"], $alls);
		$mesg = safe($_POST["mesg"], $alls.$exts);
		
		$sqlq = ("SELECT * FROM usrinfo JOIN sesinfo ON sesinfo.uidn = usrinfo.idnum WHERE (user = '".$user."' AND rand = '".$tokn."') OR user = '".$dusr."'");
		$rows = $conn->query($sqlq);
		if ($rows->num_rows > 1)
		{
			$arow = $rows->fetch_assoc();
			$brow = $rows->fetch_assoc();
			$crow = $arow;
			$drow = $brow;
			
			if ($arow["user"] == $user) { $crow = $arow; $drow = $brow; }
			else { $crow = $brow; $drow = $arow; }
			
			$mtim = explode(" ", microtime());
			$stim = ($mtim[1].".".substr($mtim[0], 2).".".rand(10000, 99999));
			$sqlq = ("INSERT INTO msginfo VALUES ('".$stim."', ".$crow["idnum"].", ".$drow["idnum"].", '".$mesg."', ".time().")");
			if ($conn->query($sqlq) === true) { print("MESG:GOOD\n"); }
		}
	}
	
	if ($_POST["mode"] == "read")
	{
		$user = safe($_POST["user"], $alls);
		$tokn = safe($_POST["auth"], $alls);
		$msgs = array();
		
		$sqlq = ("SELECT mnum, uinfo.user AS suser, data FROM msginfo JOIN sesinfo ON sesinfo.uidn = msginfo.didn JOIN usrinfo ON usrinfo.idnum = msginfo.didn JOIN usrinfo AS uinfo ON uinfo.idnum = msginfo.sidn WHERE usrinfo.user = '".$user."' AND sesinfo.rand = '".$tokn."'");
		$rows = $conn->query($sqlq);
		if ($rows->num_rows > 0)
		{
			while ($mrow = $rows->fetch_assoc())
			{
				print($mrow["mnum"].":".$mrow["suser"].":".$mrow["data"]."\n");
				array_push($msgs, $mrow["mnum"]);
			}
			
			$smsg = ("'".join("', '", $msgs)."'");
			$sqlq = ("DELETE FROM msginfo WHERE mnum IN (".$smsg.")");
			$conn->query($sqlq);
		}
	}
	
	$conn->close();
?>

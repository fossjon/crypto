//
//  ViewController.swift
//  smsg
//

import UIKit

class ViewController: UIViewController, UITableViewDelegate, UITableViewDataSource {

	let userText: UITextField = UITextField(frame: CGRect(x: 10.00, y: 32.00, width: 200.00, height: 30.00))
	let passText: UITextField = UITextField(frame: CGRect(x: 10.00, y: 64.00, width: 200.00, height: 30.00))
	let authButn: UIButton = UIButton(frame: CGRect(x: 225.00, y: 64.00, width: 100.00, height: 30.00))
	
	let frndText: UITextField = UITextField(frame: CGRect(x: 10.00, y: 96.00, width: 200.00, height: 30.00))
	let frndButn: UIButton = UIButton(frame: CGRect(x: 225.00, y: 96.00, width: 100.00, height: 30.00))
	let seckText: UILabel = UILabel(frame: CGRect(x: 10.00, y: 128.00, width: 300.00, height: 30.00))
	
	var mesgList: [String] = ["line 1", "line 2", "line 3"]
	let mesgHist: UITableView = UITableView(frame: CGRect(x: 10.00, y: 160.00, width: 300.00, height: 120.00))
	let mesgText: UITextView = UITextView(frame: CGRect(x: 10.00, y: 284.00, width: 200.00, height: 60.00))
	let mesgButn: UIButton = UIButton(frame: CGRect(x: 225.00, y: 284.00, width: 100.00, height: 30.00))
	
	let vers = "1.9.9.1"
	let serv = "http://smsg.site88.net"
	var authkey: String = ""
	var dhkey: String = ""
	var dhx: String = ""
	var dhy: String = ""
	var skey: String = ""
	var tkey: String = ""
	var ecobj = eccryp()
	
	func readReply(mode: Int, response: String) {
		if (response != "") { print("r>"+response) }
		
		if (response.characters.count > 5)
		{
			if (mode == 0)
			{
				self.authkey = response.substringWithRange(Range<String.Index>(start:response.startIndex.advancedBy(5), end:response.endIndex))
				
				dispatch_async(dispatch_get_main_queue()) { self.seckText.text = (" " + self.authkey) }
			}
			
			else if (mode == 3)
			{
				let linelist = response.characters.split{$0 == "\n"}.map(String.init)
				for line in linelist
				{
					let readlist = line.characters.split{$0 == ":"}.map(String.init)
					if (readlist.count > 2)
					{
						let infolist = readlist[2].characters.split{$0 == ","}.map(String.init)
						
						if (infolist.count == 3)
						{
							if ((readlist[1] == frndText.text) && (infolist[0] == "key"))
							{
								if (self.dhkey == "")
								{
									self.dhkey = String.fromCString(ecdh(self.ecobj, nil))!
									self.dhx = String.fromCString(bnstr(getx(self.ecobj)))!
									self.dhy = String.fromCString(bnstr(gety(self.ecobj)))!
									self.sendMesg(1, mesg: self.dhx+","+self.dhy)
								}
								
								setexy(self.ecobj, infolist[1], infolist[2])
								ecdh(self.ecobj, self.dhkey)
								let tdhx = String.fromCString(bnstr(getx(self.ecobj)))
								let tdhy = String.fromCString(bnstr(gety(self.ecobj)))
								
								print("d>"+tdhx!+","+tdhy!)
								self.skey = String.fromCString(shash(tdhx!+","+tdhy!))!
								self.tkey = String.fromCString(shash(self.skey))!
								print("k>"+self.skey+","+self.tkey)
								dispatch_async(dispatch_get_main_queue()) { self.seckText.text = (" " + self.skey) }
							}
						}
						
						else if (infolist.count == 4)
						{
							if ((readlist[1] == frndText.text) && (infolist[0] == "msg"))
							{
								let smac = String.fromCString(hmac(infolist[1]+","+infolist[2], self.tkey))
								print("v>"+smac!+"=="+infolist[3])
								if (smac == infolist[3])
								{
									let smesg = String.fromCString(sencr(infolist[1], infolist[2], self.skey, 1))
									self.mesgList.append(smesg!)
									dispatch_async(dispatch_get_main_queue()) { self.mesgHist.reloadData() }
								}
							}
						}
					}
				}
			}
		}
	}
	
	func sendMesg(mode: Int, mesg: String) {
		var postdata: String = ""
		
		if (mode == 0)
		{
			postdata = ("mode=auth&user="+self.userText.text!+"&pass="+self.passText.text!)
		}
		
		else if (mode == 1)
		{
			postdata = ("mode=mesg&user="+self.userText.text!+"&auth="+self.authkey+"&rcpt="+self.frndText.text!+"&mesg=key,"+mesg)
		}
		
		else if (mode == 2)
		{
			postdata = ("mode=mesg&user="+self.userText.text!+"&auth="+self.authkey+"&rcpt="+self.frndText.text!+"&mesg=msg,"+mesg)
		}
		
		else if (mode == 3)
		{
			postdata = ("mode=read&user="+self.userText.text!+"&auth="+self.authkey)
		}
		
		let request = NSMutableURLRequest(URL: NSURL(string: self.serv)!)
		request.HTTPMethod = "POST"
		request.HTTPBody = postdata.dataUsingEncoding(NSUTF8StringEncoding)
		let task = NSURLSession.sharedSession().dataTaskWithRequest(request) {
			data, response, error in
			let reply = NSString(data: data!, encoding: NSUTF8StringEncoding)
			self.readReply(mode, response: reply as! String)
		}
		task.resume()
	}
	
	func butnPress(butn: UIButton) {
		if (butn.titleLabel?.text == "Login") { self.sendMesg(0, mesg: "") }
		if (butn.titleLabel?.text == "Add")
		{
			if (self.dhkey == "")
			{
				self.dhkey = String.fromCString(ecdh(self.ecobj, nil))!
				self.dhx = String.fromCString(bnstr(getx(self.ecobj)))!
				self.dhy = String.fromCString(bnstr(gety(self.ecobj)))!
			}
			self.sendMesg(1, mesg: self.dhx+","+self.dhy)
		}
		if (butn.titleLabel?.text == "Send")
		{
			if (self.skey != "")
			{
				let stime = String(NSDate.timeIntervalSinceReferenceDate())
				let smesg = String.fromCString(sencr(stime, self.mesgText.text, self.skey, 0))
				let smac = String.fromCString(hmac(stime+","+smesg!, self.tkey))
				self.sendMesg(2, mesg: stime+","+smesg!+","+smac!)
			}
		}
	}
	
	func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
		return self.mesgList.count
	}
	
	func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
		let cell = UITableViewCell(style: UITableViewCellStyle.Default, reuseIdentifier: "cell")
		cell.textLabel!.text = self.mesgList[indexPath.row]
		cell.textLabel!.font = cell.textLabel!.font.fontWithSize(10)
		return cell
	}
	
	func backFunc(timer: NSTimer) {
		self.sendMesg(3, mesg: "")
	}
	
	override func viewDidLoad() {
		super.viewDidLoad()
		// Do any additional setup after loading the view, typically from a nib.
		
		print("i>draw init")
		
		/* login elements */
		
		let userPadd = UIView(frame: CGRectMake(0, 0, 5, self.userText.frame.size.height))
		self.userText.leftView = userPadd
		self.userText.leftViewMode = UITextFieldViewMode.Always
		self.userText.layer.cornerRadius = 2.0
		self.userText.layer.borderColor = UIColor(red: 0/255, green: 0/255, blue: 0/255, alpha: 1.0).CGColor
		self.userText.layer.borderWidth = 2.0
		self.userText.autocorrectionType = UITextAutocorrectionType.No
		self.userText.autocapitalizationType = UITextAutocapitalizationType.None
		self.view.addSubview(self.userText)
		
		let passPadd = UIView(frame: CGRectMake(0, 0, 5, self.passText.frame.size.height))
		self.passText.leftView = passPadd
		self.passText.leftViewMode = UITextFieldViewMode.Always
		self.passText.layer.cornerRadius = 2.0
		self.passText.layer.borderColor = UIColor(red: 0/255, green: 0/255, blue: 0/255, alpha: 1.0).CGColor
		self.passText.layer.borderWidth = 2.0
		self.passText.secureTextEntry = true
		self.view.addSubview(self.passText)
		
		self.authButn.layer.cornerRadius = 2.0
		self.authButn.layer.borderColor = UIColor(red: 0/255, green: 0/255, blue: 0/255, alpha: 1.0).CGColor
		self.authButn.layer.borderWidth = 2.0
		self.authButn.setTitle("Login", forState: UIControlState.Normal)
		self.authButn.setTitleColor(UIColor.blackColor(), forState: UIControlState.Normal)
		self.authButn.addTarget(self, action: "butnPress:", forControlEvents: UIControlEvents.TouchUpInside)
		self.view.addSubview(self.authButn)
		
		/* friend elements */
		
		let frndPadd = UIView(frame: CGRectMake(0, 0, 5, self.userText.frame.size.height))
		self.frndText.leftView = frndPadd
		self.frndText.leftViewMode = UITextFieldViewMode.Always
		self.frndText.layer.cornerRadius = 2.0
		self.frndText.layer.borderColor = UIColor(red: 0/255, green: 0/255, blue: 0/255, alpha: 1.0).CGColor
		self.frndText.layer.borderWidth = 2.0
		self.frndText.autocorrectionType = UITextAutocorrectionType.No
		self.frndText.autocapitalizationType = UITextAutocapitalizationType.None
		self.view.addSubview(self.frndText)
		
		self.frndButn.layer.cornerRadius = 2.0
		self.frndButn.layer.borderColor = UIColor(red: 0/255, green: 0/255, blue: 0/255, alpha: 1.0).CGColor
		self.frndButn.layer.borderWidth = 2.0
		self.frndButn.setTitle("Add", forState: UIControlState.Normal)
		self.frndButn.setTitleColor(UIColor.blackColor(), forState: UIControlState.Normal)
		self.frndButn.addTarget(self, action: "butnPress:", forControlEvents: UIControlEvents.TouchUpInside)
		self.view.addSubview(self.frndButn)
		
		self.seckText.layer.cornerRadius = 2.0
		self.seckText.layer.borderColor = UIColor(red: 0/255, green: 0/255, blue: 0/255, alpha: 1.0).CGColor
		self.seckText.layer.borderWidth = 2.0
		self.seckText.font = self.seckText.font.fontWithSize(8)
		self.seckText.text = (" v" + self.vers)
		self.view.addSubview(self.seckText)
		
		/* message elements */
		
		self.mesgHist.layer.cornerRadius = 2.0
		self.mesgHist.layer.borderColor = UIColor(red: 0/255, green: 0/255, blue: 0/255, alpha: 1.0).CGColor
		self.mesgHist.layer.borderWidth = 2.0
		self.mesgHist.rowHeight = 20.0
		self.mesgHist.delegate = self
		self.mesgHist.dataSource = self
		self.view.addSubview(self.mesgHist)
		
		self.mesgText.layer.cornerRadius = 2.0
		self.mesgText.layer.borderColor = UIColor(red: 0/255, green: 0/255, blue: 0/255, alpha: 1.0).CGColor
		self.mesgText.layer.borderWidth = 2.0
		self.mesgText.autocorrectionType = UITextAutocorrectionType.No
		self.mesgText.autocapitalizationType = UITextAutocapitalizationType.None
		self.mesgText.text = "message string"
		self.view.addSubview(self.mesgText)
		
		self.mesgButn.layer.cornerRadius = 2.0
		self.mesgButn.layer.borderColor = UIColor(red: 0/255, green: 0/255, blue: 0/255, alpha: 1.0).CGColor
		self.mesgButn.layer.borderWidth = 2.0
		self.mesgButn.setTitle("Send", forState: UIControlState.Normal)
		self.mesgButn.setTitleColor(UIColor.blackColor(), forState: UIControlState.Normal)
		self.mesgButn.addTarget(self, action: "butnPress:", forControlEvents: UIControlEvents.TouchUpInside)
		self.view.addSubview(self.mesgButn)
		
		/* background reader */
		
		NSTimer.scheduledTimerWithTimeInterval(5.0, target: self, selector: "backFunc:", userInfo: nil, repeats: true)
	}

	override func didReceiveMemoryWarning() {
		super.didReceiveMemoryWarning()
		// Dispose of any resources that can be recreated.
	}

}

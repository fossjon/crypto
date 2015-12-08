//
//  ViewController.swift
//  smsg
//

import UIKit

class ViewController: UIViewController, UITableViewDelegate, UITableViewDataSource {

	//let msgUser: UILabel = UILabel(frame: CGRect(x: 10.00, y: 32.00, width: 200.00, height: 64.00))
	var msgList: [String] = ["We", "Heart", "Swift"]
	let msgView: UITableView = UITableView(frame: CGRect(x: 10.00, y: 32.00, width: 150.00, height: 128.00))
	let msgText: UITextView = UITextView(frame: CGRect(x: 10.00, y: 256.00, width: 150.00, height: 64.00))
	let msgButn: UIButton = UIButton(frame: CGRect(x: 200.00, y: 256.00, width: 150.00, height: 64.00))
	
	let serv = "http://smsg.site88.net"
	var authkey: String = ""
	var dhkey: String = ""
	
	func readReply(response: String) {
		if (self.authkey == "")
		{
			self.authkey = response.substringWithRange(Range<String.Index>(start:response.startIndex.advancedBy(5), end:response.endIndex))
			print(self.authkey)
		}
	}
	
	func sendMsg(butn: UIButton) {
		var postdata: String = ""
		
		if (self.authkey == "")
		{
			postdata = ("mode=auth&user=jon&pass=abc")
		}
		else
		{
			let ecobj = eccryp()
			self.dhkey = String.fromCString(ecdh(ecobj, nil, 0))!
			let dhx = String.fromCString(bnstr(getx(ecobj)))
			let dhy = String.fromCString(bnstr(gety(ecobj)))
			postdata = ("mode=mesg&user=jon&auth="+self.authkey+"&rcpt=bob&mesg="+dhx!+","+dhy!)
			print(self.dhkey)
		}
		
		let request = NSMutableURLRequest(URL: NSURL(string: self.serv)!)
		request.HTTPMethod = "POST"
		request.HTTPBody = postdata.dataUsingEncoding(NSUTF8StringEncoding)
		let task = NSURLSession.sharedSession().dataTaskWithRequest(request) {
			data, response, error in
			let reply = NSString(data: data!, encoding: NSUTF8StringEncoding)
			self.readReply(reply as! String)
		}
		task.resume()
	}
	
	func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
		return self.msgList.count
	}
	
	func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
		let cell = UITableViewCell(style: UITableViewCellStyle.Default, reuseIdentifier: "cell")
		cell.textLabel!.text = self.msgList[indexPath.row]
		return cell
	}
	
	override func viewDidLoad() {
		super.viewDidLoad()
		// Do any additional setup after loading the view, typically from a nib.
		
		/*self.msgUser.text = "message string"
		self.view.addSubview(self.msgUser)*/
		
		self.msgView.delegate = self
		self.msgView.dataSource = self
		self.view.addSubview(self.msgView)
		
		self.msgText.backgroundColor = UIColor.blueColor()
		self.msgText.text = "input string"
		self.view.addSubview(self.msgText)
		
		self.msgButn.backgroundColor = UIColor.blueColor()
		self.msgButn.setTitle("Send", forState: UIControlState.Normal)
		self.msgButn.addTarget(self, action: "sendMsg:", forControlEvents: UIControlEvents.TouchUpInside)
		self.view.addSubview(self.msgButn)
	}

	override func didReceiveMemoryWarning() {
		super.didReceiveMemoryWarning()
		// Dispose of any resources that can be recreated.
	}

}


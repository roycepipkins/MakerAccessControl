/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */

/* ************************************************************************

#asset(accesscontrol/*)

************************************************************************ */

/**
 * This is the main application class of your custom application "AccessControl"
 */
qx.Class.define("accesscontrol.Application",
{
  extend : qx.application.Standalone,



  /*
  *****************************************************************************
     MEMBERS
  *****************************************************************************
  */

  members :
  {
    /**
     * This method contains the initial application code and gets called 
     * during startup of the application
     * 
     * @lint ignoreDeprecated(alert)
     */
    main : function()
    {
      // Call super class
      this.base(arguments);
	  this.fixDate();
      // Enable logging in debug variant
      if (qx.core.Environment.get("qx.debug"))
      {
        // support native logging capabilities, e.g. Firebug for Firefox
        qx.log.appender.Native;
        // support additional cross-browser console. Press F7 to toggle visibility
        qx.log.appender.Console;
		
		
      }

      /*
      -------------------------------------------------------------------------
        Below is your actual application code...
      -------------------------------------------------------------------------
      */

      // Create a button
      var userEdit = new accesscontrol.UserEdit(this);

      // Document is the application root
      var doc = this.getRoot();

      // Add button to document at fixed coordinates
      doc.add(userEdit, {left: 10, top: 10});

      
    },
	sendMsg : function(msg, obj) 
	{
		this.replyObj = obj;
		var msgStr = qx.lang.Json.stringify(msg);
		var req = new qx.io.request.Xhr("/configCommand");
		req.setMethod("POST");
		req.setRequestData(msgStr);
		req.addListener("load", this.onMsgReply, this);
		req.send();	
	},
	onMsgReply : function(e)
	{
		var reply = e.getTarget();
		//console.log(reply);
		if (reply.getPhase() == "success")
		{
			var replyMsg = reply.getResponse();
			var reply = qx.lang.Json.parse(replyMsg);
			this.replyObj.onMsgReply(reply);
		}
		else
		{
			//TODO handle comm error
		}
	},
	saveUserReply: function(msg, obj)
	{
	
	},
	allRolesReply: function(msg, obj)
	{
		var reply = new Object;
		reply.msgType = "AllRolesReply";
		reply.roles = new Array();
		
		var role = new Object();
		role.role_id = 1;
		role.role_name = "Full Time";
		reply.roles.push(role);
		
		role = new Object();
		role.role_id = 2;
		role.role_name = "Weekdays";
		reply.roles.push(role);
		
		role = new Object();
		role.role_id = 3;
		role.role_name = "Weekends";
		reply.roles.push(role);
		
		role = new Object();
		role.role_id = 4;
		role.role_name = "Backdoor";
		reply.roles.push(role);
		
		obj.onMsgReply(reply);
	},
	allUsersReply: function(msg, obj)
	{
		var reply = new Object();
		reply.msgType = "AllUsersReply";
		var users = new Array();
		
		var user = new Object();
		user.user_name = "Alpha";
		user.user_id = 1;
		user.passwords = new Array();
		user.passwords.push("HASH#1");
		user.passwords.push("HASH#2");
		user.passwords.push("HASH#3");
		user.roles = new Array();
		var role = new Object();
		role.role = "Weekends";
		role.role_id = 3;
		role.expiration = "2012-03-07T23:59:59";
		user.roles.push(role);
		role = new Object();
		role.role = "Backdoor";
		role.role_id = 5;
		role.expiration = "2999-03-07T23:59:59";
		user.roles.push(role);
		
		users.push(user);
		
		user = new Object();
		user.user_name = "Beta";
		user.user_id = 2;
		user.passwords = new Array();
		user.passwords.push("HASH#4");
		user.passwords.push("HASH#5");
		user.passwords.push("HASH#6");
		user.roles = new Array();
		role = new Object();
		role.role = "WeekDays";
		role.role_id = 2;
		role.expiration = "2012-03-07T23:59:59";
		user.roles.push(role);
		role = new Object();
		role.role = "Backdoor";
		role.role_id = 5;
		role.expiration = "2999-03-07T23:59:59";
		user.roles.push(role);
		users.push(user);
		
		user = new Object();
		user.user_name = "Gamma";
		user.user_id = 3;
		user.passwords = new Array();
		user.passwords.push("HASH#7");
		user.passwords.push("HASH#8");
		user.passwords.push("HASH#9");
		user.roles = new Array();
		role = new Object();
		role.role = "Fulltime";
		role.role_id = 1;
		role.expiration = "2012-03-07T23:59:59";
		user.roles.push(role);
		role = new Object();
		role.role = "Backdoor";
		role.role_id = 5;
		role.expiration = "2999-03-07T23:59:59";
		user.roles.push(role);
		users.push(user);
		
		reply.users = users;
		obj.onMsgReply(reply);
	},
	fixDate : function()
	{
		Date.prototype.toSortedString = function()
		{
			var ret;
			ret = (this.getYear() + 1900) + "-";
			if (this.getMonth() + 1 < 10) ret = ret + "0";
			ret = ret + (this.getMonth() + 1) + "-";
			if (this.getDate() < 10) ret = ret + "0";
			ret = ret + this.getDate() + " ";
			if (this.getHours() < 10) ret = ret + "0";
			ret = ret + this.getHours() + ":";
			if (this.getMinutes() < 10) ret = ret + "0";
			ret = ret + this.getMinutes() + ":";
			if (this.getSeconds() < 10) ret = ret + "0";
			ret = ret + this.getSeconds();
			
			return ret;
		};
		
		Date.prototype.pushToEndOfDay = function()
		{
			this.setHours(23);
			this.setMinutes(59);
			this.setSeconds(59);
			this.setMilliseconds(999);
		}
	}
  }
});

/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */

/* ************************************************************************

#asset(accesscontrol/*)

************************************************************************ */


qx.Class.define("accesscontrol.UserEdit",
{
	extend : qx.ui.container.Composite,

	construct : function(app)
	{
		// Call super class
		this.base(arguments);
		
		this.app = app;
		this.layout = new qx.ui.layout.Grid();
		this.setLayout(this.layout);
		this.layout.setSpacingX(4);
		this.layout.setSpacingY(4);
		this.layout.setColumnWidth(0, 300);
		
		
		this.add(new qx.ui.basic.Label("All Users:"), {row: 0, column: 0});
		
		
		
		this.layout.setColumnWidth(1, 700);
		
		this.usersTableModel = new qx.ui.table.model.Simple();
		this.usersTableModel.setColumns(["Name", "Soonest Expiration", "ID"]);
		this.usersTable = new qx.ui.table.Table(this.usersTableModel);
		this.usersTable.getSelectionModel().setSelectionMode(qx.ui.table.selection.Model.SINGLE_SELECTION);
		this.usersTable.setColumnWidth(1, 120);
		this.usersTable.setColumnWidth(2, 50);
		this.usersTable.setHeight(600);
		this.usersTable.setWidth(275);
		this.usersTable.getSelectionModel().addListener("changeSelection", this.onUserSelectionChange, this);
		this.usersTable.addListener("cellDblclick", this.fastUserUpdate, this);
		this.add(this.usersTable, {row: 1, column: 0});
		this.addUserButton = new qx.ui.form.Button("Add User");
		this.delUserButton = new qx.ui.form.Button("Delete User");
		this.add(this.addUserButton, {row: 2, column: 0});
		this.add(this.delUserButton, {row: 3, column: 0});
		this.addUserButton.addListener("execute", this.addUser, this);
		this.delUserButton.addListener("execute", this.delUser, this);
		
		
		this.user_edit_comp = new qx.ui.container.Composite();
		this.user_edit_grid = new qx.ui.layout.Grid();
		this.user_edit_grid.setSpacingX(4);
		this.user_edit_grid.setSpacingY(4);
		this.user_edit_grid.setColumnWidth(0, 300);
		this.user_edit_comp.setLayout(this.user_edit_grid);
		this.add(this.user_edit_comp, {row: 1, column: 1});
		
		this.user_edit_comp.add(new qx.ui.basic.Label("User Name:"), {row: 0, column: 0});
		this.userName 		= new  qx.ui.form.TextField();
		this.userName.addListener("input", this.userNameChanged, this);
		this.user_edit_comp.add(this.userName, {row: 1, column:0});
		
		this.user_edit_comp.add(new qx.ui.basic.Label("Key Code:"), {row: 2, column: 0});
		
		this.keysTableModel = new qx.ui.table.model.Simple();
		this.keysTableModel.setColumns(["Key Hash"]);
		this.keysTable = new qx.ui.table.Table(this.keysTableModel);
		this.keysTable.getSelectionModel().setSelectionMode(qx.ui.table.selection.Model.SINGLE_SELECTION);
		this.keysTable.setColumnWidth(0, 210);
		this.keysTable.setHeight(300);
		this.keysTable.setWidth(275);
		this.user_edit_comp.add(this.keysTable, {row: 3, column: 0});
		
		this.add_key_button = new qx.ui.form.Button("Add Key Code");
		this.add_key_button.setMaxWidth(125);
		this.add_key_button.addListener("execute", this.addKey, this);
		this.user_edit_comp.add(this.add_key_button, {row: 4, column: 0});
		
		this.del_key_button = new qx.ui.form.Button("Delete Key Code");
		this.del_key_button.setMaxWidth(125);
		this.del_key_button.addListener("execute", this.delKey, this);
		this.user_edit_comp.add(this.del_key_button, {row: 5, column: 0});
		
		this.check_key_button = new qx.ui.form.Button("Check Key Code");
		this.check_key_button.setMaxWidth(125);
		this.check_key_button.addListener("execute", this.checkKey, this);
		this.user_edit_comp.add(this.check_key_button, {row: 6, column: 0});
		//TODO add event handlers
		
		
		
		this.user_edit_comp.add(new qx.ui.basic.Label("User Roles:"), {row: 2, column: 1});
		
		this.rolesTableModel = new qx.ui.table.model.Simple();
		this.rolesTableModel.setColumns(["Role", "Expiration", "RoleID"]);
		this.rolesTable = new qx.ui.table.Table(this.rolesTableModel);
		this.rolesTable.getSelectionModel().setSelectionMode(qx.ui.table.selection.Model.SINGLE_SELECTION);
		this.rolesTable.setHeight(300);
		this.rolesTable.setWidth(350);
		this.rolesTable.setColumnWidth(1, 230);
		this.user_edit_comp.add(this.rolesTable, {row: 3, column: 1});
		
		this.add_role_button = new qx.ui.form.Button("Add Role");
		this.add_role_button.setMaxWidth(100);
		this.add_role_button.addListener("execute", this.addRole, this);
		this.user_edit_comp.add(this.add_role_button, {row: 4, column: 1});
		
		
		this.del_role_button = new qx.ui.form.Button("Delete Role");
		this.del_role_button.setMaxWidth(100);
		this.del_role_button.addListener("execute", this.delRole, this);
		this.user_edit_comp.add(this.del_role_button, {row: 5, column: 1});
		
		
		this.save_button = new qx.ui.form.Button("Save User Changes");
		this.cancel_button = new qx.ui.form.Button("Cancel User Changes");
		this.save_button.setMinHeight(50);
		this.cancel_button.setMinHeight(50);
		this.user_edit_comp.add(new qx.ui.core.Spacer(25, 25), {row: 7, column: 0});
		this.user_edit_comp.add(new qx.ui.core.Spacer(25, 25), {row:7, column: 1});
		this.user_edit_comp.add(this.save_button, {row: 8, column: 0});
		this.user_edit_comp.add(this.cancel_button, {row: 8, column: 1});
		
		
		
		this.save_button.addListener("execute", this.saveUser, this);
		
		this.cancel_button.addListener("execute", this.cancelSaveUser, this);
		
		
		
		//-----------------------
		this.getAllUsers();
		this.getAllRoles();
		
		this.disable();
		this.usersTable.setEnabled(true);
		this.addUserButton.setEnabled(true);
		this.delUserButton.setEnabled(true);
		
	},
	events :
	{
		"SendMsg" : "qx.event.type.Data"
	},
	members :
	{
		userIdx : -1,
		userHasChanged : false,
		userNameChanged : function()
		{
			if (!this.userHasChanged)
			{
				this.userHasChanged = true;
				this.enable();
			}
		},
		onUserSelectionChange : function()
		{
			var userData = this.usersTableModel.getData();
			var ranges = this.usersTable.getSelectionModel().getSelectedRanges();
			if (ranges.length > 0)
			{
				console.log(ranges.length + " " + ranges[0].minIndex + " " + userData.length);
				if (ranges[0].minIndex < userData.length)
				{
					var user_id = userData[ranges[0].minIndex][2]; 
					this.userIdx = this.getUsersIndex(user_id);
					var user = this.users[this.userIdx];
					this.setUserData(user);
					this.enable();
				}
			}
		},
		findUserById : function(id)
		{
			var idx;
			for(idx = 0; idx < this.users.length; idx++)
			{
				if (this.users[idx].user_id == id)
				{
					return this.users[idx];
				}
			}
			
			return false;
		},
		addUser : function()
		{
			var user = new Object();
			user.user_name = "New User";
			user.user_id = this.getFreeUserID();
			user.roles = new Array();
			user.passwords = new Array();
			var rowEntry = new Array(user.user_name, "1970-01-01 00:00:00", user.user_id);
			var userData = this.usersTableModel.getData();
			console.log(userData);
			userData.push(rowEntry);
			var sort_idx = this.usersTableModel.getSortColumnIndex();
			var ascend = this.usersTableModel.isSortAscending();
			this.usersTableModel.setData(userData);
			if (sort_idx != -1) this.usersTableModel.sortByColumn(sort_idx, ascend);
			this.userHasChanged = true;
			this.users.push(user);
			this.usersTable.getSelectionModel().setSelectionInterval(userData.length-1, userData.length-1)
			if (userData.length == 1) this.onUserSelectionChange();
			this.enable();
		},
		saveUser : function()
		{
			this.user.user_name = this.userName.getValue();
			var keyData = this.keysTableModel.getData();
			this.user.passwords = new Array();
			var idx;
			for(idx = 0; idx < keyData.length; idx++)
			{
				this.user.passwords.push(keyData[idx][0]);
			}
			
			var roleData = this.rolesTableModel.getData();
			this.user.roles = new Array();
			
			for(idx = 0; idx < roleData.length; idx++)
			{
				var role = new Object();
				role.role = roleData[idx][0];
				role.role_id = roleData[idx][2];
				role.expiration = roleData[idx][1].replace(" ", "T");
				this.user.roles.push(role);
			}
			
			this.userHasChanged = false;
			this.enable();
			this.setAllUsers(this.users);
			
			var cmd = new Object();
			cmd.msgType = "SaveUser"
			cmd.user = this.user;
			this.app.sendMsg(cmd, this);
		},
		cancelSaveUser : function()
		{
			this.userHasChanged = false;
			this.enable();
			this.setAllUsers(this.users);
		},
		delRole : function()
		{
			this.disable();
			
			var hashIdx;
			var mdl;
			
			if (this.userIdx < 0)
			{
				alert('Please select the user and the user\'s role that you would like to delete.');
				this.enable();
				return;
			}
			
			mdl = this.rolesTable.getSelectionModel();
			if (mdl.getSelectedCount() > 0)
			{
				hashIdx = mdl.getSelectedRanges()[0].minIndex;
			}
			else
			{
				alert('Please select the role you would like to delete.');
				this.enable();
				return;
			}
			
			
			var rowData = this.rolesTableModel.getData();
			rowData.splice(hashIdx, 1);
			this.rolesTableModel.setData(rowData);
			this.userHasChanged = true;
			this.enable();
			
			
		},
		delKey : function()
		{
			this.disable();
			var hashIdx;
			var mdl;
			
			if (this.userIdx < 0)
			{
				alert('Please select the user and the user\'s hash that you would like to delete.');
				this.enable();
				return;
			}
			
			mdl = this.keysTable.getSelectionModel();
			if (mdl.getSelectedCount() > 0)
			{
				hashIdx = mdl.getSelectedRanges()[0].minIndex;
			}
			else
			{
				alert('Please select the hash you would like to delete.');
				this.enable();
				return;
			}
			
			
			var rowData = this.keysTableModel.getData();
			rowData.splice(hashIdx, 1);
			this.keysTableModel.setData(rowData);
			this.userHasChanged = true;
			this.enable();
			
			
		},
		addKey : function()
		{
			this.disable();
			this.collectNewName(this.add_key_button, "onAddKeyValue", "Enter New Key Code");
		},
		checkKey : function()
		{
			this.disable();
			this.collectNewName(this.check_key_button, "onCheckKey", "Enter Key Code to check");
		},
		onCheckKey : function()
		{
			var hash = accesscontrol.MD5.compute(this.nameBox.getValue().toUpperCase());
			var idx;
			var found = false;
			var hashes = this.keysTableModel.getData();
			for(idx = 0; idx < hashes.length; idx++)
			{
				if (hashes[idx][0] == hash)
				{
					alert('Key #' + (idx + 1) + ' matches the entered key');
					found = true;
					break;
				}
			}
			
			if (!found) alert('No keys matched your text');
			this.cancelPopup();
			this.enable();
		},
		addRole : function()
		{
			this.disable();
			this.collectNewRole(this.add_role_button, "onAddRole", "Enter New Role");
		},
		onAddRole : function()
		{
			if (this.dateSelect.isEmpty())
			{
				alert('Please Select an expiration date or cancel.');
				return;
			}
			
			if (this.dateSelect.getValue().getTime() <= Date.now())
			{
				alert('Please select an expiration date that is in the future');
				return;
			}
			
			var role = this.roleSelect.getSelection();
			if (role.length == 0)
			{
				alert('Please select a role');
				return;
			}
			role = role[0].getLabel();
			
			var role_id = this.GetRoleIDFromName(role);
			
			var expiration = this.dateSelect.getValue();
			expiration.pushToEndOfDay();
			var roleData = this.rolesTableModel.getData();
			roleData.push(new Array(role, expiration.toSortedString(), role_id));
			this.rolesTableModel.setData(roleData);
			this.userHasChanged = true;
			this.cancelPopup();
		},
		GetRoleIDFromName : function(name)
		{
			var idx;
			var cnt = this.roles.length;
			for(idx = 0; idx < cnt; idx++)
			{
				if (this.roles[idx].role_name == name)
				{
					return this.roles[idx].role_id;
				}
			}
			
			return 0;
		},
		onAddKeyValue : function()
		{
			var rowData = this.keysTableModel.getData();
			rowData.push(new Array(accesscontrol.MD5.compute(this.nameBox.getValue().toUpperCase())));
			this.keysTableModel.setData(rowData);
			this.userHasChanged = true;
			this.cancelPopup();
			
		},
		setUserData : function(user)
		{
			this.user = user;
			this.userName.setValue(user.user_name);
			
			var passwords = new Array();
			var idx;
			for(idx = 0; idx < user.passwords.length; idx++)
			{
				passwords.push(new Array(user.passwords[idx]));
			}
			this.keysTableModel.setData(passwords);
			
			var roles = new Array();
			for(idx = 0; idx < user.roles.length; idx++)
			{
				var expire = new Date(user.roles[idx].expiration);
				roles.push(new Array(user.roles[idx].role, expire.toSortedString(), user.roles[idx].role_id));
			}
			this.rolesTableModel.setData(roles);
		},
		getAllUsers: function()
		{
			var cmd = new Object();
			cmd.msgType = "GetAllUsers";
			this.app.sendMsg(cmd, this);
		},	
		getAllRoles: function()
		{
			var cmd = new Object();
			cmd.msgType = "GetAllRoles";
			this.app.sendMsg(cmd, this);
		},
		onMsgReply : function(reply)
		{
			if (reply.msgType == "AllUsersReply")
			{
				this.setAllUsers(reply.users);
			}
			
			if (reply.msgType == "AllRolesReply")
			{
				this.setAllRoles(reply.roles);
			}
			
			if (reply.msgType == "SaveUserReply")
			{
				this.analyzeSetUserReply(reply);
			}
			
			if (reply.msgType == "DeleteUserReply")
			{
				this.onServerDeletedUser(reply);
			}
			
		},
		setAllRoles: function(roles)
		{
			this.roles = roles;
		},
		setAllUsers : function(users)
		{
			this.users = users;
			var rowData = new Array();
			var idx;
			for(var user in users)
			{
				var soonest = new Date('2999-12-31T23:59:59');
				for(idx = 0; idx < users[user].roles.length; idx++)
				{
					var expire = new Date(users[user].roles[idx].expiration);
					if (expire < soonest)
					 soonest = expire;
				}
				rowData.push( new Array(users[user].user_name, soonest.toSortedString(), users[user].user_id));
			}
			var sort_idx = this.usersTableModel.getSortColumnIndex();
			var ascend = this.usersTableModel.isSortAscending();
			this.usersTableModel.setData(rowData);
			if (sort_idx != -1) this.usersTableModel.sortByColumn(sort_idx, ascend);
			
		},
		analyzeSetUserReply : function(reply)
		{
			console.log("Set User is Done! Success: " + reply.success);
		},
		getDefaultStartDate : function()
		{
			var initDate = new Date();
			if (initDate.getDate() > 7)
			{
				if (initDate.getMonth() == 12)
				{
					initDate.setMonth(2);
					initDate.setYear(initDate.getFullYear() + 1);
				}
				else if (initDate.getMonth() == 11)
				{
					initDate.setMonth(1);
					initDate.setFullYear(initDate.getFullYear() +1)
				}	
				else
				{
					initDate.setMonth(initDate.getMonth() + 2);
				}
				initDate.setDate(7);
			}

			return initDate;
		},
		findSoonestRoleIdx : function(user)
		{
			var idx;
			var soonest = new Date('2999-12-31T23:59:59');
			var soonest_idx = 0;
			for(idx = 0; idx < user.roles.length; idx++)
			{
				var expire = new Date(user.roles[idx].expiration);
				if (expire < soonest)
				{
					soonest = expire;
					soonest_idx = idx;;
				}
			}
			
			return soonest_idx;
		},
		onFastUpdateUser : function()
		{
			//TODO this method destroys the sort. You need to preserve and re-implement the sort.
			var newExpire = this.dateSelect.getValue();
			newExpire.pushToEndOfDay();
			var expireStr = newExpire.toSortedString();
	
			expireStr = expireStr.replace(" ", "T");
			var role_idx = this.findSoonestRoleIdx(this.user);
			
			this.user.roles[role_idx].expiration = expireStr;
			
			this.setAllUsers(this.users);
			this.setUserData(this.user);
			
			var cmd = new Object();
			cmd.msgType = "SaveUser"
			cmd.user = this.user;
			this.app.sendMsg(cmd, this);
			//TODO you need to requery if there is a failure
			
			this.cancelPopup();
			
			
			
		},
		delUser : function()
		{
			//this presupposes that the table data model doesn not change when sorted test
			//oops it does change
			//TODO use invarient scans of the array by the selected user id
			var userData = this.usersTableModel.getData();
			var ranges = this.usersTable.getSelectionModel().getSelectedRanges();
			var user_id = userData[ranges[0].minIndex][2]; 
			this.userIdx = this.getUsersIndex(user_id);
			this.user = this.users[this.userIdx];
			console.log("this.userIdx: " + this.userIdx + " user_id: " + user_id + " this.user.user_id: " + this.user.user_id);
			
			this.askDeleteConfirmation();
			//TODO send this user for deletion
			//TODO get confirmation from server before actually deleting
			
			//todo show a confirm box
			//make xmit
			//if success, remove entry from users table
			//clear keys/roles/name
		},
		onDeleteUser : function()
		{
			
			
			var cmd = new Object();
			cmd.msgType = "DeleteUser"
			cmd.user = new Object();
			cmd.user.user_id = this.user.user_id;
			cmd.user.user_name = this.user.user_name;
			cmd.user.passwords = new Array();
			cmd.user.roles = new Array();
			this.app.sendMsg(cmd, this);
		},
		onServerDeletedUser : function(reply )
		{
			if (reply.success === true)
			{
				this.cancelPopup();
				var idx = this.getUsersIndex(this.user.user_id);
				this.users.splice(idx, 1);
				this.setAllUsers(this.users);
				this.clearUserData();
				this.onUserSelectionChange();
			}
			else
			{
				alert("There was a problem on the server while deleting the user");
			}
		},
		fastUserUpdate : function(evt)
		{
			var userData = this.usersTableModel.getData();
			var dateStr = new String(userData[evt.getRow()][1]);
			var expiration = new Date(dateStr.replace(" ", "T"));
			this.user = this.findUserById(userData[evt.getRow()][2]);
			this.popupLayout = new qx.ui.layout.HBox();
			this.popupLayout.setSpacing(10);
			this.popup = new qx.ui.popup.Popup(this.popupLayout);
			this.popup.setAutoHide(false);
			this.popup.setPaddingLeft(15);
			this.popup.setPaddingRight(15);
			this.popup.setPaddingTop(15);
			this.popup.setPaddingBottom(15);
			this.nameBox = new qx.ui.basic.Label(userData[evt.getRow()][0] + ":   ");
			this.nameBox.setAlignY("middle");
			this.dateSelect = new qx.ui.form.DateField();
			this.dateSelect.setValue(expiration);
			this.nameOKButton = new  qx.ui.form.Button("OK");
			this.nameOKButton.addListener("execute", this.onFastUpdateUser, this);
			this.nameCancelButton = new  qx.ui.form.Button("Cancel");
			this.nameCancelButton.addListener("execute", this.cancelPopup, this);
			this.popup.add(this.nameBox);
			this.popup.add(this.dateSelect);
			this.popup.add(this.nameOKButton);
			this.popup.add(this.nameCancelButton);
			var coords = new Object();
			
			coords.top = evt.getDocumentTop() - 15;
			coords.left = 10;
			
			this.popup.placeToPoint(coords);
			this.disable();
			this.popup.show();
			this.nameBox.getFocusElement().focus();
			
			
		},
		askDeleteConfirmation : function()
		{
		
			
			this.popupLayout = new qx.ui.layout.HBox();
			this.popupLayout.setSpacing(10);
			this.popup = new qx.ui.popup.Popup(this.popupLayout);
			this.popup.setAutoHide(false);
			this.popup.setPaddingLeft(15);
			this.popup.setPaddingRight(15);
			this.popup.setPaddingTop(15);
			this.popup.setPaddingBottom(15);
			this.nameBox = new qx.ui.basic.Label("Are you sure you want to delete " + this.user.user_name + "?   ");
			this.nameBox.setAlignY("middle");
			this.nameOKButton = new  qx.ui.form.Button("Yes");
			this.nameOKButton.addListener("execute", this.onDeleteUser, this);
			this.nameCancelButton = new  qx.ui.form.Button("Cancel");
			this.nameCancelButton.addListener("execute", this.cancelPopup, this);
			this.popup.add(this.nameBox);
			this.popup.add(this.nameOKButton);
			this.popup.add(this.nameCancelButton);
			var coords = new Object();
			
			coords.top = this.usersTable.getBounds().top - 15;
			coords.left = 10;
			
			this.popup.placeToPoint(coords);
			this.disable();
			this.popup.show();
			this.nameBox.getFocusElement().focus();
			
			
		},
		clearUserData : function()
		{
			this.keysTableModel.setData(new Array());
			this.rolesTableModel.setData(new Array());
			this.userName.setValue("");
			
		},
		enable : function()
		{
			this.keysTable.setEnabled(true);
			this.userName.setEnabled(true);
			this.rolesTable.setEnabled(true);
			this.add_key_button.setEnabled(true);
			this.del_key_button.setEnabled(true);
			this.check_key_button.setEnabled(true);
			this.add_role_button.setEnabled(true);
			this.del_role_button.setEnabled(true);
			this.save_button.setEnabled(this.userHasChanged);
			this.cancel_button.setEnabled(this.userHasChanged);
			this.usersTable.setEnabled(!this.userHasChanged);
			this.addUserButton.setEnabled(!this.userHasChanged);
			this.delUserButton.setEnabled(!this.userHasChanged);
		},
		disable : function()
		{
			this.keysTable.setEnabled(false);
			this.userName.setEnabled(false);
			this.rolesTable.setEnabled(false);
			this.add_key_button.setEnabled(false);
			this.del_key_button.setEnabled(false);
			this.check_key_button.setEnabled(false);
			this.add_role_button.setEnabled(false);
			this.del_role_button.setEnabled(false);
			this.save_button.setEnabled(false);
			this.cancel_button.setEnabled(false);
			this.usersTable.setEnabled(false);
			this.addUserButton.setEnabled(false);
			this.delUserButton.setEnabled(false);
		},
		collectNewName : function(locWidget, cb, msg)
		{
			//fire pop up
			this.popup = new qx.ui.popup.Popup(new qx.ui.layout.HBox());
			this.popup.setAutoHide(false);
			this.popup.setPaddingLeft(15);
			this.popup.setPaddingRight(15);
			this.popup.setPaddingTop(15);
			this.popup.setPaddingBottom(15);
			this.nameBox = new qx.ui.form.TextField();
			this.nameBox.setMinWidth(150);
			this.nameBox.setPlaceholder(msg);
			this.nameBox.setAlignY("middle");
			this.nameBox.setMarginRight(3);
			this.nameBox.setMarginLeft(15);
			this.nameBox.addListener("keypress", function(e)
				{
					if (e.getKeyIdentifier() == "Enter")
					{
						this[cb]();
					}
				}, this);
			this.nameOKButton = new  qx.ui.form.Button("OK");
			this.nameCancelButton = new  qx.ui.form.Button("Cancel");
			this.nameOKButton.addListener("execute", this[cb], this);
			this.nameCancelButton.addListener("execute", this.cancelPopup, this);
			this.popup.add(new qx.ui.basic.Atom("New Value: ", "resource/nexusconfig/question.png"));
			this.popup.add(this.nameBox);
			this.popup.add(this.nameOKButton);
			this.popup.add(this.nameCancelButton);
			var coords = new Object();
			var bounds = locWidget.getBounds();
			coords.top = bounds.top;
			coords.left = bounds.left;
			coords.top += Math.round(bounds.height/2);
			coords.left += bounds.width + 15;
			this.popup.placeToPoint(coords);
			this.disable();
			this.popup.show();
			this.nameBox.getFocusElement().focus();
		},
		collectNewRole : function(locWidget, cb, msg)
		{
			//fire pop up
			this.popup = new qx.ui.popup.Popup(new qx.ui.layout.Grid());
			this.popup.setAutoHide(false);
			this.popup.setPaddingLeft(15);
			this.popup.setPaddingRight(15);
			this.popup.setPaddingTop(15);
			this.popup.setPaddingBottom(15);
			this.roleSelect = new qx.ui.form.SelectBox();
			this.roleSelect.setMinWidth(150);
			this.dateSelect = new qx.ui.form.DateField();
			this.dateSelect.setValue(this.getDefaultStartDate());
			
				
			var idx;
			for(idx = 0; idx < this.roles.length; idx++)
			{
				var item = new qx.ui.form.ListItem(this.roles[idx].role_name);
				this.roleSelect.add(item);
			}
		
			this.nameOKButton = new  qx.ui.form.Button("OK");
			this.nameCancelButton = new  qx.ui.form.Button("Cancel");
			this.nameOKButton.addListener("execute", this[cb], this);
			this.nameCancelButton.addListener("execute", this.cancelPopup, this);
			this.popup.add(new qx.ui.basic.Label("Select New Role:"), {row: 0, column: 0});
			this.popup.add(new qx.ui.basic.Label("Select Expiration:"), {row: 0, column: 1});
			this.popup.add(this.roleSelect, {row: 1, column: 0});
			this.popup.add(this.dateSelect, {row: 1, column: 1});
			this.popup.add(this.nameOKButton, {row: 2, column: 0});
			this.popup.add(this.nameCancelButton, {row: 2, column: 1});
			var coords = new Object();
			var bounds = locWidget.getBounds();
			coords.top = bounds.top;
			coords.left = bounds.left;
			coords.top += Math.round(bounds.height/2);
			coords.left += bounds.width + 15;
			this.popup.placeToPoint(coords);
			this.disable();
			this.popup.show();
			this.roleSelect.getFocusElement().focus();
		},
		
		cancelPopup : function()
		{
			//hide the popup without doing anything else
			this.popup.setVisibility("excluded");
			this.enable();
		},
		getFreeUserID : function()
		{
			var maxId = 0;
			var idx;
			var rowData = this.usersTableModel.getData();
			for (idx = 0; idx < rowData.length; idx++)
			{
				if (rowData[idx][2] > maxId)
					maxId = rowData[idx][2];
			}
			
			return maxId + 1;
		},
		getUsersIndex : function(user_id)
		{
			var idx;
			for(idx = 0; idx < this.users.length; idx++)
			{
				if (this.users[idx].user_id == user_id)
				{
					return idx;
				}
			}
		}
	}
});
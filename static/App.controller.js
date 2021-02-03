sap.ui.define([
	'jquery.sap.global',
	'sap/ui/core/Fragment',
	'sap/m/MessageToast',
	'./Formatter',
	'sap/ui/core/mvc/Controller',
	'sap/ui/model/json/JSONModel',
	'sap/m/Popover',
	'sap/m/UploadCollectionParameter',
	'sap/m/Button'
], function (jQuery, Fragment, MessageToast, Formatter, Controller, JSONModel, Popover, Button) {
	"use strict";

	var oModelLokList           = new sap.ui.model.json.JSONModel();
	var oModelUserList          = new sap.ui.model.json.JSONModel();
	var oModelMainController    = new sap.ui.model.json.JSONModel();
	var oModelUser              = new sap.ui.model.json.JSONModel();



	//var socket = io.connect(location.protocol + '//' + document.domain + ':' + location.port + '');
	var socket = io.connect('ws://' + document.domain + ':' + location.port + '',{transports: ['websocket']});

	var CController = Controller.extend("view.App", {
        model: new sap.ui.model.json.JSONModel(),
		data: {

			navigation: [{
				title: 'Home',
				icon: 'sap-icon://home',
				expanded: true,
				key: 'Home'
			}, {
				title: 'Drive',
				icon: 'sap-icon://cargo-train',
				key: 'Drive',
				expanded: true,
			},
			{
				title: 'LokList',
				icon: 'sap-icon://list',
				expanded: true,
				key: 'lok_list'
			},

			{
				title: 'User',
				icon: 'sap-icon://account',
				expanded: true,
				key: 'user_list'
			},

			{
				title: 'Clients',
				icon: 'sap-icon://action',
				expanded: false,
				items: [{
					title: 'Show connected'
				}, {
					title: 'Child Item 2'
				}, {
					title: 'Child Item 3'
				}]
			}, ],

			fixedNavigation: [{
				title: 'Fixed Item 1',
				icon: 'sap-icon://employee'
			}, {
				title: 'Fixed Item 2',
				icon: 'sap-icon://building'
			}, {
				title: 'Fixed Item 3',
				icon: 'sap-icon://card'
			}],

			headerItems: [{
				text: "File"
			}, {
				text: "Edit"
			}, {
				text: "View"
			}, {
				text: "Settings"
			}, {
				text: "Help"
			}]
		},
		onInit: function() {

		    var namespace = '';

			// Dynamisches Menü
			this.model.setData(this.data);
			this.getView().setModel(this.model);
			this.getView().setModel(oModelLokList, "LokListModel");
			this.getView().setModel(oModelUserList, "oModelUserList");
			this.getView().setModel(oModelMainController, "oModelMainController");

			document._oController = this;

            socket.on('connect', function() {
				console.log(socket.id);
				MessageToast.show("Connected to server... ");
				//socket.emit('client_global_storage', {data: 'I\'m connected!'});
				
				//socket.emit('User_changed',  { user_id: "55", user_name: "TestUser" });
			});

			socket.on('disconnect', function() {
				MessageToast.show("Connection lost... ");
				
			});
			

			socket.on('initialisation', function(msg) {
				var config_model = jQuery.parseJSON(msg.data)

				var newArray = config_model.filter(function (el) {
					return el.session_id === socket.id
				  });
				
				console.log(newArray);
				oModelMainController.setData(newArray);

                var UserList_json = jQuery.parseJSON(msg.user);
				oModelUserList.setData(UserList_json);
				
				var LokList_data = jQuery.parseJSON(msg.LokList);
				oModelLokList.setData(LokList_data);

				
             });

            socket.on('config_data', function(msg) {
                var config_model = jQuery.parseJSON(msg.data)
                oModelMainController.setData(config_model);

                var UserList_json = jQuery.parseJSON(msg.user);
                oModelUserList.setData(UserList_json);

             });

             socket.on('loklist_data', function(msg) {
               var LokList_data = jQuery.parseJSON(msg.LokList);
               oModelLokList.setData(LokList_data);
            });


		},

		onAfterRendering: function ( oEvent)
		{
			
			
			//Storage
			jQuery.sap.require("jquery.sap.storage");
			var oStorage = jQuery.sap.storage(jQuery.sap.storage.Type.global);
			//Check if there is data in the Storage
			if (oStorage.get("myLocalData")) {
			console.log("Data is from Storage!");
			var oDataUser = oStorage.get("myLocalData");
			oModelUser.setData(oDataUser);
			}
			else
			{
			console.log("Local storage is empty");
			//this.handleLogonDialog();
			//MessageToast.show("Please login !" );
			var UserData = { "UserData" : [ { "Name" : "Jochen" } ] };
			oModelUser.setData(UserData);
			oStorage.put("myLocalData", UserData);
			}

			
		},

		onItemSelect: function(oEvent) {
			var item = oEvent.getParameter('item');
			var viewId = this.getView().getId();
			sap.ui.getCore().byId(viewId + "--pageContainer").to(viewId + "--" + item.getKey());
		},

		onSliderliveChange: function(oEvent) {
		    socket.emit('main_controller_value_changed', {data: oModelMainController.getData()});
		},


        handleLogonDialog: function() {
			if (!this._oDialogLogon) {
				this._oDialogLogon = sap.ui.xmlfragment("view.fragments.Logon", this);
			}

			this.getView().addDependent(this._oDialogLogon);
			// toggle compact style
			jQuery.sap.syncStyleClass("sapUiSizeCompact", this.getView(), this._oDialogLogon);
			this._oDialogLogon.open();

			},


        handleTableSelectDialogPress: function(oEvent) {
			if (!this._oDialog) {
				this._oDialog = sap.ui.xmlfragment("view.fragments.Dialog", this);
			}

			// Multi-select if required
			var bMultiSelect = !!oEvent.getSource().data("multi");
			this._oDialog.setMultiSelect(bMultiSelect);

			// Remember selections if required
			var bRemember = !!oEvent.getSource().data("remember");
			this._oDialog.setRememberSelections(bRemember);

			this.getView().addDependent(this._oDialog);
			// toggle compact style
			jQuery.sap.syncStyleClass("sapUiSizeCompact", this.getView(), this._oDialog);
			this._oDialog.open();
		},

		handleLocomotionSelectDialogClose: function(oEvent) {
			var aContexts = oEvent.getParameter("selectedContexts");
			if (aContexts && aContexts.length) {
			    var lok_name = aContexts.map(function(oContext) { return oContext.getObject().name; }).join(", ");
			    var lok_id   = aContexts.map(function(oContext) { return oContext.getObject().id; }).join(", ");

				MessageToast.show("You have chosen " + lok_name + lok_id );

				socket.emit('Lok_changed',  { who: "Dialog", newLok: lok_id,
            				                                 oldLok: 1
            				                                       });

			}
			oEvent.getSource().getBinding("items").filter([]);
		},


		handleLocomotionFunction: function(oEvent) {	

			if (oEvent.getSource().getPressed()) {
				MessageToast.show(oEvent.getSource().getId() + " Pressed");
			} else {
				MessageToast.show(oEvent.getSource().getId() + " Unpressed");
			}

			var aContexts = oEvent.getParameter("selectedContexts");
			if (aContexts && aContexts.length) {
			    var lok_name = aContexts.map(function(oContext) { return oContext.getObject().name; }).join(", ");
			    var lok_id   = aContexts.map(function(oContext) { return oContext.getObject().id; }).join(", ");

				MessageToast.show("Function " + lok_name + lok_id );

				socket.emit('Lok_changed',  { who: "Dialog", newLok: lok_id,
            				                                 oldLok: 1
            				                                       });

			}		
		},

		handleLocomotionDirection: function(oEvent) {	
			
			var lv_action = oEvent.getParameter("item").getText();
			MessageToast.show(lv_action);

			var lv_data_old = oModelMainController.getData();
			var lv_data_new = lv_data_old;

			if ( lv_action == "Stop" ) { 
				lv_data_new[0].lok_dir = 0; 
			};
			if ( lv_action == "Back" ) { 
				lv_data_new[0].lok_dir = 1; 
			};
			if ( lv_action == "Forward" ) { 
				lv_data_new[0].lok_dir = 2; 
			};

			socket.emit('main_controller_value_changed', {data: lv_data_new });
		},

		handleUserSelectDialogClose: function(oEvent) {
			var aContexts = oEvent.getParameter("selectedContexts");
			if (aContexts && aContexts.length) {
			    var user_name = aContexts.map(function(oContext) { return oContext.getObject().user_name; }).join(", ");
			    var user_id   = aContexts.map(function(oContext) { return oContext.getObject().user_id; }).join(", ");

				MessageToast.show("You are logged in as: " + user_name + "  " + user_id );
			
				var UserData = { "UserData" : [ { "Name" : user_name } ] };

				jQuery.sap.require("jquery.sap.storage");
    			var oStorage = jQuery.sap.storage(jQuery.sap.storage.Type.global);
				oStorage.put("myLocalData", UserData);
				socket.emit('User_changed',  { user_id: user_id, user_name: user_name });

			}
		},



		handleUserNamePress: function(event) {

		},

		onSideNavButtonPress: function() {
			var viewId = this.getView().getId();
			var toolPage = sap.ui.getCore().byId(viewId + "--toolPage");
			var sideExpanded = toolPage.getSideExpanded();

			this._setToggleButtonTooltip(sideExpanded);

			toolPage.setSideExpanded(!toolPage.getSideExpanded());
		},


		_setToggleButtonTooltip: function(bLarge) {
			var toggleButton = this.byId('sideNavigationToggleButton');
			if (bLarge) {
				toggleButton.setTooltip('Large Size Navigation');
			} else {
				toggleButton.setTooltip('Small Size Navigation');
			}
		}

	});

	return CController;

});
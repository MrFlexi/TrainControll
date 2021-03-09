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
	var oModelTrack             = new sap.ui.model.json.JSONModel();

	var gl_canvas;


	//var socket = io.connect(location.protocol + '//' + document.domain + ':' + location.port + '');
	var socket = io.connect('ws://' + document.domain + ':' + location.port + '',{transports: ['websocket']});

	var CController = Controller.extend("view.App", {
		model: new sap.ui.model.json.JSONModel(),

		
		onInit: function() {

		    var namespace = '';

			// Dynamisches Menü
			//this.model.setData(this.data);
			
			this.model.loadData("/static/config/menu.json");
			this.getView().setModel(this.model);
			this.getView().setModel(oModelLokList, "LokListModel");
			this.getView().setModel(oModelUserList, "oModelUserList");
			this.getView().setModel(oModelMainController, "oModelMainController");
			this.getView().setModel(oModelTrack, "oModelTrackList");

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
				//var config_model = jQuery.parseJSON(msg.data)

				//var newArray = config_model.filter(function (el) {
				//	return el.session_id === socket.id
				//  });
				
				//console.log(newArray);
				//oModelMainController.setData(newArray);

                var UserList_json = jQuery.parseJSON(msg.user);
				oModelUserList.setData(UserList_json);
				
				var LokList_data = jQuery.parseJSON(msg.LokList);
				oModelLokList.setData(LokList_data);

				var TrackList_data = jQuery.parseJSON(msg.Track);
				oModelTrack.setData(TrackList_data);

				
             });

            socket.on('config_data', function(msg) {
                var MyLok = jQuery.parseJSON(msg.MyLok)
                oModelMainController.setData(MyLok);

                var UserList_json = jQuery.parseJSON(msg.user);
				oModelUserList.setData(UserList_json);
				
				var LokList_data = jQuery.parseJSON(msg.LokList);
               oModelLokList.setData(LokList_data);

			 });
			 
			 socket.on('gleisplan_data', function(msg) {
                var TrackList_data = jQuery.parseJSON(msg.Track);
				oModelTrack.setData(TrackList_data);

             });

             socket.on('loklist_data', function(msg) {
               var LokList_data = jQuery.parseJSON(msg.LokList);
               oModelLokList.setData(LokList_data);
			});			

		},

		onAfterRendering: function ( oEvent)
		{
			
			var viewId = this.getView().getId();
			var cv = viewId + "--__fabric--canvas";    //
			var gv_tid = 1;
		
			var canvas = new fabric.Canvas(cv);
			gl_canvas = canvas;
			fabric.Object.prototype.originX = fabric.Object.prototype.originY = 'center';


			function makeCircle(left, top, line1, line2, line3, line4) {
				var c = new fabric.Circle({
				  tid : gv_tid,
				  left: left,
				  top: top,
				  strokeWidth: 5,
				  radius: 12,
				  fill: '#fff',
				  stroke: '#666'
				});
				c.hasControls = c.hasBorders = false;
			
				c.line1 = line1;
				c.line2 = line2;
				c.line3 = line3;
				c.line4 = line4;
			
				var text = new fabric.Text(String(gv_tid), { fontSize: 10, left: left+15, top: top-15 });		
				c.text = text;		
				canvas.add(c.text);
				gv_tid++;
				return c;
			  }

			function makeLine(coords) {
				return new fabric.Line(coords, {
				  fill: 'red',
				  stroke: 'red',
				  strokeWidth: 5,
				  selectable: false,
				  evented: false,
				});
			  }

			  var line = makeLine([ 250, 125, 250, 175 ]),
			  line2 = makeLine([ 250, 175, 250, 250 ]),
			  line3 = makeLine([ 250, 250, 300, 350]),
			  line4 = makeLine([ 250, 250, 200, 350]),
			  line5 = makeLine([ 250, 175, 175, 225 ]),
			  line6 = makeLine([ 250, 175, 325, 225 ]);

			  canvas.add(line, line2, line3, line4, line5, line6);

			  canvas.add(
				makeCircle(line.get('x1'), line.get('y1'), null, line),
				makeCircle(line.get('x2'), line.get('y2'), line, line2, line5, line6),
				makeCircle(line2.get('x2'), line2.get('y2'), line2, line3, line4),
				makeCircle(line3.get('x2'), line3.get('y2'), line3),
				makeCircle(line4.get('x2'), line4.get('y2'), line4),
				makeCircle(line5.get('x2'), line5.get('y2'), line5),
				makeCircle(line6.get('x2'), line6.get('y2'), line6)
			  );
	  
			  canvas.on('object:moving', function(e) {
				var p = e.target;

				p.line1 && p.line1.set({ 'x2': p.left, 'y2': p.top });
				p.line2 && p.line2.set({ 'x1': p.left, 'y1': p.top });
				p.line3 && p.line3.set({ 'x1': p.left, 'y1': p.top });
				p.line4 && p.line4.set({ 'x1': p.left, 'y1': p.top });

				p.text && p.text.set({ 'left': p.left+15, 'top': p.top-15 });
				canvas.renderAll();
				
			  });


			  canvas.on('mouse:down', function(options) {
				if (options.target) {
				  console.log('an object was clicked! ', options.target.type, options.target.tid);
				  
				  socket.emit('toggle_turnout', options.target.tid );
				}
			  });
	
		
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

			socket.on('fabric_data', function(msg) {
				canvas.loadFromJSON(msg);
			 });
			
		},

		onTileSelect: function(oEvent) { 
			var sPath = oEvent.getSource().getBindingContext().getPath();
            var oModel = this.getView().getModel();
			var oContext = oModel.getProperty(sPath);
			var viewId = this.getView().getId();

			var navTo = oContext.navigation;
			sap.ui.getCore().byId(viewId + "--pageContainer").to(viewId + "--" + navTo);
		
		},
	
		onItemSelect: function(oEvent) {
			var item = oEvent.getParameter('item');
			var viewId = this.getView().getId();
			sap.ui.getCore().byId(viewId + "--pageContainer").to(viewId + "--" + item.getKey());
		},

		onSliderliveChange: function(oEvent) {
		    socket.emit('main_controller_value_changed', {data: oModelMainController.getData()});
		},

		onTrackPress: function (oEvent) {
			MessageToast.show("Pressed item with ID " + oEvent.getSource().getId());

			var oItem = oEvent.getSource();
			var oCtx = oItem.getBindingContext("oModelTrackList");
			var sPath = oCtx.getPath();
			var oModel = oCtx.getModel();
			var oContext = oModel.getProperty(sPath);

		},


		onTrackDirectionChanged: function (oEvent) {
			MessageToast.show("Pressed item with ID " + oEvent.getSource().getId());

			var oItem = oEvent.getSource();
			var oCtx = oItem.getBindingContext("oModelTrackList");
			var sPath = oCtx.getPath();
			var oModel = oCtx.getModel();
			var oContext = oModel.getProperty(sPath);
			var id = oContext.id;

			var oPara = oEvent.getParameters();
		
			var item = oEvent.getParameter('item');
			var dir = item.getProperty('key');

			socket.emit('track_changed',  { id: id , dir:dir });
		},		

		onTrackButton: function (oEvent) {
			MessageToast.show("Pressed item with ID " + oEvent.getSource().getId());

			var oItem = oEvent.getSource();
			var oPara = oEvent.getParameters();
		
			var item = oEvent.getParameter('item');
			var dir = item.getProperty('key');

			var id = 999;
			socket.emit('track_changed',  { id: id , dir:dir });
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
				lv_data_new.dir = 0; 
			};
			if ( lv_action == "Back" ) { 
				lv_data_new.dir = 1; 
			};
			if ( lv_action == "Forward" ) { 
				lv_data_new.dir = 2; 
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

		onFabricLoad: function(event) {
			socket.emit('onFabricLoad');
		},

		onFabricSave: function(event) {
			var json = gl_canvas.toJSON(['tid']);			  
			  socket.emit('onFabricSave', {data: json});

		},

		
			

		handleUserNamePress: function(event) {
		},

		ImageAreaPressed: function(event) {

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
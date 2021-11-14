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

	var oModelLokList = new sap.ui.model.json.JSONModel();
	var oModelUserList = new sap.ui.model.json.JSONModel();
	var oModelMainController = new sap.ui.model.json.JSONModel();
	var oModelUser = new sap.ui.model.json.JSONModel();
	var oModelTrack = new sap.ui.model.json.JSONModel();


	var gl_canvas;
	var gv_dir = 0;


	//var socket = io.connect(location.protocol + '//' + document.domain + ':' + location.port + '');
	var socket = io.connect('ws://' + document.domain + ':' + location.port + '', { transports: ['websocket'] });

	var CController = Controller.extend("view.App", {
		model: new sap.ui.model.json.JSONModel(),
		oModelSwitches: new sap.ui.model.json.JSONModel(),


		onInit: function () {

			var namespace = '';

			this.model.loadData("/static/config/menu.json");
			this.getView().setModel(this.model);
			this.getView().setModel(oModelLokList, "LokListModel");
			this.getView().setModel(oModelUserList, "oModelUserList");
			this.getView().setModel(oModelMainController, "oModelMainController");
			this.getView().setModel(oModelTrack, "oModelTrackList");
			
			this.oModelSwitches.loadData("/static/config/gleisplan.json");
			this.getView().setModel(this.oModelSwitches, "oModelSwitches");

			document._oController = this;

			socket.on('connect', function () {
				console.log(socket.id);
				MessageToast.show("Connected to server... ");
				//socket.emit('client_global_storage', {data: 'I\'m connected!'});

				//socket.emit('User_changed',  { user_id: "55", user_name: "TestUser" });
			});

			socket.on('disconnect', function () {
				MessageToast.show("Connection lost... ");

			});


			socket.on('initialisation', function (msg) {
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

			socket.on('config_data', function (msg) {
				var MyLok = jQuery.parseJSON(msg.MyLok)
				oModelMainController.setData(MyLok);

				var UserList_json = jQuery.parseJSON(msg.user);
				oModelUserList.setData(UserList_json);

				var LokList_data = jQuery.parseJSON(msg.LokList);
				oModelLokList.setData(LokList_data);

			});

			socket.on('gleisplan_data', function (msg) {
				var TrackList_data = jQuery.parseJSON(msg.Track);
				oModelTrack.setData(TrackList_data);

			});

			socket.on('loklist_data', function (msg) {
				var LokList_data = jQuery.parseJSON(msg.LokList);
				oModelLokList.setData(LokList_data);
			});

		},

		onAfterRendering: function (oEvent) {

			var viewId = this.getView().getId();
			var cv = viewId + "--__fabric--canvas";    //
			var gv_tid = 1;
			var grid = 50;

			var canvas = new fabric.Canvas(cv, {
				fireRightClick: true,
				stopContextMenu: true,
			});

			gl_canvas = canvas;
			fabric.Object.prototype.originX = 'left';
			fabric.Object.prototype.originY = 'top';

			function displayGrid() {
				// create grid

				for (var i = 0; i < (600 / grid); i++) {
					canvas.add(new fabric.Line([i * grid, 0, i * grid, 600], {
						stroke: '#ccc',
						selectable: false
					}));
					canvas.add(new fabric.Line([0, i * grid, 600, i * grid], {
						stroke: '#ccc',
						selectable: false
					}))
				}
			}

			function makeTile(x, y) {
				var rect = new fabric.Rect({
					left: x,
					top: y,
					originX: 'left',
					originY: 'top',
					fill: '#FAFAFA',
					width: 50,
					height: 50,
					strokeWidth: 1,
					stroke: "black",
					rx: 10,
					ry: 10,
					angle: 0,
					scaleX: 1,
					scaleY: 1,
					opacity: 0.7,
					hasControls: false,
					centeredRotation: true
				})
				return rect;
			}

			function makeImage(x, y) {

				return fabric.loadSVGFromURL('/static/images/switch.svg', function (objects, options) {
					var obj2 = fabric.util.groupSVGElements(objects, options);
					return obj2
				});


			}



			function createW(id, x, y, aus) {
				x = x * 50;
				y = y * 50;
				var offset = grid / 2;
				var text = new fabric.Text(String(id), { fontSize: 12, left: x + 5, top: y + 5 });

				var linew1 = makeLineW([x, y + offset, x + grid, y + offset]);
				if (aus == 'left') {
					var linew2 = makeLineW([x, y + offset, x + offset, y]);
				}

				if (aus == 'right') {
					var linew2 = makeLineW([x, y + offset, x + offset, y + grid]);
				}

				//var c1 = makeCircleW(linew1.get('x1'), linew1.get('y1'), linew1, linew2);

				var group = new fabric.Group([makeTile(x, y), linew1, linew2, text], {
					id: id,
					dir: 0,
					angle: 0,
					left: x,
					top: y,
					line1: linew1,
					line2: linew2,
					centeredRotation: true
				});
				return group;
			}


			function ausWl(id, x, y) {
				x = x * 50;
				y = y * 50;
				var offset = grid / 2;

				var linew1 = makeLineW([x + offset, y, x + grid, y + offset]);
				var group = new fabric.Group([makeTile(x, y), linew1], {
					id: id,
					dir: 0,
					angle: 0,
					left: x,
					top: y
				});
				return group;
			}

			function track_g(id, x, y) {
				x = x * 50;
				y = y * 50;
				var offset = grid / 2;

				var linew1 = makeLineW([x, y + offset, x + grid, y + offset]);
				var group = new fabric.Group([makeTile(x, y), linew1], {
					id: id,
					dir: 0,
					angle: 0,
					left: x,
					top: y
				})

				return group;
			}

			
			var oData = this.getView().getModel("oModelSwitches");
			// fire the read request
			var b = this.oModelSwitches.getData();
			console.log(b[1]);
			
			
				displayGrid();
				var w1 = createW(1, 0, 1, "right");
				canvas.add(w1);
				

				var w1 = createW(2, 0, 2, "right");
				canvas.add(w1);
				

				var w1 = createW(3, 0, 3, "left");
				canvas.add(w1);
				


				var w1 = createW(4, 0, 4, "left");
				canvas.add(w1);

				var w1 = createW(5, 0, 5, "right");
				canvas.add(w1);

				var w1 = createW(6, 0, 6, "right");
				canvas.add(w1);

				var w1 = createW(7, 0, 7, "right");
				canvas.add(w1);

				var w1 = createW(8, 0, 8, "left");
				canvas.add(w1);

				var w1 = createW(9, 0, 9, "right");
				canvas.add(w1);


				gv_tid = 20;

				var w1 = ausWl(gv_tid, 2, 2);
				canvas.add(w1);
				gv_tid++;

				var w1 = track_g(gv_tid, 3, 2);
				canvas.add(w1);
				gv_tid++;

				var w1 = track_g(gv_tid, 3, 3);
				canvas.add(w1);
				gv_tid++;


				canvas.on('object:rotated', function (e) {

					var a = Math.round(e.target.angle / 90) * 90;
					e.target.animate('angle', a, {
						onChange: canvas.renderAll.bind(canvas),
						duration: 100,
						centeredRotation: true
					})

				});



				canvas.on('object:moving', function (e) {
					if (Math.round(e.target.left / grid * 1) % 1 == 0 &&
						Math.round(e.target.top / grid * 1) % 1 == 0) {
						e.target.set({
							left: Math.round(e.target.left / grid) * grid,
							top: Math.round(e.target.top / grid) * grid
						}).setCoords();
					}

				});

				function toggle(o) {

					var a = o.target.getObjects();
					var c = o.target;
					var dir = o.target.dir;
					if (dir == 0) {
						c.line1 && c.line1.set({ 'stroke': 'gray', 'strokeWidth': 4 });
						c.line2 && c.line2.set({ 'stroke': 'blue', 'strokeWidth': 4 });
						dir = 1;
					}
					else {
						c.line1 && c.line1.set({ 'stroke': 'blue', 'strokeWidth': 4 });
						c.line2 && c.line2.set({ 'stroke': 'gray', 'strokeWidth': 4 })
						dir = 0;
					}
					o.target && o.target.set({ 'dir': dir })
				}


				canvas.on('mouse:down', function (o) {

					if (o.button === 3) {
						console.log("right click");

						//var ctxTarget = canvas.findTarget(o.originalEvent);

						//var viewId = document._oController.getView().getId();
						//var cv = viewId + "--__fabric--actionSheet";  		
						//var actionsSheet = document._oController.getView().byId(cv);

						// actionsSheet.openBy(o);

					}

					if (o.target) {
						if (o.target.type == 'group') {
							toggle(o);
						}

						if (o.target.type == 'circle') {
							o.target.line2 && o.target.line2.set({ 'stroke': 'green' });
							socket.emit('toggle_turnout', o.target.id);

						}
						console.log('an object was clicked! ', o.target.type, o.target.wid);
						canvas.renderAll();
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
				else {
					console.log("Local storage is empty");
					//this.handleLogonDialog();
					//MessageToast.show("Please login !" );
					var UserData = { "UserData": [{ "Name": "Jochen" }] };
					oModelUser.setData(UserData);
					oStorage.put("myLocalData", UserData);
				}

				socket.on('fabric_data', function (msg) {
					canvas.loadFromJSON(msg);
				});

			},

			onTileSelect: function (oEvent) {
				var sPath = oEvent.getSource().getBindingContext().getPath();
				var oModel = this.getView().getModel();
				var oContext = oModel.getProperty(sPath);
				var viewId = this.getView().getId();

				var navTo = oContext.navigation;
				sap.ui.getCore().byId(viewId + "--pageContainer").to(viewId + "--" + navTo);

			},

			onItemSelect: function (oEvent) {
				var item = oEvent.getParameter('item');
				var viewId = this.getView().getId();
				sap.ui.getCore().byId(viewId + "--pageContainer").to(viewId + "--" + item.getKey());
			},

			onSliderliveChange: function (oEvent) {
				socket.emit('main_controller_value_changed', { data: oModelMainController.getData() });
			},

			onTrackPress: function (oEvent) {
				MessageToast.show("Pressed item with ID " + oEvent.getSource().getId());

				var oItem = oEvent.getSource();
				var oCtx = oItem.getBindingContext("oModelTrackList");
				var sPath = oCtx.getPath();
				var oModel = oCtx.getModel();
				var oContext = oModel.getProperty(sPath);

			},


			onCTXPress: function(oEvent) {

				var viewId = this.getView().getId();
				var cv = viewId + "--__fabric--actionSheet";

				var oButton = oEvent.getSource();
				this.byId(cv).openBy(oButton);
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

				socket.emit('track_changed', { id: id, dir: dir });
			},

			onTrackButton: function (oEvent) {
				MessageToast.show("Pressed item with ID " + oEvent.getSource().getId());

				var oItem = oEvent.getSource();
				var oPara = oEvent.getParameters();

				var item = oEvent.getParameter('item');
				var dir = item.getProperty('key');

				var id = 999;
				socket.emit('track_changed', { id: id, dir: dir });
			},


			handleLogonDialog: function () {
				if (!this._oDialogLogon) {
					this._oDialogLogon = sap.ui.xmlfragment("view.fragments.Logon", this);
				}

				this.getView().addDependent(this._oDialogLogon);
				// toggle compact style
				jQuery.sap.syncStyleClass("sapUiSizeCompact", this.getView(), this._oDialogLogon);
				this._oDialogLogon.open();

			},


			handleTableSelectDialogPress: function (oEvent) {
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

			handleLocomotionSelectDialogClose: function (oEvent) {
				var aContexts = oEvent.getParameter("selectedContexts");
				if (aContexts && aContexts.length) {
					var lok_name = aContexts.map(function (oContext) { return oContext.getObject().name; }).join(", ");
					var lok_id = aContexts.map(function (oContext) { return oContext.getObject().id; }).join(", ");

					MessageToast.show("You have chosen " + lok_name + lok_id);

					socket.emit('Lok_changed', {
						who: "Dialog", newLok: lok_id,
						oldLok: 1
					});

				}
				oEvent.getSource().getBinding("items").filter([]);
			},

			onButtonEdit: function (oEvent) {
				var aContexts = oEvent.getParameter("selectedContexts");
				if (aContexts && aContexts.length) {
					var lok_name = aContexts.map(function (oContext) { return oContext.getObject().name; }).join(", ");
					var lok_id = aContexts.map(function (oContext) { return oContext.getObject().id; }).join(", ");

					MessageToast.show("You have chosen " + lok_name + lok_id);

					

				}
				//oEvent.getSource().getBinding("items").filter([]);
			},



			handleLocomotionFunction: function (oEvent) {

				if (oEvent.getSource().getPressed()) {
					MessageToast.show(oEvent.getSource().getId() + " Pressed");
				} else {
					MessageToast.show(oEvent.getSource().getId() + " Unpressed");
				}

				var aContexts = oEvent.getParameter("selectedContexts");
				if (aContexts && aContexts.length) {
					var lok_name = aContexts.map(function (oContext) { return oContext.getObject().name; }).join(", ");
					var lok_id = aContexts.map(function (oContext) { return oContext.getObject().id; }).join(", ");

					MessageToast.show("Function " + lok_name + lok_id);

					socket.emit('Lok_changed', {
						who: "Dialog", newLok: lok_id,
						oldLok: 1
					});

				}
			},

			handleLocomotionDirection: function (oEvent) {

				var lv_action = oEvent.getParameter("item").getText();
				MessageToast.show(lv_action);

				var lv_data_old = oModelMainController.getData();
				var lv_data_new = lv_data_old;

				if (lv_action == "Stop") {
					lv_data_new.dir = 0;
				};
				if (lv_action == "Back") {
					lv_data_new.dir = 1;
				};
				if (lv_action == "Forward") {
					lv_data_new.dir = 2;
				};

				socket.emit('main_controller_value_changed', { data: lv_data_new });
			},

			handleUserSelectDialogClose: function (oEvent) {
				var aContexts = oEvent.getParameter("selectedContexts");
				if (aContexts && aContexts.length) {
					var user_name = aContexts.map(function (oContext) { return oContext.getObject().user_name; }).join(", ");
					var user_id = aContexts.map(function (oContext) { return oContext.getObject().user_id; }).join(", ");

					MessageToast.show("You are logged in as: " + user_name + "  " + user_id);

					var UserData = { "UserData": [{ "Name": user_name }] };

					jQuery.sap.require("jquery.sap.storage");
					var oStorage = jQuery.sap.storage(jQuery.sap.storage.Type.global);
					oStorage.put("myLocalData", UserData);
					socket.emit('User_changed', { user_id: user_id, user_name: user_name });

				}
			},

			onFabricLoad: function (event) {
				socket.emit('onFabricLoad');
			},

			onFabricSave: function (event) {
				var json = gl_canvas.toJSON(['id']);
				socket.emit('onFabricSave', { data: json });

			},

			onFabricPlay: function (oEvent) {
				var state = oEvent.getParameter('state');
				if ( state )
				{
					gl_canvas.forEachObject(function(o){ 
						o.hasControls = true;
						o.lockMovementX = false;
						o.lockMovementY = false;
						o.hasBorders = true;
					});
				}
				else
				{
					gl_canvas.forEachObject(function(o){ 
						o.hasControls = false;
						o.lockMovementX = true;
						o.lockMovementY = true;
						o.hasBorders = false;
					});
				}
			},




			handleUserNamePress: function (event) {
			},

			ImageAreaPressed: function (event) {

			},

			onSideNavButtonPress: function () {
				var viewId = this.getView().getId();
				var toolPage = sap.ui.getCore().byId(viewId + "--toolPage");
				var sideExpanded = toolPage.getSideExpanded();

				this._setToggleButtonTooltip(sideExpanded);

				toolPage.setSideExpanded(!toolPage.getSideExpanded());
			},


			_setToggleButtonTooltip: function (bLarge) {
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
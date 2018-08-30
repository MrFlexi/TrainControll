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
	var oModelMainController    = new sap.ui.model.json.JSONModel();
	var socket = io.connect(location.protocol + '//' + document.domain + ':' + location.port + '');

	var CController = Controller.extend("view.App", {
	    Formatter: Formatter,
		model: new sap.ui.model.json.JSONModel(),
		model_lok: new sap.ui.model.json.JSONModel(),

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
			this.getView().setModel(oModelMainController, "oModelMainController");

            socket.on('connect', function() {
                socket.emit('i_am_connected', {data: 'I\'m connected!'});
            });

            socket.on('config_data', function(msg) {
                var config_model = jQuery.parseJSON(msg.data)
                oModelMainController.setData(config_model);
             });

             socket.on('loklist_data', function(msg) {
               var LokList_data = jQuery.parseJSON(msg.LokList);
               oModelLokList.setData(LokList_data);
            });


		},

		onItemSelect: function(oEvent) {
			var item = oEvent.getParameter('item');
			var viewId = this.getView().getId();
			sap.ui.getCore().byId(viewId + "--pageContainer").to(viewId + "--" + item.getKey());
		},

		onSliderliveChange: function(oEvent) {
		    socket.emit('main_controller_value_changed', {data: oModelMainController.getData()});
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
			}
			oEvent.getSource().getBinding("items").filter([]);
		},



		handleUserNamePress: function(event) {
			var popover = new Popover({
				showHeader: false,
				placement: sap.m.PlacementType.Bottom,
				content: [
					new Button({
						text: 'Feedback',
						type: sap.m.ButtonType.Transparent
					}),
					new Button({
						text: 'Help',
						type: sap.m.ButtonType.Transparent
					}),
					new Button({
						text: 'Logout',
						type: sap.m.ButtonType.Transparent
					})
				]
			}).addStyleClass('sapMOTAPopover sapTntToolHeaderPopover');

			popover.openBy(event.getSource());
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
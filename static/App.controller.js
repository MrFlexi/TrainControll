sap.ui.define([
	'jquery.sap.global',
	'sap/ui/core/Fragment',
	'sap/ui/core/mvc/Controller',
	'sap/ui/model/json/JSONModel',
	'sap/m/Popover',
	'sap/m/Button'
], function (jQuery, Fragment, Controller, JSONModel, Popover, Button) {
	"use strict";

	var CController = Controller.extend("view.App", {
		model: new sap.ui.model.json.JSONModel(),
		model_lok: new sap.ui.model.json.JSONModel(),

		data_lok: {
			"lok": [{
				"id": "1",
				"name": "ETA 515",
				"image": "https://img00.deviantart.net/7463/i/2012/092/5/7/eta_515_by_igneo78-d4upbtb.png",
				"type": "DCC",
				"addr": "3"
			}, {
				"id": "2",
				"name": "Portfeeder",
				"image": "sap-icon://employee",
				"type": "MFX",
				"addr": "1"
			}]
		},

		data: {
			navigation: [{
				title: 'User',
				icon: 'sap-icon://employee',
				expanded: true,
				key: 'root1',
				items: [{
					title: 'List',
					key: 'user_p1'
				}, {
					title: 'New',
					key: 'user_p2'
				}]
			}, {
				title: 'Locomotion',
				icon: 'sap-icon://card',
				expanded: false,
				items: [{
					title: 'List',
					key: 'lok_list'
				}, {
					title: 'New'
				}, {
					title: 'Change'
				}]
			}, {
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
			this.model.setData(this.data);
			this.getView().setModel(this.model);

			this.model_lok.setData(this.data_lok);
			this.getView().setModel(this.model_lok, "LokListModel");
			this._setToggleButtonTooltip(!sap.ui.Device.system.desktop);
		},

		onItemSelect: function(oEvent) {
			var item = oEvent.getParameter('item');
			var viewId = this.getView().getId();
			sap.ui.getCore().byId(viewId + "--pageContainer").to(viewId + "--" + item.getKey());
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
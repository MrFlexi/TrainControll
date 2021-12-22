sap.ui.define(function() {
	"use strict";

	var Formatter = {	

		status :  function (sStatus) {
				if (sStatus === "available") {
					return "Success";
				} else if (sStatus === "Out of Stock") {
					return "Warning";
				} else if (sStatus === "bound"){
					return "Error";
				} else {
					return "Error";
				}
		},

		onoff :  function (sStatus) {
			if (sStatus === "0") {
				return false;
			} 
			else if (sStatus === "1") {
				return true;
			}
		},

		TrackIconVisibleLeft:function(input) {
			if (input === "left") {
				return true;
			} 
			else 
			{
				return false;
			}			
		},

		TrackIconVisibleRight :  function (input) {
			if (input === "right") {
				return true;
			} 
			else 
			{
				return false;
			}			
		},

		TrackActualDir :  function (data ) {

			var ev = "";

			if (data.dir === 0 ) { 
				ev = "straight";
			}
			else { 
				ev = "right";
				 }
			
			return ev;
		},

		DirIndicator :  function (dir) {
			var ev = "None";
			if (dir === 1 ) { 
				ev = "Down";
			}
			if (dir === 2 ) { 
				ev = "Up";
			}
			return ev;
		},

		trackicon :  function (sStatus) {
				if (sStatus === "left") {
					return "sap-icon://forward";
				} 
				else if (sStatus === "right") {
					return "sap-icon://response";
				}	
				else return "sap-icon://arrow-top";
				}		
	};	

	return Formatter;

}, /* bExport= */ true);
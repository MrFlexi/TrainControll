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
	}


	};

	return Formatter;

}, /* bExport= */ true);
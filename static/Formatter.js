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
		}
	};

	return Formatter;

}, /* bExport= */ true);
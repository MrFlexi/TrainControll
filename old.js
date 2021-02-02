// bei jedem Neuaufbau der Seite
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
    //var UserData = { "UserData" : [ { "Name" : "Jochen" } ] };
    //oModelUser.setData(UserData);
    //oStorage.put("myLocalData", UserData);
    }

    
},
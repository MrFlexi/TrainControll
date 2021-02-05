import json

from TrainControllLok import Lok


# Controller
class CTRL:
    @staticmethod
    def change_lok(client_id, data_in):
        print (json.dumps(data_in, indent=1, separators=(',', ': ')))

        print ("Changed by" , data_in["who"])
        if data_in["who"] == "Carousel":
            lok_new = int(data_in["newLok"][-1:]) + 1
        
        if data_in["who"] == "Dialog":
            lok_new = int(data_in["newLok"][-1:])

        print ("Lok change requested. New Locomotion: " , Lok.getName(lok_new))
        Lok.bindLokID(client_id, lok_new, "Hallo")
        

    @staticmethod
    def getDataJSONforClient(client_id):
        return (Lok.getDataJSONforClient(client_id))

    @staticmethod
    def setClientData(client_id, data_in ):
        print ("SetClientData " + json.dumps(data_in))
        Lok.setNewData(data_in["data"])

    @staticmethod
    def printListe():
        jd = ""
        i = CTRL.List.values()
        print("Controller Liste")
        for x in i:
            x.printCTRL()
        #print("Controller Liste JSON Dump")
        #print( CTRL.getDataJSON() )

    @staticmethod
    def setS(controller_id, speed):
        CTRL.List[controller_id].setSpeed(speed)

    def printCTRL(self):
        print("SessionID", self.session_id)
        print("ClientID", self.client_id)
        print("LokID", self.lok_id)
        print("LokName", self.lok_name)
        print("Speed", self.lok_speed)

    def setSpeed(self, speed):
        self.lok_speed = speed
        print("-->SetSpeed")
        print("Name", self.lok_name)
        print("Speed", self.lok_speed)

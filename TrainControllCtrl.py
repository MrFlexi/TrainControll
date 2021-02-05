import json

from TrainControllLok import Lok
from TrainControll import UDP

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

        # Data comming from client is a JSON Array. In this case it should be only one element in the array
        MyLok = data_in["MyLok"]

        speed = MyLok["speed"]
        lok_id = MyLok["id"]

        #SPEED
        if Lok.getSpeed(lok_id) != speed:
            Lok.setSpeed(lok_id, speed)
            print ("Speed was changed", lok_id, speed)
            # Update speed, UDP Paket an Raspbery CS2 Emulation senden
            UDP.setSpeed(lok_id,speed)

        # DIRECTION
        #if gr_instance.lok_dir != direction:
        #    print ("Direction was changed", gr_instance.lok_dir, direction)
            # Update speed, UDP Paket an Raspbery CS2 Emulation senden

        #    if direction == "0":
        #        UDP.setSpeed(gr_instance.lok_id, 0)
        #    else:
        #        UDP.setDir(gr_instance.lok_id,direction)
        #    # Update values in instance
        #    gr_instance.lok_dir = direction
        #    gr_instance.lok_speed = 0

            # function F1..F10
        #if gr_instance.lok_f1 != f1:
        #    print ("Lok function", gr_instance.lok_f1, f1)
        #    # Update speed, UDP Paket an Raspbery CS2 Emulation senden
        #    if f1: UDP.setLokFunction(gr_instance.lok_id, 0, 1)
        #    else: UDP.setLokFunction(gr_instance.lok_id, 0, 0)

            # Update values in instance
        #    gr_instance.lok_dir = direction
        #    gr_instance.lok_f1    = f1

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

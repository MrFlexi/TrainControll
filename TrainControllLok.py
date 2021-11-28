from pathlib import Path
import json
from TrainControll import UDP

class Lok:
    
    LokList = []
    def __init__(self,  id, addr, protocol, name, image_url ):
        #Create empty dictionary
        self.id = id
        self.speed = 0
        self.dir = 0
        self.name = name
        self.image_url = image_url
        self.addr = addr
        self.protocol = protocol
        self.client_id = ""
        self.status = "available"
        self.user_name = ""
        self.f1 = 1
        self.f2 = 0
        self.f3 = 0

        Lok.LokList.append(self)


    def find_ById(search_key):
        for obj in Lok.LokList:
            if obj.id == search_key:
                return obj

    def find_ByClient(search_key):
        for obj in Lok.LokList:
            if obj.client_id == search_key:
                return obj
    
    @staticmethod
    def setNewData(data):
        id = int(data["id"])
        speed = int(data["speed"])
        dir = int(data["dir"])

        x = Lok.find_ById(id)
        if (x):
            print("Locomotion found in LocList")
            if ( x.speed != speed ):
                Lok.setSpeed(id, speed)
                print ("Speed was changed", id, speed)

            if ( x.dir != dir ):
                Lok.setDir(id, dir)
                print ("Direction changed", id, dir)

        Lok.printLokList()

    @staticmethod
    def setDataById(id,client_id):
        x = Lok.find_ById(id)
        if (x):
            x.client_id = client_id
    @staticmethod
    def setDataByClient(client_id, name):
        for x in Lok.LokList:
            if ( x.client_id == client_id):
                x.name = name


    @staticmethod
    def printLokList():
        print("-------------------------")
        print("Lokliste")
        
        for x in Lok.LokList:
            print( x.id, x.client_id, x.name, x.user_name, x.speed)
        print("-------------------------")

    def getDataJSON():
        jd = []
        for x in Lok.LokList:
            jd.append(x.__dict__)
        return (json.dumps(jd))

    def getDataJSONforClient(client_id):
        print ("get")
        x = Lok.find_ByClient(client_id)
        if (x):
            return (json.dumps(x.__dict__))

    @staticmethod
    def bindLokID(client_id, lok_new, user_name):

        #Delete old binding
        x = Lok.find_ByClient(client_id)
        if (x):
            x.client_id = ''
            x.speed = 0
            x.status = "available"
            x.user_name = ''

        #Set new binding
        x = Lok.find_ById(lok_new)
        if (x):
            x.client_id = client_id
            x.status = "blocked"
            x.user_name = user_name
            x.speed = 0

    @staticmethod
    def save():
        jsonData = Lok.getDataJSON()
        f = open("./config/loklist.json", "w")  # opens file with name of "test.txt"
        f.write(jsonData)
        f.close()

    @staticmethod
    def getImage(id):
        x = Lok.find_ById(id)
        if (x):
            return ( x.image_url )
        else:
            return ("./static/images/Lok.png")

    @staticmethod
    def getName(id):
        x = Lok.find_ById(id)
        if (x):
            return ( x.name )
        else:
            return ("not found")

    @staticmethod
    def getSpeed(id):
        x = Lok.find_ById(id)
        if (x):
            return ( x.speed )
        else:
            return (0)

    @staticmethod
    def setSpeed(id, speed):
        x = Lok.find_ById(id)
        if (x):
            x.speed = speed
            # Update speed, UDP Paket an Raspbery CS2 Emulation senden
            UDP.setSpeed(x.addr,x.speed)

    @staticmethod
    def setDir(id, dir):
        x = Lok.find_ById(id)
        if (x):
            x.dir = dir
            # Update UDP Paket an Raspbery CS2 Emulation senden
            UDP.setDir(x.addr,x.dir)
       

    @staticmethod
    def getAddr(id):
        x = Lok.find_ById(id)
        if (x):
            return ( x.addr )
        else:
            return (0)

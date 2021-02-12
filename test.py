import json
import collections
from TrainControll import UDP
from pathlib import Path

class Gleisplan:
    Liste = []
    def __init__(self, id, addr, x1,y1,x2,y2, dir, type, aus ):
        # Create empty dictionary
        self.id = id
        self.addr = addr
        self.x1 = x1
        self.x2 = x2
        self.y1 = y1
        self.y2 = y2
        self.type = type
        self.aus = aus
        self.dir = dir
        self.position = "left"
        self.pos = 0

        Gleisplan.Liste.append(self)
    
    @staticmethod
    def find_ById(search_key):
        for obj in Gleisplan.Liste:
            if obj.id == search_key:
                return obj

    @staticmethod
    def new(message):
        jsonData = json.dumps(message, indent=1, separators=(',', ': '))
        print(jsonData)

        count = len(Gleisplan.Liste)
        new_id = count + 1

        Gleisplan(id=new_id, addr=0, x1=message["x1"], x2=message["x2"], y1=message["y1"], y2=message["y2"], dir=0, type="DCC")

        Gleisplan.printGleisplan()

    @staticmethod
    def toggle_turnout(message):
        print ("Toggle Turnout")
        print (message)

        lv_id = int(message)

        gr_instance = Gleisplan.Liste[lv_id]
        gr_instance.printGP()

        if gr_instance.dir == 0:
            gr_instance.dir = 1
        else:
            gr_instance.dir = 0

        UDP.setFunction( lv_id,  gr_instance.dir )


    @staticmethod
    def set_all_turnout(dir):
        print("Gleisplan old")
        Gleisplan.printGleisplan()

        for x in Gleisplan.Liste:
            x.setDir(dir)
            
        print("Gleisplan new")
        Gleisplan.printGleisplan()
         

    def setDir(self,dir):
            print("before", self.id, self.dir)
            self.dir = dir
            UDP.setFunction( self.addr,  self.dir )
            print("after", self.id, self.dir)
            

    @staticmethod
    def set_turnout(id,dir):
        if id != 999:
            if( dir == "straight"): new_dir = 0
            if( dir == "right"): new_dir = 1
            if( dir == "left"): new_dir = 1

            x = Gleisplan.find_ById(id)
            if (x):
                x.setDir(new_dir)
            else: 
                print( "ID not found"), id
        else: 
            Gleisplan.set_all_turnout(int(dir))

    @staticmethod
    def save(message):
        print ("Gleisplan Save")
        #jsonData = json.dumps(message, indent=1, separators=(',', ': '))
        #print(jsonData)

        for item in message:
            #print (item)
            # Get class instance
            gr_instance = Gleisplan.Liste[item["id"]]
            gr_instance.printGP()

            #Compare old and new values. if different set new value

            if gr_instance.addr != item["addr"]:
                print ("Address was changed from " +  str(gr_instance.addr) + " to: " + str(item["addr"]))
                # Update values in instance
                gr_instance.addr = item["addr"]


        jsonData = Gleisplan.getDataJSON()
        #print(jsonData)
        f = open("./config/gleisplan.json", "w")  # opens file with name of "test.txt"
        f.write(jsonData)
        f.close()


    @staticmethod
    def getDataJSON():
        jd = []
        i = Gleisplan.Liste
        for x in i:
           jd.append(x.__dict__)
        return (json.dumps(jd ,indent=1, separators=(',', ': ')))

    @staticmethod
    def printGleisplan():
        for x in Gleisplan.Liste:
            print(x.id, x.addr, x.type, x.aus, x.pos, x.dir)


    def printGP(self):
        print ("Weiche: " + str(self.id) +  " Addr: " + str(self.addr) + " Dir: " + str(self.dir))


# Load Gleisplan
with open('config/gleisplan.json') as data_file:
    gleisplan_json = json.load(data_file)
for item in gleisplan_json:
    # Create Instances for each lok
    Gleisplan( id=item["id"], addr=item["addr"],x1=item["x1"],x2=item["x2"],y1=item["y1"],y2=item["y2"],dir=item["dir"], type="DCC",aus=item["aus"]  )

Gleisplan.printGleisplan()

Gleisplan.set_turnout(2,"right")

for x in Gleisplan.Liste:
    x.dir = 2

for x in Gleisplan.Liste:
    print(x.id, x.dir)

Gleisplan.printGleisplan()

from pathlib import Path
import json

class Lok:
    
    LokList = []
    def __init__(self,  id, addr, protocol, name, image_url ):
        #Create empty dictionary
        self.id = id
        self.name = name
        self.image_url = image_url
        self.addr = addr
        self.protocol = protocol
        self.client_id = ""
        self.status = "available"
        self.user_name = ""
        Lok.LokList.append(self)

    @staticmethod
    def printLokList():
        print("-------------------------")
        print("Lokliste")

        for x in Lok.LokList:
            if ( x.name == "S-BAHN"):
                x.client_id = 99

        for x in Lok.LokList:
            print( x.id, x.client_id, x.name)
        print("-------------------------")

        a = filter(lambda x: x.client_id == 99, Lok.LokList)
        print(a)


filename = Path('config/loklist.json')
if not filename.exists():
    print("Oops, file doesn't exist!")

data_file = open(filename)
loklist_json = json.load(data_file)

for item in loklist_json:
    # Create Instances for each lok
    Lok(id=item["id"],name=item["name"], image_url=item["image_url"], addr=item["addr"], protocol=item["protocol"])

Lok.printLokList()
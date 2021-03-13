
import json
import socket
import io

from time import sleep


# Define Class CPU
class UDP:

    UDP_IP = "192.168.178.100"
    UDP_PORT = 15731

    @staticmethod
    def setFunction(iv_id, value):
        hex_data1 = "0016131406000038"
        hex_data2 = "010000"

        iv_id = iv_id - 1

        print ("UDPSetFunction:" + str(iv_id) + " Value:" + str(value) + " UDP IP:", UDP.UDP_IP + " Port:", UDP.UDP_PORT)       

        message = bytes.fromhex(hex_data1)  + iv_id.to_bytes(1, byteorder='big')  + value.to_bytes(1, byteorder='big') + bytes.fromhex(hex_data2)
        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (UDP.UDP_IP, UDP.UDP_PORT))

    @staticmethod
    def setSpeed(lv_addr, iv_speed):
        hex_data1 = "00081314060000c0"
        hex_data2 = "0000"

        if lv_addr > 0:
            print ("DCC Address:" + str(lv_addr))
            message = bytes.fromhex(hex_data1) + lv_addr.to_bytes(1, byteorder='big') + int(iv_speed * 10 ).to_bytes(2, byteorder='big') + bytes.fromhex(hex_data2)
            print("UDP Message", message )
            #message shold look like this ('UDP Message', '\x00\x08\x13\x14\x06\x00\x00\xc0\x05\x00\x14\x00\x00')
            print ("UDP IP:", UDP.UDP_IP + " Port:", UDP.UDP_PORT)
            sock = socket.socket(socket.AF_INET,  # Internet
                                socket.SOCK_DGRAM)  # UDP
            sock.sendto(message, (UDP.UDP_IP, UDP.UDP_PORT))

    @staticmethod
    def setLokFunction(lv_addr, func, value):
        hex_data1 = "000c1314060000c0"
        hex_data2 = "0000"

        lv_addr_str = chr(lv_addr)
        print ("DCC Address:" + str(lv_addr))

        lv_func = chr(func)
        lv_val = chr(value)

        message = bytes.fromhex(hex_data1) + lv_addr.to_bytes(1, byteorder='big') + func.to_bytes(1, byteorder='big') + value.to_bytes(1, byteorder='big') + bytes.fromhex(hex_data2)

        print ("UDP IP:", UDP.UDP_IP + " Port:", UDP.UDP_PORT)

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (UDP.UDP_IP, UDP.UDP_PORT))

    @staticmethod
    def setDir(lv_addr, iv_dir):

        # Message muss immer 13 Byte lang sein.
        # hex_data1 = "000A4711050000c00301000000"
        hex_data1 = "000A4711050000c0"
        hex_data2 = "000000"
    
        #print ("Set Direction:")

        message = bytes.fromhex(hex_data1) + lv_addr.to_bytes(1, byteorder='big') + iv_dir.to_bytes(1, byteorder='big') + bytes.fromhex(hex_data2)

        print ("UDP IP:", UDP.UDP_IP + " Port:", UDP.UDP_PORT)

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (UDP.UDP_IP, UDP.UDP_PORT))

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

        Gleisplan(id=new_id, addr=0, x1=message["x1"], x2=message["x2"], y1=message["y1"], y2=message["y2"], dir=0, type="DCC", aus="right")

        Gleisplan.printGleisplan()

    @staticmethod
    def toggle_turnout(message):
        print ("Toggle Turnout")
        print (message)

        lv_id = int(message)

        gr_instance = Gleisplan.find_ById(lv_id)
        gr_instance.printGP()

        if gr_instance.dir == 0:
            gr_instance.dir = 1
        else:
            gr_instance.dir = 0

        UDP.setFunction( gr_instance.addr,  gr_instance.dir )


    @staticmethod
    def set_all_turnout(dir):
        for x in Gleisplan.Liste:
            x.setDir(dir)
            sleep(0.2)          #Man muss ein bisschen auf die Weichen warten
         

    def setDir(self,dir):
            self.dir = dir
            UDP.setFunction( self.addr,  self.dir )

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
    def fabric_save(message):
        print ("Fabric Save")
        jsonData = json.dumps(message, indent=1, separators=(',', ': '))
        #print(jsonData)        
               
        f = open("./config/fabric.json", "w")  # opens file with name of "test.txt"
        f.write(jsonData)
        f.close()

    @staticmethod
    def fabric_load():
        print ("Fabric Load")      
        with open('./config/fabric.json') as data_file:
            fabric_json = json.load(data_file)
        return (fabric_json)

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

    @staticmethod
    def getAddr(Id):
        if Id in Gleisplan.Liste:
            return (Gleisplan.Liste[Id].addr)

    def printGP(self):
        print ("Weiche: " + str(self.id) +  " Addr: " + str(self.addr) + " Dir: " + str(self.dir))


# Define Class Client
class Clients:
    mt_clients = {}
    Count = 0

    def __init__(self, user_name):
     print ("Class Constructor")

    @staticmethod
    def newClient(sid):
     Clients.Count =+ Clients.Count + 1
     #print ("New client connected: " + sid)
     
     # Add new entry
     #Clients.mt_clients[Clients.Count] =  sid
     Clients.mt_clients[sid] =  sid
     print("Clients: ",len(Clients.mt_clients))
     print(Clients.mt_clients)
    

    @staticmethod
    def setUserName(client_ID, user_name):
        print ("Link Client to User")

        if client_ID in Clients.mt_clients:
            Clients.mt_clients[client_ID].user_name = user_name

        print (" Class - Clients")
        print (Clients.mt_clients)
        print ("Anzahl:", len(Clients.mt_clients))

    @staticmethod
    def getClientIDfromSID(sid):

    # Printing dictionary 
        #print ("Original dictionary is : " + str(Clients.mt_clients)) 
        #print ("Dict key-value are : ") 
        #for key, value in Clients.mt_clients.items():    
        #    print (key, value)
        #    if value == sid:
        #        return(key)
        #        break
        return sid
    

    @staticmethod
    def deleteClient(client_ID):
     #print ("Delete Client")

     if client_ID in Clients.mt_clients:
         del Clients.mt_clients[client_ID]

     #print ("Clients: ",len(Clients.mt_clients))
     #print(Clients.mt_clients)
    

    @staticmethod
    def getClientsCount():
        n = len(Clients.mt_clients)
        #print("Clients:", n)
        #print(Clients.mt_clients)
        return n


# Define Class User
class User:

 UserList = {}
 count = 0
 def __init__(self,  user_id, user_name, image_url, client_id ):
       #Create empty dictionary
       self.user_id = user_id
       self.user_name = user_name
       self.image_url = image_url
       self.client_id = client_id
       self.status = "offline"
       User.UserList[user_id] = self
       User.count = + 1

 @staticmethod
 def getDataJSON():
    jd = []
    i = User.UserList.values()
    for x in i:
         jd.append(x.__dict__)
    return (json.dumps(jd))


 @staticmethod
 def printUserList():
     i = User.UserList.values()
     print("-------------------------")
     print("Userliste")
     for x in i:
       x.printUser()
     print("-------------------------")

 @staticmethod
 def save():
     jsonData = User.getDataJSON()
     f = open("./config/userlist.json", "w")  # opens file with name of "test.txt"
     f.write(jsonData)
     f.close()

 @staticmethod
 def getImage(user_id):
     if user_id in User.UserList:
        return ( User.UserList[user_id].image_url )

 @staticmethod
 def getName(user_id):
     if user_id in User.UserList:
        return ( User.UserList[user_id].user_name )

 def printUser(self):
        print ("User:" + str(self.user_id) +" " + self.user_name + " " + self.image_url)

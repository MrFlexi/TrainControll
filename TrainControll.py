
import json
import socket
import io

# Define Class CPU
class UDP:

    UDP_IP = "192.168.1.100"
    UDP_PORT = 15731

    @staticmethod
    def setFunction(iv_id, value):
        hex_data1 = "0016131406000038"
        hex_data2 = "010000"


        lv_idstr = chr(iv_id - 1)
        print ("SetFunction")
        print ("DCC Address:" + str(iv_id) + " Value:" + str(value))

        lv_val = chr(value)

        message = hex_data1.decode("hex") + lv_idstr + lv_val + hex_data2.decode("hex")

        print ("UDP IP:", UDP.UDP_IP + " Port:", UDP.UDP_PORT)

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (UDP.UDP_IP, UDP.UDP_PORT))

       




    @staticmethod
    def setSpeed(iv_lok_id, iv_speed):
        hex_data1 = "00081314060000c0"
        hex_data2 = "0000"


        lv_addr = Lok.getAddr(iv_lok_id)
        print ("DCC Address:" + str(lv_addr))

        message = bytes.fromhex(hex_data1) + lv_addr.to_bytes(1, byteorder='big') + int(iv_speed * 10 ).to_bytes(2, byteorder='big') + bytes.fromhex(hex_data2)
        

        print("UDP Message", message )
        #message shold look like this ('UDP Message', '\x00\x08\x13\x14\x06\x00\x00\xc0\x05\x00\x14\x00\x00')

        print ("UDP IP:", UDP.UDP_IP + " Port:", UDP.UDP_PORT)

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (UDP.UDP_IP, UDP.UDP_PORT))

    @staticmethod
    def setLokFunction(iv_lok_id, func, value):
        hex_data1 = "000c1314060000c0"
        hex_data2 = "0000"

        lv_addr = Lok.getAddr(iv_lok_id)
        lv_addr_str = chr(lv_addr)
        print ("DCC Address:" + str(lv_addr))

        lv_func = chr(func)
        lv_val = chr(value)

        message = hex_data1.decode("hex") + lv_addr_str + lv_func + lv_val + hex_data2.decode("hex")

        print ("UDP IP:", UDP.UDP_IP + " Port:", UDP.UDP_PORT)

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (UDP.UDP_IP, UDP.UDP_PORT))

    @staticmethod
    def setDir(iv_lok_id, iv_dir):

        # Message muss immer 13 Byte lang sein.
        # hex_data1 = "000A4711050000c00301000000"
        hex_data1 = "000A4711050000c0"
        hex_data2 = "000000"

        lv_dir_str = chr(0)
        if iv_dir == "back":
            lv_dir_str = chr(2)
        elif iv_dir == "neutral":
            lv_dir_str = chr(0)
        elif iv_dir == "forward":
            lv_dir_str = chr(1)

        lv_addr = Lok.getAddr(iv_lok_id)
        lv_addr_str = chr(lv_addr)
        # lv_dir_str = chr(iv_dir)
        print ("Set Direction:")

        message = hex_data1.decode("hex") + lv_addr_str + lv_dir_str + hex_data2.decode("hex")

        print ("UDP IP:", UDP.UDP_IP + " Port:", UDP.UDP_PORT)

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (UDP.UDP_IP, UDP.UDP_PORT))

class Gleisplan:
    Liste = {}
    def __init__(self, id, addr, x1,y1,x2,y2, dir, type ):
            # Create empty dictionary
            self.id = id
            self.addr = addr
            self.x1 = x1
            self.x2 = x2
            self.y1 = y1
            self.y2 = y2
            self.type = type
            self.dir = dir
            Gleisplan.Liste[id] = self

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
        i = Gleisplan.Liste.values()
        for x in i:
           jd.append(x.__dict__)
        return (json.dumps(jd ,indent=1, separators=(',', ': ')))

    @staticmethod
    def printGleisplan():
            i = Gleisplan.Liste.values()
            print("-------------------------")
            print("Gleisplanliste")
            for x in i:
                x.printGP()
            print("-------------------------")

    @staticmethod
    def getAddr(Id):
        if Id in Gleisplan.Liste:
            return (Gleisplan.Liste[Id].addr)

    def printGP(self):
        print ("Weiche: " + str(self.id) +  " Addr: " + str(self.addr) + " Dir: " + str(self.dir))



# Define Class CPU
class CPU:

 Mapping = {}

 def __init__(self, client_id, lok_id):
    self.client_id = client_id
    self.lok_id = lok_id
    print ("Mapping Constructor")
    CPU.Mapping[client_id] = lok_id


 @staticmethod
 def getLokIDfromClientId(client_id):
     if client_id in CPU.Mapping:
        return(CPU.Mapping[client_id])
     else:
        return()

 @staticmethod
 def printListe():
     i = CPU.Mapping.values()

     print("CPU Liste")
     print (CPU.Mapping)
     #for x in i:
     #    x.printCPU()


 def printCPU(self):
     print (str(self.client_id) + "  " + str(self.lok_id))


 @staticmethod
 def getClientIdfromLokId(lok_id):
     try:

        for key, value in CPU.Mapping.items():    
            print (key, value)
            if value == lok_id:
                client_id = key
                return ( client_id )
                break
     except ValueError:
         print ("Lok" + str(lok_id) +" not assigned")

 @staticmethod
 def setLokID(client_id,lok_new, lok_old):
      CPU.Mapping[client_id] = lok_new

      # Update LokList table
      Lok.bindLokID(client_id, lok_new, lok_old)


# Define Class Client
class Lok:

 LokList = {}
 count = 0
 def __init__(self,  id, addr, protocol, name, image_url ):
       #Create empty dictionary
       self.count = 0
       self.id = id
       self.name = name
       self.image_url = image_url
       self.addr = addr
       self.protocol = protocol
       self.client_id = ""
       self.status = "available"
       Lok.LokList[id] = self
       Lok.count = + 1

 @staticmethod
 def getDataJSON():
    jd = []
    i = Lok.LokList.values()
    for x in i:
         x.client_id = CPU.getClientIdfromLokId(x.id)
         jd.append(x.__dict__)
    return (json.dumps(jd))

 @staticmethod
 def bindLokID(client_id, lok_new, lok_old):

     if lok_new in Lok.LokList:
        Lok.LokList[lok_new].client_id = client_id
        Lok.LokList[lok_new].status = "blocked"

     if lok_old in Lok.LokList:
        Lok.LokList[lok_old].client_id = ""
        Lok.LokList[lok_old].status = "available"

 @staticmethod
 def printLokList():
     i = Lok.LokList.values()
     print("-------------------------")
     print("Lokliste")
     for x in i:
       #print(x)
       x.printLok()
     print("-------------------------")

 @staticmethod
 def save():
     jsonData = Lok.getDataJSON()
     f = open("./config/loklist.json", "w")  # opens file with name of "test.txt"
     f.write(jsonData)
     f.close()

 @staticmethod
 def getImage(Lok_Id):
     if Lok_Id in Lok.LokList:
        return ( Lok.LokList[Lok_Id].image_url )

 @staticmethod
 def getName(Lok_Id):
     if Lok_Id in Lok.LokList:
        return ( Lok.LokList[Lok_Id].name )

 @staticmethod
 def getAddr(Lok_Id):
     if Lok_Id in Lok.LokList:
        return ( int(Lok.LokList[Lok_Id].addr) )

 def printLok(self):
        print ("Lok:" + str(self.id) +" " + self.name + " Addr:" + str(self.addr) + " " + self.image_url + " " + self.status)


# Define Class Client
class Clients:
    mt_clients = {}
    Count = 0

    def __init__(self, user_name):
     print ("Class Constructor")

    @staticmethod
    def newClient(sid):
     Clients.Count =+ Clients.Count + 1
     print ("Client Counter")
     print (Clients.Count)
     # Add new entry
     Clients.mt_clients[Clients.Count] =  sid
     print (" Class - Clients")
     print(Clients.mt_clients)
     print("Anzahl:",len(Clients.mt_clients))

    @staticmethod
    def setUserName(client_ID, user_name):
        print ("Link Client to User")

        #if client_ID in Clients.mt_clients:
        #    Clients.mt_clients[client_ID].user_name = user_name

        print (" Class - Clients")
        print (Clients.mt_clients)
        print ("Anzahl:", len(Clients.mt_clients))

    @staticmethod
    def getClientIDfromSID(sid):

    # Printing dictionary 
        #print ("Original dictionary is : " + str(Clients.mt_clients)) 
        #print ("Dict key-value are : ") 
        for key, value in Clients.mt_clients.items():    
            print (key, value)
            if value == sid:
                return(key)
                break
    

    @staticmethod
    def deleteClient(client_ID):
     print ("Delete Client from mt_clients")

     if client_ID in Clients.mt_clients:
         del Clients.mt_clients[client_ID]

     print (" Class - Clients")
     print(Clients.mt_clients)
     print("Anzahl:",len(Clients.mt_clients))


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

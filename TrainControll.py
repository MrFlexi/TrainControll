
import json
import socket
import io

# Define Class CPU
class UDP:

    UDP_IP = "192.168.1.100"
    UDP_PORT = 15731

    @staticmethod
    def setFunction(iv_id, value):
        hex_data1 = "000b131406000038"
        hex_data2 = "010019"

        lv_idstr = chr(iv_id)
        print "SetFunction"
        print "DCC Address:" + str(iv_id) + " Value:" + str(value)

        lv_val = chr(value)

        message = hex_data1.decode("hex") + lv_idstr + lv_val + hex_data2.decode("hex")

        print "UDP IP:", UDP.UDP_IP + " Port:", UDP.UDP_PORT

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (UDP.UDP_IP, UDP.UDP_PORT))




    @staticmethod
    def setSpeed(iv_lok_id, iv_speed):
        hex_data1 = "00081314060000c0"
        hex_data2 = "0000"

        lv_addr = Lok.getAddr(iv_lok_id)
        lv_addr_str = chr(lv_addr)
        print "DCC Address:" + str(lv_addr)

        low = iv_speed & 0xff
        high = (iv_speed >> 8) & 0xff

        lv_speed_low = chr(low)
        lv_speed_high = chr(high)

        message = hex_data1.decode("hex") + lv_addr_str + lv_speed_high + lv_speed_low + hex_data2.decode(
            "hex")

        print "UDP IP:", UDP.UDP_IP + " Port:", UDP.UDP_PORT

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (UDP.UDP_IP, UDP.UDP_PORT))

    @staticmethod
    def setLokFunction(iv_lok_id, func, value):
        hex_data1 = "000c1314060000c0"
        hex_data2 = "0000"

        lv_addr = Lok.getAddr(iv_lok_id)
        lv_addr_str = chr(lv_addr)
        print "DCC Address:" + str(lv_addr)

        lv_func = chr(func)
        lv_val = chr(value)

        message = hex_data1.decode("hex") + lv_addr_str + lv_func + lv_val + hex_data2.decode("hex")

        print "UDP IP:", UDP.UDP_IP + " Port:", UDP.UDP_PORT

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (UDP.UDP_IP, UDP.UDP_PORT))

    @staticmethod
    def setDir(iv_lok_id, iv_dir):

        # Message muss immer 13 Byte lang sein.
        # hex_data1 = "000A4711050000c00301000000"
        hex_data1 = "000A4711050000c0"
        hex_data2 = "000000"

        lv_dir_str = chr(00)
        if iv_dir == "back":
            lv_dir_str = chr(02)
        elif iv_dir == "neutral":
            lv_dir_str = chr(00)
        elif iv_dir == "forward":
            lv_dir_str = chr(01)

        lv_addr = Lok.getAddr(iv_lok_id)
        lv_addr_str = chr(lv_addr)
        # lv_dir_str = chr(iv_dir)
        print "Set Direction:"

        message = hex_data1.decode("hex") + lv_addr_str + lv_dir_str + hex_data2.decode("hex")

        print "UDP IP:", UDP.UDP_IP + " Port:", UDP.UDP_PORT

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
    def save(message):
        print "Gleisplan Save"
        #jsonData = json.dumps(message, indent=1, separators=(',', ': '))
        #print(jsonData)

        for item in message:
            #print (item)
            # Get class instance
            gr_instance = Gleisplan.Liste[item["id"]]
            gr_instance.printGP()

            #Compare old and new values. if different set new value

            if gr_instance.addr <> item["addr"]:
                print "Address was changed from " +  str(gr_instance.addr) + " to: " + str(item["addr"])
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
        print "Weiche: " + str(self.id) +  " Addr: " + str(self.addr) + " Dir: " + str(self.dir)



# Define Class CPU
class CPU:

 Mapping = {}

 def __init__(self, client_id, lok_id):
    self.client_id = client_id
    self.lok_id = lok_id
    print "Mapping Constructor"
    CPU.Mapping[client_id] = lok_id
    # CPU.Mapping[client_id] = self

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
     print CPU.Mapping
     #for x in i:
     #    x.printCPU()


 def printCPU(self):
     print str(self.client_id) + "  " + str(self.lok_id)


 @staticmethod
 def getClientIdfromLokId(lok_id):
     try:
         client_id = CPU.Mapping.keys()[CPU.Mapping.values().index(lok_id)]
         return ( client_id )
     except ValueError:
         print "Lok" + str(lok_id) +" not assigned"




 @staticmethod
 def setLokID(client_id,lok_id):
      CPU.Mapping[client_id] = lok_id






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
        return ( Lok.LokList[Lok_Id].addr )

 def printLok(self):
        print "Lok:" + str(self.id) +" " + self.name + " Addr:" + str(self.addr) + " " + self.image_url


# Define Class Client
class Clients:
 def __init__(self):
       #Create emplty dictionary
       self.mt_clients = {"ID":"SID"}
       self.clientCount = 0
       print "Class Constructor"

 def newClient(self, sid):
     self.clientCount =+ self.clientCount + 1
     print "Client Counter"
     print self.clientCount
     # Add new entry
     self.mt_clients[self.clientCount] =  sid
     print " Class - Clients"
     print(self.mt_clients)
     print("Anzahl:",len(self.mt_clients))

 def getClientIDfromSID(self, sid):
     # get key from value
     return( self.mt_clients.keys()[self.mt_clients.values().index(sid)] )

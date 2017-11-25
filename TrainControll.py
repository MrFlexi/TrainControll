
import json


# Define Class CPU
class UDP:
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

        print "UDP IP:", CTRL.UDP_IP + " Port:", CTRL.UDP_PORT

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (CTRL.UDP_IP, CTRL.UDP_PORT))

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

        print "UDP IP:", CTRL.UDP_IP + " Port:", CTRL.UDP_PORT

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (CTRL.UDP_IP, CTRL.UDP_PORT))

    @staticmethod
    def setDir(iv_lok_id, iv_dir):

        # Message muss immer 13 Byte lang sein.
        # hex_data1 = "000A4711050000c00301000000"
        hex_data1 = "000A4711050000c0"
        hex_data2 = "000000"

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

        print "UDP IP:", CTRL.UDP_IP + " Port:", CTRL.UDP_PORT
        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (CTRL.UDP_IP, CTRL.UDP_PORT))


# Define Class CPU
class CPU:

 Mapping = {}

 def __init__(self, client_id, lok_id):
    self.client_id = client_id
    self.lok_id = lok_id
    print "Mapping Constructor"
    CPU.Mapping[client_id] = lok_id

 @staticmethod
 def getLokIDfromClientId(client_id):
     return(CPU.Mapping[client_id])

 @staticmethod
 def setLokID(client_id,lok_id):
      CPU.Mapping[client_id] = lok_id


# Define Class Client
class Lok:

 LokList = {}
 count = 0
 def __init__(self,  id, addr, protocol, name, image_url ):
       #Create emplty dictionary
       self.count = 0
       self.id = id
       self.name = name
       self.image_url = image_url
       self.addr = addr
       self.protocol = protocol
       Lok.LokList[id] = self
       Lok.count = + 1

 @staticmethod
 def getDataJSON():
    jd = []
    i = Lok.LokList.values()
    for x in i:
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
 def getImage(Lok_Id):
     return ( Lok.LokList[Lok_Id].image_url )

 @staticmethod
 def getName(Lok_Id):
     return ( Lok.LokList[Lok_Id].name )


 @staticmethod
 def getAddr(Lok_Id):
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
     #emit('my_response', {'data': len(self.mt_clients)})

 def getClientIDfromSID(self, sid):
     # get key from value
     return( self.mt_clients.keys()[self.mt_clients.values().index(sid)] )

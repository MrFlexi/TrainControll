
import json
import socket
import io

# Define Class CPU
class UDP:

    UDP_IP = "192.168.1.100"
    UDP_PORT = 15731

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
     f = open("loklist.json", "w")  # opens file with name of "test.txt"
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
     #emit('my_response', {'data': len(self.mt_clients)})

 def getClientIDfromSID(self, sid):
     # get key from value
     return( self.mt_clients.keys()[self.mt_clients.values().index(sid)] )

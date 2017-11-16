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
       Lok.LokList[id] =  self
       Lok.count = + 1

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

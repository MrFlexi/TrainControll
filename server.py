#!/usr/bin/env python
# testwx
import json
import base64
import socket
from collections import namedtuple
from threading import Lock
from flask import Flask, render_template, request, session
from flask_socketio import SocketIO, emit, join_room, leave_room, \
    close_room, rooms, disconnect


# Set this variable to "threading", "eventlet" or "gevent" to test the
# different async modes, or leave it set to None for the application to choose
# the best option based on installed packages.
async_mode = None

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, async_mode=async_mode)
thread = None
thread_lock = Lock()

values = {
    'slider1': 25,
    'slider2': 0,
}


# Define Class Client
class CTRL:
    List = {}
    count = 0
    UDP_IP = "192.168.1.100"
    UDP_PORT = 15731

    def __init__(self, session_id, client_id, user_name, lok_id,  lok_dir, lok_speed):

        print
        "Controller Constructor Start"
        CTRL.count = + 1
        self.session_id = session_id
        self.user_name = user_name
        self.client_id = client_id
        self.image_url = Lok.getImage(lok_id)
        self.lok_id = lok_id
        self.lok_name = Lok.getName(lok_id)
        self.lok_dir = lok_dir
        self.lok_speed = lok_speed
        self.lok_f1 = False
        CTRL.List[client_id] = self
        CTRL.printListe()

    @staticmethod
    def getDataJSON():
        jd = []
        i = CTRL.List.values()
        for x in i:
            jd.append(x.__dict__)
        return (json.dumps(jd))

    @staticmethod
    def getDataJSONforClient(client_id):
        jd = []
        x = CTRL.List[client_id]
        jd.append(x.__dict__)
        return (json.dumps(jd))

    @staticmethod
    def udp_setSpeed(iv_lok_id, iv_speed):
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
    def udp_setLokFunction(iv_lok_id, func, value):
        hex_data1 = "000c1314060000c0"
        hex_data2 = "0000"

        lv_addr = Lok.getAddr(iv_lok_id)
        lv_addr_str = chr(lv_addr)
        print "DCC Address:" + str(lv_addr)

        lv_func = chr(func)
        lv_val = chr(value)

        message = hex_data1.decode("hex") + lv_addr_str + lv_func + lv_val +  hex_data2.decode("hex")

        print "UDP IP:", CTRL.UDP_IP + " Port:", CTRL.UDP_PORT

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.sendto(message, (CTRL.UDP_IP, CTRL.UDP_PORT))


    @staticmethod
    def udp_setDir(iv_lok_id, iv_dir):

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


    @staticmethod
    def setClientData(client_id, data_in ):
        print "SetClientData " + json.dumps(data_in)

        # Data comming from client is a JSON Array. In this case it should be only one element in the array
        for item in data_in["data"]:
            print "Next Line", item
            speed = item["lok_speed"]
            direction = item["lok_dir"]
            f1 = item["lok_f1"]


        # Get class instance
        gr_instance = CTRL.List[client_id]
        gr_instance.printCTRL()
        # Compare old and new values. if different set new speed and direction


        #SPEED
        if gr_instance.lok_speed <> speed:
            print "Speed was changed", gr_instance.lok_speed, speed
            # Update speed, UDP Paket an Raspbery CS2 Emulation senden
            CTRL.udp_setSpeed(gr_instance.lok_id,speed)

            # Update values in instance
            gr_instance.lok_speed = speed

        # DIRECTION
        if gr_instance.lok_dir <> direction:
            print "Direction was changed", gr_instance.lok_dir, direction
            # Update speed, UDP Paket an Raspbery CS2 Emulation senden

            if direction == "neutral":
                CTRL.udp_setSpeed(gr_instance.lok_id, 0)
            else:
                CTRL.udp_setDir(gr_instance.lok_id,direction)
            # Update values in instance
            gr_instance.lok_dir = direction
            gr_instance.lok_speed = 0

            # DIRECTION
        if gr_instance.lok_f1 <> f1:
            print "Lok function", gr_instance.lok_f1, f1
            # Update speed, UDP Paket an Raspbery CS2 Emulation senden
            if f1: CTRL.udp_setLokFunction(gr_instance.lok_id, 0, 1)
            else: CTRL.udp_setLokFunction(gr_instance.lok_id, 0, 0)

            # Update values in instance
            gr_instance.lok_dir = direction
            gr_instance.lok_f1    = f1

    @staticmethod
    def setDataJSON( data_in ):
        print json.dumps(data_in, indent=1, separators=(',', ': '))

        # Loop at data_in into item
        for item in data_in["data"]:
            print "Next Line", item

            # Get class instance
            gr_instance = CTRL.List[ item["client_id"] ]
            gr_instance.printCTRL()

            #Compare old and new values. if different set new speed and direction
            if gr_instance.lok_speed <> item["lok_speed"]:
                print "Speed was changed", gr_instance.lok_speed, item["lok_speed"]
                # Update speed, UDP Paket senden

                # Update values in instance
                gr_instance.lok_speed = item["lok_speed"]


    @staticmethod
    def printListe():
        jd = ""
        i = CTRL.List.values()
        print("Controller Liste")
        for x in i:
            print(x)
            x.printCTRL()
        #print("Controller Liste JSON Dump")
        #print( CTRL.getDataJSON() )

    @staticmethod
    def setS(controller_id, speed):
        CTRL.List[controller_id].setSpeed(speed)

    def printCTRL(self):
        print("SessionID", self.session_id)
        print("LokID", self.lok_id)
        print("LokName", self.lok_name)
        print("Speed", self.lok_speed)

    def setSpeed(self, speed):
        self.lok_speed = speed
        print("-->SetSpeed")
        print("Name", self.lok_name)
        print("Speed", self.lok_speed)


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



# ---------------------  Main ------------------------------------

nClient = Clients()

Lok(id=1,name="S-BAHN", image_url="/static/S-Bahn.jpg", addr=5 , protocol="DCC")
Lok(id=2,name="ETA-515",image_url="/static/ETA515.jpg", addr=1, protocol="DCC" )
Lok(id=3,name="BR-218",image_url="/static/BR218.jpg", addr=3, protocol="DCC" )
Lok(id=4,name="BR-212",image_url="/static/BR212.jpg", addr=6, protocol="DCC" )
Lok(id=5,name="Portfeeder",image_url="/static/Portfeeder.jpg", addr=65, protocol="MFX" )

Lok.printLokList()

#Map client to Lok
CPU( 1, 1)
CPU( 2, 2)
CPU( 3, 3)
CPU( 4, 5)


def background_thread():
    """Example of how to send server generated events to clients."""
    count = 0
    while True:
        socketio.sleep(30)
        socketio.emit('my_response',
                      {'data': 'Server generated event', 'count': count},
                      namespace='')

@app.before_request
def before_request():
    session['key_1'] = 'Hello,'
    session['key_2'] = 'World'

@app.route('/')
def index():
    return render_template('index.html', async_mode=socketio.async_mode, **values )


@app.route('/Admin')
def UI5():
    return render_template('index_xml.html', async_mode=socketio.async_mode)




# React on slide change on clients
# broadcast slider values to all clients
@socketio.on('main_controller_value_changed', namespace='')
def main_controller_value_changed(message):
    client_id = nClient.getClientIDfromSID(request.sid)
    print "Value change of Session ID" + str(request.sid)
    print "Client" + str( client_id )
    print "Lok" + str( CPU.getLokIDfromClientId(nClient.getClientIDfromSID(request.sid)))

    # Write new data into class, handle data changes
    CTRL.setClientData(client_id, message)

    # Push new data to all connected clients
    emit('server_response', {'data': CTRL.getDataJSON()}, broadcast=True)

    # Push new data to single client
    emit('config_data', {'data': CTRL.getDataJSONforClient(client_id)})



# React on slide change on clients
# broadcast slider values to all clients
@socketio.on('value changed', namespace='')
def value_changed(message):


    print "Value change of Session ID" + str(request.sid)
    print "Value change of Client" + str( nClient.getClientIDfromSID(request.sid) )
    print ("Value change of Lok", CPU.getLokIDfromClientId(nClient.getClientIDfromSID(request.sid)))
    # Write new data into class, handle data changes
    CTRL.setDataJSON(message)
    # Push new data to all connected clients
    emit('server_response', {'data': CTRL.getDataJSON()}, broadcast=True)

@socketio.on('connect', namespace='')
def onConnect():

    print "Session ID" + str( request.sid )
    nClient.newClient(request.sid)
    print "New Client connected"
    client_id = nClient.getClientIDfromSID(request.sid)
    lok_id = CPU.getLokIDfromClientId(client_id)

    # Register new client and assign an default locomotion
    # session_id, client_id, user_name, lok_id, lok_name, lok_dir, lok_speed):
    CTRL(request.sid, client_id, "Fred", lok_id, 0, 0)

    # Push new data to all connected clients
    emit('server_response', {'data': CTRL.getDataJSON() }, broadcast=True)

    # Push new data to single client
    emit('config_data', {'data': CTRL.getDataJSONforClient(client_id)})

    print "Client JSON " +  CTRL.getDataJSONforClient(client_id)




@socketio.on('connect', namespace='ESP')
def onConnect2():

    print "Session ESP8266" + str( request.sid )
    emit('ESP', {'client_id': "Hallo"})
    emit('server_response', {'data': "1234"})

@socketio.on('esp_changed', namespace='ESP')
def value_changed2(message):
    print "ESP changed" + str(request.sid)
    print json.dumps(message, indent=1, separators=(',', ': '))

    # Push new data to all connected clients
    # emit('server_response', {'data': "ATIMR REquested"})


@socketio.on('Lok_changed', namespace='')
def value_changed(message):

    print "Lok change of Session ID" + str(request.sid)
    print "Value change of Client" + str( nClient.getClientIDfromSID(request.sid) )
    print ("Value change of Lok", CPU.getLokIDfromClientId(nClient.getClientIDfromSID(request.sid)))
    print json.dumps(message, indent=1, separators=(',', ': '))

if __name__ == '__main__':
  socketio.run(app, host='0.0.0.0', port=3000, debug=True)

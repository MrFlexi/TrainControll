#!/usr/bin/env python
# testwx
from TrainControll import CPU, Lok, Clients, UDP, Gleisplan
import json
import base64

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


    def __init__(self, session_id, client_id, user_name, lok_id,  lok_dir, lok_speed):

        print
        "Controller Constructor Start"
        CTRL.count = + 1
        self.session_id = session_id
        self.user_name = user_name
        self.client_id = client_id
        self.image_url =  Lok.getImage(lok_id)
        self.lok_id = lok_id
        self.lok_name =  Lok.getName(lok_id)
        self.lok_dir = lok_dir
        self.lok_speed = lok_speed
        self.lok_f1 = False
        CTRL.List[client_id] = self
        CTRL.printListe()

    @staticmethod
    def change_lok(client_id, data_in):
        print json.dumps(data_in, indent=1, separators=(',', ': '))

        lok_old = int(data_in["oldLok"][-1:]) + 1
        lok_new = int(data_in["newLok"][-1:]) + 1
        print "Lok change requested" + Lok.getName(lok_old) + "--->" + Lok.getName(lok_new)

        # Get class instance
        gr_instance = CTRL.List[client_id]
        gr_instance.printCTRL()

        # Compare old and new values. if different set new speed and direction
        if gr_instance.lok_id <> lok_new:
            print "Lok was changed", gr_instance.lok_id
            # Update values in instance
            gr_instance.lok_id = lok_new
            gr_instance.image_url = Lok.getImage(lok_new)
            gr_instance.lok_name = Lok.getName(lok_new)

            #Update mapping table
            CPU.printListe()
            CPU.setLokID(client_id,lok_new)
            CPU.printListe()



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
            UDP.setSpeed(gr_instance.lok_id,speed)

            # Update values in instance
            gr_instance.lok_speed = speed

        # DIRECTION
        if gr_instance.lok_dir <> direction:
            print "Direction was changed", gr_instance.lok_dir, direction
            # Update speed, UDP Paket an Raspbery CS2 Emulation senden

            if direction == "neutral":
                UDP.setSpeed(gr_instance.lok_id, 0)
            else:
                UDP.setDir(gr_instance.lok_id,direction)
            # Update values in instance
            gr_instance.lok_dir = direction
            gr_instance.lok_speed = 0

            # DIRECTION
        if gr_instance.lok_f1 <> f1:
            print "Lok function", gr_instance.lok_f1, f1
            # Update speed, UDP Paket an Raspbery CS2 Emulation senden
            if f1: UDP.setLokFunction(gr_instance.lok_id, 0, 1)
            else: UDP.setLokFunction(gr_instance.lok_id, 0, 0)

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
            x.printCTRL()
        #print("Controller Liste JSON Dump")
        #print( CTRL.getDataJSON() )

    @staticmethod
    def setS(controller_id, speed):
        CTRL.List[controller_id].setSpeed(speed)

    def printCTRL(self):
        print("SessionID", self.session_id)
        print("ClientID", self.client_id)
        print("LokID", self.lok_id)
        print("LokName", self.lok_name)
        print("Speed", self.lok_speed)

    def setSpeed(self, speed):
        self.lok_speed = speed
        print("-->SetSpeed")
        print("Name", self.lok_name)
        print("Speed", self.lok_speed)


# ---------------------  Main ------------------------------------

# Load Lok Liste
with open('./config/loklist.json') as data_file:
    loklist_json = json.load(data_file)
for item in loklist_json:
    # Create Instances for each lok
    Lok(id=item["id"],name=item["name"], image_url=item["image_url"], addr=item["addr"], protocol=item["protocol"])
Lok.printLokList()

# Load Gleisplan
with open('./config/gleisplan.json') as data_file:
    gleisplan_json = json.load(data_file)
for item in gleisplan_json:
    # Create Instances for each lok
    Gleisplan( id=item["id"], addr=item["addr"],x1=item["x1"],x2=item["x2"],y1=item["y1"],y2=item["y2"],dir=item["dir"], type="DCC" )


#Map client to Lok
CPU( 1, 1)
CPU( 2, 2)
CPU( 3, 3)
CPU( 4, 5)

print "Initial Client Lok Mapping"
CPU.printListe()

print "Initial Browser Clients"
Clients()



# ---------------------  ROUTING ------------------------------------

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

@app.route('/Track')
def track():
    return render_template('track.html', async_mode=socketio.async_mode)


@app.route('/TrackShow')
def track_show():
    return render_template('trackshow.html', async_mode=socketio.async_mode)

@app.route('/TrackCreate')
def track_create():
    return render_template('trackcreate.html', async_mode=socketio.async_mode)



# ---------------------  SocketIO Event Handling ------------------------------------


# React on slide change on clients
# broadcast slider values to all clients
@socketio.on('main_controller_value_changed', namespace='')
def main_controller_value_changed(message):
    client_id = Clients.getClientIDfromSID(request.sid)
    print "Value change of Session ID" + str(request.sid)
    print "Client" + str( client_id )
    print "Lok" + str( CPU.getLokIDfromClientId(Clients.getClientIDfromSID(request.sid)))

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
    Clients.newClient(request.sid)

    print "New Client connected"
    client_id = Clients.getClientIDfromSID(request.sid)
    lok_id = CPU.getLokIDfromClientId(client_id)

    # Register new client and assign an default locomotion
    # session_id, client_id, user_name, lok_id, lok_name, lok_dir, lok_speed):
    CTRL(request.sid, client_id, "Fred", lok_id, 0, 0)

    # Push new data to all connected clients
    emit('server_response', {'data': CTRL.getDataJSON() }, broadcast=True)

    # Push new data to single client
    emit('config_data', {'data': CTRL.getDataJSONforClient(client_id),
                         'Gleisplan': Gleisplan.getDataJSON()})

    emit('loklist_data', { 'LokList': Lok.getDataJSON()} )

    print "Client JSON " +  CTRL.getDataJSONforClient(client_id)



@socketio.on('disconnect', namespace='')
def onDiscconnect():
    print "Session ID" + str( request.sid )
    client_id = nClient.getClientIDfromSID(request.sid)
    print "Client disconnected: " + str( client_id )



@socketio.on('Lok_changed', namespace='')
def value_changed(message):

    client_id = Clients.getClientIDfromSID(request.sid)

    print "Lok change of Session ID" + str(request.sid)
    print "Value change of Client" + str( client_id )
    CTRL.change_lok(client_id=client_id, data_in=message )

    # Push new data to all connected clients
    emit('server_response', {'data': CTRL.getDataJSON()}, broadcast=True)

    # Push new data to all connected clients
    emit('loklist_data', {'LokList': Lok.getDataJSON()}, broadcast=True)



@socketio.on('toggle_turnout', namespace='')
def toggle_turnout(message):
    Gleisplan.toggle_turnout(message);
    emit('gleisplan_data', {'Gleisplan': Gleisplan.getDataJSON()}, broadcast=True)

@socketio.on('gleisplan_save', namespace='')
def gleisplan_save(message):

    Gleisplan.save(message)
    # Push new data to all connected clients
    emit('gleisplan_data', {'Gleisplan': Gleisplan.getDataJSON()}, broadcast=True)


@socketio.on('Weiche_neu', namespace='')
def weiche_neu(message):
    Gleisplan.new(message)
    # Push new data to all connected clients
    emit('gleisplan_data', {'Gleisplan': Gleisplan.getDataJSON()}, broadcast=True)


if __name__ == '__main__':
  socketio.run(app, host='0.0.0.0', port=3000, debug=True)

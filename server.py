#!/usr/bin/env python
# testwx

# build requirements file:          pip freeze > requirements.txt  
# install dependencies:             pip install -r requirements.txt 

# Linux
# activate Virtual Environment:     source ./venv/bin/activate   oder   deactivate

#  windows install                   py -m pip install --user virtualenv
#          create new                py -m venv venv 
#           activate                 
#                                   ./venv/Scripts/activate
import sys
import math
import time
import datetime
import ntplib
import logging
import socket
import struct
import json
import base64
import binascii
import paho.mqtt.client as mqtt

from TrainControll import Clients, UDP, Gleisplan, User, lcd, log4j
from TrainControllLok import Lok
from TrainControllCtrl import CTRL
from collections import namedtuple
from threading import Lock
from flask import Flask, render_template, request, session
from flask_cors import CORS
from flask_apscheduler import APScheduler
from flask_socketio import SocketIO, emit, join_room, leave_room, close_room, rooms, disconnect
from pathlib import Path
from time import ctime

# Set this variable to "threading", "eventlet" or "gevent" to test the
    # different async modes, or leave it set to None for the application to choose
    # the best option based on installed packages.
async_mode = None
app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'

CORS(app)
socketio = SocketIO(app, async_mode=async_mode, cors_allowed_origins="*")

def scheduleTask():
    if sys.platform.startswith('linux'):
        print()
        #page_0()

# ---------------------  SETUP  ------------------------------------
log4j()
log4j.write(('Setup....'))

filename = Path('config/loklist.json')
if not filename.exists():
    print("Oops, file doesn't exist!")
    logging.error('file doesnt exist!')

data_file = open(filename)
loklist_json = json.load(data_file)

for item in loklist_json:
    # Create Instances for each lok
    Lok(id=item["id"],name=item["name"], image_url=item["image_url"], addr=item["addr"], protocol=item["protocol"])
Lok.printLokList()

# Load Gleisplan
with open('config/gleisplan.json') as data_file:
    gleisplan_json = json.load(data_file)
    for item in gleisplan_json['switches']:
        # Create Instances for each switch
        Gleisplan( id=item["id"], addr=item["addr"],x=item["x"],y=item["y"],dir=item["dir"], type="DCC",aus=item["aus"], element='switch' )

Gleisplan.printGleisplan()

# Load User
with open('config/userlist.json') as data_file:
    userlist_json = json.load(data_file)
for item in userlist_json:
    # Create Instances for each lok
    User( user_id=item["user_id"], user_name=item["user_name"], image_url=item["image_url"], client_id=item["client_id"] )

User.printUserList()
logging.info('End Main....')

def ack():
    print("message was received")

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

@app.route('/Old')
def index():
    return render_template('index.html', async_mode=socketio.async_mode, **values )
    
@app.route('/Upload')
def Upload():
    return render_template('Upload.html', async_mode=socketio.async_mode)

@app.route('/')
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

# ---------------------  HTTP API ------------------------------------
# React on Locomotion changes
# --------------------------------------------------------------------
@app.route('/LokChanged', methods=['POST'])
def LokChanged():
    print()
    print("------------------------------------------------------------------")   
    log4j.write("HTTP API")  
    payload = request.get_json()
    print(payload) 
    if payload:
        data = payload
        id = int(data["id"])
        speed = int(data["speed"])       
        print("Value change of Lok ID ", id)
        # Write new data into class, handle data changes
        Lok.setNewData(data)
        emit('LokList_data', {'LokList': Lok.getDataJSON()}, broadcast=True)  # List of available locomotions
        return "[SUCCESS] Message send", 200
    else:
        return "[FAILED] no Json payload found", 204

# ---------------------  SocketIO Event Handling ------------------------------------
# React on slide change on clients
# broadcast slider values to all clients
@socketio.on('main_controller_value_changed', namespace='')
def main_controller_value_changed(message):
    print()
    print("------------------------------------------------------------------")   
    data = message["data"]
    id = int(data["id"])
    speed = int(data["speed"])
    log4j.write("Socket IO")   
    print ("Value change of Lok ID ", id)     
    # Write new data into class, handle data changes
    Lok.setNewData(message["data"])

    # Push new data to single client
    emit('config_data', {'MyLok': Lok.getDataJSONforID(id),
                        'user': User.getDataJSON()})

    #emit('config_data', {'MyLok': Lok.getDataJSONforClient(request.sid),
    #                    'user': User.getDataJSON()})
    emit('LokList_data', {'LokList': Lok.getDataJSON()}, broadcast=True)  # List of available locomotions

@socketio.on('LokListDataChanged', namespace='')
def LokListDataChanged(message):
    client_id = request.sid
    print ("Value change of Session ID" + str(request.sid))
    print ("Client" + str( client_id ))
    for a in message:
        Lok.setNewData(a) 
    emit('LokList_data', {'LokList': Lok.getDataJSON()}, broadcast=True)  # List of available locomotions

@socketio.on('connect', namespace='')
def onConnect():
    print()
    print("------------------------------------------------------------------")
    print ("New Client connected :Session ID: " + str( request.sid ))
    Clients.newClient(request.sid)
    #print(Lok.getDataJSON()) 
    # Push data to all connected clients 
    time.sleep(2)  #braucht man da Fiori langsam ist und sonst das event verschluckt....
    emit('initialisation', {'data': "",
                         'UserList': User.getDataJSON(),
                         'LokList': Lok.getDataJSON(),
                         'TrackList': Gleisplan.getDataJSON(),
                         }, callback=ack)

@socketio.on('disconnect', namespace='')
def onDiscconnect():
    print ("Client disconnected: " + str( request.sid ))
    Clients.deleteClient(request.sid)    

# React on User has logged on
@socketio.on('User_changed', namespace='')
def User_changed(message):
    client_id = Clients.getClientIDfromSID(request.sid)
    user_name = message["user_name"]
    print ("User Changed on Client " + str(client_id) + " to " + user_name)
    CTRL.setUserName(client_id, user_name)
    # Push new data to single client
    emit('config_data', {'MyLok': Lok.getDataJSONforClient(request.sid),
                        'LokList': Lok.getDataJSON(),
                         'UserList': User.getDataJSON()
                             }, broadcast=True )

   # React on locomotion chaged in Picture Carousel
@socketio.on('Lok_changed', namespace='')
def value_changed(message):
    print()
    print("------------------------------------------------------------------")
    print ("Lok change of Session ID" + str(request.sid))
    CTRL.change_lok(client_id=request.sid, data_in=message )

    # Push new data to single client
    emit('config_data', {'MyLok': Lok.getDataJSONforClient(request.sid),
                         'user': User.getDataJSON()
                        })
    # Push new data to all connected clients
    emit('LokList_data', {'LokList': Lok.getDataJSON()}, broadcast=True)  # List of available locomotions

@socketio.on('toggle_turnout', namespace='')
def toggle_turnout(message):
    Gleisplan.toggle_turnout(message);
    emit('gleisplan_data', {'Track': Gleisplan.getDataJSON()}, broadcast=True)

@socketio.on('track_changed', namespace='')
def track_changed(message):
    log4j.write("Weiche: " + str(message["id"]) + " " + str(message["dir"])) 
    Gleisplan.set_turnout(message["id"], message["dir"]);
    emit('gleisplan_data', {'Track': Gleisplan.getDataJSON()}, broadcast=True)    

@socketio.on('gleisplan_save', namespace='')
def gleisplan_save(message):
    Gleisplan.save(message)
    # Push new data to all connected clients
    emit('gleisplan_data', {'Track': Gleisplan.getDataJSON()}, broadcast=True)

@socketio.on('Weiche_neu', namespace='')
def weiche_neu(message):
    Gleisplan.new(message)
    # Push new data to all connected clients
    emit('gleisplan_data', {'Track': Gleisplan.getDataJSON()}, broadcast=True)

@socketio.on('onFabricSave', namespace='')
def fabric_save(message):    
    Gleisplan.fabric_save(message)

@socketio.on('onFabricLoad', namespace='')
def fabric_load():    
    json = Gleisplan.fabric_load()
    emit('fabric_data', json, broadcast=True)

#-----------------------------------------------------------------------------------------------
#   MQTT
#-----------------------------------------------------------------------------------------------
# The callback for when the client receives a CONNACK response from the server.
def mqtt_on_connect(client, userdata, flags, rc):
    log4j.write("MQTT connected "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(mqtt_topic)

def mqtt_on_disconnect(client, userdata,rc=0):
    logging.debug("Mqtt disconnected "+str(rc))
    client.loop_stop

# The callback for when a PUBLISH message is received from the server.
def mqtt_on_message(client, userdata, msg):
    print()
    print("------------------------------------------------------------------")
    log4j.write("Mqtt in")
    print("Topic:" + msg.topic)
    print("Payload:" + str(msg.payload))
    m_decode=str(msg.payload.decode("utf-8","ignore"))
    message=json.loads(m_decode) #decode json data
    command=message["command"]
    print("processing command:",command)
        
    if command== "Lok":
        Lok.setNewData(message)
        socketio.emit('LokList_data', {'LokList': Lok.getDataJSON()}, broadcast=True)  # List of available locomotions

    if command== "Switch":
        Gleisplan.set_turnout(message["id"], message["dir"]);
        socketio.emit('gleisplan_data', {'Track': Gleisplan.getDataJSON()}, broadcast=True)    

#-----------------------------------------------------------------------------------------------
#   MAIN
#-----------------------------------------------------------------------------------------------
if __name__ == '__main__':
    logging.info('__main__')
    #Tasks
    scheduler = APScheduler()
    thread = None
    thread_lock = Lock()
    
    if sys.platform.startswith('linux'):  
        log4j.write("waiting for network..")   
        lcd.show_logo()   
        time.sleep(5)

    log4j.write('Starting MQTT')
    mqtt_topic = "TrainControll/toGleisbox/"
    client = mqtt.Client()
    client.on_connect = mqtt_on_connect
    client.on_disconnect = mqtt_on_disconnect
    client.on_message = mqtt_on_message
    client.connect("85.209.49.65", 1883, 60)
    client.loop_start()    
    
    log4j.write('Scheduling Tasks')
    scheduler.add_job(id = 'Scheduled Task', 
                        func=scheduleTask, 
                        trigger="interval", seconds=30
                        )
    scheduler.start()
    log4j.clear()
    log4j.write('Ready...')

    debug = True
    if sys.platform.startswith('linux'):
         debug = False
    
    socketio.run(app, host='0.0.0.0', port=3033, debug=debug)
    
#!/usr/bin/env python
# testwx

# build requirements file:          pip freeze > requirements.txt  
# install dependencies:             pip install -r requirements.txt 

# Linux
# activate Virtual Environment:     source ./venv/bin/activate   oder   deactivate

#  windows install                   py -m pip install --user virtualenv
#          create new                py -m venv venv 
#           activate                 ./venv/Scripts/activate

import sys
import math
import time
import datetime
import ntplib
import logging
import socket
#import fcntl
import struct


from TrainControll import Lok, Clients, UDP, Gleisplan, User
import json
import base64

from collections import namedtuple
from threading import Lock
from flask import Flask, render_template, request, session
from flask_apscheduler import APScheduler
from flask_socketio import SocketIO, emit, join_room, leave_room, close_room, rooms, disconnect
from pathlib import Path
import binascii
from time import ctime

#def get_interface_ipaddress(ifname):
#    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#    return socket.inet_ntoa(fcntl.ioctl(
#      s.fileno(),
#      0x8915,  # SIOCGIFADDR
#      struct.pack('256s', bytes(ifname[:15], 'utf-8')))[20:24])


#def get_ip():
#    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#    info = fcntl.ioctl(s.fileno(), 0x8927,  struct.pack('256s', bytes(ifname[:15], 'utf-8')))
#    return ''.join(['%02x:' % b for b in info[18:24]])[:-1]


if sys.platform.startswith('linux'):
    # Linux-specific code here...

    from PIL import Image

    #from .lib_oled96 import ssd1306
    from smbus import SMBus
    from lib_oled96.lib_oled96 import ssd1306

    #LUMA
    from luma.core.render import canvas
    from luma.core.interface.serial import spi
    from luma.core.render import canvas
    from luma.oled.device import sh1106

    serial = spi(port=0, device=1, gpio_DC=27, gpio_RST=26,  gpio_CS=18)
    device = sh1106(serial, rotate=0)
    today_last_time = "unknown"


    def init_spi_display():
        with canvas(device) as draw:
            draw.rectangle(device.bounding_box, outline="white", fill="black")
            
    
    def init_display():
        # Display einrichten
        i2cbus = SMBus(1)            # 0 = Raspberry Pi 1, 1 = Raspberry Pi > 1
        oled = ssd1306(i2cbus)

        # Ein paar Abkrzungen, um den Code zu entschlacken
        draw = oled.canvas

        # Display zum Start loeschen
        oled.cls()
        oled.display()

        # Hallo Welt
        draw.text((1, 1), "TrainControll", fill=1)
        draw.text((1, 10), "01-01-2021 ?", fill=1)

        # Ausgaben auf Display schreiben
        oled.display()

    def posn(angle, arm_length):
        dx = int(math.cos(math.radians(angle)) * arm_length)
        dy = int(math.sin(math.radians(angle)) * arm_length)
        return (dx, dy)


    def page_logo():
        img_path = str(Path(__file__).resolve().parent.joinpath('images', 'pi_logo.png'))
        logo = Image.open(img_path).convert("RGBA")
        posn = ((device.width - logo.width) // 2, 0)
        background.paste(logo, posn)
        device.display(background.convert(device.mode))
     

    background = Image.new("RGBA", device.size, "white")
    def page_0():
        #ip_s = get_interface_ipaddress('wlan0')
        #logging.info('IP:%s',ip_s)
        #n = Clients.getClientsCount()

        with canvas(device) as draw:
            draw.text((1, 1), "  TrainControll 2021  ", fill=1)
            
            s = f"Clients:  {n}"
            draw.text((1,40), s, fill = 1)
            #ip_s = ip_s +":3033"
            #draw.text((1, 50), ip_s, fill=1)



    def show_clock():
        logging.info('Show clock....')
        now = datetime.datetime.now()
        today_date = now.strftime("%d %b %y")
        today_time = now.strftime("%H:%M:%S")
        with canvas(device) as draw:
            now = datetime.datetime.now()
            today_date = now.strftime("%d %b %y")
            margin = 4
            cx = 30
            cy = min(device.height, 64) / 2
            left = cx - cy
            right = cx + cy
            hrs_angle = 270 + (30 * (now.hour + (now.minute / 60.0)))
            hrs = posn(hrs_angle, cy - margin - 7)
            min_angle = 270 + (6 * now.minute)
            mins = posn(min_angle, cy - margin - 2)
            sec_angle = 270 + (6 * now.second)
            secs = posn(sec_angle, cy - margin - 2)
            draw.ellipse((left + margin, margin, right - margin, min(device.height, 64) - margin), outline="white")
            draw.line((cx, cy, cx + hrs[0], cy + hrs[1]), fill="white")
            draw.line((cx, cy, cx + mins[0], cy + mins[1]), fill="white")
            draw.line((cx, cy, cx + secs[0], cy + secs[1]), fill="red")
            draw.ellipse((cx - 2, cy - 2, cx + 2, cy + 2), fill="white", outline="white")
            draw.text((2 * (cx + margin), cy - 8), today_date, fill="yellow")
            draw.text((2 * (cx + margin), cy), today_time, fill="yellow")  
            draw.text((64, 10), "TrainControll", fill="white")    
                
                

def scheduleTask():
    print("")
    #page_0()

# Set this variable to "threading", "eventlet" or "gevent" to test the
# different async modes, or leave it set to None for the application to choose
# the best option based on installed packages.
async_mode = None

app = Flask(__name__)
#Tasks
logging.info('scheduling tasks....')
scheduler = APScheduler()

app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, async_mode=async_mode)
thread = None
thread_lock = Lock()

# Define Class Client
class CTRL:

    @staticmethod
    def change_lok(client_id, data_in):
        print (json.dumps(data_in, indent=1, separators=(',', ': ')))

        print ("Changed by" , data_in["who"])

        if data_in["who"] == "Carousel":
            lok_new = int(data_in["newLok"][-1:]) + 1

        if data_in["who"] == "Dialog":
            lok_new = int(data_in["newLok"][-1:])

        print ("Lok change requested. New Locomotion: " , Lok.getName(lok_new))

    

        # Compare old and new values. if different set new speed and direction
        if lok_old != lok_new:
            lok_old = gr_instance.lok_id
            print ("Lok was changed", lok_old , " to " , lok_new)
            # Update values in instance
            gr_instance.lok_id = lok_new
            gr_instance.image_url = Lok.getImage(lok_new)
            gr_instance.lok_name = Lok.getName(lok_new)

            #Update mapping table
            #CPU.printListe()
            #CPU.setLokID(client_id,lok_new, lok_old, CTRL.getUserName(client_id))
            #CPU.printListe()

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
        print ("SetClientData " + json.dumps(data_in))

        # Data comming from client is a JSON Array. In this case it should be only one element in the array
        for item in data_in["data"]:
            #print ("Next Line", item)
            speed = item["lok_speed"]
            direction = item["lok_dir"]
            f1 = item["lok_f1"]


        # Get class instance
        gr_instance = CTRL.List[client_id]
        #gr_instance.printCTRL()
        # Compare old and new values. if different set new speed and direction


        #SPEED
        if gr_instance.lok_speed != speed:
            print ("Speed was changed", gr_instance.lok_speed, speed)
            # Update speed, UDP Paket an Raspbery CS2 Emulation senden
            UDP.setSpeed(gr_instance.lok_id,speed)

            # Update values in instance
            gr_instance.lok_speed = speed

        # DIRECTION
        if gr_instance.lok_dir != direction:
            print ("Direction was changed", gr_instance.lok_dir, direction)
            # Update speed, UDP Paket an Raspbery CS2 Emulation senden

            if direction == "0":
                UDP.setSpeed(gr_instance.lok_id, 0)
            else:
                UDP.setDir(gr_instance.lok_id,direction)
            # Update values in instance
            gr_instance.lok_dir = direction
            gr_instance.lok_speed = 0

            # function F1..F10
        if gr_instance.lok_f1 != f1:
            print ("Lok function", gr_instance.lok_f1, f1)
            # Update speed, UDP Paket an Raspbery CS2 Emulation senden
            if f1: UDP.setLokFunction(gr_instance.lok_id, 0, 1)
            else: UDP.setLokFunction(gr_instance.lok_id, 0, 0)

            # Update values in instance
            gr_instance.lok_dir = direction
            gr_instance.lok_f1    = f1

    @staticmethod
    def setDataJSON( data_in ):
        print (json.dumps(data_in, indent=1, separators=(',', ': ')))

        # Loop at data_in into item
        for item in data_in["data"]:
            #print ("Next Line", item)

            # Get class instance
            gr_instance = CTRL.List[ item["client_id"] ]
            #gr_instance.printCTRL()

            #Compare old and new values. if different set new speed and direction
            if gr_instance.lok_speed != item["lok_speed"]:
                print ("Speed was changed", gr_instance.lok_speed, item["lok_speed"])
                # Update speed, UDP Paket senden

                # Update values in instance
                gr_instance.lok_speed = item["lok_speed"]
            
            if gr_instance.lok_dir != item["lok_dir"]:
                print ("Directio was changed", gr_instance.lok_dir, item["lok_dir"])
                # Update speed, UDP Paket senden

                # Update values in instance
                gr_instance.lok_dir = item["lok_dir"]


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



# ---------------------  SETUP  ------------------------------------
logging.basicConfig(filename='myapp.log', level=logging.INFO, format='%(asctime)s %(message)s')
logging.info('Starting Main....')

if sys.platform.startswith('linux'):
    #init_display()     #I2C
    init_spi_display()  #SPI
    page_logo()


filename = Path('config/loklist.json')
if not filename.exists():
    print("Oops, file doesn't exist!")

data_file = open(filename)
loklist_json = json.load(data_file)

for item in loklist_json:
    # Create Instances for each lok
    Lok(id=item["id"],name=item["name"], image_url=item["image_url"], addr=item["addr"], protocol=item["protocol"])
Lok.printLokList()

# Load Gleisplan
with open('config/gleisplan.json') as data_file:
    gleisplan_json = json.load(data_file)
for item in gleisplan_json:
    # Create Instances for each lok
    Gleisplan( id=item["id"], addr=item["addr"],x1=item["x1"],x2=item["x2"],y1=item["y1"],y2=item["y2"],dir=item["dir"], type="DCC" )


# Load User
with open('config/userlist.json') as data_file:
    userlist_json = json.load(data_file)
for item in userlist_json:
    # Create Instances for each lok
    User( user_id=item["user_id"], user_name=item["user_name"], image_url=item["image_url"], client_id=item["client_id"] )

User.printUserList()

#Map client to Lok
#CPU( 1, 1)
#CPU( 2, 2)
#CPU( 3, 3)
#CPU( 4, 5)

print ("Initial Client Lok Mapping")
#CPU.printListe()


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



# ---------------------  SocketIO Event Handling ------------------------------------
# React on slide change on clients
# broadcast slider values to all clients
@socketio.on('main_controller_value_changed', namespace='')
def main_controller_value_changed(message):
    client_id = request.sid
    print ("Value change of Session ID" + str(request.sid))
    print ("Client" + str( client_id ))


    # Write new data into class, handle data changes
    CTRL.setClientData(client_id, message)

    # Push new data to all connected clients
    emit('server_response', {'data': CTRL.getDataJSON()}, broadcast=True)

    # Push new data to single client
    emit('config_data', {'data': CTRL.getDataJSONforClient(client_id),
                         'user': User.getDataJSON()
                             })



# React on slide change on clients
# broadcast slider values to all clients
@socketio.on('value changed', namespace='')
def value_changed(message):
    print ("Value change of Session ID" + str(request.sid))
    print ("Value change of Client" + str( Clients.getClientIDfromSID(request.sid) ))
  
    # Write new data into class, handle data changes
    CTRL.setDataJSON(message)
    # Push new data to all connected clients
    emit('server_response', {'data': CTRL.getDataJSON()}, broadcast=True)

@socketio.on('connect', namespace='')
def onConnect():
    print ("New Client connected :Session ID: " + str( request.sid ))
    Clients.newClient(request.sid)

    # Push data to all connected clients
    emit('initialisation', {'data': "",
                         'user': User.getDataJSON(),
                         'LokList': Lok.getDataJSON(),
                         'Gleisplan': Gleisplan.getDataJSON()})

    # Push data to all connected clients
    # emit('server_response', {'data': CTRL.getDataJSON()}, broadcast=True)



@socketio.on('disconnect', namespace='')
def onDiscconnect():
    print ("Client disconnected: " + str( request.sid ))
    
    # Push new data to all connected clients
    emit('server_response', {'data': CTRL.getDataJSON()}, broadcast=True)



# React on User has logged on
@socketio.on('User_changed', namespace='')
def User_changed(message):
    client_id = Clients.getClientIDfromSID(request.sid)
    user_name = message["user_name"]

    print ("User Changed on Client " + str(client_id) + " to " + user_name)

    CTRL.setUserName(client_id, user_name)

    # Push new data to single client
    emit('config_data', {'data': CTRL.getDataJSONforClient(client_id),
                         'user': User.getDataJSON()
                             })


   # React on locomotion chaged in Picture Carousel
@socketio.on('Lok_changed', namespace='')
def value_changed(message):

    client_id = request.sid

    print ("Lok change of Session ID" + str(request.sid))
    CTRL.change_lok(client_id=client_id, data_in=message )

    # Push new data to single client
    emit('config_data', {'data': CTRL.getDataJSONforClient(client_id),
                         'user': User.getDataJSON()
                             })


    # Push new data to all connected clients
    emit('server_response', {'data': CTRL.getDataJSON()}, broadcast=True)
    emit('loklist_data', {'LokList': Lok.getDataJSON()}, broadcast=True)  # List of available locomotions


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
    logging.info('__main__')
    scheduler.add_job(id = 'Scheduled Task', func=scheduleTask, trigger="interval", seconds=5)
    scheduler.start()
    socketio.run(app, host='0.0.0.0', port=3033, debug=True)
    #socketio.run(app)

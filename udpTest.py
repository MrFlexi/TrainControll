import socket
import sys

UDP_IP = "0.0.0.0"  #listening to all IP addresses
UDP_Destination_port = 15730;  #  CS2 Script --> Python
UDP_cs2_port = 15731;        #  Python --> CS2 Script

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((UDP_IP, UDP_Destination_port))


try:
    while True:
       data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
       print("received message: %s" % data)
       
except KeyboardInterrupt:
    pass
    print('Finally block is executed')
    sock.close( )


       




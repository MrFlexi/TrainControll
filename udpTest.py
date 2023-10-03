import socket

UDP_IP = "127.0.0.1"
UDP_PORT = 5005 
UDP_Destination_port = 15730;
UDP_Local_port = 15731;

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_Destination_port))
    
while True:
       data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
       print("received message: %s" % data)
#
# Copyright 2016, Decision Developments
#
# PI-CAN-S88 CS2 Controller
#


PiCanS88:
	g++ -o PiCanS88 PiCanS88.c CAN2UDP.cpp CAN2LAN.cpp LinuxConApp.cpp S88UDP.cpp -l bcm2835 -lpthread -lz

install: PiCanS88
	install PiCanS88 /usr/local/bin

clean:
	rm -f *.o PiCanS88


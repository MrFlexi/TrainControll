// S88UDP.cpp: Implementierung der Klasse CS88UDP.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <linux/can.h>
#include <bcm2835.h>

#include "S88UDP.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CS88UDP::CS88UDP()
{
	m_iVerbose = 0;
	m_iBackground = 1;
	m_iSensorOffset = 0;
	m_iModulCount = 1;
	m_iOn = 1;
	strcpy(m_Destip, "127.0.0.1");
	m_iUDP_Destination_port = 15730;

	Init();
}

CS88UDP::CS88UDP(CLinuxConApp App)
{
	m_iVerbose = 0;
	m_iBackground = 1;
	m_iSensorOffset = 0;
	m_iModulCount = 1;
	m_iOn = 1;
	strcpy(m_Destip, "127.0.0.1");
	m_iUDP_Destination_port = 15730;

	m_iVerbose = App.m_iOpt_Verbose;
	m_iBackground = App.m_iOpt_Background;
  	m_iUDP_Destination_port = App.m_iUDP_Destination_port;
	m_iModulCount = App.m_iOpt_ModulCount;
	m_iSensorOffset = App.m_iSensorOffset;
	strcpy(m_Destip, App.m_Destip);
	memcpy(&m_Destaddr, &App.m_Destaddr, sizeof(App.m_Destaddr));

	Init();
}

void CS88UDP::Init()
{
	/* setup udp socket */
/*	bzero(&m_Destaddr,sizeof(m_Destaddr));
	m_Destaddr.sin_family = AF_INET;
	m_Destaddr.sin_port = htons(m_iUDP_Destination_port);
	if (inet_pton(AF_INET, m_Destip, &m_Destaddr.sin_addr) < 0) 
	{
		perror("inet_pton");
		exit(1);
	}
	*/

	/* setup gpio handling */
	bcm2835_set_debug ( 0 );
	if ( !bcm2835_init() ) 
	{
		fprintf ( stderr, "GPIO Init failed!\n" );
		exit(1);
	}
	bcm2835_gpio_fsel(PI2_S88_CLOCK_PIN, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PI2_S88_LOAD_PIN, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PI2_S88_RESET_PIN, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PI2_S88_DATA_PIN, BCM2835_GPIO_FSEL_INPT);
	
	memset(m_iarSensors, 0, sizeof(m_iarSensors));
	/* Preset sensor values */
/*	for (int i=0; i < (m_iModulCount * 16); i++) 
	{
		m_iarSensors[i] = 0;
	}*/
	
	/* open udp socket */
	if ((m_iUdpsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		perror("Open UDP socket");
		exit(1);
	}
	
	if (setsockopt(m_iUdpsock, SOL_SOCKET, SO_BROADCAST, &m_iOn, sizeof(m_iOn)) < 0)
	{
		perror("setsockopt");
		exit(1);
	}
}

CS88UDP::~CS88UDP()
{
	
}

void CS88UDP::Send_Sensor_Event(int sock, const struct sockaddr *destaddr, int verbose, int offset, int address, int value)
{
	unsigned char udpframe [32];
	unsigned long can_id;
	int nbytes,i;
	
	can_id = 0x80220B01 + offset;
	
	bzero(udpframe, 13);
	udpframe[0] = (can_id >> 24) & 0x000000FF;
	udpframe[1] = (can_id >> 16) & 0x000000FF;
	udpframe[2] = (can_id >>  8) & 0x000000FF;
	udpframe[3] = can_id & 0x000000FF;
	udpframe[4] = 8;
	
	udpframe[5] = 0;
	udpframe[6] = 0;
	udpframe[7] = ((16 * offset + address) >> 8) & 0x000000FF;
	udpframe[8] = (16 * offset + address) & 0x000000FF;
	
	udpframe[9] = !value;
	udpframe[10] = value;
	
	udpframe[11] = 0;
	udpframe[12] = 0;
	
	if ( verbose ) 
	{
		printf("->S88>UDP CANID 0x%06X R",can_id);
		printf(" [%d]", udpframe[4]);
		for (i=5; i < 13; i++) 
		{
			printf(" %02x", udpframe[i]);
		}
		printf("\n");
	}
	if ( sendto(sock, udpframe, 13, 0, destaddr, sizeof(*destaddr)) != 13 ) 
	{
		perror("UDP write __");
		exit ( 1 );
	}
}

void CS88UDP::HandleS88Events()
{
	int i,j;
	int oldvalue, newvalue;
	
	bcm2835_gpio_write(PI2_S88_LOAD_PIN, HIGH);
	bcm2835_delayMicroseconds(MICRODELAY);
	bcm2835_gpio_write(PI2_S88_CLOCK_PIN, HIGH);
	bcm2835_delayMicroseconds(MICRODELAY);
	bcm2835_gpio_write(PI2_S88_CLOCK_PIN, LOW);
	bcm2835_delayMicroseconds(MICRODELAY);
	bcm2835_gpio_write(PI2_S88_RESET_PIN, HIGH);
	bcm2835_delayMicroseconds(MICRODELAY);
	bcm2835_gpio_write(PI2_S88_RESET_PIN, LOW);
	bcm2835_delayMicroseconds(MICRODELAY);
	bcm2835_gpio_write(PI2_S88_LOAD_PIN, LOW);
	
	/* get sensor data */
	for (i=0; i< m_iModulCount; i++) 
	{
		for ( j=0; j < 16; j++) 
		{
			bcm2835_delayMicroseconds(MICRODELAY / 2);
			
			oldvalue = m_iarSensors[i*16+j];
			newvalue = bcm2835_gpio_lev(PI2_S88_DATA_PIN); 
	//		if ( !m_iBackground && m_iVerbose && m_iModulCount == 1) 
	//			printf ( "%02x ", m_iarSensors[i*16+j] );
			
			if ( newvalue != oldvalue ) 
			{
				if ( m_iVerbose && m_iModulCount > 0) 
					printf ( "sensor %d changed value to %d\n", i*16+j+1, newvalue);
				
				Send_Sensor_Event( m_iUdpsock, (struct sockaddr *)&m_Destaddr, m_iVerbose, m_iSensorOffset, i*16+j+1, newvalue );
				
				m_iarSensors[i*16+j] = newvalue;
			}
			
			bcm2835_delayMicroseconds(MICRODELAY / 2);
			bcm2835_gpio_write(PI2_S88_CLOCK_PIN, HIGH);
			bcm2835_delayMicroseconds(MICRODELAY);
			bcm2835_gpio_write(PI2_S88_CLOCK_PIN, LOW);
		}
	}
//	if (!m_iBackground && m_iVerbose && m_iModulCount == 1) 
//		printf ("\r"); 
//	fflush(stdout);
	bcm2835_delayMicroseconds((MAXMODULES - m_iModulCount + 1) * 16 * MICRODELAY);
}

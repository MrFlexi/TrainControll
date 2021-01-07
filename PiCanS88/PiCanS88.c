
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>

#include <linux/can.h>
#include <bcm2835.h>
#include "PiCanS88.h"
#include "LinuxConApp.h"
#include "CAN2UDP.h"
#include "CAN2LAN.h"
#include "S88UDP.h"

void* S88Thread(void* p_oS88UDP)
{
	while(1)
		((CS88UDP*)p_oS88UDP)->HandleS88Events();
}

void* CANThread(void* p_cCAN2UDP)
{
	while(1)
		((CCAN2LAN*)p_cCAN2UDP)->HandleEvents();
}

void* PingThread(void* p_cCAN2UDP)
{
	while(1)
	{
		((CCAN2LAN*)p_cCAN2UDP)->send_ping_frame();
		sleep(30);
	}
}

main(int argc, char **argv)
{
	CLinuxConApp PiCanS88App;

	if(!PiCanS88App.GetOpts(argc, argv))
		exit(1);

	PiCanS88App.BackgroundCheck();

//	CCAN2UDP cCAN2UDP(PiCanS88App);
	CCAN2LAN cCAN2LAN(PiCanS88App);
	CS88UDP oS88UDP(PiCanS88App);
/*
	cCAN2UDP.m_iVerbose = PiCanS88App.m_iOpt_Verbose;
	cCAN2UDP.m_iBackground = PiCanS88App.m_iOpt_Background;
	cCAN2UDP.m_iDestination_port = PiCanS88App.m_iDestination_port;
	cCAN2UDP.m_iLocal_port = PiCanS88App.m_iLocal_port;
	cCAN2UDP.m_iS = PiCanS88App.m_iS;
	memcpy(&cCAN2UDP.m_sBAddr, &PiCanS88App.m_sBAddr, sizeof(PiCanS88App.m_sBAddr));
	memcpy(&cCAN2UDP.m_ifr, &PiCanS88App.m_ifr, sizeof(PiCanS88App.m_ifr));

	oS88UDP.m_iVerbose = PiCanS88App.m_iOpt_Verbose;
	oS88UDP.m_iBackground = PiCanS88App.m_iOpt_Background;
	oS88UDP.m_iDestination_port = PiCanS88App.m_iDestination_port;
	oS88UDP.m_iOpt_ModulCount = PiCanS88App.m_iOpt_ModulCount;
	oS88UDP.m_iSensorOffset = PiCanS88App.m_iSensorOffset;
	strcpy(oS88UDP.m_Destip, PiCanS88App.m_Destip);
	memcpy(&oS88UDP.m_Destaddr, &PiCanS88App.m_Destaddr, sizeof(PiCanS88App.m_Destaddr));
/*/
	
	if(PiCanS88App.m_iOpt_SearchLoco)
	{
		cCAN2LAN.m_iOpt_SearchLocoStart = PiCanS88App.m_iOpt_SearchLocoStart;
		cCAN2LAN.m_iOpt_SearchLocoEnd = PiCanS88App.m_iOpt_SearchLocoEnd;
		cCAN2LAN.m_iOpt_SearchLocoValue = PiCanS88App.m_iOpt_SearchLocoValue;
		cCAN2LAN.FindLokAddress();
		exit(EXIT_SUCCESS);
	}

	pthread_t threadCAN;
	pthread_t threadS88;
	pthread_t threadPing;
	int result_code;

	printf("Creating thread CAN\n");
	result_code = pthread_create(&threadCAN, NULL, CANThread, (void *) &cCAN2LAN);
	assert(0 == result_code);

	printf("Creating thread S88\n");
	result_code = pthread_create(&threadS88, NULL, S88Thread, (void *) &oS88UDP);
	assert(0 == result_code);

	printf("Creating thread Ping\n");
	result_code = pthread_create(&threadPing, NULL, PingThread, (void *) &cCAN2LAN);
	assert(0 == result_code);
/*
	while ( 1 ) 
	{
		cCAN2UDP.HandleEvents();
//		oS88UDP.HandleS88Events();
	}
*/
/*
	while(1)
	{
		sleep(15);
		cCAN2UDP.DiscoverLoks();
	}
*/
	result_code = pthread_join(threadCAN, NULL);
	assert(0 == result_code);
	result_code = pthread_join(threadS88, NULL);
	assert(0 == result_code);
	result_code = pthread_join(threadPing, NULL);
	assert(0 == result_code);

	exit(EXIT_SUCCESS);
}

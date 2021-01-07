// CAN2UDP.cpp: Implementierung der Klasse CCAN2UDP.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <linux/can.h>
#include <bcm2835.h>

#include "CAN2UDP.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CCAN2UDP::CCAN2UDP()
{
    memset(&m_sSAddr, 0, sizeof(m_sSAddr));
    memset(&m_sBAddr, 0, sizeof(m_sBAddr));
    memset(&m_sCANAddr, 0, sizeof(m_sCANAddr));
    memset(&m_CanFrame, 0, sizeof(m_CanFrame));
    memset(m_udpframe, 0, sizeof(m_udpframe));
    memset(m_udpframe_reply, 0, sizeof(m_udpframe_reply));
	
	m_iVerbose = 0;
	m_iBackground = 1;
 	m_iOn = 1;
	m_sCANAddrlen = sizeof(m_sCANAddr);
	strcpy(m_Destip, "127.0.0.1");
	m_iUDP_Destination_port = 15730;
	m_iUDP_Local_port = 15731;
    m_iCanid = 0;
    strcpy(m_cRocrail_server, "255.255.255.255");

	Init();
}

CCAN2UDP::CCAN2UDP(CLinuxConApp App)
{
    memset(&m_sSAddr, 0, sizeof(m_sSAddr));
    memset(&m_sBAddr, 0, sizeof(m_sBAddr));
    memset(&m_sCANAddr, 0, sizeof(m_sCANAddr));
    memset(&m_CanFrame, 0, sizeof(m_CanFrame));
    memset(m_udpframe, 0, sizeof(m_udpframe));
    memset(m_udpframe_reply, 0, sizeof(m_udpframe_reply));
	
	m_iVerbose = 0;
	m_iBackground = 1;
 	m_iOn = 1;
	m_sCANAddrlen = sizeof(m_sCANAddr);
	strcpy(m_Destip, "127.0.0.1");
	m_iUDP_Destination_port = 15730;
	m_iUDP_Local_port = 15731;
    m_iCanid = 0;
    strcpy(m_cRocrail_server, "255.255.255.255");

	
	m_iVerbose = App.m_iOpt_Verbose;
	m_iBackground = App.m_iOpt_Background;
	m_iUDP_Destination_port = App.m_iUDP_Destination_port;
	m_iUDP_Local_port = App.m_iUDP_Local_port;
	m_iS = App.m_iS;
	memcpy(&m_sBAddr, &App.m_sBAddr, sizeof(App.m_sBAddr));
	memcpy(&m_ifr, &App.m_ifr, sizeof(App.m_ifr));

	printf("m_ifr: [%s]\n", m_ifr.ifr_name);

	Init();
}

void CCAN2UDP::Init()
{
	/* prepare udp destination struct with defaults */
    m_sBAddr.sin_family = AF_INET;
    m_sBAddr.sin_port = htons(m_iUDP_Destination_port);
    m_iS = inet_pton(AF_INET, m_cRocrail_server, &m_sBAddr.sin_addr);
    if (m_iS <= 0) 
	{
		if (m_iS == 0) 
		{
			fprintf(stderr, "UDP IP invalid\n");
		} 
		else 
		{
			fprintf(stderr, "invalid address family\n");
		}
		exit(EXIT_FAILURE);
    }
	/*
	printf("m_iUDP_Destination_port: [%d]\n", m_iUDP_Destination_port);
	printf("m_iUDP_Local_port: [%d]\n", m_iUDP_Local_port);
	printf("m_sBAddr.sin_addr: [%d]\n", m_sBAddr.sin_addr);
	printf("m_sBAddr.sin_port: [%d]\n", m_sBAddr.sin_port);
	printf("m_sBAddr.sin_family: [%d]\n", m_sBAddr.sin_family);
	printf("m_sBAddr.sin_zero: [%d]\n", m_sBAddr.sin_zero);
	printf("m_iS: [%d]\n", m_iS);
	*/

    if ((m_iUDP_Socket = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		fprintf(stderr, "UDP socket error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
    }
	
    m_sSAddr.sin_family = AF_INET;
    m_sSAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_sSAddr.sin_port = htons(m_iUDP_Local_port);
	
    if (bind(m_iUDP_Socket, (struct sockaddr *)&m_sSAddr, sizeof(m_sSAddr)) < 0)
	{
		fprintf(stderr, "UDP bind error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
    }
	
    if ((m_iCAN_Socket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
	{
		fprintf(stderr, "CAN socket error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
    }
	
//	printf("m_ifr2: [%s]", m_ifr.ifr_name);
    m_sCANAddr.can_family = AF_CAN;
    if (ioctl(m_iCAN_Socket, SIOCGIFINDEX, &m_ifr) < 0)
	{
		fprintf(stderr, "CAN setup error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
    }
    m_sCANAddr.can_ifindex = m_ifr.ifr_ifindex;
	
    if (bind(m_iCAN_Socket, (struct sockaddr *)&m_sCANAddr, m_sCANAddrlen) < 0)
	{
		fprintf(stderr, "CAN bind error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
    }
	
    /* prepare UDP sending socket */
    if ((m_iUDPBroadcast_Socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		fprintf(stderr, "Send UDP socket error %s\n", strerror(errno));
		exit(EXIT_FAILURE);
    }
    if (setsockopt(m_iUDPBroadcast_Socket, SOL_SOCKET, SO_BROADCAST, &m_iOn, sizeof(m_iOn)) < 0)
	{
		fprintf(stderr, "UDP set socket option error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
    }
	
    /* start Maerklin 60113 box */
    send_magic_start_60113_frame(m_iCAN_Socket, m_iVerbose);

//	sleep(3);

//	DiscoverLoks();
}

CCAN2UDP::~CCAN2UDP()
{
    close(m_iCAN_Socket);
    close(m_iUDP_Socket);
    close(m_iUDPBroadcast_Socket);
}

void CCAN2UDP::send_magic_start_60113_frame(int can_socket, int verbose)
{
    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.can_id = 0x360301UL;
    /* use EFF */
    frame.can_id &= CAN_EFF_MASK;
    frame.can_id |= CAN_EFF_FLAG;
    frame.can_dlc = 5;
    frame.data[0] = 0;
    frame.data[1] = 0;
    frame.data[2] = 0;
    frame.data[3] = 0;
    frame.data[4] = 0x11;

    if (write(can_socket, &frame, sizeof(frame)) != sizeof(frame))
	{
		fprintf(stderr, "CAN magic 60113 start write error: %s\n", strerror(errno));
	}
	else
	{
		if (verbose)
			printf("CAN magic 60113 start write\n");
    }
}

void DecodeFrameData(unsigned char* pframe)
{
//	printf(" Frame Data: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n", pframe[0], pframe[1], pframe[2], pframe[3], pframe[4], pframe[5], pframe[6], pframe[7], pframe[8], pframe[9], pframe[10], pframe[11], pframe[12]);

	unsigned int uiCanID = pframe[0] << 24 | pframe[1] << 16 | pframe[2] << 8 | pframe[3];
	unsigned int uiPrio = (uiCanID & 0x1E000000) >> 25;
	unsigned int uiCmd	= (uiCanID & 0x01FE0000) >> 17;
	unsigned int uiResp = (uiCanID & 0x010000) >> 16;
	unsigned int uiHash = uiCanID & 0xFFFF;

	unsigned int uiTargetUID;
	unsigned int uiSubCmd;


	printf("CANID 0x%08X", uiCanID);
	printf(" Prio: 0x%02X", uiPrio);
	printf(" Cmd: 0x%02X", uiCmd);
	printf(" Resp: 0x%02X", uiResp);
	printf(" Hash: 0x%04X", uiHash);
	printf(" DLC: [%d]", pframe[4]);
/*	for (int i = 5; i < 5 + pframe[4]; i++)
	{
		printf(" %02x", pframe[i]);
	}*/
	printf("\n");
	
	switch(uiCmd)
	{
	case 0:	//System Cmd
		uiTargetUID = pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8];
		uiSubCmd = pframe[9];
		printf(" SubCmd 0x%02X:", uiSubCmd);
		switch(uiSubCmd)
		{
			case 0:
				printf(" System Stop - Zielgerät-UID: [%08X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8]);
				break;
			case 1:
				printf(" System Go - Zielgerät-UID: [%08X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8]);
				break;
			case 2:
				printf(" System Halt - Zielgerät-UID: [%08X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8]);
				break;
			case 3:
				printf(" Lok Nothalt - Loc-ID: [%08X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8]);
				break;
			case 4:
				printf(" Lok Zyklus Stop - Loc-ID: [%08X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8]);
				break;
			case 5:
				printf(" Lok Datenprotokoll - Loc-ID: [%08X], Gleisprotokoll: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[10]);
				break;
			case 6:
				printf(" Schaltzeit Zubehördecoder festlegen - Zielgerät-UID: [%08X], Zeit: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[10] << 8 | pframe[11]);
				break;
			case 7:
				printf(" Fast Read für mfx SID - Absender-UID: [%08X], MFX-SID: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[10] << 8 | pframe[11]);
				break;
			case 8:
				printf(" Gleisprotokoll Frei Schalten - Ziel-UID: [%08X], Param: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[10]);
				if(pframe[10] & 0x01)
					printf(" (MM2)");
				if(pframe[10] & 0x02)
					printf(" (MFX)");
				if(pframe[10] & 0x04)
					printf(" (DCC)");
				break;
			case 9:
				printf(" MFX set Neuanmeldezähler - Zielgerät-UID: [%08X], Neu-Zaehler: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[10] << 8 | pframe[11]);
				break;
			case 0x0A:
				printf(" System Überlast - Absender-UID: [%08X], Kanalnr: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[10]);
				break;
			case 0x0B:
				if(uiResp == 0 && pframe[4] == 6)
					printf(" System Status Anfrage - Zielgerät-UID: [%08X], Kanalnr: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[10]);
				if(uiResp == 0 && pframe[4] == 8)
					printf(" System Status Antwort - Absender-UID: [%08X], Kanalnr: [%02X], Konfigurationswert: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[10], pframe[11] << 8 | pframe[12]);
				if(uiResp == 1 && pframe[4] == 7)
					printf(" System Status Antwort - Zielgerät-UID: [%08X], Kanalnr: [%02X], True/False: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[10], pframe[11]);
				if(uiResp == 1 && pframe[4] == 8)
					printf(" System Status Antwort - Absender-UID: [%08X], Kanalnr: [%02X], Messwert: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[10], pframe[11] << 8 | pframe[12]);
				if(uiResp == 1 && pframe[4] == 6)
					printf(" System Status Antwort - Zielgerät-UID: [%08X], Kanalnr existiert nicht!", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8]);
				break;
			case 0x0C:
				if(pframe[4] == 5)
					printf(" Gerätekennung - Zielgerät-UID: [%08X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8]);
				if(pframe[4] == 7)
					printf(" Gerätekennung - Zielgerät-UID: [%08X], SystemKenner: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[10] << 8 | pframe[11]);
				break;
			case 0x80:
				printf(" System Reset - Zielgerät-UID: [%08X], ResetZiel: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[10]);
				break;
			default:
				printf(" unknown Systemcommand");
		}
		break;
	case 1:	//Verwaltung	Can-ID: 0x02
		printf("Command: Lok Discovery: ");
		if(uiResp == 0 && pframe[4] == 1)
			printf(" Protokollkennung: [%02X]", pframe[5]);
		if(pframe[4] == 5)
			printf(" MFX-UID / Loc-ID: [%08X], Range / Protokollkennung: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9]);
		if(uiResp == 1 && pframe[4] == 6)
			printf(" MFX-UID / Loc-ID: [%08X], Range: [%02X], ASK-Verhältnis: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9], pframe[10]);
		break;
	case 2:	//Bind	Can-ID: 0x04
		printf("Command: MFX Bind: ");
		printf(" MFX-UID: [%08X], MFX-SID: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9] << 8 | pframe[10]);
		break;
	case 3:	//Verify	Can-ID: 0x06
		printf("Command: MFX Verify: ");
		if(pframe[4] == 6)
			printf(" MFX-UID: [%08X], MFX-SID: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9] << 8 | pframe[10]);
		if(pframe[4] == 7)
			printf(" MFX-UID: [%08X], MFX-SID: [%04X], ASK-Verhältnis: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9] << 8 | pframe[10], pframe[11]);
		break;
	case 4:	//Lok Geschwindigkeit	Can-ID: 0x08
		printf("Command: Lok Geschwindigkeit: ");
		if(pframe[4] == 4)
			printf(" Loc-ID: [%08X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8]);
		if(pframe[4] == 6)
			printf(" Loc-ID: [%08X], Geschwindigkeit: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9] << 8 | pframe[10]);
		break;
	case 5:	//Lok Richtung	Can-ID: 0x0A
		printf("Command: Lok Richtung: ");
		if(pframe[4] == 4)
			printf(" Loc-ID: [%08X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8]);
		if(pframe[4] == 5)
			printf(" Loc-ID: [%08X], Richtung: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9]);
		break;
	case 6:	//Lok Funktion	Can-ID: 0x0C
		printf("Command: Lok Funktion: ");
		if(pframe[4] == 5)
			printf(" Loc-ID: [%08X], Funktion: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9]);
		if(pframe[4] == 6)
			printf(" Loc-ID: [%08X], Funktion: [%02X], Wert: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9], pframe[10]);
		if(pframe[4] == 8)
			printf(" Loc-ID: [%08X], Funktion: [%02X], Wert: [%02X], Funktionswert: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9], pframe[10], pframe[11] << 8 | pframe[12]);
		break;
	case 7:	//Read Config	Can-ID: 0x0E
		printf("Command: Read Config: ");
		if(pframe[4] == 7)
			printf(" Loc-ID: [%08X], CV Index/Nummer: [%04X], Anzahl/Wert: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9] << 8 | pframe[10], pframe[11]);
		if(pframe[4] == 6)
			printf(" Loc-ID: [%08X], CV Index/Nummer: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9] << 8 | pframe[10]);
		break;
	case 8:	//Write Config	Can-ID: 0x10
		printf("Command: Write Config: ");
		if(pframe[4] == 8)
			printf(" Loc-ID: [%08X], CV Index/Nummer: [%04X], Wert: [%02X], Ctrl/Rslt: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9] << 8 | pframe[10], pframe[11], pframe[12]);
		break;
	case 0x0B:	//Zubehör Schalten	Can-ID: 0x16
		printf("Command: Zubehör Schalten: ");
		if(pframe[4] == 6)
			printf(" Loc-ID: [%08X], Stellung: [%02X], Strom: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9], pframe[10]);
		if(pframe[4] == 8)
			printf(" Loc-ID: [%08X], Stellung: [%02X], Strom: [%02X], Schaltzeit/Sonderwert: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9], pframe[10], pframe[11] << 8 | pframe[12]);
		break;
	case 0x10:	//S88 Polling	Can-ID: 0x20
		printf("Command: S88 Polling: ");
		if(pframe[4] == 5)
			printf(" Gerät-UID: [%08X], Modulanzahl: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9]);
		if(pframe[4] == 7)
			printf(" Gerät-UID: [%08X], Modulanzahl: [%02X], Zusand: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9], pframe[10] << 8 | pframe[11]);
		break;
	case 0x11:	//S88 Event	Can-ID: 0x22
		printf("Command: S88 Event: ");
		if(pframe[4] == 4)
			printf(" Gerätekenner: [%04X], Kontakkennung: [%04X]", pframe[5] << 8 | pframe[6], pframe[7] << 8 | pframe[8]);
		if(pframe[4] == 5)
			printf(" Gerätekenner: [%04X], Kontakkennung: [%04X], Parameter: [%02X]", pframe[5] << 8 | pframe[6], pframe[7] << 8 | pframe[8], pframe[9]);
		if(pframe[4] == 8)
			printf(" Gerätekenner: [%04X], Kontakkennung: [%04X], Zustand alt: [%02X], Zustand neu: [%02X], Zeit: [%04X]", pframe[5] << 8 | pframe[6], pframe[7] << 8 | pframe[8], pframe[9], pframe[10], pframe[11] << 8 | pframe[12]);
		break;
	case 0x18:	//Teilnehmer Ping	Can-ID: 0x30
		printf("Command: Teilnehmer Ping: ");
		if(pframe[4] == 0)
			printf(" Ping request");
		if(pframe[4] == 8)
		{
			printf(" Absender-UID: [%08X], SW-Version: [%04X], Gerätekennung: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9] << 8 | pframe[10], pframe[11] << 8 | pframe[12]);
			if(pframe[11] == 0x00 && pframe[12] == 0x00)
				printf(" (Gleis Format Prozessor 60213,60214 / Booster 60173, 60174)");
			if(pframe[11] == 0x00 && pframe[12] == 0x10)
				printf(" (Gleisbox 60112 und 60113)");
			if(pframe[11] == 0x00 && pframe[12] == 0x20)
				printf(" (Connect 6021 Art-Nr.60128)");
			if(pframe[11] == 0x00 && pframe[12] == 0x30)
				printf(" (MS 2 60653, Txxxxx)");
			if(pframe[11] == 0xFF && pframe[12] == 0xE0)
				printf(" (Wireless Devices)");
			if(pframe[11] == 0xFF && pframe[12] == 0xFF)
				printf(" (CS2-GUI (Master))");
		}
		break;
	case 0x1D:	//Statusdaten Konfiguration	Can-ID: 0x3A
		printf("Command: Statusdaten Konfiguration: ");
		if(uiResp == 0 && pframe[4] == 5)
			printf(" Zielgerät-UID: [%08X], Index: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9]);
		if(uiResp == 1 && pframe[4] == 8)
			printf(" Antwort mit Stream, Packet # ist in Hash");
		if(uiResp == 1 && pframe[4] == 6)
			printf(" Geräte-UID: [%08X], Index: [%02X], Paketanzahl: [%02X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[9], pframe[10]);
		break;
	case 0x20:	//Anfordern Config Data	Can-ID: 0x40
		printf("Command: Anfordern Config Data für CS2: ");
		break;
	case 0x21:	//Config Data Stream	Can-ID: 0x42
		printf("Command: Config Data Stream: ");
		if(pframe[4] == 6 || pframe[4] == 7)
			printf(" Datei/Streamlänge in Bytes: [%08X], CRC: [%04X]", pframe[5] << 24 | pframe[6] << 16 | pframe[7] << 8 | pframe[8], pframe[7] << 8 | pframe[8], pframe[9] << 8 | pframe[10]);
		break;
	case 0x30:	//Automatik schalten	Can-ID: 0x42
		printf("Command: Automatik schalten: ");
		if(pframe[4] == 6)
			printf(" Gerätekenner: [%04X], Automatik Funktion: [%04X], Stellung/Status: [%02X], Parameter: [%02X]", pframe[5] << 8 | pframe[6], pframe[7] << 8 | pframe[8], pframe[9], pframe[10]);
		if(pframe[4] == 8)
			printf(" Gerätekenner: [%04X], Automatik Funktion: [%04X], Loc-ID: [%08X]", pframe[5] << 8 | pframe[6], pframe[7] << 8 | pframe[8], pframe[9] << 24 | pframe[10] << 16 | pframe[11] << 8 | pframe[12]);
		break;
	default:
		printf(" unknown Command");
	}
	printf("\n\n");
}
/*

<-UDP>CAN CANID 0x020300   [1] 6000 00 00 00 00
CANID 0x00020300 Prio: 0x00 Cmd: 0x01 Resp: 0x00 Hash: 0x0300 DLC: [1]
Command: Lok Discovery:  Protokollkennung: [60]

*/

void CCAN2UDP::FindLokAddress()
{
#if 0
    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

	frame.can_id = 0x0C2F17UL;
	// use EFF
	frame.can_id &= CAN_EFF_MASK;
	frame.can_id |= CAN_EFF_FLAG;
	frame.can_dlc = 6;
	frame.data[2] = 0x40;
	frame.data[3] = 0x07;
	frame.data[4] = 0x0;
	frame.data[5] = 0x1;

    if (write(m_iCAN_Socket, &frame, sizeof(frame)) != sizeof(frame))
	{
		fprintf(stderr, "CAN Discovering Lokos write error: %s\n", strerror(errno));
	}
	else
	{
		if (m_iVerbose)
			printf("CAN Discovering Lokos\n");
    }
#endif

#if 1
    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

	frame.can_id = 0x0C2F17UL;
	// use EFF
	frame.can_id &= CAN_EFF_MASK;
	frame.can_id |= CAN_EFF_FLAG;
	frame.can_dlc = 6;
	frame.data[2] = 0x40;
//	frame.data[3] = 0x05;
	frame.data[4] = 0x0;
	frame.data[5] = m_iOpt_SearchLocoValue;

	printf("\n");
	for(int i = m_iOpt_SearchLocoStart; i <= m_iOpt_SearchLocoEnd; i++)
	{
		frame.data[3] = i;

		printf("Turning Headlights on for Loko at address: %d\r", i);
		fflush(stdout);
		if (write(m_iCAN_Socket, &frame, sizeof(frame)) != sizeof(frame))
		{
			fprintf(stderr, "CAN Discovering Lokos write error: %s\n", strerror(errno));
		}

		sleep(1);
    }
#endif
}

void CCAN2UDP::DiscoverLoks()
{

    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

	frame.can_id = 0x0C2F17UL;
	// use EFF
	frame.can_id &= CAN_EFF_MASK;
	frame.can_id |= CAN_EFF_FLAG;
	frame.can_dlc = 5;
	frame.data[2] = 0x40;
	frame.data[3] = 0x05;
	frame.data[4] = 0x0;
	frame.data[5] = 0x1;

    if (write(m_iCAN_Socket, &frame, sizeof(frame)) != sizeof(frame))
	{
		fprintf(stderr, "CAN Discovering Lokos write error: %s\n", strerror(errno));
	}
	else
	{
		if (m_iVerbose)
			printf("CAN Discovering Lokos\n");
    }

	
#if 0
    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

	frame.can_id = 0x020300UL;
	frame.can_id &= CAN_EFF_MASK;
	frame.can_id |= CAN_EFF_FLAG;
	frame.can_dlc = 1;
	frame.data[0] = 0x21;
	frame.data[0] = 0x22;
	frame.data[0] = 0x60;
	frame.data[0] = 0x20;

    if (write(m_iCAN_Socket, &frame, sizeof(frame)) != sizeof(frame))
	{
		fprintf(stderr, "CAN Discovering Lokos write error: %s\n", strerror(errno));
	}
	else
	{
		if (m_iVerbose)
			printf("CAN Discovering Lokos\n");
    }
#endif

}

#if 0
void CCAN2UDP::DiscoverLoks()
{

    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

	frame.can_id = 0x020300UL;
	// use EFF
	frame.can_id &= CAN_EFF_MASK;
	frame.can_id |= CAN_EFF_FLAG;
	frame.can_dlc = 5;
	frame.data[2] = 0x40;
	frame.data[3] = 0x05;
	frame.data[4] = 0x60;

    if (write(m_iCAN_Socket, &frame, sizeof(frame)) != sizeof(frame))
	{
		fprintf(stderr, "CAN Discovering Lokos write error: %s\n", strerror(errno));
	}
	else
	{
		if (m_iVerbose)
			printf("CAN Discovering Lokos\n");
    }

	
#if 0
    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

	frame.can_id = 0x020300UL;
	frame.can_id &= CAN_EFF_MASK;
	frame.can_id |= CAN_EFF_FLAG;
	frame.can_dlc = 1;
	frame.data[0] = 0x21;
	frame.data[0] = 0x22;
	frame.data[0] = 0x60;
	frame.data[0] = 0x20;

    if (write(m_iCAN_Socket, &frame, sizeof(frame)) != sizeof(frame))
	{
		fprintf(stderr, "CAN Discovering Lokos write error: %s\n", strerror(errno));
	}
	else
	{
		if (m_iVerbose)
			printf("CAN Discovering Lokos\n");
    }
#endif

	/*
	struct can_frame CanFrameDiscover;

	printf("Discovering Lokos\n\n");
	/* CAN is stored in network Big Endian format *//*
	CanFrameDiscover.can_id = 0x24711;
	CanFrameDiscover.can_dlc = 1;
	CanFrameDiscover.data[5] = 0x21;
	memset(&CanFrameDiscover.data[6], 0, 7);

	if (write(m_iCAN_Socket, &CanFrameDiscover, sizeof(CanFrameDiscover)) != sizeof(CanFrameDiscover))
		fprintf(stderr, "CAN write error: %s\n", strerror(errno));
*/
}
#endif

void CCAN2UDP::HandleEvents()
{
	FD_ZERO(&m_readfds);
	FD_SET(m_iCAN_Socket, &m_readfds);
	FD_SET(m_iUDP_Socket, &m_readfds);
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 50000;	//50.000 microseconds -> 50 Milliseconds

//	printf("HandleEvents()\n");
//	if (select((m_iCAN_Socket > m_iUDP_Socket) ? m_iCAN_Socket + 1 : m_iUDP_Socket + 1, &m_readfds, NULL, NULL, &timeout) < 0)
	if (select((m_iCAN_Socket > m_iUDP_Socket) ? m_iCAN_Socket + 1 : m_iUDP_Socket + 1, &m_readfds, NULL, NULL, NULL) < 0)
	{
		fprintf(stderr, "select error: %s\n", strerror(errno));
	}
	
	/* received a CAN frame */
	if (FD_ISSET(m_iCAN_Socket, &m_readfds))
	{
//		printf("received a CAN frame\n");
		if (read(m_iCAN_Socket, &m_CanFrame, sizeof(struct can_frame)) < 0)
		{
			fprintf(stderr, "CAN read error: %s\n", strerror(errno));
		}
		else if (m_CanFrame.can_id & CAN_EFF_FLAG)
		{	/* only EFF frames are valid */
			/* prepare UDP frame */
			m_CanFrame.can_id &= CAN_EFF_MASK;
			
			m_udpframe[0] = (m_CanFrame.can_id >> 24) & 0x000000FF;
			m_udpframe[1] = (m_CanFrame.can_id >> 16) & 0x000000FF;
			m_udpframe[2] = (m_CanFrame.can_id >> 8) & 0x000000FF;
			m_udpframe[3] = m_CanFrame.can_id & 0x000000FF;
			m_udpframe[4] = m_CanFrame.can_dlc;
			memcpy(&m_udpframe[5], &m_CanFrame.data, m_CanFrame.can_dlc);
			
			/* send UDP frame */
			if (sendto(m_iUDPBroadcast_Socket, m_udpframe, 13, 0, (struct sockaddr *)&m_sBAddr, sizeof(m_sBAddr)) != 13)
				fprintf(stderr, "UDP write error: %s\n", strerror(errno));
			
			if (m_iVerbose && !m_iBackground)
			{
				printf("->CAN>UDP CANID 0x%06X R", m_CanFrame.can_id);
				printf(" [%d]", m_udpframe[4]);
				for (int i = 5; i < 5 + m_CanFrame.can_dlc; i++)
				{
					printf(" %02x", m_udpframe[i]);
				}
				printf("\n");
			
				unsigned char pframe[13];
				pframe[0] = (m_CanFrame.can_id >> 24) & 0x000000FF;
				pframe[1] = (m_CanFrame.can_id >> 16) & 0x000000FF;
				pframe[2] = (m_CanFrame.can_id >> 8) & 0x000000FF;
				pframe[3] = m_CanFrame.can_id & 0x000000FF;
				pframe[4] = m_CanFrame.can_dlc;
				memcpy(&pframe[5], &m_CanFrame.data, m_CanFrame.can_dlc);
			//	printf(" New CAN Frame Data: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n", pframe[0], pframe[1], pframe[2], pframe[3], pframe[4], pframe[5], pframe[6], pframe[7], pframe[8], pframe[9], pframe[10], pframe[11], pframe[12]);
				DecodeFrameData((unsigned char*)pframe);
			}
		}
	}
	/* received a UDP packet */
	if (FD_ISSET(m_iUDP_Socket, &m_readfds))
	{
//		printf("received a UDP packet\n");
		if (read(m_iUDP_Socket, m_udpframe, MAXDG) == 13)
		{
	//		unsigned char* pframe = m_udpframe;
	//		printf(" New UDP Frame Data: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n", pframe[0], pframe[1], pframe[2], pframe[3], pframe[4], pframe[5], pframe[6], pframe[7], pframe[8], pframe[9], pframe[10], pframe[11], pframe[12]);

		/* Maerklin UDP Format: always 13 bytes
		*   byte 0-3 CAN ID
		*   byte 4 DLC
		*   byte 5-12 CAN data
			*/
			memcpy(&m_iCanid, &m_udpframe[0], 4);
			/* CAN is stored in network Big Endian format */
			m_CanFrame.can_id = ntohl(m_iCanid);
			m_CanFrame.can_id &= CAN_EFF_MASK;
			m_CanFrame.can_id |= CAN_EFF_FLAG;
			m_CanFrame.can_dlc = m_udpframe[4];
			memcpy(&m_CanFrame.data, &m_udpframe[5], 8);
			/* send CAN frame */
			memcpy(&m_CanFrame.data, &m_udpframe[5], 8);
			/* answer to CAN ping from LAN to LAN */
			if (((m_CanFrame.can_id & 0x00FF0000UL) == 0x00310000UL) &&
				(m_udpframe[11] = 0xEE) && (m_udpframe[12] = 0xEE))
			{
				printf("  received CAN ping\n");
				memcpy(m_udpframe_reply, m_udpframe, 13);
				m_udpframe_reply[0] = 0x00;
				m_udpframe_reply[1] = 0x30;
				m_udpframe_reply[2] = 0x00;
				m_udpframe_reply[3] = 0x00;
				m_udpframe_reply[4] = 0x00;
				if (sendto(m_iUDPBroadcast_Socket, m_udpframe_reply, 13, 0, (struct sockaddr *)&m_sBAddr, sizeof(m_sBAddr)) != 13)
					fprintf(stderr, "UDP write error: %s\n", strerror(errno));
				else
					printf("  replied to CAN ping\n");
			}
			
			if (write(m_iCAN_Socket, &m_CanFrame, sizeof(m_CanFrame)) != sizeof(m_CanFrame))
				fprintf(stderr, "CAN write error: %s\n", strerror(errno));
			
			if (m_iVerbose && !m_iBackground)
			{
				printf("<-UDP>CAN CANID 0x%06X  ", m_CanFrame.can_id & CAN_EFF_MASK);
				printf(" [%d]", m_udpframe[4]);
				for (int i = 5; i < 5 + m_CanFrame.can_dlc; i++)
					printf(" %02x", m_udpframe[i]);
				printf("\n");
				DecodeFrameData(m_udpframe);
			}
		}
	}
}

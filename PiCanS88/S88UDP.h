// S88UDP.h: Schnittstelle für die Klasse CS88UDP.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S88UDP_H__3E587985_F867_46F3_B7E8_CF3615DE2130__INCLUDED_)
#define AFX_S88UDP_H__3E587985_F867_46F3_B7E8_CF3615DE2130__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LinuxConApp.h"

#define PI2_S88_DATA_PIN	RPI_V2_GPIO_P1_11
#define PI2_S88_CLOCK_PIN	RPI_V2_GPIO_P1_15
#define PI2_S88_LOAD_PIN	RPI_V2_GPIO_P1_16
#define PI2_S88_RESET_PIN	RPI_V2_GPIO_P1_18

#define MICRODELAY 50
#define MAXMODULES 64

class CS88UDP  : public CLinuxConApp
{
public:
	CS88UDP();
	CS88UDP(CLinuxConApp App);
	virtual ~CS88UDP();
	void Init();

	void Send_Sensor_Event(int sock, const struct sockaddr *destaddr, int verbose, int offset, int address, int value);
	void HandleS88Events();

	int m_iSensorOffset;
	int m_iModulCount;
	int m_iarSensors[ MAXMODULES * 16 ];
	
	int m_iVerbose;
	int m_iBackground;
	
	int m_iUdpsock;
	struct hostent *m_pHp;
	struct sockaddr_in m_Destaddr;
	int m_iOn;
	char m_Destip[16];
	int m_iUDP_Destination_port;

};

#endif // !defined(AFX_S88UDP_H__3E587985_F867_46F3_B7E8_CF3615DE2130__INCLUDED_)

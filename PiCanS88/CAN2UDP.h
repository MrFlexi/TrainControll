// CAN2UDP.h: Schnittstelle für die Klasse CCAN2UDP.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAN2UDP_H__218120DF_C249_4EF1_9C57_FDDEA1FDA237__INCLUDED_)
#define AFX_CAN2UDP_H__218120DF_C249_4EF1_9C57_FDDEA1FDA237__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LinuxConApp.h"

#define MAXDG   4096		/* maximum datagram size */
#define MAXUDP  16		/* maximum datagram size */

class CCAN2UDP  : public CLinuxConApp
{
public:
	CCAN2UDP();
	CCAN2UDP(CLinuxConApp App);
	virtual ~CCAN2UDP();
	void Init();

	void send_magic_start_60113_frame(int can_socket, int verbose);
	void HandleEvents();
	void DiscoverLoks();
	void FindLokAddress();

	int m_iS;
    struct can_frame m_CanFrame;
	int m_iUDP_Socket;
	int m_iCAN_Socket;
	int m_iUDPBroadcast_Socket;

	int m_iVerbose;
	int m_iBackground;
	
	struct sockaddr_in m_sSAddr;
	struct sockaddr_in m_sBAddr;
    struct sockaddr_can m_sCANAddr;
    struct ifreq m_ifr;
    socklen_t m_sCANAddrlen;
    fd_set m_readfds;
	int m_iOn;
	char m_Destip[16];
	int m_iUDP_Destination_port;
	int m_iUDP_Local_port;
    int m_iCanid;
    char m_cRocrail_server[16];

	unsigned char m_udpframe[MAXDG];
	unsigned char m_udpframe_reply[MAXDG];


};

#endif // !defined(AFX_CAN2UDP_H__218120DF_C249_4EF1_9C57_FDDEA1FDA237__INCLUDED_)

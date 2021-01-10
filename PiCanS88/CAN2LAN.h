// CAN2LAN.h: Schnittstelle für die Klasse CCAN2LAN.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAN2LAN_H__4F344659_C915_4740_9860_6E0F5EA33254__INCLUDED_)
#define AFX_CAN2LAN_H__4F344659_C915_4740_9860_6E0F5EA33254__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LinuxConApp.h"

#define MAXDG   4096		/* maximum datagram size */
#define MAXUDP  16		/* maximum datagram size */
#define MAXLINE			1024		/* max string length     */
#define MAX_TCP_CONN		16		/* max TCP clients       */
#define MAXMTU   		1400		/* maximum MTU           */
#define MAX_PACKETS		40		/* maximum number of CAN frames per TCP packet */
#define MAXIPLEN  		40		/* maximum IP string length */
#define MAX_UDP_BCAST_RETRY  	10		/* maximum retries getting UDP socket */
#define MAX(a,b)		((a) > (b) ? (a) : (b))

#define	CRC				0x01
#define COMPRESSED		0x02
#define TERM_SPEED		B500000

#if 0
struct cs2_config 
{
    int canid;
    int can_socket;
    int net_socket;
    int udp_socket;
    int tcp_socket;
    int config_flags;
    char *config_dir;
    char *filename;
    char **page_name;
    char *gleisbild_sr2;
};
#endif

struct id_node 
{
    uint32_t id;
    uint8_t slave_node;
    struct id_node *next;
};

struct config_data 
{
    int deflated_stream_size;
    int deflated_size;
    int inflated_size;
    uint16_t crc;
    char *name;
    int verbose;
    uint8_t *deflated_data;
    uint8_t *inflated_data;
};

#define MS1_BUFFER_SIZE 8
#define MS1_BUFFER_MASK (MS1_BUFFER_SIZE-1)

struct MS1_Node_Buffer 
{
    uint8_t data[MS1_BUFFER_SIZE];
    uint8_t read;
    uint8_t write;
};

class CCAN2LAN    : public CLinuxConApp
{
public:
	CCAN2LAN();
	CCAN2LAN(CLinuxConApp App);
	virtual ~CCAN2LAN();
	void Init();
	void OpenSockets();

	void send_magic_start_60113_frame();
	void send_can_ping_frame();
	void send_ping_frame();
	void HandleEvents();
	void DiscoverLoks();
	void FindLokAddress();

	int m_iS;
	int m_iMax_fds;
	int m_iMax_tcp;
	int m_iConn_fd;
	int m_iaTcp_client[MAX_TCP_CONN];
    struct can_frame m_CanFrame;
	int m_iUDP_Socket;
	int m_iCAN_Socket;
	int m_iUDPBroadcast_Socket;
	int m_iTCP_Socket;
	int m_iTCP_Socket2;
	int m_iTCP_Socket3;
	int m_iSelectRet;

	int m_iVerbose;
	int m_iBackground;
	
	struct sockaddr_in m_sSAddr;
	struct sockaddr_in m_sBAddr;
	struct sockaddr_in m_sTCPAddr;
	struct sockaddr_in m_sTCP2Addr;
    struct sockaddr_can m_sCANAddr;
    /* vars for determing broadcast address */
    struct ifaddrs *m_pBroadcastifap;
	struct ifaddrs *m_pBroadcastifa;
    struct ifreq m_ifr;
    socklen_t m_sCANAddrlen;
    socklen_t m_sTCPAddrlen;
    fd_set m_allfds;
    fd_set m_readfds;
	int m_iOn;
	char m_Destip[16];
	int m_iUDP_Destination_port;
	int m_iUDP_Local_port;
	int m_iTCP_Local_port;
	int m_iTCP_Local_port2;
    int m_iCanid;
    char m_cRocrail_server[16];
	char m_strBroadcastInterfaceName[16];
	char *m_pStrUdp_dst_address;

	unsigned char m_udpframe[MAXDG];
	unsigned char m_udpframe_reply[MAXDG];

	char m_strConfig_dir[MAXLINE];
	char m_strConfig_file[MAXLINE];
	char **m_ppPage_name;
};

#endif // !defined(AFX_CAN2LAN_H__4F344659_C915_4740_9860_6E0F5EA33254__INCLUDED_)

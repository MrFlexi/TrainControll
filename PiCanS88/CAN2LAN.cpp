// CAN2LAN.cpp: Implementierung der Klasse CCAN2LAN.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include <zlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/types.h>

#include <linux/can.h>
#include <bcm2835.h>
#include <ifaddrs.h>

#include "CAN2LAN.h"

#define TIME_WAIT_US	20 * 1000
/* CHUNK is the size of the memory chunk used by the zlib routines. */
#define CHUNK		0x8000
#define windowBits	15
#define GZIP_ENCODING	16

/* The following macro calls a zlib routine and checks the return
   value. If the return value ("status") is not OK, it prints an error
   message and exits the program. Zlib's error statuses are all less
   than zero. */

#define CALL_ZLIB(x) {                                                  \
        int status;                                                     \
        status = x;                                                     \
        if (status < 0) {                                               \
            fprintf (stderr,                                            \
                     "%s:%d: %s returned a bad status of %d.\n",        \
                     __FILE__, __LINE__, #x, status);                   \
            exit (EXIT_FAILURE);                                        \
        }                                                               \
    }

static const char *CAN_FORMAT_STRG      = "      CAN->  CANID 0x%08X R [%d]";
static const char *UDP_FORMAT_STRG      = "->CAN>UDP    CANID 0x%08X   [%d]";
static const char *TCP_FORMAT_STRG      = "->TCP>CAN    CANID 0x%08X   [%d]";
static const char *TCP_FORMATS_STRG     = "->TCP>CAN*   CANID 0x%08X   [%d]";
static const char *CAN_TCP_FORMAT_STRG  = "->CAN>TCP    CANID 0x%08X   [%d]";
static const char *NET_UDP_FORMAT_STRG  = "      UDP->  CANID 0x%08X   [%d]";

static const int s_iMAXPENDING = 16;	/* max outstanding tcp connections */
unsigned char g_cNetframe[MAXDG];
unsigned char g_cEC_frame[13];
struct timeval g_tvLast_senttime_CAN;

static uint16_t crc_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

/* Maerklin uses CCIT CRC, which is polynom 0x1021 -> x^16 + x^12 +x^5 + 1 */
uint16_t CRCCCITT(unsigned char *data, size_t length, unsigned short seed)
{

    size_t count;
    uint16_t crc = seed;

    for (count = 0; count < length; ++count) {
	uint16_t temp = (*data++ ^ (crc >> 8)) & 0xff;
	crc = crc_table[temp] ^ (crc << 8);
    }

    return crc;
}

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CCAN2LAN::CCAN2LAN()
{
	Init();
	OpenSockets();
}

CCAN2LAN::CCAN2LAN(CLinuxConApp App)
{
	Init();

	m_iVerbose = App.m_iOpt_Verbose;
	m_iBackground = App.m_iOpt_Background;
	m_iUDP_Destination_port = App.m_iUDP_Destination_port;
	m_iUDP_Local_port = App.m_iUDP_Local_port;
	m_iS = App.m_iS;
	memcpy(&m_sBAddr, &App.m_sBAddr, sizeof(App.m_sBAddr));
	memcpy(&m_ifr, &App.m_ifr, sizeof(App.m_ifr));

	OpenSockets();
}

void CCAN2LAN::Init()
{
	memset(&m_sSAddr, 0, sizeof(m_sSAddr));
	memset(&m_sBAddr, 0, sizeof(m_sBAddr));
	memset(&m_sTCPAddr, 0, sizeof(m_sTCPAddr));
	memset(&m_sTCP2Addr, 0, sizeof(m_sTCP2Addr));
	memset(&m_sCANAddr, 0, sizeof(m_sCANAddr));
	memset(&m_CanFrame, 0, sizeof(m_CanFrame));
	memset(m_udpframe, 0, sizeof(m_udpframe));
	memset(m_udpframe_reply, 0, sizeof(m_udpframe_reply));
	memset(&m_ifr, 0, sizeof(m_ifr));
	strcpy(m_ifr.ifr_name, "can0");
	memset(&m_strConfig_dir, 0, sizeof(m_strConfig_dir));
	memset(&m_strConfig_file, 0, sizeof(m_strConfig_file));

	memset(&g_tvLast_senttime_CAN, 0, sizeof(g_tvLast_senttime_CAN));

	m_ppPage_name = (char **)calloc(64, sizeof(char *));
	if (!m_ppPage_name)
	{
		fprintf(stderr, "can't alloc memory for page_name: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
    }

	m_iUDP_Socket = m_iCAN_Socket = m_iUDPBroadcast_Socket = m_iTCP_Socket = m_iTCP_Socket2 = m_iTCP_Socket3 = 0;

	m_iVerbose = 0;
	m_iBackground = 1;
	m_iOn = 1;
	m_sCANAddrlen = sizeof(m_sCANAddr);
	m_sTCPAddrlen = sizeof(m_sTCPAddr);
	strcpy(m_Destip, "127.0.0.1");
	m_iUDP_Destination_port = 15730;
	m_iUDP_Local_port = 15731;
	m_iTCP_Local_port = 15731;
	m_iTCP_Local_port2 = 15732;
	m_iCanid = 0;
	strcpy(m_cRocrail_server, "255.255.255.255");
	strcpy(m_strBroadcastInterfaceName, "br-lan");
	m_strConfig_file[0] = '\0';
	m_pStrUdp_dst_address = NULL;
	m_iSelectRet = 0;
}

int time_stamp(char *timestamp)
{
    /* char *timestamp = (char *)malloc(sizeof(char) * 16); */
    struct timeval tv;
    struct tm *tm;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    sprintf(timestamp, "%02d:%02d:%02d.%03d", tm->tm_hour, tm->tm_min, tm->tm_sec, (int)tv.tv_usec / 1000);
    return 0;
}

char *rmcrlf(char *s, int slen)
{
    s[slen - 1] = '\0';
    int l = strlen(s) - 1;

    while (l >= 0 && (s[l] == '\r' || s[l] == '\n'))
		s[l--] = '\0';

    return s;
}

char **read_track_file(char *filename, char **page_name)
{
	FILE *fp;
	int id = 0;
	int page = 0;
	char line[MAXLINE];
	
	if ((fp = fopen(filename, "r")) != NULL)
	{
		while (fgets(line, MAXLINE, fp) != NULL)
		{
			if (strstr(line, "seite") == line)
				page = 1;
			else if (strstr(line, " .id=") == line)
				id = strtoul(&line[5], NULL, 0);
			else if (strstr(line, " .name=") == line)
			{
				rmcrlf(line, MAXLINE);
				if (page)
				{
					page_name[id] = (char *)calloc(strlen(&line[7]) + 1, sizeof(char));
					if (page_name[id] == NULL)
					{
						fprintf(stderr, "%s: error calloc failed creating config buffer for %s\n", __func__, filename);
						return NULL;
					}
					strcpy(page_name[id], &line[7]);
				}
			}
		}
		/* fgets returned null */
		if (errno != 0)
		{
			fprintf(stderr, "error reading line\n");
			return NULL;
		}
		fclose(fp);
		/* EOF found, normal exit */
		return page_name;
	}
	else
	{
		fprintf(stderr, "error reading file %s\n", filename);
		return NULL;
	}
}

void CCAN2LAN::OpenSockets()
{
	/* read track file */
	if (m_strConfig_dir[0] == 0)
		strcat(m_strConfig_file, ".");
	strcat(m_strConfig_file, m_strConfig_dir);
	if (m_strConfig_dir[strlen(m_strConfig_dir) - 1] != '/')
		strcat(m_strConfig_file, "/");
	strncpy(m_strConfig_dir, m_strConfig_file, sizeof(m_strConfig_dir) - 1);
	strcat(m_strConfig_file, "gleisbild.cs2");

	m_ppPage_name = read_track_file(m_strConfig_file, m_ppPage_name);

	int timeout = MAX_UDP_BCAST_RETRY;
    /* we are trying to setup a UDP socket */
	for (int i = 0; i < timeout; i++)
	{
		struct sockaddr_in *bsa;
		/* trying to get the broadcast address */
		getifaddrs(&m_pBroadcastifap);
		for (m_pBroadcastifa = m_pBroadcastifap; m_pBroadcastifa; m_pBroadcastifa = m_pBroadcastifa->ifa_next)
		{
			if (m_pBroadcastifa->ifa_addr)
			{
				if (m_pBroadcastifa->ifa_addr->sa_family == AF_INET)
				{
					bsa = (struct sockaddr_in *)m_pBroadcastifa->ifa_broadaddr;
					if (strncmp(m_pBroadcastifa->ifa_name, m_strBroadcastInterfaceName, strlen(m_strBroadcastInterfaceName)) == 0)
						m_pStrUdp_dst_address = inet_ntoa(bsa->sin_addr);
				}
			}
		}

		/* prepare udp destination struct with defaults */
		m_sBAddr.sin_family = AF_INET;
		m_sBAddr.sin_port = htons(m_iUDP_Destination_port);
		if(m_pStrUdp_dst_address)
			m_iS = inet_pton(AF_INET, m_pStrUdp_dst_address, &m_sBAddr.sin_addr);

		if ( m_iS > 0)
			break;
		sleep(1);
	}
	if (m_iS <= 0) 
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

	if (m_iVerbose && !m_iBackground)
		printf("using broadcast address %s\n", m_pStrUdp_dst_address);

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

	/* prepare TCP socket */
	m_iTCP_Socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_iTCP_Socket < 0)
	{
		fprintf(stderr, "creating TCP socket error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	m_sTCPAddr.sin_family = AF_INET;
	m_sTCPAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_sTCPAddr.sin_port = htons(m_iTCP_Local_port);
	if (bind(m_iTCP_Socket, (struct sockaddr *)&m_sTCPAddr, sizeof(m_sTCPAddr)) < 0)
	{
		fprintf(stderr, "binding TCP socket error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (listen(m_iTCP_Socket, s_iMAXPENDING) < 0)
	{
		fprintf(stderr, "starting TCP listener error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* prepare TCP clients array */
	m_iMax_tcp = -1;		/* index into tcp_client[] array */
	for (int i = 0; i < MAX_TCP_CONN; i++)
		m_iaTcp_client[i] = -1;	/* -1 indicates available entry */
	
	/* prepare second TCP socket */
	m_iTCP_Socket2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_iTCP_Socket2 < 0)
	{
		fprintf(stderr, "creating second TCP socket error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	m_sTCP2Addr.sin_family = AF_INET;
	m_sTCP2Addr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_sTCP2Addr.sin_port = htons(m_iTCP_Local_port2);
	if (bind(m_iTCP_Socket2, (struct sockaddr *)&m_sTCP2Addr, sizeof(m_sTCP2Addr)) < 0)
	{
		fprintf(stderr, "binding second TCP socket 2 error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (listen(m_iTCP_Socket2, s_iMAXPENDING) < 0)
	{
		fprintf(stderr, "starting TCP 2 listener error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
#if 0
    /* prepare TCP clients array */
    m_iMax_tcp = -1;		/* index into tcp_client[] array */
    for (i = 0; i < MAX_TCP_CONN; i++)
	tcp_client[i] = -1;	/* -1 indicates available entry */
#endif

	if ((m_iCAN_Socket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
	{
		fprintf(stderr, "CAN socket error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

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

	/* start Maerklin 60113 box */
	send_magic_start_60113_frame();

	FD_ZERO(&m_allfds);
	FD_SET(m_iCAN_Socket, &m_allfds);
	FD_SET(m_iUDP_Socket, &m_allfds);
	FD_SET(m_iTCP_Socket, &m_allfds);
	FD_SET(m_iTCP_Socket2, &m_allfds);
	m_iMax_fds = MAX(MAX(MAX(m_iCAN_Socket, m_iUDP_Socket), m_iTCP_Socket), m_iTCP_Socket2);
	
	char buffer[64];
	printf("m_strConfig_file: [%s]\n", m_strConfig_file);
	printf("m_strConfig_dir: [%s]\n", m_strConfig_dir);
	printf("m_pStrUdp_dst_address: [%s]\n", m_pStrUdp_dst_address);
	printf("m_iUDP_Destination_port: [%d]\n", m_iUDP_Destination_port);
	printf("m_iUDP_Local_port: [%d]\n", m_iUDP_Local_port);
	printf("m_iTCP_Local_port: [%d]\n", m_iTCP_Local_port);
	printf("m_iTCP_Local_port2: [%d]\n", m_iTCP_Local_port2);
	printf("m_sBAddr.sin_addr: [%s]\n", inet_ntop(AF_INET, &(m_sBAddr.sin_addr), buffer, sizeof(buffer)));
	printf("m_sBAddr.sin_port: [%d]\n", m_sBAddr.sin_port);
	printf("m_sBAddr.ntohs(sin_port): [%d]\n", ntohs(m_sBAddr.sin_port));
	printf("m_sBAddr.sin_family: [%d]\n", m_sBAddr.sin_family);
	printf("m_sBAddr.sin_zero: [%d]\n", m_sBAddr.sin_zero);
	printf("m_iS: [%d]\n", m_iS);
	printf("m_iUDPBroadcast_Socket: [%d]\n", m_iUDPBroadcast_Socket);
	printf("m_iUDP_Socket: [%d]\n", m_iUDP_Socket);
	printf("m_sSAddr.sin_addr: [%s]\n", inet_ntop(AF_INET, &(m_sSAddr.sin_addr), buffer, sizeof(buffer)));
	printf("m_sSAddr.sin_port: [%d]\n", m_sSAddr.sin_port);
	printf("m_sSAddr.ntohs(sin_port): [%d]\n", ntohs(m_sSAddr.sin_port));
	printf("m_sSAddr.sin_family: [%d]\n", m_sSAddr.sin_family);
	printf("m_sSAddr.sin_zero: [%d]\n", m_sSAddr.sin_zero);
	printf("m_iTCP_Socket: [%d]\n", m_iTCP_Socket);
	printf("m_sTCPAddr.sin_addr: [%d]\n", m_sTCPAddr.sin_addr);
	printf("m_sTCPAddr.sin_addr: [%s]\n", inet_ntop(AF_INET, &(m_sTCPAddr.sin_addr), buffer, sizeof(buffer)));
	printf("m_sTCPAddr.sin_port: [%d]\n", m_sTCPAddr.sin_port);
	printf("m_sTCPAddr.ntohs(sin_port): [%d]\n", ntohs(m_sTCPAddr.sin_port));
	printf("m_sTCPAddr.sin_family: [%d]\n", m_sTCPAddr.sin_family);
	printf("m_sTCPAddr.sin_zero: [%d]\n", m_sTCPAddr.sin_zero);
	printf("m_iTCP_Socket2: [%d]\n", m_iTCP_Socket2);
	printf("m_sTCP2Addr.sin_addr: [%s]\n", inet_ntop(AF_INET, &(m_sTCP2Addr.sin_addr), buffer, sizeof(buffer)));
	printf("m_sTCP2Addr.sin_port: [%d]\n", m_sTCP2Addr.sin_port);
	printf("m_sTCP2Addr.ntohs(sin_port): [%d]\n", ntohs(m_sTCP2Addr.sin_port));
	printf("m_sTCP2Addr.sin_family: [%d]\n", m_sTCP2Addr.sin_family);
	printf("m_sTCP2Addr.sin_zero: [%d]\n", m_sTCP2Addr.sin_zero);
	printf("m_iCAN_Socket: [%d]\n", m_iCAN_Socket);
}

CCAN2LAN::~CCAN2LAN()
{
    close(m_iCAN_Socket);
    close(m_iUDP_Socket);
    close(m_iUDPBroadcast_Socket);
    close(m_iTCP_Socket);
    close(m_iTCP_Socket2);
	free(m_ppPage_name);
}

void CCAN2LAN::send_magic_start_60113_frame()
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

    if (write(m_iCAN_Socket, &frame, sizeof(frame)) != sizeof(frame))
	{
		fprintf(stderr, "CAN magic 60113 start write error: %s\n", strerror(errno));
	}
	else
	{
		if (m_iVerbose)
			printf("CAN magic 60113 start write\n");
    }
}

void CCAN2LAN::send_can_ping_frame()
{
    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.can_id = 0x304711UL;
    /* use EFF */
    frame.can_id &= CAN_EFF_MASK;
    frame.can_id |= CAN_EFF_FLAG;
    frame.can_dlc = 0;

    if (write(m_iCAN_Socket, &frame, sizeof(frame)) != sizeof(frame))
	{
		fprintf(stderr, "CAN ping write error: %s\n", strerror(errno));
	}
	else
	{
		if (m_iVerbose)
			printf("CAN ping sent\n");
    }
}

static void DecodeFrameData(unsigned char* pframe)
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
			if(pframe[11] == 0x00 && pframe[12] == 0x33)
				printf(" (MS 2 60653, SW Version: %d.%d)", pframe[9], pframe[10]);
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

void CCAN2LAN::FindLokAddress()
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

void CCAN2LAN::DiscoverLoks()
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
void CCAN2LAN::DiscoverLoks()
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

void print_can_frame(const char *format_string, unsigned char *netframe, int verbose)
{
	uint32_t canid;
	int i, dlc;
	char timestamp[16];
	
	if (!verbose)
		return;
	
	memcpy(&canid, netframe, 4);
	dlc = netframe[4];
	time_stamp(timestamp);
	printf("%s   ", timestamp);
	printf(format_string, ntohl(canid) & CAN_EFF_MASK, netframe[4]);
	for (i = 5; i < 5 + dlc; i++)
	{
		printf(" %02x", netframe[i]);
	}
	if (dlc < 8)
	{
		printf("(%02x", netframe[i]);
		for (i = 6 + dlc; i < 13; i++)
			printf(" %02x", netframe[i]);
		printf(")");
	}
	else
		printf(" ");
	printf("  ");
	for (i = 5; i < 13; i++)
	{
		if (isprint(netframe[i]))
			printf("%c", netframe[i]);
		else
			putchar('.');
	}
	
	printf("\n");

	
	unsigned char pframe[13];
	pframe[0] = (canid >> 24) & 0x000000FF;
	pframe[1] = (canid >> 16) & 0x000000FF;
	pframe[2] = (canid >> 8) & 0x000000FF;
	pframe[3] = canid & 0x000000FF;
	pframe[4] = dlc;
	memcpy(&pframe[5], &netframe[5], dlc);
//	printf(" New CAN Frame Data: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n", pframe[0], pframe[1], pframe[2], pframe[3], pframe[4], pframe[5], pframe[6], pframe[7], pframe[8], pframe[9], pframe[10], pframe[11], pframe[12]);
	DecodeFrameData((unsigned char*)netframe);
}

int net_to_net(int net_socket, struct sockaddr *net_addr, unsigned char *netframe, int length, const char *format_string, int verbose)
{
	int iLen;
	iLen = sendto(net_socket, netframe, length, 0, net_addr, sizeof(*net_addr));
	if(verbose)
		print_can_frame(format_string, g_cNetframe, verbose);
	if (iLen != length)
	{
		fprintf(stderr, "%s: error sending TCP/UDP data; %s\n", __func__, strerror(errno));
		return -1;
	}
	return 0;
}

int frame_to_net(int net_socket, struct sockaddr *net_addr, struct can_frame *frame, const char *format_string, int verbose)
{
    int iLen;
    uint32_t canid;
	
    memset(g_cNetframe, 0, 13);
    frame->can_id &= CAN_EFF_MASK;
    canid = htonl(frame->can_id);
    memcpy(g_cNetframe, &canid, 4);
    g_cNetframe[4] = frame->can_dlc;
    memcpy(&g_cNetframe[5], &frame->data, frame->can_dlc);
	
    /* send TCP/UDP frame */
    iLen = sendto(net_socket, g_cNetframe, 13, 0, net_addr, sizeof(*net_addr));
	if(verbose)
		print_can_frame(format_string, g_cNetframe, verbose);
    if (iLen != 13)
	{
		fprintf(stderr, "%s: error sending TCP/UDP data %s\n", __func__, strerror(errno));
		return -1;
    }
    return 0;
}

/* it seems Gleisbox needs a short break after every CAN message */
/* use 20ms delay between two consequences CAN frames */
#define TIME_WAIT_US	20 * 1000

int frame_to_can(int can_socket, unsigned char *netframe, const char *format_string, int verbose)
{
    uint32_t canid;
    struct can_frame frame;
    struct timespec to_wait;
    struct timeval actual_time;
    long usec;
	
    /* Maerklin TCP/UDP Format: always 13 bytes
	*   byte 0 - 3  CAN ID
	*   byte 4      DLC
	*   byte 5 - 12 CAN data
	*/
    memset(&frame, 0, sizeof(frame));
    memcpy(&canid, netframe, 4);
    /* CAN uses (network) big endian format */
    frame.can_id = ntohl(canid);
    frame.can_id &= CAN_EFF_MASK;
    frame.can_id |= CAN_EFF_FLAG;
    frame.can_dlc = netframe[4];
    memcpy(&frame.data, &netframe[5], 8);
	
    /* we calculate the difference between the actual time and the time the last command was sent */
    /* probably we don't need to wait anymore before putting next CAN frame on the wire */
    gettimeofday(&actual_time, NULL);
    usec  =(actual_time.tv_sec  - g_tvLast_senttime_CAN.tv_sec)  * 1000000;
    usec +=(actual_time.tv_usec - g_tvLast_senttime_CAN.tv_usec);
    if (usec < TIME_WAIT_US)
	{
		to_wait.tv_sec = 0;
		to_wait.tv_nsec = (TIME_WAIT_US - usec) * 1000 ;
		nanosleep(&to_wait, NULL);
    }
	
	if(verbose)
		print_can_frame(format_string, netframe, verbose);
    /* send CAN frame */
    if (write(can_socket, &frame, sizeof(frame)) != sizeof(frame))
	{
		fprintf(stderr, "%s: error writing CAN frame: %s\n", __func__, strerror(errno));
		return -1;
    }
	
    gettimeofday(&g_tvLast_senttime_CAN, NULL);
    return 0;
}

static void strm_init(z_stream * strm)
{
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    CALL_ZLIB(deflateInit2(strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY));
}

uint8_t *read_config_file(const char *filename, char *config_dir, uint32_t * nbytes)
{
    int rc;
    struct stat st;
    FILE *fp;
    uint8_t *config;
    char *file_name;
	
    file_name = (char *)calloc(MAXLINE, 1);
    if (!file_name)
		return NULL;
	
    strncat(file_name, config_dir, MAXLINE-1);
    strcat(file_name, filename);
	
    printf("%s: try reading file %s\n", __func__, file_name);
	
    rc = stat(file_name, &st);
    if (rc < 0)
	{
		fprintf(stderr, "%s: error stat failed for file %s\n", __func__, filename);
		goto read_error1;
    }
    fp = fopen(file_name, "rb");
    if (fp == NULL)
	{
		fprintf(stderr, "%s: error fopen failed for file %s\n", __func__, filename);
		goto read_error1;
    }
    *nbytes = st.st_size;
    config = (uint8_t *) calloc(*nbytes, sizeof(uint8_t));
    if (config == NULL)
	{
		fprintf(stderr, "%s: error calloc failed creating config buffer for %s\n", __func__, filename);
		goto read_error2;
    }
    rc = fread((void *)config, 1, *nbytes, fp);
    if (((unsigned int)rc != *nbytes))
	{
		fprintf(stderr, "%s: error fread failed reading %s\n", __func__, filename);
        free(config);
		goto read_error2;
    }
    fclose(fp);
    free(file_name);
    return config;
	
read_error2:
    fclose(fp);
read_error1:
    free(file_name);
    return NULL;
}

int send_tcp_config_data(const char *filename, char *config_dir, uint32_t canid, int tcp_socket, int flags)
{
    /* uint16_t crc; */
    uint32_t temp32, canid_be, nbytes = 0;
    uint8_t *config;
    uint8_t *out;
    z_stream strm;
    int inflated_size, deflated_size, padded_nbytes, i, src_i, n_packets;
    uint16_t crc, temp16;
    uint8_t netframe[MAXMTU];
	
    config = read_config_file(filename, config_dir, &nbytes);
    if (config)
	{
		printf("%s read config file %s\n", __func__, filename);
    }
	else
	{
		printf("%s: error reading config %s\n", __func__, filename);
		return -1;
    }
	
    if (flags & COMPRESSED)
	{
		/* we need some more bytes to prepare send data (includes inflated file size and padding)    */
		/* assuming that out[CHUNK] is large enough to compress the whole file, otherwise don't send */
		out = (uint8_t *) calloc(CHUNK + 12, sizeof(uint8_t));
		if (out == NULL)
		{
			printf("%s: error calloc failed creating deflation buffer\n", __func__);
			return -1;
		}
		strm_init(&strm);
		strm.next_in = config;
		strm.avail_in = nbytes;
		strm.avail_out = CHUNK;
		/* store deflated file beginning at byte 5 */
		strm.next_out = &out[4];
		CALL_ZLIB(deflate(&strm, Z_FINISH));
		deflated_size = CHUNK - strm.avail_out;
		if (strm.avail_out == 0)
		{
			printf("%s: compressed file to large : %d filesize %d strm.avail_out\n", __func__, nbytes, strm.avail_out);
			deflateEnd(&strm);
			free(config);
			free(out);
			return -1;
		}
		
		/* now prepare the send buffer */
		inflated_size = htonl(nbytes);
		memcpy(out, &inflated_size, 4);
		/* prepare padding */
		padded_nbytes = deflated_size + 4;
		if (padded_nbytes % 8)
		{
			padded_nbytes += 8 - (padded_nbytes % 8);
		}
		
		for (i = deflated_size + 4; i < padded_nbytes; i++)
		{
			out[i] = 0;
		}
		
		crc = CRCCCITT(out, padded_nbytes, 0xffff);
		printf("%s: canid 0x%08x filesize %d deflated size: %d crc 0x%04x\n", __func__, canid, nbytes, deflated_size,
			crc);
		memset(netframe, 0, MAXMTU);
		/* prepare first CAN frame   */
		/* delete response bit and set canid to config data stream */
		canid_be = htonl((canid & 0xFFFEFFFFUL) | 0x00020000UL);
		memcpy(&netframe[0], &canid_be, 4);
		/* CAN DLC is 6 */
		netframe[4] = 0x06;
		temp32 = htonl(deflated_size + 4);
		memcpy(&netframe[5], &temp32, 4);
		temp16 = htons(crc);
		memcpy(&netframe[9], &temp16, 2);
		netframe[11] = 0x00;
		netframe[12] = 0x00;
		
		if (net_to_net(tcp_socket, NULL, netframe, 13, NULL, false))
		{
			deflateEnd(&strm);
			free(config);
			free(out);
			return -1;
		}
		
		/* loop until all packets send */
		src_i = 0;
		do
		{
			n_packets = 0;
			i = 0;
			do
			{
				memcpy(&netframe[i], &canid_be, 4);
				i += 4;
				/* CAN DLC is always 8 */
				netframe[i] = 0x08;
				i++;
				memcpy(&netframe[i], &out[src_i], 8);
				i += 8;
				src_i += 8;
				n_packets++;
			}
			while ((src_i < padded_nbytes) && n_packets < MAX_PACKETS);
			/* don't use frame_to_net because we have more then 13 bytes to send */
			if (net_to_net(tcp_socket, NULL, netframe, i, NULL, false))
			{
				perror("error sending TCP data\n");
				deflateEnd(&strm);
				free(config);
				free(out);
				return -1;
			}
		} while (src_i < padded_nbytes);
#if 0
		/* print compressed data */
		temp32 = i;
		for (i = 0; i < temp32; i++)
		{
			if ((i % 13) == 0)
			{
				printf("\n");
			}
			printf("%02x ", netframe[i]);
		}
		printf("\n");
#endif
		deflateEnd(&strm);
    }
    free(config);
    free(out);
    return 0;
}

int check_data(int tcp_socket, unsigned char *netframe, char *config_dir, char **page_name)
{
	bool ms1_workaround = true;
#if 1
	uint32_t canid;
	char config_name[9];
	char gbs_name[MAXLINE];
	int ret;
	gbs_name[0] = '\0';
	
	ret = 0;
	memcpy(&canid, netframe, 4);
	canid = ntohl(canid);
	switch (canid & 0xFFFF0000UL)
	{
	case (0x00400000UL):	/* config data */
		/* mark frame to send over CAN */
		ret = 0;
		/* check for special copy request */
		if (canid == 0x0040affe)
		{
			ret = 1;
			netframe[1] |= 1;
			netframe[4] = 4;
			strcpy((char *)&netframe[5], "copy");
			net_to_net(tcp_socket, NULL, netframe, 13, NULL, false);
	//		if (verbose)
				printf("CS2 copy request\n");
		}
		else
		{
			strncpy(config_name, (char *)&netframe[5], 8);
			config_name[8] = '\0';
			printf("%s ID 0x%08x %s\n", __func__, canid, (char *)&netframe[5]);
			netframe[1] |= 1;
			net_to_net(tcp_socket, NULL, netframe, 13, NULL, false);
			if (strcmp("loks", config_name) == 0)
			{
				ret = 1;
				send_tcp_config_data("lokomotive.cs2", config_dir, canid, tcp_socket, CRC | COMPRESSED);
				break;
			}
			else if (strcmp("mags", config_name) == 0)
			{
				ret = 1;
				send_tcp_config_data("magnetartikel.cs2", config_dir, canid, tcp_socket, CRC | COMPRESSED);
				break;
			}
			else if (strncmp("gbs-", config_name, 4) == 0)
			{
				int page_number;
				ret = 1;
				page_number = atoi(&config_name[5]);
				strcat(gbs_name, "gleisbilder/");
				if (page_name)
				{
					strcat(gbs_name, page_name[page_number]);
					strcat(gbs_name, ".cs2");
					send_tcp_config_data(gbs_name, config_dir, canid, tcp_socket, CRC | COMPRESSED);
				}
				break;
			}
			else if (strcmp("gbs", config_name) == 0)
			{
				ret = 1;
				send_tcp_config_data("gleisbild.cs2", config_dir, canid, tcp_socket, CRC | COMPRESSED);
				break;
			}
			else if (strcmp("fs", config_name) == 0)
			{
				ret = 1;
				send_tcp_config_data("fahrstrassen.cs2", config_dir, canid, tcp_socket, CRC | COMPRESSED);
				break;
			}
			/* TODO : these files depends on different internal states */
			else if (strcmp("lokstat", config_name) == 0)
			{
				ret = 1;
				fprintf(stderr, "%s: lokstat (lokomotive.sr2) not implemented yet\n", __func__);
				send_tcp_config_data("lokomotive.sr2", config_dir, canid, tcp_socket, CRC | COMPRESSED);
				break;
			}
			else if (strcmp("magstat", config_name) == 0)
			{
				ret = 1;
				fprintf(stderr, "%s: magstat (magnetartikel.sr2) not implemented yet\n\n", __func__);
				send_tcp_config_data("magnetartikel.sr2", config_dir, canid, tcp_socket, CRC | COMPRESSED);
				break;
			}
			else if (strcmp("gbsstat", config_name) == 0)
			{
				ret = 1;
				fprintf(stderr, "%s: gbsstat (gbsstat.sr2) not implemented yet\n\n", __func__);
				send_tcp_config_data("gbsstat.sr2", config_dir, canid, tcp_socket, CRC | COMPRESSED);
				break;
			}
			else if (strcmp("fsstat", config_name) == 0)
			{
				ret = 1;
				fprintf(stderr, "%s: fsstat (fahrstrassen.sr2) not implemented yet\n\n", __func__);
				send_tcp_config_data("fahrstrassen.sr2", config_dir, canid, tcp_socket, CRC | COMPRESSED);
				break;
			}
			break;
		}
		/* fake cyclic MS1 slave monitoring response */
	case (0x0C000000UL):
		/* mark CAN frame to send */
		ret = 0;
		if (ms1_workaround)
			netframe[5] = 0x03;
		break;
	}
	return ret;
#endif
}

void CCAN2LAN::HandleEvents()
{
	char buffer[64];
	
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 50000;	//50.000 microseconds -> 50 Milliseconds

//	printf("HandleEvents()\n");

	m_readfds = m_allfds;
//	if (select((m_iCAN_Socket > m_iUDP_Socket) ? m_iCAN_Socket + 1 : m_iUDP_Socket + 1, &m_readfds, NULL, NULL, &timeout) < 0)
	
//	printf("HandleEvents() Waiting for event\n");
	m_iSelectRet = select(m_iMax_fds + 1, &m_readfds, NULL, NULL, &timeout);
//	printf("HandleEvents() m_iSelectRet: %d\n", m_iSelectRet);

	if(m_iSelectRet == 0)
		return;

	if(m_iSelectRet < 0)
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
		{
			frame_to_net(m_iUDPBroadcast_Socket, (struct sockaddr *)&m_sBAddr, (struct can_frame *)&m_CanFrame, UDP_FORMAT_STRG, m_iVerbose && !m_iBackground);

			for (int i = 0; i <= m_iMax_tcp; i++)
			{	/* check all clients for data */
				m_iTCP_Socket3 = m_iaTcp_client[i];
				if (m_iTCP_Socket3 < 0)
					continue;
				frame_to_net(m_iTCP_Socket3, (struct sockaddr *)&m_sTCPAddr, (struct can_frame *)&m_CanFrame, CAN_TCP_FORMAT_STRG, m_iVerbose && !m_iBackground);
			}
	
//			printf("sending can frame to ip: %s, port %d\n", inet_ntop(AF_INET, &(m_sBAddr.sin_addr), buffer, sizeof(buffer)), ntohs(m_sBAddr.sin_port));
		}
	}

	/* received a UDP packet */
	if (FD_ISSET(m_iUDP_Socket, &m_readfds))
	{
//		printf("received a UDP packet\n");
		if (read(m_iUDP_Socket, m_udpframe, MAXDG) == 13)
		{
			frame_to_can(m_iCAN_Socket, m_udpframe, NET_UDP_FORMAT_STRG, m_iVerbose && !m_iBackground);

			memcpy(&m_iCanid, &m_udpframe[0], 4);
			/* CAN is stored in network Big Endian format */
			m_CanFrame.can_id = ntohl(m_iCanid);
			m_CanFrame.can_id &= CAN_EFF_MASK;
			m_CanFrame.can_id |= CAN_EFF_FLAG;
			/* answer to CAN ping from LAN to LAN */
			if (((m_CanFrame.can_id & 0x00FF0000UL) == 0x00310000UL) &&
				(m_udpframe[11] = 0xEE) && (m_udpframe[12] = 0xEE))
			{
				if (m_iVerbose && !m_iBackground)
					printf("  received CAN ping\n");
				memcpy(m_udpframe_reply, m_udpframe, 13);
				m_udpframe_reply[0] = 0x00;
				m_udpframe_reply[1] = 0x30;
				m_udpframe_reply[2] = 0x00;
				m_udpframe_reply[3] = 0x00;
				m_udpframe_reply[4] = 0x00;

				if (net_to_net(m_iUDPBroadcast_Socket, (struct sockaddr *)&m_sBAddr, m_udpframe_reply, 13, NET_UDP_FORMAT_STRG, m_iVerbose && !m_iBackground))
				{
					fprintf(stderr, "sending UDP data (CAN Ping) error:%s \n", strerror(errno));
				}
				else
				{
					if (m_iVerbose && !m_iBackground)
						printf("                replied CAN ping\n");
				}
			}
		}
	}

	int i;
	/* received a TCP packet */
	if (FD_ISSET(m_iTCP_Socket, &m_readfds))
	{
		m_iConn_fd = accept(m_iTCP_Socket, (struct sockaddr *)&m_sTCPAddr, &m_sTCPAddrlen);
		if (m_iVerbose && !m_iBackground)
		{
			printf("1: new client: %s, port %d conn fd: %d max fds: %d on Socket: %d\n", inet_ntop(AF_INET, &(m_sTCPAddr.sin_addr),
				buffer, sizeof(buffer)), ntohs(m_sTCPAddr.sin_port), m_iConn_fd, m_iMax_fds, m_iTCP_Socket);
		}
		for (i = 0; i < MAX_TCP_CONN; i++)
		{
			if (m_iaTcp_client[i] < 0)
			{
				m_iaTcp_client[i] = m_iConn_fd;	/* save new TCP client descriptor */
				break;
			}
		}
		if (i == MAX_TCP_CONN)
			fprintf(stderr, "too many TCP clients\n");
		
		FD_SET(m_iConn_fd, &m_allfds);	/* add new descriptor to set */
		m_iMax_fds = MAX(m_iConn_fd, m_iMax_fds);	/* for select */
		m_iMax_tcp = MAX(i, m_iMax_tcp);	/* max index in tcp_client[] array */
		if (--m_iSelectRet <= 0)
			return;	/* no more readable descriptors */
	}

	/* received a packet on second TCP port */
	if (FD_ISSET(m_iTCP_Socket2, &m_readfds))
	{
		m_iConn_fd = accept(m_iTCP_Socket2, (struct sockaddr *)&m_sTCP2Addr, &m_sTCPAddrlen);
		
		/* TODO : close missing */
		if (m_iVerbose && !m_iBackground)
		{
			printf("2: new client: %s, port %d conn fd: %d max fds: %d\n", inet_ntop(AF_INET, &(m_sTCP2Addr.sin_addr),
				buffer, sizeof(buffer)), ntohs(m_sTCP2Addr.sin_port), m_iConn_fd, m_iMax_fds);
		}
		FD_SET(m_iConn_fd, &m_allfds);	/* add new descriptor to set */
		m_iMax_fds = MAX(m_iConn_fd, m_iMax_fds);	/* for select */
		m_iMax_tcp = MAX(i, m_iMax_tcp);	/* max index in tcp_client[] array */
	}

	char timestamp[16];
	/* check for already connected TCP clients */
	for (int i = 0; i <= m_iMax_tcp; i++)
	{	/* check all clients for data */
		m_iTCP_Socket3 = m_iaTcp_client[i];
		if (m_iTCP_Socket3 < 0)
			continue;
		/* printf("%s tcp packet received from client #%d  m_iMax_tcp:%d todo:%d\n", time_stamp(timestamp), i, m_iMax_tcp,nready); */
		if (FD_ISSET(m_iTCP_Socket3, &m_readfds))
		{
			if (m_iVerbose && !m_iBackground)
			{
				time_stamp(timestamp);
				printf("%s packet from: %s\n", timestamp, inet_ntop(AF_INET, &m_sTCPAddr.sin_addr, buffer, sizeof(buffer)));
			}
			int n = read(m_iTCP_Socket3, g_cNetframe, MAXDG);
			if (!n)
			{
				/* connection closed by client */
				if (m_iVerbose && !m_iBackground)
				{
					time_stamp(timestamp);
					printf("%s client %s closed connection\n", timestamp,
						inet_ntop(AF_INET, &m_sTCPAddr.sin_addr, buffer, sizeof(buffer)));
				}
				close(m_iTCP_Socket3);
				FD_CLR(m_iTCP_Socket3, &m_allfds);
				m_iaTcp_client[i] = -1;
			}
			else
			{
				/* check the whole TCP packet, if there are more than one CAN frame included */
				/* TCP packets with size modulo 13 !=0 are ignored though */
				if (n % 13)
				{
					time_stamp(timestamp);
					fprintf(stderr, "%s received packet %% 13 : length %d\n", timestamp, n);
				}
				else
				{
					for (int j = 0; j < n; j += 13)
					{
						/* check if we need to forward the message to CAN */
						if (!check_data(m_iTCP_Socket3, &g_cNetframe[j], m_strConfig_dir, m_ppPage_name))
						{
							frame_to_can(m_iCAN_Socket, &g_cNetframe[j], (j > 0 ? TCP_FORMATS_STRG : TCP_FORMAT_STRG), m_iVerbose && !m_iBackground);
							net_to_net(m_iUDPBroadcast_Socket, (struct sockaddr *)&m_sBAddr, g_cNetframe, 13, UDP_FORMAT_STRG, m_iVerbose && !m_iBackground);
						}
					}
				}
			}
			if (--m_iSelectRet <= 0)
				break;	/* no more readable descriptors */
		}
	}
}

void CCAN2LAN::send_ping_frame()
{
	send_can_ping_frame();
//	frame_to_can(m_iCAN_Socket, &g_cNetframe[j], (j > 0 ? TCP_FORMATS_STRG : TCP_FORMAT_STRG), m_iVerbose && !m_iBackground);
	memset(g_cNetframe, 0, 13);
	g_cNetframe[0] = 0x00;
	g_cNetframe[1] = 0x30;
	g_cNetframe[2] = 0x47;
	g_cNetframe[3] = 0x11;
	g_cNetframe[4] = 0x00;
	net_to_net(m_iUDPBroadcast_Socket, (struct sockaddr *)&m_sBAddr, g_cNetframe, 13, UDP_FORMAT_STRG, m_iVerbose && !m_iBackground);
}

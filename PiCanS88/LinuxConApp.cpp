// LinuxConApp.cpp: Implementierung der Klasse LinuxConApp.
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

#include "LinuxConApp.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CLinuxConApp::CLinuxConApp()
{
	m_iOpt_Verbose = 0;
	m_iOpt_Background = 1;
	m_iOpt_ModulCount = 1;
	m_iOpt_SearchLoco = false;
	m_iOpt_SearchLocoStart = 1;
	m_iOpt_SearchLocoEnd = 255;
	m_iOpt_SearchLocoValue = 1;
	m_iOpt_Use_cs2_conf = false;
	m_iOpt_Ms1_workaround = false;

	m_iSensorOffset = 0;
	strcpy(m_Destip, "127.0.0.1");
	m_iUDP_Destination_port = 15730;
	m_iUDP_Local_port = 15731;
    strcpy(m_cRocrail_server, "255.255.255.255");

	/* prepare udp destination struct with defaults */
    m_sBAddr.sin_family = AF_INET;
    m_sBAddr.sin_port = htons(m_iUDP_Destination_port);
	m_iS = inet_pton(AF_INET, m_cRocrail_server, &m_sBAddr.sin_addr);

	memset(&m_ifr, 0, sizeof(m_ifr));
    strcpy(m_ifr.ifr_name, "can0");

	/* setup udp socket for S88 */
	bzero(&m_Destaddr,sizeof(m_Destaddr));
	m_Destaddr.sin_family = AF_INET;
	m_Destaddr.sin_port = htons(m_iUDP_Destination_port);
	if(inet_pton(AF_INET, m_Destip, &m_Destaddr.sin_addr) < 0)
	{
		perror("inet_pton");
		exit(1);
	}
}

CLinuxConApp::~CLinuxConApp()
{

}

void CLinuxConApp::ShowUsage()
{
	fprintf(stderr, "\nUsage: PiCanS88 -vf [-d <RocrailIP>][-l <port>][-p <port>][-m <s88modules>][-o <offset>][-i <CANInterface>]\n");
	fprintf(stderr, "   Version 1.0\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Search for Loco address:\n");
	fprintf(stderr, "   PiCanS88 -t -s 1 -e 255\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "     -d <RocrailIP>      IP Address of Rocrail server - default 127.0.0.1\n");
    fprintf(stderr, "     -b <broadcast_addr> Broadcast address - default 255.255.255.255\n");
    fprintf(stderr, "     -l <port>           Listening UDP port for Rocrail server - default 15731\n");
	fprintf(stderr, "     -p <port>           Destination UDP port of Rocrail server - default 15730\n");
    fprintf(stderr, "     -i <can int>        CAN interface - default can0\n");
	fprintf(stderr, "     -m <s88modules>     Number of connected S88 modules - default 1\n");
	fprintf(stderr, "     -o <offset>         Number of S88 modules to skip in addressing - default 0\n");
	fprintf(stderr, "     -f                  Run in foreground (for debugging)\n");
	fprintf(stderr, "     -v                  Be verbose\n");
	fprintf(stderr, "     -t                  Test for LocoAddress\n");
	fprintf(stderr, "     -s <start>          Start address for test for LocoAddress\n");
	fprintf(stderr, "     -e <end>            End address for test for LocoAddress\n");
	fprintf(stderr, "     -w <value>          1=On/0=Off test for LocoAddress\n");
	fprintf(stderr, "\n");
}

bool CLinuxConApp::GetOpts(int argc, char **argv)
{
	int opt, ret;
	
	while ((opt = getopt(argc, argv, "w:s:e:l:b:i:d:p:m:o:fvt?")) != -1) 
	{
		switch (opt) 
		{
		case 'l':
			m_iUDP_Local_port = strtoul(optarg, (char **)NULL, 10);
			break;
		case 'b':
			m_iS = inet_pton(AF_INET, optarg, &m_sBAddr.sin_addr);
			if (m_iS <= 0)
			{
				if (m_iS == 0)
				{
					fprintf(stderr, "invalid IP address: %s\n", strerror(errno));
				}
				else
				{
					fprintf(stderr, "inet_pton error: %s\n", strerror(errno));
				}
				exit(EXIT_FAILURE);
			}
			break;
		case 'i':
			strncpy(m_ifr.ifr_name, optarg, sizeof(m_ifr.ifr_name) - 1);
			break;

		case 'p':
			m_iUDP_Destination_port = strtoul(optarg, (char **)NULL, 10);
			m_Destaddr.sin_port = htons(m_iUDP_Destination_port);
			break;
		case 'd':
			ret = inet_pton(AF_INET,optarg,&m_Destaddr.sin_addr);
			if (ret <= 0)
			{
                if (ret == 0)
				{
					m_pHp = gethostbyname(optarg);
					if ( !m_pHp )
					{
						fprintf(stderr, "s88udp: unknown host %s\n", optarg);
						return false;
					} 
					memcpy((void *)&m_Destaddr.sin_addr, m_pHp->h_addr_list[0], m_pHp->h_length);
                }
				else
				{
					perror("inet_pton");
					return false;
                }
			}
			break;
		case 'm':
			m_iOpt_ModulCount = atoi(optarg);
			if ( m_iOpt_ModulCount < 1 || m_iOpt_ModulCount > MAXMODULES )
			{
                ShowUsage();
				return false;
			}
			break;
		case 'o':
			m_iSensorOffset = atoi(optarg);
			if ( m_iSensorOffset >= MAXMODULES )
			{
                ShowUsage();
				return false;
			}
			break;
		case 's':
			m_iOpt_SearchLocoStart = atoi(optarg);
			if ( m_iOpt_SearchLocoStart >= 255 )
			{
                ShowUsage();
				return false;
			}
			break;
		case 'e':
			m_iOpt_SearchLocoEnd = atoi(optarg);
			if ( m_iOpt_SearchLocoEnd >= 255 )
			{
                ShowUsage();
				return false;
			}
			break;
		case 'w':
			m_iOpt_SearchLocoValue = atoi(optarg);
			if ( m_iOpt_SearchLocoValue > 1 )
			{
                ShowUsage();
				return false;
			}
			break;
		case 'v':
			m_iOpt_Verbose = 1;
			break;
		case 'f':
			m_iOpt_Background = 0;
			break;
		case 't':
			m_iOpt_SearchLoco = true;
			m_iOpt_Verbose = 1;
			m_iOpt_Background = 0;
			break;
		case 'h':
		case '?':
			ShowUsage();
			return false;
			
		default:
			ShowUsage();
			return false;
		}
	}

	return true;
}

void CLinuxConApp::BackgroundCheck()
{
	if (m_iOpt_Background) 
	{
		pid_t pid;
		
		/* Fork off the parent process */
		pid = fork();
		if (pid < 0) 
		{
			exit(EXIT_FAILURE);
		}
		/* If we got a good PID, then we can exit the parent process. */
		if (pid > 0) 
		{
			if ( m_iOpt_Verbose )
				printf("Going into background ...\n");
			exit(0);
		}
	}
}

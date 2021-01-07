// LinuxConApp.h: Schnittstelle für die Klasse LinuxConApp.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LINUXCONAPP_H__D8FADE44_EEA9_4D09_A903_96A06BF08B21__INCLUDED_)
#define AFX_LINUXCONAPP_H__D8FADE44_EEA9_4D09_A903_96A06BF08B21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAXMODULES 64

class CLinuxConApp  
{
public:
	CLinuxConApp();
	virtual ~CLinuxConApp();

	int m_iOpt_Verbose;
	int m_iOpt_Background;
	int m_iOpt_ModulCount;
	int m_iOpt_SearchLoco;
	int m_iOpt_SearchLocoStart;
	int m_iOpt_SearchLocoEnd;
	int m_iOpt_SearchLocoValue;
	int m_iOpt_Use_cs2_conf;
	int m_iOpt_Ms1_workaround;

	int m_iSensorOffset;
	
	int m_iUdpsock;
	struct hostent *m_pHp;
	struct sockaddr_in m_Destaddr;
	char m_Destip[16];
	int m_iUDP_Destination_port;
	int m_iUDP_Local_port;

    char m_cRocrail_server[16];
	int m_iS;
	struct ifreq m_ifr;
	struct sockaddr_in m_sBAddr;

	void ShowUsage();
	bool GetOpts(int argc, char **argv);
	void BackgroundCheck();

};

#endif // !defined(AFX_LINUXCONAPP_H__D8FADE44_EEA9_4D09_A903_96A06BF08B21__INCLUDED_)

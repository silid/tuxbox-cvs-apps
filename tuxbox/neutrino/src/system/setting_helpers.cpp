/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "global.h"
#include "neutrino.h"
#include "setting_helpers.h"

#include "libnet.h"
#include "helpers/streaminfo.h"
#include "gui/widget/messagebox.h"
#include "libucodes/libucodes.h"


extern "C" int pinghost( const char *hostname );

bool CSatDiseqcNotifier::changeNotify(string OptionName, void* Data)
{
	if( *((int*) Data)==0)   // diseqc off
	{
		satMenu->setActive(true);
		extMenu->setActive(false);
		repeatMenu->setActive(false);
	}
	else
	{
		satMenu->setActive(false);
		extMenu->setActive(true);
		repeatMenu->setActive(true);
	}
}


bool CCableSpectalInversionNotifier::changeNotify(string OptionName, void* Data)
{
	static bool messageShowed = false;

	if (!messageShowed)
	{
		ShowMsg ( "messagebox.info", g_Locale->getText("cablesetup.spectralInversionWarning"), CMessageBox::mbrYes, CMessageBox::mbYes, "info.raw");
		messageShowed = true;
	}

	if( *((int*) Data)!=0)
	{	//file anlegen (direktstart)
		FILE* fd = fopen("/var/etc/.specinv", "w");
		if(fd)
		{
			fclose(fd);
		}
	}
	else
	{
		remove("/var/etc/.specinv");
	}
}

bool CStartNeutrinoDirectNotifier::changeNotify(string OptionName, void* Data)
{
	if( *((int*) Data)!=0)
	{	//file anlegen (direktstart)
		FILE* fd = fopen("/var/etc/.neutrino", "w");
		if(fd)
		{
			fclose(fd);
		}
	}
	else
	{
		remove("/var/etc/.neutrino");
	}
}

bool CIPChangeNotifier::changeNotify(string OptionName, void* Data)
{
	int ip_1, ip_2, ip_3, ip_4;
	sscanf( (char*) Data, "%d.%d.%d.%d", &ip_1, &ip_2, &ip_3, &ip_4 );
	sprintf( g_settings.network_broadcast, "%d.%d.%d.255", ip_1, ip_2, ip_3 );
}

bool CColorSetupNotifier::changeNotify(string OptionName, void*)
{
	CFrameBuffer *frameBuffer = CFrameBuffer::getInstance();
	unsigned char r,g,b;
	//setting colors-..
	frameBuffer->paletteGenFade(COL_MENUHEAD,
	                              convertSetupColor2RGB(g_settings.menu_Head_red, g_settings.menu_Head_green, g_settings.menu_Head_blue),
	                              convertSetupColor2RGB(g_settings.menu_Head_Text_red, g_settings.menu_Head_Text_green, g_settings.menu_Head_Text_blue),
	                              8, convertSetupAlpha2Alpha( g_settings.menu_Head_alpha ) );

	frameBuffer->paletteGenFade(COL_MENUCONTENT,
	                              convertSetupColor2RGB(g_settings.menu_Content_red, g_settings.menu_Content_green, g_settings.menu_Content_blue),
	                              convertSetupColor2RGB(g_settings.menu_Content_Text_red, g_settings.menu_Content_Text_green, g_settings.menu_Content_Text_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.menu_Content_alpha) );


	frameBuffer->paletteGenFade(COL_MENUCONTENTDARK,
	                              convertSetupColor2RGB(int(g_settings.menu_Content_red*0.6), int(g_settings.menu_Content_green*0.6), int(g_settings.menu_Content_blue*0.6)),
	                              convertSetupColor2RGB(g_settings.menu_Content_Text_red, g_settings.menu_Content_Text_green, g_settings.menu_Content_Text_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.menu_Content_alpha) );

	frameBuffer->paletteGenFade(COL_MENUCONTENTSELECTED,
	                              convertSetupColor2RGB(g_settings.menu_Content_Selected_red, g_settings.menu_Content_Selected_green, g_settings.menu_Content_Selected_blue),
	                              convertSetupColor2RGB(g_settings.menu_Content_Selected_Text_red, g_settings.menu_Content_Selected_Text_green, g_settings.menu_Content_Selected_Text_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.menu_Content_Selected_alpha) );

	frameBuffer->paletteGenFade(COL_MENUCONTENTINACTIVE,
	                              convertSetupColor2RGB(g_settings.menu_Content_inactive_red, g_settings.menu_Content_inactive_green, g_settings.menu_Content_inactive_blue),
	                              convertSetupColor2RGB(g_settings.menu_Content_inactive_Text_red, g_settings.menu_Content_inactive_Text_green, g_settings.menu_Content_inactive_Text_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.menu_Content_inactive_alpha) );

	frameBuffer->paletteGenFade(COL_INFOBAR,
	                              convertSetupColor2RGB(g_settings.infobar_red, g_settings.infobar_green, g_settings.infobar_blue),
	                              convertSetupColor2RGB(g_settings.infobar_Text_red, g_settings.infobar_Text_green, g_settings.infobar_Text_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.infobar_alpha) );

/*	frameBuffer->paletteSetColor( COL_INFOBAR_SHADOW,
	                                convertSetupColor2RGB(
	                                    int(g_settings.infobar_red*0.4),
	                                    int(g_settings.infobar_green*0.4),
	                                    int(g_settings.infobar_blue*0.4)),
	                                g_settings.infobar_alpha);
*/
	frameBuffer->paletteGenFade(COL_INFOBAR_SHADOW,
	                              convertSetupColor2RGB(int(g_settings.infobar_red*0.4), int(g_settings.infobar_green*0.4), int(g_settings.infobar_blue*0.4)),
	                              convertSetupColor2RGB(g_settings.infobar_Text_red, g_settings.infobar_Text_green, g_settings.infobar_Text_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.infobar_alpha) );


	frameBuffer->paletteSet();
	return false;
}

bool CAudioSetupNotifier::changeNotify(string OptionName, void*)
{
	//printf("notify: %s\n", OptionName.c_str() );

	if(OptionName=="audiomenu.analogout")
	{
		g_Controld->setAnalogOutput(g_settings.audio_AnalogMode);
	}
	return false;
}

bool CVideoSetupNotifier::changeNotify(string OptionName, void*)
{
	if(OptionName=="videomenu.videosignal")
	{
		g_Controld->setVideoOutput( g_settings.video_Signal );
	}
	else if(OptionName=="videomenu.videoformat")
	{
		g_Controld->setVideoFormat( g_settings.video_Format );
	}

	printf("video notify: %s\n", OptionName.c_str() );
	return false;
}

bool CLanguageSetupNotifier::changeNotify(string OptionName, void*)
{
	//	printf("language notify: %s - %s\n", OptionName.c_str(), g_settings.language );
	g_Locale->loadLocale(g_settings.language);
	return true;
}

bool CKeySetupNotifier::changeNotify(string OptionName, void*)
{
	//    printf("CKeySetupNotifier notify: %s\n", OptionName.c_str() );
	g_RCInput->repeat_block = atoi(g_settings.repeat_blocker)* 1000;
	g_RCInput->repeat_block_generic = atoi(g_settings.repeat_genericblocker)* 1000;
	return false;
}

int CAPIDChangeExec::exec(CMenuTarget* parent, string actionKey)
{
	//    printf("CAPIDChangeExec exec: %s\n", actionKey.c_str());
	int sel= atoi(actionKey.c_str());
	if (g_RemoteControl->current_PIDs.PIDs.selected_apid!= sel )
	{
		g_RemoteControl->setAPID(sel);
	}
	return menu_return::RETURN_EXIT;
}


void showSubchan(string subChannelName)
{
	CFrameBuffer *frameBuffer = CFrameBuffer::getInstance();
	CNeutrinoApp *neutrino = CNeutrinoApp::getInstance();

	if(subChannelName!="")
	{
		int dx = g_Fonts->infobar_info->getRenderWidth(subChannelName.c_str()) + 20;
		int dy = 30;
		int x = g_settings.screen_EndX - dx -10;
		int y = g_settings.screen_StartY + 10;
		frameBuffer->paintBoxRel(x,y, dx,dy, COL_MENUCONTENT);
		g_Fonts->infobar_info->RenderString(x+10, y+30, dx-20, subChannelName.c_str(), COL_MENUCONTENT);

		unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( 2 );
		int res = messages_return::none;
		uint msg; uint data;

		while ( ! ( res & ( messages_return::cancel_info | messages_return::cancel_all ) ) )
		{
			g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );
			res = neutrino->handleMsg( msg, data );

			if ( res == messages_return::unhandled )
			{
				// raus hier und im Hauptfenster behandeln...
				g_RCInput->postMsg(  msg, data );
				res = messages_return::cancel_info;
			}
			struct timeval tv;
			gettimeofday( &tv, NULL );
			long long timeNow = (long long) tv.tv_usec + (long long)((long long) tv.tv_sec * (long long) 1000000);
			if(timeNow>timeoutEnd)
			{
				res = messages_return::cancel_info;
			}
		}
		frameBuffer->paintBackgroundBoxRel(x,y, dx,dy);
	}
	else
	{
		g_RCInput->postMsg( CRCInput::RC_help, 0 );
	}
}


int CNVODChangeExec::exec(CMenuTarget* parent, string actionKey)
{
	//    printf("CNVODChangeExec exec: %s\n", actionKey.c_str());
	unsigned sel= atoi(actionKey.c_str());
	string sc = g_RemoteControl->setSubChannel(sel);

	parent->hide();
	showSubchan(sc);
	return menu_return::RETURN_EXIT;
}

int CStreamFeaturesChangeExec::exec(CMenuTarget* parent, string actionKey)
{
	//printf("CStreamFeaturesChangeExec exec: %s\n", actionKey.c_str());
	int sel= atoi(actionKey.c_str());

	parent->hide();
	if (sel==-1)
	{
		CStreamInfo StreamInfo;
		StreamInfo.exec(NULL, "");
	}
	else if (sel>=0)
	{
		g_PluginList->setvtxtpid( g_RemoteControl->current_PIDs.PIDs.vtxtpid );
		g_PluginList->startPlugin( sel );
	}

	return menu_return::RETURN_EXIT;
}

int CUCodeCheckExec::exec(CMenuTarget* parent, string actionKey)
{
	string 	text;
	char res[60];
	char buf[200];

	checkFile( UCODEDIR "/avia500.ux", (char*) &res);
	sprintf((char*) buf, "%s: %s", g_Locale->getText("ucodecheck.avia500").c_str(), res );
	text= buf;

	checkFile( UCODEDIR "/avia600.ux", (char*) &res);
	sprintf((char*) buf, "%s: %s", g_Locale->getText("ucodecheck.avia600").c_str(), res );
	text= text+ "\n"+ buf;

	checkFile( UCODEDIR "/ucode.bin", (char*) &res);
	sprintf((char*) buf, "%s: %s", g_Locale->getText("ucodecheck.ucode").c_str(), res );
	text= text+ "\n"+ buf;

	checkFile( UCODEDIR "/cam-alpha.bin", (char*) &res);
	sprintf((char*) buf, "%s: %s", g_Locale->getText("ucodecheck.cam-alpha").c_str(), res );
	text= text+ "\n"+ buf;

	ShowMsg( "ucodecheck.head", text, CMessageBox::mbrBack, CMessageBox::mbBack );
}

void setNetworkAddress(char* ip, char* netmask, char* broadcast)
{
	printf("IP       : %s\n", ip);
	printf("Netmask  : %s\n", netmask);
	printf("Broadcast: %s\n", broadcast);
	netSetIP( "eth0", ip, netmask, broadcast);
}

void setDefaultGateway(char* ip)
{
	printf("Gateway  : %s\n", ip);
	netSetDefaultRoute( ip );
}

void setNameServer(char* ip)
{
	if ( strlen(ip)> 0 )
	{
		FILE* fd = fopen("/etc/resolv.conf", "w");
		if(fd)
		{
			fprintf(fd, "# resolv.conf - generated by neutrino\n\n");
			fprintf(fd, "nameserver %s\n", ip);
			fclose(fd);
		}
		else
		{
			perror("cannot write /etc/resolv.conf");
		}
	}
	else
	{
		remove("/etc/resolv.conf");
	}
}

string mypinghost(char* host)
	{
	int retvalue=0;
	retvalue = pinghost(host);
	switch (retvalue)
		{
		case 1: return ( g_Locale->getText("ping.ok") );
		case 0: return ( g_Locale->getText("ping.unreachable") );
		case -1: return ( g_Locale->getText("ping.protocol") );
		case -2: return ( g_Locale->getText("ping.socket") );
		}
	}

void testNetworkSettings(char* ip, char* netmask, char* broadcast, char* gateway, char* nameserver)
{
	printf("testNw IP       : %s\n", ip);
	printf("testNw Netmask  : %s\n", netmask);
	printf("testNw Broadcast: %s\n", broadcast);
	printf("testNw Gateway: %s\n", gateway);
	printf("testNw Nameserver: %s\n", nameserver);

	string 	text;
	char _text[100];

	sprintf(_text, "%s %s\n", ip, mypinghost(ip).c_str() );
	text= _text;

	sprintf(_text, "Gateway %s %s\n", gateway, mypinghost(gateway).c_str() );
	text= text+ _text;

	sprintf(_text, "Nameserver %s %s\n", nameserver, mypinghost(nameserver).c_str() );
	text= text+ _text;

	sprintf(_text, "dboxupdate.berlios.de %s\n",mypinghost("195.37.77.138").c_str() );
	text= text+ _text;

	ShowMsg( "networkmenu.test", text, CMessageBox::mbrBack, CMessageBox::mbBack );
}

unsigned long long getcurrenttime()
{
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (unsigned long long) tv.tv_usec + (unsigned long long)((unsigned long long) tv.tv_sec * (unsigned long long) 1000000);
}


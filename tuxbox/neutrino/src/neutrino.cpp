/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
							 and some other guys
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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define NEUTRINO_CPP

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/wait.h>


#include <sys/socket.h>

#include <iostream>
#include <fstream>
#include <string>


#include "global.h"
#include "neutrino.h"

#include <daemonc/remotecontrol.h>

#include <driver/encoding.h>
#include <driver/framebuffer.h>
#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/stream2file.h>
#include <driver/vcrcontrol.h>
#include <irsend/irsend.h>

#include "gui/widget/colorchooser.h"
#include "gui/widget/menue.h"
#include "gui/widget/messagebox.h"
#include "gui/widget/hintbox.h"
#include "gui/widget/icons.h"
#include "gui/widget/lcdcontroler.h"
#include "gui/widget/rgbcsynccontroler.h"
#include "gui/widget/keychooser.h"
#include "gui/widget/stringinput.h"
#include "gui/widget/stringinput_ext.h"

#include "gui/color.h"

#include "gui/bedit/bouqueteditor_bouquets.h"
#include "gui/bouquetlist.h"
#include "gui/eventlist.h"
#include "gui/channellist.h"
#include "gui/screensetup.h"
#include "gui/gamelist.h"
#include "gui/infoviewer.h"
#include "gui/epgview.h"
#include "gui/epg_menu.h"
#include "gui/update.h"
#include "gui/scan.h"
#include "gui/favorites.h"
#include "gui/sleeptimer.h"
#include "gui/rc_lock.h"
#include "gui/dboxinfo.h"
#include "gui/timerlist.h"
#include "gui/alphasetup.h"
#include "gui/mp3player.h"

#if HAVE_DVB_API_VERSION >= 3
#include "gui/movieplayer.h"
#endif

#include "gui/nfs.h"
#include "gui/pictureviewer.h"
#include "gui/motorcontrol.h"


#include <system/setting_helpers.h>
#include <system/settings.h>
#include <system/debug.h>
#include <system/flashtool.h>

#include <string.h>


// uncomment if you want to have a "test" menue entry  (rasc)

// #define __EXPERIMENTAL_CODE__
#ifdef __EXPERIMENTAL_CODE__

#include "gui/ch_mosaic.h"

#endif





CBouquetList   * bouquetList;
CPlugins       * g_PluginList;
CRemoteControl * g_RemoteControl;

// Globale Variablen - to use import global.h

// I don't like globals, I would have hidden them in classes,
// but if you wanna do it so... ;)
static bool parentallocked = false;

CZapitClient::SatelliteList satList;
CZapitClient::SatelliteList::iterator satList_it;

#define NEUTRINO_SETTINGS_FILE       CONFIGDIR "/neutrino.conf"
#define NEUTRINO_SCAN_SETTINGS_FILE  CONFIGDIR "/scan.conf"
#define NEUTRINO_PARENTALLOCKED_FILE DATADIR   "/neutrino/.plocked"

static void initGlobals(void)
{
	g_fontRenderer  = NULL;

	g_RCInput       = NULL;
	g_Controld      = new CControldClient;
	g_Timerd        = NULL;
	g_Zapit         = new CZapitClient;
	g_RemoteControl = NULL;

	g_EpgData       = NULL;
	g_InfoViewer    = NULL;
	g_EventList     = NULL;

	g_Locale        = new CLocaleManager;
	g_PluginList    = NULL;
}



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+                                                                                     +
+          CNeutrinoApp - Constructor, initialize g_fontRenderer                      +
+                                                                                     +
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
CNeutrinoApp::CNeutrinoApp()
: configfile('\t')
{
	standby_pressed_at.tv_sec = 0;

	frameBuffer = CFrameBuffer::getInstance();
	frameBuffer->setIconBasePath(DATADIR "/neutrino/icons/");

	SetupFrameBuffer();

	mode = mode_unknown;
	channelList = NULL;
	bouquetList = NULL;
	skipShutdownTimer=false;
}

/*-------------------------------------------------------------------------------------
-                                                                                     -
-           CNeutrinoApp - Destructor                                                 -
-                                                                                     -
-------------------------------------------------------------------------------------*/
CNeutrinoApp::~CNeutrinoApp()
{
	if (channelList)
		delete channelList;
}

CNeutrinoApp* CNeutrinoApp::getInstance()
{
	static CNeutrinoApp* neutrinoApp = NULL;

	if(!neutrinoApp)
	{
		neutrinoApp = new CNeutrinoApp();
		dprintf(DEBUG_DEBUG, "NeutrinoApp Instance created\n");
	}
	return neutrinoApp;
}


/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  setup Color Sheme (Neutrino)                               *
*                                                                                     *
**************************************************************************************/
void CNeutrinoApp::setupColors_neutrino()
{
	g_settings.menu_Head_alpha = 0x00;
	g_settings.menu_Head_red   = 0x00;
	g_settings.menu_Head_green = 0x0A;
	g_settings.menu_Head_blue  = 0x19;

	g_settings.menu_Head_Text_alpha = 0x00;
	g_settings.menu_Head_Text_red   = 0x5f;
	g_settings.menu_Head_Text_green = 0x46;
	g_settings.menu_Head_Text_blue  = 0x00;

	g_settings.menu_Content_alpha = 0x14;
	g_settings.menu_Content_red   = 0x00;
	g_settings.menu_Content_green = 0x0f;
	g_settings.menu_Content_blue  = 0x23;

	g_settings.menu_Content_Text_alpha = 0x00;
	g_settings.menu_Content_Text_red   = 0x64;
	g_settings.menu_Content_Text_green = 0x64;
	g_settings.menu_Content_Text_blue  = 0x64;

	g_settings.menu_Content_Selected_alpha = 0x14;
	g_settings.menu_Content_Selected_red   = 0x19;
	g_settings.menu_Content_Selected_green = 0x37;
	g_settings.menu_Content_Selected_blue  = 0x64;

	g_settings.menu_Content_Selected_Text_alpha  = 0x00;
	g_settings.menu_Content_Selected_Text_red    = 0x00;
	g_settings.menu_Content_Selected_Text_green  = 0x00;
	g_settings.menu_Content_Selected_Text_blue   = 0x00;

	g_settings.menu_Content_inactive_alpha = 0x14;
	g_settings.menu_Content_inactive_red   = 0x00;
	g_settings.menu_Content_inactive_green = 0x0f;
	g_settings.menu_Content_inactive_blue  = 0x23;

	g_settings.menu_Content_inactive_Text_alpha  = 0x00;
	g_settings.menu_Content_inactive_Text_red    = 55;
	g_settings.menu_Content_inactive_Text_green  = 70;
	g_settings.menu_Content_inactive_Text_blue   = 85;

	g_settings.infobar_alpha = 0x14;
	g_settings.infobar_red   = 0x00;
	g_settings.infobar_green = 0x0e;
	g_settings.infobar_blue  = 0x23;

	g_settings.infobar_Text_alpha = 0x00;
	g_settings.infobar_Text_red   = 0x64;
	g_settings.infobar_Text_green = 0x64;
	g_settings.infobar_Text_blue  = 0x64;
}

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  setup Color Sheme (classic)                                *
*                                                                                     *
**************************************************************************************/
void CNeutrinoApp::setupColors_classic()
{
	g_settings.menu_Head_alpha = 20;
	g_settings.menu_Head_red   =  5;
	g_settings.menu_Head_green = 10;
	g_settings.menu_Head_blue  = 60;

	g_settings.menu_Head_Text_alpha = 0;
	g_settings.menu_Head_Text_red   = 100;
	g_settings.menu_Head_Text_green = 100;
	g_settings.menu_Head_Text_blue  = 100;

	g_settings.menu_Content_alpha = 20;
	g_settings.menu_Content_red   = 50;
	g_settings.menu_Content_green = 50;
	g_settings.menu_Content_blue  = 50;

	g_settings.menu_Content_Text_alpha = 0;
	g_settings.menu_Content_Text_red   = 100;
	g_settings.menu_Content_Text_green = 100;
	g_settings.menu_Content_Text_blue  = 100;

	g_settings.menu_Content_Selected_alpha = 20;
	g_settings.menu_Content_Selected_red   = 5;
	g_settings.menu_Content_Selected_green = 10;
	g_settings.menu_Content_Selected_blue  = 60;

	g_settings.menu_Content_Selected_Text_alpha  = 0;
	g_settings.menu_Content_Selected_Text_red    = 100;
	g_settings.menu_Content_Selected_Text_green  = 100;
	g_settings.menu_Content_Selected_Text_blue   = 100;

	g_settings.menu_Content_inactive_alpha = 20;
	g_settings.menu_Content_inactive_red   = 50;
	g_settings.menu_Content_inactive_green = 50;
	g_settings.menu_Content_inactive_blue  = 50;

	g_settings.menu_Content_inactive_Text_alpha  = 0;
	g_settings.menu_Content_inactive_Text_red    = 80;
	g_settings.menu_Content_inactive_Text_green  = 80;
	g_settings.menu_Content_inactive_Text_blue   = 80;

	g_settings.infobar_alpha = 20;
	g_settings.infobar_red   = 5;
	g_settings.infobar_green = 10;
	g_settings.infobar_blue  = 60;

	g_settings.infobar_Text_alpha = 0;
	g_settings.infobar_Text_red   = 100;
	g_settings.infobar_Text_green = 100;
	g_settings.infobar_Text_blue  = 100;
}


typedef struct font_sizes
{
	const char * const name;
	const unsigned int defaultsize;
	const unsigned int style;
	const unsigned int size_offset;
} font_sizes_struct;

#define FONT_STYLE_REGULAR 0
#define FONT_STYLE_BOLD    1
#define FONT_STYLE_ITALIC  2

const font_sizes_struct neutrino_font[FONT_TYPE_COUNT] = 
{
	{"fontsize.menu"               ,  20, FONT_STYLE_BOLD   , 0},
	{"fontsize.menu_title"         ,  30, FONT_STYLE_BOLD   , 0},
	{"fontsize.menu_info"          ,  16, FONT_STYLE_REGULAR, 0},
	{"fontsize.epg_title"          ,  25, FONT_STYLE_REGULAR, 1},
	{"fontsize.epg_info1"          ,  17, FONT_STYLE_ITALIC , 2},
	{"fontsize.epg_info2"          ,  17, FONT_STYLE_REGULAR, 2},
	{"fontsize.epg_date"           ,  15, FONT_STYLE_REGULAR, 2},
	{"fontsize.eventlist_title"    ,  30, FONT_STYLE_REGULAR, 0},
	{"fontsize.eventlist_itemlarge",  20, FONT_STYLE_BOLD   , 1},
	{"fontsize.eventlist_itemsmall",  14, FONT_STYLE_REGULAR, 1},
	{"fontsize.eventlist_datetime" ,  16, FONT_STYLE_REGULAR, 1},
	{"fontsize.gamelist_itemlarge" ,  20, FONT_STYLE_BOLD   , 1},
	{"fontsize.gamelist_itemsmall" ,  16, FONT_STYLE_REGULAR, 1},
	{"fontsize.channellist"        ,  20, FONT_STYLE_BOLD   , 1},
	{"fontsize.channellist_descr"  ,  20, FONT_STYLE_REGULAR, 1},
	{"fontsize.channellist_number" ,  14, FONT_STYLE_BOLD   , 2},
	{"fontsize.channel_num_zap"    ,  40, FONT_STYLE_BOLD   , 0},
	{"fontsize.infobar_number"     ,  50, FONT_STYLE_BOLD   , 0},
	{"fontsize.infobar_channame"   ,  30, FONT_STYLE_BOLD   , 0},
	{"fontsize.infobar_info"       ,  20, FONT_STYLE_REGULAR, 1},
	{"fontsize.infobar_small"      ,  14, FONT_STYLE_REGULAR, 1},
	{"fontsize.filebrowser_item"   ,  16, FONT_STYLE_BOLD   , 1}
};

const neutrino_locale_t timing_setting_name[TIMING_SETTING_COUNT] =
{
	LOCALE_TIMING_MENU,
	LOCALE_TIMING_CHANLIST,
	LOCALE_TIMING_EPG,
	LOCALE_TIMING_INFOBAR,
	LOCALE_TIMING_FILEBROWSER
};

typedef struct lcd_setting_t
{
	const char * const name;
	const unsigned int default_value;
} lcd_setting_struct_t;

const lcd_setting_struct_t lcd_setting[LCD_SETTING_COUNT] =
{
	{"lcd_brightness"       , DEFAULT_LCD_BRIGHTNESS       },
	{"lcd_standbybrightness", DEFAULT_LCD_STANDBYBRIGHTNESS},
	{"lcd_contrast"         , DEFAULT_LCD_CONTRAST         },
	{"lcd_power"            , DEFAULT_LCD_POWER            },
	{"lcd_inverse"          , DEFAULT_LCD_INVERSE          },
	{"lcd_show_volume"      , DEFAULT_LCD_SHOW_VOLUME      },
	{"lcd_autodimm"         , DEFAULT_LCD_AUTODIMM         }
};


/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  loadSetup, load the application-settings                   *
*                                                                                     *
**************************************************************************************/
int CNeutrinoApp::loadSetup()
{
	int erg = 0;

	//settings laden - und dabei Defaults setzen!
	if(!configfile.loadConfig(NEUTRINO_SETTINGS_FILE))
	{
		//file existiert nicht
		erg = 1;
	}
        std::ifstream checkParentallocked(NEUTRINO_PARENTALLOCKED_FILE);
	if(checkParentallocked) 	 
	{ 	 
	        parentallocked = true; 	 
	        checkParentallocked.close(); 	 
	} 
	//video
	g_settings.video_Signal = configfile.getInt32( "video_Signal", 1 ); //RGB + CVBS
	g_settings.video_Format = configfile.getInt32( "video_Format", 2 ); //4:3
	g_settings.video_csync = configfile.getInt32( "video_csync", 0 ); 

	//fb-alphawerte f�r gtx
	g_settings.gtx_alpha1 = configfile.getInt32( "gtx_alpha1", 0);
	g_settings.gtx_alpha2 = configfile.getInt32( "gtx_alpha2", 1);

	//misc
	g_settings.shutdown_real = configfile.getInt32( "shutdown_real", true );
	g_settings.shutdown_real_rcdelay = configfile.getInt32( "shutdown_real_rcdelay", true );
	g_settings.shutdown_showclock = configfile.getInt32( "shutdown_showclock", 1 );
	g_settings.infobar_sat_display = configfile.getInt32( "infobar_sat_display", true );

	//audio
	g_settings.audio_AnalogMode = configfile.getInt32( "audio_AnalogMode", 0 );
	g_settings.audio_DolbyDigital = configfile.getInt32( "audio_DolbyDigital", 0 );
	g_settings.audio_avs_Control = configfile.getInt32( "audio_avs_Control", CControld::TYPE_AVS );
	strcpy( g_settings.audio_PCMOffset, configfile.getString( "audio_PCMOffset", "0" ).c_str() );


	//vcr
	g_settings.vcr_AutoSwitch = configfile.getInt32( "vcr_AutoSwitch", 1 );

	//language
	strcpy(g_settings.language, configfile.getString("language", "").c_str());

	//widget settings
	g_settings.widget_fade = configfile.getInt32( "widget_fade", 1 );

	//colors (neutrino defaultcolors)
	g_settings.menu_Head_alpha = configfile.getInt32( "menu_Head_alpha", 0x00 );
	g_settings.menu_Head_red = configfile.getInt32( "menu_Head_red", 0x00 );
	g_settings.menu_Head_green = configfile.getInt32( "menu_Head_green", 0x0A );
	g_settings.menu_Head_blue = configfile.getInt32( "menu_Head_blue", 0x19 );

	g_settings.menu_Head_Text_alpha = configfile.getInt32( "menu_Head_Text_alpha", 0x00 );
	g_settings.menu_Head_Text_red = configfile.getInt32( "menu_Head_Text_red", 0x5f );
	g_settings.menu_Head_Text_green = configfile.getInt32( "menu_Head_Text_green", 0x46 );
	g_settings.menu_Head_Text_blue = configfile.getInt32( "menu_Head_Text_blue", 0x00 );

	g_settings.menu_Content_alpha = configfile.getInt32( "menu_Content_alpha", 0x14 );
	g_settings.menu_Content_red = configfile.getInt32( "menu_Content_red", 0x00 );
	g_settings.menu_Content_green = configfile.getInt32( "menu_Content_green", 0x0f );
	g_settings.menu_Content_blue = configfile.getInt32( "menu_Content_blue", 0x23 );

	g_settings.menu_Content_Text_alpha = configfile.getInt32( "menu_Content_Text_alpha", 0x00 );
	g_settings.menu_Content_Text_red = configfile.getInt32( "menu_Content_Text_red", 0x64 );
	g_settings.menu_Content_Text_green = configfile.getInt32( "menu_Content_Text_green", 0x64 );
	g_settings.menu_Content_Text_blue = configfile.getInt32( "menu_Content_Text_blue", 0x64 );

	g_settings.menu_Content_Selected_alpha = configfile.getInt32( "menu_Content_Selected_alpha", 0x14 );
	g_settings.menu_Content_Selected_red = configfile.getInt32( "menu_Content_Selected_red", 0x19 );
	g_settings.menu_Content_Selected_green = configfile.getInt32( "menu_Content_Selected_green", 0x37 );
	g_settings.menu_Content_Selected_blue = configfile.getInt32( "menu_Content_Selected_blue", 0x64 );

	g_settings.menu_Content_Selected_Text_alpha = configfile.getInt32( "menu_Content_Selected_Text_alpha", 0x00 );
	g_settings.menu_Content_Selected_Text_red = configfile.getInt32( "menu_Content_Selected_Text_red", 0x00 );
	g_settings.menu_Content_Selected_Text_green = configfile.getInt32( "menu_Content_Selected_Text_green", 0x00 );
	g_settings.menu_Content_Selected_Text_blue = configfile.getInt32( "menu_Content_Selected_Text_blue", 0x00 );

	g_settings.menu_Content_inactive_alpha = configfile.getInt32( "menu_Content_inactive_alpha", 0x14 );
	g_settings.menu_Content_inactive_red = configfile.getInt32( "menu_Content_inactive_red", 0x00 );
	g_settings.menu_Content_inactive_green = configfile.getInt32( "menu_Content_inactive_green", 0x0f );
	g_settings.menu_Content_inactive_blue = configfile.getInt32( "menu_Content_inactive_blue", 0x23 );

	g_settings.menu_Content_inactive_Text_alpha = configfile.getInt32( "menu_Content_inactive_Text_alpha", 0x00 );
	g_settings.menu_Content_inactive_Text_red = configfile.getInt32( "menu_Content_inactive_Text_red", 55 );
	g_settings.menu_Content_inactive_Text_green = configfile.getInt32( "menu_Content_inactive_Text_green", 70 );
	g_settings.menu_Content_inactive_Text_blue = configfile.getInt32( "menu_Content_inactive_Text_blue", 85 );

	g_settings.infobar_alpha = configfile.getInt32( "infobar_alpha", 0x14 );
	g_settings.infobar_red = configfile.getInt32( "infobar_red", 0x00 );
	g_settings.infobar_green = configfile.getInt32( "infobar_green", 0x0e );
	g_settings.infobar_blue = configfile.getInt32( "infobar_blue", 0x23 );

	g_settings.infobar_Text_alpha = configfile.getInt32( "infobar_Text_alpha", 0x00 );
	g_settings.infobar_Text_red = configfile.getInt32( "infobar_Text_red", 0x64 );
	g_settings.infobar_Text_green = configfile.getInt32( "infobar_Text_green", 0x64 );
	g_settings.infobar_Text_blue = configfile.getInt32( "infobar_Text_blue", 0x64 );

	//network
	g_settings.network_nfs_ip[0] = configfile.getString("network_nfs_ip_1", "");
	g_settings.network_nfs_ip[1] = configfile.getString("network_nfs_ip_2", "");
	g_settings.network_nfs_ip[2] = configfile.getString("network_nfs_ip_3", "");
	g_settings.network_nfs_ip[3] = configfile.getString("network_nfs_ip_4", "");
	strcpy( g_settings.network_nfs_dir[0], configfile.getString( "network_nfs_dir_1", "" ).c_str() );
	strcpy( g_settings.network_nfs_dir[1], configfile.getString( "network_nfs_dir_2", "" ).c_str() );
	strcpy( g_settings.network_nfs_dir[2], configfile.getString( "network_nfs_dir_3", "" ).c_str() );
	strcpy( g_settings.network_nfs_dir[3], configfile.getString( "network_nfs_dir_4", "" ).c_str() );
	strcpy( g_settings.network_nfs_local_dir[0], configfile.getString( "network_nfs_local_dir_1", "" ).c_str() );
	strcpy( g_settings.network_nfs_local_dir[1], configfile.getString( "network_nfs_local_dir_2", "" ).c_str() );
	strcpy( g_settings.network_nfs_local_dir[2], configfile.getString( "network_nfs_local_dir_3", "" ).c_str() );
	strcpy( g_settings.network_nfs_local_dir[3], configfile.getString( "network_nfs_local_dir_4", "" ).c_str() );
	g_settings.network_nfs_automount[0] = configfile.getInt32( "network_nfs_automount_1", 0);
	g_settings.network_nfs_automount[1] = configfile.getInt32( "network_nfs_automount_2", 0);
	g_settings.network_nfs_automount[2] = configfile.getInt32( "network_nfs_automount_3", 0);
	g_settings.network_nfs_automount[3] = configfile.getInt32( "network_nfs_automount_4", 0);
	g_settings.network_nfs_type[0] = configfile.getInt32( "network_nfs_type_1", 0);
	g_settings.network_nfs_type[1] = configfile.getInt32( "network_nfs_type_2", 0);
	g_settings.network_nfs_type[2] = configfile.getInt32( "network_nfs_type_3", 0);
	g_settings.network_nfs_type[3] = configfile.getInt32( "network_nfs_type_4", 0);
	strcpy( g_settings.network_nfs_username[0], configfile.getString( "network_nfs_username_1", "" ).c_str() );
	strcpy( g_settings.network_nfs_username[1], configfile.getString( "network_nfs_username_2", "" ).c_str() );
	strcpy( g_settings.network_nfs_username[2], configfile.getString( "network_nfs_username_3", "" ).c_str() );
	strcpy( g_settings.network_nfs_username[3], configfile.getString( "network_nfs_username_4", "" ).c_str() );
	strcpy( g_settings.network_nfs_password[0], configfile.getString( "network_nfs_password_1", "" ).c_str() );
	strcpy( g_settings.network_nfs_password[1], configfile.getString( "network_nfs_password_2", "" ).c_str() );
	strcpy( g_settings.network_nfs_password[2], configfile.getString( "network_nfs_password_3", "" ).c_str() );
	strcpy( g_settings.network_nfs_password[3], configfile.getString( "network_nfs_password_4", "" ).c_str() );
	strcpy( g_settings.network_nfs_mount_options[0], configfile.getString( "network_nfs_mount_options_1", "ro,soft,udp" ).c_str() );
	strcpy( g_settings.network_nfs_mount_options[1], configfile.getString( "network_nfs_mount_options_2", "nolock,rsize=8192,wsize=8192" ).c_str() );
	strcpy( g_settings.network_nfs_mp3dir, configfile.getString( "network_nfs_mp3dir", "" ).c_str() );
	strcpy( g_settings.network_nfs_picturedir, configfile.getString( "network_nfs_picturedir", "" ).c_str() );
	strcpy( g_settings.network_nfs_moviedir, configfile.getString( "network_nfs_moviedir", "" ).c_str() );
	strcpy( g_settings.network_nfs_recordingdir, configfile.getString( "network_nfs_recordingdir", "" ).c_str() );

	//recording (server + vcr)
	g_settings.recording_type = configfile.getInt32("recording_type", RECORDING_OFF);
	g_settings.recording_stopplayback = configfile.getInt32( "recording_stopplayback", 0 );
	g_settings.recording_stopsectionsd = configfile.getInt32( "recording_stopsectionsd", 1 );
	g_settings.recording_server_ip = configfile.getString("recording_server_ip", "10.10.10.10");
	strcpy( g_settings.recording_server_port, configfile.getString( "recording_server_port", "4000").c_str() );
	g_settings.recording_server_wakeup = configfile.getInt32( "recording_server_wakeup", 0 );
	strcpy( g_settings.recording_server_mac, configfile.getString( "recording_server_mac", "11:22:33:44:55:66").c_str() );
	g_settings.recording_vcr_no_scart = configfile.getInt32( "recording_vcr_no_scart", false);
	strcpy( g_settings.recording_splitsize, configfile.getString( "recording_splitsize", "2048").c_str() );
	g_settings.recording_stream_all_audio_pids = configfile.getInt32("recordingmenu.stream_all_audio_pids", true);

	//streaming (server)
	g_settings.streaming_type = configfile.getInt32( "streaming_type", 0 );
	g_settings.streaming_server_ip = configfile.getString("streaming_server_ip", "10.10.10.10");
	strcpy( g_settings.streaming_server_port, configfile.getString( "streaming_server_port", "8080").c_str() );
	strcpy( g_settings.streaming_server_cddrive, configfile.getString("streaming_server_cddrive", "D:").c_str() );
	strcpy( g_settings.streaming_videorate,  configfile.getString("streaming_videorate", "1000").c_str() );
	strcpy( g_settings.streaming_audiorate, configfile.getString("streaming_audiorate", "192").c_str() );
	strcpy( g_settings.streaming_server_startdir, configfile.getString("streaming_server_startdir", "C:/Movies").c_str() );
	g_settings.streaming_transcode_audio = configfile.getInt32( "streaming_transcode_audio", 0 );
	g_settings.streaming_force_transcode_video = configfile.getInt32( "streaming_force_transcode_video", 0 );
	g_settings.streaming_transcode_video_codec = configfile.getInt32( "streaming_transcode_video_codec", 0 );
	g_settings.streaming_force_avi_rawaudio = configfile.getInt32( "streaming_force_avi_rawaudio", 0 );
	g_settings.streaming_resolution = configfile.getInt32( "streaming_resolution", 0 );
	
	//rc-key configuration
	g_settings.key_tvradio_mode = configfile.getInt32( "key_tvradio_mode", CRCInput::RC_nokey );

	g_settings.key_channelList_pageup = configfile.getInt32( "key_channelList_pageup",  CRCInput::RC_minus );
	g_settings.key_channelList_pagedown = configfile.getInt32( "key_channelList_pagedown", CRCInput::RC_plus );
	g_settings.key_channelList_cancel = configfile.getInt32( "key_channelList_cancel",  CRCInput::RC_home );
	g_settings.key_channelList_sort = configfile.getInt32( "key_channelList_sort",  CRCInput::RC_blue );
	g_settings.key_channelList_addrecord = configfile.getInt32( "key_channelList_addrecord",  CRCInput::RC_nokey );
	g_settings.key_channelList_addremind = configfile.getInt32( "key_channelList_addremind",  CRCInput::RC_nokey );

	g_settings.key_quickzap_up = configfile.getInt32( "key_quickzap_up",  CRCInput::RC_up );
	g_settings.key_quickzap_down = configfile.getInt32( "key_quickzap_down",  CRCInput::RC_down );
	g_settings.key_bouquet_up = configfile.getInt32( "key_bouquet_up",  CRCInput::RC_right );
	g_settings.key_bouquet_down = configfile.getInt32( "key_bouquet_down",  CRCInput::RC_left );
	g_settings.key_subchannel_up = configfile.getInt32( "key_subchannel_up",  CRCInput::RC_right );
	g_settings.key_subchannel_down = configfile.getInt32( "key_subchannel_down",  CRCInput::RC_left );

	strcpy(g_settings.repeat_blocker, configfile.getString("repeat_blocker", g_info.box_Type == CControld::TUXBOX_MAKER_PHILIPS ? "150" : "25").c_str());
	strcpy(g_settings.repeat_genericblocker, configfile.getString("repeat_genericblocker", g_info.box_Type == CControld::TUXBOX_MAKER_PHILIPS ? "25" : "0").c_str());

	//screen configuration
	g_settings.screen_StartX = configfile.getInt32( "screen_StartX", 37 );
	g_settings.screen_StartY = configfile.getInt32( "screen_StartY", 23 );
	g_settings.screen_EndX = configfile.getInt32( "screen_EndX", 668 );
	g_settings.screen_EndY = configfile.getInt32( "screen_EndY", 555 );

	//Software-update
	g_settings.softupdate_mode = configfile.getInt32( "softupdate_mode", 1 );
	strcpy(g_settings.softupdate_url_file, configfile.getString("softupdate_url_file", "/etc/cramfs.urls").c_str());
	strcpy(g_settings.softupdate_proxyserver, configfile.getString("softupdate_proxyserver", "" ).c_str());
	strcpy(g_settings.softupdate_proxyusername, configfile.getString("softupdate_proxyusername", "" ).c_str());
	strcpy(g_settings.softupdate_proxypassword, configfile.getString("softupdate_proxypassword", "" ).c_str());

	//BouquetHandling
	g_settings.bouquetlist_mode = configfile.getInt32( "bouquetlist_mode", 0 );

	// parentallock
	if(!parentallocked) 	 
  	{	 
	  	g_settings.parentallock_prompt = configfile.getInt32( "parentallock_prompt", 0 );
		g_settings.parentallock_lockage = configfile.getInt32( "parentallock_lockage", 12 );
	} 	 
	else 	 
	{ 	 
	        g_settings.parentallock_prompt = 3; 	 
	        g_settings.parentallock_lockage = 18; 	 
	}
	strcpy( g_settings.parentallock_pincode, configfile.getString( "parentallock_pincode", "0000" ).c_str() );

	for (int i = 0; i < TIMING_SETTING_COUNT; i++)
		g_settings.timing[i] = configfile.getInt32(timing_setting_name[i], default_timing[i]);

	for (int i = 0; i < LCD_SETTING_COUNT; i++)
		g_settings.lcd_setting[i] = configfile.getInt32(lcd_setting[i].name, lcd_setting[i].default_value);

	//Picture-Viewer
	strcpy( g_settings.picviewer_slide_time, configfile.getString( "picviewer_slide_time", "10" ).c_str() );
	g_settings.picviewer_scaling = configfile.getInt32("picviewer_scaling", 1 /*(int)CPictureViewer::SIMPLE*/);

	//MP3-Player
	g_settings.mp3player_display = configfile.getInt32("mp3player_display",(int)CMP3PlayerGui::ARTIST_TITLE);
	g_settings.mp3player_follow  = configfile.getInt32("mp3player_follow",0);
	strcpy( g_settings.mp3player_screensaver, configfile.getString( "mp3player_screensaver", "0" ).c_str() );
	g_settings.mp3player_highprio  = configfile.getInt32("mp3player_highprio",0);

	//Filebrowser
	g_settings.filebrowser_showrights =  configfile.getInt32("filebrowser_showrights", 1);
	g_settings.filebrowser_sortmethod = configfile.getInt32("filebrowser_sortmethod", 0);
	if ((g_settings.filebrowser_sortmethod < 0) || (g_settings.filebrowser_sortmethod >= FILEBROWSER_NUMBER_OF_SORT_VARIANTS))
		g_settings.filebrowser_sortmethod = 0;
		
	if(configfile.getUnknownKeyQueryedFlag() && (erg==0))
	{
		erg = 2;
	}

	if (!scanSettings.loadSettings(NEUTRINO_SCAN_SETTINGS_FILE, (g_info.delivery_system = g_Zapit->getDeliverySystem())))
	{
		dprintf(DEBUG_NORMAL, "Loading of scan settings failed. Using defaults.\n");
	}

	// uboot config file
	if(fromflash)
	{
		g_settings.uboot_console	= 0;
		g_settings.uboot_lcd_inverse	= -1;
		g_settings.uboot_lcd_contrast	= -1;
	
		FILE* fd = fopen("/var/tuxbox/boot/boot.conf", "r");
		if(fd)
		{
			char buffer[100];
	
			while(fgets(buffer, 99, fd) != NULL)
			{
				if(strncmp(buffer,"console=",8) == 0)
				{
					if(strncmp(&buffer[8], "null", 4)==0)
						g_settings.uboot_console = 0;
					else if(strncmp(&buffer[8], "ttyS0", 5)==0)
						g_settings.uboot_console = 1;
					else if(strncmp(&buffer[8], "tty", 3)==0)
						g_settings.uboot_console = 2;
				}
				else if(strncmp(buffer,"lcd_inverse=", 12) == 0)
				{
					g_settings.uboot_lcd_inverse = atoi(&buffer[12]);
				}
				else if(strncmp(buffer,"lcd_contrast=", 13) == 0)
				{
					g_settings.uboot_lcd_contrast = atoi(&buffer[13]);
				}
				else
					printf("unknown entry found in boot.conf\n");
			}
	
			fclose(fd);
		}
		g_settings.uboot_console_bak = g_settings.uboot_console;
	}

	return erg;
}

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  saveSetup, save the application-settings                   *
*                                                                                     *
**************************************************************************************/
void CNeutrinoApp::saveSetup()
{
	//uboot; write config only on changes
	if (fromflash &&
	    ((g_settings.uboot_console_bak != g_settings.uboot_console) ||
	     (g_settings.uboot_lcd_inverse  != g_settings.lcd_setting[SNeutrinoSettings::LCD_INVERSE]) ||
	     (g_settings.uboot_lcd_contrast != g_settings.lcd_setting[SNeutrinoSettings::LCD_CONTRAST])))
	{
		FILE* fd = fopen("/var/tuxbox/boot/boot.conf", "w");

		if(fd != NULL)
		{
			const char * buffer;
			g_settings.uboot_console_bak    = g_settings.uboot_console;
			g_settings.uboot_lcd_inverse	= g_settings.lcd_setting[SNeutrinoSettings::LCD_INVERSE];
			g_settings.uboot_lcd_contrast	= g_settings.lcd_setting[SNeutrinoSettings::LCD_CONTRAST];

			switch(g_settings.uboot_console)
			{
			case 1:
				buffer = "ttyS0";
				break;
			case 2:
				buffer = "tty";
				break;
			default:
				buffer = "null";
				break;
			}
			fprintf(fd, "console=%s\n" "lcd_inverse=%d\n" "lcd_contrast=%d\n", buffer, g_settings.uboot_lcd_inverse, g_settings.uboot_lcd_contrast);
			fclose(fd);
		}
		else
		{
			dprintf(DEBUG_NORMAL, "unable to write file /var/tuxbox/boot/boot.conf\n");
		}
	}

	//scan settings
	if(!scanSettings.saveSettings(NEUTRINO_SCAN_SETTINGS_FILE))
	{
		dprintf(DEBUG_NORMAL, "error while saving scan-settings!\n");
	}

	//video
	configfile.setInt32( "video_Signal", g_settings.video_Signal );
	configfile.setInt32( "video_Format", g_settings.video_Format );
	configfile.setInt32( "video_csync", g_settings.video_csync );

	//fb-alphawerte f�r gtx
	configfile.setInt32( "gtx_alpha1", g_settings.gtx_alpha1 );
	configfile.setInt32( "gtx_alpha2", g_settings.gtx_alpha2 );

	//misc
	configfile.setInt32( "shutdown_real", g_settings.shutdown_real );
	configfile.setInt32( "shutdown_real_rcdelay", g_settings.shutdown_real_rcdelay );
	configfile.setInt32( "shutdown_showclock", g_settings.shutdown_showclock);
	configfile.setInt32( "infobar_sat_display", g_settings.infobar_sat_display );

	//audio
	configfile.setInt32( "audio_AnalogMode", g_settings.audio_AnalogMode );
	configfile.setInt32( "audio_DolbyDigital", g_settings.audio_DolbyDigital );
	configfile.setInt32( "audio_avs_Control", g_settings.audio_avs_Control );
	configfile.setString( "audio_PCMOffset", g_settings.audio_PCMOffset );

	//vcr
	configfile.setInt32( "vcr_AutoSwitch", g_settings.vcr_AutoSwitch );

	//language
	configfile.setString("language", g_settings.language);

	//widget settings
	configfile.setInt32( "widget_fade", g_settings.widget_fade );

	//colors
	configfile.setInt32( "menu_Head_alpha", g_settings.menu_Head_alpha );
	configfile.setInt32( "menu_Head_red", g_settings.menu_Head_red );
	configfile.setInt32( "menu_Head_green", g_settings.menu_Head_green );
	configfile.setInt32( "menu_Head_blue", g_settings.menu_Head_blue );

	configfile.setInt32( "menu_Head_Text_alpha", g_settings.menu_Head_Text_alpha );
	configfile.setInt32( "menu_Head_Text_red", g_settings.menu_Head_Text_red );
	configfile.setInt32( "menu_Head_Text_green", g_settings.menu_Head_Text_green );
	configfile.setInt32( "menu_Head_Text_blue", g_settings.menu_Head_Text_blue );

	configfile.setInt32( "menu_Content_alpha", g_settings.menu_Content_alpha );
	configfile.setInt32( "menu_Content_red", g_settings.menu_Content_red );
	configfile.setInt32( "menu_Content_green", g_settings.menu_Content_green );
	configfile.setInt32( "menu_Content_blue", g_settings.menu_Content_blue );

	configfile.setInt32( "menu_Content_Text_alpha", g_settings.menu_Content_Text_alpha );
	configfile.setInt32( "menu_Content_Text_red", g_settings.menu_Content_Text_red );
	configfile.setInt32( "menu_Content_Text_green", g_settings.menu_Content_Text_green );
	configfile.setInt32( "menu_Content_Text_blue", g_settings.menu_Content_Text_blue );

	configfile.setInt32( "menu_Content_Selected_alpha", g_settings.menu_Content_Selected_alpha );
	configfile.setInt32( "menu_Content_Selected_red", g_settings.menu_Content_Selected_red );
	configfile.setInt32( "menu_Content_Selected_green", g_settings.menu_Content_Selected_green );
	configfile.setInt32( "menu_Content_Selected_blue", g_settings.menu_Content_Selected_blue );

	configfile.setInt32( "menu_Content_Selected_Text_alpha", g_settings.menu_Content_Selected_Text_alpha );
	configfile.setInt32( "menu_Content_Selected_Text_red", g_settings.menu_Content_Selected_Text_red );
	configfile.setInt32( "menu_Content_Selected_Text_green", g_settings.menu_Content_Selected_Text_green );
	configfile.setInt32( "menu_Content_Selected_Text_blue", g_settings.menu_Content_Selected_Text_blue );

	configfile.setInt32( "menu_Content_inactive_alpha", g_settings.menu_Content_inactive_alpha );
	configfile.setInt32( "menu_Content_inactive_red", g_settings.menu_Content_inactive_red );
	configfile.setInt32( "menu_Content_inactive_green", g_settings.menu_Content_inactive_green );
	configfile.setInt32( "menu_Content_inactive_blue", g_settings.menu_Content_inactive_blue );

	configfile.setInt32( "menu_Content_inactive_Text_alpha", g_settings.menu_Content_inactive_Text_alpha );
	configfile.setInt32( "menu_Content_inactive_Text_red", g_settings.menu_Content_inactive_Text_red );
	configfile.setInt32( "menu_Content_inactive_Text_green", g_settings.menu_Content_inactive_Text_green );
	configfile.setInt32( "menu_Content_inactive_Text_blue", g_settings.menu_Content_inactive_Text_blue );

	configfile.setInt32( "infobar_alpha", g_settings.infobar_alpha );
	configfile.setInt32( "infobar_red", g_settings.infobar_red );
	configfile.setInt32( "infobar_green", g_settings.infobar_green );
	configfile.setInt32( "infobar_blue", g_settings.infobar_blue );

	configfile.setInt32( "infobar_Text_alpha", g_settings.infobar_Text_alpha );
	configfile.setInt32( "infobar_Text_red", g_settings.infobar_Text_red );
	configfile.setInt32( "infobar_Text_green", g_settings.infobar_Text_green );
	configfile.setInt32( "infobar_Text_blue", g_settings.infobar_Text_blue );

	//network
	configfile.setString( "network_nfs_ip_1", g_settings.network_nfs_ip[0] );
	configfile.setString( "network_nfs_ip_2", g_settings.network_nfs_ip[1] );
	configfile.setString( "network_nfs_ip_3", g_settings.network_nfs_ip[2] );
	configfile.setString( "network_nfs_ip_4", g_settings.network_nfs_ip[3] );
	configfile.setString( "network_nfs_dir_1", g_settings.network_nfs_dir[0] );
	configfile.setString( "network_nfs_dir_2", g_settings.network_nfs_dir[1] );
	configfile.setString( "network_nfs_dir_3", g_settings.network_nfs_dir[2] );
	configfile.setString( "network_nfs_dir_4", g_settings.network_nfs_dir[3] );
	configfile.setString( "network_nfs_local_dir_1", g_settings.network_nfs_local_dir[0] );
	configfile.setString( "network_nfs_local_dir_2", g_settings.network_nfs_local_dir[1] );
	configfile.setString( "network_nfs_local_dir_3", g_settings.network_nfs_local_dir[2] );
	configfile.setString( "network_nfs_local_dir_4", g_settings.network_nfs_local_dir[3] );
	configfile.setInt32( "network_nfs_automount_1", g_settings.network_nfs_automount[0]);
	configfile.setInt32( "network_nfs_automount_2", g_settings.network_nfs_automount[1]);
	configfile.setInt32( "network_nfs_automount_3", g_settings.network_nfs_automount[2]);
	configfile.setInt32( "network_nfs_automount_4", g_settings.network_nfs_automount[3]);
	configfile.setInt32( "network_nfs_type_1", g_settings.network_nfs_type[0]);
	configfile.setInt32( "network_nfs_type_2", g_settings.network_nfs_type[1]);
	configfile.setInt32( "network_nfs_type_3", g_settings.network_nfs_type[2]);
	configfile.setInt32( "network_nfs_type_4", g_settings.network_nfs_type[3]);
	configfile.setString( "network_nfs_username_1", g_settings.network_nfs_username[0] );
	configfile.setString( "network_nfs_username_2", g_settings.network_nfs_username[1] );
	configfile.setString( "network_nfs_username_3", g_settings.network_nfs_username[2] );
	configfile.setString( "network_nfs_username_4", g_settings.network_nfs_username[3] );
	configfile.setString( "network_nfs_password_1", g_settings.network_nfs_password[0] );
	configfile.setString( "network_nfs_password_2", g_settings.network_nfs_password[1] );
	configfile.setString( "network_nfs_password_3", g_settings.network_nfs_password[2] );
	configfile.setString( "network_nfs_password_4", g_settings.network_nfs_password[3] );
	configfile.setString( "network_nfs_mount_options_1", g_settings.network_nfs_mount_options[0]);
	configfile.setString( "network_nfs_mount_options_2", g_settings.network_nfs_mount_options[1]);
	configfile.setString( "network_nfs_mp3dir", g_settings.network_nfs_mp3dir);
	configfile.setString( "network_nfs_picturedir", g_settings.network_nfs_picturedir);
	configfile.setString( "network_nfs_moviedir", g_settings.network_nfs_moviedir);
	configfile.setString( "network_nfs_recordingdir", g_settings.network_nfs_recordingdir);

	//recording (server + vcr)
	configfile.setInt32 ("recording_type",                      g_settings.recording_type);
	configfile.setInt32 ("recording_stopplayback",              g_settings.recording_stopplayback);
	configfile.setInt32 ("recording_stopsectionsd",             g_settings.recording_stopsectionsd);
	configfile.setString("recording_server_ip",                 g_settings.recording_server_ip);
	configfile.setString("recording_server_port",               g_settings.recording_server_port);
	configfile.setInt32 ("recording_server_wakeup",             g_settings.recording_server_wakeup);
	configfile.setString("recording_server_mac",                g_settings.recording_server_mac);
	configfile.setInt32 ("recording_vcr_no_scart",              g_settings.recording_vcr_no_scart);
	configfile.setString("recording_splitsize",                 g_settings.recording_splitsize);
	configfile.setInt32 ("recordingmenu.stream_all_audio_pids", g_settings.recording_stream_all_audio_pids);

	//streaming
	configfile.setInt32 ( "streaming_type", g_settings.streaming_type );
	configfile.setString( "streaming_server_ip", g_settings.streaming_server_ip );
	configfile.setString( "streaming_server_port", g_settings.streaming_server_port );
	configfile.setString( "streaming_server_cddrive", g_settings.streaming_server_cddrive );
	configfile.setString ( "streaming_videorate", g_settings.streaming_videorate );
	configfile.setString ( "streaming_audiorate", g_settings.streaming_audiorate );
	configfile.setString( "streaming_server_startdir", g_settings.streaming_server_startdir );
	configfile.setInt32 ( "streaming_transcode_audio", g_settings.streaming_transcode_audio );
	configfile.setInt32 ( "streaming_force_avi_rawaudio", g_settings.streaming_force_avi_rawaudio );
	configfile.setInt32 ( "streaming_force_transcode_video", g_settings.streaming_force_transcode_video );
	configfile.setInt32 ( "streaming_transcode_video_codec", g_settings.streaming_transcode_video_codec );
	configfile.setInt32 ( "streaming_resolution", g_settings.streaming_resolution );
	
	//rc-key configuration
	configfile.setInt32( "key_tvradio_mode", g_settings.key_tvradio_mode );

	configfile.setInt32( "key_channelList_pageup", g_settings.key_channelList_pageup );
	configfile.setInt32( "key_channelList_pagedown", g_settings.key_channelList_pagedown );
	configfile.setInt32( "key_channelList_cancel", g_settings.key_channelList_cancel );
	configfile.setInt32( "key_channelList_sort", g_settings.key_channelList_sort );
	configfile.setInt32( "key_channelList_addrecord", g_settings.key_channelList_addrecord );
	configfile.setInt32( "key_channelList_addremind", g_settings.key_channelList_addremind );

	configfile.setInt32( "key_quickzap_up", g_settings.key_quickzap_up );
	configfile.setInt32( "key_quickzap_down", g_settings.key_quickzap_down );
	configfile.setInt32( "key_bouquet_up", g_settings.key_bouquet_up );
	configfile.setInt32( "key_bouquet_down", g_settings.key_bouquet_down );
	configfile.setInt32( "key_subchannel_up", g_settings.key_subchannel_up );
	configfile.setInt32( "key_subchannel_down", g_settings.key_subchannel_down );

	configfile.setString( "repeat_blocker", g_settings.repeat_blocker );
	configfile.setString( "repeat_genericblocker", g_settings.repeat_genericblocker );

	//screen configuration
	configfile.setInt32( "screen_StartX", g_settings.screen_StartX );
	configfile.setInt32( "screen_StartY", g_settings.screen_StartY );
	configfile.setInt32( "screen_EndX", g_settings.screen_EndX );
	configfile.setInt32( "screen_EndY", g_settings.screen_EndY );

	//Software-update
	configfile.setInt32 ("softupdate_mode"          , g_settings.softupdate_mode          );
	configfile.setString("softupdate_url_file"      , g_settings.softupdate_url_file      );
	configfile.setString("softupdate_proxyserver"   , g_settings.softupdate_proxyserver   );
	configfile.setString("softupdate_proxyusername" , g_settings.softupdate_proxyusername );
	configfile.setString("softupdate_proxypassword" , g_settings.softupdate_proxypassword );

	//BouquetHandling
	configfile.setInt32( "bouquetlist_mode", g_settings.bouquetlist_mode );

	//parentallock
	configfile.setInt32( "parentallock_prompt", g_settings.parentallock_prompt );
	configfile.setInt32( "parentallock_lockage", g_settings.parentallock_lockage );
	configfile.setString( "parentallock_pincode", g_settings.parentallock_pincode );

	//timing
	for (int i = 0; i < TIMING_SETTING_COUNT; i++)
		configfile.setInt32(timing_setting_name[i], g_settings.timing[i]);

	for (int i = 0; i < LCD_SETTING_COUNT; i++)
		configfile.setInt32(lcd_setting[i].name, g_settings.lcd_setting[i]);

	//Picture-Viewer
	configfile.setString( "picviewer_slide_time", g_settings.picviewer_slide_time );
	configfile.setInt32( "picviewer_scaling", g_settings.picviewer_scaling );

	//MP3-Player
	configfile.setInt32( "mp3player_display", g_settings.mp3player_display );
	configfile.setInt32( "mp3player_follow", g_settings.mp3player_follow );
	configfile.setString( "mp3player_screensaver", g_settings.mp3player_screensaver );
	configfile.setInt32( "mp3player_highprio", g_settings.mp3player_highprio );

	//Filebrowser
	configfile.setInt32("filebrowser_showrights", g_settings.filebrowser_showrights);
	configfile.setInt32("filebrowser_sortmethod", g_settings.filebrowser_sortmethod);

	if (configfile.getModifiedFlag())
	{
		dprintf(DEBUG_INFO, "saving neutrino txt-config\n");
		configfile.saveConfig(NEUTRINO_SETTINGS_FILE);
	}
}

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  firstChannel, get the initial channel                      *
*                                                                                     *
**************************************************************************************/

void CNeutrinoApp::firstChannel()
{
	g_Zapit->getLastChannel(firstchannel.channelNumber, firstchannel.mode);
}

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  doChecks, check if card fits cam		              *
*                                                                                     *
**************************************************************************************/

void CNeutrinoApp::doChecks()
{
	FILE* fd;
	fd = fopen(UCODEDIR "/avia500.ux", "r");
	if(fd)
		fclose(fd);
	bool ucodes_ok= (fd);
	fd = fopen(UCODEDIR "/avia600.ux", "r");
	if(fd)
		fclose(fd);
	ucodes_ok= ucodes_ok||(fd);
/*	ucode.bin no longer needed, since built-in *
	fd = fopen(UCODEDIR "/ucode.bin", "r");
	if(fd)
		fclose(fd);
	ucodes_ok= ucodes_ok&&(fd);*/
	fd = fopen(UCODEDIR "/cam-alpha.bin", "r");
	if(fd)
		fclose(fd);
	ucodes_ok= ucodes_ok&&(fd);
	if (!ucodes_ok)
		DisplayErrorMessage(g_Locale->getText(LOCALE_UCODES_FAILURE));
}


/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  channelsInit, get the Channellist from daemon              *
*                                                                                     *
**************************************************************************************/
void CNeutrinoApp::channelsInit()
{
	dprintf(DEBUG_DEBUG, "doing channelsInit\n");
	//deleting old channelList for mode-switching.

	if (channelList)
		delete channelList;

	channelList = new CChannelList(g_Locale->getText(LOCALE_CHANNELLIST_HEAD));
	CZapitClient::BouquetChannelList zapitChannels;
	g_Zapit->getChannels(zapitChannels, CZapitClient::MODE_CURRENT, CZapitClient::SORT_BOUQUET, true); // UTF-8
	for(uint i=0; i<zapitChannels.size(); i++)
	{
		channelList->addChannel(zapitChannels[i].nr, zapitChannels[i].nr, zapitChannels[i].name, zapitChannels[i].satellitePosition, zapitChannels[i].channel_id); // UTF-8
	}

	delete bouquetList;
	bouquetList = new CBouquetList();
	bouquetList->orgChannelList = channelList;
	CZapitClient::BouquetList zapitBouquets;

	/* load non-empty bouquets only */
	g_Zapit->getBouquets(zapitBouquets, false, true); // UTF-8
	for(uint i = 0; i < zapitBouquets.size(); i++)
	{
		CZapitClient::BouquetChannelList zapitChannels;

		/* add terminating 0 to zapitBouquets[i].name */
		char bouquetname[sizeof(zapitBouquets[i].name) + 1];
		strncpy(bouquetname, zapitBouquets[i].name, sizeof(zapitBouquets[i].name));
		bouquetname[sizeof(zapitBouquets[i].name)] = 0;

		bouquetList->addBouquet(bouquetname, zapitBouquets[i].bouquet_nr, zapitBouquets[i].locked);


		g_Zapit->getBouquetChannels(zapitBouquets[i].bouquet_nr, zapitChannels, CZapitClient::MODE_CURRENT, true); // UTF-8

		for (uint j = 0; j < zapitChannels.size(); j++)
		{
			CChannelList::CChannel* channel = channelList->getChannel(zapitChannels[j].nr);

			/* observe that "bouquetList->Bouquets[i]" refers to the bouquet we just created using bouquetList->addBouquet */
			bouquetList->Bouquets[i]->channelList->addChannel(channel);

			if (zapitBouquets[i].locked)
			{
				channel->bAlwaysLocked = true;
			}
		}
	}
	dprintf(DEBUG_DEBUG, "\nAll bouquets-channels received\n");
}


/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  run, the main runloop                                      *
*                                                                                     *
**************************************************************************************/

void CNeutrinoApp::CmdParser(int argc, char **argv)
{
	softupdate = false;
	fromflash = false;

	font.name = NULL;

	for(int x=1; x<argc; x++)
	{
		if ((!strcmp(argv[x], "-u")) || (!strcmp(argv[x], "--enable-update")))
		{
			dprintf(DEBUG_NORMAL, "Software update enabled\n");
			softupdate = true;
		}
		else if ((!strcmp(argv[x], "-f")) || (!strcmp(argv[x], "--enable-flash")))
		{
			dprintf(DEBUG_NORMAL, "enable flash\n");
			fromflash = true;
		}
		else if (!strcmp(argv[x], "--font"))
		{
			if ((x + 3) < argc)
			{
				font.name = argv[x + 1];
				font.size_offset = atoi(argv[x + 2]);
				font.filename[0] = argv[x + 3];
				if ((x + 4) < argc)
				{
					font.filename[1] = argv[x + 4];
					x++;
				}
				else
					font.filename[1] = NULL;

				if ((x + 4) < argc)
				{
					font.filename[2] = argv[x + 4];
					x++;
				}
				else
					font.filename[2] = NULL;
			}
			x += 3;
		}
		else if ((!strcmp(argv[x], "-v")) || (!strcmp(argv[x], "--verbose")))
		{
			int dl = atoi(argv[x+ 1]);
			dprintf(DEBUG_NORMAL, "set debuglevel: %d\n", dl);
			setDebugLevel(dl);
			x++;
		}
		else
		{
			dprintf(DEBUG_NORMAL, "Usage: neutrino [-u | --enable-update] [-f | --enable-flash] [-v | --verbose 0..3] [--font name sizeoffset /dir/file.ttf [/dir/bold.ttf [/dir/italic.ttf]]]\n");
			exit(0);
		}
	}
}

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  setup the framebuffer                                      *
*                                                                                     *
**************************************************************************************/

void CNeutrinoApp::SetupFrameBuffer()
{
	frameBuffer->init();
	if(frameBuffer->setMode(720, 576, 8 * sizeof(fb_pixel_t)))
	{
		dprintf(DEBUG_NORMAL, "Error while setting framebuffer mode\n");
		exit(-1);
	}

	//make 1..8 transparent for dummy painting
	for(int count =0;count<8;count++)
		frameBuffer->paletteSetColor(count, 0x000000, 0xffff);
	frameBuffer->paletteSet();
}

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  setup fonts                                                *
*                                                                                     *
**************************************************************************************/

const neutrino_font_descr_struct predefined_font[2] = 
{
	{"Micron"            , {FONTDIR "/micron.ttf"        , FONTDIR "/micron_bold.ttf", FONTDIR "/micron_italic.ttf"}, 0},
	{"MD King KhammuRabi", {FONTDIR "/md_khmurabi_10.ttf", NULL                      , NULL                        }, 0}
};

const char* predefined_lcd_font[2][6] = 
{
	{FONTDIR "/12.pcf.gz", "Fix12", FONTDIR "/14B.pcf.gz", "Fix14", FONTDIR "/15B.pcf.gz", "Fix15"},
	{FONTDIR "/md_khmurabi_10.ttf", "MD King KhammuRabi", NULL, NULL,  NULL, NULL}
};

void CNeutrinoApp::SetupFonts()
{
	const char * style[3];

	if (g_fontRenderer != NULL)
		delete g_fontRenderer;

	g_fontRenderer = new FBFontRenderClass();

	style[0] = g_fontRenderer->AddFont(font.filename[0]);

	style[1] = (font.filename[1] == NULL) ? "Bold Regular" : g_fontRenderer->AddFont(font.filename[1]);

	if (font.filename[2] == NULL)
	{
		g_fontRenderer->AddFont(font.filename[0], true);  // make italics
		style[2] = "Italic";
	}
	else
		style[2] = g_fontRenderer->AddFont(font.filename[2]);

	for (int i = 0; i < FONT_TYPE_COUNT; i++)
	{
		g_Font[i] = g_fontRenderer->getFont(font.name, style[neutrino_font[i].style], configfile.getInt32(neutrino_font[i].name, neutrino_font[i].defaultsize) + neutrino_font[i].size_offset * font.size_offset);
	}
}

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  setup the menu timouts                                     *
*                                                                                     *
**************************************************************************************/

void CNeutrinoApp::SetupTiming()
{
	for (int i = 0; i < TIMING_SETTING_COUNT; i++)
		sprintf(g_settings.timing_string[i], "%d", g_settings.timing[i]);
}


/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  init main menu                                             *
*                                                                                     *
**************************************************************************************/

void CNeutrinoApp::InitMainMenu(CMenuWidget &mainMenu, CMenuWidget &mainSettings,  CMenuWidget &audioSettings, CMenuWidget &parentallockSettings,
				CMenuWidget &networkSettings, CMenuWidget &recordingSettings, CMenuWidget &colorSettings, CMenuWidget &lcdSettings,
				CMenuWidget &keySettings, CMenuWidget &videoSettings, CMenuWidget &languageSettings, CMenuWidget &miscSettings,
				CMenuWidget &service, CMenuWidget &fontSettings, CMenuWidget &mp3picSettings, CMenuWidget &streamingSettings, CMenuWidget &moviePlayer)
{
	dprintf(DEBUG_DEBUG, "init mainmenue\n");
	mainMenu.addItem(GenericMenuSeparator);

	mainMenu.addItem(new CMenuForwarder(LOCALE_MAINMENU_TVMODE, true, NULL, this, "tv", true, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED), true);
	mainMenu.addItem(new CMenuForwarder(LOCALE_MAINMENU_RADIOMODE, true, NULL, this, "radio", true, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));
	mainMenu.addItem(new CMenuForwarder(LOCALE_MAINMENU_SCARTMODE, true, NULL, this, "scart", true, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));
	mainMenu.addItem(new CMenuForwarder(LOCALE_MAINMENU_GAMES, true, NULL, new CGameList(LOCALE_MAINMENU_GAMES), "", true, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));
	mainMenu.addItem(GenericMenuSeparatorLine);
	mainMenu.addItem(new CMenuForwarder(LOCALE_MAINMENU_MP3PLAYER, true, NULL, new CMP3PlayerGui(), NULL, true, CRCInput::RC_1));

	#if HAVE_DVB_API_VERSION >= 3
	//mainMenu.addItem(new CMenuForwarder(LOCALE_MAINMENU_MOVIEPLAYER, true, NULL, new CMoviePlayerGui()));
	mainMenu.addItem(new CMenuForwarder(LOCALE_MAINMENU_MOVIEPLAYER, true, NULL, &moviePlayer, NULL, true, CRCInput::RC_2));

	moviePlayer.addItem(GenericMenuSeparator);
	moviePlayer.addItem(GenericMenuBack);
	moviePlayer.addItem(GenericMenuSeparatorLine);
	CMoviePlayerGui* moviePlayerGui = new CMoviePlayerGui();
	moviePlayer.addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_TSPLAYBACK, true, NULL, moviePlayerGui, "tsplayback", true, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));
	moviePlayer.addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_PESPLAYBACK, true, NULL, moviePlayerGui, "pesplayback", true));
	moviePlayer.addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_BOOKMARK, true, NULL, moviePlayerGui, "bookmarkplayback"));
	moviePlayer.addItem(GenericMenuSeparator);
	moviePlayer.addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_FILEPLAYBACK, true, NULL, moviePlayerGui, "fileplayback", true, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	moviePlayer.addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_DVDPLAYBACK, true, NULL, moviePlayerGui, "dvdplayback", true, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));
	moviePlayer.addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_VCDPLAYBACK, true, NULL, moviePlayerGui, "vcdplayback", true, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));
	moviePlayer.addItem(GenericMenuSeparatorLine);
	moviePlayer.addItem(new CMenuForwarder(LOCALE_MAINMENU_SETTINGS, true, NULL, &streamingSettings, NULL, true, CRCInput::RC_help, NEUTRINO_ICON_BUTTON_HELP_SMALL));
	moviePlayer.addItem(new CMenuForwarder("nfsmenu.head", true, NULL, new CNFSSmallMenu(), NULL, true, CRCInput::RC_setup, NEUTRINO_ICON_BUTTON_DBOX_SMALL));
#endif

	mainMenu.addItem(new CMenuForwarder(LOCALE_MAINMENU_PICTUREVIEWER, true, NULL, new CPictureViewerGui(), NULL, true, CRCInput::RC_3));
	mainMenu.addItem(GenericMenuSeparatorLine);
	
	mainMenu.addItem(new CMenuForwarder(LOCALE_MAINMENU_SETTINGS, true, NULL, &mainSettings, NULL, true, CRCInput::RC_4));
	mainMenu.addItem(new CLockedMenuForwarder(LOCALE_MAINMENU_SERVICE, g_settings.parentallock_pincode, false, true, NULL, &service, NULL, true, CRCInput::RC_5));
	mainMenu.addItem(GenericMenuSeparatorLine);

	mainMenu.addItem(new CMenuForwarder(LOCALE_MAINMENU_SLEEPTIMER, true, NULL, new CSleepTimerWidget, NULL, true, CRCInput::RC_6));
	mainMenu.addItem(new CMenuForwarder(LOCALE_MAINMENU_SHUTDOWN, true, NULL, this, "shutdown", true, CRCInput::RC_standby, "power.raw"));

	mainSettings.addItem(GenericMenuSeparator);
	mainSettings.addItem(GenericMenuBack);
	mainSettings.addItem(GenericMenuSeparatorLine);
	mainSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", true, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	mainSettings.addItem(GenericMenuSeparatorLine);
	mainSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_VIDEO     , true, NULL, &videoSettings    , NULL, true, CRCInput::RC_1));
	mainSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_AUDIO     , true, NULL, &audioSettings    , NULL, true, CRCInput::RC_2));
	if(g_settings.parentallock_prompt)
		mainSettings.addItem(new CLockedMenuForwarder("parentallock.parentallock", g_settings.parentallock_pincode, true, true, NULL, &parentallockSettings, NULL, true, CRCInput::RC_3));
	else
		mainSettings.addItem(new CMenuForwarder("parentallock.parentallock", true, NULL, &parentallockSettings, NULL, true, CRCInput::RC_4));
	mainSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_NETWORK   , true, NULL, &networkSettings  , NULL, true, CRCInput::RC_5));
	mainSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_RECORDING , true, NULL, &recordingSettings, NULL, true, CRCInput::RC_6));
	mainSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_STREAMING , true, NULL, &streamingSettings, NULL, true, CRCInput::RC_7));
	mainSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_LANGUAGE  , true, NULL, &languageSettings , NULL, true, CRCInput::RC_8));
	mainSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_COLORS    , true, NULL, &colorSettings    , NULL, true, CRCInput::RC_9));
	mainSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_LCD       , true, NULL, &lcdSettings      , NULL, true, CRCInput::RC_0));
	mainSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_KEYBINDING, true, NULL, &keySettings      , NULL, true, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));
	mainSettings.addItem(new CMenuForwarder("mp3picsettings.general"      , true, NULL, &mp3picSettings   , NULL, true, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));
	mainSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_MISC      , true, NULL, &miscSettings     , NULL, true, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));
}


void CNeutrinoApp::InitScanSettings(CMenuWidget &settings)
{
	dprintf(DEBUG_DEBUG, "init scansettings\n");
	CMenuOptionChooser* ojBouquets = new CMenuOptionChooser("scants.bouquet", (int *)&scanSettings.bouquetMode, true );
	ojBouquets->addOption(CZapitClient::BM_DELETEBOUQUETS, "scants.bouquet_erase");
	ojBouquets->addOption(CZapitClient::BM_CREATEBOUQUETS, "scants.bouquet_create");
	ojBouquets->addOption(CZapitClient::BM_DONTTOUCHBOUQUETS, "scants.bouquet_leave");
	ojBouquets->addOption(CZapitClient::BM_UPDATEBOUQUETS, "scants.bouquet_update");
	ojBouquets->addOption(CZapitClient::BM_CREATESATELLITEBOUQUET, "scants.bouquet_satellite");

	//sat-lnb-settings
	if(g_info.delivery_system == DVB_S)
	{
		settings.addItem(GenericMenuSeparator);
		settings.addItem(GenericMenuBack);
		settings.addItem(GenericMenuSeparatorLine);

		satList.clear();
		g_Zapit->getScanSatelliteList(satList);

		t_satellite_position currentSatellitePosition = g_Zapit->getCurrentSatellitePosition();

		if (scanSettings.diseqcMode == DISEQC_1_2)
		{
			//printf("[neutrino] received %d sats\n", satList.size());
			for (uint i = 0; i < satList.size(); i++)
			{
				//printf("[neutrino] received %d: %s, %d\n", i, satList[i].satName, satList[i].satPosition);
				scanSettings.satPosition[i] = satList[i].satPosition;
				scanSettings.satMotorPos[i] = satList[i].motorPosition;
				strcpy(scanSettings.satName[i], satList[i].satName);
				//scanSettings.satDiseqc[i] = satList[i].satDiseqc;
				if (satList[i].satPosition == currentSatellitePosition)
					strcpy(scanSettings.satNameNoDiseqc, satList[i].satName);
			}
			for (uint i = satList.size(); i < MAX_SATELLITES; i++)
			{
				scanSettings.satName[i][0] = 0;
				scanSettings.satPosition[i] = 0;
				scanSettings.satDiseqc[i] = -1;
			}
		}

		CMenuOptionStringChooser* ojSat = new CMenuOptionStringChooser("satsetup.satellite", scanSettings.satNameNoDiseqc, (scanSettings.diseqcMode == NO_DISEQC)/*, new CSatelliteNotifier*/, NULL, false);
		for (uint i=0; i < satList.size(); i++)
		{
			ojSat->addOption(satList[i].satName);
			dprintf(DEBUG_DEBUG, "got scanprovider (sat): %s\n", satList[i].satName );
		}

		CMenuOptionChooser* ojDiseqcRepeats = new CMenuOptionChooser("satsetup.diseqcrepeat", (int *)&scanSettings.diseqcRepeat, (scanSettings.diseqcMode != NO_DISEQC) && (scanSettings.diseqcMode != DISEQC_1_0)/*, new CSatelliteNotifier*/, NULL, false);
		ojDiseqcRepeats->addOption(0, "0");
		ojDiseqcRepeats->addOption(1, "1");
		ojDiseqcRepeats->addOption(2, "2");

		CMenuWidget* extSatSettings = new CMenuWidget("satsetup.extended", NEUTRINO_ICON_SETTINGS);
		extSatSettings->addItem(GenericMenuSeparator);
		extSatSettings->addItem(GenericMenuBack);
		extSatSettings->addItem(GenericMenuSeparatorLine);

		CMenuForwarder* ojExtSatSettings = new CMenuForwarder("satsetup.extended", (scanSettings.diseqcMode != NO_DISEQC), NULL, extSatSettings);
		for( uint i=0; i < satList.size(); i++)
		{
			CMenuOptionChooser* oj = new CMenuOptionChooser( satList[i].satName, scanSettings.diseqscOfSat( satList[i].satName), true/*, new CSatelliteNotifier*/);
			oj->addOption( -1, LOCALE_OPTIONS_OFF);
			for(uint j=0; j < satList.size(); j++)
			{
				char jj[2 + 1];
				sprintf( jj, "%d", j + 1);
				oj->addOption( j, jj);
			}
			extSatSettings->addItem( oj);
		}

		CMenuWidget* extMotorSettings = new CMenuWidget("satsetup.extended_motor", NEUTRINO_ICON_SETTINGS);
		extMotorSettings->addItem(GenericMenuSeparator);
		extMotorSettings->addItem(GenericMenuBack);
		extMotorSettings->addItem( new CMenuForwarder("satsetup.savesettingsnow", true, NULL, this, "savesettings") );
		extMotorSettings->addItem( new CMenuForwarder("satsetup.motorcontrol", true, NULL, new CMotorControl()) );
		extMotorSettings->addItem(GenericMenuSeparatorLine);

		CMenuForwarder* ojExtMotorSettings = new CMenuForwarder("satsetup.extended_motor", (scanSettings.diseqcMode == DISEQC_1_2), NULL, extMotorSettings);

		for( uint i=0; i < satList.size(); i++)
		{
			CMenuOptionChooser* oj = new CMenuOptionChooser( satList[i].satName, scanSettings.motorPosOfSat( satList[i].satName), true/*, new CSatelliteNotifier*/);
			oj->addOption(0, LOCALE_OPTIONS_OFF);
			for(uint j=1; j<=satList.size(); j++)
			{
				char jj[2 + 1];
				sprintf(jj, "%d", j);
				oj->addOption(j, jj);
			}
			extMotorSettings->addItem(oj);
		}

		CMenuOptionChooser* ojDiseqc = new CMenuOptionChooser("satsetup.diseqc", (int *)&scanSettings.diseqcMode, true, new CSatDiseqcNotifier( ojSat, ojExtSatSettings, ojExtMotorSettings, ojDiseqcRepeats));
		ojDiseqc->addOption( NO_DISEQC,   "satsetup.nodiseqc");
		ojDiseqc->addOption( MINI_DISEQC, "satsetup.minidiseqc");
		ojDiseqc->addOption( DISEQC_1_0,  "satsetup.diseqc10");
		ojDiseqc->addOption( DISEQC_1_1,  "satsetup.diseqc11");
		ojDiseqc->addOption( DISEQC_1_2,  "satsetup.diseqc12");
		ojDiseqc->addOption( SMATV_REMOTE_TUNING,  "satsetup.smatvremote");

		settings.addItem( ojDiseqc );
		settings.addItem( ojBouquets );
		settings.addItem( ojSat );
		settings.addItem( ojDiseqcRepeats );
		settings.addItem( ojExtSatSettings );
		settings.addItem( ojExtMotorSettings );
		settings.addItem(GenericMenuSeparatorLine);
	}
	else
	{//kabel
		settings.addItem(GenericMenuSeparator);
		settings.addItem(GenericMenuBack);
		settings.addItem(GenericMenuSeparatorLine);

		CZapitClient::SatelliteList providerList;
		g_Zapit->getScanSatelliteList(providerList);

		CMenuOptionStringChooser* oj = new CMenuOptionStringChooser(LOCALE_CABLESETUP_PROVIDER, (char*)&scanSettings.satNameNoDiseqc, true/*, new CCableProviderNotifier*/);

		for( uint i=0; i< providerList.size(); i++)
		{
			oj->addOption( providerList[i].satName);
			dprintf(DEBUG_DEBUG, "got scanprovider (cable): %s\n", providerList[i].satName );
		}
		settings.addItem( ojBouquets);
		settings.addItem( oj);
	}

	settings.addItem(new CMenuForwarder("scants.startnow", true, NULL, new CScanTs()));
}


void CNeutrinoApp::InitServiceSettings(CMenuWidget &service, CMenuWidget &scanSettings)
{
	dprintf(DEBUG_DEBUG, "init serviceSettings\n");
	service.addItem(GenericMenuSeparator);
	service.addItem(GenericMenuBack);
	service.addItem(GenericMenuSeparatorLine);
	service.addItem( new CMenuForwarder(LOCALE_BOUQUETEDITOR_NAME, true, NULL, new CBEBouquetWidget()));
	service.addItem( new CMenuForwarder("servicemenu.scants", true, NULL, &scanSettings ) );
	service.addItem( new CMenuForwarder("servicemenu.reload", true, NULL, this, "reloadchannels" ) );
	service.addItem( new CMenuForwarder("servicemenu.ucodecheck", true, NULL, UCodeChecker ) );

	//softupdate
	if(softupdate)
	{
		dprintf(DEBUG_DEBUG, "init soft-update-stuff\n");
		CMenuWidget* updateSettings = new CMenuWidget("servicemenu.update", "softupdate.raw", 550);
		updateSettings->addItem(GenericMenuSeparator);
		updateSettings->addItem(GenericMenuBack);
		updateSettings->addItem(GenericMenuSeparatorLine);


		//experten-funktionen f�r mtd lesen/schreiben
		CMenuWidget* mtdexpert = new CMenuWidget("flashupdate.expertfunctions", "softupdate.raw");
		mtdexpert->addItem(GenericMenuSeparator);
		mtdexpert->addItem(GenericMenuBack);
		mtdexpert->addItem(GenericMenuSeparatorLine);
		CFlashExpert* fe = new CFlashExpert();
		mtdexpert->addItem( new CMenuForwarder("flashupdate.readflash", true, NULL, fe, "readflash") );
		mtdexpert->addItem( new CMenuForwarder("flashupdate.writeflash", true, NULL, fe, "writeflash") );
		mtdexpert->addItem(GenericMenuSeparatorLine);
		mtdexpert->addItem( new CMenuForwarder("flashupdate.readflashmtd", true, NULL, fe, "readflashmtd") );
		mtdexpert->addItem( new CMenuForwarder("flashupdate.writeflashmtd", true, NULL, fe, "writeflashmtd") );
		mtdexpert->addItem(GenericMenuSeparatorLine);

		CStringInputSMS * updateSettings_url_file = new CStringInputSMS("flashupdate.url_file", g_settings.softupdate_url_file, 30, NULL, NULL, "abcdefghijklmnopqrstuvwxyz0123456789!""�$%&/()=?-. ");
		mtdexpert->addItem(new CMenuForwarder("flashupdate.url_file", true, g_settings.softupdate_url_file, updateSettings_url_file));

		updateSettings->addItem( new CMenuForwarder("flashupdate.expertfunctions", true, NULL, mtdexpert ) );

		updateSettings->addItem(GenericMenuSeparatorLine);
		CMenuOptionChooser *oj = new CMenuOptionChooser("flashupdate.updatemode", &g_settings.softupdate_mode,true);
		oj->addOption(0, "flashupdate.updatemode_manual");
		oj->addOption(1, "flashupdate.updatemode_internet");
		updateSettings->addItem( oj );


		/* show current version */
		updateSettings->addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "flashupdate.currentversion_sep"));

		/* get current version SBBBYYYYMMTTHHMM -- formatsting */
		CConfigFile configfile('\t');

		const char * versionString = (configfile.loadConfig("/.version")) ? (configfile.getString( "version", "????????????????").c_str()) : "????????????????";

		dprintf(DEBUG_INFO, "current flash-version: %s\n", versionString);

		static CFlashVersionInfo versionInfo(versionString);

		updateSettings->addItem(new CMenuForwarder("flashupdate.currentversiondate", false, versionInfo.getDate()));
		updateSettings->addItem(new CMenuForwarder("flashupdate.currentversiontime", false, versionInfo.getTime()));
		updateSettings->addItem(new CMenuForwarder("flashupdate.currentreleasecycle", false, versionInfo.getReleaseCycle()));
		/* versionInfo.getType() returns const char * which is never deallocated */
		updateSettings->addItem(new CMenuForwarder("flashupdate.currentversionsnapshot", false, versionInfo.getType()));

		updateSettings->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "flashupdate.proxyserver_sep") );

		CStringInputSMS * updateSettings_proxy = new CStringInputSMS("flashupdate.proxyserver", g_settings.softupdate_proxyserver, 23, "flashupdate.proxyserver_hint1", "flashupdate.proxyserver_hint2", "abcdefghijklmnopqrstuvwxyz0123456789-.: ");
		updateSettings->addItem( new CMenuForwarder("flashupdate.proxyserver", true, g_settings.softupdate_proxyserver, updateSettings_proxy ) );

		CStringInputSMS * updateSettings_proxyuser = new CStringInputSMS("flashupdate.proxyusername", g_settings.softupdate_proxyusername, 23, "flashupdate.proxyusername_hint1", "flashupdate.proxyusername_hint2", "abcdefghijklmnopqrstuvwxyz0123456789!""�$%&/()=?-. ");
		updateSettings->addItem( new CMenuForwarder("flashupdate.proxyusername", true, g_settings.softupdate_proxyusername, updateSettings_proxyuser ) );

		CStringInputSMS * updateSettings_proxypass = new CStringInputSMS("flashupdate.proxypassword", g_settings.softupdate_proxypassword, 20, "flashupdate.proxypassword_hint1", "flashupdate.proxypassword_hint2", "abcdefghijklmnopqrstuvwxyz0123456789!""�$%&/()=?-. ");
		updateSettings->addItem( new CMenuForwarder("flashupdate.proxypassword", true, g_settings.softupdate_proxypassword, updateSettings_proxypass ) );

		updateSettings->addItem(GenericMenuSeparatorLine);
		updateSettings->addItem(new CMenuForwarder("flashupdate.checkupdate", true, NULL, new CFlashUpdate()));

		service.addItem(new CMenuForwarder("servicemenu.update", true, NULL, updateSettings));
	}
}

void CNeutrinoApp::InitMp3PicSettings(CMenuWidget &mp3PicSettings)
{
	mp3PicSettings.addItem(GenericMenuSeparator);
	mp3PicSettings.addItem(GenericMenuBack);
	
	CMenuOptionChooser *oj = new CMenuOptionChooser("pictureviewer.scaling", &g_settings.picviewer_scaling, true );
	oj->addOption((int)CPictureViewer::SIMPLE, "Simple");
	oj->addOption((int)CPictureViewer::COLOR, "Color Average");
	oj->addOption((int)CPictureViewer::NONE, "None");
	CStringInput * pic_timeout= new CStringInput("pictureviewer.slide_time", g_settings.picviewer_slide_time, 2, NULL, NULL, "0123456789 ");
	mp3PicSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "pictureviewer.head") );
	mp3PicSettings.addItem( oj );
	mp3PicSettings.addItem(new CMenuForwarder("pictureviewer.slide_time", true, g_settings.picviewer_slide_time, pic_timeout));
	mp3PicSettings.addItem(new CMenuForwarder("pictureviewer.defdir", true, g_settings.network_nfs_picturedir, this, "picturedir"));

	oj = new CMenuOptionChooser("mp3player.display_order", &g_settings.mp3player_display, true );
	oj->addOption((int)CMP3PlayerGui::ARTIST_TITLE, "mp3player.artist_title");
	oj->addOption((int)CMP3PlayerGui::TITLE_ARTIST, "mp3player.title_artist");
	mp3PicSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "mp3player.name") );
	mp3PicSettings.addItem( oj );
  	oj = new CMenuOptionChooser("mp3player.follow", &g_settings.mp3player_follow, true );
	oj->addOption(0, LOCALE_MESSAGEBOX_NO);
	oj->addOption(1, LOCALE_MESSAGEBOX_YES);
	mp3PicSettings.addItem( oj );
	CStringInput * mp3_screensaver= new CStringInput("mp3player.screensaver_timeout", g_settings.mp3player_screensaver, 2, NULL, NULL, "0123456789 ");
	mp3PicSettings.addItem(new CMenuForwarder("mp3player.screensaver_timeout", true, g_settings.mp3player_screensaver, mp3_screensaver));
  	oj = new CMenuOptionChooser("mp3player.highprio", &g_settings.mp3player_highprio, true );
	oj->addOption(0, LOCALE_MESSAGEBOX_NO);
	oj->addOption(1, LOCALE_MESSAGEBOX_YES);
	mp3PicSettings.addItem( oj );
	mp3PicSettings.addItem(new CMenuForwarder("mp3player.defdir", true, g_settings.network_nfs_mp3dir, this, "mp3dir"));

	mp3PicSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_FILEBROWSER_HEAD));
	oj = new CMenuOptionChooser(LOCALE_FILEBROWSER_SHOWRIGHTS, &g_settings.filebrowser_showrights, true);
	oj->addOption(0, LOCALE_MESSAGEBOX_NO);
	oj->addOption(1, LOCALE_MESSAGEBOX_YES);
	mp3PicSettings.addItem( oj );

}

const char * misc_setting_files[MISC_SETTING_FILES_COUNT][4] =
{
	{LOCALE_MISCSETTINGS_SPTSMODE     , "/var/etc/.spts_mode"  , LOCALE_OPTIONS_OFF, LOCALE_OPTIONS_ON }, /* cf. #define MISC_SETTING_SPTS_MODE 0 (settings.h) */
	{LOCALE_MISCSETTINGS_BOOTINFO     , "/var/etc/.cdkVcInfo"  , LOCALE_OPTIONS_ON , LOCALE_OPTIONS_OFF},
#if HAVE_DVB_API_VERSION == 1
	{LOCALE_MISCSETTINGS_STARTBHDRIVER, "/var/etc/.bh"         , LOCALE_OPTIONS_OFF, LOCALE_OPTIONS_ON },
#endif
	{LOCALE_MISCSETTINGS_HWSECTIONS   , "/var/etc/.hw_sections", LOCALE_OPTIONS_ON , LOCALE_OPTIONS_OFF}
};

void CNeutrinoApp::InitMiscSettings(CMenuWidget &miscSettings)
{
	dprintf(DEBUG_DEBUG, "init miscsettings\n");
	miscSettings.addItem(GenericMenuSeparator);
	miscSettings.addItem(GenericMenuBack);
	miscSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MISCSETTINGS_GENERAL));

	CMenuOptionChooser *m1 = new CMenuOptionChooser(LOCALE_MISCSETTINGS_SHUTDOWN_REAL_RCDELAY, &g_settings.shutdown_real_rcdelay, !g_settings.shutdown_real );
	m1->addOption(0, LOCALE_OPTIONS_OFF);
	m1->addOption(1, LOCALE_OPTIONS_ON );

	CMiscNotifier* miscNotifier = new CMiscNotifier( m1 );

	CMenuOptionChooser *oj = new CMenuOptionChooser(LOCALE_MISCSETTINGS_SHUTDOWN_REAL, &g_settings.shutdown_real, true, miscNotifier );
	oj->addOption(1, LOCALE_OPTIONS_OFF);
	oj->addOption(0, LOCALE_OPTIONS_ON );
	miscSettings.addItem( oj );

	CMenuOptionChooser *oj2 = new CMenuOptionChooser(LOCALE_MISCSETTINGS_INFOBAR_SAT_DISPLAY, &g_settings.infobar_sat_display, true);
	oj2->addOption(1, LOCALE_OPTIONS_ON );
	oj2->addOption(0, LOCALE_OPTIONS_OFF);
	miscSettings.addItem( oj2 );

	if (fromflash)
	{
		miscSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MISCSETTINGS_DRIVER_BOOT));

		for (int i = 0; i < MISC_SETTING_FILES_COUNT; i++)
		{
			FILE * fd = fopen(misc_setting_files[i][1], "r");
			if (fd)
			{
				fclose(fd);
				g_settings.misc_option[i] = 1;
			}
			else
				g_settings.misc_option[i] = 0;

			oj = new CMenuOptionChooser(misc_setting_files[i][0], &(g_settings.misc_option[i]), true, new CTouchFileNotifier(misc_setting_files[i][1]));
			oj->addOption(0, misc_setting_files[i][2]);
			oj->addOption(1, misc_setting_files[i][3]);
			miscSettings.addItem(oj);
		}

		oj = new CMenuOptionChooser(LOCALE_MISCSETTINGS_FB_DESTINATION, &g_settings.uboot_console, true, ConsoleDestinationChanger );
		oj->addOption(0, LOCALE_OPTIONS_NULL  );
		oj->addOption(1, LOCALE_OPTIONS_SERIAL);
		oj->addOption(2, LOCALE_OPTIONS_FB    );
		miscSettings.addItem( oj );
	}

	keySetupNotifier = new CKeySetupNotifier;
	CStringInput * keySettings_repeat_genericblocker = new CStringInput("keybindingmenu.repeatblockgeneric", g_settings.repeat_blocker, 3, "repeatblocker.hint_1", "repeatblocker.hint_2", "0123456789 ", keySetupNotifier);
	CStringInput * keySettings_repeatBlocker = new CStringInput("keybindingmenu.repeatblock", g_settings.repeat_genericblocker, 3, "repeatblocker.hint_1", "repeatblocker.hint_2", "0123456789 ", keySetupNotifier);
	keySetupNotifier->changeNotify("initial", NULL);

	miscSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "keybindingmenu.RC") );
	miscSettings.addItem(new CMenuForwarder("keybindingmenu.repeatblock", true, g_settings.repeat_genericblocker, keySettings_repeatBlocker));
 	miscSettings.addItem(new CMenuForwarder("keybindingmenu.repeatblockgeneric", true, g_settings.repeat_blocker, keySettings_repeat_genericblocker));
	miscSettings.addItem( m1 );
}


void CNeutrinoApp::InitLanguageSettings(CMenuWidget &languageSettings)
{
	languageSettings.addItem(GenericMenuSeparator);
	languageSettings.addItem(GenericMenuBack);
	languageSettings.addItem(GenericMenuSeparatorLine);

	CMenuOptionStringChooser* oj = new CMenuOptionStringChooser(LOCALE_LANGUAGESETUP_SELECT, (char*)&g_settings.language, true, this, false);
	//search available languages....

	struct dirent **namelist;
	int n;
	//		printf("scanning locale dir now....(perhaps)\n");

	char *pfad[] = {DATADIR "/neutrino/locale","/var/tuxbox/config/locale"};
	char * locale;	

	for(int p = 0;p < 2;p++)
	{
		n = scandir(pfad[p], &namelist, 0, alphasort);
		if(n < 0)
		{
			perror("loading locales: scandir");
		}
		else
		{
			for(int count=0;count<n;count++)
			{
				locale = namelist[count]->d_name;
				char * pos = strstr(locale, ".locale");
				if(pos != NULL)
				{
					*pos = '\0';
					oj->addOption(locale);
				}
				free(namelist[count]);
			}
			free(namelist);
		}
	}
	languageSettings.addItem( oj );
}

void CNeutrinoApp::InitAudioSettings(CMenuWidget &audioSettings, CAudioSetupNotifier* audioSetupNotifier)
{
	audioSettings.addItem(GenericMenuSeparator);
	audioSettings.addItem(GenericMenuBack);
	audioSettings.addItem(GenericMenuSeparatorLine);

	CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_AUDIOMENU_ANALOGOUT, &g_settings.audio_AnalogMode, true, audioSetupNotifier);
	oj->addOption(0, LOCALE_AUDIOMENU_STEREO);
	oj->addOption(1, LOCALE_AUDIOMENU_MONOLEFT);
	oj->addOption(2, LOCALE_AUDIOMENU_MONORIGHT);

	audioSettings.addItem( oj );

	oj = new CMenuOptionChooser(LOCALE_AUDIOMENU_DOLBYDIGITAL, &g_settings.audio_DolbyDigital, true, audioSetupNotifier);
	oj->addOption(0, LOCALE_OPTIONS_OFF);
	oj->addOption(1, LOCALE_OPTIONS_ON );
	audioSettings.addItem( oj );

	CStringInput * audio_PCMOffset = new CStringInput(LOCALE_AUDIOMENU_PCMOFFSET, g_settings.audio_PCMOffset, 2, NULL, NULL, "0123456789 ", audioSetupNotifier);
	CMenuForwarder *mf = new CMenuForwarder(LOCALE_AUDIOMENU_PCMOFFSET, (g_settings.audio_avs_Control == CControld::TYPE_LIRC), g_settings.audio_PCMOffset, audio_PCMOffset );
	CAudioSetupNotifier2 *audioSetupNotifier2 = new CAudioSetupNotifier2(mf);

	oj = new CMenuOptionChooser(LOCALE_AUDIOMENU_AVS_CONTROL, &g_settings.audio_avs_Control, true, audioSetupNotifier2);
	oj->addOption(CControld::TYPE_OST, LOCALE_AUDIOMENU_OST);
	oj->addOption(CControld::TYPE_AVS, LOCALE_AUDIOMENU_AVS);
	oj->addOption(CControld::TYPE_LIRC, LOCALE_AUDIOMENU_LIRC);
	audioSettings.addItem(oj);
	audioSettings.addItem(mf);
}

void CNeutrinoApp::InitVideoSettings(CMenuWidget &videoSettings)
{
	videoSettings.addItem(GenericMenuSeparator);
	videoSettings.addItem(GenericMenuBack);
	videoSettings.addItem(GenericMenuSeparatorLine);

	CRGBCSyncControler* sc = new CRGBCSyncControler(LOCALE_VIDEOMENU_RGB_CENTERING, &g_settings.video_csync);
	bool bVisible = ( g_settings.video_Signal == 1 ) || ( g_settings.video_Signal == 3 ) || ( g_settings.video_Signal == 4 );  
	CMenuForwarder* scf = new CMenuForwarder(LOCALE_VIDEOMENU_RGB_CENTERING, bVisible, NULL, sc);
	
	CVideoSetupNotifier * videoSetupNotifier = new CVideoSetupNotifier(scf);
	CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_VIDEOMENU_VIDEOSIGNAL, &g_settings.video_Signal, true, videoSetupNotifier);
	oj->addOption(1, LOCALE_VIDEOMENU_VIDEOSIGNAL_RGB      );
	oj->addOption(2, LOCALE_VIDEOMENU_VIDEOSIGNAL_SVIDEO   );
	oj->addOption(3, LOCALE_VIDEOMENU_VIDEOSIGNAL_YUV_V    );
	oj->addOption(4, LOCALE_VIDEOMENU_VIDEOSIGNAL_YUV_C    );
	oj->addOption(0, LOCALE_VIDEOMENU_VIDEOSIGNAL_COMPOSITE);
	videoSettings.addItem( oj );

	videoSettings.addItem(scf);

	oj = new CMenuOptionChooser(LOCALE_VIDEOMENU_VIDEOFORMAT, &g_settings.video_Format, true, videoSetupNotifier);
	oj->addOption(2, LOCALE_VIDEOMENU_VIDEOFORMAT_43        );
	oj->addOption(3, LOCALE_VIDEOMENU_VIDEOFORMAT_431       );
	oj->addOption(1, LOCALE_VIDEOMENU_VIDEOFORMAT_169       );
	oj->addOption(0, LOCALE_VIDEOMENU_VIDEOFORMAT_AUTODETECT);

	if (g_settings.video_Format == 0) // autodetect has to be initialized
	{
		videoSetupNotifier->changeNotify(LOCALE_VIDEOMENU_VIDEOFORMAT, NULL);
	}

	videoSettings.addItem( oj );

	oj = new CMenuOptionChooser(LOCALE_VIDEOMENU_VCRSWITCH, &g_settings.vcr_AutoSwitch, true);
	oj->addOption(0, LOCALE_OPTIONS_OFF);
	oj->addOption(1, LOCALE_OPTIONS_ON );
	videoSettings.addItem( oj );

	videoSettings.addItem(GenericMenuSeparatorLine);
	videoSettings.addItem(new CMenuForwarder(LOCALE_VIDEOMENU_SCREENSETUP, true, NULL, new CScreenSetup()));
}

void CNeutrinoApp::InitParentalLockSettings(CMenuWidget &parentallockSettings)
{
	parentallockSettings.addItem(GenericMenuSeparator);
	parentallockSettings.addItem(GenericMenuBack);
	parentallockSettings.addItem(GenericMenuSeparatorLine);

	CMenuOptionChooser* oj = new CMenuOptionChooser("parentallock.prompt", &g_settings.parentallock_prompt, !parentallocked);
	oj->addOption(PARENTALLOCK_PROMPT_NEVER         , "parentallock.never");
//	oj->addOption(PARENTALLOCK_PROMPT_ONSTART       , "parentallock.onstart");
	oj->addOption(PARENTALLOCK_PROMPT_CHANGETOLOCKED, "parentallock.changetolocked");
	oj->addOption(PARENTALLOCK_PROMPT_ONSIGNAL      , "parentallock.onsignal");
	parentallockSettings.addItem( oj );

	oj = new CMenuOptionChooser("parentallock.lockage", &g_settings.parentallock_lockage, !parentallocked);
	oj->addOption(12, "parentallock.lockage12");
	oj->addOption(16, "parentallock.lockage16");
	oj->addOption(18, "parentallock.lockage18");
	parentallockSettings.addItem( oj );

	CPINChangeWidget * pinChangeWidget = new CPINChangeWidget("parentallock.changepin", g_settings.parentallock_pincode, 4, "parentallock.changepin_hint1");
	parentallockSettings.addItem( new CMenuForwarder("parentallock.changepin", true, g_settings.parentallock_pincode, pinChangeWidget));
}

void CNeutrinoApp::InitNetworkSettings(CMenuWidget &networkSettings)
{
	networkSettings.addItem(GenericMenuSeparator);
	networkSettings.addItem(GenericMenuBack);
	networkSettings.addItem(GenericMenuSeparatorLine);

	network_automatic_start = networkConfig.automatic_start ? 1 : 0;
	CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_NETWORKMENU_SETUPONSTARTUP, &network_automatic_start, true);
	oj->addOption(0, LOCALE_OPTIONS_OFF);
	oj->addOption(1, LOCALE_OPTIONS_ON );

	networkSettings.addItem( oj );
	networkSettings.addItem(new CMenuForwarder(LOCALE_NETWORKMENU_TEST, true, NULL, this, "networktest"));
	networkSettings.addItem(new CMenuForwarder(LOCALE_NETWORKMENU_SHOW, true, NULL, this, "networkshow"));
	CMenuForwarder *m0 = new CMenuForwarder(LOCALE_NETWORKMENU_SETUPNOW, true, NULL, this, "network");
	networkSettings.addItem( m0 );

	networkSettings.addItem(GenericMenuSeparatorLine);

	CIPInput * networkSettings_NetworkIP  = new CIPInput(LOCALE_NETWORKMENU_IPADDRESS , networkConfig.address   , LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2, MyIPChanger);
	CIPInput * networkSettings_NetMask    = new CIPInput(LOCALE_NETWORKMENU_NETMASK   , networkConfig.netmask   , LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	CIPInput * networkSettings_Broadcast  = new CIPInput(LOCALE_NETWORKMENU_BROADCAST , networkConfig.broadcast , LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	CIPInput * networkSettings_Gateway    = new CIPInput(LOCALE_NETWORKMENU_GATEWAY   , networkConfig.gateway   , LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	CIPInput * networkSettings_NameServer = new CIPInput(LOCALE_NETWORKMENU_NAMESERVER, networkConfig.nameserver, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);

	CMenuForwarder *m1 = new CMenuForwarder(LOCALE_NETWORKMENU_IPADDRESS , networkConfig.inet_static, networkConfig.address   , networkSettings_NetworkIP );
	CMenuForwarder *m2 = new CMenuForwarder(LOCALE_NETWORKMENU_NETMASK   , networkConfig.inet_static, networkConfig.netmask   , networkSettings_NetMask   );
	CMenuForwarder *m3 = new CMenuForwarder(LOCALE_NETWORKMENU_BROADCAST , networkConfig.inet_static, networkConfig.broadcast , networkSettings_Broadcast );
	CMenuForwarder *m4 = new CMenuForwarder(LOCALE_NETWORKMENU_GATEWAY   , networkConfig.inet_static, networkConfig.gateway   , networkSettings_Gateway   );
	CMenuForwarder *m5 = new CMenuForwarder(LOCALE_NETWORKMENU_NAMESERVER, networkConfig.inet_static, networkConfig.nameserver, networkSettings_NameServer);

	CDHCPNotifier* dhcpNotifier = new CDHCPNotifier(m1,m2,m3,m4,m5);

	network_dhcp = networkConfig.inet_static ? 0 : 1;
	oj = new CMenuOptionChooser("dhcp", &network_dhcp, true, dhcpNotifier);
	oj->addOption(0, LOCALE_OPTIONS_OFF);
	oj->addOption(1, LOCALE_OPTIONS_ON );
	networkSettings.addItem( oj );
	networkSettings.addItem(GenericMenuSeparatorLine);

	networkSettings.addItem( m1);
	networkSettings.addItem( m2);
	networkSettings.addItem( m3);

	networkSettings.addItem(GenericMenuSeparatorLine);
	networkSettings.addItem( m4);
	networkSettings.addItem( m5);
	networkSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "NFS/CIFS") );
	networkSettings.addItem(new CMenuForwarder(LOCALE_NFS_MOUNT , true, NULL, new CNFSMountGui()));
	networkSettings.addItem(new CMenuForwarder(LOCALE_NFS_UMOUNT, true, NULL, new CNFSUmountGui()));
}

void CNeutrinoApp::InitRecordingSettings(CMenuWidget &recordingSettings)
{
	CIPInput * recordingSettings_server_ip = new CIPInput("recordingmenu.server_ip", g_settings.recording_server_ip, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	CStringInput * recordingSettings_server_port = new CStringInput("recordingmenu.server_port", g_settings.recording_server_port, 6, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2, "0123456789 ");

	CMenuForwarder * mf1 = new CMenuForwarder("recordingmenu.server_ip", (g_settings.recording_type == RECORDING_SERVER), g_settings.recording_server_ip, recordingSettings_server_ip);
	CMenuForwarder * mf2 = new CMenuForwarder("recordingmenu.server_port", (g_settings.recording_type == RECORDING_SERVER), g_settings.recording_server_port, recordingSettings_server_port);

	CMACInput * recordingSettings_server_mac = new CMACInput("recordingmenu.server_mac",  g_settings.recording_server_mac, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	CMenuForwarder * mf3 = new CMenuForwarder("recordingmenu.server_mac", ((g_settings.recording_type == RECORDING_SERVER) && g_settings.recording_server_wakeup==1), g_settings.recording_server_mac, recordingSettings_server_mac);
	CRecordingNotifier2 * RecordingNotifier2 = new CRecordingNotifier2(mf3);
	CMenuOptionChooser * oj2 = new CMenuOptionChooser("recordingmenu.server_wakeup", &g_settings.recording_server_wakeup, (g_settings.recording_type == RECORDING_SERVER), RecordingNotifier2);
	oj2->addOption(0, LOCALE_OPTIONS_OFF);
	oj2->addOption(1, LOCALE_OPTIONS_ON );


	CMenuOptionChooser* oj3 = new CMenuOptionChooser("recordingmenu.stopplayback", &g_settings.recording_stopplayback, (g_settings.recording_type == RECORDING_SERVER));
	oj3->addOption(0, LOCALE_OPTIONS_OFF);
	oj3->addOption(1, LOCALE_OPTIONS_ON );

	CMenuOptionChooser* oj4 = new CMenuOptionChooser("recordingmenu.stopsectionsd", &g_settings.recording_stopsectionsd, (g_settings.recording_type == RECORDING_SERVER));
	oj4->addOption(0, LOCALE_OPTIONS_OFF);
	oj4->addOption(1, LOCALE_OPTIONS_ON );

	CMenuOptionChooser* oj5 = new CMenuOptionChooser("recordingmenu.no_scart", &g_settings.recording_vcr_no_scart, (g_settings.recording_type == RECORDING_VCR));
	oj5->addOption(0, LOCALE_OPTIONS_OFF);
	oj5->addOption(1, LOCALE_OPTIONS_ON );

	int pre,post;
	g_Timerd->getRecordingSafety(pre,post);
	sprintf(g_settings.record_safety_time_before, "%02d", pre/60);
	sprintf(g_settings.record_safety_time_after, "%02d", post/60);
	CRecordingSafetyNotifier *RecordingSafetyNotifier = new CRecordingSafetyNotifier;
	CStringInput * timerSettings_record_safety_time_before = new CStringInput("timersettings.record_safety_time_before", g_settings.record_safety_time_before, 2, "timersettings.record_safety_time_before.hint_1", "timersettings.record_safety_time_before.hint_2","0123456789 ", RecordingSafetyNotifier);
	CMenuForwarder *mf5 = new CMenuForwarder("timersettings.record_safety_time_before", true, g_settings.record_safety_time_before, timerSettings_record_safety_time_before );

	CStringInput * timerSettings_record_safety_time_after = new CStringInput("timersettings.record_safety_time_after", g_settings.record_safety_time_after, 2, "timersettings.record_safety_time_after.hint_1", "timersettings.record_safety_time_after.hint_2","0123456789 ", RecordingSafetyNotifier);
	CMenuForwarder *mf6 = new CMenuForwarder("timersettings.record_safety_time_after", true, g_settings.record_safety_time_after, timerSettings_record_safety_time_after );

	// for direct recording
	CMenuForwarder* mf7 = new CMenuForwarder("recordingmenu.defdir", (g_settings.recording_type == RECORDING_FILE), g_settings.network_nfs_recordingdir,this,"recordingdir");
	CStringInput * recordingSettings_splitsize = new CStringInput("recordingmenu.splitsize", g_settings.recording_splitsize, 6, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2, "0123456789 ");
	CMenuForwarder* mf8 = new CMenuForwarder("recordingmenu.splitsize", (g_settings.recording_type == RECORDING_FILE), g_settings.recording_splitsize,recordingSettings_splitsize);

	CMenuOptionChooser* oj6 = new CMenuOptionChooser("recordingmenu.stream_all_audio_pids", &g_settings.recording_stream_all_audio_pids, (g_settings.recording_type == RECORDING_FILE));
	oj6->addOption(0, LOCALE_OPTIONS_OFF);
	oj6->addOption(1, LOCALE_OPTIONS_ON );
	
	CRecordingNotifier *RecordingNotifier = new CRecordingNotifier(mf1,mf2,oj2,mf3,oj3,oj4,oj5,mf7,mf8,oj6);

	CMenuOptionChooser* oj1 = new CMenuOptionChooser("recordingmenu.recording_type", &g_settings.recording_type, true, RecordingNotifier);
	oj1->addOption(RECORDING_OFF   , "recordingmenu.off"   );
	oj1->addOption(RECORDING_SERVER, "recordingmenu.server");
	oj1->addOption(RECORDING_VCR   , "recordingmenu.vcr"   );
	oj1->addOption(RECORDING_FILE  , "recordingmenu.file"  );

	recordingSettings.addItem(GenericMenuSeparator);
	recordingSettings.addItem(GenericMenuBack);
	recordingSettings.addItem(new CMenuForwarder("recordingmenu.setupnow", true, NULL, this, "recording"));
	recordingSettings.addItem(GenericMenuSeparatorLine);
	recordingSettings.addItem( oj1);
	recordingSettings.addItem(GenericMenuSeparatorLine);
	recordingSettings.addItem( mf1);
	recordingSettings.addItem( mf2);
	recordingSettings.addItem( oj2);
	recordingSettings.addItem( mf3);
	recordingSettings.addItem( oj3 );
	recordingSettings.addItem( oj4);
	recordingSettings.addItem(GenericMenuSeparatorLine);
	recordingSettings.addItem( oj5);
	recordingSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "timersettings.separator") );
	recordingSettings.addItem( mf5);
	recordingSettings.addItem( mf6);
	recordingSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "recordingmenu.filesettingsseparator") );
	recordingSettings.addItem( mf7);
	recordingSettings.addItem( mf8);
	recordingSettings.addItem( oj6);

	recordingstatus = 0;
}

void CNeutrinoApp::InitStreamingSettings(CMenuWidget &streamingSettings)
{
	CIPInput * streamingSettings_server_ip = new CIPInput("streamingmenu.server_ip",  g_settings.streaming_server_ip, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	CStringInput * streamingSettings_server_port = new CStringInput("streamingmenu.server_port", g_settings.streaming_server_port, 6, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2,"0123456789 ");
 	CStringInputSMS * cddriveInput = new CStringInputSMS("streamingmenu.streaming_server_cddrive", g_settings.streaming_server_cddrive, 20, NULL, NULL, "abcdefghijklmnopqrstuvwxyz0123456789!""�$%&/()=?-:\\ ");
	CStringInput * streamingSettings_videorate = new CStringInput("streamingmenu.streaming_videorate", g_settings.streaming_videorate, 5, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2,"0123456789 ");
	CStringInput * streamingSettings_audiorate = new CStringInput("streamingmenu.streaming_audiorate", g_settings.streaming_audiorate, 5, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2,"0123456789 ");
	CStringInputSMS * startdirInput = new CStringInputSMS("streamingmenu.streaming_server_startdir", g_settings.streaming_server_startdir, 30, NULL, NULL,"abcdefghijklmnopqrstuvwxyz0123456789!""�$%&/()=?-:\\ ");

	CMenuForwarder* mf1 = new CMenuForwarder("streamingmenu.server_ip", (g_settings.streaming_type==1), g_settings.streaming_server_ip,streamingSettings_server_ip);
	CMenuForwarder* mf2 = new CMenuForwarder("streamingmenu.server_port", (g_settings.streaming_type==1), g_settings.streaming_server_port,streamingSettings_server_port);
	CMenuForwarder* mf3 = new CMenuForwarder("streamingmenu.streaming_server_cddrive", (g_settings.streaming_type==1), g_settings.streaming_server_cddrive,cddriveInput);
	CMenuForwarder* mf4 = new CMenuForwarder("streamingmenu.streaming_videorate", (g_settings.streaming_type==1), g_settings.streaming_videorate,streamingSettings_videorate);
	CMenuForwarder* mf5 = new CMenuForwarder("streamingmenu.streaming_audiorate", (g_settings.streaming_type==1), g_settings.streaming_audiorate,streamingSettings_audiorate);
	CMenuForwarder* mf6 = new CMenuForwarder("streamingmenu.streaming_server_startdir", (g_settings.streaming_type==1), g_settings.streaming_server_startdir,startdirInput);
	CMenuForwarder* mf7 = new CMenuForwarder(LOCALE_MOVIEPLAYER_DEFDIR, true, g_settings.network_nfs_moviedir,this,"moviedir");
 
	CMenuOptionChooser* oj1 = new CMenuOptionChooser("streamingmenu.streaming_transcode_audio", &g_settings.streaming_transcode_audio, true);
	oj1->addOption(0, LOCALE_MESSAGEBOX_NO);
	oj1->addOption(1, LOCALE_MESSAGEBOX_YES);
                                             
	CMenuOptionChooser* oj2 = new CMenuOptionChooser("streamingmenu.streaming_force_avi_rawaudio", &g_settings.streaming_force_avi_rawaudio, true);
	oj2->addOption(0, LOCALE_MESSAGEBOX_NO);
	oj2->addOption(1, LOCALE_MESSAGEBOX_YES);
                                             
	CMenuOptionChooser* oj3 = new CMenuOptionChooser("streamingmenu.streaming_force_transcode_video", &g_settings.streaming_force_transcode_video, true);
	oj3->addOption(0, LOCALE_MESSAGEBOX_NO);
	oj3->addOption(1, LOCALE_MESSAGEBOX_YES);

// not yet supported by VLC
	CMenuOptionChooser* oj4 = new CMenuOptionChooser("streamingmenu.streaming_transcode_video_codec", &g_settings.streaming_transcode_video_codec, true);
	oj4->addOption(0, "streamingmenu.mpeg1");
	oj4->addOption(1, "streamingmenu.mpeg2");

  CMenuOptionChooser* oj5 = new CMenuOptionChooser("streamingmenu.streaming_resolution", &g_settings.streaming_resolution, true);
	oj5->addOption(0, "streamingmenu.352x288");
	oj5->addOption(1, "streamingmenu.352x576");
	oj5->addOption(2, "streamingmenu.480x576");
	oj5->addOption(3, "streamingmenu.704x576");

  CStreamingNotifier *StreamingNotifier = new CStreamingNotifier(mf1,mf2,mf3,mf4,mf5,mf6,mf7,oj1,oj2,oj3,oj4,oj5);
	CMenuOptionChooser* oj0 = new CMenuOptionChooser("streamingmenu.streaming_type", &g_settings.streaming_type, true, StreamingNotifier);
	oj0->addOption(0, "streamingmenu.off");
	oj0->addOption(1, "streamingmenu.on");


	streamingSettings.addItem(GenericMenuSeparator);
	streamingSettings.addItem(GenericMenuBack);
	streamingSettings.addItem(GenericMenuSeparatorLine);
	streamingSettings.addItem( oj0);                          //Streaming Type
	streamingSettings.addItem(GenericMenuSeparatorLine);
	streamingSettings.addItem( mf1);                          //Server IP
	streamingSettings.addItem( mf2);                          //Server Port
	streamingSettings.addItem( mf3);                          //CD-Drive
	streamingSettings.addItem( mf6);                          //Startdir
	streamingSettings.addItem(GenericMenuSeparatorLine);
	streamingSettings.addItem( mf4);                          //Video-Rate
	streamingSettings.addItem( oj3);
	streamingSettings.addItem( oj4);                          
	streamingSettings.addItem( oj5);                          
	streamingSettings.addItem(GenericMenuSeparatorLine);
	streamingSettings.addItem( mf5);                          //Audiorate
	streamingSettings.addItem( oj1);                          
	streamingSettings.addItem( oj2);                          
	streamingSettings.addItem(GenericMenuSeparatorLine);
	streamingSettings.addItem( mf7);                          //default dir
}


class CMenuNumberInput : public CMenuForwarder, CMenuTarget, CChangeObserver
{
private:
	CChangeObserver * observer;
	CConfigFile     * configfile;
	int32_t           defaultvalue;
	char              value[11];

protected:

	virtual const char * getOption(void)
		{
			sprintf(value, "%u", configfile->getInt32(text, defaultvalue));
			return value;
		}

	virtual bool changeNotify(const std::string & OptionName, void * Data)
		{
			configfile->setInt32(text, atoi(value));
			return observer->changeNotify(OptionName, Data);
		}
  

public:
	CMenuNumberInput(const char * const Text, const int32_t DefaultValue, CChangeObserver * const _observer, CConfigFile * const _configfile) : CMenuForwarder(Text, true, NULL, this)
		{
			observer     = _observer;
			configfile   = _configfile;
			defaultvalue = DefaultValue;
		}

	int exec(CMenuTarget * parent, const std::string & actionKey)
		{
			CStringInput input(text.c_str(), (char *)getOption(), 3, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2, "0123456789 ", this);
			return input.exec(parent, actionKey);
		}
};

void CNeutrinoApp::AddFontSettingItem(CMenuWidget &fontSettings, const SNeutrinoSettings::FONT_TYPES number_of_fontsize_entry)
{
	fontSettings.addItem(new CMenuNumberInput(neutrino_font[number_of_fontsize_entry].name, neutrino_font[number_of_fontsize_entry].defaultsize, this, &configfile));
}


typedef struct font_sizes_groups
{
	const char * const                                groupname;
	const unsigned int                                count;
	const SNeutrinoSettings::FONT_TYPES * const content;
	const char * const                                actionkey;
} font_sizes_groups_struct;

const SNeutrinoSettings::FONT_TYPES channellist_font_sizes[4] =
{
	SNeutrinoSettings::FONT_TYPE_CHANNELLIST,
	SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR,
	SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER,
	SNeutrinoSettings::FONT_TYPE_CHANNEL_NUM_ZAP
};

const SNeutrinoSettings::FONT_TYPES eventlist_font_sizes[4] =
{
	SNeutrinoSettings::FONT_TYPE_EVENTLIST_TITLE,
	SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE,
	SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMSMALL,
	SNeutrinoSettings::FONT_TYPE_EVENTLIST_DATETIME,
};

const SNeutrinoSettings::FONT_TYPES infobar_font_sizes[4] =
{
	SNeutrinoSettings::FONT_TYPE_INFOBAR_NUMBER,
	SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME,
	SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO,
	SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL
};

const SNeutrinoSettings::FONT_TYPES epg_font_sizes[4] =
{
	SNeutrinoSettings::FONT_TYPE_EPG_TITLE,
	SNeutrinoSettings::FONT_TYPE_EPG_INFO1,
	SNeutrinoSettings::FONT_TYPE_EPG_INFO2,
	SNeutrinoSettings::FONT_TYPE_EPG_DATE
};

const SNeutrinoSettings::FONT_TYPES gamelist_font_sizes[2] =
{
	SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMLARGE,
	SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMSMALL
};

const SNeutrinoSettings::FONT_TYPES other_font_sizes[4] =
{
	SNeutrinoSettings::FONT_TYPE_MENU,
	SNeutrinoSettings::FONT_TYPE_MENU_TITLE,
	SNeutrinoSettings::FONT_TYPE_MENU_INFO,
	SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM
};

const font_sizes_groups font_sizes_groups[6] =
{
	{"fontmenu.channellist", 4, channellist_font_sizes, "fontsize.dcha"},
	{"fontmenu.eventlist"  , 4, eventlist_font_sizes  , "fontsize.deve"},
	{"fontmenu.epg"        , 4, epg_font_sizes        , "fontsize.depg"},
	{"fontmenu.infobar"    , 4, infobar_font_sizes    , "fontsize.dinf"},
	{"fontmenu.gamelist"   , 2, gamelist_font_sizes   , "fontsize.dgam"},
	{NULL                  , 4, other_font_sizes      , "fontsize.doth"}
};

void CNeutrinoApp::InitFontSettings(CMenuWidget &fontSettings)
{
	fontSettings.addItem(GenericMenuSeparator);
	fontSettings.addItem(GenericMenuBack);
	fontSettings.addItem(GenericMenuSeparatorLine);
	AddFontSettingItem(fontSettings, SNeutrinoSettings::FONT_TYPE_MENU);
	AddFontSettingItem(fontSettings, SNeutrinoSettings::FONT_TYPE_MENU_TITLE);
	AddFontSettingItem(fontSettings, SNeutrinoSettings::FONT_TYPE_MENU_INFO);
	fontSettings.addItem(GenericMenuSeparatorLine);

	for (int i = 0; i < 5; i++)
	{
		CMenuWidget * fontSettingsSubMenu = new CMenuWidget(font_sizes_groups[i].groupname, "colors.raw");
		fontSettingsSubMenu->addItem(GenericMenuSeparator);
		fontSettingsSubMenu->addItem(GenericMenuBack);
		fontSettingsSubMenu->addItem(GenericMenuSeparatorLine);
		for (unsigned int j = 0; j < font_sizes_groups[i].count; j++)
		{
			AddFontSettingItem(*fontSettingsSubMenu, font_sizes_groups[i].content[j]);
		}
		fontSettingsSubMenu->addItem(GenericMenuSeparatorLine);
		fontSettingsSubMenu->addItem(new CMenuForwarder(LOCALE_OPTIONS_DEFAULT, true, NULL, this, font_sizes_groups[i].actionkey, this));

		fontSettings.addItem(new CMenuForwarder(font_sizes_groups[i].groupname, true, NULL, fontSettingsSubMenu));
	}

	AddFontSettingItem(fontSettings, SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM);
	fontSettings.addItem(GenericMenuSeparatorLine);
	fontSettings.addItem(new CMenuForwarder(LOCALE_OPTIONS_DEFAULT, true, NULL, this, font_sizes_groups[5].actionkey, this) );
}

void CNeutrinoApp::InitColorSettings(CMenuWidget &colorSettings, CMenuWidget &fontSettings )
{
	colorSettings.addItem(GenericMenuSeparator);
	colorSettings.addItem(GenericMenuBack);
	colorSettings.addItem(GenericMenuSeparatorLine);

	CMenuWidget *colorSettings_Themes = new CMenuWidget(LOCALE_COLORTHEMEMENU_HEAD, NEUTRINO_ICON_SETTINGS);
	InitColorThemesSettings(*colorSettings_Themes);

	colorSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_THEMESELECT, true, NULL, colorSettings_Themes) );
	CMenuWidget *colorSettings_menuColors = new CMenuWidget(LOCALE_COLORMENUSETUP_HEAD, NEUTRINO_ICON_SETTINGS, 400, 400);
	InitColorSettingsMenuColors(*colorSettings_menuColors);
	colorSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_MENUCOLORS, true, NULL, colorSettings_menuColors) );

	CMenuWidget *colorSettings_statusbarColors = new CMenuWidget(LOCALE_COLORMENU_STATUSBAR, NEUTRINO_ICON_SETTINGS);
	InitColorSettingsStatusBarColors(*colorSettings_statusbarColors);
	colorSettings.addItem( new CMenuForwarder(LOCALE_COLORSTATUSBAR_HEAD, true, NULL, colorSettings_statusbarColors) );

	colorSettings.addItem(GenericMenuSeparatorLine);
	colorSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_FONT, true, NULL, &fontSettings) );
	CMenuWidget *colorSettings_timing = new CMenuWidget(LOCALE_COLORMENU_TIMING, NEUTRINO_ICON_SETTINGS);
	InitColorSettingsTiming(*colorSettings_timing);
	colorSettings.addItem(new CMenuForwarder(LOCALE_TIMING_HEAD, true, NULL, colorSettings_timing));

	colorSettings.addItem(GenericMenuSeparatorLine);
	if ((g_info.box_Type == CControld::TUXBOX_MAKER_PHILIPS) || (g_info.box_Type == CControld::TUXBOX_MAKER_SAGEM)) // eNX
	{
		CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_COLORMENU_FADE, &g_settings.widget_fade, true );
		oj->addOption(0, LOCALE_OPTIONS_OFF);
		oj->addOption(1, LOCALE_OPTIONS_ON );
		colorSettings.addItem( oj );
	}
	else // GTX, ...
	{
		CAlphaSetup* chAlphaSetup = new CAlphaSetup(LOCALE_COLORMENU_GTX_ALPHA, &g_settings.gtx_alpha1, &g_settings.gtx_alpha2);
		colorSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_GTX_ALPHA, true, NULL, chAlphaSetup));
	}
}


void CNeutrinoApp::InitColorThemesSettings(CMenuWidget &colorSettings_Themes)
{
	colorSettings_Themes.addItem(GenericMenuSeparator);
	colorSettings_Themes.addItem(GenericMenuBack);
	colorSettings_Themes.addItem(GenericMenuSeparatorLine);
	colorSettings_Themes.addItem(new CMenuForwarder(LOCALE_COLORTHEMEMENU_NEUTRINO_THEME, true, NULL, this, "theme_neutrino"));
	colorSettings_Themes.addItem(new CMenuForwarder(LOCALE_COLORTHEMEMENU_CLASSIC_THEME, true, NULL, this, "theme_classic"));
}

void CNeutrinoApp::InitColorSettingsMenuColors(CMenuWidget &colorSettings_menuColors)
{
	colorSettings_menuColors.addItem(GenericMenuSeparator);
	colorSettings_menuColors.addItem(GenericMenuBack);

	CColorChooser* chHeadcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND_HEAD, &g_settings.menu_Head_red, &g_settings.menu_Head_green, &g_settings.menu_Head_blue,
																  &g_settings.menu_Head_alpha, colorSetupNotifier);
	CColorChooser* chHeadTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR_HEAD, &g_settings.menu_Head_Text_red, &g_settings.menu_Head_Text_green, &g_settings.menu_Head_Text_blue,
																		NULL, colorSetupNotifier);
	CColorChooser* chContentcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND_HEAD, &g_settings.menu_Content_red, &g_settings.menu_Content_green, &g_settings.menu_Content_blue,
																	  &g_settings.menu_Content_alpha, colorSetupNotifier);
	CColorChooser* chContentTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR_HEAD, &g_settings.menu_Content_Text_red, &g_settings.menu_Content_Text_green, &g_settings.menu_Content_Text_blue,
																			NULL, colorSetupNotifier);
	CColorChooser* chContentSelectedcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND_HEAD, &g_settings.menu_Content_Selected_red, &g_settings.menu_Content_Selected_green, &g_settings.menu_Content_Selected_blue,
																				 &g_settings.menu_Content_Selected_alpha, colorSetupNotifier);
	CColorChooser* chContentSelectedTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR_HEAD, &g_settings.menu_Content_Selected_Text_red, &g_settings.menu_Content_Selected_Text_green, &g_settings.menu_Content_Selected_Text_blue,
																					  NULL, colorSetupNotifier);
	CColorChooser* chContentInactivecolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND_HEAD, &g_settings.menu_Content_inactive_red, &g_settings.menu_Content_inactive_green, &g_settings.menu_Content_inactive_blue,
																				 &g_settings.menu_Content_inactive_alpha, colorSetupNotifier);
	CColorChooser* chContentInactiveTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR_HEAD, &g_settings.menu_Content_inactive_Text_red, &g_settings.menu_Content_inactive_Text_green, &g_settings.menu_Content_inactive_Text_blue,
																					  NULL, colorSetupNotifier);
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUHEAD));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chHeadcolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chHeadTextcolor ));
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chContentcolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chContentTextcolor ));
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT_INACTIVE));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chContentInactivecolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chContentInactiveTextcolor));
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT_SELECTED));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chContentSelectedcolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chContentSelectedTextcolor ));
}

void CNeutrinoApp::InitColorSettingsStatusBarColors(CMenuWidget &colorSettings_statusbarColors)
{
	colorSettings_statusbarColors.addItem(GenericMenuSeparator);

	colorSettings_statusbarColors.addItem(GenericMenuBack);

	CColorChooser* chInfobarcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND_HEAD, &g_settings.infobar_red, &g_settings.infobar_green, &g_settings.infobar_blue,
																	  &g_settings.infobar_alpha, colorSetupNotifier);
	CColorChooser* chInfobarTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR_HEAD, &g_settings.infobar_Text_red, &g_settings.infobar_Text_green, &g_settings.infobar_Text_blue,
																			NULL, colorSetupNotifier);

	colorSettings_statusbarColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORSTATUSBAR_TEXT));
	colorSettings_statusbarColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chInfobarcolor ));
	colorSettings_statusbarColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chInfobarTextcolor ));
}

void CNeutrinoApp::InitColorSettingsTiming(CMenuWidget &colorSettings_timing)
{
	/* note: SetupTiming() is already called in CNeutrinoApp::run */

	colorSettings_timing.addItem(GenericMenuSeparator);
	colorSettings_timing.addItem(GenericMenuBack);
	colorSettings_timing.addItem(GenericMenuSeparatorLine);

	for (int i = 0; i < TIMING_SETTING_COUNT; i++)
	{
		CStringInput * colorSettings_timing_item = new CStringInput(timing_setting_name[i], g_settings.timing_string[i], 3, LOCALE_TIMING_HINT_1, LOCALE_TIMING_HINT_2, "0123456789 ", this);
		colorSettings_timing.addItem(new CMenuForwarder(timing_setting_name[i], true, g_settings.timing_string[i], colorSettings_timing_item));
	}

	colorSettings_timing.addItem(GenericMenuSeparatorLine);
	colorSettings_timing.addItem(new CMenuForwarder(LOCALE_OPTIONS_DEFAULT, true, NULL, this, "osd.def"));
}

void CNeutrinoApp::InitLcdSettings(CMenuWidget &lcdSettings)
{
	lcdSettings.addItem(GenericMenuSeparator);

	lcdSettings.addItem(GenericMenuBack);
	lcdSettings.addItem(GenericMenuSeparatorLine);

	CLcdControler* lcdsliders = new CLcdControler(LOCALE_LCDMENU_HEAD, NULL);

	CLcdNotifier* lcdnotifier = new CLcdNotifier();

	CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_LCDMENU_INVERSE, &g_settings.lcd_setting[SNeutrinoSettings::LCD_INVERSE], true, lcdnotifier);
	oj->addOption(0, LOCALE_OPTIONS_OFF);
	oj->addOption(1, LOCALE_OPTIONS_ON );
	lcdSettings.addItem( oj );

	oj = new CMenuOptionChooser(LOCALE_LCDMENU_POWER, &g_settings.lcd_setting[SNeutrinoSettings::LCD_POWER], true, lcdnotifier);
	oj->addOption(0, LOCALE_OPTIONS_OFF);
	oj->addOption(1, LOCALE_OPTIONS_ON );
	lcdSettings.addItem( oj );

	if ((g_info.box_Type == CControld::TUXBOX_MAKER_PHILIPS) || (g_info.box_Type == CControld::TUXBOX_MAKER_SAGEM))
	{
		// Autodimm available on Sagem/Philips only
		oj = new CMenuOptionChooser(LOCALE_LCDMENU_AUTODIMM, &g_settings.lcd_setting[SNeutrinoSettings::LCD_AUTODIMM], true, lcdnotifier);
		oj->addOption(0, LOCALE_OPTIONS_OFF);
		oj->addOption(1, LOCALE_OPTIONS_ON );
		lcdSettings.addItem( oj );
	}

	lcdSettings.addItem(GenericMenuSeparatorLine);
	lcdSettings.addItem(new CMenuForwarder(LOCALE_LCDMENU_LCDCONTROLER, true, NULL, lcdsliders));

	lcdSettings.addItem(GenericMenuSeparatorLine);
	oj = new CMenuOptionChooser(LOCALE_LCDMENU_STATUSLINE, &g_settings.lcd_setting[SNeutrinoSettings::LCD_SHOW_VOLUME], true);
	oj->addOption(0, LOCALE_LCDMENU_STATUSLINE_PLAYTIME);
	oj->addOption(1, LOCALE_LCDMENU_STATUSLINE_VOLUME  );
	oj->addOption(2, LOCALE_LCDMENU_STATUSLINE_BOTH    );
	lcdSettings.addItem( oj );
}

void CNeutrinoApp::InitKeySettings(CMenuWidget &keySettings)
{
	keySettings.addItem(GenericMenuSeparator);
	keySettings.addItem(GenericMenuBack);

	enum keynames {
		KEY_TV_RADIO_MODE,
		KEY_PAGE_UP,
		KEY_PAGE_DOWN,
		KEY_CANCEL_ACTION,
		KEY_SORT,
		KEY_ADD_RECORD,
		KEY_ADD_REMIND,
		KEY_CHANNEL_UP,
		KEY_CHANNEL_DOWN,
		KEY_BOUQUET_UP,
		KEY_BOUQUET_DOWN,
		KEY_SUBCHANNEL_UP,
		KEY_SUBCHANNEL_DOWN
	};

	const char * keydescription_head[13] =
		{
			"keybindingmenu.tvradiomode_head",
			"keybindingmenu.pageup_head",
			"keybindingmenu.pagedown_head",
			"keybindingmenu.cancel_head",
			"keybindingmenu.sort_head",
			"keybindingmenu.addrecord_head",
			"keybindingmenu.addremind_head",
			"keybindingmenu.channelup_head",
			"keybindingmenu.channeldown_head",
			"keybindingmenu.bouquetup_head",
			"keybindingmenu.bouquetdown_head",
			"keybindingmenu.subchannelup_head",
			"keybindingmenu.subchanneldown_head"
		};

	const char * keydescription[13] =
		{
			"keybindingmenu.tvradiomode",
			"keybindingmenu.pageup",
			"keybindingmenu.pagedown",
			"keybindingmenu.cancel",
			"keybindingmenu.sort",
			"keybindingmenu.addrecord",
			"keybindingmenu.addremind",
			"keybindingmenu.channelup",
			"keybindingmenu.channeldown",
			"keybindingmenu.bouquetup",
			"keybindingmenu.bouquetdown",
			"keybindingmenu.subchannelup",
			"keybindingmenu.subchanneldown"
		};

	int * keyvalue_p[13] =
		{
			&g_settings.key_tvradio_mode,
			&g_settings.key_channelList_pageup,
			&g_settings.key_channelList_pagedown,
			&g_settings.key_channelList_cancel,
			&g_settings.key_channelList_sort,
			&g_settings.key_channelList_addrecord,
			&g_settings.key_channelList_addremind,
			&g_settings.key_quickzap_up,
			&g_settings.key_quickzap_down,
			&g_settings.key_bouquet_up,
			&g_settings.key_bouquet_down,
			&g_settings.key_subchannel_up,
			&g_settings.key_subchannel_down
		};

	CKeyChooser * keychooser[13];

	for (int i = 0; i < 13; i++)
		keychooser[i] = new CKeyChooser(keyvalue_p[i], keydescription_head[i], NEUTRINO_ICON_SETTINGS);

	keySettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "keybindingmenu.modechange") );
	keySettings.addItem(new CMenuForwarder(keydescription[KEY_TV_RADIO_MODE], true, NULL, keychooser[KEY_TV_RADIO_MODE]));

	keySettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "keybindingmenu.channellist") );
	CMenuOptionChooser *oj = new CMenuOptionChooser("keybindingmenu.bouquethandling" , &g_settings.bouquetlist_mode, true );
	oj->addOption(0, "keybindingmenu.bouquetchannels_on_ok");
	oj->addOption(1, "keybindingmenu.bouquetlist_on_ok");
	oj->addOption(2, "keybindingmenu.allchannels_on_ok");
	keySettings.addItem( oj );
	for (int i = KEY_PAGE_UP; i <= KEY_ADD_REMIND; i++)
		keySettings.addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));

	keySettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "keybindingmenu.quickzap") );
	for (int i = KEY_CHANNEL_UP; i <= KEY_SUBCHANNEL_DOWN; i++)
		keySettings.addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));
}

void CNeutrinoApp::SelectNVOD()
{
	if (!(g_RemoteControl->subChannels.empty()))
	{
		// NVOD/SubService- Kanal!
		CMenuWidget NVODSelector(g_RemoteControl->are_subchannels ? LOCALE_NVODSELECTOR_SUBSERVICE : LOCALE_NVODSELECTOR_HEAD, "video.raw", 350);

		NVODSelector.addItem(GenericMenuSeparator);

		int count = 0;
		char nvod_id[5];

		for( CSubServiceListSorted::iterator e=g_RemoteControl->subChannels.begin(); e!=g_RemoteControl->subChannels.end(); ++e)
		{
			sprintf(nvod_id, "%d", count);

			if( !g_RemoteControl->are_subchannels )
			{
				char nvod_time_a[50], nvod_time_e[50], nvod_time_x[50];
				char nvod_s[100];
				struct  tm *tmZeit;

				tmZeit= localtime(&e->startzeit );
				sprintf(nvod_time_a, "%02d:%02d", tmZeit->tm_hour, tmZeit->tm_min);

				time_t endtime = e->startzeit+ e->dauer;
				tmZeit= localtime(&endtime);
				sprintf(nvod_time_e, "%02d:%02d", tmZeit->tm_hour, tmZeit->tm_min);

				time_t jetzt=time(NULL);
				if(e->startzeit > jetzt)
				{
					int mins=(e->startzeit- jetzt)/ 60;
					sprintf(nvod_time_x, g_Locale->getText(LOCALE_NVOD_STARTING), mins);
				}
				else
					if( (e->startzeit<= jetzt) && (jetzt < endtime) )
				{
					int proz=(jetzt- e->startzeit)*100/ e->dauer;
					sprintf(nvod_time_x, g_Locale->getText(LOCALE_NVOD_PERCENTAGE), proz);
				}
				else
					nvod_time_x[0]= 0;

				sprintf(nvod_s, "%s - %s %s", nvod_time_a, nvod_time_e, nvod_time_x);
				NVODSelector.addItem(new CMenuForwarder(nvod_s, true, NULL, NVODChanger, nvod_id, false), (count == g_RemoteControl->selected_subchannel));
			}
			else
			{
				NVODSelector.addItem(new CMenuForwarder((Latin1_to_UTF8(e->subservice_name)).c_str(), true, NULL, NVODChanger, nvod_id, false, (uint)(CRCInput::convertDigitToKey(count))), (count == g_RemoteControl->selected_subchannel));
			}

			count++;
		}

		if( g_RemoteControl->are_subchannels )
		{
			NVODSelector.addItem(GenericMenuSeparatorLine);
			CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_NVODSELECTOR_DIRECTORMODE, &g_RemoteControl->director_mode, true, NULL, true, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW);
			oj->addOption(0, LOCALE_OPTIONS_OFF);
			oj->addOption(1, LOCALE_OPTIONS_ON );
			NVODSelector.addItem( oj );
		}

		NVODSelector.exec(NULL, "");
	}
}

void CNeutrinoApp::SelectAPID()
{
	if( g_RemoteControl->current_PIDs.APIDs.size()> 1 )
	{
		// wir haben APIDs f�r diesen Kanal!

		CMenuWidget APIDSelector(LOCALE_APIDSELECTOR_HEAD, "audio.raw", 300);
		APIDSelector.addItem(GenericMenuSeparator);

		for( unsigned int count=0; count<g_RemoteControl->current_PIDs.APIDs.size(); count++ )
		{
			char apid[5];
			sprintf(apid, "%d", count);
			APIDSelector.addItem(new CMenuForwarder(g_RemoteControl->current_PIDs.APIDs[count].desc, true, NULL, APIDChanger, apid, false, (uint)(CRCInput::convertDigitToKey(count + 1))), (count == g_RemoteControl->current_PIDs.PIDs.selected_apid));
		}
		APIDSelector.exec(NULL, "");
	}
}

void CNeutrinoApp::ShowStreamFeatures()
{
	CMenuWidget StreamFeatureSelector("streamfeatures.head", "features.raw", 350);
	StreamFeatureSelector.addItem(GenericMenuSeparator);

	char id[5];
	int cnt = 0;
	int enabled_count = 0;

	for(unsigned int count=0;count < (unsigned int) g_PluginList->getNumberOfPlugins();count++)
	{
		if (g_PluginList->getType(count)== PLUGIN_TYPE_TOOL)
		{
			// zB vtxt-plugins

			sprintf(id, "%d", count);

			enabled_count++;

			StreamFeatureSelector.addItem(new CMenuForwarder(g_PluginList->getName(count), true, NULL, StreamFeaturesChanger, id, false, (cnt== 0) ? CRCInput::RC_blue :(uint)(CRCInput::convertDigitToKey(enabled_count-1 )), (cnt== 0)?NEUTRINO_ICON_BUTTON_BLUE:""), (cnt == 0) );
			cnt++;
		}
	}

	if(cnt>0)
	{
		StreamFeatureSelector.addItem(GenericMenuSeparatorLine);
	}

	sprintf(id, "%d", -1);

	// -- Add Channel to favorites
	StreamFeatureSelector.addItem(new CMenuForwarder(LOCALE_FAVORITES_MENUEADD, true, NULL, new CFavorites, id, true, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN), false);

	// start/stop recording
	if (g_settings.recording_type != RECORDING_OFF)
	{
		CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_MAINMENU_RECORDING, &recordingstatus, true, this, true, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED);
		oj->addOption(0, LOCALE_MAINMENU_RECORDING_START);
		oj->addOption(1, LOCALE_MAINMENU_RECORDING_STOP);
		StreamFeatureSelector.addItem( oj );
	}
	// -- Timer Liste
	StreamFeatureSelector.addItem(new CMenuForwarder("timerlist.name", true, NULL, new CTimerList(), id, true, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW), false);

	StreamFeatureSelector.addItem(GenericMenuSeparatorLine);

	// --  Lock remote control
	StreamFeatureSelector.addItem(new CMenuForwarder("rclock.menueadd", true, NULL, new CRCLock, id, true, CRCInput::RC_nokey, ""), false);

	// -- Sectionsd pause
	int dummy = g_Sectionsd->getIsScanningActive();
	CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_MAINMENU_PAUSESECTIONSD, &dummy, true, new CPauseSectionsdNotifier );
	oj->addOption(0, LOCALE_OPTIONS_OFF);
	oj->addOption(1, LOCALE_OPTIONS_ON );
	StreamFeatureSelector.addItem( oj );

	// -- Stream Info
	// -- !! obsolete (rasc 2004-03-06) 
	// StreamFeatureSelector.addItem(new CMenuForwarder("streamfeatures.info", true, NULL, StreamFeaturesChanger, id, true, CRCInput::RC_help, NEUTRINO_ICON_BUTTON_HELP_SMALL), false);


	// ------
#ifdef __EXPERIMENTAL_CODE__
	// -- Experimental Code
	// -- rasc (2003-12)

	StreamFeatureSelector.addItem(new CMenuForwarder("experimental", true, NULL, new CChMosaicHandler(), id, true, CRCInput::RC_nokey, ""), false);


#endif
	// ------

	StreamFeatureSelector.exec(NULL, "");

	// restore mute symbol
	AudioMute(current_muted, true);
}


void CNeutrinoApp::InitZapper()
{

	g_InfoViewer->start();
	g_EpgData->start();

	firstChannel();
	if(firstchannel.mode == 't')
	{
		tvMode();
	}
	else
	{
		radioMode();
	}
}

void CNeutrinoApp::setupRecordingDevice(void)
{
	if (g_settings.recording_type == RECORDING_SERVER)
	{
		CVCRControl::CServerDeviceInfo * info = new CVCRControl::CServerDeviceInfo;
		unsigned int port;
		sscanf(g_settings.recording_server_port, "%u", &port);
		info->ServerPort = port;
		info->ServerAddress = g_settings.recording_server_ip;
		info->StopPlayBack = (g_settings.recording_stopplayback == 1);
		info->StopSectionsd = (g_settings.recording_stopsectionsd == 1);
		info->Name = "ngrab";
		CVCRControl::getInstance()->registerDevice(CVCRControl::DEVICE_SERVER,info);
		delete info;		
	}
	else if (g_settings.recording_type == RECORDING_FILE)
	{
		CVCRControl::CFileDeviceInfo * info = new CVCRControl::CFileDeviceInfo;
		unsigned int splitsize;
		sscanf(g_settings.recording_splitsize, "%u", &splitsize);
		info->SplitSize = splitsize;
		info->Directory = g_settings.network_nfs_recordingdir;
		info->StopPlayBack = (g_settings.recording_stopplayback == 1);
		info->StopSectionsd = (g_settings.recording_stopsectionsd == 1);
		info->Name = "file";
		CVCRControl::getInstance()->registerDevice(CVCRControl::DEVICE_FILE, info);
		delete info;
	}
	else if(g_settings.recording_type == RECORDING_VCR)
	{
		CVCRControl::CVCRDeviceInfo * info = new CVCRControl::CVCRDeviceInfo;
		info->Name = "vcr";
		info->SwitchToScart = (g_settings.recording_vcr_no_scart==0);
		CVCRControl::getInstance()->registerDevice(CVCRControl::DEVICE_VCR,info);
		delete info;
	}
	else
	{
		if(CVCRControl::getInstance()->isDeviceRegistered())
			CVCRControl::getInstance()->unregisterDevice();
	}
}

int CNeutrinoApp::run(int argc, char **argv)
{
	CmdParser(argc, argv);
	fserverpid = -1;

	int loadSettingsErg = loadSetup();

	/* load locales before setting up any fonts to determine whether we need a true unicode font */
	bool display_language_selection;
	CLocaleManager::loadLocale_ret_t loadLocale_ret = g_Locale->loadLocale(g_settings.language);
	if (loadLocale_ret == CLocaleManager::NO_SUCH_LOCALE)
	{
		strcpy(g_settings.language, "deutsch");
		loadLocale_ret = g_Locale->loadLocale(g_settings.language);
		display_language_selection = true;
	}
	else
		display_language_selection = false;

	if (font.name == NULL) /* no font specified in command line */
	{
		unsigned int use_true_unicode_font = (loadLocale_ret == CLocaleManager::ISO_8859_1_FONT) ? 0 : 1;
		
		font = predefined_font[use_true_unicode_font];
		CLCD::getInstance()->init(predefined_lcd_font[use_true_unicode_font][0], 
		                          predefined_lcd_font[use_true_unicode_font][1],
		                          predefined_lcd_font[use_true_unicode_font][2],
		                          predefined_lcd_font[use_true_unicode_font][3],
		                          predefined_lcd_font[use_true_unicode_font][4],
		                          predefined_lcd_font[use_true_unicode_font][5]);
	}
	else
		CLCD::getInstance()->init(font.filename[0], font.name);

	CLCD::getInstance()->showVolume(g_Controld->getVolume((CControld::volume_type)g_settings.audio_avs_Control));
	CLCD::getInstance()->setMuted(g_Controld->getMute((CControld::volume_type)g_settings.audio_avs_Control));

	g_info.box_Type = g_Controld->getBoxType();

	dprintf( DEBUG_DEBUG, "[neutrino] box_Type: %d\n", g_info.box_Type);

	SetupTiming();

	SetupFonts();

	frameBuffer->ClearFrameBuffer();

	g_RCInput = new CRCInput;

	g_Sectionsd = new CSectionsdClient;
	g_Timerd = new CTimerdClient;

	g_RemoteControl = new CRemoteControl;
	g_EpgData = new CEpgData;
	g_InfoViewer = new CInfoViewer;
	g_EventList = new EventList;

	g_PluginList = new CPlugins;
	g_PluginList->setPluginDir(PLUGINDIR);

#ifdef USEACTIONLOG
	g_ActionLog = new CActionLog;
	g_ActionLog->println("neutrino startup");
#endif

	colorSetupNotifier        = new CColorSetupNotifier;
	audioSetupNotifier        = new CAudioSetupNotifier;
	APIDChanger               = new CAPIDChangeExec;
	UCodeChecker              = new CUCodeCheckExec;
	NVODChanger               = new CNVODChangeExec;
	StreamFeaturesChanger     = new CStreamFeaturesChangeExec;
	MyIPChanger               = new CIPChangeNotifier;
	ConsoleDestinationChanger = new CConsoleDestChangeNotifier;

	colorSetupNotifier->changeNotify("initial", NULL);

	CNFSMountGui::automount();

	// setup recording device
	if (g_settings.recording_type != RECORDING_OFF)
		setupRecordingDevice();

	dprintf( DEBUG_NORMAL, "menue setup\n");
	//Main settings
	CMenuWidget mainMenu            (LOCALE_MAINMENU_HEAD       , "mainmenue.raw"       );
	CMenuWidget mainSettings        (LOCALE_MAINSETTINGS_HEAD   , NEUTRINO_ICON_SETTINGS);
	CMenuWidget languageSettings    (LOCALE_LANGUAGESETUP_HEAD  , "language.raw"        );
	CMenuWidget videoSettings       (LOCALE_VIDEOMENU_HEAD      , "video.raw"           );
	CMenuWidget audioSettings       (LOCALE_AUDIOMENU_HEAD      , "audio.raw"           );
	CMenuWidget parentallockSettings("parentallock.parentallock", "lock.raw"            , 500);
	CMenuWidget networkSettings     (LOCALE_NETWORKMENU_HEAD    , "network.raw"         );
	CMenuWidget recordingSettings   ("recordingmenu.head"       , "recording.raw"       );
	CMenuWidget streamingSettings   ("streamingmenu.head"       , "streaming.raw"       );
	CMenuWidget colorSettings       (LOCALE_COLORMENU_HEAD      , "colors.raw"          );
	CMenuWidget fontSettings        ("fontmenu.head"            , "colors.raw"          );
	CMenuWidget lcdSettings         (LOCALE_LCDMENU_HEAD        , "lcd.raw"             );
	CMenuWidget keySettings         ("keybindingmenu.head"      , "keybinding.raw"      , 400);
	CMenuWidget miscSettings        (LOCALE_MISCSETTINGS_HEAD   , NEUTRINO_ICON_SETTINGS);
	CMenuWidget mp3picSettings      ("mp3picsettings.general"   , NEUTRINO_ICON_SETTINGS);
	CMenuWidget scanSettings        ("servicemenu.scants"       , NEUTRINO_ICON_SETTINGS);
	CMenuWidget service             ("servicemenu.head"         , NEUTRINO_ICON_SETTINGS);
	CMenuWidget moviePlayer         (LOCALE_MOVIEPLAYER_HEAD    , "streaming.raw"       );
    
	InitMainMenu(mainMenu, mainSettings, audioSettings, parentallockSettings, networkSettings, recordingSettings,
					 colorSettings, lcdSettings, keySettings, videoSettings, languageSettings, miscSettings,
					 service, fontSettings, mp3picSettings, streamingSettings, moviePlayer);

	//service
	InitServiceSettings(service, scanSettings);

	//language Setup
	InitLanguageSettings(languageSettings);

	//mp3/picviewer Setup
	InitMp3PicSettings(mp3picSettings);

   	//misc Setup
	InitMiscSettings(miscSettings);
	miscSettings.setOnPaintNotifier(this);

	//audio Setup
	InitAudioSettings(audioSettings, audioSetupNotifier);

	//video Setup
	InitVideoSettings(videoSettings);
	videoSettings.setOnPaintNotifier(this);

	// Parentallock settings
	InitParentalLockSettings( parentallockSettings);

	// ScanSettings
	InitScanSettings(scanSettings);

	dprintf( DEBUG_NORMAL, "registering as event client\n");

	g_Controld->registerEvent(CControldClient::EVT_MUTECHANGED, 222, NEUTRINO_UDS_NAME);
	g_Controld->registerEvent(CControldClient::EVT_VOLUMECHANGED, 222, NEUTRINO_UDS_NAME);
	g_Controld->registerEvent(CControldClient::EVT_MODECHANGED, 222, NEUTRINO_UDS_NAME);
	g_Controld->registerEvent(CControldClient::EVT_VCRCHANGED, 222, NEUTRINO_UDS_NAME);

	g_Sectionsd->registerEvent(CSectionsdClient::EVT_TIMESET, 222, NEUTRINO_UDS_NAME);
	g_Sectionsd->registerEvent(CSectionsdClient::EVT_GOT_CN_EPG, 222, NEUTRINO_UDS_NAME);

#ifndef SKIP_CA_STATUS
#define ZAPIT_EVENT_COUNT 26
#else
#define ZAPIT_EVENT_COUNT 23
#endif
	const CZapitClient::events zapit_event[ZAPIT_EVENT_COUNT] =
		{
			CZapitClient::EVT_ZAP_COMPLETE,
			CZapitClient::EVT_ZAP_COMPLETE_IS_NVOD,
			CZapitClient::EVT_ZAP_FAILED,
			CZapitClient::EVT_ZAP_SUB_COMPLETE,
			CZapitClient::EVT_ZAP_SUB_FAILED,
			CZapitClient::EVT_ZAP_MOTOR,
#ifndef SKIP_CA_STATUS
			CZapitClient::EVT_ZAP_CA_CLEAR,
			CZapitClient::EVT_ZAP_CA_LOCK,
			CZapitClient::EVT_ZAP_CA_FTA,
#endif
			CZapitClient::EVT_RECORDMODE_ACTIVATED,
			CZapitClient::EVT_RECORDMODE_DEACTIVATED,
			CZapitClient::EVT_SCAN_COMPLETE,
			CZapitClient::EVT_SCAN_FAILED,
			CZapitClient::EVT_SCAN_NUM_TRANSPONDERS,
			CZapitClient::EVT_SCAN_REPORT_NUM_SCANNED_TRANSPONDERS,
			CZapitClient::EVT_SCAN_REPORT_FREQUENCY,
			CZapitClient::EVT_SCAN_REPORT_FREQUENCYP,
			CZapitClient::EVT_SCAN_SATELLITE,
			CZapitClient::EVT_SCAN_NUM_CHANNELS,
			CZapitClient::EVT_SCAN_PROVIDER,
			CZapitClient::EVT_BOUQUETS_CHANGED,
			CZapitClient::EVT_SCAN_SERVICENAME,
			CZapitClient::EVT_SCAN_FOUND_A_CHAN,
			CZapitClient::EVT_SCAN_FOUND_TV_CHAN,
			CZapitClient::EVT_SCAN_FOUND_RADIO_CHAN,
			CZapitClient::EVT_SCAN_FOUND_DATA_CHAN,
		};

	for (int i = 0; i < ZAPIT_EVENT_COUNT; i++)
		g_Zapit->registerEvent(zapit_event[i], 222, NEUTRINO_UDS_NAME);

	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_SHUTDOWN, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_SHUTDOWN, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_NEXTPROGRAM, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_NEXTPROGRAM, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_STANDBY_ON, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_STANDBY_OFF, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_RECORD, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_RECORD_START, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_RECORD_STOP, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_ZAPTO, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ZAPTO, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_SLEEPTIMER, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_SLEEPTIMER, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_REMIND, 222, NEUTRINO_UDS_NAME);

	if (display_language_selection)
		languageSettings.exec(NULL, "");

	//ucodes testen
	doChecks();
	//settins
	if(loadSettingsErg==1)
	{
		dprintf(DEBUG_INFO, "config file missing\n");
		ShowHintUTF(LOCALE_MESSAGEBOX_INFO, g_Locale->getText("settings.noconffile")); // UTF-8
		configfile.setModifiedFlag(true);
		saveSetup();
	}
	else if(loadSettingsErg==2)
	{
		dprintf(DEBUG_INFO, "parts of configfile missing\n");
		ShowHintUTF(LOCALE_MESSAGEBOX_INFO, g_Locale->getText("settings.missingoptionsconffile")); // UTF-8
		configfile.setModifiedFlag(true);
		saveSetup();
	}

	//init programm
	InitZapper();

	//network Setup
	InitNetworkSettings(networkSettings);

	//Recording Setup
	InitRecordingSettings(recordingSettings);

	//VLC Setup
	InitStreamingSettings(streamingSettings);

	//font Setup
	InitFontSettings(fontSettings);

	//color Setup
	InitColorSettings(colorSettings, fontSettings);

	//LCD Setup
	InitLcdSettings(lcdSettings);

	//keySettings
	InitKeySettings(keySettings);

	AudioMute( g_Controld->getMute((CControld::volume_type)g_settings.audio_avs_Control), true );

	//load Pluginlist
	g_PluginList->loadPlugins();

	RealRun(mainMenu);

	ExitRun();

	return 0;
}


void CNeutrinoApp::RealRun(CMenuWidget &mainMenu)
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	dprintf(DEBUG_NORMAL, "initialized everything\n");

	while( true )
	{
		g_RCInput->getMsg(&msg, &data, 100);	// 10 secs..

		if( ( mode == mode_tv ) || ( ( mode == mode_radio ) ) )
		{
			if( msg == NeutrinoMessages::SHOW_EPG )
			{
				// show EPG

				g_EpgData->show( channelList->getActiveChannel_ChannelID() );

			}
			else if( msg == (neutrino_msg_t) g_settings.key_tvradio_mode )
			{
				if( mode == mode_tv )
				{
					radioMode();
				}
				else if( mode == mode_radio )
				{
					tvMode();
				}
			}
			else if( msg == CRCInput::RC_setup )
			{
				mainMenu.exec(NULL, "");
				// restore mute symbol
				AudioMute(current_muted, true);
			}
			if( msg == CRCInput::RC_ok)
			{
				int bouqMode = g_settings.bouquetlist_mode;//bsmChannels;

				if((bouquetList!=NULL) && (bouquetList->Bouquets.empty()))
				{
					dprintf(DEBUG_DEBUG, "bouquets are empty\n");
					bouqMode = bsmAllChannels;
				}
				if((bouquetList!=NULL) && (bouqMode == 1/*bsmBouquets*/))
				{
					bouquetList->exec(true);
				}
				else if((bouquetList!=NULL) && (bouqMode == 0/*bsmChannels*/))
				{
					int nNewChannel = bouquetList->Bouquets[bouquetList->getActiveBouquetNumber()]->channelList->show();
					if(nNewChannel>-1)
					{
						channelList->zapTo(bouquetList->Bouquets[bouquetList->getActiveBouquetNumber()]->channelList->getKey(nNewChannel)-1);
					}
				}
				else
				{
					dprintf(DEBUG_DEBUG, "using all channels\n");
					channelList->exec();
				}
			}
			else if( msg == CRCInput::RC_red )
			{	// eventlist
				// -- !! obsolete (2004-03-06 rasc)
				// g_EventList->exec(channelList->getActiveChannel_ChannelID(), channelList->getActiveChannelName()); // UTF-8

				// -- new EPG Menu (rasc 2004-03-06)
				{  CEPGMenuHandler  *epg_menu;

				   epg_menu = new CEPGMenuHandler;
				   epg_menu-> doMenu();
				}
			}
			else if( msg == CRCInput::RC_blue )
			{	// streaminfo
				ShowStreamFeatures();
			}
			else if( msg == CRCInput::RC_green )
			{	// APID
				SelectAPID();
			}
			else if( msg == CRCInput::RC_yellow )
			{	// NVODs
				SelectNVOD();
			}
			else if( ( msg == (neutrino_msg_t) g_settings.key_quickzap_up ) || ( msg == (neutrino_msg_t) g_settings.key_quickzap_down ) )
			{
				//quickzap
				channelList->quickZap( msg );
			}
			else if( ( msg == CRCInput::RC_help ) ||
						( msg == NeutrinoMessages::SHOW_INFOBAR ) )
			{
				// show Infoviewer
				g_InfoViewer->showTitle(channelList->getActiveChannelNumber(), channelList->getActiveChannelName(), channelList->getActiveSatellitePosition(), channelList->getActiveChannel_ChannelID()); // UTF-8
			}
			else if (CRCInput::isNumeric(msg))
			{ //numeric zap
				if( g_RemoteControl->director_mode )
				{
					g_RemoteControl->setSubChannel(CRCInput::getNumericValue(msg));
					g_InfoViewer->showSubchan();
				}
				else
					channelList->numericZap( msg );
			}
			else if( msg == (neutrino_msg_t) g_settings.key_subchannel_up )
			{
				g_RemoteControl->subChannelUp();
				g_InfoViewer->showSubchan();
			}
			else if( msg == (neutrino_msg_t) g_settings.key_subchannel_down )
			{
				g_RemoteControl->subChannelDown();
				g_InfoViewer->showSubchan();
			}
			else
			{
				handleMsg(msg, data);
			}

		}
		else
		{
			// mode == mode_scart
			if( msg == CRCInput::RC_home )
			{
				if( mode == mode_scart )
				{
					//wenn VCR Aufnahme dann stoppen
					if(CVCRControl::getInstance()->isDeviceRegistered())
					{
						if ((CVCRControl::getInstance()->Device->getDeviceType() == CVCRControl::DEVICE_VCR) &&
						    (CVCRControl::getInstance()->getDeviceState() == CVCRControl::CMD_VCR_RECORD || 
						     CVCRControl::getInstance()->getDeviceState() == CVCRControl::CMD_VCR_PAUSE))
						{
							CVCRControl::getInstance()->Stop();
							recordingstatus=0;
							startNextRecording();
						}
					}
					// Scart-Mode verlassen
					scartMode( false );
				}
			}
			else
			{
				handleMsg(msg, data);
			}
		}
	}
}

int CNeutrinoApp::handleMsg(const neutrino_msg_t msg, neutrino_msg_data_t data)
{
	int res = 0;

	res = res | g_RemoteControl->handleMsg(msg, data);
	res = res | g_InfoViewer->handleMsg(msg, data);
	res = res | channelList->handleMsg(msg, data);

	if( res != messages_return::unhandled )
	{
		if( ( msg>= CRCInput::RC_WithData ) && ( msg< CRCInput::RC_WithData+ 0x10000000 ) )
			delete (unsigned char*) data;
		return( res & ( 0xFFFFFFFF - messages_return::unhandled ) );
	}

	if( msg == NeutrinoMessages::EVT_VCRCHANGED )
	{
		if( g_settings.vcr_AutoSwitch == 1 )
		{
			if( data != VCR_STATUS_OFF )
				g_RCInput->postMsg( NeutrinoMessages::VCR_ON, 0 );
			else
				g_RCInput->postMsg( NeutrinoMessages::VCR_OFF, 0 );
		}
		return messages_return::handled | messages_return::cancel_info;
	}
	else if (msg == CRCInput::RC_standby)
	{
		if (data == 0)
		{
			neutrino_msg_t new_msg;

			/* Note: pressing the power button on the dbox (not the remote control) over 1 second */
			/*       shuts down the system even if !g_settings.shutdown_real_rcdelay (see below)  */
			gettimeofday(&standby_pressed_at, NULL);

			if ((mode != mode_standby) && (g_settings.shutdown_real))
			{
				new_msg = NeutrinoMessages::SHUTDOWN;
			}
			else
			{
				new_msg = (mode == mode_standby) ? NeutrinoMessages::STANDBY_OFF : NeutrinoMessages::STANDBY_ON;

				if ((g_settings.shutdown_real_rcdelay))
				{
					neutrino_msg_t      msg;
					neutrino_msg_data_t data;
					struct timeval      endtime;
					time_t              seconds;

					int timeout = 0;
					int timeout1 = 0;

					sscanf(g_settings.repeat_blocker, "%d", &timeout);
					sscanf(g_settings.repeat_genericblocker, "%d", &timeout1);

					if (timeout1 > timeout)
						timeout = timeout1;

					timeout += 500;

					while(true)
					{
						g_RCInput->getMsg_ms(&msg, &data, timeout);

						if (msg == CRCInput::RC_timeout)
							break;

						gettimeofday(&endtime, NULL);
						seconds = endtime.tv_sec - standby_pressed_at.tv_sec;
						if (endtime.tv_usec < standby_pressed_at.tv_usec)
							seconds--;
						if (seconds >= 1)
						{
							new_msg = NeutrinoMessages::SHUTDOWN;
							break;
						}
					}
				}
			}
			g_RCInput->postMsg(new_msg, 0);
			return messages_return::cancel_all | messages_return::handled;
		}
		else                                        /* data == 1: KEY_POWER released                         */
			if (standby_pressed_at.tv_sec != 0) /* check if we received a KEY_POWER pressed event before */
			{                                   /* required for correct handling of KEY_POWER events of  */
				                            /* the power button on the dbox (not the remote control) */
				struct timeval endtime;
				gettimeofday(&endtime, NULL);
				time_t seconds = endtime.tv_sec - standby_pressed_at.tv_sec;
				if (endtime.tv_usec < standby_pressed_at.tv_usec)
					seconds--;
				if (seconds >= 1)
				{
					g_RCInput->postMsg(NeutrinoMessages::SHUTDOWN, 0);
					return messages_return::cancel_all | messages_return::handled;
				}
			}
	}
	else if ((msg == CRCInput::RC_plus) ||
		 (msg == CRCInput::RC_minus) ||
		 (msg == NeutrinoMessages::EVT_VOLCHANGED))
	{
		setVolume(msg, (mode != mode_scart));
		return messages_return::handled;
	}
	else if( msg == CRCInput::RC_spkr )
	{
		if( mode == mode_standby )
		{
			//switch lcd off/on
			CLCD::getInstance()->togglePower();
		}
		else
		{
			//mute
			AudioMute( !current_muted );
		}
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::EVT_MUTECHANGED )
	{
		CControldMsg::commandMute* cmd = (CControldMsg::commandMute*) data;
		if(cmd->type == (CControld::volume_type)g_settings.audio_avs_Control)
			AudioMute( cmd->mute, true );
		delete (unsigned char*) data;
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::EVT_RECORDMODE )
	{
		dprintf(DEBUG_DEBUG, "neutino - recordmode %s\n", ( data ) ? "on":"off" );

		if( ( !g_InfoViewer->is_visible ) && data )
			g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );

		t_channel_id old_id = channelList->getActiveChannel_ChannelID();

		channelsInit();

		if((old_id == 0) || (!(channelList->adjustToChannelID(old_id))))
			channelList->zapTo(0);
	}
	else if( msg == NeutrinoMessages::EVT_BOUQUETSCHANGED )			// EVT_BOUQUETSCHANGED: initiated by zapit ; EVT_SERVICESCHANGED: no longer used
	{
		t_channel_id old_id = channelList->getActiveChannel_ChannelID();

		channelsInit();

		if((old_id == 0) || (!(channelList->adjustToChannelID(old_id))))
			channelList->zapTo(0);

		return messages_return::handled;
	}
	else if(msg == NeutrinoMessages::RECORD_START)
	{
		if(recordingstatus == 0)
		{
			if(CVCRControl::getInstance()->isDeviceRegistered())
			{
				recording_id = ((CTimerd::RecordingInfo *) data)->eventID;
				if(CVCRControl::getInstance()->Record((CTimerd::RecordingInfo *) data))
					recordingstatus = 1;
				else
					recordingstatus = 0;
			}
			else
				printf("Keine vcr Devices registriert\n");
			delete (unsigned char*) data;
		}
		else
		{
			// Es l�uft bereits eine Aufnahme
			if(nextRecordingInfo!=NULL)
			{
				delete nextRecordingInfo;
				nextRecordingInfo=NULL;
			}
			nextRecordingInfo=((CTimerd::RecordingInfo*)data);
		}
		return messages_return::handled | messages_return::cancel_all;
	}
	else if( msg == NeutrinoMessages::RECORD_STOP)
	{
		if(((CTimerd::RecordingStopInfo*)data)->eventID==recording_id)
		{ // passendes stop zur Aufnahme
			if(CVCRControl::getInstance()->isDeviceRegistered())
			{
				if(CVCRControl::getInstance()->getDeviceState() == CVCRControl::CMD_VCR_RECORD || CVCRControl::getInstance()->getDeviceState() == CVCRControl::CMD_VCR_PAUSE)
				{
					CVCRControl::getInstance()->Stop();
					recordingstatus=0;
				}
				else
					printf("falscher state\n");
			}
			else
				printf("Keine vcr Devices registriert\n");
			startNextRecording();
		}
		else if(nextRecordingInfo!=NULL)
		{
			if(((CTimerd::RecordingStopInfo*)data)->eventID == nextRecordingInfo->eventID)
			{
				delete nextRecordingInfo;
				nextRecordingInfo=NULL;
			}
		}
		delete (unsigned char*) data;
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ZAPTO)
	{
		CTimerd::EventInfo * eventinfo;
		eventinfo = (CTimerd::EventInfo *) data;
		if(recordingstatus==0)
		{
			bool isTVMode = g_Zapit->isChannelTVChannel(eventinfo->channel_id);

			if ((!isTVMode) && (mode != mode_radio))
			{
				radioMode(false);
				channelsInit();
			}
			else if (isTVMode && (mode != mode_tv))
			{
				tvMode(false);
				channelsInit();
			}
			channelList->zapTo_ChannelID(eventinfo->channel_id);
		}
		delete (unsigned char*) data;
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ANNOUNCE_ZAPTO)
	{
		if( mode == mode_standby )
		{
			// WAKEUP
			standbyMode( false );
		}
		if( mode != mode_scart )
			ShowHintUTF(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_ZAPTOTIMER_ANNOUNCE)); // UTF-8
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ANNOUNCE_RECORD)
	{
		if( g_settings.recording_server_wakeup )
		{
			std::string command = "etherwake ";
			command += g_settings.recording_server_mac;
			if(system(command.c_str()) != 0)
				perror("etherwake failed");
		}
		if( mode != mode_scart )
			ShowHintUTF(LOCALE_MESSAGEBOX_INFO, g_Locale->getText("recordtimer.announce")); // UTF-8
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ANNOUNCE_SLEEPTIMER)
	{
		if( mode != mode_scart )
			ShowHintUTF(LOCALE_MESSAGEBOX_INFO, g_Locale->getText("sleeptimerbox.announce")); // UTF-8
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::SLEEPTIMER)
	{
		CIRSend irs("sleep");
		irs.Send();

		if(g_settings.shutdown_real)
			ExitRun();
		else
			standbyMode( true );
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::STANDBY_TOGGLE )
	{
		standbyMode( !(mode & mode_standby) );
		g_RCInput->clearRCMsg();
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::STANDBY_ON )
	{
		if( mode != mode_standby )
		{
			// noch nicht im Standby-Mode...
			standbyMode( true );
		}
		g_RCInput->clearRCMsg();
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::STANDBY_OFF )
	{
		if( mode == mode_standby )
		{
			// WAKEUP
			standbyMode( false );
		}
		g_RCInput->clearRCMsg();
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ANNOUNCE_SHUTDOWN)
	{
		if( mode != mode_scart )
			skipShutdownTimer = (ShowMsgUTF(LOCALE_MESSAGEBOX_INFO, g_Locale->getText("shutdowntimer.announce"), CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo, NULL, 450, 5) == CMessageBox::mbrYes); // UTF-8
	}
	else if( msg == NeutrinoMessages::SHUTDOWN )
	{
		// AUSSCHALTEN...
		if(!skipShutdownTimer)
		{
			ExitRun();
		}
		else
		{
			skipShutdownTimer=false;
		}
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_POPUP)
	{
		if (mode != mode_scart)
			ShowHintUTF(LOCALE_MESSAGEBOX_INFO, (const char *) data); // UTF-8
		delete (unsigned char*) data;
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_EXTMSG)
	{
		if (mode != mode_scart)
			ShowMsgUTF(LOCALE_MESSAGEBOX_INFO, (const char *) data, CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw"); // UTF-8
		delete (unsigned char*) data;
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_RECORDING_ENDED)
	{
		if (mode != mode_scart)
		{
			const char * msgbody;
			
			if ((* (stream2file_status_t *) data) == STREAM2FILE_STATUS_IDLE)
				msgbody = "streaming.success";
			else if ((* (stream2file_status_t *) data) == STREAM2FILE_STATUS_BUFFER_OVERFLOW)
				msgbody = "streaming.buffer_overflow";
			else if ((* (stream2file_status_t *) data) == STREAM2FILE_STATUS_WRITE_OPEN_FAILURE)
				msgbody = "streaming.write_error_open";
			else if ((* (stream2file_status_t *) data) == STREAM2FILE_STATUS_WRITE_FAILURE)
				msgbody = "streaming.write_error";
			else
				goto skip_message;

			ShowMsgUTF(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(msgbody), CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw"); // UTF-8

		skip_message:
			;
		}
		if ((* (stream2file_status_t *) data) != STREAM2FILE_STATUS_IDLE)
		{
			/* 
			 * note that changeNotify does not distinguish between LOCALE_MAINMENU_RECORDING_START and LOCALE_MAINMENU_RECORDING_STOP
			 * instead it checks the state of the variable recordingstatus
			 */ 
			changeNotify(LOCALE_MAINMENU_RECORDING_START, NULL);
		}

		delete (unsigned char*) data;
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::REMIND)
	{
		std::string text = (char*)data;
		std::string::size_type pos;
		while((pos=text.find('/'))!= std::string::npos)
		{
			text[pos] = '\n';
		}
		if( mode != mode_scart )
			ShowMsgUTF("timerlist.type.remind", text, CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw"); // UTF-8
		delete (unsigned char*) data;
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::CHANGEMODE )
	{
		if((data & mode_mask)== mode_radio)
		{
			if( mode != mode_radio )
				if((data & norezap)==norezap)
					radioMode(false);
				else
					radioMode(true);
		}
		if((data & mode_mask)== mode_tv)
		{
			if( mode != mode_tv )
				if((data & norezap)==norezap)
					tvMode(false);
				else
					tvMode(true);
		}
		if((data &mode_mask)== mode_standby)
		{
			if(mode != mode_standby)
				standbyMode( true );
		}
		if((data &mode_mask)== mode_mp3)
		{
			lastMode=mode;
			mode=mode_mp3;
		}
		if((data &mode_mask)== mode_pic)
		{
			lastMode=mode;
			mode=mode_pic;
		}
		if((data &mode_mask)== mode_ts)
		{
			lastMode=mode;
			mode=mode_ts;
		}
	}
	else if( msg == NeutrinoMessages::VCR_ON )
	{
		if( mode != mode_scart )
		{
			// noch nicht im Scart-Mode...
			scartMode( true );
		}
		else // sonst nur lcd akt.
			CLCD::getInstance()->setMode(CLCD::MODE_SCART);
	}

	else if( msg == NeutrinoMessages::VCR_OFF )
	{
		if( mode == mode_scart )
		{
			// noch nicht im Scart-Mode...
			scartMode( false );
		}
	}
	else if (msg == NeutrinoMessages::EVT_START_PLUGIN)
	{
		g_PluginList->setvtxtpid(g_RemoteControl->current_PIDs.PIDs.vtxtpid);
		g_PluginList->startPlugin((const char *)data);
		delete (unsigned char*) data;
		return messages_return::handled;
	}

	if ((msg >= CRCInput::RC_WithData) && (msg < CRCInput::RC_WithData + 0x10000000))
		delete (unsigned char*) data;

	return messages_return::unhandled;
}



void CNeutrinoApp::ExitRun()
{
#ifdef USEACTIONLOG
	g_ActionLog->println("neutrino shutdown");
#endif
	CLCD::getInstance()->setMode(CLCD::MODE_SHUTDOWN);

	dprintf(DEBUG_INFO, "exit\n");
	for(int x=0;x<256;x++)
		frameBuffer->paletteSetColor(x, 0x000000, 0xffff);
	frameBuffer->paletteSet();

	frameBuffer->loadPicture2FrameBuffer("shutdown.raw");
	frameBuffer->loadPal("shutdown.pal");

	networkConfig.automatic_start = (network_automatic_start == 1);
	networkConfig.commitConfig();
	saveSetup();
	g_Controld->shutdown();

	if (g_RCInput != NULL)
		delete g_RCInput;

	if (frameBuffer != NULL)
		delete frameBuffer;

	if (fserverpid > 0) {
	    if(kill(fserverpid,SIGTERM)) {
		    fprintf(stderr,"\n[neutrino.cpp] fserver process not killed\n");
		}
		waitpid(fserverpid,0,0);
		fprintf(stderr,"[neutrino.cpp] fserver stopped\n");
		fserverpid = -1;
	}
		
	exit(0);
}

bool CNeutrinoApp::onPaintNotify(const std::string & MenuName)
{
	if (ARE_LOCALES_EQUAL(MenuName.c_str(), LOCALE_VIDEOMENU_HEAD))
	{//aktuelle werte vom controld holen...
		g_settings.video_Signal = g_Controld->getVideoOutput();
		g_settings.video_Format = g_Controld->getVideoFormat();
	}

	return false;
}

void CNeutrinoApp::AudioMute( bool newValue, bool isEvent )
{
   if((CControld::volume_type)g_settings.audio_avs_Control==CControld::TYPE_LIRC) //lirc
   { // bei LIRC wissen wir nicht wikrlich ob jetzt ge oder entmuted wird, deswegen nix zeigen---
		if( !isEvent )
			g_Controld->Mute((CControld::volume_type)g_settings.audio_avs_Control);
   }
   else
   {
      int dx = 40;
      int dy = 40;
      int x = g_settings.screen_EndX-dx;
      int y = g_settings.screen_StartY;

      CLCD::getInstance()->setMuted(newValue);
      if( newValue != current_muted )
      {
         current_muted = newValue;

         if( !isEvent )
         {
            if( current_muted )
               g_Controld->Mute((CControld::volume_type)g_settings.audio_avs_Control);
            else
               g_Controld->UnMute((CControld::volume_type)g_settings.audio_avs_Control);
         }
      }

      if( isEvent && ( mode != mode_scart ) && ( mode != mode_mp3) && ( mode != mode_pic))
      {
	      // anzeigen NUR, wenn es vom Event kommt
	      if( current_muted )
	      {
		      frameBuffer->paintBoxRel(x, y, dx, dy, COL_INFOBAR_PLUS_0);
		      frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_MUTE, x+5, y+5);
	      }
	      else
		      frameBuffer->paintBackgroundBoxRel(x, y, dx, dy);
      }
   }
}

void CNeutrinoApp::setVolume(const neutrino_msg_t key, const bool bDoPaint)
{
	neutrino_msg_t msg = key;

	int dx = 256;
	int dy = 40;
	int x = (((g_settings.screen_EndX- g_settings.screen_StartX)- dx) / 2) + g_settings.screen_StartX;
	int y = g_settings.screen_EndY- 100;

	fb_pixel_t * pixbuf = NULL;

	if(bDoPaint)
	{
		pixbuf = new fb_pixel_t[dx * dy];
		if(pixbuf!= NULL)
			frameBuffer->SaveScreen(x, y, dx, dy, pixbuf);
		frameBuffer->paintIcon("volume.raw",x,y, COL_INFOBAR);
	}

	neutrino_msg_data_t data;

	unsigned long long timeoutEnd;

	char current_volume = g_Controld->getVolume((CControld::volume_type)g_settings.audio_avs_Control);
	
	do
	{
		if (msg <= CRCInput::RC_MaxRC)
		{
			if (msg == CRCInput::RC_plus)
			{
				if((CControld::volume_type)g_settings.audio_avs_Control==CControld::TYPE_LIRC)
				{
					current_volume = 60; //>50 is plus
				}
				else
				{
					if (current_volume < 100 - 5)
						current_volume += 5;
					else
						current_volume = 100;
				}
			}
			else if (msg == CRCInput::RC_minus)
			{
				if((CControld::volume_type)g_settings.audio_avs_Control==CControld::TYPE_LIRC)
				{
					current_volume = 40; //<40 is minus
				}
				else
				{
					if (current_volume > 5)
						current_volume -= 5;
					else
						current_volume = 0;
				}
			}
			else
			{
				g_RCInput->postMsg(msg, data);
				break;
			}
			
			g_Controld->setVolume(current_volume, (CControld::volume_type)g_settings.audio_avs_Control);
			
			if((CControld::volume_type)g_settings.audio_avs_Control==CControld::TYPE_LIRC)
			{
				current_volume = 50;
			}
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR] / 2);
		}
		else if (msg == NeutrinoMessages::EVT_VOLCHANGED)
		{
			current_volume = g_Controld->getVolume((CControld::volume_type)g_settings.audio_avs_Control);
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR] / 2);
		}
		else if (handleMsg(msg, data) & messages_return::unhandled)
		{
			g_RCInput->postMsg(msg, data);
			break;
		}
		
		if (bDoPaint)
		{
			int vol = current_volume << 1;
			frameBuffer->paintBoxRel(x + 40      , y + 12, vol      , 15, COL_INFOBAR_PLUS_3);
			frameBuffer->paintBoxRel(x + 40 + vol, y + 12, 200 - vol, 15, COL_INFOBAR_PLUS_1);
		}
		
		CLCD::getInstance()->showVolume(current_volume);
		if (msg != CRCInput::RC_timeout)
		{
			g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd );
		}
		
		}
	while (msg != CRCInput::RC_timeout);
	
	if( (bDoPaint) && (pixbuf!= NULL) )
		frameBuffer->RestoreScreen(x, y, dx, dy, pixbuf);
}	

void CNeutrinoApp::tvMode( bool rezap )
{
	CLCD::getInstance()->setMode(CLCD::MODE_TVRADIO);
	if( mode == mode_tv )
	{
		return;
	}
	else if( mode == mode_scart )
	{
		g_Controld->setScartMode( 0 );
	}
	else if( mode == mode_standby )
	{
		CLCD::getInstance()->setMode(CLCD::MODE_TVRADIO);
		g_Controld->videoPowerDown(false);
	}

	mode = mode_tv;
#ifdef USEACTIONLOG
	g_ActionLog->println("mode: tv");
#endif

	if(g_settings.video_Format != CControldClient::VIDEOFORMAT_4_3)
		g_Controld->setVideoFormat(g_settings.video_Format);

	//printf( "tv-mode\n" );

	frameBuffer->useBackground(false);
	frameBuffer->paintBackground();

	g_RemoteControl->tvMode();
	if( rezap )
	{
		firstChannel();
		channelsInit();
		channelList->zapTo( firstchannel.channelNumber -1 );
	}
}

void CNeutrinoApp::scartMode( bool bOnOff )
{
#ifdef USEACTIONLOG
	g_ActionLog->println( ( bOnOff ) ? "mode: scart on" : "mode: scart off" );
#endif


	//printf( ( bOnOff ) ? "mode: scart on\n" : "mode: scart off\n" );

	if( bOnOff )
	{
		// SCART AN
		frameBuffer->useBackground(false);
		frameBuffer->paintBackground();

		g_Controld->setScartMode( 1 );
		CLCD::getInstance()->setMode(CLCD::MODE_SCART);
		lastMode = mode;
		mode = mode_scart;
	}
	else
	{
		// SCART AUS
		g_Controld->setScartMode( 0 );

		mode = mode_unknown;

		//re-set mode
		if( lastMode == mode_radio )
		{
			radioMode( false );
		}
		else if( lastMode == mode_tv )
		{
			tvMode( false );
		}
		else if( lastMode == mode_standby )
		{
			standbyMode( true );
		}
	}
}

void CNeutrinoApp::standbyMode( bool bOnOff )
{
#ifdef USEACTIONLOG
	g_ActionLog->println( ( bOnOff ) ? "mode: standby on" : "mode: standby off" );
#endif


	//printf( ( bOnOff ) ? "mode: standby on\n" : "mode: standby off\n" );

	if( bOnOff )
	{
		// STANDBY AN

		if( mode == mode_scart )
		{
			g_Controld->setScartMode( 0 );
		}

		frameBuffer->useBackground(false);
		frameBuffer->paintBackground();

		CLCD::getInstance()->setMode(CLCD::MODE_STANDBY);
		g_Controld->videoPowerDown(true);

		lastMode = mode;
		mode = mode_standby;

		//Send ir
		CIRSend irs("sbon");
		irs.Send();
	}
	else
	{
		// STANDBY AUS

		CLCD::getInstance()->setMode(CLCD::MODE_TVRADIO);
		g_Controld->videoPowerDown(false);

		//Send ir
		CIRSend irs("sboff");
		irs.Send();

		mode = mode_unknown;

		//re-set mode
		if( lastMode == mode_radio )
		{
			radioMode( false );
		}
		else
		{
			tvMode( false );
		}
	}
}

void CNeutrinoApp::radioMode( bool rezap)
{
	CLCD::getInstance()->setMode(CLCD::MODE_TVRADIO);
	if( mode==mode_radio )
	{
		return;
	}
	else if( mode == mode_scart )
	{
		g_Controld->setScartMode( 0 );
	}
	else if( mode == mode_standby )
	{
		CLCD::getInstance()->setMode(CLCD::MODE_TVRADIO);
		g_Controld->videoPowerDown(false);
	}

	mode = mode_radio;
#ifdef USEACTIONLOG
	g_ActionLog->println("mode: radio");
#endif

	if(g_settings.video_Format != CControldClient::VIDEOFORMAT_4_3)
		g_Controld->setVideoFormat(CControldClient::VIDEOFORMAT_4_3);

	frameBuffer->loadPal("radiomode.pal", 18, COL_MAXFREE);
	frameBuffer->loadBackground("radiomode.raw");
	frameBuffer->useBackground(true);
	frameBuffer->paintBackground();

	g_RemoteControl->radioMode();
	if( rezap )
	{
		firstChannel();
		channelsInit();
		channelList->zapTo( firstchannel.channelNumber -1 );
	}
}

void CNeutrinoApp::startNextRecording()
{
	if(recordingstatus==0 && nextRecordingInfo!=NULL)
	{
		if(CVCRControl::getInstance()->isDeviceRegistered())
		{
			recording_id = nextRecordingInfo->eventID;
			if(CVCRControl::getInstance()->Record(nextRecordingInfo))
				recordingstatus = 1;
			else
				recordingstatus = 0;
		}
		delete nextRecordingInfo;
		nextRecordingInfo=NULL;
	}
}
/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  exec, menuitem callback (shutdown)                         *
*                                                                                     *
**************************************************************************************/
int CNeutrinoApp::exec(CMenuTarget* parent, const std::string & actionKey)
{
	//	printf("ac: %s\n", actionKey.c_str());
	int returnval = menu_return::RETURN_REPAINT;

	if(actionKey=="theme_neutrino")
	{
		setupColors_neutrino();
		colorSetupNotifier->changeNotify("initial", NULL);
	}
	else if(actionKey=="theme_classic")
	{
		setupColors_classic();
		colorSetupNotifier->changeNotify("initial", NULL);
	}
	else if(actionKey=="shutdown")
	{
		ExitRun();
	}
	else if(actionKey=="tv")
	{
		tvMode();
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey=="radio")
	{
		radioMode();
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey=="scart")
	{
		g_RCInput->postMsg( NeutrinoMessages::VCR_ON, 0 );
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey=="network")
	{
		networkConfig.automatic_start = (network_automatic_start == 1);
		networkConfig.commitConfig();
		networkConfig.stopNetwork();
		networkConfig.startNetwork();
	}
	else if(actionKey=="networktest")
	{
		dprintf(DEBUG_INFO, "doing network test...\n");
		testNetworkSettings(networkConfig.address.c_str(), networkConfig.netmask.c_str(), networkConfig.broadcast.c_str(), networkConfig.gateway.c_str(), networkConfig.nameserver.c_str(), networkConfig.inet_static);
	}
	else if(actionKey=="networkshow")
	{
		dprintf(DEBUG_INFO, "showing current network settings...\n");
		showCurrentNetworkSettings();
	}
	else if(actionKey=="savesettings")
	{
		CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_MAINSETTINGS_SAVESETTINGSNOW_HINT)); // UTF-8
		hintBox->paint();

		g_Controld->saveSettings();
		networkConfig.automatic_start = (network_automatic_start == 1);
		networkConfig.commitConfig();
		saveSetup();

		/* send motor position list to zapit */
		if (scanSettings.diseqcMode == DISEQC_1_2)
		{
			printf("[neutrino] sending motor positions list to zapit...\n");
			CZapitClient::ScanMotorPosList motorPosList;
			CNeutrinoApp::getInstance()->getScanSettings().toMotorPosList(motorPosList);
			g_Zapit->setScanMotorPosList(motorPosList);
		}

		hintBox->hide();
		delete hintBox;
	}
	else if(actionKey=="recording")
	{
		setupRecordingDevice();
	}
	else if(actionKey=="reloadchannels")
	{
	 	CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText("servicemenu.reload_hint")); // UTF-8
		hintBox->paint();

		g_Zapit->reinitChannels();

		hintBox->hide();
		delete hintBox;
	}
	else if(strncmp(actionKey.c_str(), "fontsize.d", 10) == 0)
	{
		for (int i = 0; i < 6; i++)
		{
			if (actionKey == font_sizes_groups[i].actionkey)
				for (unsigned int j = 0; j < font_sizes_groups[i].count; j++)
				{
					SNeutrinoSettings::FONT_TYPES k = font_sizes_groups[i].content[j];
					configfile.setInt32(neutrino_font[k].name, neutrino_font[k].defaultsize);
				}
		}
		changeNotify("fontsize.", NULL);
	}
	else if(actionKey=="osd.def")
	{
		for (int i = 0; i < TIMING_SETTING_COUNT; i++)
			g_settings.timing[i] = default_timing[i];

		SetupTiming();
	}
	else if(actionKey == "mp3dir")
	{
		parent->hide();
		CFileBrowser b;
		b.Dir_Mode=true;
		std::string startdir=g_settings.network_nfs_mp3dir;
		if (b.exec(startdir))
			strncpy(g_settings.network_nfs_mp3dir, b.getSelectedFile()->Name.c_str(), sizeof(g_settings.network_nfs_mp3dir)-1);
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "picturedir")
	{
		parent->hide();
		CFileBrowser b;
		b.Dir_Mode=true;
		std::string startdir=g_settings.network_nfs_picturedir;
		if (b.exec(startdir))
			strncpy(g_settings.network_nfs_picturedir, b.getSelectedFile()->Name.c_str(), sizeof(g_settings.network_nfs_picturedir)-1);
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "moviedir")
	{
		parent->hide();
		CFileBrowser b;
		b.Dir_Mode=true;
		std::string startdir=g_settings.network_nfs_moviedir;
		if (b.exec(startdir))
			strncpy(g_settings.network_nfs_moviedir, b.getSelectedFile()->Name.c_str(), sizeof(g_settings.network_nfs_moviedir)-1);
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "recordingdir")
	{
		parent->hide();
		CFileBrowser b;
		b.Dir_Mode=true;
		std::string startdir=g_settings.network_nfs_recordingdir;
		if (b.exec(startdir))
			strncpy(g_settings.network_nfs_recordingdir, b.getSelectedFile()->Name.c_str(), sizeof(g_settings.network_nfs_recordingdir)-1);
		return menu_return::RETURN_REPAINT;
	}

	return returnval;
}

/**************************************************************************************
*                                                                                     *
*          changeNotify - features menu recording start / stop                        *
*                                                                                     *
**************************************************************************************/
bool CNeutrinoApp::changeNotify(const std::string & OptionName, void * data)
{
	return changeNotify(OptionName.c_str(), data);
}

bool CNeutrinoApp::changeNotify(const char * const OptionName, void * data)
{
	for (int i = 0; i < TIMING_SETTING_COUNT; i++)
	{
		if (ARE_LOCALES_EQUAL(OptionName, timing_setting_name[i]))
		{
			g_settings.timing[i] = 	atoi(g_settings.timing_string[i]);
			return true;
		}
	}

	if (strncmp(OptionName, "fontsize.", 9) == 0)
	{
		CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText("fontsize.hint")); // UTF-8
		hintBox->paint();

		SetupFonts();

		hintBox->hide();
		delete hintBox;
	}
	else if ((ARE_LOCALES_EQUAL(OptionName, LOCALE_MAINMENU_RECORDING_START)) || (ARE_LOCALES_EQUAL(OptionName, LOCALE_MAINMENU_RECORDING)))
	{
		CTimerd::RecordingInfo eventinfo;

		if(CVCRControl::getInstance()->isDeviceRegistered())
		{
			if(recordingstatus == 1)
			{
				CZapitClient::CCurrentServiceInfo si = g_Zapit->getCurrentServiceInfo();
				eventinfo.channel_id = CREATE_CHANNEL_ID_FROM_SERVICE_ORIGINALNETWORK_TRANSPORTSTREAM_ID(si.sid, si.onid, si.tsid);
				CEPGData		epgData;
				if (g_Sectionsd->getActualEPGServiceKey(g_RemoteControl->current_channel_id, &epgData ))
				{
					eventinfo.epgID = epgData.eventID;
					eventinfo.epg_starttime = epgData.epg_times.startzeit;
				}
				else
				{
					eventinfo.epgID = 0;
					eventinfo.epg_starttime = 0;
				}
				strcpy(eventinfo.apids, "");

				if(CVCRControl::getInstance()->Record(&eventinfo)==false)
				{
					recordingstatus=0;
					return false;
				}
				else
				{
					time_t now = time(NULL);
					recording_id=g_Timerd->addImmediateRecordTimerEvent(eventinfo.channel_id, now, now+4*60*60, 
											    eventinfo.epgID, eventinfo.epg_starttime);
				}
			}
			else
			{
				g_Timerd->stopTimerEvent(recording_id);
				startNextRecording();
			}
			return true;
		}
		else
			printf("Keine Streamingdevices registriert\n");
	}
	else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_LANGUAGESETUP_SELECT))
	{
		bool use_true_unicode_font = g_Locale->loadLocale(g_settings.language);

		if (font.name == predefined_font[(!use_true_unicode_font)].name)
		{
			font = predefined_font[use_true_unicode_font];
			SetupFonts();
#warning TODO: reload LCD fonts, too
		}
		return true;
	}
	return false;
}



/**************************************************************************************
*                                                                                     *
*          Main programm - no function here                                           *
*                                                                                     *
**************************************************************************************/
int main(int argc, char **argv)
{
	setDebugLevel(DEBUG_NORMAL);

	tzset();
	initGlobals();
	return CNeutrinoApp::getInstance()->run(argc, argv);
}

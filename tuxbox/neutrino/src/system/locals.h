#ifndef __locals__
#define __locals__

/*
 * $Id: locals.h,v 1.59 2005/12/10 10:28:51 barf Exp $
 *
 * (C) 2004 by thegoodguy <thegoodguy@berlios.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

typedef enum
{
	NONEXISTANT_LOCALE,
	LOCALE_AUDIOSELECTMENUE_HEAD,
	LOCALE_EPGMENU_EPGPLUS,
	LOCALE_EPGMENU_EVENTINFO,
	LOCALE_EPGMENU_EVENTLIST,
	LOCALE_EPGMENU_HEAD,
	LOCALE_EPGMENU_STREAMINFO,
	LOCALE_EPGPLUS_ACTIONS,
	LOCALE_EPGPLUS_BYBOUQUET_MODE,
	LOCALE_EPGPLUS_BYPAGE_MODE,
	LOCALE_EPGPLUS_CHANGE_FONT_SIZE,
	LOCALE_EPGPLUS_CHANGE_FONT_STYLE,
	LOCALE_EPGPLUS_CHANGE_SIZE,
	LOCALE_EPGPLUS_CHANNELENTRY_FONT,
	LOCALE_EPGPLUS_CHANNELENTRY_SEPARATIONLINEHEIGHT,
	LOCALE_EPGPLUS_CHANNELENTRY_WIDTH,
	LOCALE_EPGPLUS_CHANNELEVENTENTRY_FONT,
	LOCALE_EPGPLUS_EDIT_FONTS,
	LOCALE_EPGPLUS_EDIT_SIZES,
	LOCALE_EPGPLUS_FONT_STYLE_BOLD,
	LOCALE_EPGPLUS_FONT_STYLE_ITALIC,
	LOCALE_EPGPLUS_FONT_STYLE_REGULAR,
	LOCALE_EPGPLUS_FOOTER_FONTBOUQUETCHANNELNAME,
	LOCALE_EPGPLUS_FOOTER_FONTBUTTONS,
	LOCALE_EPGPLUS_FOOTER_FONTEVENTDESCRIPTION,
	LOCALE_EPGPLUS_FOOTER_FONTEVENTSHORTDESCRIPTION,
	LOCALE_EPGPLUS_HEAD,
	LOCALE_EPGPLUS_HEADER_FONT,
	LOCALE_EPGPLUS_HORGAP1_HEIGHT,
	LOCALE_EPGPLUS_HORGAP2_HEIGHT,
	LOCALE_EPGPLUS_NEXT_BOUQUET,
	LOCALE_EPGPLUS_OPTIONS,
	LOCALE_EPGPLUS_PAGE_DOWN,
	LOCALE_EPGPLUS_PAGE_UP,
	LOCALE_EPGPLUS_PREV_BOUQUET,
	LOCALE_EPGPLUS_RECORD,
	LOCALE_EPGPLUS_REFRESH_EPG,
	LOCALE_EPGPLUS_REMIND,
	LOCALE_EPGPLUS_RESET_SETTINGS,
	LOCALE_EPGPLUS_SAVE_SETTINGS,
	LOCALE_EPGPLUS_SCROLL_MODE,
	LOCALE_EPGPLUS_SELECT_FONT_NAME,
	LOCALE_EPGPLUS_SETTINGS,
	LOCALE_EPGPLUS_SLIDER_WIDTH,
	LOCALE_EPGPLUS_STRETCH_MODE,
	LOCALE_EPGPLUS_SWAP_MODE,
	LOCALE_EPGPLUS_TIMELINE_FONTDATE,
	LOCALE_EPGPLUS_TIMELINE_FONTTIME,
	LOCALE_EPGPLUS_VERGAP1_WIDTH,
	LOCALE_EPGPLUS_VERGAP2_WIDTH,
	LOCALE_EPGPLUS_VIEW_MODE,
	LOCALE_GENRE_ARTS_0,
	LOCALE_GENRE_ARTS_1,
	LOCALE_GENRE_ARTS_10,
	LOCALE_GENRE_ARTS_11,
	LOCALE_GENRE_ARTS_2,
	LOCALE_GENRE_ARTS_3,
	LOCALE_GENRE_ARTS_4,
	LOCALE_GENRE_ARTS_5,
	LOCALE_GENRE_ARTS_6,
	LOCALE_GENRE_ARTS_7,
	LOCALE_GENRE_ARTS_8,
	LOCALE_GENRE_ARTS_9,
	LOCALE_GENRE_CHILDRENS_PROGRAMMES_0,
	LOCALE_GENRE_CHILDRENS_PROGRAMMES_1,
	LOCALE_GENRE_CHILDRENS_PROGRAMMES_2,
	LOCALE_GENRE_CHILDRENS_PROGRAMMES_3,
	LOCALE_GENRE_CHILDRENS_PROGRAMMES_4,
	LOCALE_GENRE_CHILDRENS_PROGRAMMES_5,
	LOCALE_GENRE_DOCUS_MAGAZINES_0,
	LOCALE_GENRE_DOCUS_MAGAZINES_1,
	LOCALE_GENRE_DOCUS_MAGAZINES_2,
	LOCALE_GENRE_DOCUS_MAGAZINES_3,
	LOCALE_GENRE_DOCUS_MAGAZINES_4,
	LOCALE_GENRE_DOCUS_MAGAZINES_5,
	LOCALE_GENRE_DOCUS_MAGAZINES_6,
	LOCALE_GENRE_DOCUS_MAGAZINES_7,
	LOCALE_GENRE_MOVIE_0,
	LOCALE_GENRE_MOVIE_1,
	LOCALE_GENRE_MOVIE_2,
	LOCALE_GENRE_MOVIE_3,
	LOCALE_GENRE_MOVIE_4,
	LOCALE_GENRE_MOVIE_5,
	LOCALE_GENRE_MOVIE_6,
	LOCALE_GENRE_MOVIE_7,
	LOCALE_GENRE_MOVIE_8,
	LOCALE_GENRE_MUSIC_DANCE_0,
	LOCALE_GENRE_MUSIC_DANCE_1,
	LOCALE_GENRE_MUSIC_DANCE_2,
	LOCALE_GENRE_MUSIC_DANCE_3,
	LOCALE_GENRE_MUSIC_DANCE_4,
	LOCALE_GENRE_MUSIC_DANCE_5,
	LOCALE_GENRE_MUSIC_DANCE_6,
	LOCALE_GENRE_NEWS_0,
	LOCALE_GENRE_NEWS_1,
	LOCALE_GENRE_NEWS_2,
	LOCALE_GENRE_NEWS_3,
	LOCALE_GENRE_NEWS_4,
	LOCALE_GENRE_SHOW_0,
	LOCALE_GENRE_SHOW_1,
	LOCALE_GENRE_SHOW_2,
	LOCALE_GENRE_SHOW_3,
	LOCALE_GENRE_SOCIAL_POLITICAL_0,
	LOCALE_GENRE_SOCIAL_POLITICAL_1,
	LOCALE_GENRE_SOCIAL_POLITICAL_2,
	LOCALE_GENRE_SOCIAL_POLITICAL_3,
	LOCALE_GENRE_SPORTS_0,
	LOCALE_GENRE_SPORTS_1,
	LOCALE_GENRE_SPORTS_10,
	LOCALE_GENRE_SPORTS_11,
	LOCALE_GENRE_SPORTS_2,
	LOCALE_GENRE_SPORTS_3,
	LOCALE_GENRE_SPORTS_4,
	LOCALE_GENRE_SPORTS_5,
	LOCALE_GENRE_SPORTS_6,
	LOCALE_GENRE_SPORTS_7,
	LOCALE_GENRE_SPORTS_8,
	LOCALE_GENRE_SPORTS_9,
	LOCALE_GENRE_TRAVEL_HOBBIES_0,
	LOCALE_GENRE_TRAVEL_HOBBIES_1,
	LOCALE_GENRE_TRAVEL_HOBBIES_2,
	LOCALE_GENRE_TRAVEL_HOBBIES_3,
	LOCALE_GENRE_TRAVEL_HOBBIES_4,
	LOCALE_GENRE_TRAVEL_HOBBIES_5,
	LOCALE_GENRE_TRAVEL_HOBBIES_6,
	LOCALE_GENRE_TRAVEL_HOBBIES_7,
	LOCALE_GENRE_UNKNOWN,
	LOCALE_APIDS_HINT_1,
	LOCALE_APIDS_HINT_2,
	LOCALE_APIDSELECTOR_HEAD,
	LOCALE_AUDIOMENU_PCMOFFSET,
	LOCALE_AUDIOMENU_ANALOGOUT,
	LOCALE_AUDIOMENU_AUDIO_LEFT_RIGHT_SELECTABLE,
	LOCALE_AUDIOMENU_AUDIOCHANNEL_UP_DOWN_ENABLE,
	LOCALE_AUDIOMENU_AVS,
	LOCALE_AUDIOMENU_AVS_CONTROL,
	LOCALE_AUDIOMENU_DOLBYDIGITAL,
	LOCALE_AUDIOMENU_HEAD,
	LOCALE_AUDIOMENU_LIRC,
	LOCALE_AUDIOMENU_MONOLEFT,
	LOCALE_AUDIOMENU_MONORIGHT,
	LOCALE_AUDIOMENU_OST,
	LOCALE_AUDIOMENU_STEREO,
	LOCALE_AUDIOPLAYER_ADD,
	LOCALE_AUDIOPLAYER_ARTIST_TITLE,
	LOCALE_AUDIOPLAYER_BUILDING_SEARCH_INDEX,
	LOCALE_AUDIOPLAYER_BUTTON_SELECT_TITLE_BY_ID,
	LOCALE_AUDIOPLAYER_BUTTON_SELECT_TITLE_BY_NAME,
	LOCALE_AUDIOPLAYER_DEFDIR,
	LOCALE_AUDIOPLAYER_DELETE,
	LOCALE_AUDIOPLAYER_DELETEALL,
	LOCALE_AUDIOPLAYER_DISPLAY_ORDER,
	LOCALE_AUDIOPLAYER_ENABLE_SC_METADATA,
	LOCALE_AUDIOPLAYER_FASTFORWARD,
	LOCALE_AUDIOPLAYER_FOLLOW,
	LOCALE_AUDIOPLAYER_HEAD,
	LOCALE_AUDIOPLAYER_HIGHPRIO,
	LOCALE_AUDIOPLAYER_ID3SCAN,
	LOCALE_AUDIOPLAYER_JUMP_BACKWARDS,
	LOCALE_AUDIOPLAYER_JUMP_DIALOG_HINT1,
	LOCALE_AUDIOPLAYER_JUMP_DIALOG_HINT2,
	LOCALE_AUDIOPLAYER_JUMP_DIALOG_TITLE,
	LOCALE_AUDIOPLAYER_JUMP_FORWARDS,
	LOCALE_AUDIOPLAYER_KEYLEVEL,
	LOCALE_AUDIOPLAYER_NAME,
	LOCALE_AUDIOPLAYER_PAUSE,
	LOCALE_AUDIOPLAYER_PLAY,
	LOCALE_AUDIOPLAYER_PLAYING,
	LOCALE_AUDIOPLAYER_PLAYLIST_FILEERROR_MSG,
	LOCALE_AUDIOPLAYER_PLAYLIST_FILEERROR_TITLE,
	LOCALE_AUDIOPLAYER_PLAYLIST_FILEOVERWRITE_MSG,
	LOCALE_AUDIOPLAYER_PLAYLIST_FILEOVERWRITE_TITLE,
	LOCALE_AUDIOPLAYER_PLAYLIST_NAME,
	LOCALE_AUDIOPLAYER_PLAYLIST_NAME_HINT1,
	LOCALE_AUDIOPLAYER_PLAYLIST_NAME_HINT2,
	LOCALE_AUDIOPLAYER_READING_FILES,
	LOCALE_AUDIOPLAYER_REPEAT_ON,
	LOCALE_AUDIOPLAYER_REWIND,
	LOCALE_AUDIOPLAYER_SAVE_PLAYLIST,
	LOCALE_AUDIOPLAYER_SCREENSAVER_TIMEOUT,
	LOCALE_AUDIOPLAYER_SELECT_TITLE_BY_NAME,
	LOCALE_AUDIOPLAYER_SHOW_PLAYLIST,
	LOCALE_AUDIOPLAYER_SHUFFLE,
	LOCALE_AUDIOPLAYER_STOP,
	LOCALE_AUDIOPLAYER_TITLE_ARTIST,
	LOCALE_AUDIOPLAYERPICSETTINGS_GENERAL,
	LOCALE_BOOKMARKMANAGER_DELETE,
	LOCALE_BOOKMARKMANAGER_NAME,
	LOCALE_BOOKMARKMANAGER_RENAME,
	LOCALE_BOOKMARKMANAGER_SELECT,
	LOCALE_BOUQUETEDITOR_ADD,
	LOCALE_BOUQUETEDITOR_BOUQUETNAME,
	LOCALE_BOUQUETEDITOR_DELETE,
	LOCALE_BOUQUETEDITOR_DISCARDINGCHANGES,
	LOCALE_BOUQUETEDITOR_HIDE,
	LOCALE_BOUQUETEDITOR_LOCK,
	LOCALE_BOUQUETEDITOR_MOVE,
	LOCALE_BOUQUETEDITOR_NAME,
	LOCALE_BOUQUETEDITOR_NEWBOUQUETNAME,
	LOCALE_BOUQUETEDITOR_RENAME,
	LOCALE_BOUQUETEDITOR_RETURN,
	LOCALE_BOUQUETEDITOR_SAVECHANGES,
	LOCALE_BOUQUETEDITOR_SAVINGCHANGES,
	LOCALE_BOUQUETEDITOR_SWITCH,
	LOCALE_BOUQUETEDITOR_SWITCHMODE,
	LOCALE_BOUQUETLIST_HEAD,
	LOCALE_CABLESETUP_PROVIDER,
	LOCALE_CHANNELLIST_HEAD,
	LOCALE_CHANNELLIST_NONEFOUND,
	LOCALE_CHANNELLIST_SINCE,
	LOCALE_COLORCHOOSER_ALPHA,
	LOCALE_COLORCHOOSER_BLUE,
	LOCALE_COLORCHOOSER_GREEN,
	LOCALE_COLORCHOOSER_RED,
	LOCALE_COLORMENU_BACKGROUND,
	LOCALE_COLORMENU_BACKGROUND_HEAD,
	LOCALE_COLORMENU_FADE,
	LOCALE_COLORMENU_FONT,
	LOCALE_COLORMENU_GTX_ALPHA,
	LOCALE_COLORMENU_HEAD,
	LOCALE_COLORMENU_MENUCOLORS,
	LOCALE_COLORMENU_STATUSBAR,
	LOCALE_COLORMENU_TEXTCOLOR,
	LOCALE_COLORMENU_TEXTCOLOR_HEAD,
	LOCALE_COLORMENU_THEMESELECT,
	LOCALE_COLORMENU_TIMING,
	LOCALE_COLORMENUSETUP_HEAD,
	LOCALE_COLORMENUSETUP_MENUCONTENT,
	LOCALE_COLORMENUSETUP_MENUCONTENT_INACTIVE,
	LOCALE_COLORMENUSETUP_MENUCONTENT_SELECTED,
	LOCALE_COLORMENUSETUP_MENUHEAD,
	LOCALE_COLORSTATUSBAR_HEAD,
	LOCALE_COLORSTATUSBAR_TEXT,
	LOCALE_COLORTHEMEMENU_CLASSIC_THEME,
	LOCALE_COLORTHEMEMENU_DBLUE_THEME,
	LOCALE_COLORTHEMEMENU_DVB2K_THEME,
	LOCALE_COLORTHEMEMENU_HEAD,
	LOCALE_COLORTHEMEMENU_NEUTRINO_THEME,
	LOCALE_DATE_APR,
	LOCALE_DATE_AUG,
	LOCALE_DATE_DEC,
	LOCALE_DATE_FEB,
	LOCALE_DATE_FRI,
	LOCALE_DATE_JAN,
	LOCALE_DATE_JUL,
	LOCALE_DATE_JUN,
	LOCALE_DATE_MAR,
	LOCALE_DATE_MAY,
	LOCALE_DATE_MON,
	LOCALE_DATE_NOV,
	LOCALE_DATE_OCT,
	LOCALE_DATE_SAT,
	LOCALE_DATE_SEP,
	LOCALE_DATE_SUN,
	LOCALE_DATE_THU,
	LOCALE_DATE_TUE,
	LOCALE_DATE_WED,
	LOCALE_EPGEXTENDED_ACTORS,
	LOCALE_EPGEXTENDED_DIRECTOR,
	LOCALE_EPGEXTENDED_GUESTS,
	LOCALE_EPGEXTENDED_ORIGINAL_TITLE,
	LOCALE_EPGEXTENDED_PRESENTER,
	LOCALE_EPGEXTENDED_YEAR_OF_PRODUCTION,
	LOCALE_EPGLIST_HEAD,
	LOCALE_EPGLIST_NOEVENTS,
	LOCALE_EPGVIEWER_MORE_SCREENINGS,
	LOCALE_EPGVIEWER_NODETAILED,
	LOCALE_EPGVIEWER_NOTFOUND,
	LOCALE_EVENTLISTBAR_CHANNELSWITCH,
	LOCALE_EVENTLISTBAR_EVENTSORT,
	LOCALE_EVENTLISTBAR_RECORDEVENT,
	LOCALE_FAVORITES_ADDCHANNEL,
	LOCALE_FAVORITES_BOUQUETNAME,
	LOCALE_FAVORITES_BQCREATED,
	LOCALE_FAVORITES_CHADDED,
	LOCALE_FAVORITES_CHALREADYINBQ,
	LOCALE_FAVORITES_FINALHINT,
	LOCALE_FAVORITES_MENUEADD,
	LOCALE_FAVORITES_NOBOUQUETS,
	LOCALE_FILEBROWSER_DELETE,
	LOCALE_FILEBROWSER_DENYDIRECTORYLEAVE,
	LOCALE_FILEBROWSER_DODELETE1,
	LOCALE_FILEBROWSER_DODELETE2,
	LOCALE_FILEBROWSER_FILTER_ACTIVE,
	LOCALE_FILEBROWSER_FILTER_INACTIVE,
	LOCALE_FILEBROWSER_HEAD,
	LOCALE_FILEBROWSER_MARK,
	LOCALE_FILEBROWSER_NEXTPAGE,
	LOCALE_FILEBROWSER_PREVPAGE,
	LOCALE_FILEBROWSER_SCAN,
	LOCALE_FILEBROWSER_SELECT,
	LOCALE_FILEBROWSER_SHOWRIGHTS,
	LOCALE_FILEBROWSER_SORT_DATE,
	LOCALE_FILEBROWSER_SORT_NAME,
	LOCALE_FILEBROWSER_SORT_NAMEDIRSFIRST,
	LOCALE_FILEBROWSER_SORT_SIZE,
	LOCALE_FILEBROWSER_SORT_TYPE,
	LOCALE_FILESYSTEM_IS_UTF8,
	LOCALE_FILESYSTEM_IS_UTF8_OPTION_ISO8859_1,
	LOCALE_FILESYSTEM_IS_UTF8_OPTION_UTF8,
	LOCALE_FLASHUPDATE_ACTIONREADFLASH,
	LOCALE_FLASHUPDATE_CANTOPENFILE,
	LOCALE_FLASHUPDATE_CANTOPENMTD,
	LOCALE_FLASHUPDATE_CHECKUPDATE,
	LOCALE_FLASHUPDATE_CURRENTRELEASECYCLE,
	LOCALE_FLASHUPDATE_CURRENTVERSION_SEP,
	LOCALE_FLASHUPDATE_CURRENTVERSIONDATE,
	LOCALE_FLASHUPDATE_CURRENTVERSIONSNAPSHOT,
	LOCALE_FLASHUPDATE_CURRENTVERSIONTIME,
	LOCALE_FLASHUPDATE_ERASEFAILED,
	LOCALE_FLASHUPDATE_ERASING,
	LOCALE_FLASHUPDATE_EXPERIMENTALIMAGE,
	LOCALE_FLASHUPDATE_EXPERTFUNCTIONS,
	LOCALE_FLASHUPDATE_FILEIS0BYTES,
	LOCALE_FLASHUPDATE_FILESELECTOR,
	LOCALE_FLASHUPDATE_FLASHREADYREBOOT,
	LOCALE_FLASHUPDATE_GETINFOFILE,
	LOCALE_FLASHUPDATE_GETINFOFILEERROR,
	LOCALE_FLASHUPDATE_GETUPDATEFILE,
	LOCALE_FLASHUPDATE_GETUPDATEFILEERROR,
	LOCALE_FLASHUPDATE_GLOBALPROGRESS,
	LOCALE_FLASHUPDATE_HEAD,
	LOCALE_FLASHUPDATE_MD5CHECK,
	LOCALE_FLASHUPDATE_MD5SUMERROR,
	LOCALE_FLASHUPDATE_MSGBOX,
	LOCALE_FLASHUPDATE_MSGBOX_MANUAL,
	LOCALE_FLASHUPDATE_MTDSELECTOR,
	LOCALE_FLASHUPDATE_PROGRAMMINGFLASH,
	LOCALE_FLASHUPDATE_PROXYPASSWORD,
	LOCALE_FLASHUPDATE_PROXYPASSWORD_HINT1,
	LOCALE_FLASHUPDATE_PROXYPASSWORD_HINT2,
	LOCALE_FLASHUPDATE_PROXYSERVER,
	LOCALE_FLASHUPDATE_PROXYSERVER_HINT1,
	LOCALE_FLASHUPDATE_PROXYSERVER_HINT2,
	LOCALE_FLASHUPDATE_PROXYSERVER_SEP,
	LOCALE_FLASHUPDATE_PROXYUSERNAME,
	LOCALE_FLASHUPDATE_PROXYUSERNAME_HINT1,
	LOCALE_FLASHUPDATE_PROXYUSERNAME_HINT2,
	LOCALE_FLASHUPDATE_READFLASH,
	LOCALE_FLASHUPDATE_READFLASHMTD,
	LOCALE_FLASHUPDATE_READY,
	LOCALE_FLASHUPDATE_REALLYFLASHMTD,
	LOCALE_FLASHUPDATE_SAVESUCCESS,
	LOCALE_FLASHUPDATE_SELECTIMAGE,
	LOCALE_FLASHUPDATE_SQUASHFS_NOVERSION,
	LOCALE_FLASHUPDATE_TITLEREADFLASH,
	LOCALE_FLASHUPDATE_TITLEWRITEFLASH,
	LOCALE_FLASHUPDATE_UPDATEMODE,
	LOCALE_FLASHUPDATE_UPDATEMODE_INTERNET,
	LOCALE_FLASHUPDATE_UPDATEMODE_MANUAL,
	LOCALE_FLASHUPDATE_URL_FILE,
	LOCALE_FLASHUPDATE_VERSIONCHECK,
	LOCALE_FLASHUPDATE_WRITEFLASH,
	LOCALE_FLASHUPDATE_WRITEFLASHMTD,
	LOCALE_FLASHUPDATE_WRONGBASE,
	LOCALE_FONTMENU_CHANNELLIST,
	LOCALE_FONTMENU_EPG,
	LOCALE_FONTMENU_EVENTLIST,
	LOCALE_FONTMENU_GAMELIST,
	LOCALE_FONTMENU_HEAD,
	LOCALE_FONTMENU_INFOBAR,
	LOCALE_FONTSIZE_CHANNEL_NUM_ZAP,
	LOCALE_FONTSIZE_CHANNELLIST,
	LOCALE_FONTSIZE_CHANNELLIST_DESCR,
	LOCALE_FONTSIZE_CHANNELLIST_NUMBER,
	LOCALE_FONTSIZE_EPG_DATE,
	LOCALE_FONTSIZE_EPG_INFO1,
	LOCALE_FONTSIZE_EPG_INFO2,
	LOCALE_FONTSIZE_EPG_TITLE,
	LOCALE_FONTSIZE_EVENTLIST_DATETIME,
	LOCALE_FONTSIZE_EVENTLIST_ITEMLARGE,
	LOCALE_FONTSIZE_EVENTLIST_ITEMSMALL,
	LOCALE_FONTSIZE_EVENTLIST_TITLE,
	LOCALE_FONTSIZE_FILEBROWSER_ITEM,
	LOCALE_FONTSIZE_GAMELIST_ITEMLARGE,
	LOCALE_FONTSIZE_GAMELIST_ITEMSMALL,
	LOCALE_FONTSIZE_HINT,
	LOCALE_FONTSIZE_IMAGEINFO_INFO,
	LOCALE_FONTSIZE_IMAGEINFO_SMALL,
	LOCALE_FONTSIZE_INFOBAR_CHANNAME,
	LOCALE_FONTSIZE_INFOBAR_INFO,
	LOCALE_FONTSIZE_INFOBAR_NUMBER,
	LOCALE_FONTSIZE_INFOBAR_SMALL,
	LOCALE_FONTSIZE_MENU,
	LOCALE_FONTSIZE_MENU_INFO,
	LOCALE_FONTSIZE_MENU_TITLE,
	LOCALE_GTXALPHA_ALPHA1,
	LOCALE_GTXALPHA_ALPHA2,
	LOCALE_IMAGEINFO_CREATOR,
	LOCALE_IMAGEINFO_DATE,
	LOCALE_IMAGEINFO_DOKUMENTATION,
	LOCALE_IMAGEINFO_FORUM,
	LOCALE_IMAGEINFO_HEAD,
	LOCALE_IMAGEINFO_HOMEPAGE,
	LOCALE_IMAGEINFO_IMAGE,
	LOCALE_IMAGEINFO_LICENSE,
	LOCALE_IMAGEINFO_VERSION,
	LOCALE_INFOVIEWER_EPGNOTLOAD,
	LOCALE_INFOVIEWER_EPGWAIT,
	LOCALE_INFOVIEWER_EVENTLIST,
	LOCALE_INFOVIEWER_LANGUAGES,
	LOCALE_INFOVIEWER_MOTOR_MOVING,
	LOCALE_INFOVIEWER_NOCURRENT,
	LOCALE_INFOVIEWER_NOEPG,
	LOCALE_INFOVIEWER_NOTAVAILABLE,
	LOCALE_INFOVIEWER_SELECTTIME,
	LOCALE_INFOVIEWER_STREAMINFO,
	LOCALE_INFOVIEWER_SUBCHAN_DISP_POS,
	LOCALE_INFOVIEWER_SUBSERVICE,
	LOCALE_INFOVIEWER_WAITTIME,
	LOCALE_IPSETUP_HINT_1,
	LOCALE_IPSETUP_HINT_2,
	LOCALE_KEYBINDINGMENU_RC,
	LOCALE_KEYBINDINGMENU_ADDRECORD,
	LOCALE_KEYBINDINGMENU_ADDRECORD_HEAD,
	LOCALE_KEYBINDINGMENU_ADDREMIND,
	LOCALE_KEYBINDINGMENU_ADDREMIND_HEAD,
	LOCALE_KEYBINDINGMENU_ALLCHANNELS_ON_OK,
	LOCALE_KEYBINDINGMENU_BOUQUETCHANNELS_ON_OK,
	LOCALE_KEYBINDINGMENU_BOUQUETDOWN,
	LOCALE_KEYBINDINGMENU_BOUQUETDOWN_HEAD,
	LOCALE_KEYBINDINGMENU_BOUQUETHANDLING,
	LOCALE_KEYBINDINGMENU_BOUQUETLIST_ON_OK,
	LOCALE_KEYBINDINGMENU_BOUQUETUP,
	LOCALE_KEYBINDINGMENU_BOUQUETUP_HEAD,
	LOCALE_KEYBINDINGMENU_CANCEL,
	LOCALE_KEYBINDINGMENU_CANCEL_HEAD,
	LOCALE_KEYBINDINGMENU_CHANNELDOWN,
	LOCALE_KEYBINDINGMENU_CHANNELDOWN_HEAD,
	LOCALE_KEYBINDINGMENU_CHANNELLIST,
	LOCALE_KEYBINDINGMENU_CHANNELUP,
	LOCALE_KEYBINDINGMENU_CHANNELUP_HEAD,
	LOCALE_KEYBINDINGMENU_HEAD,
	LOCALE_KEYBINDINGMENU_LASTCHANNEL,
	LOCALE_KEYBINDINGMENU_LASTCHANNEL_HEAD,
	LOCALE_KEYBINDINGMENU_MODECHANGE,
	LOCALE_KEYBINDINGMENU_PAGEDOWN,
	LOCALE_KEYBINDINGMENU_PAGEDOWN_HEAD,
	LOCALE_KEYBINDINGMENU_PAGEUP,
	LOCALE_KEYBINDINGMENU_PAGEUP_HEAD,
	LOCALE_KEYBINDINGMENU_QUICKZAP,
	LOCALE_KEYBINDINGMENU_REPEATBLOCK,
	LOCALE_KEYBINDINGMENU_REPEATBLOCKGENERIC,
	LOCALE_KEYBINDINGMENU_SORT,
	LOCALE_KEYBINDINGMENU_SORT_HEAD,
	LOCALE_KEYBINDINGMENU_SUBCHANNELDOWN,
	LOCALE_KEYBINDINGMENU_SUBCHANNELDOWN_HEAD,
	LOCALE_KEYBINDINGMENU_SUBCHANNELUP,
	LOCALE_KEYBINDINGMENU_SUBCHANNELUP_HEAD,
	LOCALE_KEYBINDINGMENU_TVRADIOMODE,
	LOCALE_KEYBINDINGMENU_TVRADIOMODE_HEAD,
	LOCALE_KEYBINDINGMENU_ZAPHISTORY,
	LOCALE_KEYBINDINGMENU_ZAPHISTORY_HEAD,
	LOCALE_KEYCHOOSER_HEAD,
	LOCALE_KEYCHOOSER_TEXT1,
	LOCALE_KEYCHOOSER_TEXT2,
	LOCALE_KEYCHOOSERMENU_CURRENTKEY,
	LOCALE_KEYCHOOSERMENU_SETNEW,
	LOCALE_KEYCHOOSERMENU_SETNONE,
	LOCALE_LANGUAGESETUP_HEAD,
	LOCALE_LANGUAGESETUP_SELECT,
	LOCALE_LCDCONTROLER_BRIGHTNESS,
	LOCALE_LCDCONTROLER_BRIGHTNESSSTANDBY,
	LOCALE_LCDCONTROLER_CONTRAST,
	LOCALE_LCDCONTROLER_HEAD,
	LOCALE_LCDMENU_AUTODIMM,
	LOCALE_LCDMENU_DIM_BRIGHTNESS,
	LOCALE_LCDMENU_DIM_TIME,
	LOCALE_LCDMENU_HEAD,
	LOCALE_LCDMENU_INVERSE,
	LOCALE_LCDMENU_LCDCONTROLER,
	LOCALE_LCDMENU_POWER,
	LOCALE_LCDMENU_STATUSLINE,
	LOCALE_LCDMENU_STATUSLINE_BOTH,
	LOCALE_LCDMENU_STATUSLINE_PLAYTIME,
	LOCALE_LCDMENU_STATUSLINE_VOLUME,
	LOCALE_MAINMENU_AUDIOPLAYER,
	LOCALE_MAINMENU_GAMES,
	LOCALE_MAINMENU_HEAD,
	LOCALE_MAINMENU_MOVIEPLAYER,
	LOCALE_MAINMENU_PAUSESECTIONSD,
	LOCALE_MAINMENU_PICTUREVIEWER,
	LOCALE_MAINMENU_RADIOMODE,
	LOCALE_MAINMENU_RECORDING,
	LOCALE_MAINMENU_RECORDING_START,
	LOCALE_MAINMENU_RECORDING_STOP,
	LOCALE_MAINMENU_SCARTMODE,
	LOCALE_MAINMENU_SCRIPTS,
	LOCALE_MAINMENU_SERVICE,
	LOCALE_MAINMENU_SETTINGS,
	LOCALE_MAINMENU_SHUTDOWN,
	LOCALE_MAINMENU_SLEEPTIMER,
	LOCALE_MAINMENU_TVMODE,
	LOCALE_MAINSETTINGS_AUDIO,
	LOCALE_MAINSETTINGS_COLORS,
	LOCALE_MAINSETTINGS_HEAD,
	LOCALE_MAINSETTINGS_KEYBINDING,
	LOCALE_MAINSETTINGS_LANGUAGE,
	LOCALE_MAINSETTINGS_LCD,
	LOCALE_MAINSETTINGS_MISC,
	LOCALE_MAINSETTINGS_NETWORK,
	LOCALE_MAINSETTINGS_RECORDING,
	LOCALE_MAINSETTINGS_SAVESETTINGSNOW,
	LOCALE_MAINSETTINGS_SAVESETTINGSNOW_HINT,
	LOCALE_MAINSETTINGS_STREAMING,
	LOCALE_MAINSETTINGS_VIDEO,
	LOCALE_MENU_BACK,
	LOCALE_MESSAGEBOX_BACK,
	LOCALE_MESSAGEBOX_CANCEL,
	LOCALE_MESSAGEBOX_DISCARD,
	LOCALE_MESSAGEBOX_ERROR,
	LOCALE_MESSAGEBOX_INFO,
	LOCALE_MESSAGEBOX_NO,
	LOCALE_MESSAGEBOX_YES,
	LOCALE_MISCSETTINGS_BOOTINFO,
	LOCALE_MISCSETTINGS_BOOTMENU,
	LOCALE_MISCSETTINGS_DRIVER_BOOT,
	LOCALE_MISCSETTINGS_FB_DESTINATION,
	LOCALE_MISCSETTINGS_GENERAL,
	LOCALE_MISCSETTINGS_HEAD,
	LOCALE_MISCSETTINGS_HWSECTIONS,
	LOCALE_MISCSETTINGS_INFOBAR_SAT_DISPLAY,
	LOCALE_MISCSETTINGS_NOAVIAWATCHDOG,
	LOCALE_MISCSETTINGS_NOENXWATCHDOG,
	LOCALE_MISCSETTINGS_PMTUPDATE,
	LOCALE_MISCSETTINGS_SHUTDOWN_COUNT,
	LOCALE_MISCSETTINGS_SHUTDOWN_COUNT_HINT1,
	LOCALE_MISCSETTINGS_SHUTDOWN_COUNT_HINT2,
	LOCALE_MISCSETTINGS_SHUTDOWN_REAL,
	LOCALE_MISCSETTINGS_SHUTDOWN_REAL_RCDELAY,
	LOCALE_MISCSETTINGS_SPTSMODE,
	LOCALE_MISCSETTINGS_STARTBHDRIVER,
	LOCALE_MISCSETTINGS_TUXTXT_CACHE,
	LOCALE_MOTORCONTROL_HEAD,
	LOCALE_MOVIEBROWSER_BOOK_HEAD,
	LOCALE_MOVIEBROWSER_BOOK_LASTMOVIESTOP,
	LOCALE_MOVIEBROWSER_BOOK_MOVIEEND,
	LOCALE_MOVIEBROWSER_BOOK_MOVIESTART,
	LOCALE_MOVIEBROWSER_BOOK_NAME,
	LOCALE_MOVIEBROWSER_BOOK_NEW,
	LOCALE_MOVIEBROWSER_BOOK_POSITION,
	LOCALE_MOVIEBROWSER_BOOK_TYPE,
	LOCALE_MOVIEBROWSER_BOOK_TYPE_BACKWARD,
	LOCALE_MOVIEBROWSER_BOOK_TYPE_FORWARD,
	LOCALE_MOVIEBROWSER_EDIT_BOOK,
	LOCALE_MOVIEBROWSER_EDIT_BOOK_NAME_INFO1,
	LOCALE_MOVIEBROWSER_EDIT_BOOK_NAME_INFO2,
	LOCALE_MOVIEBROWSER_EDIT_BOOK_POS_INFO1,
	LOCALE_MOVIEBROWSER_EDIT_BOOK_POS_INFO2,
	LOCALE_MOVIEBROWSER_EDIT_BOOK_TYPE_INFO1,
	LOCALE_MOVIEBROWSER_EDIT_BOOK_TYPE_INFO2,
	LOCALE_MOVIEBROWSER_EDIT_SERIE,
	LOCALE_MOVIEBROWSER_ERROR_NO_MOVIES,
	LOCALE_MOVIEBROWSER_FOOT_FILTER,
	LOCALE_MOVIEBROWSER_FOOT_PLAY,
	LOCALE_MOVIEBROWSER_FOOT_SORT,
	LOCALE_MOVIEBROWSER_HEAD,
	LOCALE_MOVIEBROWSER_HEAD_FILTER,
	LOCALE_MOVIEBROWSER_HEAD_PLAYLIST,
	LOCALE_MOVIEBROWSER_HEAD_RECORDLIST,
	LOCALE_MOVIEBROWSER_HINT_JUMPBACKWARD,
	LOCALE_MOVIEBROWSER_HINT_JUMPFORWARD,
	LOCALE_MOVIEBROWSER_HINT_MOVIEEND,
	LOCALE_MOVIEBROWSER_HINT_NEWBOOK_BACKWARD,
	LOCALE_MOVIEBROWSER_HINT_NEWBOOK_FORWARD,
	LOCALE_MOVIEBROWSER_INFO_AUDIO,
	LOCALE_MOVIEBROWSER_INFO_CHANNEL,
	LOCALE_MOVIEBROWSER_INFO_FILENAME,
	LOCALE_MOVIEBROWSER_INFO_GENRE_MAJOR,
	LOCALE_MOVIEBROWSER_INFO_GENRE_MINOR,
	LOCALE_MOVIEBROWSER_INFO_HEAD,
	LOCALE_MOVIEBROWSER_INFO_INFO1,
	LOCALE_MOVIEBROWSER_INFO_INFO2,
	LOCALE_MOVIEBROWSER_INFO_LENGTH,
	LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE,
	LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_0YEAR,
	LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_12YEAR,
	LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_16YEAR,
	LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_18YEAR,
	LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_6YEAR,
	LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_ALWAYS,
	LOCALE_MOVIEBROWSER_INFO_PATH,
	LOCALE_MOVIEBROWSER_INFO_PREVPLAYDATE,
	LOCALE_MOVIEBROWSER_INFO_PRODCOUNTRY,
	LOCALE_MOVIEBROWSER_INFO_PRODYEAR,
	LOCALE_MOVIEBROWSER_INFO_QUALITY,
	LOCALE_MOVIEBROWSER_INFO_RECORDDATE,
	LOCALE_MOVIEBROWSER_INFO_SERIE,
	LOCALE_MOVIEBROWSER_INFO_SIZE,
	LOCALE_MOVIEBROWSER_INFO_TITLE,
	LOCALE_MOVIEBROWSER_INFO_VIDEOFORMAT,
	LOCALE_MOVIEBROWSER_MENU_DIRECTORIES_HEAD,
	LOCALE_MOVIEBROWSER_MENU_HELP_HEAD,
	LOCALE_MOVIEBROWSER_MENU_MAIN_BOOKMARKS,
	LOCALE_MOVIEBROWSER_MENU_MAIN_HEAD,
	LOCALE_MOVIEBROWSER_MENU_MAIN_MOVIEINFO,
	LOCALE_MOVIEBROWSER_MENU_MAIN_SAVEANDBACK,
	LOCALE_MOVIEBROWSER_MENU_NFS_HEAD,
	LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_ACTIVATED,
	LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_ACTIVATED_NO,
	LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_ACTIVATED_YES,
	LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_ACTIVATED_YES_TEMP,
	LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_HEAD,
	LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_RATE_HEAD,
	LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES,
	LOCALE_MOVIEBROWSER_SERIE_EXISTINGNAME,
	LOCALE_MOVIEBROWSER_SERIE_HEAD,
	LOCALE_MOVIEBROWSER_SERIE_NAME,
	LOCALE_MOVIEBROWSER_SHORT_AUDIO,
	LOCALE_MOVIEBROWSER_SHORT_BOOK,
	LOCALE_MOVIEBROWSER_SHORT_CHANNEL,
	LOCALE_MOVIEBROWSER_SHORT_COUNTRY,
	LOCALE_MOVIEBROWSER_SHORT_FILENAME,
	LOCALE_MOVIEBROWSER_SHORT_FORMAT,
	LOCALE_MOVIEBROWSER_SHORT_GENRE_MAJOR,
	LOCALE_MOVIEBROWSER_SHORT_GENRE_MINOR,
	LOCALE_MOVIEBROWSER_SHORT_INFO1,
	LOCALE_MOVIEBROWSER_SHORT_INFO2,
	LOCALE_MOVIEBROWSER_SHORT_LENGTH,
	LOCALE_MOVIEBROWSER_SHORT_PARENTAL_LOCKAGE,
	LOCALE_MOVIEBROWSER_SHORT_PATH,
	LOCALE_MOVIEBROWSER_SHORT_PREVPLAYDATE,
	LOCALE_MOVIEBROWSER_SHORT_PRODYEAR,
	LOCALE_MOVIEBROWSER_SHORT_QUALITY,
	LOCALE_MOVIEBROWSER_SHORT_RECORDDATE,
	LOCALE_MOVIEBROWSER_SHORT_SERIE,
	LOCALE_MOVIEBROWSER_SHORT_SIZE,
	LOCALE_MOVIEBROWSER_SHORT_TITLE,
	LOCALE_MOVIEBROWSER_START_HEAD,
	LOCALE_MOVIEBROWSER_START_RECORD_START,
	LOCALE_MOVIEPLAYER_BOOKMARK,
	LOCALE_MOVIEPLAYER_BOOKMARKNAME,
	LOCALE_MOVIEPLAYER_BOOKMARKNAME_HINT1,
	LOCALE_MOVIEPLAYER_BOOKMARKNAME_HINT2,
	LOCALE_MOVIEPLAYER_BUFFERING,
	LOCALE_MOVIEPLAYER_DEFDIR,
	LOCALE_MOVIEPLAYER_DEFPLUGIN,
	LOCALE_MOVIEPLAYER_DVDPLAYBACK,
	LOCALE_MOVIEPLAYER_FILEPLAYBACK,
	LOCALE_MOVIEPLAYER_GOTO,
	LOCALE_MOVIEPLAYER_GOTO_H1,
	LOCALE_MOVIEPLAYER_GOTO_H2,
	LOCALE_MOVIEPLAYER_HEAD,
	LOCALE_MOVIEPLAYER_NOSTREAMINGSERVER,
	LOCALE_MOVIEPLAYER_PLEASEWAIT,
	LOCALE_MOVIEPLAYER_TOOMANYBOOKMARKS,
	LOCALE_MOVIEPLAYER_TSHELP1,
	LOCALE_MOVIEPLAYER_TSHELP10,
	LOCALE_MOVIEPLAYER_TSHELP11,
	LOCALE_MOVIEPLAYER_TSHELP12,
	LOCALE_MOVIEPLAYER_TSHELP2,
	LOCALE_MOVIEPLAYER_TSHELP3,
	LOCALE_MOVIEPLAYER_TSHELP4,
	LOCALE_MOVIEPLAYER_TSHELP5,
	LOCALE_MOVIEPLAYER_TSHELP6,
	LOCALE_MOVIEPLAYER_TSHELP7,
	LOCALE_MOVIEPLAYER_TSHELP8,
	LOCALE_MOVIEPLAYER_TSHELP9,
	LOCALE_MOVIEPLAYER_TSPLAYBACK,
	LOCALE_MOVIEPLAYER_TSPLAYBACK_PC,
	LOCALE_MOVIEPLAYER_VCDPLAYBACK,
	LOCALE_MOVIEPLAYER_VLCHELP1,
	LOCALE_MOVIEPLAYER_VLCHELP10,
	LOCALE_MOVIEPLAYER_VLCHELP11,
	LOCALE_MOVIEPLAYER_VLCHELP12,
	LOCALE_MOVIEPLAYER_VLCHELP2,
	LOCALE_MOVIEPLAYER_VLCHELP3,
	LOCALE_MOVIEPLAYER_VLCHELP4,
	LOCALE_MOVIEPLAYER_VLCHELP5,
	LOCALE_MOVIEPLAYER_VLCHELP6,
	LOCALE_MOVIEPLAYER_VLCHELP7,
	LOCALE_MOVIEPLAYER_VLCHELP8,
	LOCALE_MOVIEPLAYER_VLCHELP9,
	LOCALE_MOVIEPLAYER_WRONGVLCVERSION,
	LOCALE_NETWORKMENU_BROADCAST,
	LOCALE_NETWORKMENU_DHCP,
	LOCALE_NETWORKMENU_GATEWAY,
	LOCALE_NETWORKMENU_HEAD,
	LOCALE_NETWORKMENU_IPADDRESS,
	LOCALE_NETWORKMENU_MOUNT,
	LOCALE_NETWORKMENU_NAMESERVER,
	LOCALE_NETWORKMENU_NETMASK,
	LOCALE_NETWORKMENU_SETUPNOW,
	LOCALE_NETWORKMENU_SETUPONSTARTUP,
	LOCALE_NETWORKMENU_SHOW,
	LOCALE_NETWORKMENU_TEST,
	LOCALE_NFS_ALREADYMOUNTED,
	LOCALE_NFS_AUTOMOUNT,
	LOCALE_NFS_DIR,
	LOCALE_NFS_IP,
	LOCALE_NFS_LOCALDIR,
	LOCALE_NFS_MOUNT,
	LOCALE_NFS_MOUNT_OPTIONS,
	LOCALE_NFS_MOUNTERROR,
	LOCALE_NFS_MOUNTERROR_NOTSUP,
	LOCALE_NFS_MOUNTNOW,
	LOCALE_NFS_MOUNTOK,
	LOCALE_NFS_MOUNTTIMEOUT,
	LOCALE_NFS_PASSWORD,
	LOCALE_NFS_REMOUNT,
	LOCALE_NFS_TYPE,
	LOCALE_NFS_TYPE_CIFS,
	LOCALE_NFS_TYPE_LUFS,
	LOCALE_NFS_TYPE_NFS,
	LOCALE_NFS_UMOUNT,
	LOCALE_NFS_UMOUNTERROR,
	LOCALE_NFS_USERNAME,
	LOCALE_NFSMENU_HEAD,
	LOCALE_NVOD_PERCENTAGE,
	LOCALE_NVOD_STARTING,
	LOCALE_NVODSELECTOR_DIRECTORMODE,
	LOCALE_NVODSELECTOR_HEAD,
	LOCALE_NVODSELECTOR_SUBSERVICE,
	LOCALE_OPTIONS_DEFAULT,
	LOCALE_OPTIONS_FB,
	LOCALE_OPTIONS_NULL,
	LOCALE_OPTIONS_OFF,
	LOCALE_OPTIONS_ON,
	LOCALE_OPTIONS_ON_WITHOUT_MESSAGES,
	LOCALE_OPTIONS_SERIAL,
	LOCALE_PARENTALLOCK_CHANGEPIN,
	LOCALE_PARENTALLOCK_CHANGEPIN_HINT1,
	LOCALE_PARENTALLOCK_CHANGETOLOCKED,
	LOCALE_PARENTALLOCK_HEAD,
	LOCALE_PARENTALLOCK_LOCKAGE,
	LOCALE_PARENTALLOCK_LOCKAGE12,
	LOCALE_PARENTALLOCK_LOCKAGE16,
	LOCALE_PARENTALLOCK_LOCKAGE18,
	LOCALE_PARENTALLOCK_LOCKEDCHANNEL,
	LOCALE_PARENTALLOCK_LOCKEDPROGRAM,
	LOCALE_PARENTALLOCK_NEVER,
	LOCALE_PARENTALLOCK_ONSIGNAL,
	LOCALE_PARENTALLOCK_PARENTALLOCK,
	LOCALE_PARENTALLOCK_PROMPT,
	LOCALE_PICTUREVIEWER_DECODE_SERVER_IP,
	LOCALE_PICTUREVIEWER_DECODE_SERVER_PORT,
	LOCALE_PICTUREVIEWER_DEFDIR,
	LOCALE_PICTUREVIEWER_HEAD,
	LOCALE_PICTUREVIEWER_HELP1,
	LOCALE_PICTUREVIEWER_HELP10,
	LOCALE_PICTUREVIEWER_HELP11,
	LOCALE_PICTUREVIEWER_HELP12,
	LOCALE_PICTUREVIEWER_HELP13,
	LOCALE_PICTUREVIEWER_HELP14,
	LOCALE_PICTUREVIEWER_HELP15,
	LOCALE_PICTUREVIEWER_HELP16,
	LOCALE_PICTUREVIEWER_HELP17,
	LOCALE_PICTUREVIEWER_HELP18,
	LOCALE_PICTUREVIEWER_HELP19,
	LOCALE_PICTUREVIEWER_HELP2,
	LOCALE_PICTUREVIEWER_HELP20,
	LOCALE_PICTUREVIEWER_HELP21,
	LOCALE_PICTUREVIEWER_HELP22,
	LOCALE_PICTUREVIEWER_HELP3,
	LOCALE_PICTUREVIEWER_HELP4,
	LOCALE_PICTUREVIEWER_HELP5,
	LOCALE_PICTUREVIEWER_HELP6,
	LOCALE_PICTUREVIEWER_HELP7,
	LOCALE_PICTUREVIEWER_HELP8,
	LOCALE_PICTUREVIEWER_HELP9,
	LOCALE_PICTUREVIEWER_RESIZE_COLOR_AVERAGE,
	LOCALE_PICTUREVIEWER_RESIZE_NONE,
	LOCALE_PICTUREVIEWER_RESIZE_SIMPLE,
	LOCALE_PICTUREVIEWER_SCALING,
	LOCALE_PICTUREVIEWER_SHOW,
	LOCALE_PICTUREVIEWER_SLIDE_TIME,
	LOCALE_PICTUREVIEWER_SLIDESHOW,
	LOCALE_PICTUREVIEWER_SORTORDER,
	LOCALE_PICTUREVIEWER_SORTORDER_DATE,
	LOCALE_PICTUREVIEWER_SORTORDER_FILENAME,
	LOCALE_PING_OK,
	LOCALE_PING_PROTOCOL,
	LOCALE_PING_SOCKET,
	LOCALE_PING_UNREACHABLE,
	LOCALE_PINPROTECTION_HEAD,
	LOCALE_PINPROTECTION_WRONGCODE,
	LOCALE_PLUGINS_RESULT,
	LOCALE_RCLOCK_LOCKMSG,
	LOCALE_RCLOCK_MENUEADD,
	LOCALE_RCLOCK_TITLE,
	LOCALE_RCLOCK_UNLOCKMSG,
	LOCALE_RECORDINGMENU_CHOOSE_DIRECT_REC_DIR,
	LOCALE_RECORDINGMENU_DEFDIR,
	LOCALE_RECORDINGMENU_DIR_PERMISSIONS,
	LOCALE_RECORDINGMENU_DIR_PERMISSIONS_HINT,
	LOCALE_RECORDINGMENU_EPG_FOR_FILENAME,
	LOCALE_RECORDINGMENU_FILE,
	LOCALE_RECORDINGMENU_FILENAME_TEMPLATE,
	LOCALE_RECORDINGMENU_FILENAME_TEMPLATE_HINT,
	LOCALE_RECORDINGMENU_FILESETTINGS,
	LOCALE_RECORDINGMENU_HEAD,
	LOCALE_RECORDINGMENU_HELP,
	LOCALE_RECORDINGMENU_NO_SCART,
	LOCALE_RECORDINGMENU_OFF,
	LOCALE_RECORDINGMENU_RECORD_IN_SPTS_MODE,
	LOCALE_RECORDINGMENU_RECORDING_TYPE,
	LOCALE_RECORDINGMENU_RINGBUFFERS,
	LOCALE_RECORDINGMENU_SERVER,
	LOCALE_RECORDINGMENU_SERVER_IP,
	LOCALE_RECORDINGMENU_SERVER_MAC,
	LOCALE_RECORDINGMENU_SERVER_PORT,
	LOCALE_RECORDINGMENU_SERVER_WAKEUP,
	LOCALE_RECORDINGMENU_SETUPNOW,
	LOCALE_RECORDINGMENU_SPLITSIZE,
	LOCALE_RECORDINGMENU_STOPPLAYBACK,
	LOCALE_RECORDINGMENU_STOPSECTIONSD,
	LOCALE_RECORDINGMENU_STREAM_ALL_AUDIO_PIDS,
	LOCALE_RECORDINGMENU_STREAM_VTXT_PID,
	LOCALE_RECORDINGMENU_USE_FDATASYNC,
	LOCALE_RECORDINGMENU_USE_O_SYNC,
	LOCALE_RECORDINGMENU_VCR,
	LOCALE_RECORDTIMER_ANNOUNCE,
	LOCALE_REPEATBLOCKER_HINT_1,
	LOCALE_REPEATBLOCKER_HINT_2,
	LOCALE_SATSETUP_DISEQC,
	LOCALE_SATSETUP_DISEQC10,
	LOCALE_SATSETUP_DISEQC11,
	LOCALE_SATSETUP_DISEQC12,
	LOCALE_SATSETUP_DISEQCREPEAT,
	LOCALE_SATSETUP_EXTENDED,
	LOCALE_SATSETUP_EXTENDED_MOTOR,
	LOCALE_SATSETUP_MINIDISEQC,
	LOCALE_SATSETUP_MOTORCONTROL,
	LOCALE_SATSETUP_NODISEQC,
	LOCALE_SATSETUP_SATELLITE,
	LOCALE_SATSETUP_SAVESETTINGSNOW,
	LOCALE_SATSETUP_SMATVREMOTE,
	LOCALE_SCANTP_FEC,
	LOCALE_SCANTP_FEC_1_2,
	LOCALE_SCANTP_FEC_2_3,
	LOCALE_SCANTP_FEC_3_4,
	LOCALE_SCANTP_FEC_5_6,
	LOCALE_SCANTP_FEC_7_8,
	LOCALE_SCANTP_FREQ,
	LOCALE_SCANTP_POL,
	LOCALE_SCANTP_POL_H,
	LOCALE_SCANTP_POL_V,
	LOCALE_SCANTP_RATE,
	LOCALE_SCANTP_SCAN,
	LOCALE_SCANTP_SCANMODE,
	LOCALE_SCANTS_ABORT_BODY,
	LOCALE_SCANTS_ABORT_HEADER,
	LOCALE_SCANTS_ACTCABLE,
	LOCALE_SCANTS_ACTSATELLITE,
	LOCALE_SCANTS_BOUQUET,
	LOCALE_SCANTS_BOUQUET_CREATE,
	LOCALE_SCANTS_BOUQUET_ERASE,
	LOCALE_SCANTS_BOUQUET_LEAVE,
	LOCALE_SCANTS_BOUQUET_SATELLITE,
	LOCALE_SCANTS_BOUQUET_UPDATE,
	LOCALE_SCANTS_CHANNEL,
	LOCALE_SCANTS_FAILED,
	LOCALE_SCANTS_FINISHED,
	LOCALE_SCANTS_FREQDATA,
	LOCALE_SCANTS_HEAD,
	LOCALE_SCANTS_NUMBEROFDATASERVICES,
	LOCALE_SCANTS_NUMBEROFRADIOSERVICES,
	LOCALE_SCANTS_NUMBEROFTOTALSERVICES,
	LOCALE_SCANTS_NUMBEROFTVSERVICES,
	LOCALE_SCANTS_PROVIDER,
	LOCALE_SCANTS_STARTNOW,
	LOCALE_SCANTS_TRANSPONDERS,
	LOCALE_SCREENSETUP_LOWERRIGHT,
	LOCALE_SCREENSETUP_UPPERLEFT,
	LOCALE_SECTIONSD_SCANMODE,
	LOCALE_SERVICEMENU_GETPLUGINS,
	LOCALE_SERVICEMENU_GETPLUGINS_HINT,
	LOCALE_SERVICEMENU_HEAD,
	LOCALE_SERVICEMENU_IMAGEINFO,
	LOCALE_SERVICEMENU_RELOAD,
	LOCALE_SERVICEMENU_RELOAD_HINT,
	LOCALE_SERVICEMENU_RESTART,
	LOCALE_SERVICEMENU_RESTART_FAILED,
	LOCALE_SERVICEMENU_RESTART_HINT,
	LOCALE_SERVICEMENU_RESTART_REFUSED_RECORDING,
	LOCALE_SERVICEMENU_SCANTS,
	LOCALE_SERVICEMENU_UCODECHECK,
	LOCALE_SERVICEMENU_UPDATE,
	LOCALE_SETTINGS_HELP,
	LOCALE_SETTINGS_MISSINGOPTIONSCONFFILE,
	LOCALE_SETTINGS_NOCONFFILE,
	LOCALE_SETTINGS_POS_BOTTOM_LEFT,
	LOCALE_SETTINGS_POS_BOTTOM_RIGHT,
	LOCALE_SETTINGS_POS_TOP_LEFT,
	LOCALE_SETTINGS_POS_TOP_RIGHT,
	LOCALE_SHUTDOWN_RECODING_QUERY,
	LOCALE_SHUTDOWNTIMER_ANNOUNCE,
	LOCALE_SLEEPTIMERBOX_ANNOUNCE,
	LOCALE_SLEEPTIMERBOX_HINT1,
	LOCALE_SLEEPTIMERBOX_HINT2,
	LOCALE_SLEEPTIMERBOX_TITLE,
	LOCALE_STREAMFEATURES_HEAD,
	LOCALE_STREAMINFO_ARATIO,
	LOCALE_STREAMINFO_ARATIO_UNKNOWN,
	LOCALE_STREAMINFO_AUDIOTYPE,
	LOCALE_STREAMINFO_AUDIOTYPE_UNKNOWN,
	LOCALE_STREAMINFO_BITRATE,
	LOCALE_STREAMINFO_FRAMERATE,
	LOCALE_STREAMINFO_FRAMERATE_UNKNOWN,
	LOCALE_STREAMINFO_HEAD,
	LOCALE_STREAMINFO_NOT_AVAILABLE,
	LOCALE_STREAMINFO_RESOLUTION,
	LOCALE_STREAMINFO_SIGNAL,
	LOCALE_STREAMING_BUFFER_OVERFLOW,
	LOCALE_STREAMING_BUSY,
	LOCALE_STREAMING_DIR_NOT_WRITABLE,
	LOCALE_STREAMING_SUCCESS,
	LOCALE_STREAMING_WRITE_ERROR,
	LOCALE_STREAMING_WRITE_ERROR_OPEN,
	LOCALE_STREAMINGMENU_352X288,
	LOCALE_STREAMINGMENU_352X576,
	LOCALE_STREAMINGMENU_480X576,
	LOCALE_STREAMINGMENU_704X576,
	LOCALE_STREAMINGMENU_HEAD,
	LOCALE_STREAMINGMENU_MPEG1,
	LOCALE_STREAMINGMENU_MPEG2,
	LOCALE_STREAMINGMENU_OFF,
	LOCALE_STREAMINGMENU_ON,
	LOCALE_STREAMINGMENU_SERVER_IP,
	LOCALE_STREAMINGMENU_SERVER_PORT,
	LOCALE_STREAMINGMENU_STREAMING_AUDIORATE,
	LOCALE_STREAMINGMENU_STREAMING_FORCE_AVI_RAWAUDIO,
	LOCALE_STREAMINGMENU_STREAMING_FORCE_TRANSCODE_VIDEO,
	LOCALE_STREAMINGMENU_STREAMING_RESOLUTION,
	LOCALE_STREAMINGMENU_STREAMING_SERVER_CDDRIVE,
	LOCALE_STREAMINGMENU_STREAMING_SERVER_STARTDIR,
	LOCALE_STREAMINGMENU_STREAMING_TRANSCODE_AUDIO,
	LOCALE_STREAMINGMENU_STREAMING_TRANSCODE_VIDEO_CODEC,
	LOCALE_STREAMINGMENU_STREAMING_TYPE,
	LOCALE_STREAMINGMENU_STREAMING_VIDEORATE,
	LOCALE_STREAMINGSERVER_NOCONNECT,
	LOCALE_STRINGINPUT_CAPS,
	LOCALE_STRINGINPUT_CLEAR,
	LOCALE_TIMER_EVENTRECORD_MSG,
	LOCALE_TIMER_EVENTRECORD_TITLE,
	LOCALE_TIMER_EVENTTIMED_MSG,
	LOCALE_TIMER_EVENTTIMED_TITLE,
	LOCALE_TIMERBAR_CHANNELSWITCH,
	LOCALE_TIMERBAR_RECORDEVENT,
	LOCALE_TIMERLIST_ALARMTIME,
	LOCALE_TIMERLIST_APIDS,
	LOCALE_TIMERLIST_BOUQUETSELECT,
	LOCALE_TIMERLIST_CHANNEL,
	LOCALE_TIMERLIST_CHANNELSELECT,
	LOCALE_TIMERLIST_DELETE,
	LOCALE_TIMERLIST_MENUMODIFY,
	LOCALE_TIMERLIST_MENUNEW,
	LOCALE_TIMERLIST_MESSAGE,
	LOCALE_TIMERLIST_MODERADIO,
	LOCALE_TIMERLIST_MODESELECT,
	LOCALE_TIMERLIST_MODETV,
	LOCALE_TIMERLIST_MODIFY,
	LOCALE_TIMERLIST_NAME,
	LOCALE_TIMERLIST_NEW,
	LOCALE_TIMERLIST_OVERLAPPING_TIMER,
	LOCALE_TIMERLIST_PLUGIN,
	LOCALE_TIMERLIST_PROGRAM_UNKNOWN,
	LOCALE_TIMERLIST_RECORDING_DIR,
	LOCALE_TIMERLIST_RELOAD,
	LOCALE_TIMERLIST_REPEAT,
	LOCALE_TIMERLIST_REPEAT_BIWEEKLY,
	LOCALE_TIMERLIST_REPEAT_BYEVENTDESCRIPTION,
	LOCALE_TIMERLIST_REPEAT_DAILY,
	LOCALE_TIMERLIST_REPEAT_FOURWEEKLY,
	LOCALE_TIMERLIST_REPEAT_FRIDAY,
	LOCALE_TIMERLIST_REPEAT_MONDAY,
	LOCALE_TIMERLIST_REPEAT_MONTHLY,
	LOCALE_TIMERLIST_REPEAT_ONCE,
	LOCALE_TIMERLIST_REPEAT_SATURDAY,
	LOCALE_TIMERLIST_REPEAT_SUNDAY,
	LOCALE_TIMERLIST_REPEAT_THURSDAY,
	LOCALE_TIMERLIST_REPEAT_TUESDAY,
	LOCALE_TIMERLIST_REPEAT_UNKNOWN,
	LOCALE_TIMERLIST_REPEAT_WEDNESDAY,
	LOCALE_TIMERLIST_REPEAT_WEEKDAYS,
	LOCALE_TIMERLIST_REPEAT_WEEKLY,
	LOCALE_TIMERLIST_REPEATCOUNT,
	LOCALE_TIMERLIST_REPEATCOUNT_HELP1,
	LOCALE_TIMERLIST_REPEATCOUNT_HELP2,
	LOCALE_TIMERLIST_SAVE,
	LOCALE_TIMERLIST_STANDBY,
	LOCALE_TIMERLIST_STANDBY_OFF,
	LOCALE_TIMERLIST_STANDBY_ON,
	LOCALE_TIMERLIST_STOPTIME,
	LOCALE_TIMERLIST_TYPE,
	LOCALE_TIMERLIST_TYPE_EXECPLUGIN,
	LOCALE_TIMERLIST_TYPE_NEXTPROGRAM,
	LOCALE_TIMERLIST_TYPE_RECORD,
	LOCALE_TIMERLIST_TYPE_REMIND,
	LOCALE_TIMERLIST_TYPE_SHUTDOWN,
	LOCALE_TIMERLIST_TYPE_SLEEPTIMER,
	LOCALE_TIMERLIST_TYPE_STANDBY,
	LOCALE_TIMERLIST_TYPE_UNKNOWN,
	LOCALE_TIMERLIST_TYPE_ZAPTO,
	LOCALE_TIMERLIST_WEEKDAYS,
	LOCALE_TIMERLIST_WEEKDAYS_HINT_1,
	LOCALE_TIMERLIST_WEEKDAYS_HINT_2,
	LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_AFTER,
	LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_AFTER_HINT_1,
	LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_AFTER_HINT_2,
	LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_BEFORE,
	LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_BEFORE_HINT_1,
	LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_BEFORE_HINT_2,
	LOCALE_TIMERSETTINGS_SEPARATOR,
	LOCALE_TIMING_CHANLIST,
	LOCALE_TIMING_EPG,
	LOCALE_TIMING_FILEBROWSER,
	LOCALE_TIMING_HEAD,
	LOCALE_TIMING_HINT_1,
	LOCALE_TIMING_HINT_2,
	LOCALE_TIMING_INFOBAR,
	LOCALE_TIMING_MENU,
	LOCALE_TIMING_NUMERICZAP,
	LOCALE_UCODECHECK_AVIA500,
	LOCALE_UCODECHECK_AVIA600,
	LOCALE_UCODECHECK_CAM_ALPHA,
	LOCALE_UCODECHECK_HEAD,
	LOCALE_UCODECHECK_UCODE,
	LOCALE_UCODES_FAILURE,
	LOCALE_VIDEOMENU_CSYNC,
	LOCALE_VIDEOMENU_HEAD,
	LOCALE_VIDEOMENU_RGB_CENTERING,
	LOCALE_VIDEOMENU_SCREENSETUP,
	LOCALE_VIDEOMENU_VCRSWITCH,
	LOCALE_VIDEOMENU_VIDEOFORMAT,
	LOCALE_VIDEOMENU_VIDEOFORMAT_169,
	LOCALE_VIDEOMENU_VIDEOFORMAT_43,
	LOCALE_VIDEOMENU_VIDEOFORMAT_431,
	LOCALE_VIDEOMENU_VIDEOFORMAT_AUTODETECT,
	LOCALE_VIDEOMENU_VIDEOSIGNAL,
	LOCALE_VIDEOMENU_VIDEOSIGNAL_COMPOSITE,
	LOCALE_VIDEOMENU_VIDEOSIGNAL_RGB,
	LOCALE_VIDEOMENU_VIDEOSIGNAL_SVIDEO,
	LOCALE_VIDEOMENU_VIDEOSIGNAL_YUV_C,
	LOCALE_VIDEOMENU_VIDEOSIGNAL_YUV_V,
	LOCALE_ZAPIT_SCANTYPE,
	LOCALE_ZAPIT_SCANTYPE_ALL,
	LOCALE_ZAPIT_SCANTYPE_RADIO,
	LOCALE_ZAPIT_SCANTYPE_TV,
	LOCALE_ZAPIT_SCANTYPE_TVRADIO,
	LOCALE_ZAPTOTIMER_ANNOUNCE,
} neutrino_locale_t;
#endif

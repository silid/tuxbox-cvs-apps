#ifndef __locals_intern__
#define __locals_intern__

/*
 * $Id: locals_intern.h,v 1.7 2004/07/02 09:45:55 thegoodguy Exp $
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

const char * locale_real_names[] =
{
	"INTERNAL ERROR - PLEASE REPORT",
	"EPGMenu.epgplus",
	"EPGMenu.eventinfo",
	"EPGMenu.eventlist",
	"EPGMenu.head",
	"EPGMenu.streaminfo",
	"EPGPlus.actions",
	"EPGPlus.bybouquet_mode",
	"EPGPlus.bypage_mode",
	"EPGPlus.change_font_size",
	"EPGPlus.change_font_style",
	"EPGPlus.change_size",
	"EPGPlus.channelentry_font",
	"EPGPlus.channelentry_separationlineheight",
	"EPGPlus.channelentry_width",
	"EPGPlus.channelevententry_font",
	"EPGPlus.edit_fonts",
	"EPGPlus.edit_sizes",
	"EPGPlus.font_style_bold",
	"EPGPlus.font_style_italic",
	"EPGPlus.font_style_regular",
	"EPGPlus.footer_fontbouquetchannelname",
	"EPGPlus.footer_fontbuttons",
	"EPGPlus.footer_fonteventdescription",
	"EPGPlus.footer_fonteventshortdescription",
	"EPGPlus.head",
	"EPGPlus.header_font",
	"EPGPlus.horgap1_height",
	"EPGPlus.horgap2_height",
	"EPGPlus.next_bouquet",
	"EPGPlus.options",
	"EPGPlus.page_down",
	"EPGPlus.page_up",
	"EPGPlus.prev_bouquet",
	"EPGPlus.record",
	"EPGPlus.refresh_epg",
	"EPGPlus.remind",
	"EPGPlus.reset_settings",
	"EPGPlus.save_settings",
	"EPGPlus.scroll_mode",
	"EPGPlus.select_font_name",
	"EPGPlus.settings",
	"EPGPlus.slider_width",
	"EPGPlus.stretch_mode",
	"EPGPlus.swap_mode",
	"EPGPlus.timeline_fontdate",
	"EPGPlus.timeline_fonttime",
	"EPGPlus.vergap1_width",
	"EPGPlus.vergap2_width",
	"EPGPlus.view_mode",
	"GENRE.ARTS.0",
	"GENRE.ARTS.1",
	"GENRE.ARTS.10",
	"GENRE.ARTS.11",
	"GENRE.ARTS.2",
	"GENRE.ARTS.3",
	"GENRE.ARTS.4",
	"GENRE.ARTS.5",
	"GENRE.ARTS.6",
	"GENRE.ARTS.7",
	"GENRE.ARTS.8",
	"GENRE.ARTS.9",
	"GENRE.CHILDRENs_PROGRAMMES.0",
	"GENRE.CHILDRENs_PROGRAMMES.1",
	"GENRE.CHILDRENs_PROGRAMMES.2",
	"GENRE.CHILDRENs_PROGRAMMES.3",
	"GENRE.CHILDRENs_PROGRAMMES.4",
	"GENRE.CHILDRENs_PROGRAMMES.5",
	"GENRE.DOCUS_MAGAZINES.0",
	"GENRE.DOCUS_MAGAZINES.1",
	"GENRE.DOCUS_MAGAZINES.2",
	"GENRE.DOCUS_MAGAZINES.3",
	"GENRE.DOCUS_MAGAZINES.4",
	"GENRE.DOCUS_MAGAZINES.5",
	"GENRE.DOCUS_MAGAZINES.6",
	"GENRE.DOCUS_MAGAZINES.7",
	"GENRE.MOVIE.0",
	"GENRE.MOVIE.1",
	"GENRE.MOVIE.2",
	"GENRE.MOVIE.3",
	"GENRE.MOVIE.4",
	"GENRE.MOVIE.5",
	"GENRE.MOVIE.6",
	"GENRE.MOVIE.7",
	"GENRE.MOVIE.8",
	"GENRE.MUSIC_DANCE.0",
	"GENRE.MUSIC_DANCE.1",
	"GENRE.MUSIC_DANCE.2",
	"GENRE.MUSIC_DANCE.3",
	"GENRE.MUSIC_DANCE.4",
	"GENRE.MUSIC_DANCE.5",
	"GENRE.MUSIC_DANCE.6",
	"GENRE.NEWS.0",
	"GENRE.NEWS.1",
	"GENRE.NEWS.2",
	"GENRE.NEWS.3",
	"GENRE.NEWS.4",
	"GENRE.SHOW.0",
	"GENRE.SHOW.1",
	"GENRE.SHOW.2",
	"GENRE.SHOW.3",
	"GENRE.SOZIAL_POLITICAL.0",
	"GENRE.SOZIAL_POLITICAL.1",
	"GENRE.SOZIAL_POLITICAL.2",
	"GENRE.SOZIAL_POLITICAL.3",
	"GENRE.SPORTS.0",
	"GENRE.SPORTS.1",
	"GENRE.SPORTS.10",
	"GENRE.SPORTS.11",
	"GENRE.SPORTS.2",
	"GENRE.SPORTS.3",
	"GENRE.SPORTS.4",
	"GENRE.SPORTS.5",
	"GENRE.SPORTS.6",
	"GENRE.SPORTS.7",
	"GENRE.SPORTS.8",
	"GENRE.SPORTS.9",
	"GENRE.TRAVEL_HOBBIES.0",
	"GENRE.TRAVEL_HOBBIES.1",
	"GENRE.TRAVEL_HOBBIES.2",
	"GENRE.TRAVEL_HOBBIES.3",
	"GENRE.TRAVEL_HOBBIES.4",
	"GENRE.TRAVEL_HOBBIES.5",
	"GENRE.TRAVEL_HOBBIES.6",
	"GENRE.TRAVEL_HOBBIES.7",
	"GENRE.UNKNOWN",
	"apids.hint_1",
	"apids.hint_2",
	"apidselector.head",
	"audiomenu.PCMOffset",
	"audiomenu.analogout",
	"audiomenu.avs",
	"audiomenu.avs_control",
	"audiomenu.dolbydigital",
	"audiomenu.head",
	"audiomenu.lirc",
	"audiomenu.monoleft",
	"audiomenu.monoright",
	"audiomenu.ost",
	"audiomenu.stereo",
	"audioplayer.head",
	"audioplayer.name",
	"audioplayerpicsettings.general",
	"bookmarkmanager.delete",
	"bookmarkmanager.name",
	"bookmarkmanager.rename",
	"bookmarkmanager.select",
	"bouqueteditor.add",
	"bouqueteditor.bouquetname",
	"bouqueteditor.delete",
	"bouqueteditor.discardingchanges",
	"bouqueteditor.hide",
	"bouqueteditor.lock",
	"bouqueteditor.move",
	"bouqueteditor.name",
	"bouqueteditor.newbouquetname",
	"bouqueteditor.rename",
	"bouqueteditor.return",
	"bouqueteditor.savechanges?",
	"bouqueteditor.savingchanges",
	"bouqueteditor.switch",
	"bouqueteditor.switchmode",
	"bouquetlist.head",
	"cablesetup.provider",
	"channellist.head",
	"channellist.nonefound",
	"channellist.since",
	"colorchooser.alpha",
	"colorchooser.blue",
	"colorchooser.green",
	"colorchooser.red",
	"colormenu.background",
	"colormenu.background_head",
	"colormenu.fade",
	"colormenu.font",
	"colormenu.gtx_alpha",
	"colormenu.head",
	"colormenu.menucolors",
	"colormenu.statusbar",
	"colormenu.textcolor",
	"colormenu.textcolor_head",
	"colormenu.themeselect",
	"colormenu.timing",
	"colormenusetup.head",
	"colormenusetup.menucontent",
	"colormenusetup.menucontent_inactive",
	"colormenusetup.menucontent_selected",
	"colormenusetup.menuhead",
	"colorstatusbar.head",
	"colorstatusbar.text",
	"colorthememenu.classic_theme",
	"colorthememenu.dblue_theme",
	"colorthememenu.dvb2k_theme",
	"colorthememenu.head",
	"colorthememenu.neutrino_theme",
	"date.Apr",
	"date.Aug",
	"date.Dec",
	"date.Feb",
	"date.Fri",
	"date.Jan",
	"date.Jul",
	"date.Jun",
	"date.Mar",
	"date.May",
	"date.Mon",
	"date.Nov",
	"date.Oct",
	"date.Sat",
	"date.Sep",
	"date.Sun",
	"date.Thu",
	"date.Tue",
	"date.Wed",
	"epglist.head",
	"epglist.noevents",
	"epgviewer.More_Screenings",
	"epgviewer.nodetailed",
	"epgviewer.notfound",
	"eventlistbar.channelswitch",
	"eventlistbar.eventsort",
	"eventlistbar.recordevent",
	"favorites.addchannel",
	"favorites.bouquetname",
	"favorites.bqcreated",
	"favorites.chadded",
	"favorites.chalreadyinbq",
	"favorites.finalhint",
	"favorites.menueadd",
	"favorites.nobouquets",
	"filebrowser.delete",
	"filebrowser.dodelete1",
	"filebrowser.dodelete2",
	"filebrowser.filter.active",
	"filebrowser.filter.inactive",
	"filebrowser.head",
	"filebrowser.mark",
	"filebrowser.nextpage",
	"filebrowser.prevpage",
	"filebrowser.scan",
	"filebrowser.select",
	"filebrowser.showrights",
	"filebrowser.sort.date",
	"filebrowser.sort.name",
	"filebrowser.sort.namedirsfirst",
	"filebrowser.sort.size",
	"filebrowser.sort.type",
	"flashupdate.actionreadflash",
	"flashupdate.cantopenfile",
	"flashupdate.cantopenmtd",
	"flashupdate.checkupdate",
	"flashupdate.currentreleasecycle",
	"flashupdate.currentversion_sep",
	"flashupdate.currentversiondate",
	"flashupdate.currentversionsnapshot",
	"flashupdate.currentversiontime",
	"flashupdate.erasefailed",
	"flashupdate.eraseingflash",
	"flashupdate.experimentalimage",
	"flashupdate.expertfunctions",
	"flashupdate.fileis0bytes",
	"flashupdate.fileselector",
	"flashupdate.flashreadyreboot",
	"flashupdate.getinfofile",
	"flashupdate.getinfofileerror",
	"flashupdate.getupdatefile",
	"flashupdate.getupdatefileerror",
	"flashupdate.globalprogress",
	"flashupdate.head",
	"flashupdate.md5check",
	"flashupdate.md5sumerror",
	"flashupdate.msgbox",
	"flashupdate.msgbox_manual",
	"flashupdate.mtdselector",
	"flashupdate.programmingflash",
	"flashupdate.proxypassword",
	"flashupdate.proxypassword_hint1",
	"flashupdate.proxypassword_hint2",
	"flashupdate.proxyserver",
	"flashupdate.proxyserver_hint1",
	"flashupdate.proxyserver_hint2",
	"flashupdate.proxyserver_sep",
	"flashupdate.proxyusername",
	"flashupdate.proxyusername_hint1",
	"flashupdate.proxyusername_hint2",
	"flashupdate.readflash",
	"flashupdate.readflashmtd",
	"flashupdate.ready",
	"flashupdate.reallyflashmtd",
	"flashupdate.savesuccess",
	"flashupdate.selectimage",
	"flashupdate.squashfs.noversion",
	"flashupdate.titlereadflash",
	"flashupdate.titlewriteflash",
	"flashupdate.updatemode",
	"flashupdate.updatemode_internet",
	"flashupdate.updatemode_manual",
	"flashupdate.url_file",
	"flashupdate.versioncheck",
	"flashupdate.writeflash",
	"flashupdate.writeflashmtd",
	"flashupdate.wrongbase",
	"fontmenu.channellist",
	"fontmenu.epg",
	"fontmenu.eventlist",
	"fontmenu.gamelist",
	"fontmenu.head",
	"fontmenu.infobar",
	"fontsize.channel_num_zap",
	"fontsize.channellist",
	"fontsize.channellist_descr",
	"fontsize.channellist_number",
	"fontsize.epg_date",
	"fontsize.epg_info1",
	"fontsize.epg_info2",
	"fontsize.epg_title",
	"fontsize.eventlist_datetime",
	"fontsize.eventlist_itemlarge",
	"fontsize.eventlist_itemsmall",
	"fontsize.eventlist_title",
	"fontsize.filebrowser_item",
	"fontsize.gamelist_itemlarge",
	"fontsize.gamelist_itemsmall",
	"fontsize.hint",
	"fontsize.infobar_channame",
	"fontsize.infobar_info",
	"fontsize.infobar_number",
	"fontsize.infobar_small",
	"fontsize.menu",
	"fontsize.menu_info",
	"fontsize.menu_title",
	"gtxalpha.alpha1",
	"gtxalpha.alpha2",
	"infoviewer.epgnotload",
	"infoviewer.epgwait",
	"infoviewer.eventlist",
	"infoviewer.languages",
	"infoviewer.motor_moving",
	"infoviewer.nocurrent",
	"infoviewer.noepg",
	"infoviewer.notavailable",
	"infoviewer.selecttime",
	"infoviewer.streaminfo",
	"infoviewer.subservice",
	"infoviewer.waittime",
	"ipsetup.hint_1",
	"ipsetup.hint_2",
	"keybindingmenu.RC",
	"keybindingmenu.addrecord",
	"keybindingmenu.addrecord_head",
	"keybindingmenu.addremind",
	"keybindingmenu.addremind_head",
	"keybindingmenu.allchannels_on_ok",
	"keybindingmenu.bouquetchannels_on_ok",
	"keybindingmenu.bouquetdown",
	"keybindingmenu.bouquetdown_head",
	"keybindingmenu.bouquethandling",
	"keybindingmenu.bouquetlist_on_ok",
	"keybindingmenu.bouquetup",
	"keybindingmenu.bouquetup_head",
	"keybindingmenu.cancel",
	"keybindingmenu.cancel_head",
	"keybindingmenu.channeldown",
	"keybindingmenu.channeldown_head",
	"keybindingmenu.channellist",
	"keybindingmenu.channelup",
	"keybindingmenu.channelup_head",
	"keybindingmenu.head",
	"keybindingmenu.modechange",
	"keybindingmenu.pagedown",
	"keybindingmenu.pagedown_head",
	"keybindingmenu.pageup",
	"keybindingmenu.pageup_head",
	"keybindingmenu.quickzap",
	"keybindingmenu.repeatblock",
	"keybindingmenu.repeatblockgeneric",
	"keybindingmenu.sort",
	"keybindingmenu.sort_head",
	"keybindingmenu.subchanneldown",
	"keybindingmenu.subchanneldown_head",
	"keybindingmenu.subchannelup",
	"keybindingmenu.subchannelup_head",
	"keybindingmenu.tvradiomode",
	"keybindingmenu.tvradiomode_head",
	"keychooser.head",
	"keychooser.text1",
	"keychooser.text2",
	"keychoosermenu.currentkey",
	"keychoosermenu.setnew",
	"keychoosermenu.setnone",
	"languagesetup.head",
	"languagesetup.select",
	"lcdcontroler.brightness",
	"lcdcontroler.brightnessstandby",
	"lcdcontroler.contrast",
	"lcdcontroler.head",
	"lcdmenu.autodimm",
	"lcdmenu.head",
	"lcdmenu.inverse",
	"lcdmenu.lcdcontroler",
	"lcdmenu.power",
	"lcdmenu.statusline",
	"lcdmenu.statusline.both",
	"lcdmenu.statusline.playtime",
	"lcdmenu.statusline.volume",
	"mainmenu.audioplayer",
	"mainmenu.games",
	"mainmenu.head",
	"mainmenu.movieplayer",
	"mainmenu.pausesectionsd",
	"mainmenu.pictureviewer",
	"mainmenu.radiomode",
	"mainmenu.recording",
	"mainmenu.recording_start",
	"mainmenu.recording_stop",
	"mainmenu.scartmode",
	"mainmenu.service",
	"mainmenu.settings",
	"mainmenu.shutdown",
	"mainmenu.sleeptimer",
	"mainmenu.tvmode",
	"mainsettings.audio",
	"mainsettings.colors",
	"mainsettings.head",
	"mainsettings.keybinding",
	"mainsettings.language",
	"mainsettings.lcd",
	"mainsettings.misc",
	"mainsettings.network",
	"mainsettings.recording",
	"mainsettings.savesettingsnow",
	"mainsettings.savesettingsnow_hint",
	"mainsettings.streaming",
	"mainsettings.video",
	"menu.back",
	"messagebox.back",
	"messagebox.cancel",
	"messagebox.discard",
	"messagebox.error",
	"messagebox.info",
	"messagebox.no",
	"messagebox.yes",
	"miscsettings.bootinfo",
	"miscsettings.bootmenu",
	"miscsettings.driver_boot",
	"miscsettings.fb_destination",
	"miscsettings.general",
	"miscsettings.head",
	"miscsettings.hwsections",
	"miscsettings.infobar_sat_display",
	"miscsettings.shutdown_real",
	"miscsettings.shutdown_real_rcdelay",
	"miscsettings.sptsmode",
	"miscsettings.startbhdriver",
	"motorcontrol.head",
	"movieplayer.bookmark",
	"movieplayer.bookmarkname",
	"movieplayer.bookmarkname_hint1",
	"movieplayer.bookmarkname_hint2",
	"movieplayer.buffering",
	"movieplayer.defdir",
	"movieplayer.dvdplayback",
	"movieplayer.fileplayback",
	"movieplayer.goto",
	"movieplayer.goto.h1",
	"movieplayer.goto.h2",
	"movieplayer.head",
	"movieplayer.nostreamingserver",
	"movieplayer.pesplayback",
	"movieplayer.pleasewait",
	"movieplayer.toomanybookmarks",
	"movieplayer.tshelp",
	"movieplayer.tsplayback",
	"movieplayer.vcdplayback",
	"movieplayer.vlchelp",
	"movieplayer.wrongvlcversion",
	"mp3player.add",
	"mp3player.artist_title",
	"mp3player.defdir",
	"mp3player.delete",
	"mp3player.deleteall",
	"mp3player.display_order",
	"mp3player.fastforward",
	"mp3player.follow",
	"mp3player.highprio",
	"mp3player.id3scan",
	"mp3player.keylevel",
	"mp3player.pause",
	"mp3player.play",
	"mp3player.playing",
	"mp3player.rewind",
	"mp3player.screensaver_timeout",
	"mp3player.shuffle",
	"mp3player.stop",
	"mp3player.title_artist",
	"networkmenu.broadcast",
	"networkmenu.dhcp",
	"networkmenu.gateway",
	"networkmenu.head",
	"networkmenu.ipaddress",
	"networkmenu.mount",
	"networkmenu.nameserver",
	"networkmenu.netmask",
	"networkmenu.setupnow",
	"networkmenu.setuponstartup",
	"networkmenu.show",
	"networkmenu.test",
	"nfs.alreadymounted",
	"nfs.automount",
	"nfs.dir",
	"nfs.ip",
	"nfs.localdir",
	"nfs.mount",
	"nfs.mount_options",
	"nfs.mounterror",
	"nfs.mounterror_notsup",
	"nfs.mountnow",
	"nfs.mounttimeout",
	"nfs.password",
	"nfs.remount",
	"nfs.type",
	"nfs.type_cifs",
	"nfs.type_nfs",
	"nfs.umount",
	"nfs.umounterror",
	"nfs.username",
	"nfsmenu.head",
	"nvod.proz",
	"nvod.starting",
	"nvodselector.directormode",
	"nvodselector.head",
	"nvodselector.subservice",
	"options.default",
	"options.fb",
	"options.null",
	"options.off",
	"options.on",
	"options.serial",
	"parentallock.changepin",
	"parentallock.changepin_hint1",
	"parentallock.changetolocked",
	"parentallock.head",
	"parentallock.lockage",
	"parentallock.lockage12",
	"parentallock.lockage16",
	"parentallock.lockage18",
	"parentallock.lockedprogram",
	"parentallock.lockedsender",
	"parentallock.never",
	"parentallock.onsignal",
	"parentallock.parentallock",
	"parentallock.prompt",
	"pictureviewer.defdir",
	"pictureviewer.head",
	"pictureviewer.resize.color_average",
	"pictureviewer.resize.none",
	"pictureviewer.resize.simple",
	"pictureviewer.scaling",
	"pictureviewer.show",
	"pictureviewer.slide_time",
	"pictureviewer.slideshow",
	"pictureviewer.sortorder",
	"pictureviewer.sortorder.date",
	"pictureviewer.sortorder.filename",
	"ping.ok",
	"ping.protocol",
	"ping.socket",
	"ping.unreachable",
	"pinprotection.head",
	"pinprotection.wrongcode",
	"rclock.lockmsg",
	"rclock.menueadd",
	"rclock.title",
	"rclock.unlockmsg",
	"recordingmenu.defdir",
	"recordingmenu.file",
	"recordingmenu.filesettingsseparator",
	"recordingmenu.head",
	"recordingmenu.no_scart",
	"recordingmenu.off",
	"recordingmenu.recording_type",
	"recordingmenu.server",
	"recordingmenu.server_ip",
	"recordingmenu.server_mac",
	"recordingmenu.server_port",
	"recordingmenu.server_wakeup",
	"recordingmenu.setupnow",
	"recordingmenu.splitsize",
	"recordingmenu.stopplayback",
	"recordingmenu.stopsectionsd",
	"recordingmenu.stream_all_audio_pids",
	"recordingmenu.use_o_sync",
	"recordingmenu.vcr",
	"recordtimer.announce",
	"repeatblocker.hint_1",
	"repeatblocker.hint_2",
	"satsetup.diseqc",
	"satsetup.diseqc10",
	"satsetup.diseqc11",
	"satsetup.diseqc12",
	"satsetup.diseqcrepeat",
	"satsetup.extended",
	"satsetup.extended_motor",
	"satsetup.minidiseqc",
	"satsetup.motorcontrol",
	"satsetup.nodiseqc",
	"satsetup.satellite",
	"satsetup.savesettingsnow",
	"satsetup.smatvremote",
	"scants.actcable",
	"scants.actsatellite",
	"scants.bouquet",
	"scants.bouquet_create",
	"scants.bouquet_erase",
	"scants.bouquet_leave",
	"scants.bouquet_satellite",
	"scants.bouquet_update",
	"scants.channel",
	"scants.failed",
	"scants.finished",
	"scants.freqdata",
	"scants.head",
	"scants.numberofdataservices",
	"scants.numberofradioservices",
	"scants.numberoftotalservices",
	"scants.numberoftvservices",
	"scants.provider",
	"scants.startnow",
	"scants.transponders",
	"screensetup.lowerright",
	"screensetup.upperleft",
	"servicemenu.head",
	"servicemenu.reload",
	"servicemenu.reload_hint",
	"servicemenu.scants",
	"servicemenu.ucodecheck",
	"servicemenu.update",
	"settings.missingoptionsconffile",
	"settings.noconffile",
	"shutdowntimer.announce",
	"sleeptimerbox.announce",
	"sleeptimerbox.hint1",
	"sleeptimerbox.hint2",
	"sleeptimerbox.title",
	"streamfeatures.head",
	"streaminfo.aratio",
	"streaminfo.aratio_unknown",
	"streaminfo.audiotype",
	"streaminfo.audiotype_unknown",
	"streaminfo.bitrate",
	"streaminfo.framerate",
	"streaminfo.framerate_unknown",
	"streaminfo.head",
	"streaminfo.not_available",
	"streaminfo.resolution",
	"streaminfo.signal",
	"streaming.buffer_overflow",
	"streaming.busy",
	"streaming.dir_not_writable",
	"streaming.success",
	"streaming.write_error",
	"streaming.write_error_open",
	"streamingmenu.352x288",
	"streamingmenu.352x576",
	"streamingmenu.480x576",
	"streamingmenu.704x576",
	"streamingmenu.head",
	"streamingmenu.mpeg1",
	"streamingmenu.mpeg2",
	"streamingmenu.off",
	"streamingmenu.on",
	"streamingmenu.server_ip",
	"streamingmenu.server_port",
	"streamingmenu.streaming_audiorate",
	"streamingmenu.streaming_force_avi_rawaudio",
	"streamingmenu.streaming_force_transcode_video",
	"streamingmenu.streaming_resolution",
	"streamingmenu.streaming_server_cddrive",
	"streamingmenu.streaming_server_startdir",
	"streamingmenu.streaming_transcode_audio",
	"streamingmenu.streaming_transcode_video_codec",
	"streamingmenu.streaming_type",
	"streamingmenu.streaming_videorate",
	"streamingserver.noconnect",
	"stringinput.caps",
	"stringinput.clear",
	"timer.eventrecord.msg",
	"timer.eventrecord.title",
	"timer.eventtimed.msg",
	"timer.eventtimed.title",
	"timerbar.channelswitch",
	"timerbar.recordevent",
	"timerlist.alarmtime",
	"timerlist.apids",
	"timerlist.bouquetselect",
	"timerlist.channel",
	"timerlist.channelselect",
	"timerlist.delete",
	"timerlist.menumodify",
	"timerlist.menunew",
	"timerlist.message",
	"timerlist.moderadio",
	"timerlist.modeselect",
	"timerlist.modetv",
	"timerlist.modify",
	"timerlist.name",
	"timerlist.new",
	"timerlist.program.unknown",
	"timerlist.reload",
	"timerlist.repeat",
	"timerlist.repeat.biweekly",
	"timerlist.repeat.byeventdescription",
	"timerlist.repeat.daily",
	"timerlist.repeat.fourweekly",
	"timerlist.repeat.friday",
	"timerlist.repeat.monday",
	"timerlist.repeat.monthly",
	"timerlist.repeat.once",
	"timerlist.repeat.saturday",
	"timerlist.repeat.sunday",
	"timerlist.repeat.thursday",
	"timerlist.repeat.tuesday",
	"timerlist.repeat.unknown",
	"timerlist.repeat.wednesday",
	"timerlist.repeat.weekdays",
	"timerlist.repeat.weekly",
	"timerlist.save",
	"timerlist.standby",
	"timerlist.standby.off",
	"timerlist.standby.on",
	"timerlist.stoptime",
	"timerlist.type",
	"timerlist.type.nextprogram",
	"timerlist.type.record",
	"timerlist.type.remind",
	"timerlist.type.shutdown",
	"timerlist.type.sleeptimer",
	"timerlist.type.standby",
	"timerlist.type.unknown",
	"timerlist.type.zapto",
	"timerlist.weekdays",
	"timerlist.weekdays.hint_1",
	"timerlist.weekdays.hint_2",
	"timersettings.record_safety_time_after",
	"timersettings.record_safety_time_after.hint_1",
	"timersettings.record_safety_time_after.hint_2",
	"timersettings.record_safety_time_before",
	"timersettings.record_safety_time_before.hint_1",
	"timersettings.record_safety_time_before.hint_2",
	"timersettings.separator",
	"timing.chanlist",
	"timing.epg",
	"timing.filebrowser",
	"timing.head",
	"timing.hint_1",
	"timing.hint_2",
	"timing.infobar",
	"timing.menu",
	"ucodecheck.avia500",
	"ucodecheck.avia600",
	"ucodecheck.cam-alpha",
	"ucodecheck.head",
	"ucodecheck.ucode",
	"ucodes.failure",
	"videomenu.csync",
	"videomenu.head",
	"videomenu.rgb_centering",
	"videomenu.screensetup",
	"videomenu.vcrswitch",
	"videomenu.videoformat",
	"videomenu.videoformat_169",
	"videomenu.videoformat_43",
	"videomenu.videoformat_431",
	"videomenu.videoformat_autodetect",
	"videomenu.videosignal",
	"videomenu.videosignal_composite",
	"videomenu.videosignal_rgb",
	"videomenu.videosignal_svideo",
	"videomenu.videosignal_yuv_c",
	"videomenu.videosignal_yuv_v",
	"zaptotimer.announce",
};
#endif

#include "enigma_main.h"

#include <errno.h>
#include <iomanip>
//#include <stdio.h>
#include <dirent.h>

#include <apps/enigma/enigma_mainmenu.h>
#include <apps/enigma/enigma_event.h>
#include <apps/enigma/sselect.h>
#include <apps/enigma/enigma.h>
#include <apps/enigma/enigma_lcd.h>
#include <apps/enigma/enigma_plugins.h>
#include <apps/enigma/download.h>
#include <apps/enigma/epgwindow.h>
#include <apps/enigma/enigma.h>
#include <core/base/i18n.h>
#include <core/system/init.h>
#include <core/system/econfig.h>
#include <core/dvb/servicedvb.h>
#include <core/dvb/epgcache.h>
#include <core/dvb/esection.h>
#include <core/dvb/decoder.h>
#include <core/dvb/iso639.h>
#include <core/gdi/font.h>
#include <core/gui/elabel.h>
#include <core/gui/eprogress.h>
#include <core/gui/enumber.h>
#include <core/gui/eskin.h>
#include <core/gui/ebutton.h>
#include <core/gui/actions.h>
#include <core/driver/rc.h>
#include <core/driver/streamwd.h>
#include <core/driver/eavswitch.h>
#include <core/dvb/dvbservice.h>
#include <core/gdi/lcd.h>
#include <core/dvb/servicestructure.h>
#include <core/dvb/serviceplaylist.h>

struct enigmaMainActions
{
	eActionMap map;
	eAction showMainMenu, standby_press, standby_repeat, standby_release, toggleInfobar, showServiceSelector,
		showSubservices, showAudio, pluginVTXT, showEPGList, showEPG, 
		nextService, prevService, playlistNextService, playlistPrevService,
		serviceListDown, serviceListUp, volumeUp, volumeDown, toggleMute,
		stop, pause, play, record, 
		startSkipForward, repeatSkipForward, stopSkipForward, 
		startSkipReverse, repeatSkipReverse, stopSkipReverse;
	enigmaMainActions(): 
		map("enigmaMain", _("enigma Zapp")),
		showMainMenu(map, "showMainMenu", _("show main menu"), eAction::prioDialog),
		standby_press(map, "standby_press", _("go to standby (press)"), eAction::prioDialog),
		standby_repeat(map, "standby_repeat", _("go to standby (repeat)"), eAction::prioDialog),
		standby_release(map, "standby_release", _("go to standby (release)"), eAction::prioDialog),
		toggleInfobar(map, "toggleInfobar", _("toggle infobar"), eAction::prioDialog),
		showServiceSelector(map, "showServiceSelector", _("show service selector"), eAction::prioDialog),
		showSubservices(map, "showSubservices", _("show subservices/NVOD"), eAction::prioDialog),
		showAudio(map, "showAudio", _("show audio selector"), eAction::prioDialog),
		pluginVTXT(map, "pluginVTXT", _("show Videotext"), eAction::prioDialog),
		showEPGList(map, "showEPGList", _("show epg schedule list"), eAction::prioDialog),
		showEPG(map, "showEPG", _("show extended info"), eAction::prioDialog),
		nextService(map, "nextService", _("quickzap next"), eAction::prioDialog),
		prevService(map, "prevService", _("quickzap prev"), eAction::prioDialog),

		playlistNextService(map, "playlistNextService", _("history next"), eAction::prioDialog),
		playlistPrevService(map, "playlistPrevService", _("history prev"), eAction::prioDialog),
		
		serviceListDown(map, "serviceListDown", _("service list and down"), eAction::prioDialog),
		serviceListUp(map, "serviceListUp", _("service list and up"), eAction::prioDialog),
		volumeUp(map, "volumeUp", _("volume up"), eAction::prioDialog),
		volumeDown(map, "volumeDown", _("volume down"), eAction::prioDialog),
		toggleMute(map, "toggleMute", _("toggle mute flag"), eAction::prioDialog),
		
		stop(map, "stop", _("stop playback"), eAction::prioWidget),
		pause(map, "pause", _("pause playback"), eAction::prioWidget),
		play(map, "play", _("resume playback"), eAction::prioWidget),
		record(map, "record", _("record"), eAction::prioWidget),
		
		startSkipForward(map, "startSkipF", _("start skipping forward"), eAction::prioWidget),
		repeatSkipForward(map, "repeatSkipF", _("repeat skipping forward"), eAction::prioWidget),
		stopSkipForward(map, "stopSkipF", _("stop skipping forward"), eAction::prioWidget),

		startSkipReverse(map, "startSkipR", _("start skipping reverse"), eAction::prioWidget),
		repeatSkipReverse(map, "repeatSkipR", _("repeat skipping reverse"), eAction::prioWidget),
		stopSkipReverse(map, "stopSkipR", _("stop skipping reverse"), eAction::prioWidget)
	{
	}
};
eAutoInitP0<enigmaMainActions> i_enigmaMainActions(5, "enigma main actions");

struct enigmaStandbyActions
{
	eActionMap map;
	eAction wakeUp;
	enigmaStandbyActions(): 
		map("enigmaStandby", _("enigma standby")),
		wakeUp(map, "wakeUp", _("wake up enigma"), eAction::prioDialog)
	{
	}
};
eAutoInitP0<enigmaStandbyActions> i_enigmaStandbyActions(5, "enigma standby actions");

class eZapStandby: public eWidget
{
protected:
	int eventHandler(const eWidgetEvent &);
public:
	eZapStandby();
};

int eZapStandby::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
	case eWidgetEvent::evtAction:
		if (event.action == &i_enigmaStandbyActions->wakeUp)
			close(0);
		else
			break;
		return 0;
	case eWidgetEvent::execBegin:
	{
		eDBoxLCD::getInstance()->switchLCD(0);
		eZapLCD *pLCD=eZapLCD::getInstance();
		pLCD->lcdMain->hide();
		pLCD->lcdStandby->show();

		eAVSwitch::getInstance()->setInput(1);

		break;
	}
	case eWidgetEvent::execDone:
	{
		eZapLCD *pLCD=eZapLCD::getInstance();
		pLCD->lcdStandby->hide();
		pLCD->lcdMain->show();
		eAVSwitch::getInstance()->setInput(0);
		eDBoxLCD::getInstance()->switchLCD(1);
		break;
	}
	default:
		break;
	}
	return eWidget::eventHandler(event);
}

eZapStandby::eZapStandby(): eWidget(0, 1)
{
	addActionMap(&i_enigmaStandbyActions->map);
}

eString getISO639Description(char *iso)
{
	for (unsigned int i=0; i<sizeof(iso639)/sizeof(*iso639); ++i)
	{
		if (!strnicmp(iso639[i].iso639foreign, iso, 3))
			return iso639[i].description1;
		if (!strnicmp(iso639[i].iso639int, iso, 3))
			return iso639[i].description1;
	}
	return eString()+iso[0]+iso[1]+iso[2];
}

void NVODStream::EITready(int error)
{
	eDebug("NVOD eit ready: %d", error);
	
	if (eit.ready && !eit.error)
	{
		for (ePtrList<EITEvent>::const_iterator event(eit.events); event != eit.events.end(); ++event)		// always take the first one
		{
			tm *begin=event->start_time!=-1?localtime(&event->start_time):0;

			if (begin)
				text << std::setfill('0') << std::setw(2) << begin->tm_hour << ':' << std::setw(2) << begin->tm_min;

			time_t endtime=event->start_time+event->duration;
			tm *end=event->start_time!=-1?localtime(&endtime):0;

			if (end)
				text << _(" to ") << std::setw(2) << end->tm_hour << ':' << std::setw(2) << end->tm_min;

			time_t now=time(0)+eDVB::getInstance()->time_difference;
			if ((event->start_time <= now) && (now < endtime))
			{
				int perc=(now-event->start_time)*100/event->duration;
				text << " (" << perc << "%, " << perc*3/100 << '.' << std::setw(2) << (perc*3)%100 << _(" Euro lost)");
			}
			break;
		}
	}
	else
		text << "Service " << std::setw(4) << std::hex << service.getServiceID().get();

	((eListBox<NVODStream>*)listbox)->sort(); // <<< without explicit cast the compiler nervs ;)

	if (listbox && listbox->isVisible())
		listbox->invalidate();

}

NVODStream::NVODStream(eListBox<NVODStream> *listbox, const NVODReferenceEntry *ref, int type)
	: eListBoxEntryTextStream((eListBox<eListBoxEntryTextStream>*)listbox), 
		service(eTransportStreamID(ref->transport_stream_id), eOriginalNetworkID(ref->original_network_id),
			eServiceID(ref->service_id), 5), eit(EIT::typeNowNext, ref->service_id, type)
{
	CONNECT(eit.tableReady, NVODStream::EITready);
	eit.start();
}

void eNVODSelector::selected(NVODStream* nv)
{
	if (nv)
	{
		nv->service.descr = _("NVOD");
		eServiceInterface::getInstance()->play(nv->service);
	}
	close(0);
}

eNVODSelector::eNVODSelector()
	:eListBoxWindow<NVODStream>(_("NVOD"), 10, 440)
{
	move(ePoint(100, 100));
	list.setActiveColor(eSkin::getActive()->queryScheme("eServiceSelector.highlight.background"), eSkin::getActive()->queryScheme("eServiceSelector.highlight.foreground"));
	CONNECT(list.selected, eNVODSelector::selected);
}

void eNVODSelector::clear()
{
	list.clearList();
}

void eNVODSelector::add(NVODReferenceEntry *ref)
{
	eServiceReference &s=eServiceInterface::getInstance()->service;
	if (s.type != eServiceReference::idDVB)
		return ;
	eServiceReferenceDVB &service=(eServiceReferenceDVB&)s;

	int type= ((service.getTransportStreamID()==eTransportStreamID(ref->transport_stream_id))
			&&	(service.getOriginalNetworkID()==eOriginalNetworkID(ref->original_network_id))) ? EIT::tsActual:EIT::tsOther;
	new NVODStream(&list, ref, type);
}

AudioStream::AudioStream(eListBox<AudioStream> *listbox, PMTEntry *stream)
	:eListBoxEntryText((eListBox<eListBoxEntryText>*)listbox), stream(stream)
{
	int isAC3=0;
	int component_tag=-1;
	for (ePtrList<Descriptor>::iterator c(stream->ES_info); c != stream->ES_info.end(); ++c)
	{
		if (c->Tag()==DESCR_AC3)
			isAC3=1;
		else if (c->Tag()==DESCR_ISO639_LANGUAGE)
			text=getISO639Description(((ISO639LanguageDescriptor*)*c)->language_code);
		else if (c->Tag()==DESCR_STREAM_ID)
			component_tag=((StreamIdentifierDescriptor*)*c)->component_tag;
		else if (c->Tag()==DESCR_LESRADIOS)
		{
			text=eString().sprintf("%d.) ", (((LesRadiosDescriptor*)*c)->id));
			text+=((LesRadiosDescriptor*)*c)->name;
		}
	}
	if (!text)
		text.sprintf("PID %04x", stream->elementary_PID);
	if (component_tag!=-1)
	{
		eServiceHandler *service=eServiceInterface::getInstance()->getService();
		if (service)
		{
			EIT *eit=service->getEIT();
			if (eit)
			{
				for (ePtrList<EITEvent>::iterator e(eit->events); e != eit->events.end(); ++e)
					if ((e->running_status>=2)||(!e->running_status))		// currently running service
						for (ePtrList<Descriptor>::iterator d(e->descriptor); d != e->descriptor.end(); ++d)
							if (d->Tag()==DESCR_COMPONENT && ((ComponentDescriptor*)*d)->component_tag == component_tag)
									text=((ComponentDescriptor*)*d)->text;
	 			eit->unlock();
			}
		}
	}
	if (isAC3)
		text+=" (AC3)";
}

void eAudioSelector::selected(AudioStream *l)
{
	eServiceHandler *service=eServiceInterface::getInstance()->getService();
	if (l && service)
	{
		service->setPID(l->stream);
//		service->setDecoder();
	}

	close(0);
}

eAudioSelector::eAudioSelector()
	:eListBoxWindow<AudioStream>(_("Audio"), 10, 330)
{
	move(ePoint(100, 100));
	CONNECT(list.selected, eAudioSelector::selected);
}

void eAudioSelector::clear()
{
	list.clearList();
}

void eAudioSelector::add(PMTEntry *pmt)
{
	new AudioStream(&list, pmt);
}

SubService::SubService(eListBox<SubService> *listbox, const LinkageDescriptor *descr)
	:eListBoxEntryText((eListBox<eListBoxEntryText>*) listbox),
		service(eTransportStreamID(descr->transport_stream_id), 
			eOriginalNetworkID(descr->original_network_id),
			eServiceID(descr->service_id), 1)
{
	text=(const char*)descr->private_data;
}

eSubServiceSelector::eSubServiceSelector()
	:eListBoxWindow<SubService>(_("multiple Services"), 10, 330)
{
	move(ePoint(100, 100));
	CONNECT(list.selected, eSubServiceSelector::selected);
}

void eSubServiceSelector::selected(SubService *ss)
{
	if (ss)
	{
		ss->service.setServiceType(5);  // faked service type
		ss->service.descr = ss->getText();
		eServiceInterface::getInstance()->play(ss->service);
	}
	close(0);
}

void eSubServiceSelector::clear()
{
	list.clearList();
}

void eSubServiceSelector::add(const LinkageDescriptor *ref)
{
	new SubService(&list, ref);
}

void eServiceNumberWidget::selected(int *res)
{
	if (!res)
	{
		close(-1);
		return;
	}
	close(number->getNumber());
}

void eServiceNumberWidget::timeout()
{
	close(number->getNumber());
}

eServiceNumberWidget::eServiceNumberWidget(int initial)
										:eWindow(0)
{
	setText("Channel");
	move(ePoint(200, 140));
	resize(eSize(280, 120));
	eLabel *label;
	label=new eLabel(this);
	label->setText("Channel:");
	label->move(ePoint(50, 15));
	label->resize(eSize(110, eSkin::getActive()->queryValue("fontsize", 20)+4));
	
	number=new eNumber(this, 1, 1, 999, 3, 0, 1, label);
	number->move(ePoint(160, 15));
	number->resize(eSize(50, eSkin::getActive()->queryValue("fontsize", 20)+4));
	number->setNumber(initial);
	CONNECT(number->selected, eServiceNumberWidget::selected);
	
	timer=new eTimer(eApp);
	timer->start(2000);
	CONNECT(timer->timeout, eServiceNumberWidget::timeout);	
}

eServiceNumberWidget::~eServiceNumberWidget()
{
	if (timer)
		delete timer;
}

eZapMain *eZapMain::instance;

void eZapMain::redrawWidget(gPainter *painter, const eRect &where)
{
}

void eZapMain::eraseBackground(gPainter *painter, const eRect &where)
{
}

eZapMain::eZapMain()
	:eWidget(0, 1), mute( eZap::getInstance()->getDesktop( eZap::desktopFB ) ),
	 pMsg(0), message_notifier(eApp), timeout(eApp), clocktimer(eApp), messagetimeout(eApp), progresstimer(eApp)
{
	if (!instance)
		instance=this;

	gPixmap *pm = eSkin::getActive()->queryImage("mute_symbol");
	int x = eSkin::getActive()->queryValue("mute.pos.x", 0),
		  y = eSkin::getActive()->queryValue("mute.pos.y", 0);

	if (pm && x && y )
	{
		mute.setPixmap(pm);
		mute.move( ePoint(x, y) );
		mute.resize( eSize( pm->x, pm->y ) );
		mute.pixmap_position = ePoint(0,0);
		mute.hide();
		mute.setBlitFlags( BF_ALPHATEST );
	}

	isVT=0;
	eSkin *skin=eSkin::getActive();
	if (skin->build(this, "ezap_main"))
		eFatal("skin load of \"ezap_main\" failed");

	eDebug("[PROFILE] eZapMain");
	lcdmain.show();
	eDebug("<-- show lcd.");

	ASSIGN(ChannelNumber, eLabel, "ch_number");
	ASSIGN(ChannelName, eLabel, "ch_name");

	ASSIGN(EINow, eLabel, "e_now_title");
	ASSIGN(EINext, eLabel, "e_next_title");
	
	ASSIGN(EINowDuration, eLabel, "e_now_duration");
	ASSIGN(EINextDuration, eLabel, "e_next_duration");

	ASSIGN(EINowTime, eLabel, "e_now_time");
	ASSIGN(EINextTime, eLabel, "e_next_time");

	ASSIGN(Description, eLabel, "description");
	ASSIGN(VolumeBar, eProgress, "volume_bar");
	ASSIGN(Progress, eProgress, "progress_bar");
	
	ASSIGN(ButtonRedEn, eLabel, "button_red_enabled");
	ASSIGN(ButtonGreenEn, eLabel, "button_green_enabled");
	ASSIGN(ButtonYellowEn, eLabel, "button_yellow_enabled");
	ASSIGN(ButtonBlueEn, eLabel, "button_blue_enabled");
	ASSIGN(ButtonRedDis, eLabel, "button_red_disabled");
	ASSIGN(ButtonGreenDis, eLabel, "button_green_disabled");
	ASSIGN(ButtonYellowDis, eLabel, "button_yellow_disabled");
	ASSIGN(ButtonBlueDis, eLabel, "button_blue_disabled");

	ASSIGN(DolbyOn, eLabel, "osd_dolby_on");
	ASSIGN(CryptOn, eLabel, "osd_crypt_on");
	ASSIGN(WideOn, eLabel, "osd_format_on");
	ASSIGN(DolbyOff, eLabel, "osd_dolby_off");
	ASSIGN(CryptOff, eLabel, "osd_crypt_off");
	ASSIGN(WideOff, eLabel, "osd_format_off");
	DolbyOn->hide();
	CryptOn->hide();
	WideOn->hide();
	DolbyOff->show();
	CryptOff->show();
	WideOff->show();

	ButtonRedEn->hide();
	ButtonRedDis->show();
	ButtonGreenEn->hide();
	ButtonGreenDis->show();
	ButtonYellowEn->hide();
	ButtonYellowDis->show();
	ButtonBlueEn->hide();
	ButtonBlueDis->show();

	Clock=new eLabel(this);
	ASSIGN(Clock, eLabel, "time");

	cur_start=cur_duration=-1;
	CONNECT(eEPGCache::getInstance()->EPGAvail, eZapMain::setEPGButton);

	CONNECT(eServiceInterface::getInstance()->serviceEvent, eZapMain::handleServiceEvent);

	CONNECT(timeout.timeout, eZapMain::timeOut);

	CONNECT(clocktimer.timeout, eZapMain::clockUpdate);
	CONNECT(messagetimeout.timeout, eZapMain::nextMessage);
	
	CONNECT(progresstimer.timeout, eZapMain::updateProgress);

	CONNECT(eDVB::getInstance()->timeUpdated, eZapMain::clockUpdate);
	CONNECT(eAVSwitch::getInstance()->volumeChanged, eZapMain::updateVolume);

	CONNECT(message_notifier.recv_msg, eZapMain::gotMessage);

	actual_eventDisplay=0;

	clockUpdate();
	standbyTime=-1;
	
	skipcounter=0;
	skipping=0;
	
	recording=0;
	
	addActionMap(&i_enigmaMainActions->map);
	addActionMap(&i_numberActions->map);
	
	gotPMT();
	gotSDT();
	gotEIT();

	playlistref=eServiceReference(eServicePlaylistHandler::ID, eServiceReference::flagDirectory, 1, 0);
	eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeFavourite), playlistref);
	
	curlist=(ePlaylist*)eServiceInterface::getInstance()->addRef(playlistref);
	
	if (!curlist)
		eFatal("couldn't allocate playlist!");

	timerlistref=eServiceReference(eServicePlaylistHandler::ID, eServiceReference::flagDirectory, 1, 1);
	timerlist=(ePlaylist*)eServiceInterface::getInstance()->addRef(timerlistref);
	ASSERT(timerlist);
	timerlist->service_name=_("Timerlist");
	timerlist->load(CONFIGDIR "/enigma/timer.epl");

	for (int i = modeTV; i <= modeFile; i++)
	{
		favouriteref[i]=eServiceReference(eServicePlaylistHandler::ID, eServiceReference::flagDirectory, 2, i);
		favourite[i]=(ePlaylist*)eServiceInterface::getInstance()->addRef(favouriteref[i]);
		ASSERT(favourite[i]);
		char *descr="";
		switch (i)
		{
		case modeTV: descr=_("TV"); break;
		case modeRadio: descr=_("Radio"); break;
		case modeFile: descr=_("File"); break;
		}
		favourite[i]->service_name=eString(_("Favourites (")) + eString(descr) + ")";
		favourite[i]->load(eString().sprintf( CONFIGDIR "/enigma/favourite.%d.epl", i).c_str());
		int modemap[]={eServiceStructureHandler::modeTV, eServiceStructureHandler::modeRadio, eServiceStructureHandler::modeFile};
		eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(modemap[i]), favouriteref[i]);
		eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeFavourite), favouriteref[i]);
	}

	CONNECT(eZap::getInstance()->getServiceSelector()->addServiceToList, eZapMain::doPlaylistAdd);
	CONNECT(eZap::getInstance()->getServiceSelector()->addServiceToFavourite, eZapMain::addServiceToFavourite);
	CONNECT(eZap::getInstance()->getServiceSelector()->showMenu, eZapMain::showServiceMenu);
	CONNECT(eZap::getInstance()->getServiceSelector()->showFavourite, eZapMain::showFavourite);
	CONNECT(eZap::getInstance()->getServiceSelector()->setMode, eZapMain::setModeD);
	
	last_mode=mode=-1;

	// read for all modes last servicePath from registry
	mode = modeTV;  // begin by modeTV
	while ( mode < modeEnd )  //.. modeRadio .. modeFile .. modeEnd
	{
		char* str;
		if ( !eConfig::getInstance()->getKey( eString().sprintf("/ezap/ui/modes/%i", mode).c_str(), str) )
			modeLast[mode++].setString(str);
		else
		{
			modeLast[mode]=eServiceStructureHandler::getRoot(mode+1);
			modeLast[mode++].down( eServiceReference() );
		}
	}

	int tmp;
  // read last mode from registry
  if ( eConfig::getInstance()->getKey("ezap/ui/lastmode", tmp ) )
		tmp = 0;  // defaut TV Mode

	mode=-1;  // fake mode for first call of setMode
	curlist->load(CONFIGDIR "/enigma/playlist.epl");

	// now we set the last mode
	setMode(tmp);

	if (curlist->current != curlist->list.end())
		playService(*curlist->current, psDontAdd);

	startMessages();
}

eZapMain::~eZapMain()
{
	getPlaylistPosition();
	if (mode != -1)
		getServiceSelectorPath(modeLast[mode]);

  // save last mode to registry
 	eConfig::getInstance()->setKey("ezap/ui/lastmode", (last_mode==-1) ? mode : last_mode);

	// save for all modes the servicePath to registry
	mode=0;	
	while ( mode < modeEnd )
		eConfig::getInstance()->setKey( eString().sprintf("/ezap/ui/modes/%i", mode).c_str(), modeLast[mode++].toString().c_str() );

	timerlist->save(CONFIGDIR "/enigma/timer.epl");
	eServiceInterface::getInstance()->removeRef(timerlistref);

	for (int i = modeTV; i <= modeFile; i++)
	{
		favourite[i]->save(eString().sprintf( CONFIGDIR "/enigma/favourite.%d.epl", i).c_str());
		eServiceInterface::getInstance()->removeRef(favouriteref[i]);
	}

	curlist->save(CONFIGDIR "/enigma/playlist.epl");
	eServiceInterface::getInstance()->removeRef(playlistref);
	
	if (instance == this)
		instance = 0;
	eZapLCD *pLCD=eZapLCD::getInstance();
	pLCD->lcdMain->hide();
	pLCD->lcdShutdown->show();
}

void eZapMain::set16_9Logo(int aspect)
{
	if (aspect)
	{
		WideOff->hide();
		WideOn->show();
	} else
	{
		WideOn->hide();
		WideOff->show();
	}
}

void eZapMain::setEPGButton(bool b)
{
	if (b)
	{
		isEPG=1;
		ButtonRedDis->hide();
		ButtonRedEn->show();
	}
	else
	{
		isEPG=0;
		ButtonRedEn->hide();
		ButtonRedDis->show();
	}
}

void eZapMain::setVTButton(bool b)
{
	if (b)
	{
		ButtonBlueDis->hide();
		ButtonBlueEn->show();
	}
	else
	{
		ButtonBlueEn->hide();
		ButtonBlueDis->show();
	}
}

void eZapMain::setAC3Logo(bool b)
{
	if (b)
	{
		DolbyOff->hide();
		DolbyOn->show();
	} else
	{
		DolbyOn->hide();
		DolbyOff->show();
	}
}

void eZapMain::setSmartcardLogo(bool b)
{
	if (b)
	{
		CryptOff->hide();
		CryptOn->show();
	} else
	{
		CryptOn->hide();
		CryptOff->show();
	}
}

void eZapMain::setEIT(EIT *eit)
{
	int numsub=0;
	subservicesel.clear();
	
	if (eit)
	{
		eString nowtext, nexttext, nowtime="", nexttime="", descr;
		int val=0;
		int p=0;
		
		for (ePtrList<EITEvent>::iterator i(eit->events); i != eit->events.end(); ++i)
		{
			EITEvent *event=*i;
			if ((event->running_status>=2) || ((!p) && (!event->running_status)))
			{
				cur_start=event->start_time;
				cur_duration=event->duration;
				clockUpdate();

				eZapLCD* pLCD = eZapLCD::getInstance();
				pLCD->lcdMain->updateProgress(cur_start,cur_duration);

				for (ePtrList<Descriptor>::iterator d(event->descriptor); d != event->descriptor.end(); ++d)
					if (d->Tag()==DESCR_LINKAGE)
					{
						LinkageDescriptor *ld=(LinkageDescriptor*)*d;
						if (ld->linkage_type!=0xB0)
							continue;
						subservicesel.add(ld);
						numsub++;
					}
			}
			for (ePtrList<Descriptor>::iterator d(event->descriptor); d != event->descriptor.end(); ++d)
			{
				Descriptor *descriptor=*d;
				if (descriptor->Tag()==DESCR_SHORT_EVENT)
				{
					ShortEventDescriptor *ss=(ShortEventDescriptor*)descriptor;
					switch (p)
					{
					case 0:
						nowtext=ss->event_name;
						val|=1;
						descr=ss->text;
						break;
					case 1:
						nexttext=ss->event_name;
						val|=2;
						break;
					}
				}
			}
			tm *t=event->start_time!=-1?localtime(&event->start_time):0;
			eString start="";
			if (t && event->duration)
				start.sprintf("%02d:%02d", t->tm_hour, t->tm_min);
			eString duration;
			if (event->duration>0)
				duration.sprintf("%d min", event->duration/60);
			else
				duration="";
			switch (p)
			{
			case 0:
				EINowDuration->setText(duration);
				nowtime=start;
				break;
			case 1:
				EINextDuration->setText(duration);
				nexttime=start;
				break;
			}
			Description->setText(descr);
			p++;
		}
		if (val&1)
		{
			EINow->setText(nowtext);
			EINowTime->setText(nowtime);
		}

		if (val&2)
		{
			EINext->setText(nexttext);
			EINextTime->setText(nexttime);
		}
	} else
	{
		EINow->setText(_("no EPG available"));
		EINext->setText("");
		EINowDuration->setText("");
		EINextDuration->setText("");
		EINowTime->setText("");
		EINextTime->setText("");
	}
	if (numsub>1)
		flags|=ENIGMA_SUBSERVICES;
	else
		flags&=~ENIGMA_SUBSERVICES;
	if (flags&(ENIGMA_NVOD|ENIGMA_SUBSERVICES))
	{
		ButtonGreenEn->show();
		ButtonGreenDis->hide();
	}
	else
	{
		ButtonGreenDis->show();
		ButtonGreenEn->hide();
	}
	ePtrList<EITEvent> dummy;
	if (actual_eventDisplay)
		actual_eventDisplay->setList(eit?eit->events:dummy);
}

void eZapMain::updateProgress()
{
	if (serviceFlags & eServiceHandler::flagSupportPosition)
	{
		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		if (!handler)
			return;
		int total=handler->getPosition(eServiceHandler::posQueryLength);
		int current=handler->getPosition(eServiceHandler::posQueryCurrent);
		if ((total > 0) && (current != -1))
		{
			Progress->setPerc(current*100/total);
			Progress->show();
			int min=total-current;
			int sec=min%60;
			min/=60;
			int sign=-1;
			ChannelNumber->setText(eString().sprintf("%s%d:%02d", (sign==-1)?"-":"", min, sec));
		} else
		{
			Progress->hide();
			ChannelNumber->setText("");
		}
	}
	else
	{
		time_t c=time(0)+eDVB::getInstance()->time_difference;
		tm *t=localtime(&c);
		if (t && eDVB::getInstance()->time_difference)
		{
			if ((cur_start <= c) && (c < cur_start+cur_duration))
			{
				Progress->setPerc((c-cur_start)*100/cur_duration);
				Progress->show();
			} else
			{
				Progress->clear();
				Progress->hide();
			}
		} else
		{
			Progress->clear();
			Progress->hide();
		}
	}
}

void eZapMain::setPlaylistPosition()
{
	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	if (!handler)
		return;
	if (curlist->current != curlist->list.end())
		if (curlist->current->current_position != -1)
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekReal, curlist->current->current_position));
}

void eZapMain::getPlaylistPosition()
{
	int time=-1;
	if (serviceFlags & eServiceHandler::flagIsSeekable)
	{
		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		if (!handler)
			return;
		time=handler->getPosition(eServiceHandler::posQueryRealCurrent);
	}
	if (curlist->current != curlist->list.end())
		curlist->current->current_position=time;
}


void eZapMain::handleNVODService(SDTEntry *sdtentry)
{
	nvodsel.clear();
	for (ePtrList<Descriptor>::iterator i(sdtentry->descriptors); i != sdtentry->descriptors.end(); ++i)
		if (i->Tag()==DESCR_NVOD_REF)
			for (ePtrList<NVODReferenceEntry>::iterator e(((NVODReferenceDescriptor*)*i)->entries); e != ((NVODReferenceDescriptor*)*i)->entries.end(); ++e)
				nvodsel.add(*e);
	eService *service=eServiceInterface::getInstance()->addRef(eServiceInterface::getInstance()->service);
	if (service)
		nvodsel.setText(service->service_name.c_str());
	eServiceInterface::getInstance()->removeRef(eServiceInterface::getInstance()->service);
}

void eZapMain::showServiceSelector(int dir)
{
	hide();

	eZapLCD* pLCD = eZapLCD::getInstance();
	pLCD->lcdMain->hide();
	pLCD->lcdMenu->show();

	entered_playlistmode=0;
	
	eServiceSelector *e = eZap::getInstance()->getServiceSelector();
	e->setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);

	getServiceSelectorPath(modeLast[mode]);

	const eServiceReference *service = e->choose(dir);	// reset path only when NOT showing specific list
	
	pLCD->lcdMain->show();
	pLCD->lcdMenu->hide();

	if (!service)
	{
		setServiceSelectorPath(modeLast[mode]);
		return;
	}
	
	if (*service == eServiceInterface::getInstance()->service)
		return;

	getServiceSelectorPath(modeLast[mode]);

	if (eZap::getInstance()->getServiceSelector()->getPath().current() != playlistref)
	{
		if (!entered_playlistmode)
			playlistmode=0;
		playService(*service, playlistmode?psAdd:0);
	} else
		playService(*service, psDontAdd);
}

void eZapMain::nextService(int add)
{
	const eServiceReference *service=eZap::getInstance()->getServiceSelector()->next();
	if (!service)
		return;
	else
		getServiceSelectorPath( modeLast[mode] );
	
	if (service->flags & eServiceReference::mustDescent)
		return;

	playService(*service, add?0:psDontAdd);}

void eZapMain::prevService()
{
	const eServiceReference *service=eZap::getInstance()->getServiceSelector()->prev();
	if (!service)
		return;
	else
		getServiceSelectorPath( modeLast[mode] );
	
	if (service->flags & eServiceReference::mustDescent)
		return;

	playService(*service, psDontAdd);
}

void eZapMain::playlistPrevService()
{
	eDebug("playlist prev");
	getPlaylistPosition();
	if (curlist->current != curlist->list.begin())
	{
		if (playlistmode)
			curlist->current->current_position=-1;
		curlist->current--;
		playService(*curlist->current, psDontAdd);
	}
}

void eZapMain::playlistNextService()
{
	eDebug("playlist next");
	getPlaylistPosition();
	if (curlist->current != curlist->list.end())
	{
		if (playlistmode)
			curlist->current->current_position=-1;
		curlist->current++;
		if (curlist->current == curlist->list.end())
		{
			curlist->current--;
			return;
		}
		playService(*curlist->current, psDontAdd);
	}
}

void eZapMain::volumeUp()
{
	eAVSwitch::getInstance()->changeVolume(0, -4);
//		if (!isVisible())
//			show();
//		timeout.start(1000, 1);
}

void eZapMain::volumeDown()
{
	eAVSwitch::getInstance()->changeVolume(0, +4);
//		if (!isVisible())
//			show();
//		timeout.start(1000, 1);
}

void eZapMain::toggleMute()
{
	eAVSwitch::getInstance()->toggleMute();
}

void eZapMain::showMainMenu()
{
	if (isVisible())
		hide();

	eZapLCD* pLCD = eZapLCD::getInstance();
	pLCD->lcdMain->hide();
	pLCD->lcdMenu->show();

	eMainMenu mm;
	mm.setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
	mm.show();
	if (mm.exec() == 1)
		eZap::getInstance()->quit();
	mm.hide();

	pLCD->lcdMenu->hide();
	pLCD->lcdMain->show();
}

void eZapMain::standbyPress()
{
	standbyTime = time(0);
}

void eZapMain::standbyRepeat()
{
	if (standbyTime == -1)		// just waking up
		return;
	int diff = time(0) - standbyTime;
	if (diff > 2)
		standbyRelease();
}

void eZapMain::standbyRelease()
{
	if (standbyTime == -1)		// just waking up
		return;
	int diff = time(0) - standbyTime;
	standbyTime=-1;
	if (diff > 2)
		eZap::getInstance()->quit();
	else
	{
		eZapStandby standby;
		if (isVisible())
			hide();
		standby.show();
		standby.exec();
		standby.hide();
	}
}

void eZapMain::showInfobar()
{
	timeout.start(10000, 1);
	show();
}

void eZapMain::hideInfobar()
{
	timeout.stop();
	hide();
}

void eZapMain::play()
{
	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	if (!handler)
		return;
	
	if (skipping)
	{
		skipping=0;
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, 1));
	} else if (handler->getState() == eServiceHandler::statePause)
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, 1));
	else
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekAbsolute, 0));
	updateProgress();
}

void eZapMain::stop()
{
	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	if (!handler)
		return;
	handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, 0));
	handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekAbsolute, 0));
	updateProgress();
}

void eZapMain::pause()
{
	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	if (!handler)
		return;
	if (handler->getState() == eServiceHandler::statePause)
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, 1));
	else
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, 0));
}

void eZapMain::record()
{
	eDebug("record *g*");
	
	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	if (!handler)
		return;
	if (!recording)
	{
//		#ifdef DBOX
//		Decoder::parms.recordmode=1;
//		Decoder::Set();
//		#endif

		eDebug("start recording...");
		if (handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordOpen)))
		{
			eDebug("couldn't record... :/");
		} else
		{
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordStart));
			eDebug("ok, recording...");
			recording=1;
		}
	} else
	{
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordStop));
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordClose));
		recording=0;
//		#ifdef DBOX
//		Decoder::parms.recordmode=0;
//		Decoder::Set();
//		#endif
	}
}

void eZapMain::startSkip(int dir)
{
	skipcounter=0;
	eDebug("startSkip: %d", dir);
	if (!isVisible())
		show();
	timeout.stop();
}

void eZapMain::repeatSkip(int dir)
{
	eDebug("repeatSkip: %d", dir);
	if (!skipping)
	{
		skipcounter++;
		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		if (handler)
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSkip, (dir == skipForward)?5000:-5000));
		updateProgress();
	}
}

void eZapMain::stopSkip(int dir)
{
	eDebug("stopSkip: %d", dir);
	if (!skipcounter)
	{
		if (dir == skipForward)
			skipping++;
		else
			skipping--;
			
		if (skipping > 3)
			skipping=3;
		else if (skipping < -3)
			skipping=-3;

		int speed;
		if (!skipping)
			speed=1;
		else if (skipping < 0)
			speed=-(1<<(-skipping));
		else
			speed=1<<skipping;

		eDebug("speed: %d", speed);
		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		if (handler)
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, speed));
	}
	if (isVisible())
		timeout.start(1000, 1);
}

void eZapMain::showServiceMenu(eServiceSelector *sel)
{
	sel->hide();
	eServiceContextMenu m(sel->getSelected());
	m.show();
	int res=m.exec();
	m.hide();
	switch (res)
	{
	case 0:
		break;
	case 1:
	{
		eMessageBox box(_("Really delete this service?"), _("delete service"), eMessageBox::btYes|eMessageBox::btNo, eMessageBox::btNo);
		box.show();
		int r=box.exec();
		box.hide();
		if (r == eMessageBox::btYes)
		{
			eServiceReference ref=sel->getSelected();
			if (eServiceInterface::getInstance()->service == ref)
				eServiceInterface::getInstance()->stop();
			int res=eServiceInterface::getInstance()->deleteService(sel->getPath().current(), ref);
			if (!res)
				sel->actualize();
			else
			{
				eMessageBox box(_("Sorry, you cannot delete this service."), _("delete service"));
				box.show();
				box.exec();
				box.hide();
				break;
			}
		}
		break;
	}
	case 2:
	{
		eMessageBox box(_("Sorry, not yet implemented."), _("move service"));
		box.show();
		box.exec();
		box.hide();
		break;
	}
	}
	sel->show();
}

void eZapMain::showFavourite(eServiceSelector *)
{
	if (last_mode == -1)	// not in playlist mode
	{
		playlistmode=1;
		setMode(modePlaylist);
	} else
		setMode(last_mode);
}

void eZapMain::playService(const eServiceReference &service, int flags)
{
	int first=0;

	if (flags & psDontAdd)
	{
		if ((curlist->current != curlist->list.end() ) && (curlist->current->service != service))
			getPlaylistPosition();
		eServiceInterface::getInstance()->play(service);

		std::list<ePlaylistEntry>::iterator i=std::find(curlist->list.begin(), curlist->list.end(), service);
		if (i != curlist->list.end())
		{
			curlist->current=i;
			setPlaylistPosition();
		}
		return;
	}
		// we assume that no "isDirectory" is set - we won't open the service selector again.
	if (!(flags & psAdd))
	{
		if (!playlistmode)		// dem user liebgewonnene playlists nicht einfach killen
			while (curlist->list.size() > 10)
				curlist->list.pop_front();
		if ((!playlistmode) && (service.flags & eServiceReference::mustDescent)) // a playlist/complete directory..
		{
			curlist->list.clear();
			first=1;
			playlistmode=1;
		} else
		{
			curlist->current=curlist->list.end();
			if (curlist->current == curlist->list.begin())
				first=1;
			else
				--curlist->current;
		}
	}
	addService(service);
	if (! (flags & psAdd))
	{
		if (first)
			curlist->current=curlist->list.begin();
		else if (curlist->current != curlist->list.end())
			curlist->current++;
		
		if (curlist->current != curlist->list.end())
			eServiceInterface::getInstance()->play(*curlist->current);
		else
		{
			Description->setText(_("This directory doesn't contain anything playable!"));
			postMessage(eZapMessage(0, _("Play"), _("This directory doesn't contain anything playable!")), 1);
		}
	}
	if (eZap::getInstance()->getServiceSelector()->getPath().current() == playlistref)
		eZap::getInstance()->getServiceSelector()->actualize();
}

void eZapMain::addService(const eServiceReference &service)
{
	if (service.flags & eServiceReference::mustDescent) // recursive add services..
	{
		Signal1<void,const eServiceReference&> signal;
		CONNECT(&signal, eZapMain::addService);
		eServiceInterface::getInstance()->enterDirectory(service, signal);
		eServiceInterface::getInstance()->leaveDirectory(service);
	} else
	{
		curlist->list.remove(service);
		curlist->list.push_back(ePlaylistEntry(service));
	}
}

void eZapMain::doPlaylistAdd(const eServiceReference &service)
{
	entered_playlistmode=1;
	if (!playlistmode)
	{
		playlistmode=1;
		curlist->list.clear();
		curlist->current=curlist->list.begin();
		playService(service, 0);
	} else
		playService(service, psAdd);
}

void eZapMain::addServiceToFavourite(eServiceSelector *sel)
{
	const eServiceReference &service=sel->getSelected();
	if ((mode > modeFile) || (mode < 0))
		return;
	
	for (std::list<ePlaylistEntry>::iterator i(favourite[mode]->list.begin()); i != favourite[mode]->list.end(); ++i)
		if (i->service == service)
		{
			eMessageBox box(_("This service is already in your favourite list."), _("Add Channel to Favourite"), eMessageBox::btOK);
			sel->hide();
			box.show();
			box.exec();
			box.hide();
			sel->show();
			return;
		}

	eMessageBox box(_("Really add this channel to your favourite list?"), _("Add Channel to Favourite"), eMessageBox::btYes|eMessageBox::btNo);
	sel->hide();
	box.show();
	int res=box.exec();
	box.hide();
	sel->show();
	
	if ( res == eMessageBox::btYes )
		favourite[mode]->list.push_back(service);
}

void eZapMain::showSubserviceMenu()
{
	if (!(flags & (ENIGMA_NVOD|ENIGMA_SUBSERVICES)))
		return;

	eZapLCD* pLCD = eZapLCD::getInstance();
	pLCD->lcdMain->hide();
	pLCD->lcdMenu->show();
	if (flags&ENIGMA_NVOD)
	{
		if (isVisible())
		{
			timeout.stop();
			hide();
		}
		nvodsel.setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
		nvodsel.show();
		nvodsel.exec();
		nvodsel.hide();
	}
	else if (flags&ENIGMA_SUBSERVICES)
	{
		if (isVisible())
		{
			timeout.stop();
			hide();
		}
		subservicesel.setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
		subservicesel.show();
		subservicesel.exec();
		subservicesel.hide();
	}
	pLCD->lcdMenu->hide();
	pLCD->lcdMain->show();
}

void eZapMain::showAudioMenu()
{
	if (flags&ENIGMA_AUDIO)
	{
		eZapLCD* pLCD = eZapLCD::getInstance();
		pLCD->lcdMain->hide();
		pLCD->lcdMenu->show();
		if (isVisible())
		{
			timeout.stop();
			hide();
		}
		audiosel.setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
		audiosel.show();
		audiosel.exec();
		audiosel.hide();
		pLCD->lcdMenu->hide();
		pLCD->lcdMain->show();
	}
}

extern eString getInfo(const char *file, const char *info);

void eZapMain::runVTXT()
{
	if (isVT)
	{
		eZapPlugins plugins;
		
		struct dirent **namelist;

		int n = scandir(PLUGINDIR "/", &namelist, 0, alphasort);

		if (n < 0)
		{
			eDebug("Error Read Plugin Directory");
			eMessageBox msg("Error Read Plugin Directory", "Error");
			msg.show();
			msg.exec();
			msg.hide();
			return;
		}

		for(int count=0;count<n;count++)
		{
			eString	FileName(PLUGINDIR "/");
			FileName+=namelist[count]->d_name;
			if ( FileName.find(".cfg") != eString::npos )
			{
				eString aneedvtxtpid=getInfo(FileName.c_str(), "needvtxtpid");
				if ( aneedvtxtpid.isNull()?false:atoi(aneedvtxtpid.c_str()) )
					plugins.execPluginByName(namelist[count]->d_name);
			}
			free(namelist[count]);
	  }
		free(namelist);
	}
}

void eZapMain::showEPGList()
{
#if 1
	eServiceReferenceDVB &service=(eServiceReferenceDVB&)eServiceInterface::getInstance()->service;
	if (service.type != eServiceReference::idDVB)
		return;
	if (isEPG)
	{
		eZapLCD* pLCD = eZapLCD::getInstance();
		pLCD->lcdMain->hide();
		pLCD->lcdMenu->show();
		eEPGSelector wnd(service);
		wnd.setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
		if (isVisible())
		{
			timeout.stop();
			hide();
		}
		wnd.show();
		wnd.exec();
		wnd.hide();
		pLCD->lcdMenu->hide();
		pLCD->lcdMain->show();
	}
#else
	VCR bla;
	hide();
	bla.show();
	bla.exec();
	bla.hide();
	show();
#endif
}

void eZapMain::showEPG()
{
	eService* service = eDVB::getInstance()->settings->getTransponders()->searchService(eServiceInterface::getInstance()->service);

	if (!service)
		return;
		
	if (isVisible())
	{
		timeout.stop();
		hide();
	}

#ifdef USE_CACHED_EPG
#error no
	const eventMap* pMap = eEPGCache::getInstance()->getEventMap(service->getOriginalNetworkID(), service->getServiceID());

	if (pMap && isEPG)  // EPG vorhanden
	{
		eventMap::const_iterator It = pMap->begin();
			
		ePtrList<EITEvent> events;
		events.setAutoDelete(true);
			
		while (It != pMap->end())  // sicher ist sicher !
		{
			events.push_back( new EITEvent(*It->second));
			It++;
		}
		eEventDisplay ei(service->service_name.c_str(), &events);			
		actual_eventDisplay=&ei;
		eZapLCD* pLCD = eZapLCD::getInstance();
		pLCD->lcdMain->hide();
		pLCD->lcdMenu->show();
		ei.setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
		ei.show();
		ei.exec();
		ei.hide();
		pLCD->lcdMenu->hide();
		pLCD->lcdMain->show();
		actual_eventDisplay=0;
	} else	
#else
	{
		EIT *eit=eDVB::getInstance()->getEIT();
		ePtrList<EITEvent> dummy;
		{
			eEventDisplay ei(service->service_name.c_str(), eit?&eit->events:&dummy);
			if (eit)
				eit->unlock();		// HIER liegt der hund begraben.
			actual_eventDisplay=&ei;
			eZapLCD* pLCD = eZapLCD::getInstance();
			pLCD->lcdMain->hide();
			pLCD->lcdMenu->show();
			ei.setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
			ei.show();
			ei.exec();
			ei.hide();
			pLCD->lcdMenu->hide();
			pLCD->lcdMain->show();
			actual_eventDisplay=0;
		}
	}
#endif
}

int eZapMain::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
	case eWidgetEvent::evtAction:
	{
		int num=0;
		stopMessages();
		if (event.action == &i_enigmaMainActions->showMainMenu)
			showMainMenu();
		else if (event.action == &i_enigmaMainActions->standby_press)
			standbyPress();
		else if (event.action == &i_enigmaMainActions->standby_repeat)
			standbyRepeat();
		else if (event.action == &i_enigmaMainActions->standby_release)
			standbyRelease();
		else if ((!isVisible()) && (event.action == &i_enigmaMainActions->toggleInfobar))
			showInfobar();
		else if (isVisible() && (event.action == &i_enigmaMainActions->toggleInfobar))
			hideInfobar();
		else if (event.action == &i_enigmaMainActions->showServiceSelector)
			showServiceSelector(-1);
		else if (event.action == &i_enigmaMainActions->showSubservices)
			showSubserviceMenu();
		else if (event.action == &i_enigmaMainActions->showAudio)
			showAudioMenu();
		else if (event.action == &i_enigmaMainActions->pluginVTXT)
			runVTXT();
		else if (event.action == &i_enigmaMainActions->showEPGList)
			showEPGList();
		else if (event.action == &i_enigmaMainActions->showEPG)
			showEPG();
		else if (event.action == &i_enigmaMainActions->nextService)
			nextService();
		else if (event.action == &i_enigmaMainActions->prevService)
			prevService();
		else if (event.action == &i_enigmaMainActions->playlistNextService)
			playlistNextService();
		else if (event.action == &i_enigmaMainActions->playlistPrevService)
			playlistPrevService();
		else if (event.action == &i_enigmaMainActions->serviceListDown)
			showServiceSelector(eServiceSelector::dirDown);
		else if (event.action == &i_enigmaMainActions->serviceListUp)
			showServiceSelector(eServiceSelector::dirUp);
		else if (event.action == &i_enigmaMainActions->volumeUp)
			volumeUp();
		else if (event.action == &i_enigmaMainActions->volumeDown) 
			volumeDown();
		else if (event.action == &i_enigmaMainActions->toggleMute)
			toggleMute();
		else if (isSeekable() && event.action == &i_enigmaMainActions->play)
			play();
		else if (isSeekable() && event.action == &i_enigmaMainActions->stop)
			stop();
		else if (isSeekable() && event.action == &i_enigmaMainActions->pause)
			pause();
		else if (event.action == &i_enigmaMainActions->record)
			record();
		else if (isSeekable() && event.action == &i_enigmaMainActions->startSkipForward)
			startSkip(skipForward);
		else if (isSeekable() && event.action == &i_enigmaMainActions->repeatSkipForward)
			repeatSkip(skipForward);
		else if (isSeekable() && event.action == &i_enigmaMainActions->stopSkipForward)
			stopSkip(skipForward);
		else if (isSeekable() && event.action == &i_enigmaMainActions->startSkipReverse)
			startSkip(skipReverse);
		else if (isSeekable() && event.action == &i_enigmaMainActions->repeatSkipReverse)
			repeatSkip(skipReverse);
		else if (isSeekable() && event.action == &i_enigmaMainActions->stopSkipReverse)
			stopSkip(skipReverse);
		else if (event.action == &i_numberActions->key1)
			num=1;
		else if (event.action == &i_numberActions->key2)
			num=2;
		else if (event.action == &i_numberActions->key3)
			num=3;
		else if (event.action == &i_numberActions->key4)
			num=4;
		else if (event.action == &i_numberActions->key5)
			num=5;
		else if (event.action == &i_numberActions->key6)
			num=6;
		else if (event.action == &i_numberActions->key7)
			num=7;
		else if (event.action == &i_numberActions->key8)
			num=8;
		else if (event.action == &i_numberActions->key9)
			num=9;
		else
		{
			startMessages();
			break;
		}
		startMessages();
		if ( num )
		{
			if (isVisible())
				hide();
			eServiceNumberWidget s(num);
			s.show();
			num = s.exec();
			if (num != -1)
			{
				if ( eZap::getInstance()->getServiceSelector()->selectService( num ) )
				{
					getServiceSelectorPath(modeLast[mode]);
					playService(modeLast[mode].current(), psDontAdd);
				}
				else
					setServiceSelectorPath(modeLast[mode]);
			}
			num = 0;
  	}
		return 1;
	}
	default:
		break;
	}
	return eWidget::eventHandler(event);
}

void eZapMain::handleServiceEvent(const eServiceEvent &event)
{
	switch (event.type)
	{
	case eServiceEvent::evtStateChanged:
		break;
	case eServiceEvent::evtFlagsChanged:
	{
		serviceFlags = eServiceInterface::getInstance()->getService()->getFlags();
		setSmartcardLogo( serviceFlags & eServiceHandler::flagIsScrambled );
		if (serviceFlags & eServiceHandler::flagSupportPosition)
		{
			eDebug("support position query..");
			progresstimer.start(1000);
		} else
		{	
			eDebug("doesn't support position query");
			progresstimer.stop();
		}
		updateProgress();
		break;
	}
	case eServiceEvent::evtAspectChanged:
	{
		int aspect = eServiceInterface::getInstance()->getService()->getAspectRatio();
		set16_9Logo(aspect);
		break;
	}
	case eServiceEvent::evtStart:
	{
		int err = eServiceInterface::getInstance()->getService()->getErrorInfo();
		serviceFlags = eServiceInterface::getInstance()->getService()->getFlags();
		startService(eServiceInterface::getInstance()->service, err);
		break;
	}
	case eServiceEvent::evtStop:
		leaveService();
		break;
	case eServiceEvent::evtGotEIT:
		gotEIT();
		break;
	case eServiceEvent::evtGotSDT:
		gotSDT();
		break;
	case eServiceEvent::evtGotPMT:
		gotPMT();
		break;
	case eServiceEvent::evtEnd:
	{
		eDebug("evtEnd - next entry!");
		if (curlist->current != curlist->list.end())
		{
			curlist->current->current_position=-1;
			++curlist->current;
		}
		if (curlist->current != curlist->list.end() )
		{
			curlist->current->current_position=-1;		// start from beginning.
			eServiceInterface::getInstance()->play(*curlist->current);
		}
		else if (!playlistmode)
			nextService(1);
		else
			eDebug("ende im gelaende.");
		break;
	}
	}
}

void eZapMain::startService(const eServiceReference &_serviceref, int err)
{
	eServiceHandler *sapi=eServiceInterface::getInstance()->getService();
	if (!sapi)
		return;
		
	eService *service=eServiceInterface::getInstance()->addRef(_serviceref);

	if (_serviceref.type == eServiceReference::idDVB)
	{
		isVT = Decoder::parms.tpid != -1;

		const eServiceReferenceDVB &serviceref=(const eServiceReferenceDVB&)_serviceref;

		setVTButton(isVT);

			// es wird nur dann versucht einen service als referenz-service zu uebernehmen, wenns den ueberhaupt
			// gibt.

		if (service)
			switch (serviceref.getServiceType())
			{
			case 1:	// TV
			case 2: // radio
			case 4: // nvod ref
				refservice=serviceref;
				break;
			}

		eService *rservice=0;
		if (refservice != serviceref)
			rservice=eDVB::getInstance()->settings->getTransponders()->searchService(refservice);
	
		if (refservice.getServiceType()==4)
			flags|=ENIGMA_NVOD;
		else
			flags&=~ENIGMA_NVOD;

		eString name="";

		if (rservice)
			name=rservice->service_name + " - ";
	
		if (service)
			name+=service->service_name;
		else
			switch (serviceref.getServiceType())
			{
			case 5: // nvod stream or linkage subservice ( type faked in SubServiceSelector::selected )
				name+=serviceref.descr;
			}

		if (!name.length())
			name="unknown service";

		ChannelName->setText(name);		

		switch (err)
		{
		case 0:
			Description->setText("");
			postMessage(eZapMessage(0), 1);
			break;
		case -EAGAIN:
			Description->setText(_("One moment please..."));
			postMessage(eZapMessage(0, _("switch"), _("One moment please...")), 1);
			break;
		case -ENOENT:
			Description->setText(_("Service could not be found !"));
			postMessage(eZapMessage(0, _("switch"), _("Service could not be found !")), 1);
			eDebug("Sender konnte nicht gefunden werden.");
			break;
		case -ENOCASYS:
		{
			int serviceFlags = eServiceInterface::getInstance()->getService()->getFlags();
			if( serviceFlags & eServiceHandler::flagIsScrambled )
			{
				Description->setText(_("This service could not be descrambled"));
				postMessage(eZapMessage(0, _("switch"), _("This service could not be descrambled")), 1);
				eDebug("This service could not be descrambled");
			}
			break;
		}
		case -ENOSTREAM:
			Description->setText(_("This service sends (currently) no signal"));
			postMessage(eZapMessage(0, _("switch"), _("This service sends (currently) no signal")), 1);
			eDebug("This service sends (currently) no signal");
			break;
		case -ENOSYS:
			Description->setText(_("This content could not be displayed"));
			eDebug("This content could not be displayed");
			postMessage(eZapMessage(0, _("switch"), _("This content could not be displayed")), 1);
			break;
		case -ENVOD:
			Description->setText(_("NVOD Service - select a starttime, please"));
			eDebug("NVOD Service - select a starttime, please");
			postMessage(eZapMessage(0, _("switch"), _("NVOD Service - select a starttime, please")), 1);
			break;
		default:
			Description->setText(_("Unknown error!!"));
			eDebug("Unknown error!!");
			postMessage(eZapMessage(0, _("switch"), _("Unknown error!!")), 1);
			break;
		}

		int num=-1;
	
		if (rservice)
			num=eZap::getInstance()->getServiceSelector()->getServiceNum(_serviceref);
		else if (service)
			num=eZap::getInstance()->getServiceSelector()->getServiceNum(_serviceref);

		if (num != -1)
			ChannelNumber->setText(eString().sprintf("%d", num));
		else
			ChannelNumber->setText("");

		if (flags&(ENIGMA_NVOD|ENIGMA_SUBSERVICES))
		{
			ButtonGreenDis->hide();
			ButtonGreenEn->show();
		}
		else
		{
			ButtonGreenEn->hide();
			ButtonGreenDis->show();	
		}

		if (flags&ENIGMA_AUDIO)
		{
			ButtonYellowDis->hide();
			ButtonYellowEn->show();
		}
		else
		{
			ButtonYellowEn->hide();
			ButtonYellowDis->show();
		}
	} else
	{
		eString name;
		postMessage(eZapMessage(0), 1);
		if (service)
			name=service->service_name;
		else
			name="bla :(";

		ChannelName->setText(name);		
	}
	
	if (!eZap::getInstance()->focus)
		show();

	eServiceInterface::getInstance()->removeRef(_serviceref);

// Quick und Dirty ... damit die aktuelle Volume sofort angezeigt wird.
	eAVSwitch::getInstance()->sendVolumeChanged();

	timeout.start((sapi->getState() == eServiceHandler::statePlaying)?10000:2000, 1);
}

void eZapMain::gotEIT()
{
	eServiceHandler *sapi=eServiceInterface::getInstance()->getService();
	if (!sapi)
		return;

	EIT *eit=sapi->getEIT();
	setEIT(eit);

	if (eit)
	{
		int state=0;
		eConfig::getInstance()->getKey("/ezap/osd/showOSDOnEITUpdate", state);

		if (!eZap::getInstance()->focus && state)
		{
			show();
			timeout.start((sapi->getState() == eServiceHandler::statePlaying)?10000:2000, 1);
		}
		eit->unlock();
	}
}

void eZapMain::gotSDT()
{
	eServiceHandler *sapi=eServiceInterface::getInstance()->getService();
	if (!sapi)
		return;
		
	SDT *sdt=sapi->getSDT();
	if (!sdt)
		return;

	switch (((eServiceReferenceDVB&)eServiceInterface::getInstance()->service).getServiceType())
	{
	case 0x4:	// nvod reference
	{
		for (ePtrList<SDTEntry>::iterator i(sdt->entries); i != sdt->entries.end(); ++i)
		{
			if (eServiceID(i->service_id)==((eServiceReferenceDVB&)eServiceInterface::getInstance()->service).getServiceID())
			{
				handleNVODService(*i);
			}
		}
		break;
	}
	}
	sdt->unlock();
}

void eZapMain::gotPMT()
{
	eServiceHandler *sapi=eServiceInterface::getInstance()->getService();
	if (!sapi)
		return;
		
	PMT *pmt=sapi->getPMT();
	if (!pmt)
		return;

	bool isAc3 = false;
	eDebug("got pmt");
	int numaudio=0;
	audiosel.clear();
	for (ePtrList<PMTEntry>::iterator i(pmt->streams); i != pmt->streams.end(); ++i)
	{
		PMTEntry *pe=*i;
		int isaudio=0;
		if (pe->stream_type==3)
			isaudio=1;
		if (pe->stream_type==4)
			isaudio=1;
		if (pe->stream_type==6)
		{
			for (ePtrList<Descriptor>::iterator d(pe->ES_info); d != pe->ES_info.end(); ++d)
				if (d->Tag()==DESCR_AC3)
				{
					isaudio++;
					isAc3=true;
				}
				
		}
		if (isaudio)
		{
			audiosel.add(pe);
			numaudio++;
		}
	}
	if (numaudio>1)
		flags|=ENIGMA_AUDIO;
	else
		flags&=~ENIGMA_AUDIO;
		
	setAC3Logo(isAc3);
	
	pmt->unlock();
}

void eZapMain::timeOut()
{
	if (eZap::getInstance()->focus==this)
		hide();
}

void eZapMain::leaveService()
{
	eDebug("leaving service");

//	flags=0;
	
	ChannelName->setText("");
	ChannelNumber->setText("");
	Description->setText("");

	EINow->setText("");
	EINowDuration->setText("");
	EINowTime->setText("");
	EINext->setText("");
	EINextDuration->setText("");
	EINextTime->setText("");
	
	Progress->clear();
	Progress->hide();
}

void eZapMain::clockUpdate()
{
	time_t c=time(0)+eDVB::getInstance()->time_difference;
	tm *t=localtime(&c);
	if (t && eDVB::getInstance()->time_difference)
	{
		eString s;
		s.sprintf("%02d:%02d", t->tm_hour, t->tm_min);
		clocktimer.start((70-t->tm_sec)*1000);
		Clock->setText(s);
	} else
	{
		Clock->setText("--:--");
		clocktimer.start(60000);
	}
	updateProgress();
}

void eZapMain::updateVolume(int vol)
{
	if ( vol == 63 && !mute.isVisible() )
		mute.show();
	else if (vol < 63 && mute.isVisible() )
		mute.hide();

	VolumeBar->setPerc((63-vol)*100/63);
}

void eZapMain::postMessage(const eZapMessage &message, int clear)
{
	eLocker l(messagelock);
	
	int c=0;
	if (clear)
	{
		for (std::list<eZapMessage>::iterator i(messages.begin()); i != messages.end(); )
		{
			if (message.isSameType(*i))
			{
				if (i == messages.begin())
				{
					c=1;
					++i;
				} else
					i = messages.erase(i);
			} else
				++i;
		}
	}
	if (!message.isEmpty())
		messages.push_back(message);
	message_notifier.send(c);
}

void eZapMain::gotMessage(const int &c)
{
	if ((!c) && pMsg) // noch eine gueltige message vorhanden
	{
		return;
	}
	pauseMessages();
	while (!messages.empty())
	{
		nextMessage();
		if (pMsg)
			break;
	}
	startMessages();
}

void eZapMain::nextMessage()
{
	eZapMessage msg;
	messagelock.lock();
	
	messagetimeout.stop();

	if (pMsg)
	{
#if 0
		if (pMsg->in_loop)
			pMsg->close();
#endif 
		pMsg->hide();
		delete pMsg;
		pMsg=0;
		messages.pop_front();
	}

	std::list<eZapMessage>::iterator i(messages.begin());
 	if (i != messages.end())
 	{
 		msg=messages.front();
		messagelock.unlock();
		int showonly=msg.getTimeout()>=0;
		if (!showonly)
			hide();
		pMsg = new eMessageBox(msg.getBody(), msg.getCaption(), showonly?0:eMessageBox::btOK);
		pMsg->show();
		if (!showonly)
		{
			pMsg->exec();
			pMsg->hide();
			delete pMsg;
			pMsg=0;
			messages.pop_front();
		} else if (msg.getTimeout())
			messagetimeout.start(msg.getTimeout()*1000, 1);
	} else
		messagelock.unlock();
}

void eZapMain::stopMessages()
{
	pauseMessages();
	if (pMsg)
	{
		delete pMsg;
		pMsg=0;
		messages.pop_front();
	}
}

void eZapMain::startMessages()
{
	message_notifier.start();
}

void eZapMain::pauseMessages()
{
	message_notifier.stop();
}

void eZapMain::setMode(int newmode, int user)
{
	if ( newmode == modeFile && mode != newmode )
		eEPGCache::getInstance()->pauseEPG();
	else if ( mode == modeFile && mode != newmode && newmode != -1 )
		eEPGCache::getInstance()->restartEPG();

	eDebug("setting mode to %d", newmode);
	if (mode != -1)
		getServiceSelectorPath(modeLast[mode]);
	if (mode == newmode)
		user=0;

	if (newmode == modePlaylist)
		last_mode=mode;
	else
		last_mode=-1;

	mode=newmode;

	if (mode != -1)
		setServiceSelectorPath(modeLast[mode]);
	if (user)
		playService(modeLast[mode].current(), psDontAdd);
}

void eZapMain::setModeD(int newmode)
{
	setMode(newmode, 0);
}

void eZapMain::setServiceSelectorPath(eServicePath path)
{
	eServiceReference ref=path.current();
	path.up();
	eServicePath p = path;
//	eDebug("Setting currentService to %s", ref.toString().c_str() );
//	eDebug("setting path to %s", p.toString().c_str());
	eZap::getInstance()->getServiceSelector()->setPath(path, ref);
}

void eZapMain::getServiceSelectorPath(eServicePath &path)
{
//	eDebug("selected = %s",eZap::getInstance()->getServiceSelector()->getSelected().toString().c_str() );

	path=eZap::getInstance()->getServiceSelector()->getPath();
	path.down(eZap::getInstance()->getServiceSelector()->getSelected());

//	eDebug("stored path for mode %d: %s", mode, eServicePath(path).toString().c_str());
}

eServiceContextMenu::eServiceContextMenu(const eServiceReference &ref): eListBoxWindow<eListBoxEntryText>(_("Service Menu"), 4), ref(ref)
{
	move(ePoint(150, 200));
	new eListBoxEntryText(&list, _("back"), (void*)0);
	new eListBoxEntryText(&list, _("delete service"), (void*)1);
	new eListBoxEntryText(&list, _("move service"), (void*)2);
	CONNECT(list.selected, eServiceContextMenu::entrySelected);
}

void eServiceContextMenu::entrySelected(eListBoxEntryText *test)
{
	if (!test)
		close(0);
	else
		close((int)test->getKey());
}

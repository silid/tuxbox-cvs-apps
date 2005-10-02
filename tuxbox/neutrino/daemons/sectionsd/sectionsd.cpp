//
//  $Id: sectionsd.cpp,v 1.193 2005/10/02 21:27:58 mogway Exp $
//
//	sectionsd.cpp (network daemon for SI-sections)
//	(dbox-II-project)
//
//	Copyright (C) 2001 by fnbrd
//
//    Homepage: http://dbox2.elxsi.de
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//
//

#include <malloc.h>
#include <dmx.h>
#include <dmxapi.h>
#include <debug.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
//#include <sys/resource.h> // getrusage
#include <set>
#include <map>
#include <algorithm>
#include <string>

#include <sys/wait.h>
#include <sys/time.h>

#include <connection/basicsocket.h>
#include <connection/basicserver.h>

// Daher nehmen wir SmartPointers aus der Boost-Lib (www.boost.org)
#include <boost/shared_ptr.hpp>

#include <sectionsdclient/sectionsdMsg.h>
#include <sectionsdclient/sectionsdclient.h>
#include <eventserver.h>

#include "SIutils.hpp"
#include "SIservices.hpp"
#include "SIevents.hpp"
#include "SIsections.hpp"


/* please use the same define status as in dmx.cpp! */
#define PAUSE_EQUALS_STOP 1



//#include "timerdclient.h"
//#include "../timermanager.h"

// 60 Minuten Zyklus...
// #define TIME_EIT_SCHEDULED_PAUSE 60* 60
// -- 5 Minutes max. pause should improve behavior  (rasc, 2005-05-02)
#define TIME_EIT_SCHEDULED_PAUSE 5* 60
// Zeit die fuer die gewartet wird, bevor der Filter weitergeschaltet wird, falls es automatisch nicht klappt
#define TIME_EIT_SKIPPING 30

// 12h Pause f�r SDT
//#define TIME_SDT_SCHEDULED_PAUSE 12* 60* 60
// -- shorter time for pause should  result in better behavior  (rasc, 2005-05-02)
#define TIME_SDT_SCHEDULED_PAUSE 2* 60* 60
#define TIME_SDT_SKIPPING 5


// Timeout bei tcp/ip connections in ms
#define READ_TIMEOUT_IN_SECONDS  2
#define WRITE_TIMEOUT_IN_SECONDS 2

// Gibt die Anzahl Timeouts an, nach der die Verbindung zum DMX neu gestartet wird (wegen evtl. buffer overflow)
#define RESTART_DMX_AFTER_TIMEOUTS 5

// Gibt die Anzahl Timeouts an, nach der �berpr�ft wird, ob die Timeouts von einem Sender ohne EIT kommen oder nicht
#define CHECK_RESTART_DMX_AFTER_TIMEOUTS 3

// Wieviele Sekunden EPG gecached werden sollen
//static long secondsToCache=4*24*60L*60L; // 4 Tage - weniger Prozessorlast?!
static long secondsToCache = 21*24*60L*60L; // 21 Tage - Prozessorlast <3% (rasc)
// Ab wann ein Event als alt gilt (in Sekunden)
//static long oldEventsAre = 60*60L; // 1h
static long oldEventsAre = 180*60L; // 3h  (sometimes want to know something about current/last movie)
static int scanning = 1;


// EVENTS...

CEventServer *eventServer;
//CTimerdClient   *timerdClient;
//bool            timerd = false;

static pthread_mutex_t eventsLock = PTHREAD_MUTEX_INITIALIZER; // Unsere (fast-)mutex, damit nicht gleichzeitig in die Menge events geschrieben und gelesen wird
static pthread_mutex_t servicesLock = PTHREAD_MUTEX_INITIALIZER; // Unsere (fast-)mutex, damit nicht gleichzeitig in die Menge services geschrieben und gelesen wird
static pthread_mutex_t messagingLock = PTHREAD_MUTEX_INITIALIZER;

// k.A. ob volatile im Kampf gegen Bugs trotz mutex's was bringt,
// falsch ist es zumindest nicht
/*
static DMX dmxEIT(0x12, 0x4f, (0xff- 0x01), 0x50, (0xff- 0x0f), 256);
static DMX dmxSDT(0x11, 0x42, 0xff, 0x42, 0xff, 256);
*/
static DMX dmxEIT(0x12, 256);
static DMX dmxSDT(0x11, 256);

// Houdini: added for Premiere Private EPG section for Sport/Direkt Portal
static DMX dmxPPT(0x00, 256);
unsigned int privatePid=0;

inline void lockServices(void)
{
	pthread_mutex_lock(&servicesLock);
}

inline void unlockServices(void)
{
	pthread_mutex_unlock(&servicesLock);
}

inline void lockMessaging(void)
{
	pthread_mutex_lock(&messagingLock);
}

inline void unlockMessaging(void)
{
	pthread_mutex_unlock(&messagingLock);
}

inline void lockEvents(void)
{
	pthread_mutex_lock(&eventsLock);
}

inline void unlockEvents(void)
{
	pthread_mutex_unlock(&eventsLock);
}

inline int EITThreadsPause(void)
{
	return(	dmxEIT.pause() || 
		dmxPPT.pause());
}

inline int EITThreadsUnPause(void)
{
	return(	dmxEIT.unpause() || 
		dmxPPT.unpause());
}

static long long last_profile_call;

void showProfiling( std::string text )
{

	struct timeval tv;

	gettimeofday( &tv, NULL );
	long long now = (long long) tv.tv_usec + (long long)((long long) tv.tv_sec * (long long) 1000000);


	dprintf("--> '%s' %f\n", text.c_str(), (now - last_profile_call) / 1000.);
	last_profile_call = now;
}

bool timeset = false;
static const SIevent nullEvt; // Null-Event

//------------------------------------------------------------
// Wir verwalten die events in SmartPointers
// und nutzen verschieden sortierte Menge zum Zugriff
//------------------------------------------------------------

// SmartPointer auf SIevent
//typedef Loki::SmartPtr<class SIevent, Loki::RefCounted, Loki::DisallowConversion, Loki::NoCheck>
//  SIeventPtr;
typedef boost::shared_ptr<class SIevent>
SIeventPtr;

typedef std::map<event_id_t, SIeventPtr, std::less<event_id_t> > MySIeventsOrderUniqueKey;
static MySIeventsOrderUniqueKey mySIeventsOrderUniqueKey;

// Mengen mit SIeventPtr sortiert nach Event-ID fuer NVOD-Events (mehrere Zeiten)
static MySIeventsOrderUniqueKey mySIeventsNVODorderUniqueKey;

struct OrderServiceUniqueKeyFirstStartTimeEventUniqueKey
{
	bool operator()(const SIeventPtr &p1, const SIeventPtr &p2)
	{
		return
		    (p1->get_channel_id() == p2->get_channel_id()) ?
		    (p1->times.begin()->startzeit == p2->times.begin()->startzeit ? p1->eventID < p2->eventID : p1->times.begin()->startzeit < p2->times.begin()->startzeit )
		    :
		    (p1->get_channel_id() < p2->get_channel_id());
	}
};

typedef std::set<SIeventPtr, OrderServiceUniqueKeyFirstStartTimeEventUniqueKey > MySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey;
static MySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey;

struct OrderFirstEndTimeServiceIDEventUniqueKey
{
	bool operator()(const SIeventPtr &p1, const SIeventPtr &p2)
	{
		return
		    p1->times.begin()->startzeit + (long)p1->times.begin()->dauer == p2->times.begin()->startzeit + (long)p2->times.begin()->dauer ?
		    //      ( p1->serviceID == p2->serviceID ? p1->uniqueKey() < p2->uniqueKey() : p1->serviceID < p2->serviceID )
		    (p1->service_id == p2->service_id ? p1->uniqueKey() > p2->uniqueKey() : p1->service_id < p2->service_id)
				    :
				    ( p1->times.begin()->startzeit + (long)p1->times.begin()->dauer < p2->times.begin()->startzeit + (long)p2->times.begin()->dauer ) ;
	}
};

typedef std::set<SIeventPtr, OrderFirstEndTimeServiceIDEventUniqueKey > MySIeventsOrderFirstEndTimeServiceIDEventUniqueKey;
static MySIeventsOrderFirstEndTimeServiceIDEventUniqueKey mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey;

// Hier landen alle Service-Ids von Meta-Events inkl. der zugehoerigen Event-ID (nvod)
// d.h. key ist der Unique Service-Key des Meta-Events und Data ist der unique Event-Key
typedef std::map<t_channel_id, event_id_t, std::less<t_channel_id> > MySIeventUniqueKeysMetaOrderServiceUniqueKey;
static MySIeventUniqueKeysMetaOrderServiceUniqueKey mySIeventUniqueKeysMetaOrderServiceUniqueKey;

/*
class NvodSubEvent {
  public:
    NvodSubEvent() {
      uniqueServiceID=0;
      uniqueEventID=0;
    }
    NvodSubEvent(const NvodSubEvent &n) {
      uniqueServiceID=n.uniqueServiceID;
      uniqueEventID=n.uniqueEventID;
    }
    t_channel_id uniqueServiceID;
    event_id_t   uniqueMetaEventID; // ID des Meta-Events
    event_id_t   uniqueMetaEventID; // ID des eigentlichen Events
};

// Menge sortiert nach Meta-ServiceIDs (NVODs)
typedef std::multimap<t_channel_id, class NvodSubEvent *, std::less<t_channel_id> > nvodSubEvents;
*/

// Loescht ein Event aus allen Mengen
static bool deleteEvent(const event_id_t uniqueKey)
{
	MySIeventsOrderUniqueKey::iterator e = mySIeventsOrderUniqueKey.find(uniqueKey);

	if (e != mySIeventsOrderUniqueKey.end())
	{
		if (e->second->times.size())
		{
			mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.erase(e->second);
			mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.erase(e->second);
		}

		mySIeventsOrderUniqueKey.erase(uniqueKey);
		mySIeventsNVODorderUniqueKey.erase(uniqueKey);
		return true;
	}
	else
		return false;

	/*
	  for(MySIeventIDsMetaOrderServiceID::iterator i=mySIeventIDsMetaOrderServiceID.begin(); i!=mySIeventIDsMetaOrderServiceID.end(); i++)
	    if(i->second==eventID)
	      mySIeventIDsMetaOrderServiceID.erase(i);
	*/
}

// Fuegt ein Event in alle Mengen ein
static void addEvent(const SIevent &evt)
{
	SIevent *eptr = new SIevent(evt);

	if (!eptr)
	{
		printf("[sectionsd::addEvent] new SIevent failed.\n");
		throw std::bad_alloc();
	}

	SIeventPtr e(eptr);

	// Damit in den nicht nach Event-ID sortierten Mengen
	// Mehrere Events mit gleicher ID sind, diese vorher loeschen
	deleteEvent(e->uniqueKey());

	// Pruefen ob es ein Meta-Event ist
	MySIeventUniqueKeysMetaOrderServiceUniqueKey::iterator i = mySIeventUniqueKeysMetaOrderServiceUniqueKey.find(e->get_channel_id());

	if (i != mySIeventUniqueKeysMetaOrderServiceUniqueKey.end())
	{
		// ist ein MetaEvent, d.h. mit Zeiten fuer NVOD-Event

		if (e->times.size())
		{
			// D.h. wir fuegen die Zeiten in das richtige Event ein
			MySIeventsOrderUniqueKey::iterator ie = mySIeventsOrderUniqueKey.find(i->second);

			if (ie != mySIeventsOrderUniqueKey.end())
			{
				// Event vorhanden
				// Falls das Event in den beiden Mengen mit Zeiten nicht vorhanden
				// ist, dieses dort einfuegen
				MySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey::iterator i2 = mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.find(ie->second);

				if (i2 == mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.end())
				{
					// nicht vorhanden -> einfuegen
					mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.insert(ie->second);
					mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.insert(ie->second);

				}

				// Und die Zeiten im Event updaten
				ie->second->times.insert(e->times.begin(), e->times.end());
			}
		}
	}

	// normales Event
	mySIeventsOrderUniqueKey.insert(std::make_pair(e->uniqueKey(), e));

	if (e->times.size())
	{
		// diese beiden Mengen enthalten nur Events mit Zeiten
		mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.insert(e);
		mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.insert(e);

	}
}

static void addNVODevent(const SIevent &evt)
{
	SIevent *eptr = new SIevent(evt);

	if (!eptr)
	{
		printf("[sectionsd::addNVODevent] new SIevent failed.\n");
		throw std::bad_alloc();
	}

	SIeventPtr e(eptr);

	MySIeventsOrderUniqueKey::iterator e2 = mySIeventsOrderUniqueKey.find(e->uniqueKey());

	if (e2 != mySIeventsOrderUniqueKey.end())
	{
		// bisher gespeicherte Zeiten retten
		e->times.insert(e2->second->times.begin(), e2->second->times.end());
	}

	// Damit in den nicht nach Event-ID sortierten Mengen
	// mehrere Events mit gleicher ID sind, diese vorher loeschen
	deleteEvent(e->uniqueKey());

	mySIeventsOrderUniqueKey.insert(std::make_pair(e->uniqueKey(), e));

	mySIeventsNVODorderUniqueKey.insert(std::make_pair(e->uniqueKey(), e));

	if (e->times.size())
	{
		// diese beiden Mengen enthalten nur Events mit Zeiten
		mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.insert(e);
		mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.insert(e);
	}
}

#if 0
static void removeNewEvents(void)
{
	// Alte events loeschen
	time_t zeit = time(NULL);

	// Mal umgekehrt wandern

	for (MySIeventsOrderFirstEndTimeServiceIDEventUniqueKey::iterator e = mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.begin(); e != mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.end(); e++)
		if ((*e)->times.begin()->startzeit > zeit + secondsToCache)
			deleteEvent((*e)->uniqueKey());

	return ;
}
#endif

static void removeOldEvents(const long seconds)
{
   bool goodtimefound;

   // Alte events loeschen
   time_t zeit = time(NULL);

   for (MySIeventsOrderFirstEndTimeServiceIDEventUniqueKey::iterator e = mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.begin(); e != mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.end(); e++) {

      goodtimefound = false;
      for (SItimes::iterator t = (*e)->times.begin(); t != (*e)->times.end(); t++)
      {
         if (t->startzeit + (long)t->dauer >= zeit - seconds) {
            goodtimefound=true;
            // one time found -> exit times loop
            break;
         }
      }
      if (false == goodtimefound)
         deleteEvent((*e)->uniqueKey());
      else
;//solange das nicht richtig funktioniert einfach bis zum ende suchen
//         break; // sortiert nach Endzeit, daher weiteres Suchen unnoetig
   }
   return ;
}

//  SIservicePtr;
typedef boost::shared_ptr<class SIservice>
SIservicePtr;

typedef std::map<t_channel_id, SIservicePtr, std::less<t_channel_id> > MySIservicesOrderUniqueKey;
static MySIservicesOrderUniqueKey mySIservicesOrderUniqueKey;

typedef std::map<t_channel_id, SIservicePtr, std::less<t_channel_id> > MySIservicesNVODorderUniqueKey;
static MySIservicesNVODorderUniqueKey mySIservicesNVODorderUniqueKey;

// Hier sollte man die hash-funktion fuer strings der stl benutzen
// Muss mal schauen ob es die auch fuer 'ignore_case' gibt

struct OrderServiceName
{
	bool operator()(const SIservicePtr &p1, const SIservicePtr &p2)
	{
		return strcasecmp(p1->serviceName.c_str(), p2->serviceName.c_str()) < 0;
	}
};

typedef std::set<SIservicePtr, OrderServiceName > MySIservicesOrderServiceName;
static MySIservicesOrderServiceName mySIservicesOrderServiceName;

// Fuegt ein Service in alle Mengen ein
static void addService(const SIservice &s)
{
	SIservice *sp = new SIservice(s);

	if (!sp)
	{
		printf("[sectionsd::addService] new SIservice failed.\n");
		throw std::bad_alloc();
	}

	SIservicePtr sptr(sp);

	// Controlcodes entfernen
	char servicename[50];

	strncpy(servicename, sptr->serviceName.c_str(), sizeof(servicename) - 1);

	servicename[sizeof(servicename) - 1] = 0;

	removeControlCodes(servicename);

	sptr->serviceName = servicename;

	mySIservicesOrderUniqueKey.insert(std::make_pair(sptr->uniqueKey(), sptr));

	if (sptr->nvods.size())
		mySIservicesNVODorderUniqueKey.insert(std::make_pair(sptr->uniqueKey(), sptr));

	//  if(sptr->serviceID==0x01 || sptr->serviceID==0x02 || sptr->serviceID==0x04)
	mySIservicesOrderServiceName.insert(sptr);
}

/*
 *
 * communication with sectionsdclient:
 *
 */

inline bool readNbytes(int fd, char *buf, const size_t numberOfBytes, const time_t timeoutInSeconds)
{
	timeval timeout;
	timeout.tv_sec  = timeoutInSeconds;
	timeout.tv_usec = 0;
	return receive_data(fd, buf, numberOfBytes, timeout);
}

inline bool writeNbytes(int fd, const char *buf,  const size_t numberOfBytes, const time_t timeoutInSeconds)
{
	timeval timeout;
	timeout.tv_sec  = timeoutInSeconds;
	timeout.tv_usec = 0;
	return send_data(fd, buf, numberOfBytes, timeout);
}


//------------------------------------------------------------
// misc. functions
//------------------------------------------------------------

static t_channel_id findServiceUniqueKeyforServiceName(const char * const serviceName)
{
	SIservice *sp = new SIservice(0, 0, 0);

	if (!sp)
	{
		printf("[sectionsd::findServiceUniqueKeyforServiceName] new SIservice failed.\n");
		throw std::bad_alloc();
	}

	SIservicePtr s(sp);

	s->serviceName = serviceName;

	dprintf("Search for Service '%s'\n", serviceName);

	MySIservicesOrderServiceName::iterator si = mySIservicesOrderServiceName.find(s);

	if (si != mySIservicesOrderServiceName.end())
		return (*si)->uniqueKey();

	dputs("Service not found");

	return 0;
}

static const SIevent& findSIeventForEventUniqueKey(const event_id_t eventUniqueKey)
{
	// Event (eventid) suchen
	MySIeventsOrderUniqueKey::iterator e = mySIeventsOrderUniqueKey.find(eventUniqueKey);

	if (e != mySIeventsOrderUniqueKey.end())
		return *(e->second);

	return nullEvt;
}

static const SIevent& findActualSIeventForServiceUniqueKey(const t_channel_id serviceUniqueKey, SItime& zeit, long plusminus = 0, unsigned *flag = 0)
{
	time_t azeit = time(NULL);

	if (flag != 0)
		*flag = 0;

	for (MySIeventsOrderFirstEndTimeServiceIDEventUniqueKey::iterator e = mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.begin(); e != mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.end(); e++)
		if ((*e)->get_channel_id() == serviceUniqueKey)
		{
			if (flag != 0)
				*flag |= CSectionsdClient::epgflags::has_anything; // �berhaupt was da...

			for (SItimes::reverse_iterator t = (*e)->times.rend(); t != (*e)->times.rbegin(); t--)
				if ((long)(azeit + plusminus) < (long)(t->startzeit + t->dauer))
				{
					if (flag != 0)
						*flag |= CSectionsdClient::epgflags::has_later; // sp�tere events da...

					if (t->startzeit <= (long)(azeit + plusminus))
					{
						//printf("azeit %d, startzeit+t->dauer %d \n", azeit, (long)(t->startzeit+t->dauer) );

						if (flag != 0)
							*flag |= CSectionsdClient::epgflags::has_current; // aktuelles event da...

						zeit = *t;

						return *(*e);
					}
				}
		}

	return nullEvt;
}

static const SIevent& findNextSIeventForServiceUniqueKey(const t_channel_id serviceUniqueKey, SItime& zeit)
{
	time_t azeit = time(NULL);

	for (MySIeventsOrderFirstEndTimeServiceIDEventUniqueKey::iterator e = mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.begin(); e != mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.end(); e++)
		if ((*e)->get_channel_id() == serviceUniqueKey)
		{
			for (SItimes::iterator t = (*e)->times.begin(); t != (*e)->times.end(); t++)
				if ((long)(azeit) < (long)(t->startzeit + t->dauer))
				{
					zeit = *t;
					return *(*e);
				}
		}

	return nullEvt;
}


static const SIevent &findActualSIeventForServiceName(const char * const serviceName, SItime& zeit)
{
	t_channel_id serviceUniqueKey = findServiceUniqueKeyforServiceName(serviceName);

	if (serviceUniqueKey)
		return findActualSIeventForServiceUniqueKey(serviceUniqueKey, zeit);

	return nullEvt;
}



// Sucht das naechste Event anhand unique key und Startzeit
static const SIevent &findNextSIevent(const event_id_t uniqueKey, SItime &zeit)
{
	MySIeventsOrderUniqueKey::iterator eFirst = mySIeventsOrderUniqueKey.find(uniqueKey);

	if (eFirst != mySIeventsOrderUniqueKey.end())
	{
		if (eFirst->second->times.size() > 1)
		{
			// Wir haben ein NVOD-Event
			// d.h. wir suchen die aktuelle Zeit und nehmen die naechste davon, falls existent

			for (SItimes::iterator t = eFirst->second->times.begin(); t != eFirst->second->times.end(); t++)
				if (t->startzeit == zeit.startzeit)
				{
					t++;

					if (t != eFirst->second->times.end())
					{
						zeit = *t;
						return *(eFirst->second);
					}

					break; // ganz normal naechstes Event suchen
				}
		}

		MySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey::iterator eNext = mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.find(eFirst->second);
		eNext++;

		if (eNext != mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.end())
		{
			if ((*eNext)->get_channel_id() == eFirst->second->get_channel_id())
			{
				zeit = *((*eNext)->times.begin());
				return *(*eNext);
			}
			else
				return nullEvt;
		}
	}

	return nullEvt;
}

// Sucht das naechste UND vorhergehende Event anhand unique key und Startzeit
static void findPrevNextSIevent(const event_id_t uniqueKey, SItime &zeit, SIevent &prev, SItime &prev_zeit, SIevent &next, SItime &next_zeit)
{
	prev = nullEvt;
	next = nullEvt;
	bool prev_ok = false;
	bool next_ok = false;

	MySIeventsOrderUniqueKey::iterator eFirst = mySIeventsOrderUniqueKey.find(uniqueKey);

	if (eFirst != mySIeventsOrderUniqueKey.end())
	{
		if (eFirst->second->times.size() > 1)
		{
			// Wir haben ein NVOD-Event
			// d.h. wir suchen die aktuelle Zeit und nehmen die naechste davon, falls existent

			for (SItimes::iterator t = eFirst->second->times.begin(); t != eFirst->second->times.end(); t++)
				if (t->startzeit == zeit.startzeit)
				{
					if (t != eFirst->second->times.begin())
					{
						t--;
						prev_zeit = *t;
						prev = *(eFirst->second);
						prev_ok = true;
						t++;
					}

					t++;

					if (t != eFirst->second->times.end())
					{
						next_zeit = *t;
						next = *(eFirst->second);
						next_ok = true;
					}

					if ( prev_ok && next_ok )
						return ; // beide gefunden...
					else
						break;
				}
		}

		MySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey::iterator eNext = mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.find(eFirst->second);

		if ( (!prev_ok) && (eNext != mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.begin() ) )
		{
			eNext--;

			if ((*eNext)->get_channel_id() == eFirst->second->get_channel_id())
			{
				prev_zeit = *((*eNext)->times.begin());
				prev = *(*eNext);
			}

			eNext++;
		}

		eNext++;

		if ( (!next_ok) && (eNext != mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.end()) )
		{
			if ((*eNext)->get_channel_id() == eFirst->second->get_channel_id())
			{
				next_zeit = *((*eNext)->times.begin());
				next = *(*eNext);
			}
		}

		// printf("evt_id >%llx<, time %x - evt_id >%llx<, time %x\n", prev.uniqueKey(), prev_zeit.startzeit, next.uniqueKey(), next_zeit.startzeit);
	}
}

//---------------------------------------------------------------------
//			connection-thread
// handles incoming requests
//---------------------------------------------------------------------

struct connectionData
{
	int connectionSocket;

	struct sockaddr_in clientAddr;
};

static void commandPauseScanning(int connfd, char *data, const unsigned dataLength)
{
	if (dataLength != 4)
		return ;

	int pause = *(int *)data;

	if (pause && pause != 1)
		return ;

	dprintf("Request of %s scanning.\n", pause ? "stop" : "continue" );

	if (scanning && pause)
	{
		dmxEIT.request_pause();
		dmxSDT.request_pause();
		dmxPPT.request_pause();
		scanning = 0;
	}
	else if (!pause && !scanning)
	{
		dmxSDT.request_unpause();
		dmxEIT.request_unpause();
		dmxPPT.request_unpause();
		scanning = 1;
	}

	struct sectionsd::msgResponseHeader msgResponse;

	msgResponse.dataLength = 0;

	writeNbytes(connfd, (const char *)&msgResponse, sizeof(msgResponse), WRITE_TIMEOUT_IN_SECONDS);

	return ;
}

static void commandGetIsScanningActive(int connfd, char* /*data*/, const unsigned /*dataLength*/)
{
	struct sectionsd::msgResponseHeader responseHeader;

	responseHeader.dataLength = sizeof(scanning);

	if (writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS) == true)
	{
		writeNbytes(connfd, (const char *)&scanning, responseHeader.dataLength, WRITE_TIMEOUT_IN_SECONDS);
	}
	else
		dputs("[sectionsd] Fehler/Timeout bei write");
}

static void commandPauseSorting(int connfd, char *data, const unsigned dataLength)
{
	if (dataLength != 4)
		return ;

	int pause = *(int *)data;

	if (pause && pause != 1)
		return ;

	dprintf("Request of %s sorting events.\n", pause ? "stop" : "continue" );

	if (pause)
		EITThreadsPause();
	else
		EITThreadsUnPause();

	struct sectionsd::msgResponseHeader msgResponse;

	msgResponse.dataLength = 0;

	writeNbytes(connfd, (const char *)&msgResponse, sizeof(msgResponse), WRITE_TIMEOUT_IN_SECONDS);

	return ;
}

static void commandDumpAllServices(int connfd, char* /*data*/, const unsigned /*dataLength*/)
{
	dputs("Request of service list.\n");

	char *serviceList = new char[65*1024]; // 65kb should be enough and dataLength is unsigned short

	if (!serviceList)
	{
		fprintf(stderr, "low on memory!\n");
		return ;
	}

	*serviceList = 0;
	lockServices();
	char daten[200];

	for (MySIservicesOrderServiceName::iterator s = mySIservicesOrderServiceName.begin(); s != mySIservicesOrderServiceName.end(); s++)
	{
		sprintf(daten,
			PRINTF_CHANNEL_ID_TYPE_NO_LEADING_ZEROS
			" %hu %hhu %d %d %d %d %u ",
		        (*s)->uniqueKey(),
		        (*s)->service_id, (*s)->serviceTyp,
		        (*s)->eitScheduleFlag(), (*s)->eitPresentFollowingFlag(),
		        (*s)->runningStatus(), (*s)->freeCAmode(),
		        (*s)->nvods.size());
		strcat(serviceList, daten);
		strcat(serviceList, "\n");
		strcat(serviceList, (*s)->serviceName.c_str());
		strcat(serviceList, "\n");
		strcat(serviceList, (*s)->providerName.c_str());
		strcat(serviceList, "\n");
	}

	unlockServices();
	if (strlen(serviceList) + 1 > 65*1024) 
		printf("warning: commandDumpAllServices: length=%d\n", strlen(serviceList) + 1);

	struct sectionsd::msgResponseHeader msgResponse;
	msgResponse.dataLength = strlen(serviceList) + 1;

	if (msgResponse.dataLength == 1)
		msgResponse.dataLength = 0;

	if (writeNbytes(connfd, (const char *)&msgResponse, sizeof(msgResponse), WRITE_TIMEOUT_IN_SECONDS) == true)
	{
		if (msgResponse.dataLength)
			writeNbytes(connfd, serviceList, msgResponse.dataLength, WRITE_TIMEOUT_IN_SECONDS);
	}
	else
		dputs("[sectionsd] Fehler/Timeout bei write");

	delete[] serviceList;

	return ;
}

static void commandSetEventsAreOldInMinutes(int connfd, char *data, const unsigned dataLength)
{
	if (dataLength != 2)
		return ;

	dprintf("Set events are old after minutes: %hd\n", *((unsigned short*)data));

	oldEventsAre = *((unsigned short*)data)*60L;

	struct sectionsd::msgResponseHeader responseHeader;

	responseHeader.dataLength = 0;

	writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS);

	return ;
}

static void commandSetHoursToCache(int connfd, char *data, const unsigned dataLength)
{
	if (dataLength != 2)
		return ;

	dprintf("Set hours to cache: %hd\n", *((unsigned short*)data));

	secondsToCache = *((unsigned short*)data)*60L*60L;

	struct sectionsd::msgResponseHeader responseHeader;

	responseHeader.dataLength = 0;

	writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS);

	return ;
}

static void sendAllEvents(int connfd, t_channel_id serviceUniqueKey, bool oldFormat = true )
{
	char *evtList = new char[65*1024]; // 65kb should be enough and dataLength is unsigned short

	if (!evtList)
	{
		fprintf(stderr, "low on memory!\n");
		return ;
	}

	dprintf("sendAllEvents for " PRINTF_CHANNEL_ID_TYPE "\n", serviceUniqueKey);
	*evtList = 0;
	char *liste = evtList;

	if (serviceUniqueKey != 0)
	{
		// service Found

		if (EITThreadsPause())
		{
			delete[] evtList;
			return ;
		}

		lockEvents();
		int serviceIDfound = 0;

		for (MySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey::iterator e = mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.begin(); e != mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.end(); e++)
		{
			if ((*e)->get_channel_id() == serviceUniqueKey)
			{
				serviceIDfound = 1;

				for (SItimes::iterator t = (*e)->times.begin(); t != (*e)->times.end(); t++)
				{
					if ( oldFormat )
					{
						char strZeit[50];
						sprintf(strZeit, "%012llx ", (*e)->uniqueKey());
						strcat(liste, strZeit);

						struct tm *tmZeit;
						tmZeit = localtime(&(t->startzeit));
						sprintf(strZeit, "%02d.%02d %02d:%02d %u ",
						        tmZeit->tm_mday, tmZeit->tm_mon + 1, tmZeit->tm_hour, tmZeit->tm_min, (*e)->times.begin()->dauer / 60);
						strcat(liste, strZeit);
						strcat(liste, (*e)->name.c_str());
						strcat(liste, "\n");
					}
					else
					{
						*((event_id_t *)liste) = (*e)->uniqueKey();
						liste += sizeof(event_id_t);
						*((unsigned *)liste) = t->startzeit;
						liste += 4;
						*((unsigned *)liste) = t->dauer;
						liste += 4;
						strcpy(liste, (*e)->name.c_str());
						liste += strlen(liste);
						liste++;

						if (((*e)->text).empty())
						{
							strcpy(liste, (*e)->extendedText.substr(0, 40).c_str());
							liste += strlen(liste);
						}
						else
						{
							strcpy(liste, (*e)->text.c_str());
							liste += strlen(liste);
						}

						liste++;
					}
				}
			} // if = serviceID
			else if ( serviceIDfound )
				break; // sind nach serviceID und startzeit sortiert -> nicht weiter suchen
		}

		unlockEvents();

		if (EITThreadsUnPause())
		{
			delete[] evtList;
			return ;
		}
	}

	struct sectionsd::msgResponseHeader responseHeader;

	if (liste - evtList > 65*1024) 
		printf("warning: [sectionsd] all events - response-size: 0x%x\n", liste - evtList);
	responseHeader.dataLength = liste - evtList;

	dprintf("[sectionsd] all events - response-size: 0x%x\n", responseHeader.dataLength);

	if ( responseHeader.dataLength == 1 )
		responseHeader.dataLength = 0;

	if (writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS) == true)
	{
		if (responseHeader.dataLength)
			writeNbytes(connfd, evtList, responseHeader.dataLength, WRITE_TIMEOUT_IN_SECONDS);
	}
	else
		dputs("[sectionsd] Fehler/Timeout bei write");

	delete[] evtList;

	return ;
}

static void commandAllEventsChannelName(int connfd, char *data, const unsigned dataLength)
{
	data[dataLength - 1] = 0; // to be sure it has an trailing 0
	dprintf("Request of all events for '%s'\n", data);
	lockServices();
	t_channel_id uniqueServiceKey = findServiceUniqueKeyforServiceName(data);
	unlockServices();
	sendAllEvents(connfd, uniqueServiceKey);
	return ;
}

static void commandAllEventsChannelID(int connfd, char *data, const unsigned dataLength)
{
	if (dataLength != sizeof(t_channel_id))
		return ;

	t_channel_id serviceUniqueKey = *(t_channel_id *)data;

	dprintf("Request of all events for " PRINTF_CHANNEL_ID_TYPE "\n", serviceUniqueKey);

	sendAllEvents(connfd, serviceUniqueKey, false);

	return ;
}

static void commandDumpStatusInformation(int connfd, char* /*data*/, const unsigned /*dataLength*/)
{
	dputs("Request of status information");

	lockEvents();

	unsigned anzEvents = mySIeventsOrderUniqueKey.size();

	unsigned anzNVODevents = mySIeventsNVODorderUniqueKey.size();

	unsigned anzMetaServices = mySIeventUniqueKeysMetaOrderServiceUniqueKey.size();

	unlockEvents();

	lockServices();

	unsigned anzServices = mySIservicesOrderUniqueKey.size();

	unsigned anzNVODservices = mySIservicesNVODorderUniqueKey.size();

	//  unsigned anzServices=services.size();
	unlockServices();

	struct mallinfo speicherinfo = mallinfo();

	//  struct rusage resourceUsage;
	//  getrusage(RUSAGE_CHILDREN, &resourceUsage);
	//  getrusage(RUSAGE_SELF, &resourceUsage);
	time_t zeit = time(NULL);

	char stati[2024];

	sprintf(stati,
	        "$Id: sectionsd.cpp,v 1.193 2005/10/02 21:27:58 mogway Exp $\n"
	        "Current time: %s"
	        "Hours to cache: %ld\n"
	        "Events are old %ldmin after their end time\n"
	        "Number of cached services: %u\n"
	        "Number of cached nvod-services: %u\n"
	        "Number of cached events: %u\n"
	        "Number of cached nvod-events: %u\n"
	        "Number of cached meta-services: %u\n"
	        //    "Resource-usage: maxrss: %ld ixrss: %ld idrss: %ld isrss: %ld\n"
	        "Total size of memory occupied by chunks handed out by malloc: %d\n"
	        "Total bytes memory allocated with `sbrk' by malloc, in bytes: %d (%dkb, %.2fMB)\n",
	        ctime(&zeit),
	        secondsToCache / (60*60L), oldEventsAre / 60, anzServices, anzNVODservices, anzEvents, anzNVODevents, anzMetaServices,
	        //    resourceUsage.ru_maxrss, resourceUsage.ru_ixrss, resourceUsage.ru_idrss, resourceUsage.ru_isrss,
	        speicherinfo.uordblks,
	        speicherinfo.arena, speicherinfo.arena / 1024, (float)speicherinfo.arena / (1024.*1024.)
	       );

	struct sectionsd::msgResponseHeader responseHeader;

	responseHeader.dataLength = strlen(stati) + 1;

	if (writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS) == true)
	{
		if (responseHeader.dataLength)
			writeNbytes(connfd, stati, responseHeader.dataLength, WRITE_TIMEOUT_IN_SECONDS);
	}
	else
		dputs("[sectionsd] Fehler/Timeout bei write");

	return ;
}

static void commandCurrentNextInfoChannelName(int connfd, char *data, const unsigned dataLength)
{
	int nResultDataSize = 0;
	char* pResultData = 0;

	data[dataLength - 1] = 0; // to be sure it has an trailing 0
	dprintf("Request of current/next information for '%s'\n", data);

	if (EITThreadsPause()) // -> lock
		return ;

	lockServices();

	lockEvents();

	SItime zeitEvt1(0, 0);

	const SIevent &evt = findActualSIeventForServiceName(data, zeitEvt1);

	unlockServices();

	if (evt.service_id != 0)
	{ //Found
		dprintf("current EPG found.\n");
		SItime zeitEvt2(zeitEvt1);
		const SIevent &nextEvt = findNextSIevent(evt.uniqueKey(), zeitEvt2);

		if (nextEvt.service_id != 0)
		{
			dprintf("next EPG found.\n");
			// Folgendes ist grauenvoll, habs aber einfach kopiert aus epgd
			// und keine Lust das grossartig zu verschoenern
			nResultDataSize =
			    12 + 1 + 					// Unique-Key + del
			    strlen(evt.name.c_str()) + 1 + 		//Name + del
			    3 + 2 + 1 + 					//std:min + del
			    4 + 1 + 					//dauer (mmmm) + del
			    3 + 1 + 					//100 + del
			    12 + 1 + 					// Unique-Key + del
			    strlen(nextEvt.name.c_str()) + 1 + 		//Name + del
			    3 + 2 + 1 + 					//std:min + del
			    4 + 1 + 1;					//dauer (mmmm) + del + 0
			pResultData = new char[nResultDataSize];

			if (!pResultData)
			{
				fprintf(stderr, "low on memory!\n");
				unlockEvents();
				dmxEIT.unpause();
				EITThreadsUnPause();
				return ;
			}

			struct tm *pStartZeit = localtime(&zeitEvt1.startzeit);

			int nSH(pStartZeit->tm_hour), nSM(pStartZeit->tm_min);

			unsigned dauer = zeitEvt1.dauer / 60;

			unsigned nProcentagePassed = (unsigned)((float)(time(NULL) - zeitEvt1.startzeit) / (float)zeitEvt1.dauer * 100.);

			pStartZeit = localtime(&zeitEvt2.startzeit);

			int nSH2(pStartZeit->tm_hour), nSM2(pStartZeit->tm_min);

			unsigned dauer2 = zeitEvt2.dauer / 60;

			sprintf(pResultData,
			        "%012llx\n%s\n%02d:%02d\n%04u\n%03u\n%012llx\n%s\n%02d:%02d\n%04u\n",
			        evt.uniqueKey(),
			        evt.name.c_str(),
			        nSH, nSM, dauer, nProcentagePassed,
			        nextEvt.uniqueKey(),
			        nextEvt.name.c_str(),
			        nSH2, nSM2, dauer2);
		}
	}

	unlockEvents();
	EITThreadsUnPause(); // -> unlock
	
	// response

	struct sectionsd::msgResponseHeader pmResponse;
	pmResponse.dataLength = nResultDataSize;
	bool rc = writeNbytes(connfd, (const char *)&pmResponse, sizeof(pmResponse), WRITE_TIMEOUT_IN_SECONDS);

	if ( nResultDataSize > 0 )
	{
		if (rc == true)
			writeNbytes(connfd, pResultData, nResultDataSize, WRITE_TIMEOUT_IN_SECONDS);
		else
			dputs("[sectionsd] Fehler/Timeout bei write");

		delete[] pResultData;
	}
	else
	{
		dprintf("current/next EPG not found!\n");
	}

	return ;
}

static void commandComponentTagsUniqueKey(int connfd, char *data, const unsigned dataLength)
{
	int nResultDataSize = 0;
	char* pResultData = 0;

	if (dataLength != 8)
		return ;

	event_id_t uniqueKey = *(event_id_t *)data;

	dprintf("Request of ComponentTags for 0x%llx\n", uniqueKey);

	if (EITThreadsPause()) // -> lock
		return ;

	lockEvents();

	nResultDataSize = sizeof(int);    // num. Component-Tags

	MySIeventsOrderUniqueKey::iterator eFirst = mySIeventsOrderUniqueKey.find(uniqueKey);

	if (eFirst != mySIeventsOrderUniqueKey.end())
	{
		//Found
		dprintf("ComponentTags found.\n");
		dprintf("components.size %d \n", eFirst->second->components.size());

		for (SIcomponents::iterator cmp = eFirst->second->components.begin(); cmp != eFirst->second->components.end(); cmp++)
		{
			dprintf(" %s \n", cmp->component.c_str());
			nResultDataSize += strlen(cmp->component.c_str()) + 1 +  	// name
			                   sizeof(unsigned char) +  //componentType
			                   sizeof(unsigned char) +  //componentTag
			                   sizeof(unsigned char); //streamContent
		}
	}

	pResultData = new char[nResultDataSize];

	if (!pResultData)
	{
		fprintf(stderr, "low on memory!\n");
		unlockEvents();
		EITThreadsUnPause();
		return ;
	}

	char *p = pResultData;

	if (eFirst != mySIeventsOrderUniqueKey.end())
	{
		*((int *)p) = eFirst->second->components.size();
		p += sizeof(int);

		for (SIcomponents::iterator cmp = eFirst->second->components.begin(); cmp != eFirst->second->components.end(); cmp++)
		{

			strcpy(p, cmp->component.c_str());
			p += strlen(cmp->component.c_str()) + 1;
			*((unsigned char *)p) = cmp->componentType;
			p += sizeof(unsigned char);
			*((unsigned char *)p) = cmp->componentTag;
			p += sizeof(unsigned char);
			*((unsigned char *)p) = cmp->streamContent;
			p += sizeof(unsigned char);
		}
	}
	else
	{
		*((int *)p) = 0;
		p += sizeof(int);
	}

	unlockEvents();
	EITThreadsUnPause(); // -> unlock

	struct sectionsd::msgResponseHeader responseHeader;
	responseHeader.dataLength = nResultDataSize;

	if (writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS) == true)
	{
		if (responseHeader.dataLength)
			writeNbytes(connfd, pResultData, responseHeader.dataLength, WRITE_TIMEOUT_IN_SECONDS);
	}
	else
		dputs("[sectionsd] Fehler/Timeout bei write");

	delete[] pResultData;

	return ;
}

static void commandLinkageDescriptorsUniqueKey(int connfd, char *data, const unsigned dataLength)
{
	int nResultDataSize = 0;
	char* pResultData = 0;

	if (dataLength != 8)
		return ;

	event_id_t uniqueKey = *(event_id_t *)data;

	dprintf("Request of LinkageDescriptors for 0x%llx\n", uniqueKey);

	if (EITThreadsPause()) // -> lock
		return ;

	lockEvents();

	nResultDataSize = sizeof(int);    // num. Component-Tags

	int countDescs = 0;

	MySIeventsOrderUniqueKey::iterator eFirst = mySIeventsOrderUniqueKey.find(uniqueKey);

	if (eFirst != mySIeventsOrderUniqueKey.end())
	{
		//Found
		dprintf("LinkageDescriptors found.\n");
		dprintf("linkage_descs.size %d \n", eFirst->second->linkage_descs.size());


		for (SIlinkage_descs::iterator linkage_desc = eFirst->second->linkage_descs.begin(); linkage_desc != eFirst->second->linkage_descs.end(); linkage_desc++)
		{
			if (linkage_desc->linkageType == 0xB0)
			{
				countDescs++;
				dprintf(" %s \n", linkage_desc->name.c_str());
				nResultDataSize += strlen(linkage_desc->name.c_str()) + 1 +  	// name
				                   sizeof(t_transport_stream_id) +  //transportStreamId
				                   sizeof(t_original_network_id) +  //originalNetworkId
				                   sizeof(t_service_id); //serviceId
			}
		}
	}

	pResultData = new char[nResultDataSize];

	if (!pResultData)
	{
		fprintf(stderr, "low on memory!\n");
		unlockEvents();
		EITThreadsUnPause();
		return ;
	}

	char *p = pResultData;

	*((int *)p) = countDescs;
	p += sizeof(int);

	if (eFirst != mySIeventsOrderUniqueKey.end())
	{
		for (SIlinkage_descs::iterator linkage_desc = eFirst->second->linkage_descs.begin(); linkage_desc != eFirst->second->linkage_descs.end(); linkage_desc++)
		{
			if (linkage_desc->linkageType == 0xB0)
			{
				strcpy(p, linkage_desc->name.c_str());
				p += strlen(linkage_desc->name.c_str()) + 1;
				*((t_transport_stream_id *)p) = linkage_desc->transportStreamId;
				p += sizeof(t_transport_stream_id);
				*((t_original_network_id *)p) = linkage_desc->originalNetworkId;
				p += sizeof(t_original_network_id);
				*((t_service_id *)p) = linkage_desc->serviceId;
				p += sizeof(t_service_id);
			}
		}
	}

	unlockEvents();
	EITThreadsUnPause(); // -> unlock

	struct sectionsd::msgResponseHeader responseHeader;
	responseHeader.dataLength = nResultDataSize;

	if (writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader),  WRITE_TIMEOUT_IN_SECONDS) == true)
	{
		if (responseHeader.dataLength)
			writeNbytes(connfd, pResultData, responseHeader.dataLength, WRITE_TIMEOUT_IN_SECONDS);
	}
	else
		dputs("[sectionsd] Fehler/Timeout bei write");

	delete[] pResultData;

	return ;
}

static t_channel_id	messaging_current_servicekey = 0;
std::vector<long long>	messaging_skipped_sections_ID [0x22];			// 0x4e .. 0x6f
static long long 	messaging_sections_max_ID [0x22];			// 0x4e .. 0x6f
static int 		messaging_sections_got_all [0x22];			// 0x4e .. 0x6f

std::vector<long long>	messaging_sdt_skipped_sections_ID [2];			// 0x42, 0x46
static long long 	messaging_sdt_sections_max_ID [2];			// 0x42, 0x46
static int 		messaging_sdt_sections_got_all [2];			// 0x42, 0x46

static bool	messaging_wants_current_next_Event = false;
static time_t	messaging_last_requested = time(NULL);
static bool	messaging_neutrino_sets_time = false;
static bool 	messaging_WaitForServiceDesc = false;

static void commandserviceChanged(int connfd, char *data, const unsigned dataLength)
{

	if (dataLength != sizeof(sectionsd::commandSetServiceChanged))
		return;

	t_channel_id * uniqueServiceKey = &(((sectionsd::commandSetServiceChanged *)data)->channel_id);
	bool         * requestCN_Event  = &(((sectionsd::commandSetServiceChanged *)data)->requestEvent);

	bool doWakeUp = false;

	dprintf("[sectionsd] commandserviceChanged: Service changed to " PRINTF_CHANNEL_ID_TYPE "\n", *uniqueServiceKey);

	showProfiling("[sectionsd] commandserviceChanged: before messaging lock");

	time_t zeit = time(NULL);

	lockMessaging();


	if ( ( messaging_current_servicekey != *uniqueServiceKey ) ||
	        ( zeit > ( messaging_last_requested + 5 ) ) )
	{
		messaging_current_servicekey = *uniqueServiceKey;

		for ( int i = 0x4e; i <= 0x6f; i++)
		{
			messaging_skipped_sections_ID[i - 0x4e].clear();
			messaging_sections_max_ID[i - 0x4e] = -1;
			messaging_sections_got_all[i - 0x4e] = false;
		}

		for ( int i = 0; i <= 1; i++)
		{
			messaging_sdt_skipped_sections_ID[i].clear();
			messaging_sdt_sections_max_ID[i] = -1;
			messaging_sdt_sections_got_all[i] = false;
		}

		doWakeUp = true;


		lockServices();

		MySIservicesOrderUniqueKey::iterator si = mySIservicesOrderUniqueKey.end();
		si = mySIservicesOrderUniqueKey.find(*uniqueServiceKey);

		messaging_WaitForServiceDesc = (si == mySIservicesOrderUniqueKey.end() );
		if ( messaging_WaitForServiceDesc )
			dputs("[sectionsd] commandserviceChanged: current service-descriptor not loaded yet!" );

		unlockServices();
	}



	if ( ( !doWakeUp ) && ( messaging_sections_got_all[0] ) && ( *requestCN_Event ) && ( !messaging_WaitForServiceDesc ) )
	{
		messaging_wants_current_next_Event = false;
		messaging_WaitForServiceDesc = false;
		eventServer->sendEvent(CSectionsdClient::EVT_GOT_CN_EPG, CEventServer::INITID_SECTIONSD, &messaging_current_servicekey, sizeof(messaging_current_servicekey) );
	}
	else
	{
		if ( messaging_current_servicekey != *uniqueServiceKey )
		{
			messaging_wants_current_next_Event = *requestCN_Event;
		}
		else if ( *requestCN_Event )
			messaging_wants_current_next_Event = true;

		if ( messaging_WaitForServiceDesc )
			messaging_wants_current_next_Event = false;

		if (messaging_wants_current_next_Event)
			dprintf("[sectionsd] commandserviceChanged: requesting current_next event...\n");
	}

	showProfiling("[sectionsd] commandserviceChanged: before wakeup");
	messaging_last_requested = zeit;

	if ( doWakeUp )
	{
		// nur wenn lange genug her, oder wenn anderer Service :)
		dmxEIT.change( 0 );
		dmxSDT.change( 0 );
	}
	else
		dprintf("[sectionsd] commandserviceChanged: ignoring wakeup request...\n");

	unlockMessaging();

	showProfiling("[sectionsd] commandserviceChanged: after doWakeup");

	struct sectionsd::msgResponseHeader msgResponse;

	msgResponse.dataLength = 0;

	writeNbytes(connfd, (const char *)&msgResponse, sizeof(msgResponse), WRITE_TIMEOUT_IN_SECONDS);

	return ;
}

static void commandCurrentNextInfoChannelID(int connfd, char *data, const unsigned dataLength)
{
	int nResultDataSize = 0;
	char* pResultData = 0;

	if (dataLength != sizeof(t_channel_id))
		return ;

	t_channel_id * uniqueServiceKey = (t_channel_id *)data;

	dprintf("[sectionsd] Request of current/next information for " PRINTF_CHANNEL_ID_TYPE "\n", *uniqueServiceKey);

	if (EITThreadsPause()) // -> lock
		return ;

	lockServices();

	lockEvents();

	SItime zeitEvt1(0, 0);

	unsigned flag = 0;

	const SIevent &evt = findActualSIeventForServiceUniqueKey(*uniqueServiceKey, zeitEvt1, 0, &flag);
	if(evt.name.empty() && flag !=0)
	{
		dmxEIT.change( 0 );
	}
	if (evt.service_id == 0)
	{
		MySIservicesOrderUniqueKey::iterator si = mySIservicesOrderUniqueKey.end();
		si = mySIservicesOrderUniqueKey.find(*uniqueServiceKey);

		if (si != mySIservicesOrderUniqueKey.end())
		{
			dprintf("[sectionsd] current service has%s scheduled events, and has%s present/following events\n", si->second->eitScheduleFlag() ? "" : " no", si->second->eitPresentFollowingFlag() ? "" : " no" );

			if ( /*( !si->second->eitScheduleFlag() ) || */
			    ( !si->second->eitPresentFollowingFlag() ) )
			{
				flag |= CSectionsdClient::epgflags::not_broadcast;
			}
		}
	}

	//dprintf("[sectionsd] current flag %d\n", flag);

	unlockServices();

	SIevent nextEvt;

	SItime zeitEvt2(zeitEvt1);

	if (evt.service_id != 0)
	{ //Found
		dprintf("[sectionsd] current EPG found.\n");

		for (unsigned int i = 0; i < evt.linkage_descs.size(); i++)
			if (evt.linkage_descs[i].linkageType == 0xB0)
			{
				flag |= CSectionsdClient::epgflags::current_has_linkagedescriptors;
				break;
			}


		nextEvt = findNextSIevent(evt.uniqueKey(), zeitEvt2);
	}
	else
		if ( flag & CSectionsdClient::epgflags::has_anything )
		{

			nextEvt = findNextSIeventForServiceUniqueKey(*uniqueServiceKey, zeitEvt2);

			if (nextEvt.service_id != 0)
			{
				MySIeventsOrderUniqueKey::iterator eFirst = mySIeventsOrderUniqueKey.find(*uniqueServiceKey);

				if (eFirst != mySIeventsOrderUniqueKey.end())
				{
					eFirst--;

					if (eFirst != mySIeventsOrderUniqueKey.begin())
					{
						time_t azeit = time(NULL);

						if ( ( eFirst->second->times.begin()->startzeit < azeit ) &&
						        ( eFirst->second->uniqueKey() == (nextEvt.uniqueKey() - 1) ) )
							flag |= CSectionsdClient::epgflags::has_no_current;
					}
				}
			}
		}

	if (nextEvt.service_id != 0)
	{
		dprintf("[sectionsd] next EPG found.\n");
		flag |= CSectionsdClient::epgflags::has_next;
	}

	nResultDataSize =
	    sizeof(event_id_t) +                        // Unique-Key
	    sizeof(CSectionsdClient::sectionsdTime) +  	// zeit
	    strlen(evt.name.c_str()) + 1 + 		// name + 0
	    sizeof(event_id_t) +                        // Unique-Key
	    sizeof(CSectionsdClient::sectionsdTime) +  	// zeit
	    strlen(nextEvt.name.c_str()) + 1 +    	// name + 0
	    sizeof(unsigned) + 				// flags
	    1						// CurrentFSK
	    ;

	pResultData = new char[nResultDataSize];

	if (!pResultData)
	{
		fprintf(stderr, "low on memory!\n");
		unlockEvents();
		EITThreadsUnPause();
		return ;
	}

	char *p = pResultData;
	*((event_id_t *)p) = evt.uniqueKey();
	p += sizeof(event_id_t);
	CSectionsdClient::sectionsdTime zeit;
	zeit.startzeit = zeitEvt1.startzeit;
	zeit.dauer = zeitEvt1.dauer;
	*((CSectionsdClient::sectionsdTime *)p) = zeit;
	p += sizeof(CSectionsdClient::sectionsdTime);
	strcpy(p, evt.name.c_str());
	p += strlen(evt.name.c_str()) + 1;
	*((event_id_t *)p) = nextEvt.uniqueKey();
	p += sizeof(event_id_t);
	zeit.startzeit = zeitEvt2.startzeit;
	zeit.dauer = zeitEvt2.dauer;
	*((CSectionsdClient::sectionsdTime *)p) = zeit;
	p += sizeof(CSectionsdClient::sectionsdTime);
	strcpy(p, nextEvt.name.c_str());
	p += strlen(nextEvt.name.c_str()) + 1;
	*((unsigned*)p) = flag;
	p += sizeof(unsigned);
	*p = evt.getFSK();
	//int x= evt.getFSK();
	p++;

	unlockEvents();
	EITThreadsUnPause(); // -> unlock

	// response

	struct sectionsd::msgResponseHeader pmResponse;
	pmResponse.dataLength = nResultDataSize;
	bool rc = writeNbytes(connfd, (const char *)&pmResponse, sizeof(pmResponse), WRITE_TIMEOUT_IN_SECONDS);

	if ( nResultDataSize > 0 )
	{
		if (rc == true)
			writeNbytes(connfd, pResultData, nResultDataSize, WRITE_TIMEOUT_IN_SECONDS);
		else
			dputs("[sectionsd] Fehler/Timeout bei write");

		delete[] pResultData;
	}
	else
	{
		dprintf("[sectionsd] current/next EPG not found!\n");
	}

	return ;
}

// Sendet ein EPG, unlocked die events, unpaused dmxEIT

static void sendEPG(int connfd, const SIevent& e, const SItime& t, int shortepg = 0)
{

	struct sectionsd::msgResponseHeader responseHeader;

	if (!shortepg)
	{
		// new format - 0 delimiters
		responseHeader.dataLength =
		    sizeof(event_id_t) +                        // Unique-Key
		    strlen(e.name.c_str()) + 1 + 		// Name + del
		    strlen(e.text.c_str()) + 1 + 		// Text + del
		    strlen(e.extendedText.c_str()) + 1 + 	// ext + del
			// 21.07.2005 - rainerk
			// Send extended events
		    strlen(e.itemDescription.c_str()) + 1 + // Item Description + del
		    strlen(e.item.c_str()) + 1 + // Item + del
		    strlen(e.contentClassification.c_str()) + 1 + 		// Text + del
		    strlen(e.userClassification.c_str()) + 1 + 	// ext + del
		    1 +                                   // fsk
		    sizeof(CSectionsdClient::sectionsdTime); // zeit
	}
	else
		responseHeader.dataLength =
		    strlen(e.name.c_str()) + 1 + 		// Name + del
		    strlen(e.text.c_str()) + 1 + 		// Text + del
		    strlen(e.extendedText.c_str()) + 1 + 1; // ext + del + 0

	char* msgData = new char[responseHeader.dataLength];

	if (!msgData)
	{
		fprintf(stderr, "low on memory!\n");
		unlockEvents();
		EITThreadsUnPause(); // -> unlock
		return ;
	}

	if (!shortepg)
	{
		char *p = msgData;
		*((event_id_t *)p) = e.uniqueKey();
		p += sizeof(event_id_t);

		strcpy(p, e.name.c_str());
		p += strlen(e.name.c_str()) + 1;
		strcpy(p, e.text.c_str());
		p += strlen(e.text.c_str()) + 1;
		strcpy(p, e.extendedText.c_str());
		p += strlen(e.extendedText.c_str()) + 1;
		// 21.07.2005 - rainerk
		// Send extended events
		strcpy(p, e.itemDescription.c_str());
		p += strlen(e.itemDescription.c_str()) + 1;
		strcpy(p, e.item.c_str());
		p += strlen(e.item.c_str()) + 1;
		strcpy(p, e.contentClassification.c_str());
		p += strlen(e.contentClassification.c_str()) + 1;
		strcpy(p, e.userClassification.c_str());
		p += strlen(e.userClassification.c_str()) + 1;
		*p = e.getFSK();
		p++;

		CSectionsdClient::sectionsdTime zeit;
		zeit.startzeit = t.startzeit;
		zeit.dauer = t.dauer;
		*((CSectionsdClient::sectionsdTime *)p) = zeit;
		p += sizeof(CSectionsdClient::sectionsdTime);

	}
	else
		sprintf(msgData,
		        "%s\xFF%s\xFF%s\xFF",
		        e.name.c_str(),
		        e.text.c_str(),
		        e.extendedText.c_str()
		       );

	unlockEvents();

	EITThreadsUnPause(); // -> unlock

	bool rc = writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS);

	if (rc == true)
		writeNbytes(connfd, msgData, responseHeader.dataLength, WRITE_TIMEOUT_IN_SECONDS);
	else
		dputs("[sectionsd] Fehler/Timeout bei write");

	delete[] msgData;
}

static void commandGetNextEPG(int connfd, char *data, const unsigned dataLength)
{
	if (dataLength != 8 + 4)
		return ;

	event_id_t * uniqueEventKey = (event_id_t *)data;

	time_t *starttime = (time_t *)(data + 8);

	dprintf("Request of next epg for 0x%llx %s", *uniqueEventKey, ctime(starttime));

	if (EITThreadsPause()) // -> lock
		return ;

	lockEvents();

	SItime zeit(*starttime, 0);

	const SIevent &nextEvt = findNextSIevent(*uniqueEventKey, zeit);

	if (nextEvt.service_id != 0)
	{
		dprintf("next epg found.\n");
		sendEPG(connfd, nextEvt, zeit);
	}
	else
	{
		unlockEvents();
		EITThreadsUnPause(); // -> unlock
		dprintf("next epg not found!\n");

		struct sectionsd::msgResponseHeader responseHeader;
		responseHeader.dataLength = 0;
		writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS);
	}

	return ;
}

static void commandActualEPGchannelID(int connfd, char *data, const unsigned dataLength)
{
	if (dataLength != sizeof(t_channel_id))
		return ;

	t_channel_id * uniqueServiceKey = (t_channel_id *)data;

	dprintf("Request of actual EPG for " PRINTF_CHANNEL_ID_TYPE "\n", * uniqueServiceKey);

	if (EITThreadsPause()) // -> lock
		return ;

	lockEvents();

	SItime zeit(0, 0);

	const SIevent &evt = findActualSIeventForServiceUniqueKey(*uniqueServiceKey, zeit);

	if (evt.service_id != 0)
	{
		dprintf("EPG found.\n");
		sendEPG(connfd, evt, zeit);
	}
	else
	{
		unlockEvents();
		EITThreadsUnPause(); // -> unlock
		dprintf("EPG not found!\n");

		struct sectionsd::msgResponseHeader responseHeader;
		responseHeader.dataLength = 0;
		writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS);
	}

	return ;
}

static void commandGetEPGPrevNext(int connfd, char *data, const unsigned dataLength)
{
	if (dataLength != 8 + 4)
		return ;

	event_id_t * uniqueEventKey = (event_id_t *)data;

	time_t *starttime = (time_t *)(data + 8);

	dprintf("Request of Prev/Next EPG for 0x%llx %s", *uniqueEventKey, ctime(starttime));

	if (EITThreadsPause()) // -> lock
		return ;

	lockEvents();

	SItime zeit(*starttime, 0);

	SItime prev_zeit(0, 0);

	SItime next_zeit(0, 0);

	SIevent prev_evt;

	SIevent next_evt;

	findPrevNextSIevent(*uniqueEventKey, zeit, prev_evt, prev_zeit, next_evt, next_zeit);

	struct sectionsd::msgResponseHeader responseHeader;

	responseHeader.dataLength =
	    12 + 1 + 				// Unique-Key + del
	    8 + 1 + 				// start time + del
	    12 + 1 + 				// Unique-Key + del
	    8 + 1 + 1;				// start time + del

	char* msgData = new char[responseHeader.dataLength];

	if (!msgData)
	{
		fprintf(stderr, "low on memory!\n");
		unlockEvents();
		EITThreadsUnPause(); // -> unlock
		return ;
	}

	sprintf(msgData, "%012llx\xFF%08lx\xFF%012llx\xFF%08lx\xFF",
	        prev_evt.uniqueKey(),
	        prev_zeit.startzeit,
	        next_evt.uniqueKey(),
	        next_zeit.startzeit
	       );
	unlockEvents();
	EITThreadsUnPause(); // -> unlock

	bool rc = writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS);

	if (rc == true)
		writeNbytes(connfd, msgData, responseHeader.dataLength, WRITE_TIMEOUT_IN_SECONDS);
	else
		dputs("[sectionsd] Fehler/Timeout bei write");

	delete[] msgData;

	return ;
}

// Mostly copied from epgd (something bugfixed ;) )

static void commandActualEPGchannelName(int connfd, char *data, const unsigned dataLength)
{
	int nResultDataSize = 0;
	char* pResultData = 0;

	data[dataLength - 1] = 0; // to be sure it has an trailing 0
	dprintf("Request of actual EPG for '%s'\n", data);

	if (EITThreadsPause()) // -> lock
		return ;

	lockServices();

	lockEvents();

	SItime zeitEvt(0, 0);

	const SIevent &evt = findActualSIeventForServiceName(data, zeitEvt);

	unlockServices();

	if (evt.service_id != 0)
	{ //Found
		dprintf("EPG found.\n");
		nResultDataSize =
		    12 + 1 + 					// Unique-Key + del
		    strlen(evt.name.c_str()) + 1 + 		//Name + del
		    strlen(evt.text.c_str()) + 1 + 		//Text + del
		    strlen(evt.extendedText.c_str()) + 1 + 	//ext + del
		    3 + 3 + 4 + 1 + 					//dd.mm.yyyy + del
		    3 + 2 + 1 + 					//std:min + del
		    3 + 2 + 1 + 					//std:min+ del
		    3 + 1 + 1;					//100 + del + 0
		pResultData = new char[nResultDataSize];

		if (!pResultData)
		{
			fprintf(stderr, "low on memory!\n");
			unlockEvents();
			EITThreadsUnPause();
			return ;
		}

		struct tm *pStartZeit = localtime(&zeitEvt.startzeit);

		int nSDay(pStartZeit->tm_mday), nSMon(pStartZeit->tm_mon + 1), nSYear(pStartZeit->tm_year + 1900),
		nSH(pStartZeit->tm_hour), nSM(pStartZeit->tm_min);

		long int uiEndTime(zeitEvt.startzeit + zeitEvt.dauer);

		struct tm *pEndeZeit = localtime((time_t*) & uiEndTime);

		int nFH(pEndeZeit->tm_hour), nFM(pEndeZeit->tm_min);

		unsigned nProcentagePassed = (unsigned)((float)(time(NULL) - zeitEvt.startzeit) / (float)zeitEvt.dauer * 100.);

		sprintf(pResultData, "%012llx\xFF%s\xFF%s\xFF%s\xFF%02d.%02d.%04d\xFF%02d:%02d\xFF%02d:%02d\xFF%03u\xFF",
		        evt.uniqueKey(),
		        evt.name.c_str(),
		        evt.text.c_str(),
		        evt.extendedText.c_str(), nSDay, nSMon, nSYear, nSH, nSM, nFH, nFM, nProcentagePassed );
	}
	else
		dprintf("actual EPG not found!\n");

	unlockEvents();

	EITThreadsUnPause(); // -> unlock

	// response

	struct sectionsd::msgResponseHeader pmResponse;

	pmResponse.dataLength = nResultDataSize;

	bool rc = writeNbytes(connfd, (const char *)&pmResponse, sizeof(pmResponse), WRITE_TIMEOUT_IN_SECONDS);

	if ( nResultDataSize > 0 )
	{
		if (rc == true)
			writeNbytes(connfd, pResultData, nResultDataSize, WRITE_TIMEOUT_IN_SECONDS);
		else
			dputs("[sectionsd] Fehler/Timeout bei write");

		delete[] pResultData;
	}
}

static void sendEventList(int connfd, const unsigned char serviceTyp1, const unsigned char serviceTyp2 = 0, int sendServiceName = 1)
{
	char *evtList = new char[128* 1024]; // 256kb..? should be enough and dataLength is unsigned short

	if (!evtList)
	{
		fprintf(stderr, "low on memory!\n");
		return ;
	}

	*evtList = 0;

	if (EITThreadsPause()) // -> lock
	{
		delete[] evtList;
		return ;
	}

	if (dmxSDT.pause())
	{
		delete[] evtList;
		EITThreadsUnPause();
		return ;
	}

	char *liste = evtList;
	lockServices();
	lockEvents();

	t_channel_id uniqueNow = 0;
	t_channel_id uniqueOld = 0;
	bool found_already = false;
	time_t azeit = time(NULL);
	std::string sname;

	for (MySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey::iterator e = mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.begin(); e != mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.end(); e++)
	{
		uniqueNow = (*e)->get_channel_id();

		if ( uniqueNow != uniqueOld )
		{
			found_already = true;

			// new service, check service- type
			MySIservicesOrderUniqueKey::iterator s = mySIservicesOrderUniqueKey.find(uniqueNow);

			if (s != mySIservicesOrderUniqueKey.end())
			{
				if (s->second->serviceTyp == serviceTyp1 || (serviceTyp2 && s->second->serviceTyp == serviceTyp2))
				{
					sname = s->second->serviceName;
					found_already = false;
				}
			}
			else
			{
				// wenn noch nie hingetuned wurde, dann gibts keine Info �ber den ServiceTyp...
				// im Zweifel mitnehmen
				found_already = false;
			}

			uniqueOld = uniqueNow;
		}

		if ( !found_already )
		{
			for (SItimes::iterator t = (*e)->times.begin(); t != (*e)->times.end(); t++)
				if (t->startzeit <= azeit && azeit <= (long)(t->startzeit + t->dauer))
				{
					if (sendServiceName)
					{
						sprintf(liste, "%012llx\n", (*e)->uniqueKey());
						liste += 13;
						strcpy(liste, sname.c_str());
						liste += strlen(sname.c_str());
						*liste = '\n';
						liste++;
						strcpy(liste, (*e)->name.c_str());
						liste += strlen((*e)->name.c_str());
						*liste = '\n';
						liste++;
					} // if sendServiceName
					else
					{
						*((event_id_t *)liste) = (*e)->uniqueKey();
						liste += sizeof(event_id_t);
						*((unsigned *)liste) = t->startzeit;
						liste += 4;
						*((unsigned *)liste) = t->dauer;
						liste += 4;
						strcpy(liste, (*e)->name.c_str());
						liste += strlen(liste);
						liste++;

						if (((*e)->text).empty())
						{
							strcpy(liste, (*e)->extendedText.substr(0, 40).c_str());
							liste += strlen(liste);
						}
						else
						{
							strcpy(liste, (*e)->text.c_str());
							liste += strlen(liste);
						}

						liste++;
					} // else !sendServiceName

					found_already = true;

					break;
				}
		}
	}

	if (sendServiceName)
	{
		*liste = 0;
		liste++;
	}

	unlockEvents();
	unlockServices();

	dmxSDT.unpause();
	EITThreadsUnPause();

	struct sectionsd::msgResponseHeader msgResponse;
	if (liste - evtList > 128*1024) 
		printf("warning: [sectionsd] sendEventList- response-size: 0x%x\n", liste - evtList);
	msgResponse.dataLength = liste - evtList;
	dprintf("[sectionsd] all channels - response-size: 0x%x\n", msgResponse.dataLength);

	if ( msgResponse.dataLength == 1 )
		msgResponse.dataLength = 0;

	if (writeNbytes(connfd, (const char *)&msgResponse, sizeof(msgResponse), WRITE_TIMEOUT_IN_SECONDS) == true)
	{
		if (msgResponse.dataLength)
			writeNbytes(connfd, evtList, msgResponse.dataLength, WRITE_TIMEOUT_IN_SECONDS);
	}
	else
		dputs("[sectionsd] Fehler/Timeout bei write");

	delete[] evtList;
}

// Sendet ein short EPG, unlocked die events, unpaused dmxEIT

static void sendShort(int connfd, const SIevent& e, const SItime& t)
{

	struct sectionsd::msgResponseHeader responseHeader;

	responseHeader.dataLength =
	    12 + 1 + 				// Unique-Key + del
	    strlen(e.name.c_str()) + 1 + 		// name + del
	    8 + 1 + 				// start time + del
	    8 + 1 + 1;				// duration + del + 0
	char* msgData = new char[responseHeader.dataLength];

	if (!msgData)
	{
		fprintf(stderr, "low on memory!\n");
		unlockEvents();
		EITThreadsUnPause(); // -> unlock
		return ;
	}

	sprintf(msgData,
	        "%012llx\n%s\n%08lx\n%08x\n",
	        e.uniqueKey(),
	        e.name.c_str(),
	        t.startzeit,
	        t.dauer
	       );
	unlockEvents();
	EITThreadsUnPause(); // -> unlock

	bool rc = writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS);

	if (rc == true)
		writeNbytes(connfd, msgData, responseHeader.dataLength, WRITE_TIMEOUT_IN_SECONDS);
	else
		dputs("[sectionsd] Fehler/Timeout bei write");

	delete[] msgData;
}

static void commandGetNextShort(int connfd, char *data, const unsigned dataLength)
{
	if (dataLength != 8 + 4)
		return ;

	event_id_t * uniqueEventKey = (event_id_t *)data;

	time_t *starttime = (time_t *)(data + 8);

	dprintf("Request of next short for 0x%llx %s", *uniqueEventKey, ctime(starttime));

	if (EITThreadsPause()) // -> lock
		return ;

	lockEvents();

	SItime zeit(*starttime, 0);

	const SIevent &nextEvt = findNextSIevent(*uniqueEventKey, zeit);

	if (nextEvt.service_id != 0)
	{
		dprintf("next short found.\n");
		sendShort(connfd, nextEvt, zeit);
	}
	else
	{
		unlockEvents();
		EITThreadsUnPause(); // -> unlock
		dprintf("next short not found!\n");

		struct sectionsd::msgResponseHeader responseHeader;
		responseHeader.dataLength = 0;
		writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS);
	}
}

static void commandEventListTV(int connfd, char* /*data*/, const unsigned /*dataLength*/)
{
	dputs("Request of TV event list.\n");
	sendEventList(connfd, 0x01, 0x04);
}

static void commandEventListTVids(int connfd, char* /*data*/, const unsigned /*dataLength*/)
{
	dputs("Request of TV event list (IDs).\n");
	sendEventList(connfd, 0x01, 0x04, 0);
}

static void commandEventListRadio(int connfd, char* /*data*/, const unsigned /*dataLength*/)
{
	dputs("Request of radio event list.\n");
	sendEventList(connfd, 0x02);
}

static void commandEventListRadioIDs(int connfd, char* /*data*/, const unsigned /*dataLength*/)
{
	dputs("Request of radio event list (IDs).\n");
	sendEventList(connfd, 0x02, 0, 0);
}

static void commandEPGepgID(int connfd, char *data, const unsigned dataLength)
{
	if (dataLength != 8 + 4)
		return ;

	event_id_t * epgID = (event_id_t *)data;

	time_t* startzeit = (time_t *)(data + 8);

	dprintf("Request of actual EPG for 0x%llx 0x%lx\n", *epgID, *startzeit);

	if (EITThreadsPause()) // -> lock
		return ;

	lockEvents();

	const SIevent& evt = findSIeventForEventUniqueKey(*epgID);

	if (evt.service_id != 0)
	{ // Event found
		SItimes::iterator t = evt.times.begin();

		for (; t != evt.times.end(); t++)
			if (t->startzeit == *startzeit)
				break;

		if (t == evt.times.end())
		{
			dputs("EPG not found!");
			unlockEvents();
			EITThreadsUnPause(); // -> unlock
		}
		else
		{
			dputs("EPG found.");
			// Sendet ein EPG, unlocked die events, unpaused dmxEIT
			sendEPG(connfd, evt, *t);
		}
	}
	else
	{
		dputs("EPG not found!");
		unlockEvents();
		EITThreadsUnPause(); // -> unlock
		// response

		struct sectionsd::msgResponseHeader pmResponse;
		pmResponse.dataLength = 0;

		writeNbytes(connfd, (const char *)&pmResponse, sizeof(pmResponse), WRITE_TIMEOUT_IN_SECONDS);
	}
}

static void commandEPGepgIDshort(int connfd, char *data, const unsigned dataLength)
{
	if (dataLength != 8)
		return;

	event_id_t * epgID = (event_id_t *)data;

	dprintf("Request of actual EPG for 0x%llx\n", *epgID);

	if (EITThreadsPause()) // -> lock
		return;

	lockEvents();

	const SIevent& evt = findSIeventForEventUniqueKey(*epgID);

	if (evt.service_id != 0)
	{ // Event found
		dputs("EPG found.");
		sendEPG(connfd, evt, SItime(0, 0), 1);
	}
	else
	{
		dputs("EPG not found!");
		unlockEvents();
		EITThreadsUnPause(); // -> unlock
		// response

		struct sectionsd::msgResponseHeader pmResponse;
		pmResponse.dataLength = 0;

		writeNbytes(connfd, (const char *)&pmResponse, sizeof(pmResponse), WRITE_TIMEOUT_IN_SECONDS);
	}
}

static void commandTimesNVODservice(int connfd, char *data, const unsigned dataLength)
{
	if (dataLength != sizeof(t_channel_id))
		return ;

	t_channel_id uniqueServiceKey = *(t_channel_id *)data;

	dprintf("Request of NVOD times for " PRINTF_CHANNEL_ID_TYPE "\n", uniqueServiceKey);

	if (EITThreadsPause()) // -> lock
		return ;

	lockServices();

	lockEvents();

	MySIservicesNVODorderUniqueKey::iterator si = mySIservicesNVODorderUniqueKey.find(uniqueServiceKey);

	char *msgData = 0;

	struct sectionsd::msgResponseHeader responseHeader;

	responseHeader.dataLength = 0;

	if (si != mySIservicesNVODorderUniqueKey.end())
	{
		dprintf("NVODServices: %u\n", si->second->nvods.size());

		if (si->second->nvods.size())
		{
			responseHeader.dataLength = (sizeof(t_service_id) + sizeof(t_original_network_id) + sizeof(t_transport_stream_id) + 4 + 4) * si->second->nvods.size();
			msgData = new char[responseHeader.dataLength];

			if (!msgData)
			{
				fprintf(stderr, "low on memory!\n");
				unlockEvents();
				unlockServices();
				EITThreadsUnPause(); // -> unlock
				return ;
			}

			char *p = msgData;
			//      time_t azeit=time(NULL);

			for (SInvodReferences::iterator ni = si->second->nvods.begin(); ni != si->second->nvods.end(); ni++)
			{
				// Zeiten sind erstmal dummy, d.h. pro Service eine Zeit
				ni->toStream(p); // => p += sizeof(t_service_id) + sizeof(t_original_network_id) + sizeof(t_transport_stream_id);

				SItime zeitEvt1(0, 0);
				//        const SIevent &evt=
				findActualSIeventForServiceUniqueKey(ni->uniqueKey(), zeitEvt1, 15*60);
				*(time_t *)p = zeitEvt1.startzeit;
				p += 4;
				*(unsigned *)p = zeitEvt1.dauer;
				p += 4;

				/*        MySIeventUniqueKeysMetaOrderServiceUniqueKey::iterator ei=mySIeventUniqueKeysMetaOrderServiceUniqueKey.find(ni->uniqueKey());
				        if(ei!=mySIeventUniqueKeysMetaOrderServiceUniqueKey.end())
				        {
				            dprintf("found NVod - Service: %0llx\n", ei->second);
				            MySIeventsOrderUniqueKey::iterator e=mySIeventsOrderUniqueKey.find(ei->second);
				            if(e!=mySIeventsOrderUniqueKey.end())
				            {
				                // ist ein MetaEvent, d.h. mit Zeiten fuer NVOD-Event
				                for(SItimes::iterator t=e->second->times.begin(); t!=e->second->times.end(); t++)
				                if(t->startzeit<=azeit && azeit<=(long)(t->startzeit+t->dauer))
				                {
				                    *(time_t *)p=t->startzeit;
				                    break;
				                }
				            }
				        }
				*/

			}
		}
	}

	dprintf("data bytes: %u\n", responseHeader.dataLength);
	bool rc = writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS);

	if (rc == true)
	{
		if (responseHeader.dataLength)
			writeNbytes(connfd, msgData, responseHeader.dataLength, WRITE_TIMEOUT_IN_SECONDS);
	}
	else
		dputs("[sectionsd] Fehler/Timeout bei write");

	if (responseHeader.dataLength)
	{
		delete[] msgData;
	}

	unlockEvents();

	unlockServices();

	EITThreadsUnPause(); // -> unlock
}


static void commandGetIsTimeSet(int connfd, char* /*data*/, const unsigned /*dataLength*/)
{
	sectionsd::responseIsTimeSet rmsg;

	rmsg.IsTimeSet = timeset;

	dprintf("Request of Time-Is-Set %d\n", rmsg.IsTimeSet);

	struct sectionsd::msgResponseHeader responseHeader;

	responseHeader.dataLength = sizeof(rmsg);

	if (writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS) == true)
	{
		writeNbytes(connfd, (const char *)&rmsg, responseHeader.dataLength, WRITE_TIMEOUT_IN_SECONDS);
	}
	else
		dputs("[sectionsd] Fehler/Timeout bei write");
}


static void commandRegisterEventClient(int /*connfd*/, char *data, const unsigned dataLength)
{
	if (dataLength == sizeof(CEventServer::commandRegisterEvent))
	{
		eventServer->registerEvent2(((CEventServer::commandRegisterEvent*)data)->eventID, ((CEventServer::commandRegisterEvent*)data)->clientID, ((CEventServer::commandRegisterEvent*)data)->udsName);

		if (((CEventServer::commandRegisterEvent*)data)->eventID == CSectionsdClient::EVT_TIMESET)
			messaging_neutrino_sets_time = true;
	}
}



static void commandUnRegisterEventClient(int /*connfd*/, char *data, const unsigned dataLength)
{
	if (dataLength == sizeof(CEventServer::commandUnRegisterEvent))
		eventServer->unRegisterEvent2(((CEventServer::commandUnRegisterEvent*)data)->eventID, ((CEventServer::commandUnRegisterEvent*)data)->clientID);
}


static void commandSetPrivatePid(int connfd, char *data, const unsigned dataLength)
{
	unsigned short pid;
	
	if (dataLength != 2)
		return ;

	lockMessaging();
	pid = *((unsigned short*)data);
//	if (privatePid != pid)
	{
		privatePid = pid;
		if (pid != 0) {
			/*d*/printf("[sectionsd] wakeup PPT Thread, pid=%x\n", pid);
			dmxPPT.change( 0 );
		}
	}
	unlockMessaging();
	
	struct sectionsd::msgResponseHeader responseHeader;
	responseHeader.dataLength = 0;
	writeNbytes(connfd, (const char *)&responseHeader, sizeof(responseHeader), WRITE_TIMEOUT_IN_SECONDS);
	return ;
}


static void (*connectionCommands[sectionsd::numberOfCommands]) (int connfd, char *, const unsigned) =
    {
        commandActualEPGchannelName,
        commandEventListTV,
        commandCurrentNextInfoChannelName,
        commandDumpStatusInformation,
        commandAllEventsChannelName,
        commandSetHoursToCache,
        commandSetEventsAreOldInMinutes,
        commandDumpAllServices,
        commandEventListRadio,
        commandGetNextEPG,
        commandGetNextShort,
        commandPauseScanning,
        commandGetIsScanningActive,
        commandActualEPGchannelID,
        commandEventListTVids,
        commandEventListRadioIDs,
        commandCurrentNextInfoChannelID,
        commandEPGepgID,
        commandEPGepgIDshort,
        commandComponentTagsUniqueKey,
        commandAllEventsChannelID,
        commandTimesNVODservice,
        commandGetEPGPrevNext,
        commandGetIsTimeSet,
        commandserviceChanged,
        commandLinkageDescriptorsUniqueKey,
        commandPauseSorting,
	commandRegisterEventClient,
	commandUnRegisterEventClient,
	commandSetPrivatePid
    };

//static void *connectionThread(void *conn)
bool parse_command(CBasicMessage::Header &rmsg, int connfd)
{
	/*
	  pthread_t threadConnection;
	  rc = pthread_create(&threadConnection, &conn_attrs, connectionThread, client);
	  if(rc)
	  {
	  fprintf(stderr, "[sectionsd] failed to create connection-thread (rc=%d)\n", rc);
	  return 4;
	  }
	*/
	// VERSUCH OHNE CONNECTION-THREAD!
	// spart die thread-creation-zeit, und die Locks lassen ohnehin nur ein cmd gleichzeitig zu
	try
	{
		dprintf("Connection from UDS\n");

		struct sectionsd::msgRequestHeader header;

		memcpy(&header, &rmsg, sizeof(CBasicMessage::Header));
		memset(((char *)&header) + sizeof(CBasicMessage::Header), 0, sizeof(header) - sizeof(CBasicMessage::Header));

		bool readbytes = readNbytes(connfd, ((char *)&header) + sizeof(CBasicMessage::Header), sizeof(header) - sizeof(CBasicMessage::Header), READ_TIMEOUT_IN_SECONDS);

		if (readbytes == true)
		{
			dprintf("version: %hhd, cmd: %hhd, numbytes: %d\n", header.version, header.command, readbytes);

			if (header.command < sectionsd::numberOfCommands)
			{
				dprintf("data length: %hd\n", header.dataLength);
				char *data = new char[header.dataLength + 1];

				if (!data)
					fprintf(stderr, "low on memory!\n");
				else
				{
					bool rc = true;

					if (header.dataLength)
						rc = readNbytes(connfd, data, header.dataLength, READ_TIMEOUT_IN_SECONDS);

					if (rc == true)
					{
						dprintf("Starting command %hhd\n", header.command);
						connectionCommands[header.command](connfd, data, header.dataLength);
					}

					delete[] data;
				}
			}
			else
				dputs("Unknow format or version of request!");
		}
	} // try
	catch (std::exception& e)
	{
		fprintf(stderr, "Caught std-exception in connection-thread %s!\n", e.what());
	}
	catch (...)
	{
		fprintf(stderr, "Caught exception in connection-thread!\n");
	}

	return true;
}

//---------------------------------------------------------------------
//			sdt-thread
// reads sdt for service list
//---------------------------------------------------------------------
static void *sdtThread(void *)
{

	struct SI_section_header header;
	char *buf;
	const unsigned timeoutInMSeconds = 250;
	bool sendToSleepNow = false;

	dmxSDT.addfilter(0x42, 0xff );
	//    dmxSDT.addfilter(0x46, 0xff );

	try
	{
		dprintf("[%sThread] pid %d start\n", "sdt", getpid());

		int timeoutsDMX = 0;
		dmxSDT.start(); // -> unlock

		for (;;)
		{
			time_t zeit = time(NULL);

			if (timeset)
			{
				// Nur wenn ne richtige Uhrzeit da ist

				if ( (sendToSleepNow) )
				{
					sendToSleepNow = false;

					struct timespec abs_wait;

					struct timeval now;

					gettimeofday(&now, NULL);
					TIMEVAL_TO_TIMESPEC(&now, &abs_wait);
					abs_wait.tv_sec += (TIME_SDT_SCHEDULED_PAUSE);

					dmxSDT.real_pause();
					pthread_mutex_lock( &dmxSDT.start_stop_mutex );
					dprintf("dmxSDT: going to sleep...\n");

					int rs = pthread_cond_timedwait( &dmxSDT.change_cond, &dmxSDT.start_stop_mutex, &abs_wait );

					if (rs == ETIMEDOUT)
					{
						dprintf("dmxSDT: waking up again - looking for new events :)\n");
						pthread_mutex_unlock( &dmxSDT.start_stop_mutex );
#ifdef PAUSE_EQUALS_STOP
						dmxSDT.real_unpause();
#endif
						dmxSDT.change( 0 ); // -> restart
					}
					else if (rs == 0)
					{
						dprintf("dmxSDT: waking up again - requested from .change()\n");
						pthread_mutex_unlock( &dmxSDT.start_stop_mutex );
#ifdef PAUSE_EQUALS_STOP
						dmxSDT.real_unpause();
#endif
					}
					else
					{
						dprintf("dmxSDT:  waking up again - unknown reason?!\n");
						pthread_mutex_unlock( &dmxSDT.start_stop_mutex );
						dmxSDT.real_unpause();
					}
					// update zeit after sleep
					zeit = time(NULL);
					timeoutsDMX = 0;
				}
				else if (zeit > dmxSDT.lastChanged + TIME_SDT_SKIPPING )
				{
					lockMessaging();

					if ( dmxSDT.filter_index + 1 < (signed) dmxSDT.filters.size() )
					{
						dmxSDT.change(dmxSDT.filter_index + 1);
						dputs("[sdtThread] skipping to next filter (> TIME_SDT_SKIPPING)");
						unlockMessaging();
					}
					else
					{
						sendToSleepNow = true;
						dputs("[sdtThread] sending to sleep (> TIME_SDT_SKIPPING)");
						unlockMessaging();
						continue;
					}
				};
			}

			if (timeoutsDMX >= RESTART_DMX_AFTER_TIMEOUTS && scanning)
			{
				timeoutsDMX = 0;
				dmxSDT.stop();
				dmxSDT.start(); // leaves unlocked
				dputs("\n !!! dmxSDT restarted !!!\n");
			}

			struct timeval tv;

			gettimeofday( &tv, NULL );
			long long _now = (long long) tv.tv_usec + (long long)((long long) tv.tv_sec * (long long) 1000000);

			buf = dmxSDT.getSection(timeoutInMSeconds, timeoutsDMX);

			gettimeofday( &tv, NULL );
			_now = (long long) tv.tv_usec + (long long)((long long) tv.tv_sec * (long long) 1000000)- _now;
        		dprintf("[sdtThread] dmxSDT.getSection returned after %f\n", _now / 1000.);


			if (buf == NULL)
				continue;

			unsigned short section_length = (((SI_section_header*)buf)->section_length_hi << 8) |
							((SI_section_header*)buf)->section_length_lo;
			// copy the header
			memcpy(&header, buf, std::min((unsigned)section_length + 3, sizeof(header)));

			if ((header.current_next_indicator) && (!dmxSDT.pauseCounter))
			{
				// Wir wollen nur aktuelle sections
				dprintf("[sdtThread] adding services [table 0x%x] (begin)\n", header.table_id);

				SIsectionSDT sdt(SIsection(sizeof(header) + section_length - 5, buf));

				lockServices();

				for (SIservices::iterator s = sdt.services().begin(); s != sdt.services().end(); s++)
					addService(*s);

				unlockServices();

				dprintf("[sdtThread] added %d services (end)\n",  sdt.services().size());


				lockMessaging();


				//SI_section_SDT_header* _header = (SI_section_SDT_header*) &header;
				int msg_index = ( header.table_id == 0x42) ? 0 : 1;

				if ( !messaging_sdt_sections_got_all[msg_index] )
				{
					long long _id = (((unsigned long long)header.table_id) << 40) +
					                (((unsigned long long)header.table_id_extension_hi) << 32) +
					                (((unsigned long long)header.table_id_extension_lo) << 24) +
					                (((unsigned long long)header.section_number) << 16) +
					                (((unsigned long long)header.version_number) << 8) +
					                (((unsigned long long)header.current_next_indicator));

					if ( messaging_sdt_sections_max_ID[msg_index] == -1 )
					{
						messaging_sdt_sections_max_ID[msg_index] = _id;
						dprintf("[sdtThread] first msg_index 0x%llx\n", _id);
					}
					else
					{
						for ( std::vector<long long>::iterator i = messaging_sdt_skipped_sections_ID[msg_index].begin();
						        i != messaging_sdt_skipped_sections_ID[msg_index].end(); ++i )
							if ( *i == _id)
							{
								messaging_sdt_skipped_sections_ID[msg_index].erase(i);
								break;
							}
						dprintf("[sdtThread] now msg_index 0x%llx / skipsize %d\n", _id, messaging_sdt_skipped_sections_ID[msg_index].size());

						if ((messaging_sdt_sections_max_ID[msg_index] == _id) &&
						    (messaging_sdt_skipped_sections_ID[msg_index].empty()))
						{
							// alle pakete f�r den ServiceKey da!
							dprintf("[sdtThread] got all packages for table_id 0x%x (%d)\n", header.table_id, msg_index);
							messaging_sdt_sections_got_all[msg_index] = true;
						}
					}

					// �berpr�fen, ob n�chster Filter gew�nscht :)
					if ( messaging_sdt_sections_got_all[dmxSDT.filter_index] )
					{
						if ( ( messaging_WaitForServiceDesc ) && ( dmxSDT.filter_index == 0 ) )
						{
					        	// restart EIT!
					        	for ( int i = 0x4e; i <= 0x6f; i++)
							{
								messaging_skipped_sections_ID[i - 0x4e].clear();
								messaging_sections_max_ID[i - 0x4e] = -1;
								messaging_sections_got_all[i - 0x4e] = false;
							}

							messaging_wants_current_next_Event = true;
							dmxEIT.change( 0 );

							messaging_WaitForServiceDesc = false;
						}
						if ( dmxSDT.filter_index + 1 < (signed) dmxSDT.filters.size() )
						{
							dmxSDT.change(dmxSDT.filter_index + 1);
							dputs("[sdtThread] change!")
						}
						else
						{
							sendToSleepNow = true;
							dputs("[sdtThread] sendtosleep!")
						}
					}
				}

				unlockMessaging();


			} // if
			else
			{
				lockMessaging();

				int msg_index = ( header.table_id == 0x42) ? 0 : 1;
				long long _id = (((unsigned long long)header.table_id) << 40) +
				                (((unsigned long long)header.table_id_extension_hi) << 32) +
				                (((unsigned long long)header.table_id_extension_lo) << 24) +
				                (((unsigned long long)header.section_number) << 16) +
				                (((unsigned long long)header.version_number) << 8) +
				                (((unsigned long long)header.current_next_indicator));

				if (messaging_sdt_sections_max_ID[msg_index] != -1)
					messaging_sdt_skipped_sections_ID[msg_index].push_back(_id);

				unlockMessaging();

				dprintf("[sdtThread] skipped sections for table 0x%x\n", header.table_id);

				delete[] buf;
			}
		} // for

		dmxSDT.closefd();
	} // try
	catch (std::exception& e)
	{
		fprintf(stderr, "Caught std-exception in connection-thread %s!\n", e.what());
	}
	catch (...)
	{
		fprintf(stderr, "Caught exception in sdt-thread!\n");
	}

	dprintf("sdt-thread ended\n");

	pthread_exit(NULL);
}

//---------------------------------------------------------------------
//			Time-thread
// updates system time according TOT every 30 minutes
//---------------------------------------------------------------------


/*
// BR schickt falschen Time-Offset, daher per TZ und Rest hier auskommentiert

struct descr_gen_struct {
  unsigned char descriptor_tag : 8;
  unsigned char descriptor_length : 8;
} __attribute__ ((packed)) ;

struct local_time_offset {
  char country_code1 : 8;
  char country_code2 : 8;
  char country_code3 : 8;
  unsigned char country_region_id : 6;
  unsigned char reserved : 1;
  unsigned char local_time_offset_polarity : 1;
  unsigned short local_time_offs : 16;
  unsigned long long time_of_chng : 40;
  unsigned short next_time_offset : 8;
} __attribute__ ((packed)) ;

static int timeOffsetMinutes=0; // minutes
static int timeOffsetFound=0;

static void parseLocalTimeOffsetDescriptor(const char *buf, const char *countryCode)
{
  struct descr_gen_struct *desc=(struct descr_gen_struct *)buf;
  buf+=2;
  while(buf<((char *)desc)+2+desc->descriptor_length-sizeof(struct local_time_offset)) {
    struct local_time_offset *lto=(struct local_time_offset *)buf;
    if(!strncmp(countryCode, buf, 3)) {
      timeOffsetMinutes=(((lto->local_time_offs)>>12)&0x0f)*10*60L+(((lto->local_time_offs)>>8)&0x0f)*60L+
	(((lto->local_time_offs)>>4)&0x0f)*10+((lto->local_time_offs)&0x0f);
      if(lto->local_time_offset_polarity)
        timeOffsetMinutes=-timeOffsetMinutes;
      timeOffsetFound=1;
      break;
    }
//    else
//      printf("Code: %c%c%c\n", lto->country_code1, lto->country_code2, lto->country_code3);
    buf+=sizeof(struct local_time_offset);
  }
}

static void parseDescriptors(const char *des, unsigned len, const char *countryCode)
{
  struct descr_gen_struct *desc;
  while(len>=sizeof(struct descr_gen_struct)) {
    desc=(struct descr_gen_struct *)des;
    if(desc->descriptor_tag==0x58) {
//      printf("Found time descriptor\n");
      parseLocalTimeOffsetDescriptor((const char *)desc, countryCode);
      if(timeOffsetFound)
        break;
    }
    len-=desc->descriptor_length+2;
    des+=desc->descriptor_length+2;
  }
}

*/
static void *timeThread(void *)
{
	UTC_t UTC;
	time_t tim;
	unsigned int seconds;
	bool first_time = true; /* we don't sleep the first time (we try to get a TOT header) */
	
	try
	{
		dprintf("[%sThread] pid %d start\n", "time", getpid());


		// -- check if time is already on box (e.g. using rdate/ntpd)  (2005-05-02 rasc)
		// -- if so skip first_time, etc. flags for better/quick EPG startup
		{
		  time_t actTime;
		  struct tm *tmTime;
		  actTime=time(NULL);
		  tmTime = localtime(&actTime);

		  // -- do we already have a valid(???) date/time?
		  if ((tmTime->tm_year + 1900) >= 2005) {
			first_time = false;
			timeset = true;
		  	dprintf("we already have a time set\n");
		  }
		}



		while(1)
		{
			if (scanning && (getUTC(&UTC, true))) // always use TDT, a lot of transponders don't provide a TOT
			{
				tim = changeUTCtoCtime((const unsigned char *) &UTC);
				
				if (tim) {
					if ((!messaging_neutrino_sets_time) && (geteuid() == 0)) {
						struct timeval tv;
						tv.tv_sec = tim;
						tv.tv_usec = 0;
						if (settimeofday(&tv, NULL) < 0) {
							perror("[sectionsd] settimeofday");
							pthread_exit(NULL);
						}
					}

					time_t actTime;
					struct tm *tmTime;
					actTime=time(NULL);
					tmTime = localtime(&actTime);
					printf("[%sThread] time(): %02d.%02d.%04d %02d:%02d:%02d, tim: %s", "time", tmTime->tm_mday, tmTime->tm_mon+1, tmTime->tm_year+1900, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec, ctime(&tim));

					timeset = true;
					eventServer->sendEvent(CSectionsdClient::EVT_TIMESET, CEventServer::INITID_SECTIONSD, &tim, sizeof(tim));
				}
			}
			
			if (timeset && first_time)
			{
				first_time = false;

				/*
				 * automatically restart scanning of events, because
				 * current events were most likely ignored as they seem 
				 * to be too far in the future (cf. secondsToCache)
				 */
				// -- do not trash read events, cleanup will be done hopefully
				// -- by housekeeping anyway  (rasc (2005-05-02)
				// dmxEIT.change(0);
				// dmxSDT.change(0);
			}
			else {
				if (timeset) {
					seconds = 60 * 30;
					dprintf("[%sThread] - dmxTOT: going to sleep for %d seconds.\n", "time", seconds);
				}
				else if (!scanning){
					seconds = 60;
				}
				else {
					seconds = 1;
				}
				
				while (seconds)
					seconds = sleep(seconds);
			}
		}
	}
	catch (std::exception& e)
	{
		fprintf(stderr, "Caught std-exception in time-thread %s!\n", e.what());
	}
	catch (...)
	{
		fprintf(stderr, "Caught exception in time-thread!\n");
	}

	dprintf("time-thread ended\n");

	pthread_exit(NULL);
}

//---------------------------------------------------------------------
//			EIT-thread
// reads EPG-datas
//---------------------------------------------------------------------
static void *eitThread(void *)
{

	struct SI_section_header header;
	char *buf;
	unsigned timeoutInMSeconds = 500;
	bool sendToSleepNow = false;
	//dmxEIT.addfilter( 0x4e, (0xff) );
	//dmxEIT.addfilter( 0x4f, (0xff) );
/*	
	dmxEIT.addfilter( 0x4e, (0xff - 0x01) );
	dmxEIT.addfilter( 0x50, (0xff) );
	dmxEIT.addfilter( 0x51, (0xff) );
	dmxEIT.addfilter( 0x52, (0xff - 0x01) );
	dmxEIT.addfilter( 0x54, (0xff - 0x03) );
	dmxEIT.addfilter( 0x58, (0xff - 0x03) );
	dmxEIT.addfilter( 0x5c, (0xff - 0x03) );
	dmxEIT.addfilter( 0x60, (0xff - 0x03) );
	dmxEIT.addfilter( 0x64, (0xff - 0x03) );
	dmxEIT.addfilter( 0x68, (0xff - 0x03) );
	dmxEIT.addfilter( 0x6c, (0xff - 0x03) );
*/
	dmxEIT.addfilter( 0x4e, 0xfe );
	dmxEIT.addfilter( 0x50, 0xe0 );
	try
	{
		dprintf("[%sThread] pid %d start\n", "eit", getpid());
		int timeoutsDMX = 0;
		time_t lastRestarted = time(NULL);
		dmxEIT.start(); // -> unlock

		for (;;)
		{

			time_t zeit = time(NULL);

			if (timeoutsDMX >= CHECK_RESTART_DMX_AFTER_TIMEOUTS - 1)
			{
				lockServices();
				lockMessaging();

				MySIservicesOrderUniqueKey::iterator si = mySIservicesOrderUniqueKey.end();
				//dprintf("timeoutsDMX %x\n",currentServiceKey);

				if ( messaging_current_servicekey )
					si = mySIservicesOrderUniqueKey.find( messaging_current_servicekey );

				if (si != mySIservicesOrderUniqueKey.end())
				{
					if ( ( ( dmxEIT.filter_index == 0 ) && ( !si->second->eitPresentFollowingFlag() ) ) ||
					        ( ( dmxEIT.filter_index == 1 ) && ( !si->second->eitScheduleFlag() ) ) )
					{
						timeoutsDMX = 0;
						dprintf("[eitThread] timeoutsDMX for 0x"
							PRINTF_CHANNEL_ID_TYPE_NO_LEADING_ZEROS
							" reset to 0 (not broadcast)\n", messaging_current_servicekey );

						dprintf("New Filterindex: %d (ges. %d)\n", dmxEIT.filter_index + 1, (signed) dmxEIT.filters.size() );
						dmxEIT.change( dmxEIT.filter_index + 1 );
					}
					else
						if ( dmxEIT.filter_index > 1 )
						{
							bool dont_change = false;

							for ( int i = (dmxEIT.filters[dmxEIT.filter_index].filter & dmxEIT.filters[dmxEIT.filter_index].mask); i <= dmxEIT.filters[dmxEIT.filter_index].filter; i++)
							{
								//dprintf("%x - %x |", i, messaging_sections_max_ID[i- 0x4e]);

								if ( messaging_sections_max_ID[i - 0x4e] != -1 )
								{
									dont_change = true;
									break;
								}
							}

							dprintf("dontchange %d\n", dont_change);

							if ( !dont_change )
							{
								if ( dmxEIT.filter_index + 1 < (signed) dmxEIT.filters.size() )
								{
									dprintf("New Filterindex: %d (ges. %d)\n", dmxEIT.filter_index + 1, (signed) dmxEIT.filters.size() );
									dmxEIT.change(dmxEIT.filter_index + 1);
									//dprintf("[eitThread] timeoutsDMX for 0x%x reset to 0 (skipping to next filter)\n" );

									timeoutsDMX = 0;
								}
								else
								{
									sendToSleepNow = true;
									dputs("sendToSleepNow = true")
								}
							}
						}
				}

				unlockMessaging();
				unlockServices();
			}

			if (timeoutsDMX >= CHECK_RESTART_DMX_AFTER_TIMEOUTS && scanning)
			{
				if ( (zeit > lastRestarted + 3) || (dmxEIT.real_pauseCounter != 0) ) // letzter restart l�nger als 3secs her, daher cache NICHT verkleinern
				{
					dmxEIT.stop(); // -> lock
					dmxEIT.start(); // -> unlock
					dprintf("[eitThread] dmxEIT restarted, cache NOT decreased (dt=%ld)\n", (int)zeit - lastRestarted);
				}
				else
				{

					// sectionsd ist zu langsam, da zu viele events -> cache kleiner machen
					dmxEIT.stop(); // -> lock
					/*                    lockEvents();
					                    if(secondsToCache>24*60L*60L && mySIeventsOrderUniqueKey.size()>3000)
					                    {
					                        // kleiner als 1 Tag machen wir den Cache nicht,
					                        // da die timeouts ja auch von einem Sender ohne EPG kommen koennen
					                        // Die 3000 sind ne Annahme und beruhen auf (wenigen) Erfahrungswerten
					                        // Man koennte auch ab 3000 Events nur noch jedes 3 Event o.ae. einsortieren
					                        dmxSDT.real_pause();
					                        lockServices();
					                        unsigned anzEventsAlt=mySIeventsOrderUniqueKey.size();
					                        secondsToCache-=5*60L*60L; // 5h weniger
					                        dprintf("[eitThread] decreasing cache 5h (now %ldh)\n", secondsToCache/(60*60L));
					                        removeNewEvents();
					                        removeOldEvents(oldEventsAre);
					                        if(anzEventsAlt>mySIeventsOrderUniqueKey.size())
					                            dprintf("[eitThread] Removed %u Events (%u -> %u)\n", anzEventsAlt-mySIeventsOrderUniqueKey.size(), anzEventsAlt, mySIeventsOrderUniqueKey.size());
					                        unlockServices();
					                        dmxSDT.real_unpause();
					                    }
					                    unlockEvents();
					*/
					dmxEIT.start(); // -> unlock
					dputs("[eitThread] dmxEIT restarted");

				}

				lastRestarted = zeit;
				timeoutsDMX = 0;
			}

			if (timeset)
			{
				// Nur wenn ne richtige Uhrzeit da ist

				if ( (sendToSleepNow) )
				{
					sendToSleepNow = false;

					struct timespec abs_wait;

					struct timeval now;

					gettimeofday(&now, NULL);
					TIMEVAL_TO_TIMESPEC(&now, &abs_wait);
					abs_wait.tv_sec += (TIME_EIT_SCHEDULED_PAUSE);

					dmxEIT.real_pause();
					pthread_mutex_lock( &dmxEIT.start_stop_mutex );
					dprintf("dmxEIT: going to sleep...\n");

					int rs = pthread_cond_timedwait( &dmxEIT.change_cond, &dmxEIT.start_stop_mutex, &abs_wait );

					if (rs == ETIMEDOUT)
					{
						dprintf("dmxEIT: waking up again - looking for new events :)\n");
						pthread_mutex_unlock( &dmxEIT.start_stop_mutex );
						
#ifdef PAUSE_EQUALS_STOP
						dmxEIT.real_unpause();
#endif
// must call dmxEIT.change after! unpause otherwise dev is not open, 
// dmxEIT.lastChanded will not be set, and filter is advanced the next iteration
						dprintf("New Filterindex3: %d (ges. %d)\n", 0, (signed) dmxEIT.filters.size() );
						dmxEIT.change( 0 ); // -> restart
					}
					else if (rs == 0)
					{
						dprintf("dmxEIT: waking up again - requested from .change()\n");
						pthread_mutex_unlock( &dmxEIT.start_stop_mutex );
#ifdef PAUSE_EQUALS_STOP
						dmxEIT.real_unpause();
#endif
					}
					else
					{
						dprintf("dmxEIT:  waking up again - unknown reason?!\n");
						pthread_mutex_unlock( &dmxEIT.start_stop_mutex );
						dmxEIT.real_unpause();
					}
					// update zeit after sleep
					zeit = time(NULL);
				}
				else if (zeit > dmxEIT.lastChanged + TIME_EIT_SKIPPING )
				{
					lockMessaging();

					if ( dmxEIT.filter_index + 1 < (signed) dmxEIT.filters.size() )
					{
						dprintf("[eitThread] skipping to next filter(%d) (> TIME_EIT_SKIPPING)\n", dmxEIT.filter_index+1 );
						dmxEIT.change(dmxEIT.filter_index + 1);
					}
					else
						sendToSleepNow = true;

					unlockMessaging();
				};
			}

			buf = dmxEIT.getSection(timeoutInMSeconds, timeoutsDMX);

			if (buf == NULL)
				continue;

			unsigned short section_length = (((SI_section_header*)buf)->section_length_hi << 8) |
							((SI_section_header*)buf)->section_length_lo;
			
			// copy the header
			memcpy(&header, buf, std::min((unsigned)section_length + 3, sizeof(header)));

			if ((header.current_next_indicator) && (!dmxEIT.pauseCounter ))
			{
				// Wir wollen nur aktuelle sections

				/*                // Zum debuggen
				        //        if(dmxEIT.isScheduled)
				                {
				                    printf("%hhx\n", header.section_number);
				                    //if(header.section_number==0xb1)
				                    {
				                        printf("length: %hd\n", header.section_length);
				                        dmxEIT.pause();
				                        char    dn[100];
				                        sprintf(dn, "eit.debug.%d", header.section_number);
				                        FILE *file=fopen(dn, "wb");
				                        if(file)
				                        {
				                            fwrite(buf, sizeof(header)+header.section_length-5, 1, file);
				                            fclose(file);
				                        }
				                        dmxEIT.unpause();
				                    }
				                }
				*/

				SIsectionEIT eit(SIsection(section_length + 3, buf));

				if (eit.header())
				{
					// == 0 -> kein event

					dprintf("[eitThread] adding %d events [table 0x%x] (begin)\n", eit.events().size(), header.table_id);

					zeit = time(NULL);
					// Nicht alle Events speichern

					for (SIevents::iterator e = eit.events().begin(); e != eit.events().end(); e++)
					{
						if (!(e->times.empty()))
						{
							if ( ( e->times.begin()->startzeit < zeit + secondsToCache ) &&
							        ( ( e->times.begin()->startzeit + (long)e->times.begin()->dauer ) > zeit - oldEventsAre ) )
							{
								lockEvents();
								addEvent(*e);
								unlockEvents();
							}
						}
						else
						{
							// pruefen ob nvod event
							lockServices();
							MySIservicesNVODorderUniqueKey::iterator si = mySIservicesNVODorderUniqueKey.find(e->get_channel_id());

							if (si != mySIservicesNVODorderUniqueKey.end())
							{
								// Ist ein nvod-event
								lockEvents();

								for (SInvodReferences::iterator i = si->second->nvods.begin(); i != si->second->nvods.end(); i++)
									mySIeventUniqueKeysMetaOrderServiceUniqueKey.insert(std::make_pair(i->uniqueKey(), e->uniqueKey()));

								addNVODevent(*e);

								unlockEvents();
							}
							unlockServices();
						}


					} // for

					//dprintf("[eitThread] added %d events (end)\n",  eit.events().size());

				} // if

				lockMessaging();

				if ((header.table_id != 0x4e) ||
				        (((header.table_id_extension_hi << 8) | header.table_id_extension_lo) ==
					 (messaging_current_servicekey & 0xFFFF)))
				{
					if (!messaging_sections_got_all[header.table_id - 0x4e])
					{
						long long _id = (((unsigned long long)header.table_id) << 40) +
						                (((unsigned long long)header.table_id_extension_hi) << 32) +
						                (((unsigned long long)header.table_id_extension_lo) << 24) +
						                (((unsigned long long)header.section_number) << 16) +
						                (((unsigned long long)header.version_number) << 8) +
						                (((unsigned long long)header.current_next_indicator));

						if (messaging_sections_max_ID[header.table_id - 0x4e] == -1)
						{
							messaging_sections_max_ID[header.table_id - 0x4e] = _id;
						}
						else
						{
							if (!messaging_sections_got_all[header.table_id - 0x4e])
							{
								for ( std::vector<long long>::iterator i = messaging_skipped_sections_ID[header.table_id - 0x4e].begin();
								        i != messaging_skipped_sections_ID[header.table_id - 0x4e].end(); ++i )
									if ( *i == _id)
									{
										messaging_skipped_sections_ID[header.table_id - 0x4e].erase(i);
										break;
									}

								if ((messaging_sections_max_ID[header.table_id - 0x4e] == _id) &&
								    (messaging_skipped_sections_ID[header.table_id - 0x4e].empty()))
								{
									// alle pakete f�r den ServiceKey da!
									dprintf("[eitThread] got all packages for table_id 0x%x\n", header.table_id);
									messaging_sections_got_all[header.table_id - 0x4e] = true;
								}
							}
						}

						if ( messaging_wants_current_next_Event && messaging_sections_got_all[0] )
						{
							dprintf("[eitThread] got all current_next - sending event!\n");
							messaging_wants_current_next_Event = false;
							eventServer->sendEvent(CSectionsdClient::EVT_GOT_CN_EPG, CEventServer::INITID_SECTIONSD, &messaging_current_servicekey, sizeof(messaging_current_servicekey) );
						}

						// �berpr�fen, ob n�chster Filter gew�nscht :)
						int	change_filter = 0;

						for ( int i = (dmxEIT.filters[dmxEIT.filter_index].filter & dmxEIT.filters[dmxEIT.filter_index].mask); i <= ( dmxEIT.filters[dmxEIT.filter_index].filter | ( !dmxEIT.filters[dmxEIT.filter_index].mask ) ); i++)
						{
							if ( messaging_sections_got_all[i - 0x4e] )
								change_filter++;
							else
								if ( messaging_sections_max_ID[i - 0x4e] != -1 )
								{
									change_filter = -1;
									break;
								}
						}

						if ( change_filter > 0 )
						{
							if ( dmxEIT.filter_index + 1 < (signed) dmxEIT.filters.size() ) {
								dprintf("New Filterindex: %d (ges. %d)\n", dmxEIT.filter_index + 1, (signed) dmxEIT.filters.size() );
								dmxEIT.change(dmxEIT.filter_index + 1);
							}
							else
								sendToSleepNow = true;
						}
					}
				}

				unlockMessaging();

			} // if
			else
			{
				lockMessaging();
				//SI_section_EIT_header* _header = (SI_section_EIT_header*) & header;

				long long _id = (((unsigned long long)header.table_id) << 40) +
				                (((unsigned long long)header.table_id_extension_hi) << 32) +
				                (((unsigned long long)header.table_id_extension_lo) << 24) +
				                (((unsigned long long)header.section_number) << 16) +
				                (((unsigned long long)header.version_number) << 8) +
				                (((unsigned long long)header.current_next_indicator));

				if (messaging_sections_max_ID[header.table_id - 0x4e] != -1)
					messaging_skipped_sections_ID[header.table_id - 0x4e].push_back(_id);

				unlockMessaging();

				delete[] buf;

				dprintf("[eitThread] skipped sections for table 0x%x\n", header.table_id);
			}
		} // for
	} // try
	catch (std::exception& e)
	{
		fprintf(stderr, "[eitThread] Caught std-exception %s!\n", e.what());
	}
	catch (...)
	{
		fprintf(stderr, "[eitThread] Caught exception!\n");
	}

	dputs("[eitThread] end");

	pthread_exit(NULL);
}


//---------------------------------------------------------------------
// Premiere Private EPG Thread
// reads EPG-datas
//---------------------------------------------------------------------

static void *pptThread(void *)
{
	struct SI_section_header header;
	char *buf;
	unsigned timeoutInMSeconds = 500;
	bool sendToSleepNow = false;
	unsigned short start_section = 0;
	unsigned short pptpid=0;

//	dmxPPT.addfilter( 0xa0, (0xff - 0x01) );
	dmxPPT.addfilter( 0xa0, (0xff));
	try
	{
		dprintf("[%sThread] pid %d start\n", "ppt", getpid());
		int timeoutsDMX = 0;
		time_t lastRestarted = time(NULL);
		time_t lastData = time(NULL);
		
		dmxPPT.start(); // -> unlock

		for (;;)
		{

			time_t zeit = time(NULL);

			if (timeoutsDMX >= CHECK_RESTART_DMX_AFTER_TIMEOUTS && scanning)
			{
				if ( (zeit > lastRestarted + 3) || (dmxPPT.real_pauseCounter != 0) ) // letzter restart l�nger als 3secs her, daher cache NICHT verkleinern
				{
					dmxPPT.stop(); // -> lock
					dmxPPT.start(); // -> unlock
					dprintf("[pptThread] dmxPPT restarted, cache NOT decreased (dt=%ld)\n", (int)zeit - lastRestarted);
				}
				else
				{

					// sectionsd ist zu langsam, da zu viele events -> cache kleiner machen
					dmxPPT.stop(); // -> lock
					/*                    lockEvents();
					                    if(secondsToCache>24*60L*60L && mySIeventsOrderUniqueKey.size()>3000)
					                    {
					                        // kleiner als 1 Tag machen wir den Cache nicht,
					                        // da die timeouts ja auch von einem Sender ohne EPG kommen koennen
					                        // Die 3000 sind ne Annahme und beruhen auf (wenigen) Erfahrungswerten
					                        // Man koennte auch ab 3000 Events nur noch jedes 3 Event o.ae. einsortieren
					                        dmxSDT.real_pause();
					                        lockServices();
					                        unsigned anzEventsAlt=mySIeventsOrderUniqueKey.size();
					                        secondsToCache-=5*60L*60L; // 5h weniger
					                        dprintf("[eitThread] decreasing cache 5h (now %ldh)\n", secondsToCache/(60*60L));
					                        removeNewEvents();
					                        removeOldEvents(oldEventsAre);
					                        if(anzEventsAlt>mySIeventsOrderUniqueKey.size())
					                            dprintf("[eitThread] Removed %u Events (%u -> %u)\n", anzEventsAlt-mySIeventsOrderUniqueKey.size(), anzEventsAlt, mySIeventsOrderUniqueKey.size());
					                        unlockServices();
					                        dmxSDT.real_unpause();
					                    }
					                    unlockEvents();
					*/
					dmxPPT.start(); // -> unlock
//					dputs("[pptThread] dmxPPT restarted");

				}

				lastRestarted = zeit;
				timeoutsDMX = 0;
				lastData = zeit;
			}

			if (timeset)
			{
				// Nur wenn ne richtige Uhrzeit da ist
				if ( (sendToSleepNow) )
				{
					sendToSleepNow = false;

					struct timespec abs_wait;

					struct timeval now;

					gettimeofday(&now, NULL);
					TIMEVAL_TO_TIMESPEC(&now, &abs_wait);
					abs_wait.tv_sec += (TIME_EIT_SCHEDULED_PAUSE);

					dmxPPT.real_pause();
					pthread_mutex_lock( &dmxPPT.start_stop_mutex );
					dprintf("[pptThread] going to sleep...\n");

					int rs = pthread_cond_timedwait( &dmxPPT.change_cond, &dmxPPT.start_stop_mutex, &abs_wait );

					if (rs == ETIMEDOUT)
					{
						dprintf("dmxPPT: waking up again - looking for new events :)\n");
						pthread_mutex_unlock( &dmxPPT.start_stop_mutex );
#ifdef PAUSE_EQUALS_STOP
						dmxPPT.real_unpause();
#endif
						dmxPPT.change( 0 ); // -> restart
					}
					else if (rs == 0)
					{
						dprintf("dmxPPT: waking up again - requested from .change()\n");
						pthread_mutex_unlock( &dmxPPT.start_stop_mutex );
#ifdef PAUSE_EQUALS_STOP
						dmxPPT.real_unpause();
#endif
					}
					else
					{
						dprintf("dmxPPT: waking up again - unknown reason?!\n");
						pthread_mutex_unlock( &dmxPPT.start_stop_mutex );
						dmxPPT.real_unpause();
					}
					// after sleeping get actual time
					zeit = time(NULL);
					start_section = 0; // fetch new? events
					lastData = zeit; // restart timer
					
				}
			}

			if (0 == privatePid)
			{
				sendToSleepNow = true; // if there is no valid pid -> sleep
				dprintf("dmxPPT: no valid pid 0\n");
				sleep(1);
				continue;
			}
         
			if (pptpid != privatePid)
			{
				pptpid = privatePid;
				dprintf("Setting PrivatePid %x\n", pptpid);
				dmxPPT.setPid(pptpid);
			}

			buf = dmxPPT.getSection(timeoutInMSeconds, timeoutsDMX);

			if (buf == NULL) {
				if (zeit > lastData + 5) 
				{
					sendToSleepNow = true; // if there are no data for 5 seconds -> sleep
					dprintf("dmxPPT: no data for 5 seconds\n");
				}
				continue;
			}
			lastData = zeit;
			
			unsigned short section_length = (((SI_section_header*)buf)->section_length_hi << 8) |
							((SI_section_header*)buf)->section_length_lo;
			
			// copy the header
			memcpy(&header, buf, std::min((unsigned)section_length + 3, sizeof(header)));

			if ((header.current_next_indicator) && (!dmxPPT.pauseCounter ))
			{
				// Wir wollen nur aktuelle sections
				if (start_section == 0) start_section = header.section_number;
				else if (start_section == header.section_number) 
				{
					sendToSleepNow = true; // no more scanning
					dprintf("[pptThread] got all sections\n");
					delete[] buf;
					continue;
				}
				SIsectionPPT ppt(SIsection(section_length + 3, buf));

				if (ppt.header())
				{
					// == 0 -> kein event
//					dprintf("[pptThread] adding %d events [table 0x%x] (begin)\n", ppt.events().size(), header.table_id);
//					dprintf("got %d: ", header.section_number);
					zeit = time(NULL);
					// Nicht alle Events speichern
					for (SIevents::iterator e = ppt.events().begin(); e != ppt.events().end(); e++)
					{
						if (!(e->times.empty()))
						{
							for (SItimes::iterator t = e->times.begin(); t != e->times.end(); t++) {
//								if ( ( e->times.begin()->startzeit < zeit + secondsToCache ) &&
//								        ( ( e->times.begin()->startzeit + (long)e->times.begin()->dauer ) > zeit - oldEventsAre ) )
								// add the event if at least one starttime matches
								if ( ( t->startzeit < zeit + secondsToCache ) &&
								        ( ( t->startzeit + (long)t->dauer ) > zeit - oldEventsAre ) )
								{
//									dprintf("chId: " PRINTF_CHANNEL_ID_TYPE " Dauer: %ld, Startzeit: %s", e->get_channel_id(),  (long)e->times.begin()->dauer, ctime(&e->times.begin()->startzeit));
									lockEvents();
									addEvent(*e);
									unlockEvents();
									break; // only add the event once
								}
#if 0									
// why is the following not compiling, fuXX STL								
								else {
									// remove unusable times in event
									SItimes::iterator kt = t;
									t--; // the iterator t points to the last element
									e->times.erase(kt);
								}
#endif							
							}
						}
						else
						{
							// pruefen ob nvod event
							lockServices();
							MySIservicesNVODorderUniqueKey::iterator si = mySIservicesNVODorderUniqueKey.find(e->get_channel_id());

							if (si != mySIservicesNVODorderUniqueKey.end())
							{
								// Ist ein nvod-event
								lockEvents();

								for (SInvodReferences::iterator i = si->second->nvods.begin(); i != si->second->nvods.end(); i++)
									mySIeventUniqueKeysMetaOrderServiceUniqueKey.insert(std::make_pair(i->uniqueKey(), e->uniqueKey()));

								addNVODevent(*e);
								unlockEvents();
							}
							unlockServices();
						}
					} // for
					//dprintf("[eitThread] added %d events (end)\n",  ppt.events().size());
				} // if
			} // if
			else
			{
				delete[] buf;
			}
		} // for
	} // try
	catch (std::exception& e)
	{
		fprintf(stderr, "[pptThread] Caught std-exception %s!\n", e.what());
	}
	catch (...)
	{
		fprintf(stderr, "[pptThread] Caught exception!\n");
	}

	dputs("[pptThread] end");

	pthread_exit(NULL);
}


//---------------------------------------------------------------------
//			housekeeping-thread
// does cleaning on fetched datas
//---------------------------------------------------------------------
static void *houseKeepingThread(void *)
{
	try
	{
		dprintf("housekeeping-thread started.\n");

		for (;;)
		{
			int rc = 5 * 60;  // sleep 5 minutes

			while (rc)
				rc = sleep(rc);

			while (!scanning)
				sleep(1);	// wait for streaming to end...

			dprintf("housekeeping.\n");

			EITThreadsPause();
			
			dmxSDT.pause();

			struct mallinfo speicherinfo1;

			if (debug)
			{
				// Speicher-Info abfragen
				speicherinfo1 = mallinfo();
			}

			lockEvents();

			unsigned anzEventsAlt = mySIeventsOrderUniqueKey.size();
			removeOldEvents(oldEventsAre); // alte Events

			if (mySIeventsOrderUniqueKey.size() != anzEventsAlt)
			{
				dprintf("total size of memory occupied by chunks handed out by malloc: %d\n", speicherinfo1.uordblks);
				dprintf("total bytes memory allocated with `sbrk' by malloc, in bytes: %d (%dkb, %.2fMB)\n", speicherinfo1.arena, speicherinfo1.arena / 1024, (float)speicherinfo1.arena / (1024.*1024));
				dprintf("Removed %d old events.\n", anzEventsAlt - mySIeventsOrderUniqueKey.size());
			}

			dprintf("Number of sptr events (event-ID): %u\n", mySIeventsOrderUniqueKey.size());
			dprintf("Number of sptr events (service-id, start time, event-id): %u\n", mySIeventsOrderServiceUniqueKeyFirstStartTimeEventUniqueKey.size());
			dprintf("Number of sptr events (end time, service-id, event-id): %u\n", mySIeventsOrderFirstEndTimeServiceIDEventUniqueKey.size());
			dprintf("Number of sptr nvod events (event-ID): %u\n", mySIeventsNVODorderUniqueKey.size());
			dprintf("Number of cached meta-services: %u\n", mySIeventUniqueKeysMetaOrderServiceUniqueKey.size());

			unlockEvents();

			if (debug)
			{
				lockServices();
				dprintf("Number of services: %u\n", mySIservicesOrderUniqueKey.size());
				dprintf("Number of services (name): %u\n", mySIservicesOrderServiceName.size());
				dprintf("Number of cached nvod-services: %u\n", mySIservicesNVODorderUniqueKey.size());
				unlockServices();
			}

			if (debug)
			{
				// Speicher-Info abfragen

				struct mallinfo speicherinfo = mallinfo();
				dprintf("total size of memory occupied by chunks handed out by malloc: %d\n", speicherinfo.uordblks);
				dprintf("total bytes memory allocated with `sbrk' by malloc, in bytes: %d (%dkb, %.2fMB)\n", speicherinfo.arena, speicherinfo.arena / 1024, (float)speicherinfo.arena / (1024.*1024));
			}

			dmxSDT.unpause();
			EITThreadsUnPause();

		} // for endlos
	} // try
	catch (std::exception& e)
	{
		fprintf(stderr, "Caught std-exception in housekeeping-thread %s!\n", e.what());
	}
	catch (...)
	{
		fprintf(stderr, "Caught Exception in housekeeping-thread!\n");
	}

	dprintf("housekeeping-thread ended.\n");

	pthread_exit(NULL);
}

static void printHelp(void)
{
	printf("\nUsage: sectionsd [-d]\n\n");
}

// Just to get our listen socket closed cleanly
static void signalHandler(int signum)
{
	switch (signum)
	{

	case SIGHUP:
		break;

	default:
		exit(0);
	}
}

int main(int argc, char **argv)
{
	pthread_t threadTOT, threadEIT, threadSDT, threadHouseKeeping, threadPPT;
	int rc;

	printf("$Id: sectionsd.cpp,v 1.193 2005/10/02 21:27:58 mogway Exp $\n");

	try {
		if (argc != 1 && argc != 2) {
			printHelp();
			return EXIT_FAILURE;
		}

		if (argc == 2) {
			if (!strcmp(argv[1], "-d"))
				debug = 1;
			else {
				printHelp();
				return EXIT_FAILURE;
			}
		}

		printf("caching %ld hours\n", secondsToCache / (60*60L));
		printf("events are old %ldmin after their end time\n", oldEventsAre / 60);
		tzset(); // TZ auswerten

		CBasicServer sectionsd_server;

		if (!sectionsd_server.prepare(SECTIONSD_UDS_NAME))
			return EXIT_FAILURE;

		if (!debug) {
			switch (fork()) { // switching to background
			case -1:
				perror("[sectionsd] fork");
				return EXIT_FAILURE;
			case 0:
				break;
			default:
				return EXIT_SUCCESS;
			}

			if (setsid() == -1) {
				perror("[sectionsd] setsid");
				return EXIT_FAILURE;
			}
		}

		// from here on forked

		signal(SIGHUP, signalHandler);

		eventServer = new CEventServer;
		/*
				timerdClient = new CTimerdClient;

				printf("[sectionsd ] checking timerd\n");
				timerd = timerdClient->isTimerdAvailable();
				if (timerd)
					printf("[sectionsd ] timerd available\n");
				else
					printf("[sectionsd ] timerd NOT available\n");
		*/
		// SDT-Thread starten
		rc = pthread_create(&threadSDT, 0, sdtThread, 0);

		if (rc) {
			fprintf(stderr, "[sectionsd] failed to create sdt-thread (rc=%d)\n", rc);
			return EXIT_FAILURE;
		}

		// EIT-Thread starten
		rc = pthread_create(&threadEIT, 0, eitThread, 0);

		if (rc) {
			fprintf(stderr, "[sectionsd] failed to create eit-thread (rc=%d)\n", rc);
			return EXIT_FAILURE;
		}

		// time-Thread starten
		rc = pthread_create(&threadTOT, 0, timeThread, 0);

		if (rc) {
			fprintf(stderr, "[sectionsd] failed to create time-thread (rc=%d)\n", rc);
			return EXIT_FAILURE;
		}

		// premiere private epg -Thread starten
		rc = pthread_create(&threadPPT, 0, pptThread, 0);

		if (rc) {
			fprintf(stderr, "[sectionsd] failed to create ppt-thread (rc=%d)\n", rc);
			return EXIT_FAILURE;
		}

		// housekeeping-Thread starten
		rc = pthread_create(&threadHouseKeeping, 0, houseKeepingThread, 0);

		if (rc) {
			fprintf(stderr, "[sectionsd] failed to create houskeeping-thread (rc=%d)\n", rc);
			return EXIT_FAILURE;
		}

		pthread_attr_t conn_attrs;
		pthread_attr_init(&conn_attrs);
		pthread_attr_setdetachstate(&conn_attrs, PTHREAD_CREATE_DETACHED);

		sectionsd_server.run(parse_command, sectionsd::ACTVERSION);
	} // try
	catch (std::exception& e)
	{
		fprintf(stderr, "[sectionsd] Caught std-exception %s in main-thread!\n", e.what());
	}
	catch (...)
	{
		fprintf(stderr, "[sectionsd] Caught exception in main-thread!\n");
	}

	puts("[sectionsd] ended");

	return EXIT_SUCCESS;
}


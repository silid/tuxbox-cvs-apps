#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <set>

#include <core/dvb/dvb.h>
#include <core/dvb/si.h>
#include <core/dvb/frontend.h>
#include <core/system/econfig.h>

eTransponderList* eTransponderList::instance=0;

static struct
{
	int original_network_id, service_id, channel_number;
} defaultChannelNumbers[]=
	{{0x0001, 0x6dca,  1},		// "Das Erste"
	 {0x0001, 0x6d66,  2},		// "ZDF"
	 {0x0001, 0x6e40,  3},		// "N3" *hihi* -> sollte vielleicht raus damit entry #3 nicht blockiert wird.
	 {0x0001, 0x6d67,  4},		// "3sat"
	 {0x0085, 0x0383,  5},		// "KABEL1"
	 {0x0085, 0x002e,  6},		// "Sat.1"
	 {0x0085, 0x0382,  7}, 		// "ProSieben"
	 {0x0001, 0x2ee3,  8}, 		// "RTL"
	 {0x0085, 0x0381,  9},		// "NEUN LIV Television"
	 {0x0085, 0x0034, 10}, 		// "N24"
	 {0x0001, 0x31ba, 11},		// "n-tv"
	 {0x0001, 0x2f1c, 12},		// "VOX"
	 {0x0001, 0x2f08, 13},		// "Super RTL"
	 {0x0001, 0x2ef4, 13},		// "RTL2"
	 {0x0001, 0x31bc, 20},		// "VIVA"
	 {0x0001, 0x2f58, 21},		// "VIVA ZWEI"
	 {0x0001, 0x6fe3, 21}			// "MTV Central"
	};

static int beautifyChannelNumber(int transport_stream_id, int original_network_id, int service_id)
{
	for (unsigned int i=0; i<sizeof(defaultChannelNumbers)/sizeof(*defaultChannelNumbers); i++)
		if ((defaultChannelNumbers[i].original_network_id==original_network_id) &&
				(defaultChannelNumbers[i].service_id==service_id))
			return defaultChannelNumbers[i].channel_number;	
	if ((original_network_id==0x0001) && (service_id>=0x6D80) && (service_id<=0x6F40))		// lame ARD/ZDF heuristic
		return 30;

	if (original_network_id==0x0085)
			/*
				eigentlich mag ich das ja nicht so gerne, weil auf diese weise der komplette kirch-krams oben steht.
				aber anders gehts wohl leider nicht.
			*/
		return 100;
	return -1;
}

#if 0
static std::string escape(const std::string s)
{
	std::string t;
	for (unsigned int i=0; i<s.length(); i++)
	{
		char c = s[i];
		switch (c)
		{
		case '&':
			t+="&amp;";
			break;
		case '\"':
			t+="&quot;";
			break;
		case '<':
			t+="&lt;";
			break;
		default:
			t+=c;
		}
	}
	return t;
}
#endif

void eTransponder::cable::set(const CableDeliverySystemDescriptor *descriptor)
{
			// reject <100Mhz, >1000Mhz
	if (descriptor->frequency < 100*1000)
		return;
	if (descriptor->frequency > 1000*1000)
		return;
	frequency=descriptor->frequency;
	symbol_rate=descriptor->symbol_rate;
	modulation=descriptor->modulation;
	fec_inner=descriptor->FEC_inner;
	inversion=0;
	valid=1;
}

int eTransponder::cable::tune(eTransponder *trans)
{
	eDebug("[TUNE] tuning to %d/%d", frequency, symbol_rate);
	int inv=0;
	return eFrontend::getInstance()->tune_qam(trans, frequency, symbol_rate, fec_inner, inv, modulation);
}

void eTransponder::satellite::set(const SatelliteDeliverySystemDescriptor *descriptor)
{
	if (descriptor->frequency < 7000*1000)		// ACHTUNG wegen C-Band
		return;
	frequency=descriptor->frequency;
	symbol_rate=descriptor->symbol_rate;
	polarisation=descriptor->polarisation;
	fec=descriptor->FEC_inner;
	orbital_position=descriptor->orbital_position;
	if (!descriptor->west_east_flag)
		orbital_position=-orbital_position;
//	eDebug("%d %d", descriptor->orbital_position, descriptor->west_east_flag);
	inversion=0;
	valid=1;
}

int eTransponder::satellite::tune(eTransponder *trans)
{
	eDebug("[TUNE] tuning to %d/%d/%s/%d@%d", frequency, symbol_rate, polarisation?"V":"H", fec, orbital_position);
	int inv=0;
	eSatellite *sat=trans->tplist.findSatellite(orbital_position);
	if (!sat)
	{
		eDebug("couldn't find sat %d..", orbital_position);
		return -ENOENT;
	}
	
	eLNB &lnb=sat->getLNB();
	
	return eFrontend::getInstance()->tune_qpsk(trans, frequency, polarisation, symbol_rate, fec, inv, lnb);
}

eService::eService(eTransportStreamID transport_stream_id, eOriginalNetworkID original_network_id, eServiceID service_id, int service_number):
		transport_stream_id(transport_stream_id), original_network_id(original_network_id), service_id(service_id), service_number(service_number)
{
	clearCache();
}

eService::eService(eTransportStreamID transport_stream_id, eOriginalNetworkID original_network_id, const SDTEntry *sdtentry, int service_number):
		transport_stream_id(transport_stream_id), original_network_id(original_network_id), service_number(service_number)
{
	clearCache();
	service_id=sdtentry->service_id;
	update(sdtentry);
}


void eBouquet::add(const eServiceReference &service)
{
	list.push_back(service);
}

int eBouquet::remove(const eServiceReference &service)
{
	list.remove(service);
	return 0;
}

eTransponder::eTransponder(eTransponderList &tplist, eTransportStreamID transport_stream_id, eOriginalNetworkID original_network_id):
	tplist(tplist), transport_stream_id(transport_stream_id), original_network_id(original_network_id)
{
	cable.valid=0;
	satellite.valid=0;
	state=stateToScan;
}

eTransponder::eTransponder(eTransponderList &tplist): tplist(tplist), transport_stream_id(-1), original_network_id(-1)
{
	cable.valid=satellite.valid=0;
	state=stateToScan;
}

void eTransponder::setSatellite(int frequency, int symbol_rate, int polarisation, int fec, int orbital_position, int inversion)
{
	satellite.frequency=frequency;
	satellite.symbol_rate=symbol_rate;
	satellite.polarisation=polarisation;
	satellite.fec=fec;
	satellite.orbital_position=orbital_position;
	satellite.valid=1;
	satellite.inversion=inversion;
}

void eTransponder::setCable(int frequency, int symbol_rate, int inversion)
{
	cable.frequency=frequency;
	cable.symbol_rate=symbol_rate;
	cable.inversion=inversion;
	cable.valid=1;
}

int eTransponder::tune()
{
	switch (eFrontend::getInstance()->Type())
	{
	case eFrontend::feCable:
		if (cable.isValid())
			return cable.tune(this);
		else
			return -ENOENT;
	case eFrontend::feSatellite:
		if (satellite.isValid())
			return satellite.tune(this);
		else
			return -ENOENT;
	default:
		return -ENOSYS;
	}
}

int eTransponder::isValid()
{
	switch (eFrontend::getInstance()->Type())
	{
	case eFrontend::feCable:
		return cable.isValid();
	case eFrontend::feSatellite:
		return satellite.isValid();
	default:
		return 0;
	}
}

void eService::update(const SDTEntry *sdtentry)
{
	if (eServiceID(sdtentry->service_id) != service_id)
	{
		eDebug("tried to update sid %x with sdt-sid %x", service_id.get(), sdtentry->service_id);
		return;
	}
	service_name="unknown";
	service_provider="unknown";
	service_type=0;
	service_id=sdtentry->service_id;
	for (ePtrList<Descriptor>::const_iterator d(sdtentry->descriptors); d != sdtentry->descriptors.end(); ++d)
		if (d->Tag()==DESCR_SERVICE)
		{
			service_name="";
			const ServiceDescriptor *nd=(ServiceDescriptor*)*d;
		
	  	int i=0;
			char ch;
			while ((ch=nd->service_name[i++]))
			{
				if (ch<32)
					continue;
				if (ch==0x86)
					continue;
				if (ch==0x87)
					continue;
				service_name+=ch;
			}
			if (!service_name.length())
				service_name="no name";

			service_provider=nd->service_provider;

			service_type=nd->service_type;
		}
//	printf("%04x:%04x %02x %s", transport_stream_id, service_id, service_type, (const char*)service_name);
}

eSatellite::eSatellite(eTransponderList &tplist, int orbital_position, eLNB &lnb):
		tplist(tplist), orbital_position(orbital_position), lnb(lnb)
{
	tpiterator=tplist.satellites.insert(std::pair<int,eSatellite*>(orbital_position,this)).first;
}

eSatellite::~eSatellite()
{
	tplist.satellites.erase(tpiterator);
}

eSatellite *eLNB::addSatellite(int orbital_position)
{
	satellites.push_back(new eSatellite(tplist, orbital_position, *this));
	return satellites.back();
}

void eLNB::deleteSatellite(eSatellite *satellite)
{
	satellites.remove(satellite);
}

eTransponderList::eTransponderList()
{
	if (!instance)
		instance = this;
	
	readLNBData();
}

eTransponder &eTransponderList::createTransponder(eTransportStreamID transport_stream_id, eOriginalNetworkID original_network_id)
{
	std::map<tsref,eTransponder>::iterator i=transponders.find(tsref(transport_stream_id, original_network_id));
	if (i==transponders.end())
	{
		i=transponders.insert(
				std::pair<tsref,eTransponder>
					(tsref(transport_stream_id, original_network_id),
						eTransponder(*this, transport_stream_id, original_network_id)
					)
			).first;
		/*emit*/ transponder_added(&(*i).second);
	}
	return (*i).second;
}

eService &eTransponderList::createService(const eServiceReference &service, int chnum, bool *newService)
{
	std::map<eServiceReference,eService>::iterator i=services.find(service);
                                                  	
	if (newService)
			*newService = ( i == services.end() );

	if ( i == services.end() )
	{
		if (chnum==-1)
			chnum=beautifyChannelNumber(service.transport_stream_id.get(), service.original_network_id.get(), service.service_id.get());
		
		if (chnum==-1)
		{

			if (channel_number.end()==channel_number.begin())
				chnum=200;
			else
			{
				std::map<int,eService*>::iterator i=channel_number.end();
				--i;
				chnum=i->first+1;	// letzte kanalnummer +1
			}
		}

		while (channel_number.find(chnum)!=channel_number.end())
			chnum++;
	
		eService *n=&services.insert(
					std::pair<eServiceReference,eService>
						(service,
						eService(service.transport_stream_id, service.original_network_id, service.service_id, chnum))
					).first->second;

		channel_number.insert(std::pair<int,eService*>(chnum,n));
		
		return *n;
	}
	return (*i).second;
}

int eTransponderList::handleSDT(const SDT *sdt)
{
	std::set<eServiceID> s;
	int changed=0;
	bool newAdded;

	for (ePtrList<SDTEntry>::const_iterator i(sdt->entries); i != sdt->entries.end(); ++i)
	{
		int service_type=-1;
		for (ePtrList<Descriptor>::const_iterator d(i->descriptors); d != i->descriptors.end(); ++d)
			if (d->Tag()==DESCR_SERVICE)
			{
				const ServiceDescriptor *nd=(ServiceDescriptor*)*d;
				service_type=nd->service_type;
			}

		if (service_type == -1)
			continue;
		
		eServiceReference sref=
				eServiceReference(eServiceReference::idDVB, 
					eTransportStreamID(sdt->transport_stream_id), 
					eOriginalNetworkID(sdt->original_network_id), 
					eServiceID(i->service_id),
					service_type);

		eService &service=createService(sref, -1, &newAdded);
		service.update(*i);
		s.insert(eServiceID(i->service_id));

		/*emit*/ service_found(sref, newAdded);
	}

	for (std::map<eServiceReference,eService>::iterator i(services.begin()); i != services.end(); ++i)
		if ((i->first.original_network_id == sdt->original_network_id)	&& // if service on this on
				(i->second.transport_stream_id == sdt->transport_stream_id) && 	// and on this transponder
				(!s.count(i->first.service_id))) // but does not exist
			{
				services.erase(i->first);
				for (std::map<int,eService*>::iterator m(channel_number.begin()); m != channel_number.end(); ++m)
					if (m->second == &i->second)
					{
						channel_number.erase(m);
						break;
					}
				i=services.begin();
				changed=1;
			}
	return changed;
}

eTransponder *eTransponderList::searchTS(eTransportStreamID transport_stream_id, eOriginalNetworkID original_network_id)
{
	std::map<tsref,eTransponder>::iterator i=transponders.find(tsref(transport_stream_id, original_network_id));
	if (i==transponders.end())
		return 0;
		return &i->second;
}

eService *eTransponderList::searchService(const eServiceReference &service)
{
	std::map<eServiceReference,eService>::iterator i=services.find(service);
	if (i==services.end())
		return 0;
	return &i->second;
}

const eServiceReference *eTransponderList::searchService(eOriginalNetworkID original_network_id, eServiceID service_id)
{
	for (std::map<eServiceReference,eService>::iterator i(services.begin()); i != services.end(); ++i)
		if ((i->first.original_network_id == original_network_id) &&
				(i->first.service_id == service_id))
					return &i->first;
	return 0;
}

eService *eTransponderList::searchServiceByNumber(int chnum)
{
	std::map<int,eService*>::iterator i=channel_number.find(chnum);
	if (i==channel_number.end())
		return 0;
	return i->second;
}

eTransponder *eTransponderList::getFirstTransponder(int state)
{
	for (std::map<tsref,eTransponder>::iterator i(transponders.begin()); i!=transponders.end(); ++i)
		if (i->second.state==state)
			return &i->second;
	return 0;
}

eSatellite *eTransponderList::findSatellite(int orbital_position)
{
	eDebug("findSatellite: %d items", satellites.size());
	std::map<int,eSatellite*>::iterator i=satellites.find(orbital_position);
	if (i == satellites.end())
		return 0;
	return i->second;
}

void eTransponderList::readLNBData()
{
	eString basepath="/elitedvb/DVB/config/lnbs/";

	int lnbread=0;
 	while (1)
	{
		unsigned int tmp=0;

		if ( eConfig::getInstance()->getKey( (basepath+eString().setNum(lnbread)+"/lofH").c_str(), tmp) )
			break;
		lnbs.push_back(eLNB(*this));
		eLNB &lnb=lnbs.back();

		lnb.setLOFHi(tmp);

		eConfig::getInstance()->getKey( (basepath+eString().setNum(lnbread)+"/lofL").c_str(), tmp);
		lnb.setLOFLo(tmp);

		eConfig::getInstance()->getKey( (basepath+eString().setNum(lnbread)+"/lofThreshold").c_str(), tmp);
		lnb.setLOFThreshold(tmp);

		int tmpint;
		eConfig::getInstance()->getKey( (basepath+eString().setNum(lnbread)+"/DISEqCpara").c_str(), tmpint);
		lnb.getDiSEqC().sat=tmpint;

		int satread=0;
		while(1)
		{
			char * descr=0;
			if ( eConfig::getInstance()->getKey( (basepath+eString().setNum(lnbread)+"/satellites/"+eString().setNum(satread)+"/OrbitalPosition").c_str(), tmpint) )
				break;  // no satellite for this lnb found

			eConfig::getInstance()->getKey( (basepath+eString().setNum(lnbread)+"/satellites/"+eString().setNum(satread)+"/description").c_str(), descr) ;

			lnb.addSatellite(tmpint)->setDescription(descr);
			satread++;
		}
		lnbread++;
	}

	if (lnbread<1)
	{
		eDebug("couldn't read satellite data.. use default");
		{	
			lnbs.push_back(eLNB(*this));
			eLNB &lnb=lnbs.back();
			lnb.setLOFHi(10600000);
			lnb.setLOFLo(9750000);
			lnb.setLOFThreshold(11700000);
			lnb.getDiSEqC().sat=0;
			lnb.addSatellite(192)->setDescription("Astra 19.2E");
		}
		{
			lnbs.push_back(eLNB(*this));
			eLNB &lnb=lnbs.back();
			lnb.setLOFHi(10600000);
			lnb.setLOFLo(9750000);
			lnb.setLOFThreshold(11700000);
			lnb.getDiSEqC().sat=1;
			lnb.addSatellite(130)->setDescription("Eutelsat 13.0E");
		}
	}
	else
		eDebug("%i lnbs readed", lnbread);
}

void eTransponderList::writeLNBData()
{
	eString basepath="/elitedvb/DVB/config/lnbs/";

	int lnbwrite=0;
	for ( std::list<eLNB>::iterator it( lnbs.begin() ); it != lnbs.end(); it++)
	{
		eConfig::getInstance()->setKey( (basepath+eString().setNum(lnbwrite)+"/lofH").c_str(), it->getLOFHi() );
		eConfig::getInstance()->setKey( (basepath+eString().setNum(lnbwrite)+"/lofL").c_str(), it->getLOFLo() );
		eConfig::getInstance()->setKey( (basepath+eString().setNum(lnbwrite)+"/lofThreshold").c_str(), it->getLOFThreshold() );
		eConfig::getInstance()->setKey( (basepath+eString().setNum(lnbwrite)+"/DISEqCpara").c_str(), it->getDiSEqC().sat );

		int satwrite=0;
		for ( ePtrList<eSatellite>::iterator s ( it->getSatelliteList().begin() ); s != it->getSatelliteList().end(); s++)
		{
			eConfig::getInstance()->setKey( (basepath+eString().setNum(lnbwrite)+"/satellites/"+eString().setNum(satwrite)+"/OrbitalPosition").c_str(), s->getOrbitalPosition() );
			eConfig::getInstance()->setKey( (basepath+eString().setNum(lnbwrite)+"/satellites/"+eString().setNum(satwrite)+"/description").c_str(), s->getDescription().c_str() );
			satwrite++;
		}
		lnbwrite++;
	}
	eDebug("%i LNBs written", lnbwrite);
}

#ifndef __core_dvb_servicedvb_h
#define __core_dvb_servicedvb_h

#include <core/dvb/service.h>
#include <core/dvb/servicecache.h>

class eServiceHandlerDVB: public eServiceHandler
{
	void scrambledStatusChanged(bool);
	void switchedService(const eServiceReferenceDVB &, int);
	void gotEIT(EIT *eit, int);
	void gotSDT(SDT *sdt);
	void gotPMT(PMT *pmt);
	void leaveService(const eServiceReferenceDVB &);
	void aspectRatioChanged(int ratio);
	int flags, state, aspect, error;
	
	eServiceCache<eServiceHandlerDVB> cache;
public:
	int getID() const;
	eServiceHandlerDVB();
	~eServiceHandlerDVB();
	eService *lookupService(const eServiceReference &service);

	int play(const eServiceReference &service);

		// for DVB audio channels:
	PMT *getPMT();
	void setPID(const PMTEntry *);
	
		// for DVB nvod channels:
	SDT *getSDT();

		// for DVB events, nvod, audio....
	EIT *getEIT();
	
	int getFlags();
	int getAspectRatio();
	int getState();
	int getErrorInfo();

	int stop();

	void loadNode(eServiceCache<eServiceHandlerDVB>::eNode &node, const eServiceReference &ref);
	eService *createService(const eServiceReference &node);

	void enterDirectory(const eServiceReference &dir, Signal1<void,const eServiceReference&> &callback);
	void leaveDirectory(const eServiceReference &dir);
};

#endif

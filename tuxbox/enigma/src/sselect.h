#ifndef __sselect_h
#define __sselect_h

#include <core/gui/ewindow.h>
#include <core/gui/listbox.h>
#include <core/gui/statusbar.h>
#include <core/dvb/epgcache.h>
#include "channelinfo.h"

#include <stack>
#include <core/dvb/service.h>

class eService;
class eLabel;

class eListBoxEntryService: public eListBoxEntry
{
	friend class eListBox<eListBoxEntryService>;
	friend struct moveFirstChar;
	friend struct moveServiceNum;
	friend struct _selectService;
	friend struct updateEPGChangedService;
	friend struct renumber;
	eString sort;
	eString short_name;
	static gFont serviceFont, descrFont, numberFont;
	eTextPara *numPara, *namePara, *descrPara;
	int nameXOffs, descrXOffs, numYOffs, nameYOffs, descrYOffs;
	int num;
public:
	int getNum() const { return num; }
	void invalidate();
	void invalidateDescr();
	static int getEntryHeight();
	eServiceReference service;
	eListBoxEntryService(eListBox<eListBoxEntryService> *lb, const eServiceReference &service);
	~eListBoxEntryService();
	
	bool operator<(const eListBoxEntryService &r) const
	{
		if (service.getSortKey() == r.service.getSortKey())
			return sort < r.sort;
		else
			return service.getSortKey() > r.service.getSortKey();		// sort andersrum
	}
protected:
	eString redraw(gPainter *rc, const eRect &rect, gColor, gColor, gColor, gColor, int hilited);
};

class eServiceSelector: public eWindow
{
	eServiceReference selected;
	eServiceReference *result;
	eListBox<eListBoxEntryService> *services;

	eChannelInfo* ci;

	eServicePath path;
	
	void addService(const eServiceReference &service);
protected:
	int eventHandler(const eWidgetEvent &event);
private:
	void fillServiceList(const eServiceReference &ref);
	void entrySelected(eListBoxEntryService *entry);
	void selchanged(eListBoxEntryService *entry);
	char BrowseChar;
	eTimer BrowseTimer;
	void ResetBrowseChar();
	void gotoChar(char c);
public:
	Signal1<void,const eServiceReference &> addServiceToList;
	Signal1<void,eServiceSelector*> showFavourite, showMenu, addServiceToFavourite;
	Signal1<void,int> setMode;
	
	const eServicePath &getPath()	{	return path; }
	void setPath(const eServicePath &path, const eServiceReference &select=eServiceReference());

	enum
	{
		dirNo,
		dirUp,
		dirDown
	};
	void actualize();
	void selectService(const eServiceReference &ref);
	bool selectService(int num);	
	int getServiceNum( const eServiceReference &ref);
	void enterDirectory(const eServiceReference &ref);
	const eServiceReference &getSelected() { return selected; }
	eServiceSelector();
	~eServiceSelector();
	const eServiceReference *choose(int irc=-1);
	const eServiceReference *next();
	const eServiceReference *prev();
	void EPGUpdated( const tmpMap* );
};

#endif

#include <apps/enigma/enigma.h>
#include <apps/enigma/enigma_mainmenu.h>
#include <apps/enigma/enigma_setup.h>
#include <apps/enigma/enigma_plugins.h>
#include <apps/enigma/enigma_info.h>
#include <apps/enigma/enigma_lcd.h>
#include <apps/enigma/enigma_vcr.h>

#include <core/gui/eskin.h>
#include <core/driver/eavswitch.h>
#include <core/gui/elabel.h>
#include <core/dvb/epgcache.h>
#include <core/base/i18n.h>

eMainMenu::eMainMenu()
	:window("enigma" , 8, 200)
{
	window.cmove(ePoint(150, 150));
	CONNECT((new eListBoxEntryMenu(&window.list, _("TV Mode")))->selected, eMainMenu::sel_tv);
	CONNECT((new eListBoxEntryMenu(&window.list, _("Radio Mode")))->selected, eMainMenu::sel_radio);
	CONNECT((new eListBoxEntryMenu(&window.list, _("VCR Mode")))->selected, eMainMenu::sel_vcr);
	CONNECT((new eListBoxEntryMenu(&window.list, _("Plugins")))->selected, eMainMenu::sel_plugins);	
	CONNECT((new eListBoxEntryMenu(&window.list, _("Infos")))->selected, eMainMenu::sel_info);
	CONNECT((new eListBoxEntryMenu(&window.list, _("Setup")))->selected, eMainMenu::sel_setup);
	CONNECT((new eListBoxEntryMenu(&window.list, _("Shutdown")))->selected, eMainMenu::sel_quit);	
}

int eMainMenu::exec()
{
	window.show();
	int res=window.exec();
	window.hide();
	return res;
}

void eMainMenu::sel_tv()
{
  eZap::getInstance()->setMode(eZap::TV);
	window.close(0);
}

void eMainMenu::sel_radio()
{
  eZap::getInstance()->setMode(eZap::Radio);
	window.close(0);
}

void eMainMenu::sel_vcr()
{
	window.hide();
	eAVSwitch::getInstance()->setInput(1);
	enigmaVCR mb("If you can read this, your scartswitch doesn't work", "VCR");
	mb.show();
	eZapLCD *pLCD=eZapLCD::getInstance();
	pLCD->lcdMenu->hide();
	pLCD->lcdScart->show();
	mb.exec();
	pLCD->lcdScart->hide();
	pLCD->lcdMenu->show();
	mb.hide();
	eAVSwitch::getInstance()->setInput(0);
	window.show();
}

void eMainMenu::sel_info()
{
	eZapLCD *pLCD=eZapLCD::getInstance();
	eZapInfo info;
	info.setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
	window.hide();
	info.show();
	info.exec();
	info.hide();
	window.show();
}

void eMainMenu::sel_setup()
{
	eZapLCD *pLCD=eZapLCD::getInstance();
	eZapSetup setup;
	setup.setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
	window.hide();
	setup.show();
	setup.exec();
	setup.hide();
	window.show();
}

void eMainMenu::sel_plugins()
{
	eZapLCD *pLCD=eZapLCD::getInstance();
	eZapPlugins plugins(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
	window.hide();
	plugins.exec();
	window.show();
}

void eMainMenu::sel_quit()
{
	window.close(1);
}

void eMainMenu::setLCD(eWidget *a, eWidget *b)
{
	window.setLCD(a, b);
}

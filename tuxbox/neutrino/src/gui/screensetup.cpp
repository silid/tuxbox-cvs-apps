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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gui/screensetup.h>

#include <gui/color.h>
#include <gui/widget/messagebox.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <system/settings.h>

#include <global.h>
#include <neutrino.h>


CScreenSetup::CScreenSetup()
{
	frameBuffer = CFrameBuffer::getInstance();
}

int CScreenSetup::exec(CMenuTarget* parent, const std::string &)
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int res = menu_return::RETURN_REPAINT;

	if (parent)
	{
		parent->hide();
	}

	x_coord[0] = g_settings.screen_StartX;
	x_coord[1] = g_settings.screen_EndX;
	y_coord[0] = g_settings.screen_StartY;
	y_coord[1] = g_settings.screen_EndY;

	paint();

	selected = 0;

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

	bool loop=true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd, true );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

		switch ( msg )
		{
			case CRCInput::RC_ok:
				// abspeichern
				g_settings.screen_StartX = x_coord[0];
    			g_settings.screen_EndX = x_coord[1];
    			g_settings.screen_StartY = y_coord[0];
    			g_settings.screen_EndY = y_coord[1];
				loop = false;
				break;

			case CRCInput::RC_home:
				if ( ( ( g_settings.screen_StartX != x_coord[0] ) ||
    				   ( g_settings.screen_EndX != x_coord[1] ) ||
    				   ( g_settings.screen_StartY != y_coord[0] ) ||
    				   ( g_settings.screen_EndY != y_coord[1] ) ) &&
				     (ShowMsgUTF(LOCALE_VIDEOMENU_SCREENSETUP, g_Locale->getText(LOCALE_MESSAGEBOX_DISCARD), CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbCancel) == CMessageBox::mbrCancel)) // UTF-8
					break;

			case CRCInput::RC_timeout:
				loop = false;
				break;

			case CRCInput::RC_red:
			case CRCInput::RC_green:
				{
					selected = ( msg == CRCInput::RC_green ) ? 1 : 0 ;

					int x=15*5;
					int y=15*24;

					g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+30,y+29, 15*23, g_Locale->getText("screensetup.upperleft"), (selected == 0)?COL_MENUHEAD:COL_MENUCONTENT, 0, true); // UTF-8
					g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+30,y+49, 15*23, g_Locale->getText("screensetup.lowerright"), (selected == 1)?COL_MENUHEAD:COL_MENUCONTENT, 0, true); // UTF-8
                	break;
                }
			case CRCInput::RC_up:
				{
			    	y_coord[selected]--;

				    int min = ( selected == 0 ) ? 0 : 400;
					if ( y_coord[selected] < min )
						y_coord[selected] = min ;
					else
						paintBorder( selected );
					break;
				}
			case CRCInput::RC_down:
				{
			    	y_coord[selected]++;

			    	int max = ( selected == 0 ) ? 200 : 575;
					if ( y_coord[selected] > max )
						y_coord[selected] = max ;
					else
						paintBorder( selected );
					break;
				}
			case CRCInput::RC_left:
				{
			    	x_coord[selected]--;

				    int min = ( selected == 0 ) ? 0 : 400;
					if ( x_coord[selected] < min )
						x_coord[selected] = min ;
					else
						paintBorder( selected );
					break;
				}
			case CRCInput::RC_right:
				{
			    	x_coord[selected]++;

			    	int max = ( selected == 0 ) ? 200 : 719;
					if ( y_coord[selected] > max )
						y_coord[selected] = max ;
					else
						paintBorder( selected );
					break;
				}

			default:
				if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
				{
					loop = false;
					res = menu_return::RETURN_EXIT_ALL;
				}
		}

	}

	hide();
	return res;
}

void CScreenSetup::hide()
{
	frameBuffer->paintBackgroundBox(0,0,720,576);
}

void CScreenSetup::paintBorder( int selected )
{
	if ( selected == 0 )
		paintBorderUL();
	else
		paintBorderLR();

	paintCoords();
}

void CScreenSetup::paintBorderUL()
{
	frameBuffer->paintIcon( "border_ul.raw", x_coord[0], y_coord[0] );
}

void CScreenSetup::paintBorderLR()
{
	frameBuffer->paintIcon("border_lr.raw", x_coord[1]- 96, y_coord[1]- 96 );
}

void CScreenSetup::paintCoords()
{
	int x=15*19;
	int y=15*16;
	frameBuffer->paintBoxRel(x,y, 15*9,15*6, COL_MENUCONTENT_PLUS_0);
	char xpos[30];
	char ypos[30];
	char xepos[30];
	char yepos[30];

	sprintf((char*) &xpos, "SX: %d",x_coord[0] );
	sprintf((char*) &ypos, "SY: %d", y_coord[0] );
	sprintf((char*) &xepos, "EX: %d", x_coord[1] );
	sprintf((char*) &yepos, "EY: %d", y_coord[1] );

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+10,y+30, 200, xpos, COL_MENUCONTENT);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+10,y+50, 200, ypos, COL_MENUCONTENT);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+10,y+70, 200, xepos, COL_MENUCONTENT);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+10,y+90, 200, yepos, COL_MENUCONTENT);
}

void CScreenSetup::paint()
{
	if (!frameBuffer->getActive())
		return;
	memset(frameBuffer->getFrameBufferPointer(), 8, frameBuffer->getStride()*576);

	for(int count=0;count<576;count+=15)
		frameBuffer->paintHLine(0,719, count, 9 );

	for(int count=0;count<720;count+=15)
		frameBuffer->paintVLine(count,0, 575, 9 );

	frameBuffer->paintBox(0,0, 15*15,15*15, 8);
	frameBuffer->paintBox(32*15+1,23*15+1, 719,575, 8);

	int x=15*5;
	int y=15*24;
	frameBuffer->paintBoxRel(x,y, 15*23,15*4, COL_MENUCONTENT_PLUS_0);

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+30,y+29, 15*23, g_Locale->getText("screensetup.upperleft"), COL_MENUHEAD, 0, true); // UTF-8
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+30,y+49, 15*23, g_Locale->getText("screensetup.lowerright"), COL_MENUCONTENT, 0, true); // UTF-8

	paintBorderUL();
	paintBorderLR();
	paintCoords();
}

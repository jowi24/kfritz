/*
 * KFritz
 *
 * Copyright (C) 2008 Joachim Wilke <vdr@joachim-wilke.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "KFritz.h"

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KMessageBox>
#include <vector>

#include <Config.h>
#include <FonbookManager.h>
#include "KFritzWindow.h"

static const char *VERSION        = "0.1";
//TODO: use KUniqueApplication ?

KFritz::KFritz(QWidget *mainWindow, KAboutData *aboutData)
:KSystemTrayIcon("modem", mainWindow)
{
	this->aboutData = aboutData;

}

KFritz::~KFritz()
{
}


int main (int argc, char *argv[])
{
	// init KDE-stuff
	KAboutData aboutData(
			// The program name used internally.
			"kfritz",
			// The message catalog name
			// If null, program name is used instead.
			0,
			// A displayable program name string.
			ki18n("KFritz"),
			// The program version string.
			VERSION,
			// Short description of what the app does.
			ki18n("Notifies about phone activity and browses call history and telephone book on your Fritz!Box"),
			// The license this code is released under
			KAboutData::License_GPL,
			// Copyright Statement
			ki18n("(c) 2008-2010"),
			// Optional text shown in the About box.
			// Can contain any information desired.
			ki18n("Developed by Matthias Becker and Joachim Wilke."),
			// The program homepage string.
			"http://www.joachim-wilke.de/kfritzbox.htm",
			// The bug report email address
			"kfritzbox@joachim-wilke.de");

	aboutData.setProgramIconName("modem");

	KCmdLineArgs::init( argc, argv, &aboutData );
	KApplication app;

	// create GUI elements, hand-over logArea to mainWindow
	KFritzWindow *mainWindow = new KFritzWindow();
	KFritz *trayIcon 	    = new KFritz(mainWindow, &aboutData);
	trayIcon->show();
//	mainWindow->show();
	return app.exec();
}

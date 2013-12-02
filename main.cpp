/*
 * KFritz
 *
 * Copyright (C) 2010-2012 Joachim Wilke <kfritz@joachim-wilke.de>
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
#include <Config.h>

#include "KSettings.h"

static const char *VERSION        = "0.0.13";

int main (int argc, char *argv[]) {
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
			ki18n("(c) 2008-2012"),
			// Optional text shown in the About box.
			// Can contain any information desired.
			ki18n("Developed by Matthias Becker and Joachim Wilke."),
			// The program homepage string.
			"http://www.joachim-wilke.de/kfritz.html",
			// The bug report email address
			"kfritz@joachim-wilke.de");

	aboutData.setProgramIconName("modem");

	KCmdLineOptions options;
	options.add("p");
	options.add("log-personal-info", ki18n("Log personal information (e.g. passwords, phone numbers, ...)"));
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;

	// create GUI elements, hand-over logArea to mainWindow
	KFritzWindow mainWindow;
	if (!KSettings::startMinimized())
		mainWindow.show();
	KFritz *trayIcon 	    = new KFritz(&mainWindow, &aboutData);
	trayIcon->show();
	int ret = app.exec();
	fritz::Config::Shutdown();
	return ret;
}

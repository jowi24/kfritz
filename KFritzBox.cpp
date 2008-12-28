/*
 * KFritzBox
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

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KMessageBox>
#include <vector>

#include <Config.h>
#include <FonbookManager.h>
#include "KFritzBox.h"
#include "KFritzBoxWindow.h"
#include "KEventHandler.h"

//TODO: use KUniqueApplication ?

KFritzBox::KFritzBox(QWidget *mainWindow, KAboutData *aboutData)
:KSystemTrayIcon("internet-telephony", mainWindow)
{
	this->aboutData = aboutData;

	KEventHandler *eventHandler = new KEventHandler();
	fritz::Listener::CreateListener(eventHandler);
	connect(eventHandler, SIGNAL(notify(QString)), this, SLOT(notify(QString)));

}

KFritzBox::~KFritzBox()
{
	fritz::Listener::DeleteListener();
}

void KFritzBox::notify(QString message) {
	showMessage(aboutData->programName(), message);
	KNotification *notification= new KNotification ( "incomingCall" );
	notification->setText(message);
	notification->sendEvent();
}

int main (int argc, char *argv[])
{
	// init KDE-stuff
	KAboutData aboutData(
			// The program name used internally.
			"kfritzbox",
			// The message catalog name
			// If null, program name is used instead.
			0,
			// A displayable program name string.
			ki18n("KFritz!Box"),
			// The program version string.
			"1.0",
			// Short description of what the app does.
			ki18n("Notify about phone activity on your Fritz!Box"),
			// The license this code is released under
			KAboutData::License_GPL,
			// Copyright Statement
			ki18n("(c) 2008"),
			// Optional text shown in the About box.
			// Can contain any information desired.
			ki18n(""),
			// The program homepage string.
			"http://www.joachim-wilke.de/kfritzbox.htm",
			// The bug report email address
			"kfritzbox@joachim-wilke.de");

	aboutData.setProgramIconName("internet-telephony");

	KCmdLineArgs::init( argc, argv, &aboutData );
	KApplication app;

	// init libfritz++
	fritz::Config::Setup("fritz.box", "JmH44b76");

	std::vector<std::string> vMsn;
	vMsn.push_back("3020431");
	fritz::Config::SetupMsnFilter(vMsn);

	// create GUI elements
	KFritzBoxWindow *mainWindow = new KFritzBoxWindow();
	KFritzBox *trayIcon 	    = new KFritzBox(mainWindow, &aboutData);
	trayIcon->show();


	return app.exec();
}

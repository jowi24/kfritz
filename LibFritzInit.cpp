/*
 * KFritz
 *
 * Copyright (C) 2010 Joachim Wilke <kfritz@joachim-wilke.de>
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

#include "LibFritzInit.h"

#include <CallList.h>
#include <KGlobal>
#include <KCmdLineArgs>
#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KAboutData>
#include <KStandardDirs>

#include <FonbookManager.h>
#include <Config.h>

#include "KSettings.h"
#include "Log.h"

LibFritzInit::LibFritzInit(QString password, fritz::EventHandler *eventHandler) {
	setTerminationEnabled(true);
	this->eventHandler = eventHandler;
	setPassword(password);
}

LibFritzInit::~LibFritzInit() {
}

void LibFritzInit::run() {

	emit ready(false);

	bool locationSettingsDetected;
	std::string countryCode = KSettings::countryCode().toStdString();
	std::string areaCode = KSettings::areaCode().toStdString();
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	if (args->isSet("log-personal-info")) {
		INF("Warning: Logging personal information requested!")
	}
	// start libfritz++
	fritz::Config::Setup(KSettings::hostname().toStdString(), password.toStdString(), args->isSet("log-personal-info"));

	std::vector<std::string> vFonbook;
	QStringList phonebookList = KSettings::phonebookList();
	while (phonebookList.count())
		vFonbook.push_back(phonebookList.takeFirst().toStdString());
	fritz::FonbookManager::CreateFonbookManager(vFonbook, "", false);

	bool validPassword = fritz::Config::Init(&locationSettingsDetected, &countryCode, &areaCode);
	if (!validPassword) {
		emit invalidPassword();
		return;
	}

	if (locationSettingsDetected) {
		KSettings::setCountryCode(QString(countryCode.c_str()));
		KSettings::setAreaCode(QString(areaCode.c_str()));
		KSettings::self()->writeConfig();
	}

	fritz::Config::SetupConfigDir(KStandardDirs::locateLocal("data", KGlobal::mainComponent().aboutData()->appName()+'/').toStdString());

	std::vector<std::string> vMsn;
	QStringList msnList = KSettings::mSNFilter();
	while (msnList.count())
		vMsn.push_back(msnList.takeFirst().toStdString());
	fritz::Config::SetupMsnFilter(vMsn);

	fritz::Listener::CreateListener(eventHandler);

	fritz::CallList::CreateCallList();

	emit ready(true);
}

void LibFritzInit::setPassword(QString password) {
	this->password = password;
}

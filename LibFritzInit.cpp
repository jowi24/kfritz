/*
 * KFritzBox
 *
 * Copyright (C) 2010 Joachim Wilke <vdr@joachim-wilke.de>
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

#include <FonbookManager.h>
#include <Config.h>
#include <CallList.h>
#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>

#include "KSettings.h"

LibFritzInit::LibFritzInit(QString password) {
	eventHandler = NULL;
	setPassword(password);
	start();
}

LibFritzInit::~LibFritzInit() {
	fritz::Listener::DeleteListener();

	if (eventHandler)
		delete eventHandler;
}

void LibFritzInit::run() {

	emit ready(false);

	bool locationSettingsDetected;
	std::string countryCode = KSettings::countryCode().toStdString();
	std::string areaCode = KSettings::areaCode().toStdString();

	// start libfritz++
	bool validPassword = fritz::Config::Setup(KSettings::hostname().toStdString(),
			                                  password.toStdString(),
			                                  &locationSettingsDetected, &countryCode, &areaCode);
	if (!validPassword) {
		emit invalidPassword();
		return;
	}

	if (locationSettingsDetected) {
		KSettings::setCountryCode(QString(countryCode.c_str()));
		KSettings::setAreaCode(QString(areaCode.c_str()));
		KSettings::self()->writeConfig();
	}

	std::vector<std::string> vMsn;
	vMsn.push_back("3020431"); //TODO: solve some other way?
	fritz::Config::SetupMsnFilter(vMsn);

	eventHandler = new KEventHandler();
	fritz::Listener::CreateListener(eventHandler);

	std::vector<std::string> vFonbook;
	vFonbook.push_back("FRITZ");
	vFonbook.push_back("OERT");
	fritz::FonbookManager::CreateFonbookManager(vFonbook, "FRITZ");

	fritz::CallList::CreateCallList();

	emit ready(true);
}

void LibFritzInit::setPassword(QString password) {
	this->password = password;
}

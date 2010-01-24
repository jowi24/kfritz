/*
 * LibFritzInit.cpp
 *
 *  Created on: Jan 22, 2010
 *      Author: jo
 */

#include "LibFritzInit.h"

#include <FonbookManager.h>
#include <Config.h>
#include <CallList.h>
#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>
#include <KWallet/Wallet>

#include "KSettings.h"

LibFritzInit::LibFritzInit() {
	eventHandler = NULL;
	start();
}

LibFritzInit::~LibFritzInit() {
	if (eventHandler)
		delete eventHandler;
	fritz::Listener::DeleteListener();
}

QString LibFritzInit::getBoxPwd() {
//	KWallet::Wallet *wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0);
//	if (wallet) {
//		if (!wallet->hasFolder(aboutData.appName()))
//			wallet->createFolder(aboutData.appName());
//		wallet->setFolder(aboutData.appName());
//
//	} else {
//	}

}

void LibFritzInit::run() {
	emit ready(false);
	bool locationSettingsDetected;
	std::string countryCode = KSettings::countryCode().toStdString();
	std::string areaCode = KSettings::areaCode().toStdString();

	// start libfritz++
	fritz::Config::Setup(KSettings::hostname().toStdString(), "JmH44b76", &locationSettingsDetected, &countryCode, &areaCode);
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

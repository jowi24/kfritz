/*
 * libfritz++
 *
 * Copyright (C) 2007-2010 Joachim Wilke <libfritz@joachim-wilke.de>
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


#include "Config.h"
#include "FonbookManager.h"
#include "Tools.h"
#include <PThread++.h>

namespace fritz {

Config* gConfig = NULL;
std::ostream *dsyslog = &std::clog;
std::ostream *isyslog = &std::cout;
std::ostream *esyslog = &std::cerr;

bool Config::Setup(std::string hostname, std::string password,
		           bool *locationSettingsDetected,
				   std::string *countryCode, std::string *regionCode) {

	if (gConfig)
		delete gConfig;
	gConfig = new Config( hostname, password);
	// preload phone settings from Fritz!Box
	bool validPassword = Tools::GetLocationSettings();
	if (gConfig->getCountryCode().empty() || gConfig->getRegionCode().empty()) {
		if (locationSettingsDetected)
			*locationSettingsDetected = false;
		if (countryCode)
			gConfig->setCountryCode(*countryCode);
		if (regionCode)
			gConfig->setRegionCode(*regionCode);
	} else {
		if (locationSettingsDetected)
			*locationSettingsDetected = true;
		if (countryCode)
			*countryCode = gConfig->getCountryCode();
		if (regionCode)
			*regionCode  = gConfig->getRegionCode();
	}

	// fetch SIP provider names
	Tools::GetSipSettings();

	return validPassword;
}

void Config::SetupPorts ( size_t listener, size_t ui ) {
	if (gConfig) {
		gConfig->mConfig.listenerPort = listener;
		gConfig->mConfig.uiPort = ui;
	}
}

void Config::SetupMsnFilter( std::vector <std::string> vMsn) {
	if (gConfig)
		gConfig->mConfig.msn = vMsn;
}

void Config::SetupConfigDir(std::string dir)
{
	if (gConfig)
	gConfig->mConfig.configDir = dir;
}

void Config::SetupLogging(std::ostream *d, std::ostream *i, std::ostream *e) {
	// set own logging objects
	dsyslog = d;
	isyslog = i;
	esyslog = e;
	// delegate to libpthread
	pthread::SetupLogging(d, i, e);
}

Config::Config( std::string url, std::string password) {
	mConfig.url          	= url;
	mConfig.password     	= password;
	mConfig.uiPort       	= 80;
	mConfig.listenerPort    = 1012;
	mConfig.loginType       = UNKNOWN;
	mConfig.lastRequestTime = 0;
	CharSetConv::DetectCharset();
}

Config::~Config() {
}

}

/*
 * libfritz++
 *
 * Copyright (C) 2007-2008 Joachim Wilke <vdr@joachim-wilke.de>
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

void Config::Setup(std::string hostname, std::string password){
	if (gConfig)
		delete gConfig;
	gConfig = new Config( hostname, password);
	// detect interface language of fritz box
	Tools::GetLang();
	// preload phone settings from Fritz!Box
	Tools::GetLocationSettings();
	// fetch SIP provider names
	Tools::GetSipSettings();
}

void Config::SetupMsnFilter( std::vector <std::string> vMsn){
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
	mConfig.url = url;
	mConfig.password = password;
	CharSetConv::DetectCharset();
}

Config::~Config() {
}

}

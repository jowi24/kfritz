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



#ifndef SETUP_H_
#define SETUP_H_

#include <string>
#include <vector>

#define RETRY_DELAY 60
#define MAX_MSN_COUNT 22

struct sFritzboxConfig {
public:
	enum eDirection {
		DIRECTION_IN,
		DIRECTION_OUT,
		DIRECTION_ANY,
	};
	sFritzboxConfig(void);
	bool SetupParseMsn(const char *value);
	bool SetupParseFonbooks(const char *value);
	bool SetupParse(const char *Name, const char *Value);
	std::string configDir;              // path to plugins' config files (e.g., local phone book)
	std::string pluginName;             // name of this plugin (e.g., for cRemote::CallPlugin)
	std::string lang;                   // webinterface language
	std::string url;                    // fritz!box url
	std::string password;               // fritz!box web interface password
	std::string countryCode;            // fritz!box country-code
	std::string regionCode;             // fritz!box region-code
	int reactOnDirection;               // what type of calls are we interested in (eDirection)?
	int muteOnCall;                     // mute audio on calls
	int pauseOnCall;                    // pause playback on calls
	int showNumber;                     // show notification on osd on calls
	int useNotifyOsd;                   // use the extended notification osd and not Skins.Message
	int showNumberInCallList;           // simple or extended details in call lists
	time_t lastKnownMissedCall;         // the time of the last missed call the user is aware of
	int showDaySeparator;               // separate call lists by day
	int hideMainMenu;                   // hide plugins' main menu entry
	std::string activeFonbookID;        // last shown phone book
	std::vector <std::string> msn;      // msn's we are interesed in
	std::vector <std::string> selectedFonbookIDs; // active phone books
};

extern sFritzboxConfig fritzboxConfig;

#endif /*SETUP_H_*/

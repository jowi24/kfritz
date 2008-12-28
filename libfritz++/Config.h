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


#ifndef CONFIG_H_
#define CONFIG_H_

#include <iostream>
#include <vector>
#include <string>

#define RETRY_DELAY 60
#define PORT_WWW 80
#define PORT_MONITOR 1012


namespace fritz {

/**
 * Global config class for libfritz++.
 * This class maintains all configuration information needed by classes part of libfritz++.
 * It is instantiated once automatically, a pointer gConfig is globally available.
 */
class Config {

private:
	struct sConfig{
		std::string configDir;              			// path to plugins' config files (e.g., local phone book)
		std::string lang;                   			// webinterface language
		std::string url;                    			// fritz!box url
		std::string password;               			// fritz!box web interface password
		std::string countryCode;            			// fritz!box country-code
		std::string regionCode;             			// fritz!box region-code
		std::vector <std::string> sipNames;				// the SIP provider names
		std::vector <std::string> msn;      			// msn's we are interesed in
		std::vector <std::string> selectedFonbookIDs; 	// active phone books
		std::string activeFonbook;						// currently selected Fonbook
	}mConfig;

	Config( std::string url, std::string password );

public:
	/**
	 * Initiates the libfritz++ library.
	 * This has to be the first call to libfritz++.
	 * @param the hostname of the Fritz!Box device, defaults to fritz.box
	 * @param the password of the Fritz!Box device, defaults to an empty one
	 */
	void static Setup( std::string hostname="fritz.box", std::string password="" );
	/**
	 * Establishes MSN filtering.
	 * An MsnFilter enables the library to only notify the application on
	 * events which occur on one of the MSNs specified. A call to this method is only
	 * needed if filtering is wanted. Default is no filtering.
	 * @param the list of MSNs to filter on
	 */
	void static SetupMsnFilter(  std::vector <std::string> vMsn );
	/**
	 * Modifies logging channels.
	 * As default, logging of libfritz++ actions is performed using default c++
	 * cout, cerr and clog objects. This method enables the application to redirect
	 * logging to arbitrary ostream objects.
	 * @param the ostream for debbuging messages
	 * @param the ostream for information messages
	 * @param the ostream for error messages
	 */
	void static SetupLogging( std::ostream *dsyslog, std::ostream *isyslog, std::ostream *esyslog );
	/**
	 * Sets up a directory for arbitrary data storage.
	 * This is currently used by local fonbook to persist the fonbook entries to a file.
	 * @param full path to the writable directory
	 */
	void static SetupConfigDir( std::string dir);
	std::string &getConfigDir( )                      { return mConfig.configDir; }
	std::string &getLang( )                           { return mConfig.lang; }
	void setLang( std::string l )                     { mConfig.lang = l; }
	std::string &getUrl( )                            { return mConfig.url; }
	std::string &getPassword( )                       { return mConfig.password; }
	std::string &getCountryCode( )        	          { return mConfig.countryCode; }
	void setCountryCode( std::string cc )             { mConfig.countryCode = cc; }
	std::string &getRegionCode( )                     { return mConfig.regionCode; }
	void setRegionCode( std::string rc )              { mConfig.regionCode = rc; }
	std::vector <std::string> &getSipNames( )         { return mConfig.sipNames; }
	void setSipNames( std::vector<std::string> names) { mConfig.sipNames = names; }
	std::vector <std::string> getMsnFilter( )         { return mConfig.msn; }
	std::vector <std::string> getFonbookIDs( )        { return mConfig.selectedFonbookIDs; }
	void setFonbookIDs(std::vector<std::string> v)    { mConfig.selectedFonbookIDs = v; }
	std::string &getActiveFonbook( )                  { return mConfig.activeFonbook; }
	void setActiveFonbook( std::string f )            { mConfig.activeFonbook = f; }
	virtual ~Config();
};

extern Config* gConfig;
extern std::ostream *dsyslog;
extern std::ostream *isyslog;
extern std::ostream *esyslog;

}

#endif /* CONFIG_H_ */

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


#ifndef CONFIG_H_
#define CONFIG_H_

#include <iostream>
#include <vector>
#include <string>

#define NAMESPACE "libfritz++"
#define LOCATOR "[" << NAMESPACE << "/" <<  \
                std::string(__FILE__, std::string(__FILE__).rfind('/') == std::string::npos ? \
                		          0 : std::string(__FILE__).rfind('/')+1, std::string::npos ) \
                << ":" << __LINE__ << "] "
#define DBG(x) *::fritz::dsyslog << LOCATOR << x << std::endl;
#define INF(x) *::fritz::isyslog << LOCATOR << x << std::endl;
#define ERR(x) *::fritz::esyslog << LOCATOR << x << std::endl;

#define RETRY_DELAY 60

namespace fritz {

/**
 * Global config class for libfritz++.
 * This class maintains all configuration information needed by classes part of libfritz++.
 * It is instantiated once automatically, a pointer gConfig is globally available.
 */
class Config {
public:
	enum eLoginType {
		UNKNOWN,
		PASSWORD,
		SID
	};
private:
	struct sConfig {
		std::string configDir;              			// path to libraries' config files (e.g., local phone book)
		std::string lang;                   			// webinterface language
		std::string url;                    			// fritz!box url
		int uiPort;						                // the port of the fritz box web interface
		int listenerPort;					            // the port of the fritz box call monitor
		std::string password;               			// fritz!box web interface password
		time_t lastRequestTime;                         // with eLoginType::SID: time of last request sent to fritz box
		eLoginType loginType;                           // type of login procedure
		std::string sid;                                // SID to access boxes with firmware >= xx.04.74
		std::string countryCode;            			// fritz!box country-code
		std::string regionCode;             			// fritz!box region-code
		std::vector <std::string> sipNames;				// the SIP provider names
		std::vector <std::string> msn;      			// msn's we are interesed in
		std::vector <std::string> selectedFonbookIDs; 	// active phone books
		std::string activeFonbook;						// currently selected Fonbook
	} mConfig;

	Config( std::string url, std::string password );

public:
	/**
	 * Initiates the libfritz++ library.
	 * This has to be the first call to libfritz++.
	 * @param the hostname of the Fritz!Box device, defaults to fritz.box
	 * @param the password of the Fritz!Box device, defaults to an empty one
	 * @param indicates, whether auto-detection of location settings was successful
	 * @param Sets the default value for countryCode. If locationSettingsDetected == true, this returns the detected countryCode.
	 * @param Sets the default value for regionCode. If locationSettingsDetected == true, this returns the detected regionCode.
	 */
	bool static Setup( std::string hostname="fritz.box", std::string password="",
			           bool *locationSettingsDetected = NULL,
			           std::string *countryCode = NULL, std::string *regionCode = NULL);
	/**
	 * Sets arbitrary ports for connections to the Fritz!Box's listener and webinterface.
	 * @param the port to connect to the listener
	 * @param the port to connect to the webinterface
	 */
	void static SetupPorts ( size_t listener = 1012, size_t ui = 80 );
	/**
	 * Establishes MSN filtering.
	 * An MsnFilter enables the library to only notify the application on
	 * events which occur on one of the MSNs specified. A call to this method is only
	 * needed if filtering is wanted. Default is no filtering.
	 * @param the list of MSNs to filter on
	 */
	void static SetupMsnFilter( std::vector <std::string> vMsn );
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
	int getUiPort( )				                  { return mConfig.uiPort; }
	int getListenerPort( )				              { return mConfig.listenerPort; }
	std::string &getPassword( )                       { return mConfig.password; }
	eLoginType getLoginType( )                        { return mConfig.loginType; }
	void setLoginType(eLoginType type)                { mConfig.loginType = type; }
	time_t getLastRequestTime()                       { return mConfig.lastRequestTime; }
	void updateLastRequestTime()                      { mConfig.lastRequestTime = time(NULL); }
	std::string &getSid( )                            { return mConfig.sid; }
	void setSid(std::string sid)                      { mConfig.sid = sid; }
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

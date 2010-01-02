/*
 * libfritz++
 *
 * Copyright (C) 2007-2009 Joachim Wilke <vdr@joachim-wilke.de>
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

#include <openssl/md5.h>
#include <iomanip>
#include <cstring>

#include "Config.h"
#include "FritzClient.h"

#define RETRY_BEGIN                                  \
    	unsigned int retry_delay = RETRY_DELAY / 2;  \
		bool dataRead = false;                       \
		do {  				                         \
			try {                                    \
				Login();                             \
				retry_delay = retry_delay > 1800 ? 3600 : retry_delay * 2;

#define RETRY_END																							\
			dataRead = true;                                                                                \
		} catch (tcpclient::TcpException te) {																\
			*esyslog << __FILE__ << ": Exception - " << te.what() << std::endl;								\
			*esyslog << __FILE__ << ": waiting " << retry_delay << " seconds before retrying" << std::endl;	\
			sleep(retry_delay); /* delay a possible retry */												\
		}																									\
	} while (!dataRead);

namespace fritz {

pthread::Mutex* FritzClient::mutex = new pthread::Mutex();

FritzClient::FritzClient() {
	mutex->Lock();
}

FritzClient::~FritzClient() {
	mutex->Unlock();
}

std::string FritzClient::CalculateLoginResponse(std::string challenge) {
	std::string challengePwd = challenge + '-' + gConfig->getPassword();
	// the box needs an md5 sum of the string "challenge-password"
	// to make things worse, it needs this in UTF-16LE character set
	// last but not least, for "compatibility" reasons (*LOL*) we have to replace
	// every char > "0xFF 0x00" with "0x2e 0x00"
	CharSetConv conv(NULL, "UTF-16LE");
	char challengePwdConv[challengePwd.length()*2];
	memcpy(challengePwdConv, conv.Convert(challengePwd.c_str()), challengePwd.length()*2);
	for (size_t pos=1; pos < challengePwd.length()*2; pos+= 2)
		if (challengePwdConv[pos] != 0x00) {
			challengePwdConv[pos] = 0x00;
			challengePwdConv[pos-1] = 0x2e;
		}
	unsigned char hash[16];
	MD5((unsigned char*)challengePwdConv, challengePwd.length()*2, hash);
	std::stringstream response;
	response << challenge << '-';
	for (size_t pos=0; pos < 16; pos++)
		response << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)hash[pos];
	return response.str();
}

std::string FritzClient::UrlEncode(std::string &s_input) {
	std::string result;
	std::string s;
	std::string hex = "0123456789abcdef";
	CharSetConv *conv = new CharSetConv(CharSetConv::SystemCharacterTable(), "ISO-8859-15");
	s = conv->Convert(s_input.c_str());
	delete(conv);
	for (unsigned int i=0; i<s.length(); i++) {
		if( ('a' <= s[i] && s[i] <= 'z')
				|| ('A' <= s[i] && s[i] <= 'Z')
				|| ('0' <= s[i] && s[i] <= '9') ) {
			result += s[i];
		} else {
			result += '%';
			result += hex[(unsigned char) s[i] >> 4];
			result += hex[(unsigned char) s[i] & 0x0f];
		}
	}
	return result;
}

void FritzClient::Login() throw(tcpclient::TcpException) {
	// when using SIDs, a new login is only needed if the last request was more than 5 minutes ago
	if (gConfig->getLoginType() == Config::SID && (time(NULL) - gConfig->getLastRequestTime() < 300)) {
		return;
	}

	// detect type of login once
	std::string sXml; // sXml is used twice!
	if (gConfig->getLoginType() == Config::UNKNOWN || gConfig->getLoginType() == Config::SID) {
		// detect if this Fritz!Box uses SIDs
		*dsyslog << __FILE__ << ": requesting login_sid.xml from fritz.box." << std::endl;
		tcpclient::HttpClient tc( gConfig->getUrl(), gConfig->getUiPort());
		tc 	<< tcpclient::get
			<< "/cgi-bin/webcm?getpage=../html/login_sid.xml"
			<< std::flush;
		tc 	>> sXml;
		if (sXml.find("<iswriteaccess>") != std::string::npos)
			gConfig->setLoginType(Config::SID);
		else
			gConfig->setLoginType(Config::PASSWORD);
	}

	if (gConfig->getLoginType() == Config::SID) {
		*dsyslog << __FILE__ << ": logging into fritz box using SIDs." << std::endl;
		if (gConfig->getSid().length() > 0) {
			// logout, drop old SID (if FB has not already dropped this SID because of a timeout)
			*dsyslog << __FILE__ << ": dropping old SID" << std::endl;
			std::string sDummy;
			tcpclient::HttpClient tc( gConfig->getUrl(), gConfig->getUiPort());
			tc 	<< tcpclient::post
				<< "/cgi-bin/webcm"
				<< std::flush
				<< "sid="
				<< gConfig->getSid()
				<< "&security:command/logout=abc"
				<< std::flush;
			tc 	>> sDummy;
		}
		// check if no password is needed (SID is directly available)
		size_t pwdFlag = sXml.find("<iswriteaccess>");
		if (pwdFlag == std::string::npos) {
			*esyslog << __FILE__ << ": Error - Expected <iswriteacess> not found in login_sid.xml." << std::endl;
			return;
		}
		pwdFlag += 15;
		if (sXml[pwdFlag] == '1') {
			// extract SID
			size_t sidStart = sXml.find("<SID>");
			if (sidStart == std::string::npos) {
				*esyslog << __FILE__ << ": Error - Expected <SID> not found in login_sid.xml." << std::endl;
				return;
			}
			sidStart += 5;
			// save SID
			gConfig->setSid(sXml.substr(sidStart, 16));
			gConfig->updateLastRequestTime();
		} else {
			// generate response out of challenge and password
			size_t challengeStart = sXml.find("<Challenge>");
			if (challengeStart == std::string::npos) {
				*esyslog << __FILE__ << ": Error - Expected <Challenge> not found in login_sid.xml." << std::endl;
				return;
			}
			challengeStart += 11;
			size_t challengeStop = sXml.find("<", challengeStart);
            std::string challenge = sXml.substr(challengeStart, challengeStop - challengeStart);
            std::string response = CalculateLoginResponse(challenge);
            // send response to box
			std::string sMsg;
			tcpclient::HttpClient tc( gConfig->getUrl(), gConfig->getUiPort());
			tc 	<< tcpclient::post
				<< "/cgi-bin/webcm"
				<< std::flush
				<< "login:command/response="
				<< response
				<< "&getpage=../html/de/menus/menu2.html"
				<< std::flush;
			tc 	>> sMsg;
			// get SID out of sMsg
			size_t sidStart = sMsg.find("name=\"sid\"");
			if (sidStart == std::string::npos) {
				*esyslog << __FILE__ << ": Error - Expected sid field not found." << std::endl;
				return;
			}
			sidStart = sMsg.find("value=\"", sidStart + 10) + 7;
			size_t sidStop = sMsg.find("\"", sidStart);
			// save SID
			gConfig->setSid(sMsg.substr(sidStart, sidStop-sidStart));
			// check if SID is valid
			bool isValidSid = false;
			for (size_t pos=0; pos < gConfig->getSid().length(); pos++)
				if (gConfig->getSid()[pos] != '0')
					isValidSid = true;
			if (isValidSid) {
				*dsyslog << __FILE__ << ": login successful." << std::endl;
				gConfig->updateLastRequestTime();
			} else
				*esyslog << __FILE__ << ": login failed!." << std::endl;
		}
	}
	if (gConfig->getLoginType() == Config::PASSWORD) {
		*dsyslog << __FILE__ << ": logging into fritz box using old scheme without SIDs." << std::endl;
		// no password, no login
		if ( gConfig->getPassword().length() == 0)
			return;

		std::string sMsg;

		tcpclient::HttpClient tc( gConfig->getUrl(), gConfig->getUiPort());
		tc 	<< tcpclient::post
			<< "/cgi-bin/webcm"
			<< std::flush
			<< "login:command/password="
			<< UrlEncode(gConfig->getPassword())
			<< std::flush;
		tc 	>> sMsg;

		// determine if login was successful
		if (sMsg.find("class=\"errorMessage\"") != std::string::npos) {
			*esyslog << __FILE__ << ": login failed, check your password settings." << std::endl;
		}
		*dsyslog << __FILE__ << ": login successful." << std::endl;
	}
}

std::string FritzClient::GetLang() throw(tcpclient::TcpException) {
	if ( gConfig && gConfig->getLang().size() == 0) {
		std::vector<std::string> langs;
		langs.push_back("en");
		langs.push_back("de");
		langs.push_back("fr");
		for (unsigned int p=0; p<langs.size(); p++) {
			std::string sMsg;
			tcpclient::HttpClient tc(gConfig->getUrl(), gConfig->getUiPort());
			tc 	<< tcpclient::get
				<< "/cgi-bin/webcm?getpage=../html/"
				<< langs[p]
				<< "/menus/menu2.html"
				<< (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid()
				<< std::flush;
			tc 	>> sMsg;
			if (sMsg.find("<html>") != std::string::npos) {
				gConfig->setLang(langs[p]);
				*dsyslog << __FILE__ << ": interface language is " << gConfig->getLang().c_str() << std::endl;
				return gConfig->getLang();
			}
		}
		*dsyslog << __FILE__ << ": error parsing interface language, assuming 'de'" << std::endl;
		gConfig->setLang("de");
	}
	return gConfig->getLang();
}

bool FritzClient::InitCall(std::string &number) {
	std::string msg;
	try {
		*isyslog << __FILE__ << ": sending call init request " << number.c_str() << std::endl;
		tcpclient::HttpClient tc( gConfig->getUrl(), gConfig->getUiPort());
		tc << tcpclient::post
		   << "/cgi-bin/webcm"
		   << std::flush
		   << "getpage=../html/"
		   << GetLang()
		   << "/menus/menu2.html&var%3Apagename=fonbuch&var%3Amenu=home&telcfg%3Acommand/Dial="
		   << number
  	       << (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid()
		   << std::flush;
		tc >> msg;
		*isyslog << __FILE__ << ": call initiated." << std::endl;
	} catch (tcpclient::TcpException te) {
		*esyslog << __FILE__ << ": Exception - " << te.what() << std::endl;
		return false;
	}
	return true;
}

std::string FritzClient::RequestLocationSettings() {
	std::string msg;
	RETRY_BEGIN {
		*dsyslog << __FILE__ << ": Looking up Phone Settings..." << std::endl;
		tcpclient::HttpClient hc(gConfig->getUrl(), gConfig->getUiPort());
		hc 	<< tcpclient::get
			<< "/cgi-bin/webcm?getpage=../html/"
			<<  GetLang()
			<< "/menus/menu2.html&var%3Alang="
			<<  GetLang()
			<< "&var%3Apagename=sipoptionen&var%3Amenu=fon"
			<< (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid()
			<< std::flush;
		hc >> msg;
	} RETRY_END
	return msg;
}

std::string FritzClient::RequestSipSettings() {
	std::string msg;
	RETRY_BEGIN {
		*dsyslog << __FILE__ << ": Looking up SIP Settings..." << std::endl;
		tcpclient::HttpClient hc(gConfig->getUrl(), gConfig->getUiPort());
		hc 	<< tcpclient::get
			<< "/cgi-bin/webcm?getpage=../html/"
			<< GetLang()
			<< "/menus/menu2.html&var%3Alang="
			<< GetLang()
			<< "&var%3Apagename=siplist&var%3Amenu=fon"
			<< (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid()
			<< std::flush;
		hc >> msg;
	} RETRY_END
	return msg;
}

std::string FritzClient::RequestCallList () {
	std::string msg;
	RETRY_BEGIN {
		// now, process call list
		*dsyslog << __FILE__ << ": sending callList request." << std::endl;
		// force an update of the fritz!box csv list and wait until all data is received
		tcpclient::HttpClient hc(gConfig->getUrl(), gConfig->getUiPort());
		hc 	<< tcpclient::get
			<< "/cgi-bin/webcm?getpage=../html/"
			<<  GetLang()
			<< "/menus/menu2.html&var:lang="
			<<  GetLang()
			<< "&var:pagename=foncalls&var:menu=fon"
			<< (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid()
			<< std::flush;
		hc >> msg;
		// get the URL of the CSV-File-Export
		unsigned int urlPos   = msg.find(".csv");
		unsigned int urlStop  = msg.find('"', urlPos);
		unsigned int urlStart = msg.rfind('"', urlPos) + 1;
		std::string csvUrl    = msg.substr(urlStart, urlStop-urlStart);
		// retrieve csv list
		msg = "";
		tcpclient::HttpClient hc2(gConfig->getUrl(), gConfig->getUiPort());
		hc2 << tcpclient::get
			<< "/cgi-bin/webcm?getpage="
			<<  csvUrl
			<< (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid()
			<< std::flush;
		hc2 >> msg;
		// convert answer to current SystemCodeSet (we assume, Fritz!Box sends its answer in latin15)
		CharSetConv *conv = new CharSetConv("ISO-8859-15", CharSetConv::SystemCharacterTable());
		const char *msg_converted = conv->Convert(msg.c_str());
		msg = msg_converted;
		delete(conv);
	} RETRY_END
	return msg;
}

std::string FritzClient::RequestFonbook () {
	std::string msg;
	RETRY_BEGIN {
		*dsyslog << __FILE__ << ": sending fonbook request." << std::endl;
		tcpclient::HttpClient tc(gConfig->getUrl(), gConfig->getUiPort());
		tc 	<< tcpclient::get
			<< "/cgi-bin/webcm?getpage=../html/"
			<< GetLang()
			<< "/menus/menu2.html"
			<< "&var:lang="
			<< GetLang()
			<< "&var:pagename=fonbuch&var:menu=fon"
			<< (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid()
			<< std::flush;
		tc >> msg;
	} RETRY_END
	return msg;
}

//TODO: update lastRequestTime with any request
}

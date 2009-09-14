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

#include <openssl/md5.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <langinfo.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <TcpClient++.h>
#include <errno.h>
#include "Tools.h"
#include "Config.h"


namespace fritz{

// --- UTF-8 support ---------------------------------------------------------

static uint SystemToUtf8[128] = { 0 };

int Utf8CharLen(const char *s)
{
	if (CharSetConv::SystemCharacterTable())
		return 1;
#define MT(s, m, v) ((*(s) & (m)) == (v)) // Mask Test
	if (MT(s, 0xE0, 0xC0) && MT(s + 1, 0xC0, 0x80))
		return 2;
	if (MT(s, 0xF0, 0xE0) && MT(s + 1, 0xC0, 0x80) && MT(s + 2, 0xC0, 0x80))
		return 3;
	if (MT(s, 0xF8, 0xF0) && MT(s + 1, 0xC0, 0x80) && MT(s + 2, 0xC0, 0x80) && MT(s + 3, 0xC0, 0x80))
		return 4;
	return 1;
}

uint Utf8CharGet(const char *s, int Length)
{
	if (CharSetConv::SystemCharacterTable())
		return (uchar)*s < 128 ? *s : SystemToUtf8[(uchar)*s - 128];
	if (!Length)
		Length = Utf8CharLen(s);
	switch (Length) {
	case 2: return ((*s & 0x1F) <<  6) |  (*(s + 1) & 0x3F);
	case 3: return ((*s & 0x0F) << 12) | ((*(s + 1) & 0x3F) <<  6) |  (*(s + 2) & 0x3F);
	case 4: return ((*s & 0x07) << 18) | ((*(s + 1) & 0x3F) << 12) | ((*(s + 2) & 0x3F) << 6) | (*(s + 3) & 0x3F);
	}
	return *s;
}

char *CharSetConv::systemCharacterTable = NULL;

CharSetConv::CharSetConv(const char *FromCode, const char *ToCode)
{
	if (!FromCode)
		FromCode = systemCharacterTable ? systemCharacterTable : "UTF-8";
	if (!ToCode)
		ToCode = "UTF-8";
	cd = iconv_open(ToCode, FromCode);
	result = NULL;
	length = 0;
}

CharSetConv::~CharSetConv()
{
	free(result);
	iconv_close(cd);
}

void CharSetConv::DetectCharset() {
	char *CodeSet = NULL;
	if (setlocale(LC_CTYPE, ""))
		CodeSet = nl_langinfo(CODESET);
	else {
		char *LangEnv = getenv("LANG"); // last resort in case locale stuff isn't installed
		if (LangEnv) {
			CodeSet = strchr(LangEnv, '.');
			if (CodeSet)
				CodeSet++; // skip the dot
		}
	}
	if (CodeSet) {
		*isyslog << __FILE__ << ": detected codeset is '" << CodeSet << "'" << std::endl;
		SetSystemCharacterTable(CodeSet);
	}
}

void CharSetConv::SetSystemCharacterTable(const char *CharacterTable)
{
	free(systemCharacterTable);
	systemCharacterTable = NULL;
	if (!strcasestr(CharacterTable, "UTF-8")) {
		// Set up a map for the character values 128...255:
		char buf[129];
		for (int i = 0; i < 128; i++)
			buf[i] = i + 128;
		buf[128] = 0;
		CharSetConv csc(CharacterTable);
		const char *s = csc.Convert(buf);
		int i = 0;
		while (*s) {
			int sl = Utf8CharLen(s);
			SystemToUtf8[i] = Utf8CharGet(s, sl);
			s += sl;
			i++;
		}
		systemCharacterTable = strdup(CharacterTable);
	}
}

const char *CharSetConv::Convert(const char *From, char *To, size_t ToLength)
{
	if (From && *From) {
		char *FromPtr = (char *)From;
		size_t FromLength = strlen(From);
		char *ToPtr = To;
		if (!ToPtr) {
			if (length < (FromLength * 2)) // some reserve to avoid later reallocations
				length = FromLength * 2;
			result = (char *)realloc(result, length);
			ToPtr = result;
			ToLength = length;
		}
		else if (!ToLength)
			return From; // can't convert into a zero sized buffer
		ToLength--; // save space for terminating 0
		char *Converted = ToPtr;
		while (FromLength > 0) {
			if (iconv(cd, &FromPtr, &FromLength, &ToPtr, &ToLength) == size_t(-1)) {
				if (errno == E2BIG || (errno == EILSEQ && ToLength < 1)) {
					if (To)
						break; // caller provided a fixed size buffer, but it was too small
					// The result buffer is too small, so increase it:
					size_t d = ToPtr - result;
					size_t r = length / 2;
					length += r;
					Converted = result = (char *)realloc(result, length);
					ToLength += r;
					ToPtr = result + d;
				}
				if (errno == EILSEQ) {
					// A character can't be converted, so mark it with '?' and proceed:
					FromPtr++;
					FromLength--;
					*ToPtr++ = '?';
					ToLength--;
				}
				else if (errno != E2BIG)
					return From; // unknown error, return original string
			}
		}
		*ToPtr = 0;
		return Converted;
	}
	return From;
}

pthread::Mutex* Tools::mutex = new pthread::Mutex();

Tools::Tools()
{
}

Tools::~Tools()
{
}

bool Tools::MatchesMsnFilter(const std::string &number){
	// if no MSN filter is set, true is returned
	if (gConfig->getMsnFilter().size() == 0)
		return true;
	// if number does contain a MSN out of the MSN filter, true is returned
	for (size_t pos=0; pos < gConfig->getMsnFilter().size(); pos++) {
		if (number.find(gConfig->getMsnFilter()[pos]) != std::string::npos ) {
			//matched
			return true;
		}
	}
	// no match
	return false;
}

std::string Tools::GetLang() {
	// TODO: this does "not always" work for fw 29.04.67 (behaves indeterministically)
	//	if ( gConfig->getLang().size() == 0) {
	//		std::string sMsg;
	//		*dsyslog << __FILE__ << ": detecting fritz.box interface language..." << std::endl;
	//		tcpclient::HttpClient tc(gConfig->getUrl(), PORT_WWW);
	//		tc << "GET /cgi-bin/webcm?getpage=../html/index_inhalt.html&var:loginDone=0 HTTP/1.1\n\n";
	//		tc >> sMsg;
	//
	//		// determine language of webinterface
	//		size_t p = sMsg.find("name=\"var:lang\"");
	//		if (p != std::string::npos) {
	//			p += 16; // skip lang-tag
	//			size_t langStart = sMsg.find('"', p);
	//			size_t langStop  = sMsg.find('"', ++langStart);
	//			if (langStart != std::string::npos && langStop != std::string::npos) {
	//				gConfig->setLang( sMsg.substr(langStart, langStop - langStart) );
	//				*dsyslog << __FILE__ << ": interface language is " << gConfig->getLang().c_str() << std::endl;
	//			}
	//		}
	//		// if detection was not successful (newer firmware versions)
	//		if ( gConfig->getLang().size() == 0) {
	//			// alternative detection method
	//			p = sMsg.find("/js/jsl.js");
	//			if (p != std::string::npos) {
	//				size_t langStart = sMsg.rfind('/', p-1)+1;
	//				size_t langStop = p;
	//				if (langStart != std::string::npos) {
	//					gConfig->setLang( sMsg.substr(langStart, langStop - langStart) );
	//					*dsyslog << __FILE__ << ": interface language is " << gConfig->getLang().c_str() << std::endl;
	//				}
	//			}
	//		}
	//		// fallback if detection was unsuccessful
	//		if ( gConfig->getLang().size() == 0) {
	//			gConfig->getLang() = "de";
	//			*dsyslog << __FILE__ << ": error parsing interface language, assuming 'de'" << std::endl;
	//		}
	//	}
	// Workaround: "Try-and-Error"
	if ( gConfig && gConfig->getLang().size() == 0) {
		try {
			Login();
			std::vector<std::string> langs;
			langs.push_back("en");
			langs.push_back("de");
			langs.push_back("fr");
			for (unsigned int p=0; p<langs.size(); p++) {
				std::string sMsg;
				tcpclient::HttpClient tc(gConfig->getUrl(), gConfig->getUiPort());
				tc << tcpclient::get
				   << "/cgi-bin/webcm?getpage=../html/"
				   << langs[p]
				   << "/menus/menu2.html"
				   << (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid()
				   << std::flush;
				tc >> sMsg;
				if (sMsg.find("<html>") != std::string::npos) {
					gConfig->setLang(langs[p]);
					*dsyslog << __FILE__ << ": interface language is " << gConfig->getLang().c_str() << std::endl;
					return gConfig->getLang();
				}
			}
		} catch (tcpclient::TcpException te) {
			*esyslog << __FILE__ << ": Exception - " << te.what() << std::endl;
		}
		*dsyslog << __FILE__ << ": error parsing interface language, assuming 'de'" << std::endl;
		gConfig->setLang("de");
	}
	return gConfig->getLang();
}

std::string Tools::CalculateLoginResponse(std::string challenge) {
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

void Tools::Login() {
	*dsyslog << __FILE__ << ": logging in to fritz.box." << std::endl;

	// detect if this Fritz!Box uses SIDs
	*dsyslog << __FILE__ << ": requesting login_sid.xml from fritz.box." << std::endl;
	std::string sXml;
	try {
		tcpclient::HttpClient tc( gConfig->getUrl(), gConfig->getUiPort());
		tc << tcpclient::get
		   << "/cgi-bin/webcm?getpage=../html/login_sid.xml"
		   << std::flush;
		tc >> sXml;
	} catch (tcpclient::TcpException te) {
		*esyslog << __FILE__ << ": Exception - " << te.what() << std::endl;
		return;
	}
	if (sXml.find("<iswriteaccess>") != std::string::npos) { // login using SID
		*dsyslog << __FILE__ << ": using new login scheme with SIDs" << std::endl;
		// logout, drop old SID
		*dsyslog << __FILE__ << ": dropping old SID" << std::endl;
		try {
			std::string sDummy;
			tcpclient::HttpClient tc( gConfig->getUrl(), gConfig->getUiPort());
			tc << tcpclient::post
			   << "/cgi-bin/webcm"
			   << std::flush
			   << "sid="
			   << gConfig->getSid()
			   << "&security:command/logout=abc"
			   << std::flush;
			tc >> sDummy;
		} catch (tcpclient::TcpException te) {
			*esyslog << __FILE__ << ": Exception - " << te.what() << std::endl;
			return;
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
			try {
				tcpclient::HttpClient tc( gConfig->getUrl(), gConfig->getUiPort());
				tc << tcpclient::post
				   << "/cgi-bin/webcm"
				   << std::flush
				   << "login:command/response="
				   << response
				   << "&getpage=../html/de/menus/menu2.html"
				   << std::flush;
				tc >> sMsg;
			} catch (tcpclient::TcpException te) {
				*esyslog << __FILE__ << ": Exception - " << te.what() << std::endl;
				return;
			}
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
			*dsyslog << __FILE__ << ": login successful." << std::endl;
		}
	} else { // login without SID
		*dsyslog << __FILE__ << ": using old login scheme without SIDs" << std::endl;
		// no password, no login
		if ( gConfig->getPassword().length() == 0)
			return;

		std::string sMsg;

		try {
			tcpclient::HttpClient tc( gConfig->getUrl(), gConfig->getUiPort());
			tc << tcpclient::post
			   << "/cgi-bin/webcm"
			   << std::flush
			   << "login:command/password="
			   << UrlEncode(gConfig->getPassword())
			   << std::flush;
			tc >> sMsg;
		} catch (tcpclient::TcpException te) {
			*esyslog << __FILE__ << ": Exception - " << te.what() << std::endl;
			return;
		}

		// determine if login was successful
		if (sMsg.find("class=\"errorMessage\"") != std::string::npos) {
			*esyslog << __FILE__ << ": login failed, check your password settings." << std::endl;
			throw ToolsException(ToolsException::ERR_LOGIN_FAILED);
		}
		*dsyslog << __FILE__ << ": login successful." << std::endl;
	}
}

std::string Tools::UrlEncode(std::string &s_input) {
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

bool Tools::InitCall(std::string &number) {
	std::string msg;
	try {
		Login();
		*isyslog << __FILE__ << ": sending call init request " << number.c_str() << std::endl;
		tcpclient::HttpClient tc( gConfig->getUrl(), gConfig->getUiPort());
		tc << tcpclient::post
		   << "/cgi-bin/webcm"
		   << std::flush
		   << "getpage=../html/"
		   << Tools::GetLang()
		   << "/menus/menu2.html&var%3Apagename=fonbuch&var%3Amenu=home&telcfg%3Acommand/Dial="
		   << number
  	       << (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid()
		   <<	std::flush;
		tc >> msg;
		*isyslog << __FILE__ << ": call initiated." << std::endl;
	} catch (tcpclient::TcpException te) {
		*esyslog << __FILE__ << ": Exception - " << te.what() << std::endl;
		return false;
	}
	return true;
}

std::string Tools::NormalizeNumber(std::string number) {
	// Only for Germany: Remove Call-By-Call Provider Selection Codes 010(0)xx
	if ( gConfig->getCountryCode() == "49") {
		if (number[0] == '0' && number[1] == '1' && number[2] == '0') {
			if (number[3] == '0')
				number.erase(0, 6);
			else
				number.erase(0, 5);
		}
	}
	// Modifies 'number' to the following format
	// '00' + countryCode + regionCode + phoneNumber
	if (number[0] == '+') {
		//international prefix given in form +49 -> 0049
		number.replace(0, 1, "00");
	} else if (number[0] == '0' && number[1] != '0') {
		//national prefix given 089 -> 004989
		number.replace(0, 1, gConfig->getCountryCode().c_str());
		number = "00" + number;
	} else if (number[0] != '0') {
		// number without country or region code, 1234 -> +49891234
		number = "00" + gConfig->getCountryCode() + gConfig->getRegionCode() + number;
	} // else: number starts with '00', do not change
	return number;
}

int Tools::CompareNormalized(std::string number1, std::string number2) {
	return NormalizeNumber(number1).compare(NormalizeNumber(number2));
}

void Tools::GetLocationSettings() {
//	get settings from Fritz!Box.
	*dsyslog << __FILE__ << ": Looking up Phone Settings..." << std::endl;
	std::string msg;
	try {
		Login();
		tcpclient::HttpClient hc(gConfig->getUrl(), gConfig->getUiPort());
		hc << tcpclient::get
		   << "/cgi-bin/webcm?getpage=../html/"
		   <<  Tools::GetLang()
		   << "/menus/menu2.html&var%3Alang="
		   <<  Tools::GetLang()
		   << "&var%3Apagename=sipoptionen&var%3Amenu=fon"
  	           << (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid()
                   << std::flush;
		hc >> msg;
	} catch (tcpclient::TcpException te) {
		*esyslog << __FILE__ << ": cTcpException - " << te.what() << std::endl;
		return;
	} catch (ToolsException te) {
		*esyslog << __FILE__ << ": cToolsException - " << te.what() << std::endl;
		return;
	}
	size_t lkzStart = msg.find("telcfg:settings/Location/LKZ");
	if (lkzStart == std::string::npos) {
		*esyslog << __FILE__ << ": Parser error in GetLocationSettings(). Could not find LKZ." << std::endl;
		*esyslog << __FILE__ << ": LKZ not set! Assuming 49 (Germany)." << std::endl;
		*esyslog << __FILE__ << ": OKZ not set! Resolving phone numbers may not always work." << std::endl;
		gConfig->setCountryCode("49");
		return;
	}
	lkzStart += 37;
	size_t lkzStop  = msg.find("\"", lkzStart);
	size_t okzStart = msg.find("telcfg:settings/Location/OKZ");
	if (okzStart == std::string::npos) {
		*esyslog << __FILE__ << ": Parser error in GetLocationSettings(). Could not find OKZ." << std::endl;
		*esyslog << __FILE__ << ": OKZ not set! Resolving phone numbers may not always work." << std::endl;
		return;
	}
	okzStart += 37;
	size_t okzStop = msg.find("\"", okzStart);
	gConfig->setCountryCode( msg.substr(lkzStart, lkzStop - lkzStart) );
	gConfig->setRegionCode( msg.substr(okzStart, okzStop - okzStart) );
	if (gConfig->getCountryCode().size() > 0) {
		*dsyslog << __FILE__ << ": Found LKZ " << gConfig->getCountryCode() << std::endl;
	} else {
		*esyslog << __FILE__ << ": LKZ not set! Assuming 49 (Germany)." << std::endl;
		gConfig->setCountryCode("49");
	}
	if (gConfig->getRegionCode().size() > 0) {
		*dsyslog << __FILE__ << ": Found OKZ " << gConfig->getRegionCode() << std::endl;
	} else {
		*esyslog << __FILE__ << ": OKZ not set! Resolving phone numbers may not always work." << std::endl;
	}
}

void Tools::GetSipSettings(){
	// if SIP settings are already set, exit here...
	if ( gConfig->getSipNames().size() > 0 )
		return;
	// ...otherwise get settings from Fritz!Box.
	*dsyslog << __FILE__ << ": Looking up SIP Settings..." << std::endl;
	std::string msg;
	try {
		Login();
		tcpclient::HttpClient hc(gConfig->getUrl(), gConfig->getUiPort());
		hc << tcpclient::get
		   << "/cgi-bin/webcm?getpage=../html/"
		   << Tools::GetLang()
		   << "/menus/menu2.html&var%3Alang="
		   << Tools::GetLang()
		   << "&var%3Apagename=siplist&var%3Amenu=fon"
		   << (gConfig->getSid().size() ? "&sid=" : "") << gConfig->getSid()
		   << std::flush;
		hc >> msg;
	} catch (tcpclient::TcpException te) {
		*esyslog << __FILE__ << ": cTcpException - " << te.what() << std::endl;
		return;
	} catch (ToolsException te) {
		*esyslog << __FILE__ << ": cToolsException - " << te.what() << std::endl;
		return;
	}
	std::vector<std::string> sipNames;

	// check if the structure of the HTML page matches our search pattern
	if (msg.find("function AuswahlDisplay") == std::string::npos){
		*esyslog << __FILE__ << ": Parser error in GetSipSettings(). Could not find SIP list." << std::endl;
		*esyslog << __FILE__ << ": SIP provider names not set! Usage of SIP provider names not possible." << std::endl;
		return;
	}

	size_t sipStart = 0;
	for(size_t i=0; i < 10; i++){
		sipStart = msg.find("AuswahlDisplay(\"", sipStart +1);
		if (sipStart == std::string::npos) {
			// end of list reached
			break;
		}
		size_t hostStart = msg.rfind("ProviderDisplay(\"",sipStart);
		if (hostStart == std::string::npos) {
			// something is wrong with the structure of the HTML page
			*esyslog << __FILE__ << ": Parser error in GetSipSettings(). Could not find SIP provider name." << std::endl;
			*esyslog << __FILE__ << ": SIP provider names not set! Usage of SIP provider names not possible." << std::endl;
			return;
		}
		hostStart += 17;
		size_t hostStop      = msg.find("\")", hostStart);
		std::string hostName = msg.substr(hostStart, hostStop - hostStart);
		std::string sipName  = hostName;

		// now translate hostname into real provider name according to internal translation table of fritzbox
		size_t tableStart     = msg.find("function ProviderDisplay");
		size_t tableStop      = msg.find("}", tableStart);
		size_t tableHostStart = msg.find("case \"",   tableStart);
		if (tableStart     == std::string::npos || tableStop     == std::string::npos ||
			tableHostStart == std::string::npos) {
				// something is wrong with the structure of the HTML page
				*esyslog << __FILE__ << ": Parser error in GetSipSettings(). Could not find SIP provider name." << std::endl;
				*esyslog << __FILE__ << ": SIP provider names not set! Usage of SIP provider names not possible." << std::endl;
				return;
			}
		while (tableHostStart <= tableStop && tableHostStart != std::string::npos) {
			size_t tableHostStop  = msg.find("\"",        tableHostStart + 6);
			size_t tableNameStart = msg.find("return \"", tableHostStop);
			size_t tableNameStop  = msg.find("\"",        tableNameStart + 8);
			if (tableHostStart == std::string::npos || tableHostStop == std::string::npos ||
				tableNameStart == std::string::npos || tableNameStop == std::string::npos) {
				// something is wrong with the structure of the HTML page
				*esyslog << __FILE__ << ": Parser error in GetSipSettings(). Could not find SIP provider name." << std::endl;
				*esyslog << __FILE__ << ": SIP provider names not set! Usage of SIP provider names not possible." << std::endl;
				return;
			}
			tableHostStart += 6;
			std::string tableHost = msg.substr(tableHostStart, tableHostStop - tableHostStart);
			tableNameStart += 8;
			std::string tableName = msg.substr(tableNameStart, tableNameStop - tableNameStart);
			if (hostName.find(tableHost) != std::string::npos) {
				// we found a match in the table
				sipName = tableName;
				break;
			}
			// search the next table line
			tableHostStart = msg.find("case \"",   tableNameStop);
		}

		sipNames.push_back(sipName);
		*dsyslog << __FILE__ << ": Found SIP" << i << " (" << hostName << ") provider name " << sipName << std::endl;
	}
	gConfig->setSipNames(sipNames);

}

std::string Tools::Tokenize(const std::string &buffer, const char delimiter, size_t pos) {
	size_t tokenStart = 0;
	for (size_t i=0; i<pos; i++) {
		tokenStart = buffer.find(delimiter, tokenStart+1);
		if (tokenStart == std::string::npos)
			return "";
	}
	if (tokenStart > 0)
		tokenStart++;
	size_t tokenStop = buffer.find(delimiter, tokenStart);
	if (tokenStop == std::string::npos)
		tokenStop = buffer.size();
	std::string token = buffer.substr(tokenStart, tokenStop - tokenStart);
	return token;
}

}


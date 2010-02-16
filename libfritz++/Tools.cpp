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

#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <langinfo.h>
#include <sstream>
#include <iostream>
#include <errno.h>
#include "FritzClient.h"
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
		INF("detected codeset is '" << CodeSet << "'");
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

std::string Tools::NormalizeNumber(std::string number) {
	// Remove Fritz!Box control codes *xyz# if used
	if (number[0] == '*') {
		size_t hashPos = number.find('#');
		if (hashPos != std::string::npos)
			number.erase(0, hashPos + 1);
	}
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

bool Tools::GetLocationSettings() {
	//	get settings from Fritz!Box.
	FritzClient fc;
	std::string msg = fc.RequestLocationSettings();

	size_t lkzStart = msg.find("telcfg:settings/Location/LKZ");
	if (lkzStart == std::string::npos) {
		ERR("Parser error in GetLocationSettings(). Could not find LKZ.");
		ERR("LKZ/OKZ not set! Resolving phone numbers may not always work.");
		return fc.hasValidPassword();
	}
	lkzStart += 37;
	size_t lkzStop  = msg.find("\"", lkzStart);
	size_t okzStart = msg.find("telcfg:settings/Location/OKZ");
	if (okzStart == std::string::npos) {
		ERR("Parser error in GetLocationSettings(). Could not find OKZ.");
		ERR("OKZ not set! Resolving phone numbers may not always work.");
		return fc.hasValidPassword();
	}
	okzStart += 37;
	size_t okzStop = msg.find("\"", okzStart);
	gConfig->setCountryCode( msg.substr(lkzStart, lkzStop - lkzStart) );
	gConfig->setRegionCode( msg.substr(okzStart, okzStop - okzStart) );
	if (gConfig->getCountryCode().size() > 0) {
		DBG("Found LKZ " << gConfig->getCountryCode());
	} else {
		ERR("LKZ not set! Resolving phone numbers may not always work.");
	}
	if (gConfig->getRegionCode().size() > 0) {
		DBG("Found OKZ " << gConfig->getRegionCode());
	} else {
		ERR("OKZ not set! Resolving phone numbers may not always work.");
	}
	return fc.hasValidPassword();
}

void Tools::GetSipSettings() {
	// if SIP settings are already set, exit here...
	if ( gConfig->getSipNames().size() > 0 )
		return;
	// ...otherwise get settings from Fritz!Box.
	FritzClient fc;
	std::string msg = fc.RequestSipSettings();

	std::vector<std::string> sipNames;

	// check if the structure of the HTML page matches our search pattern
	if (msg.find("function AuswahlDisplay") == std::string::npos){
		ERR("Parser error in GetSipSettings(). Could not find SIP list.");
		ERR("SIP provider names not set! Usage of SIP provider names not possible.");
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
			ERR("Parser error in GetSipSettings(). Could not find SIP provider name.");
			ERR("SIP provider names not set! Usage of SIP provider names not possible.");
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
				ERR("Parser error in GetSipSettings(). Could not find SIP provider name.");
				ERR("SIP provider names not set! Usage of SIP provider names not possible.");
				return;
			}
		while (tableHostStart <= tableStop && tableHostStart != std::string::npos) {
			size_t tableHostStop  = msg.find("\"",        tableHostStart + 6);
			size_t tableNameStart = msg.find("return \"", tableHostStop);
			size_t tableNameStop  = msg.find("\"",        tableNameStart + 8);
			if (tableHostStart == std::string::npos || tableHostStop == std::string::npos ||
				tableNameStart == std::string::npos || tableNameStop == std::string::npos) {
				// something is wrong with the structure of the HTML page
				ERR("Parser error in GetSipSettings(). Could not find SIP provider name.");
				ERR("SIP provider names not set! Usage of SIP provider names not possible.");
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
		DBG("Found SIP" << i << " (" << hostName << ") provider name " << sipName);
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


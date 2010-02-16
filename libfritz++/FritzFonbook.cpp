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

#include <string.h>
#include <algorithm>
#include <sstream>
#include "FritzFonbook.h"
#include "FritzClient.h"
#include "Tools.h"
#include <TcpClient++.h>
#include "Config.h"

namespace fritz {

// this array contains characters encoded with ISO-8859-15, take care when editing this file
const char *Entities[97][2] = {
	{"&nbsp;",  " "},
	{"&iexcl;", "¡"},
	{"&cent;",  "¢"},
	{"&pound;", "£"},
	{"&curren;","¤"},
	{"&yen;",   "¥"},
	{"&brvbar;","¦"},
	{"&sect;",  "§"},
	{"&uml;",   "¨"},
	{"&copy;",  "©"},
	{"&ordf;",  "ª"},
	{"&laquo;", "«"},
	{"&not;",   "¬"},
	{"&shy;",   "­"},
	{"&reg;",   "®"},
	{"&macr;",  "¯"},
	{"&deg;",   "°"},
	{"&plusmn;","±"},
	{"&sup2;",  "²"},
	{"&sup3;",  "³"},
	{"&acute;", "´"},
	{"&micro;", "µ"},
	{"&para;",  "¶"},
	{"&middot;","·"},
	{"&cedil;", "¸"},
	{"&sup1;",  "¹"},
	{"&ordm;",  "º"},
	{"&raquo;", "»"},
	{"&frac14;","¼"},
	{"&frac12;","½"},
	{"&frac34;","¾"},
	{"&iquest;","¿"},
	{"&Agrave;","À"},
	{"&Aacute;","Á"},
	{"&Acirc;", "Â"},
	{"&Atilde;","Ã"},
	{"&Auml;",  "Ä"},
	{"&Aring;", "Å"},
	{"&AElig;", "Æ"},
	{"&Ccedil;","Ç"},
	{"&Egrave;","È"},
	{"&Eacute;","É"},
	{"&Ecirc;", "Ê"},
	{"&Euml;",  "Ë"},
	{"&Igrave;","Ì"},
	{"&Iacute;","Í"},
	{"&Icirc;", "Î"},
	{"&Iuml;",  "Ï"},
	{"&ETH;",   "Ð"},
	{"&Ntilde;","Ñ"},
	{"&Ograve;","Ò"},
	{"&Oacute;","Ó"},
	{"&Ocirc;", "Ô"},
	{"&Otilde;","Õ"},
	{"&Ouml;",  "Ö"},
	{"&times;", "×"},
	{"&Oslash;","Ø"},
	{"&Ugrave;","Ù"},
	{"&Uacute;","Ú"},
	{"&Ucirc;", "Û"},
	{"&Uuml;",  "Ü"},
	{"&Yacute;","Ý"},
	{"&THORN;", "Þ"},
	{"&szlig;", "ß"},
	{"&agrave;","à"},
	{"&aacute;","á"},
	{"&acirc;", "â"},
	{"&atilde;","ã"},
	{"&auml;",  "ä"},
	{"&aring;", "å"},
	{"&aelig;", "æ"},
	{"&ccedil;","ç"},
	{"&egrave;","è"},
	{"&eacute;","é"},
	{"&ecirc;", "ê"},
	{"&euml;",  "ë"},
	{"&igrave;","ì"},
	{"&iacute;","í"},
	{"&icirc;", "î"},
	{"&iuml;",  "ï"},
	{"&eth;",   "ð"},
	{"&ntilde;","ñ"},
	{"&ograve;","ò"},
	{"&oacute;","ó"},
	{"&ocirc;", "ô"},
	{"&otilde;","õ"},
	{"&ouml;",  "ö"},
	{"&divide;","÷"},
	{"&oslash;","ø"},
	{"&ugrave;","ù"},
	{"&uacute;","ú"},
	{"&ucirc;", "û"},
	{"&uuml;",  "ü"},
	{"&yacute;","ý"},
	{"&thorn;", "þ"},
	{"&yuml;",  "ÿ"},
	{"&amp;",   "&"},
};

std::string &convertEntities(std::string &s) {
	if (s.find("&") != std::string::npos) {
		// convert the entities from iso-8859-15 to current system character table
		CharSetConv *conv = new CharSetConv("ISO-8859-15", CharSetConv::SystemCharacterTable());

		for (int i=0; i<97; i++) {
			std::string::size_type pos = s.find(Entities[i][0]);
			if (pos != std::string::npos) {
				s.replace(pos, strlen(Entities[i][0]), conv->Convert(Entities[i][1]));
				i--; //search for the same entity again
			}
		}
		delete (conv);
	}
	return s;
}

FritzFonbook::FritzFonbook()
:PThread("FritzFonbook")
{
	title = I18N_NOOP("Fritz!Box phone book");
	techId = "FRITZ";
	displayable = true;
	setInitialized(false);
}

FritzFonbook::~FritzFonbook() {
	// don't delete the object, while the thread is still active
	while (Active())
		pthread::CondWait::SleepMs(100);
}

bool FritzFonbook::Initialize() {
	return Start();
}

void FritzFonbook::Action() {
	setInitialized(false);
	fonbookList.clear();

	FritzClient fc;
	std::string msg = fc.RequestFonbook();

	size_t pos, p1, p2;
	// determine charset (default for old firmware versions is iso-8859-15)
	std::string charset = "ISO-8859-15";
	pos = msg.find("<meta http-equiv=content-type");
	if (pos != std::string::npos) {
		pos = msg.find("charset=", pos);
		if (pos != std::string::npos)
			charset = msg.substr(pos+8, msg.find('"', pos)-pos-8);
	}
	DBG("using charset " << charset);

	CharSetConv *conv = new CharSetConv(charset.c_str(), CharSetConv::SystemCharacterTable());
	const char *s_converted = conv->Convert(msg.c_str());
	msg = s_converted;
	delete (conv);

	// parse answer
	pos = 0;
	int count = 0;
	// parser for old format
	const std::string tag("(TrFon(");
	while ((p1 = msg.find(tag, pos)) != std::string::npos) {
		p1 += 7; // points to the first "
		int nameStart     = msg.find(',', p1)          +3;
		int nameStop      = msg.find('"', nameStart)   -1;
		int numberStart   = msg.find(',', nameStop)    +3;
		int numberStop    = msg.find('"', numberStart) -1;
		if (msg[nameStart] == '!') // skip '!' char, older firmware versions use to mark VIPs
			nameStart++;
		std::string namePart = msg.substr(nameStart, nameStop - nameStart+1);
		std::string namePart2 = convertEntities(namePart);
		std::string numberPart = msg.substr(numberStart, numberStop - numberStart+1);
		if (namePart2.length() && numberPart.length()) {
			FonbookEntry fe(namePart2, numberPart, FonbookEntry::TYPE_NONE);
			fonbookList.push_back(fe);
			//DBG("(%s / %s)", fe.number.c_str(), fe.name.c_str());
		}
		pos = p1+10;
		count++;
	}
	// parser for new format
	pos = 0;
	const std::string tag2("TrFonName(");
	const std::string tag3("TrFonNr("	);
	while ((p2 = msg.find(tag3, pos)) != std::string::npos) {
		int typeStart     = p2 + 9;
		int numberStart   = msg.find(',', p2)    +3;
		int typeStop      = numberStart - 5;
		int numberStop    = msg.find('"', numberStart) -1;
		p1 = msg.rfind(tag2, p2);
		p1 += 7; // points to the first "
		int nameStart     = msg.find(',', p1)          +3;
		int nameStop      = msg.find('"', nameStart)   -1;
		std::string namePart   = msg.substr(nameStart, nameStop - nameStart+1);
		std::string namePart2  = convertEntities(namePart);
		std::string numberPart = msg.substr(numberStart, numberStop - numberStart+1);

		std::string typePart   = msg.substr(typeStart, typeStop - typeStart+1);
		FonbookEntry::eType type = FonbookEntry::TYPE_NONE;
		if      (typePart.compare("home") == 0)
			type = FonbookEntry::TYPE_HOME;
		else if (typePart.compare("mobile") == 0)
			type = FonbookEntry::TYPE_MOBILE;
		else if (typePart.compare("work") == 0)
			type = FonbookEntry::TYPE_WORK;

		if (namePart2.length() && numberPart.length()) {
			FonbookEntry fe(namePart2, numberPart, type);
			fonbookList.push_back(fe);
			//DBG("(%s / %s / %i)", fe.number.c_str(), fe.name.c_str(), fe.type);
		}
		pos = p2+10;
		count++;
	}
	INF("read " << count << " entries.");
	setInitialized(true);

	std::sort(fonbookList.begin(), fonbookList.end());
}

void FritzFonbook::Reload() {
	this->Start();
}

}

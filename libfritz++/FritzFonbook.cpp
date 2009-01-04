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


#include <string.h>
#include <algorithm>
#include <sstream>
#include "FritzFonbook.h"
#include "Tools.h"
#include <TcpClient++.h>
#include "Config.h"

namespace fritz{

const char *Entities[97][2] = {
	{"&nbsp;",  " "},
	{"&iexcl;", "�"},
	{"&cent;",  "�"},
	{"&pound;", "�"},
	{"&curren;","�"},
	{"&yen;",   "�"},
	{"&brvbar;","�"},
	{"&sect;",  "�"},
	{"&uml;",   "�"},
	{"&copy;",  "�"},
	{"&ordf;",  "�"},
	{"&laquo;", "�"},
	{"&not;",   "�"},
	{"&shy;",   "�"},
	{"&reg;",   "�"},
	{"&macr;",  "�"},
	{"&deg;",   "�"},
	{"&plusmn;","�"},
	{"&sup2;",  "�"},
	{"&sup3;",  "�"},
	{"&acute;", "�"},
	{"&micro;", "�"},
	{"&para;",  "�"},
	{"&middot;","�"},
	{"&cedil;", "�"},
	{"&sup1;",  "�"},
	{"&ordm;",  "�"},
	{"&raquo;", "�"},
	{"&frac14;","�"},
	{"&frac12;","�"},
	{"&frac34;","�"},
	{"&iquest;","�"},
	{"&Agrave;","�"},
	{"&Aacute;","�"},
	{"&Acirc;", "�"},
	{"&Atilde;","�"},
	{"&Auml;",  "�"},
	{"&Aring;", "�"},
	{"&AElig;", "�"},
	{"&Ccedil;","�"},
	{"&Egrave;","�"},
	{"&Eacute;","�"},
	{"&Ecirc;", "�"},
	{"&Euml;",  "�"},
	{"&Igrave;","�"},
	{"&Iacute;","�"},
	{"&Icirc;", "�"},
	{"&Iuml;",  "�"},
	{"&ETH;",   "�"},
	{"&Ntilde;","�"},
	{"&Ograve;","�"},
	{"&Oacute;","�"},
	{"&Ocirc;", "�"},
	{"&Otilde;","�"},
	{"&Ouml;",  "�"},
	{"&times;", "�"},
	{"&Oslash;","�"},
	{"&Ugrave;","�"},
	{"&Uacute;","�"},
	{"&Ucirc;", "�"},
	{"&Uuml;",  "�"},
	{"&Yacute;","�"},
	{"&THORN;", "�"},
	{"&szlig;", "�"},
	{"&agrave;","�"},
	{"&aacute;","�"},
	{"&acirc;", "�"},
	{"&atilde;","�"},
	{"&auml;",  "�"},
	{"&aring;", "�"},
	{"&aelig;", "�"},
	{"&ccedil;","�"},
	{"&egrave;","�"},
	{"&eacute;","�"},
	{"&ecirc;", "�"},
	{"&euml;",  "�"},
	{"&igrave;","�"},
	{"&iacute;","�"},
	{"&icirc;", "�"},
	{"&iuml;",  "�"},
	{"&eth;",   "�"},
	{"&ntilde;","�"},
	{"&ograve;","�"},
	{"&oacute;","�"},
	{"&ocirc;", "�"},
	{"&otilde;","�"},
	{"&ouml;",  "�"},
	{"&divide;","�"},
	{"&oslash;","�"},
	{"&ugrave;","�"},
	{"&uacute;","�"},
	{"&ucirc;", "�"},
	{"&uuml;",  "�"},
	{"&yacute;","�"},
	{"&thorn;", "�"},
	{"&yuml;",  "�"},
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
	title = "Fritz!Box phone book";
	techId = "FRITZ";
	displayable = true;
	setInitialized(false);
}

FritzFonbook::~FritzFonbook() {
}

bool FritzFonbook::Initialize() {
	return Start();
}

void FritzFonbook::Action() {
	unsigned int retry_delay = RETRY_DELAY / 2;
	std::string msg;
	Tools::GetFritzBoxMutex()->Lock();
	do {
		try {
			retry_delay = retry_delay > 1800 ? 3600 : retry_delay * 2;
			setInitialized(false);
			fonbookList.clear();

			Tools::Login();
			*dsyslog << __FILE__ << ": sending fonbook request." << std::endl;
			tcpclient::HttpClient tc(gConfig->getUrl(), gConfig->getUiPort());
			tc  <<	"GET /cgi-bin/webcm?getpage=../html/"
				<<  Tools::GetLang()
				<<	"/menus/menu2.html"
				<<	"&var:lang="
				<<  Tools::GetLang()
				<<  "&var:pagename=fonbuch&var:menu=fon HTTP/1.1\n\n\0";
			tc >> msg;

			size_t pos, p1, p2;
			// determine charset (default for old firmware versions is iso-8859-15)
			std::string charset = "ISO-8859-15";
			pos = msg.find("<meta http-equiv=content-type");
			if (pos != std::string::npos) {
				pos = msg.find("charset=", pos);
				if (pos != std::string::npos)
					charset = msg.substr(pos+8, msg.find('"', pos)-pos-8);
			}
			*dsyslog << __FILE__ << ": using charset " << charset << std::endl;

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
					//*dsyslog << __FILE__ << ": (%s / %s)", fe.number.c_str(), fe.name.c_str() << std::endl;
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
					//*dsyslog << __FILE__ << ": (%s / %s / %i)", fe.number.c_str(), fe.name.c_str(), fe.type << std::endl;
				}
				pos = p2+10;
				count++;
			}
			*isyslog << __FILE__ << ": read " << count << " entries." << std::endl;
			setInitialized(true);
		} catch (tcpclient::TcpException te) {
			//cFritzTools::getFritzBoxMutex()->Unlock();
			*esyslog << __FILE__ << ": cTcpException - " << te.what() << std::endl;
			*esyslog << __FILE__ << ": waiting " << retry_delay << " seconds before retrying" << std::endl;
			sleep(retry_delay); // delay a possible retry
		} catch (ToolsException te) {
			//cFritzTools::getFritzBoxMutex()->Unlock();
			*esyslog << __FILE__ << ": cToolsException - " << te.what() << std::endl;
			*esyslog << __FILE__ << ": waiting " << retry_delay << " seconds before retrying" << std::endl;
			sleep(retry_delay); // delay a possible retry
		}
	} while (!isInitialized());
	Tools::GetFritzBoxMutex()->Unlock();
	std::sort(fonbookList.begin(), fonbookList.end());
}

void FritzFonbook::Reload() {
	this->Start();
}

}

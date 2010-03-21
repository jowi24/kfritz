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


#include <unistd.h>
#include <TcpClient++.h>
#include "Config.h"
#include "Nummerzoeker.h"
#include "Tools.h"

namespace fritz{

NummerzoekerFonbook::NummerzoekerFonbook()
{
	title = I18N_NOOP("nummerzoeker.com");
	techId = "ZOEK";
	displayable = false;
}

NummerzoekerFonbook::~NummerzoekerFonbook()
{
}

bool NummerzoekerFonbook::Initialize() {
	setInitialized(true);
	return true;
}

Fonbook::sResolveResult NummerzoekerFonbook::ResolveToName(std::string number) {
	Fonbook::sResolveResult result;
	result.name = number;
	result.type = FonbookEntry::TYPE_NONE;

	// resolve only NL phone numbers
	std::string normNumber = Tools::NormalizeNumber(number);
	if (normNumber.find("0031") != 0)
		return result;

	// __FILE__om works only with national number: remove 0031 prefix, add 0
	normNumber = "0" + normNumber.substr(4);

	std::string msg;
	try {
		DBG("sending reverse lookup request for " << normNumber << " to www.nummerzoeker.com");
		std::string host = "www.nummerzoeker.com";
		tcpclient::HttpClient tc(host);
		tc << tcpclient::get
		   << "/index.php?search=Zoeken&phonenumber="
		   << normNumber
		   << "&export=csv"
		   << "\nAccept-Charset: ISO-8859-1\nUser-Agent: Lynx/2.8.5"
		   << std::flush;
		tc >> msg;
	} catch (tcpclient::TcpException te) {
		ERR("Exception - " << te.what());
		return result;
	}

	if (msg.find("Content-Type: text/html") != std::string::npos) {
		INF("no entry found.");
		return result;
	}

	// parse answer, format is "number",name,surname,street,zip,city
	size_t lineStart = 0;
	std::string name, surname;
	while ((lineStart = msg.find("\n", lineStart)) != std::string::npos) {
	  lineStart++;
	  if (msg[lineStart] == '"') {
			size_t nameStart    = msg.find(",", lineStart);
			size_t surnameStart = msg.find(",", nameStart+1);
			size_t streetStart  = msg.find(",", surnameStart+1);
			name                = msg.substr(nameStart, surnameStart-nameStart-1);
			surname             = msg.substr(surnameStart, streetStart-surnameStart-1);
			name = surname + " " + name;
			break;
	  }
	}
	// convert the string from latin1 to current system character table
	// Q: is this really ISO-8859-1, the webservers' response is unclear (html pages are UTF8)
	CharSetConv *conv = new CharSetConv("ISO-8859-1", CharSetConv::SystemCharacterTable());
	const char *s_converted = conv->Convert(name.c_str());
	name = s_converted;
	delete (conv);
	INF("resolves to " << name.c_str());
	result.name = name;
	return result;
}

}

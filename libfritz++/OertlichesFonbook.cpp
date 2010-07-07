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
#include "OertlichesFonbook.h"
#include "Config.h"
#include "Tools.h"

namespace fritz{

OertlichesFonbook::OertlichesFonbook()
{
	title = I18N_NOOP("das-oertliche.de");
	techId = "OERT";
	displayable = false;
}

OertlichesFonbook::~OertlichesFonbook()
{
}

bool OertlichesFonbook::Initialize() {
	setInitialized(true);
	return true;
}

Fonbook::sResolveResult OertlichesFonbook::ResolveToName(std::string number) {
	Fonbook::sResolveResult result;
	result.name = number;
	result.type = FonbookEntry::TYPE_NONE;

	// resolve only german phone numbers
	if (Tools::NormalizeNumber(number).find("0049") != 0)
		return result;

	std::string msg;
	std::string name;
	try {
		DBG("sending reverse lookup request for " << Tools::NormalizeNumber(number) << " to www.dasoertliche.de");
		std::string host = "www.dasoertliche.de";
		tcpclient::HttpClient tc(host);
		tc << tcpclient::get
		   << "/Controller?topKw=0&form_name=search_nat&context=0&choose=true&page=0&rci=yes&action=43&kw=" << Tools::NormalizeNumber(number)
		   << "\nAccept-Charset: ISO-8859-1\nUser-Agent: Lynx/2.8.5"
		   << std::flush;
		tc >> msg;
	} catch (tcpclient::TcpException te) {
		ERR("Exception - " << te.what());
		return result;
	}
	// parse answer
	size_t start = msg.find("class=\"preview\">");
	if (start == std::string::npos) {
		INF("no entry found.");
		return result;
	}
	// add the length of search pattern
	start += 16;

	size_t stop  = msg.find("<", start);
	name = msg.substr(start, stop - start);
	// convert the string from latin1 to current system character table
	CharSetConv *conv = new CharSetConv("ISO-8859-1", CharSetConv::SystemCharacterTable());
	const char *s_converted = conv->Convert(name.c_str());
	name = s_converted;
	delete (conv);
	INF("resolves to " << name.c_str());
	result.name = name;
	return result;
}

}

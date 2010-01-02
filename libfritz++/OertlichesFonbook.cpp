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


#include <unistd.h>
#include <TcpClient++.h>
#include "OertlichesFonbook.h"
#include "Config.h"
#include "Tools.h"

namespace fritz{

OertlichesFonbook::OertlichesFonbook()
{
	title = "das-oertliche.de";
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

FonbookEntry &OertlichesFonbook::ResolveToName(FonbookEntry &fe) {
	std::string number = fe.getNumber();
	// resolve only german phone numbers
	if (Tools::NormalizeNumber(number).find("0049") != 0) {
		fe.setName(number);
		fe.setType(FonbookEntry::TYPE_NONE);
		return fe;
	}
	std::string msg;
	std::string name;
	try {
		*dsyslog << __FILE__ << ": sending reverse lookup request for " << Tools::NormalizeNumber(number) << " to www.dasoertliche.de" << std::endl;
		std::string host = "www.dasoertliche.de";
		tcpclient::HttpClient tc(host);
		tc << tcpclient::get
		   << "/Controller?ciid=&district=&kgs=&plz=&zvo_ok=&form_name=search_inv&buc=&kgs=&buab=&zbuab=&ph=" << Tools::NormalizeNumber(number) << "&image="
		   << "\nAccept-Charset: ISO-8859-1\nUser-Agent: Lynx/2.8.5"
		   << std::flush;
		tc >> msg;
	} catch (tcpclient::TcpException te) {
		*esyslog << __FILE__ << ": Exception - " << te.what() << std::endl;
		fe.setName(number);
		fe.setType(FonbookEntry::TYPE_NONE);
		return fe;
	}
	// parse answer
	size_t start = msg.find("class=\"preview\">");
	if (start == std::string::npos) {
		*isyslog << __FILE__ << ": no entry found." << std::endl;
		fe.setName(number);
		fe.setType(FonbookEntry::TYPE_NONE);
		return fe;
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
	*isyslog << __FILE__ << ": resolves to " << name.c_str() << std::endl;
	fe.setName(name);
	fe.setType(FonbookEntry::TYPE_NONE);
	return fe;
}

}

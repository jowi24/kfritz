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

#include "Config.h"
#include "FritzClient.h"
#include "Tools.h"
#include <TcpClient++.h>

namespace fritz {

FritzClient::FritzClient() {
	Tools::Login(); // TODO: move Login to FritzClient
}

FritzClient::~FritzClient() {

}

std::string FritzClient::RequestCallList () {
	unsigned int retry_delay = RETRY_DELAY / 2;
	std::string msg;
	bool callListRead = false;
	Tools::GetFritzBoxMutex()->Lock();
	do {
		try {
			retry_delay = retry_delay > 1800 ? 3600 : retry_delay * 2;
			// now, process call list
			*dsyslog << __FILE__ << ": sending callList request." << std::endl;
			// force an update of the fritz!box csv list and wait until all data is received
			tcpclient::HttpClient hc(gConfig->getUrl(), gConfig->getUiPort());
			hc << tcpclient::get
			<< "/cgi-bin/webcm?getpage=../html/"
			<<  Tools::GetLang()
			<< "/menus/menu2.html&var:lang="
			<<  Tools::GetLang()
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
			callListRead = true;
		} catch (tcpclient::TcpException te) {
			*esyslog << __FILE__ << ": Exception - " << te.what() << std::endl;
			*esyslog << __FILE__ << ": waiting " << retry_delay << " seconds before retrying" << std::endl;
			sleep(retry_delay); // delay a possible retry
		} catch (ToolsException te) {
			*esyslog << __FILE__ << ": Exception - " << te.what() << std::endl;
			*esyslog << __FILE__ << ": waiting " << retry_delay << " seconds before retrying" << std::endl;
			sleep(retry_delay); // delay a possible retry
		}
	} while (!callListRead);
	Tools::GetFritzBoxMutex()->Unlock();
	return msg;
}

//TODO: update lastRequestTime with any request
}

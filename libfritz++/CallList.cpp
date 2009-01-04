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

#include "CallList.h"
#include "Tools.h"
#include "Config.h"
#include <TcpClient++.h>
#include <time.h>
#include <stdlib.h>

namespace fritz{

CallList *CallList::me = NULL;

CallList::CallList()
:PThread("CallList")
{
	lastMissedCall = 0;
	callListRead = false;
	this->Start();
}

CallList *CallList::getCallList(bool create){
	if(!me && create){
		me = new CallList();
	}
	return me;
}

void CallList::CreateCallList() {
	DeleteCallList();
	me = new CallList();
}

void CallList::DeleteCallList(){
	if (me){
		*dsyslog << __FILE__ << ": deleting call list" << std::endl;
		delete me;
		me = NULL;
	}
}

CallList::~CallList()
{
}

void CallList::Action() {
	unsigned int retry_delay = RETRY_DELAY / 2;
	std::string msg;
	Tools::GetFritzBoxMutex()->Lock();
	do {
		try {
			retry_delay = retry_delay > 1800 ? 3600 : retry_delay * 2;
			// now, process call list
			Tools::Login();
			*dsyslog << __FILE__ << ": sending callList request." << std::endl;
			// force an update of the fritz!box csv list and wait until all data is received
			tcpclient::HttpClient tc2( gConfig->getUrl(), gConfig->getUiPort());
			tc2 << "GET /cgi-bin/webcm?getpage=../html/"
			<<  Tools::GetLang()
			<< "/menus/menu2.html&var:lang="
			<<  Tools::GetLang()
			<< "&var:pagename=foncalls&var:menu=fon HTTP/1.1\n\n\0";
			tc2 >> msg;
			// get the URL of the CSV-File-Export
			unsigned int urlPos   = msg.find(".csv");
			unsigned int urlStop  = msg.find('"', urlPos);
			unsigned int urlStart = msg.rfind('"', urlPos) + 1;
			std::string csvUrl    = msg.substr(urlStart, urlStop-urlStart);
			// retrieve csv list
			msg = "";
			tcpclient::HttpClient tc(gConfig->getUrl(), gConfig->getUiPort());
			tc << "GET /cgi-bin/webcm?getpage="
			<<  csvUrl
			<< " HTTP/1.1\n\n\0";
			tc >> msg;
			// convert answer to current SystemCodeSet (we assume, Fritz!Box sends its answer in latin15)
			CharSetConv *conv = new CharSetConv("ISO-8859-15", CharSetConv::SystemCharacterTable());
			const char *msg_converted = conv->Convert(msg.c_str());
			msg = msg_converted;
			delete(conv);
			// parse answer
			callListAll.clear();
			callListIn.clear();
			callListOut.clear();
			callListMissed.clear();
			lastMissedCall = 0;
			size_t pos = 0;
			// skip HTTP header data
			while (msg[pos] != '\r' && msg[pos] != '\n') {
				pos = msg.find("\n", pos) + 1;
			}
			pos += 2;
			// parse body
			int count = 0;
			while ((pos = msg.find("\n", pos)) != std::string::npos /*&& msg[pos+1] != '\n'*/) {
				pos++;
				int type          = pos;
				if (msg[type] < '0' || msg[type] > '9') { // ignore lines not starting with a number (headers, comments, etc.) {
					*dsyslog << __FILE__ << ": parser skipped line in calllist" << std::endl;
					continue;
				}
				int dateStart     = msg.find(';', type)          +1;
				int timeStart	  = msg.find(' ', dateStart)     +1;
				int nameStart     = msg.find(';', timeStart)     +1;
				int numberStart   = msg.find(';', nameStart)     +1;
				int lNameStart    = msg.find(';', numberStart)   +1;
				int lNumberStart  = msg.find(';', lNameStart)    +1;
				int durationStart = msg.find(';', lNumberStart)  +1;
				int durationStop  = msg.find("\n", durationStart)-1;
				if (msg[durationStop] == '\r') // fix for new Fritz!Box Firmwares that use "\r\n" on linebreak
					durationStop--;

				CallEntry ce;
				ce.type           = (CallEntry::callType)atoi(&msg[type]);
				ce.date           = msg.substr(dateStart,     timeStart     - dateStart     -1);
				ce.time           = msg.substr(timeStart,     nameStart     - timeStart     -1);
				ce.remoteName     = msg.substr(nameStart,     numberStart   - nameStart     -1);
				ce.remoteNumber   = msg.substr(numberStart,   lNameStart    - numberStart   -1);
				ce.localName      = msg.substr(lNameStart,    lNumberStart  - lNameStart    -1);
				ce.localNumber    = msg.substr(lNumberStart,  durationStart - lNumberStart  -1);
				ce.duration       = msg.substr(durationStart, durationStop -  durationStart +1);

				//       01234567        01234
				// date: dd.mm.yy, time: hh:mm
				tm tmCallTime;
				tmCallTime.tm_mday = atoi(ce.date.substr(0, 2).c_str());
				tmCallTime.tm_mon  = atoi(ce.date.substr(3, 2).c_str()) - 1;
				tmCallTime.tm_year = atoi(ce.date.substr(6, 2).c_str()) + 100;
				tmCallTime.tm_hour = atoi(ce.time.substr(0, 2).c_str());
				tmCallTime.tm_min  = atoi(ce.time.substr(2, 2).c_str());
				tmCallTime.tm_sec  = 0;

				ce.timestamp = mktime(&tmCallTime);
				//				dsyslog << "__FILE__<< " (%s / %s / %s / %s / %s / %s / %s)", ce.date.c_str(), ce.time.c_str(), ce.duration.c_str(), ce.remoteNumber.c_str(), ce.remoteName.c_str(), ce.localName.c_str(), ce.localNumber.c_str());

				// workaround for AVM debugging entries in CVS list
				if (ce.remoteNumber.compare("1234567") == 0 && ce.date.compare("12.03.2005") == 0)
					continue;

				callListAll.push_back(ce);
				switch (ce.type) {
				case CallEntry::INCOMING:
					callListIn.push_back(ce);
					break;
				case CallEntry::OUTGOING:
					callListOut.push_back(ce);
					break;
				case CallEntry::MISSED:
					if (lastMissedCall < ce.timestamp)
						lastMissedCall = ce.timestamp;
					callListMissed.push_back(ce);
					break;
				default:
					*dsyslog << __FILE__ << ": parser skipped unknown call type" << std::endl;
					continue;
				}
				count++;
			}
			std::cout.put('c');
			*isyslog << __FILE__ << ": CallList -> read " << count << " entries." << std::endl;
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
}

CallEntry *CallList::RetrieveEntry(CallEntry::callType type, size_t id) {
	if (id < 0)
		return NULL;
	switch (type) {
	case CallEntry::ALL:
		return &callListAll[id];
	case CallEntry::INCOMING:
		return &callListIn[id];
	case CallEntry::OUTGOING:
		return &callListOut[id];
	case CallEntry::MISSED:
		return &callListMissed[id];
	default:
		return NULL;
	}
}

size_t CallList::GetSize(CallEntry::callType type) {
	switch (type) {
	case CallEntry::ALL:
		return callListAll.size();
	case CallEntry::INCOMING:
		return callListIn.size();
	case CallEntry::OUTGOING:
		return callListOut.size();
	case CallEntry::MISSED:
		return callListMissed.size();
	default:
		return 0;
	}
}

size_t CallList::MissedCalls(time_t since) {
	size_t missedCalls = 0;
	for (unsigned int pos=0; pos < callListMissed.size(); pos++) {
		CallEntry ce = callListMissed[pos];
		// track number of new missed calls
		if (ce.timestamp > since) {
			if (ce.MatchesFilter())
				missedCalls++;
		} else {
			break; // no older calls will match the missed-calls condition
		}
	}
	return missedCalls;
}

bool CallEntry::MatchesFilter() {
	// entries are filtered according to the MSN filter)
	if ( Tools::MatchesMsnFilter(localNumber))
		return true;
	else{
		// if local number does not contain any of the MSNs in MSN filter, we test
		// if it does contain any number (if POTS is used fritzbox reports "Festnetz"
		// instead of the local number)
		for (unsigned int pos=0; pos < localNumber.size(); pos++) {
			if (localNumber[pos] >= '0' && localNumber[pos] <= '9')
				return false;
		}
		return true;
	}
}

}


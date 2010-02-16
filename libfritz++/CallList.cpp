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

#include "CallList.h"
#include "Tools.h"
#include "Config.h"
#include "FritzClient.h"
#include <time.h>
#include <stdlib.h>
#include <algorithm>

namespace fritz{

class CallEntrySort {
private:
	bool ascending;
	CallEntry::eElements element;
public:
	CallEntrySort(CallEntry::eElements element = CallEntry::ELEM_DATE, bool ascending = true) {
		this->element   = element;
		this->ascending = ascending;
	}
	bool operator() (CallEntry ce1, CallEntry ce2){
		switch(element) {
		case CallEntry::ELEM_DATE:
			return (ascending ? (ce1.timestamp < ce2.timestamp) : (ce1.timestamp > ce2.timestamp));
			break;
		case CallEntry::ELEM_DURATION:
			if (ce1.duration.size() < ce2.duration.size())
				return (ascending ? true : false);
			if (ce1.duration.size() > ce2.duration.size())
				return (ascending ? false : true);
			return (ascending ? (ce1.duration < ce2.duration) : (ce1.duration > ce2.duration));
			break;
		case CallEntry::ELEM_LOCALNAME:
			return (ascending ? (ce1.localName < ce2.localName) : (ce1.localName > ce2.localName));
			break;
		case CallEntry::ELEM_LOCALNUMBER:
			return (ascending ? (ce1.localNumber < ce2.localNumber) : (ce1.localNumber > ce2.localNumber));
			break;
		case CallEntry::ELEM_REMOTENAME:
			if (ce1.remoteName == "unknown" && ce2.remoteName == "unknown")
				return false;
			if (ce1.remoteName == "unknown")
				return (ascending ? true : false);
			if (ce2.remoteName == "unknown")
				return (ascending ? false : true);
			return (ascending ? (ce1.remoteName < ce2.remoteName) : (ce1.remoteName > ce2.remoteName));
			break;
		case CallEntry::ELEM_REMOTENUMBER:
			return (ascending ? (ce1.remoteNumber < ce2.remoteNumber) : (ce1.remoteNumber > ce2.remoteNumber));
			break;
		case CallEntry::ELEM_TYPE:
			return (ascending ? (ce1.type < ce2.type) : (ce1.type > ce2.type));
			break;
		default:
			ERR("invalid element given for sorting.");
			return false;
		}
	}
};

CallList *CallList::me = NULL;

CallList::CallList()
:PThread("CallList")
{
	lastMissedCall = 0;
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

void CallList::DeleteCallList() {
	if (me) {
		DBG("deleting call list");
		delete me;
		me = NULL;
	}
}

CallList::~CallList()
{
	// don't delete the object, while the thread is still active
	while (Active())
		pthread::CondWait::SleepMs(100);
	DBG("deleting call list");
}

void CallList::Action() {
	FritzClient fc;
	std::string msg = fc.RequestCallList();
	// parse answer
	callListAll.clear();
	callListIn.clear();
	callListOut.clear();
	callListMissed.clear();
	lastCall = 0;
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
			DBG("parser skipped line in calllist");
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
		ce.type           = (CallEntry::eCallType)atoi(&msg[type]);
		ce.date           = msg.substr(dateStart,     timeStart     - dateStart     -1);
		ce.time           = msg.substr(timeStart,     nameStart     - timeStart     -1);
		ce.remoteName     = msg.substr(nameStart,     numberStart   - nameStart     -1);
		ce.remoteNumber   = msg.substr(numberStart,   lNameStart    - numberStart   -1);
		ce.localName      = msg.substr(lNameStart,    lNumberStart  - lNameStart    -1);
		ce.localNumber    = msg.substr(lNumberStart,  durationStart - lNumberStart  -1);
		ce.duration       = msg.substr(durationStart, durationStop -  durationStart +1);

		// put the number into the name field if name is not available
		if (ce.remoteName.size() == 0)
			ce.remoteName = ce.remoteNumber;
		// if also no number is available, put "unknown" into the name field
		if (ce.remoteName.size() == 0)
			ce.remoteName = I18N_NOOP("unknown");

		//       01234567        01234
		// date: dd.mm.yy, time: hh:mm
		tm tmCallTime;
		tmCallTime.tm_mday = atoi(ce.date.substr(0, 2).c_str());
		tmCallTime.tm_mon  = atoi(ce.date.substr(3, 2).c_str()) - 1;
		tmCallTime.tm_year = atoi(ce.date.substr(6, 2).c_str()) + 100;
		tmCallTime.tm_hour = atoi(ce.time.substr(0, 2).c_str());
		tmCallTime.tm_min  = atoi(ce.time.substr(3, 2).c_str());
		tmCallTime.tm_sec  = 0;
		tmCallTime.tm_isdst = 0;

		ce.timestamp = mktime(&tmCallTime);

		// workaround for AVM debugging entries in CVS list
		if (ce.remoteNumber.compare("1234567") == 0 && ce.date.compare("12.03.2005") == 0)
			continue;

		callListAll.push_back(ce);

		if (lastCall < ce.timestamp)
			lastCall = ce.timestamp;
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
			DBG("parser skipped unknown call type");
			continue;
		}
		count++;
	}
	INF("CallList -> read " << count << " entries.");
}

CallEntry *CallList::RetrieveEntry(CallEntry::eCallType type, size_t id) {
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

size_t CallList::GetSize(CallEntry::eCallType type) {
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

void CallList::Sort(CallEntry::eElements element, bool ascending) {
	CallEntrySort ces(element, ascending);
	std::sort(callListAll.begin(), callListAll.end(), ces); //TODO: other lists?
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


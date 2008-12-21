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


#ifndef FRITZLISTENER_H_
#define FRITZLISTENER_H_

#include <string>
#include <PThread++.h>
#include <TcpClient++.h>
#include "CallList.h"
#include "Fonbook.h"

namespace fritz{

class sCallInfo{
public:
	bool isOutgoing;
	std::string remoteNumber;
	std::string remoteName;
	std::string localNumber;
	std::string medium;
};

class EventHandler {

public:
	EventHandler() { }
	virtual ~EventHandler() { }

	virtual void HandleCall(bool outgoing, int connId, std::string remoteParty, std::string localParty, std::string medium) = 0;
	virtual void HandleConnect(int connId) = 0;
	virtual void HandleDisconnect(int connId, std::string duration) = 0;
};

class Listener : public pthread::PThread
{
private:
	EventHandler *event;
	CallList *callList;
	tcpclient::TcpClient *tcpclient;

public:
	Listener(EventHandler *event, CallList *callList);
	virtual ~Listener();
	virtual void Action();
};

}

#endif /*FRITZLISTENER_H_*/

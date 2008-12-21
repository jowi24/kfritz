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
#include <stdlib.h>
#include <sstream>
#include <vector>
#include "Fonbook.h"
#include "Config.h"
#include "Listener.h"

namespace fritz{

Listener::Listener(EventHandler *event, CallList *callList)
:PThread("fritzlistener")
{
	this->callList = callList;
	this->event = event;
	tcpclient = new tcpclient::TcpClient(gConfig->getUrl(), PORT_MONITOR);
}

Listener::~Listener()
{
	delete tcpclient;
}



void Listener::Action() {
	std::string data = "";
	unsigned int retry_delay = RETRY_DELAY / 2;
	while (true) {
		try {
			retry_delay = retry_delay > 1800 ? 3600 : retry_delay * 2;
			while (true) {
				*dsyslog << __FILE__ << ": Waiting for a message." << std::endl;
				//data.erase();
				if (data.length() == 0) {
					*tcpclient >> data;
					if (data.length() == 0) {
						// i.e., connection to Fritz!Box closed
						// wait, then retry by setting up a new connection
						throw tcpclient::TcpException(tcpclient::TcpException::ERR_INVALID_DATA);
					}
					*dsyslog << __FILE__ << ": Got message " << data << std::endl;
				}
				// parse message
				size_t pos[8];
				memset(pos, -1, 8 * sizeof(size_t));
				pos[0] = 0;
				for (int i=1; i<8; i++) {
					pos[i] = data.find(";", pos[i-1]+1);
					if (pos[i] == std::string::npos)
						break;
				}
				std::string date    = data.substr(pos[0], pos[1]-pos[0]);
				std::string type    = data.substr(pos[1]+1, pos[2]-pos[1]-1);
				int connId 			= atoi(data.substr(pos[2]+1, pos[3]-pos[2]-1).c_str());
				std::string partA   = pos[4] == std::string::npos ? "" : data.substr(pos[3]+1, pos[4]-pos[3]-1);
				std::string partB   = pos[5] == std::string::npos ? "" : data.substr(pos[4]+1, pos[5]-pos[4]-1);
				std::string partC   = pos[6] == std::string::npos ? "" : data.substr(pos[5]+1, pos[6]-pos[5]-1);
				std::string partD   = pos[7] == std::string::npos ? "" : data.substr(pos[6]+1, pos[7]-pos[6]-1);


				if (type.compare("CALL") == 0) {
					// partA => box port
					// partB => caller Id
					// partC => called Id
					// partD => medium (POTS, SIP[1-9], ISDN, ...)

					// an '#' can be appended to outgoing calls by the phone, so delete it
					if (partC[partC.length()-1] == '#')
						partC = partC.substr(0, partC.length()-1);
					// init display, muting or replay pausing
					if (event) event->HandleCall(true, connId, partC, partB, partD);

				} else if (type.compare("RING") == 0) {
					// partA => caller Id
					// partB => called Id
					// partC => medium (POTS, SIP[1-9], ISDN, ...)

					// init display, muting or replay pausing
					if (event) event->HandleCall(false, connId, partA, partB, partC);

				} else if (type.compare("CONNECT") == 0) {
					// partA => box port
					// partB => Id

					// stop call notification
					if (event) event->HandleConnect(connId);
				} else if (type.compare("DISCONNECT") == 0) {
					// partA => call duration

					if (event) event->HandleDisconnect(connId, partA);
					// force reload of callList
					if (callList)
						callList->Start();
				} else {
					throw tcpclient::TcpException(tcpclient::TcpException::ERR_INVALID_DATA);
				}
				// remove first line in data
				size_t nl = data.find('\n', 0);
				if (nl != std::string::npos) {
					data = data.substr(nl+1);
				} else {
					data.erase();
				}
			}
		} catch (tcpclient::TcpException te) {
			*esyslog << __FILE__ << ": Exception - " << te.what() << std::endl;
			if (te.errcode == tcpclient::TcpException::ERR_HOST_NOT_REACHABLE || te.errcode == tcpclient::TcpException::ERR_CONNECTION_REFUSED) {
				*esyslog << __FILE__ << ": Make sure to enable the Fritz!Box call monitor by dialing #96*5* once." << std::endl;
			}
			tcpclient->Disconnect();
		}
		*esyslog << __FILE__ << ": waiting " << retry_delay << " seconds before retrying" << std::endl;
		sleep(retry_delay); // delay the retry
	}
}

}

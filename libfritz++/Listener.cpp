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
#include "FonbookManager.h"
#include "Config.h"
#include "Listener.h"
#include "Tools.h"

namespace fritz{

Listener *Listener::me = NULL;

Listener::Listener(EventHandler *event)
:PThread("fritzlistener")
{
	this->event = event;
	tcpclient = new tcpclient::TcpClient(gConfig->getUrl(), PORT_MONITOR);
	this->Start();
}

Listener::~Listener()
{
	this->Cancel();
	delete tcpclient;
}

void Listener::CreateListener(EventHandler *event) {
	EventHandler *oldEvent = me ? me->event : NULL;
	DeleteListener();
	if (event || oldEvent)
		me = new Listener(event ? event : oldEvent);
	else
		*esyslog << __FILE__ << ": Invalid call parameter. First call to CreateListener needs event handler object." << std::endl;
}

void Listener::DeleteListener() {
	if (me) {
		*dsyslog << __FILE__ << ": deleting listener" << std::endl;
		delete me;
		me = NULL;
	}
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

				// copy message in buffer
				char *buffer;
				int ret = asprintf(&buffer, "%s", data.c_str());
				if (ret <= 0){
					*esyslog << __FILE__ << ": Could not allocate memory" << std::endl;
					continue;
				}
				// parse buffer
				char *tok[7];
				for (size_t i=0; i<7; i++) {
					tok[i] = strtok(i==0 ? buffer : NULL, ";");
				}
				std::string date  = tok[0];
				std::string type  = tok[1];
				int connId        = atoi(tok[2]);
				std::string partA = tok[3] ? tok[3] : "";
				std::string partB = tok[4] ? tok[4] : "";
				std::string partC = tok[5] ? tok[5] : "";
				std::string partD = tok[6] ? tok[6] : "";
				// after copying all tokens, delete buffer
				free(buffer);

				if (type.compare("CALL") == 0) {
					// partA => box port
					// partB => caller Id (local)
					// partC => called Id (remote)
					// partD => medium (POTS, SIP[1-9], ISDN, ...)

					// an '#' can be appended to outgoing calls by the phone, so delete it
					if (partC[partC.length()-1] == '#')
						partC = partC.substr(0, partC.length()-1);

					if ( Tools::MatchesMsnFilter(partB) ) {
						// do reverse lookup
						std::string remoteName;
						fritz::FonbookEntry fe(remoteName, partC);
						FonbookManager::GetFonbook()->ResolveToName(fe);
						// resolve SIP names
						std::string mediumName;
						if (partD.find("SIP")           != std::string::npos &&
						    gConfig->getSipNames().size() >= (size_t)atoi(&partD[3]))
							mediumName = gConfig->getSipNames()[atoi(&partD[3])];
						else
							mediumName = partD;
						// notify application
						if (event) event->HandleCall(true, connId, partC, fe.getName(), fe.getTypeName(), partB, partD, mediumName);
						activeConnections.push_back(connId);
					}

				} else if (type.compare("RING") == 0) {
					// partA => caller Id (remote)
					// partB => called Id (local)
					// partC => medium (POTS, SIP[1-9], ISDN, ...)

					if ( Tools::MatchesMsnFilter(partB) ) {
						// do reverse lookup
						std::string remoteName;
						fritz::FonbookEntry fe(remoteName, partA);
						FonbookManager::GetFonbook()->ResolveToName(fe);
						// resolve SIP names
						std::string mediumName;
						if (partC.find("SIP")           != std::string::npos &&
						    gConfig->getSipNames().size() >= (size_t)atoi(&partC[3]))
							mediumName = gConfig->getSipNames()[atoi(&partC[3])];
						else
							mediumName = partC;
						// notify application
						if (event) event->HandleCall(false, connId, partA, fe.getName(), fe.getTypeName(), partB, partC, mediumName);
						activeConnections.push_back(connId);
					}
				} else if (type.compare("CONNECT") == 0) {
					// partA => box port
					// partB => Id
					// only notify application if this connection is part of activeConnections
					bool notify = false;
					for (std::vector<int>::iterator it = activeConnections.begin(); it < activeConnections.end(); it++) {
						if (*it == connId) {
							notify = true;
							break;
						}
					}
					if (notify)
						if (event) event->HandleConnect(connId);
				} else if (type.compare("DISCONNECT") == 0) {
					// partA => call duration
					// only notify application if this connection is part of activeConnections
					bool notify = false;
					for (std::vector<int>::iterator it = activeConnections.begin(); it < activeConnections.end(); it++) {
						if (*it == connId) {
							activeConnections.erase(it);
							notify = true;
							break;
						}
					}
					if (notify) {
						if (event) event->HandleDisconnect(connId, partA);
						// force reload of callList
						CallList *callList = CallList::getCallList(false);
						if (callList)
							callList->Start();
					}
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

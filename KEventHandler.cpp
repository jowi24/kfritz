/*
 * KFritzBox
 *
 * Copyright (C) 2008 Joachim Wilke <vdr@joachim-wilke.de>
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


#include "KEventHandler.h"

KEventHandler::KEventHandler() {

}

KEventHandler::~KEventHandler() {

}

void KEventHandler::HandleCall(bool outgoing, int connId, std::string remoteParty, std::string localParty, std::string medium)
{
	QString message;
	if (outgoing)
		message=i18n("Outgoing call to %1 using %2", remoteParty.c_str(), medium.c_str());
	else
		message=i18n("Incoming call from %1 using %2", remoteParty.c_str(), medium.c_str());

	emit notify(message);
}

void KEventHandler::HandleConnect(int connId)
{
}

void KEventHandler::HandleDisconnect(int connId, std::string duration)
{
}







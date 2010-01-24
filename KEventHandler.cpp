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

#include <Tools.h>
#include <KNotification>

KEventHandler::KEventHandler() {
	inputCodec  = QTextCodec::codecForName(fritz::CharSetConv::SystemCharacterTable() ? fritz::CharSetConv::SystemCharacterTable() : "UTF-8");
}

KEventHandler::~KEventHandler() {

}

void KEventHandler::HandleCall(bool outgoing, int connId __attribute__((unused)), std::string remoteNumber, std::string remoteName, std::string remoteType, std::string localParty __attribute__((unused)), std::string medium __attribute__((unused)), std::string mediumName)
{
	QString qRemoteName = inputCodec->toUnicode(remoteName.c_str());
	if (remoteType.size() > 0){
		qRemoteName += " ";
		qRemoteName += remoteType.c_str();
	}
	//QString qLocalParty = inputCodec->toUnicode(localParty.c_str());
	QString qMediumName     = inputCodec->toUnicode(mediumName.c_str());
	QString qMessage;
	if (outgoing)
		qMessage=i18n("Outgoing call to %1 using %2",   qRemoteName.size() ? qRemoteName : remoteNumber.c_str(),                                    qMediumName);
	else
		qMessage=i18n("Incoming call from %1 using %2", qRemoteName.size() ? qRemoteName : remoteNumber.size() ? remoteNumber.c_str() : "unknown",  qMediumName);

	KNotification *notification= new KNotification ( "incomingCall" );
	notification->setText(qMessage);
	notification->sendEvent();

}

void KEventHandler::HandleConnect(int connId __attribute__((unused)))
{
}

void KEventHandler::HandleDisconnect(int connId __attribute__((unused)), std::string duration __attribute__((unused)))
{
}







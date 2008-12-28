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

#ifndef KEVENTHANDLER_H_
#define KEVENTHANDLER_H_

#include <KAboutData>
#include <QObject>
#include <QTextCodec>
#include <Listener.h>

class KEventHandler: public QObject, public fritz::EventHandler {
	Q_OBJECT

private:
	QTextCodec *inputCodec;
public:
	KEventHandler();
	virtual ~KEventHandler();

	virtual void HandleCall(bool outgoing, int connId, std::string remoteNumber, std::string remoteName, std::string remoteType, std::string localParty, std::string medium, std::string mediumName);
	virtual void HandleConnect(int connId);
	virtual void HandleDisconnect(int connId, std::string duration);
signals:
	void notify(QString m);
};

#endif /* KEVENTHANDLER_H_ */

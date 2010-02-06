/*
 * KFritz
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

#ifndef KFRITZBOXWINDOW_H_
#define KFRITZBOXWINDOW_H_

#include <KXmlGuiWindow>
#include <KTextEdit>
#include <KTabWidget>
#include <KNotification>
#include <KWallet/Wallet>
#include <QTreeView>
#include <QTextCodec>

#include "KFonbookModel.h"
#include "KCalllistModel.h"
#include "LibFritzInit.h"
#include "QAdaptTreeView.h"

#include "pkg-config.h"
#ifdef INDICATEQT_FOUND
#include <qindicateindicator.h>
#include <qindicateserver.h>
#else
namespace QIndicate { class Indicator; class Server; }
#endif


class KFritzWindow : public KXmlGuiWindow, public fritz::EventHandler
{
	Q_OBJECT
private:
	QVector<QAdaptTreeView*> treeFonbooks;
	KTabWidget *tabWidget;
	KTextEdit *logArea;
	LibFritzInit *libFritzInit;
	QString fbPassword;
	QString appName;
	QString programName;
	KNotification *notification;
	QIndicate::Indicator *missedCallsIndicator;
	void saveToWallet(KWallet::Wallet *wallet);
	bool showPasswordDialog(QString &password, bool offerSaving = false);
	void setupActions();
    void initIndicator();
    virtual bool queryClose();
Q_SIGNALS:
	void signalNotification(QString event, QString qMessage, bool persistent);
private Q_SLOTS:
	void slotNotification(QString event, QString qMessage, bool persistent);
	void notificationClosed();
public  Q_SLOTS:
    void updateMissedCallsIndicator();
    void find();
    void findNext();
    void findPrev();
public:
    KFritzWindow();
    virtual ~KFritzWindow();
	virtual void HandleCall(bool outgoing, int connId, std::string remoteNumber, std::string remoteName, fritz::FonbookEntry::eType, std::string localParty, std::string medium, std::string mediumName);
	virtual void HandleConnect(int connId);
	virtual void HandleDisconnect(int connId, std::string duration);
public Q_SLOTS:
	void showSettings();
	void showNotificationSettings();
	void updateConfiguration(const QString &dialogName = QString());
	void reenterPassword();
	void showMainWindow();
	void showMissedCalls(QIndicate::Indicator* indicator);
	void showLog(bool b);
};

#endif /*KFRITZBOXWINDOW_H_*/
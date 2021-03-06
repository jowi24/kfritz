/*
 * KFritz
 *
 * Copyright (C) 2010-2013 Joachim Wilke <kfritz@joachim-wilke.de>
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

#ifndef KFRITZWINDOW_H
#define KFRITZWINDOW_H

#include <vector>
#include <QtCore>
#include <QTreeView>
#include <QTextCodec>
#include <KXmlGuiWindow>
#include <KTextEdit>
#include <KTabWidget>
#include <KNotification>
#include <KWallet/Wallet>

#include "KFonbookModel.h"
#include "KCalllistModel.h"
#include "KFritzDbusService.h"
#include "LibFritzInit.h"
#include "LogDialog.h"
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
	KTabWidget *tabWidget;
	LogDialog *logDialog;
	LibFritzInit *libFritzInit;
	KFritzDbusService *dbusIface;
    QString fbUsername;
	QString fbPassword;
	QString appName;
	QString programName;
	KNotification *notification;
	QAdaptTreeView *treeCallList;
	QIndicate::Indicator *missedCallsIndicator;
	QWidget *progressIndicator;
	QTextCodec *inputCodec;
	QString toUnicode(const std::string string) const;
	void saveToWallet(KWallet::Wallet *wallet);
    bool showPasswordDialog(QString &username, QString &password, bool offerSaving = false);
	void setupActions();
    void initIndicator();
    void setProgressIndicator(QString message = QString());
    virtual bool queryClose();
    std::string getCurrentNumber();
    fritz::FonbookEntry::eType mapIndexToType(int index);
Q_SIGNALS:
	void signalNotification(QString event, QString qMessage, bool persistent);
private Q_SLOTS:
	void slotNotification(QString event, QString qMessage, bool persistent);
	void notificationClosed();
public  Q_SLOTS:
    void updateMissedCallsIndicator();
    void showStatusbarBoxBusy(bool b);
    void updateMainWidgets(bool b);
    void save();
    void quit();
public:
    KFritzWindow();
    virtual ~KFritzWindow();
    virtual void handleCall(bool outgoing, int connId, std::string remoteNumber, std::string remoteName, fritz::FonbookEntry::eType, std::string localParty, std::string medium, std::string mediumName) override;
    virtual void handleConnect(int connId) override;
    virtual void handleDisconnect(int connId, std::string duration) override;
public Q_SLOTS:
	void showSettings();
	void showNotificationSettings();
	void updateConfiguration(const QString &dialogName = QString());
	void reenterPassword();
	void showMainWindow();
	void showMissedCalls(QIndicate::Indicator* indicator);
	void showLog();
    void dialNumber();
    void copyNumberToClipboard();
    void setDefault();
    void setType(int index);
    void reload();
    void reconnectISP();
    void getIP();
    void addEntry(fritz::FonbookEntry *fe = NULL);
    void deleteEntry();
    void cutEntry();
    void copyEntry();
    void pasteEntry();
    void resolveNumber();
    void updateActionProperties(int tabIndex);
    void updateCallListContextMenu(const QModelIndex &current, const QModelIndex &previous);
    void updateFonbookContextMenu(const QModelIndex &current, const QModelIndex &previous);
    void updateFonbookState();
};

#endif /*KFRITZBOXWINDOW_H_*/

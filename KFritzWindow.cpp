/*
 * KFritz
 *
 * Copyright (C) 2010 Joachim Wilke <kfritz@joachim-wilke.de>
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
#include "KFritzWindow.h"

#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KLocale>
#include <KAboutData>
#include <KFindDialog>
#include <KStandardAction>
#include <KService>
#include <KStatusBar>
#include <KConfigSkeleton>
#include <KConfigDialog>
#include <KMessageBox>
#include <KNotifyConfigWidget>
#include <KPasswordDialog>
#include <QTextCodec>
#include <QClipboard>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QStackedLayout>

#include <Config.h>
#include <Tools.h>
#include <CallList.h>
#include <FritzClient.h>

#include "DialDialog.h"
#include "KSettings.h"
#include "KSettingsFonbooks.h"
#include "KSettingsFritzBox.h"
#include "Log.h"

/* General TODOs
 *
 * TODO: Support multiple FBs (via profiles or similar)
 * TODO: Add "find" action
 * TODO: Copy phone books
 * TODO: Show status information of FB in own tab (suggested by Sir_Aim <aim@perception.de>)
 * TODO: Configure Port forwarding (suggested by Sir_Aim <aim@perception.de>)
 * TODO: Configure DynDNS (suggested by Sir_Aim <aim@perception.de>)
 * TODO: Handle empty FB password (reported by Richard Bos)
 */


KFritzWindow::KFritzWindow()
{
	appName     = KGlobal::mainComponent().aboutData()->appName();
	programName = KGlobal::mainComponent().aboutData()->programName();
	notification = NULL;

	connect(this, SIGNAL(signalNotification(QString, QString, bool)), this, SLOT(slotNotification(QString, QString, bool)));

	initIndicator();
	updateMissedCallsIndicator();
	progressIndicator = NULL;

	logDialog = new LogDialog(this);
	KTextEdit *logArea = logDialog->getLogArea();
	fritz::Config::SetupLogging(LogStream::getLogStream(LogBuf::DEBUG)->setLogWidget(logArea),
							    LogStream::getLogStream(LogBuf::INFO)->setLogWidget(logArea),
					            LogStream::getLogStream(LogBuf::ERROR)->setLogWidget(logArea));
	bool savetoWallet = false;
	bool requestPassword = true;

	KWallet::Wallet *wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), winId());
	if (wallet) {
		if (wallet->hasFolder(appName)) {
			wallet->setFolder(appName);
			if (wallet->hasEntry(KSettings::hostname())) {
				if (wallet->readPassword(KSettings::hostname(), fbPassword) == 0) {
					DBG("Got password data from KWallet.");
						requestPassword = false;
				}
			} else {
				DBG("No password for this host available.");
			}
		} else {
			DBG("No relevant data in KWallet yet.");
		}
	} else {
		INF("No access to KWallet.");
	}

	if (requestPassword)
		savetoWallet = showPasswordDialog(fbPassword, wallet != NULL);

	tabWidget = NULL;

	libFritzInit = new LibFritzInit(fbPassword, this);
	connect(libFritzInit, SIGNAL(ready(bool)),       this, SLOT(showStatusbarBoxBusy(bool)));
	connect(libFritzInit, SIGNAL(invalidPassword()), this, SLOT(reenterPassword()));
	connect(libFritzInit, SIGNAL(ready(bool)),       this, SLOT(updateMainWidgets(bool)));
	libFritzInit->start();

	if (wallet && savetoWallet)
		saveToWallet(wallet);

	setupActions();

	setupGUI();
}

KFritzWindow::~KFritzWindow()
{
	// move logging to console
	fritz::Config::SetupLogging(&std::clog, &std::cout, &std::cerr);

	fritz::FonbookManager::DeleteFonbookManager();
	fritz::CallList::DeleteCallList();

	delete libFritzInit;
}

void KFritzWindow::HandleCall(bool outgoing, int connId __attribute__((unused)), std::string remoteNumber, std::string remoteName, fritz::FonbookEntry::eType type, std::string localParty __attribute__((unused)), std::string medium __attribute__((unused)), std::string mediumName)
{
	QTextCodec *inputCodec  = QTextCodec::codecForName(fritz::CharSetConv::SystemCharacterTable() ? fritz::CharSetConv::SystemCharacterTable() : "UTF-8");
	QString qRemoteName    = inputCodec->toUnicode(remoteName.c_str());
	QString qTypeName      = KFonbookModel::getTypeName(type);

	if (qTypeName.size() > 0)
		qRemoteName += " (" + qTypeName + ")";

	//QString qLocalParty = inputCodec->toUnicode(localParty.c_str());
	QString qMediumName    = inputCodec->toUnicode(mediumName.c_str());
	QString qMessage;
	if (outgoing)
		qMessage=i18n("Outgoing call to <b>%1</b><br/>using %2",   qRemoteName.size() ? qRemoteName : remoteNumber.c_str(),                                    qMediumName);
	else
		qMessage=i18n("Incoming call from <b>%1</b><br/>using %2", qRemoteName.size() ? qRemoteName : remoteNumber.size() ? remoteNumber.c_str() : i18n("unknown"),  qMediumName);

	emit signalNotification(outgoing ? "outgoingCall" : "incomingCall", qMessage, true);
}

void KFritzWindow::HandleConnect(int connId __attribute__((unused)))
{
	if (notification)
		notification->close();
	emit signalNotification("callConnected", i18n("Call connected."), false);
}

void KFritzWindow::HandleDisconnect(int connId __attribute__((unused)), std::string duration)
{
	if (notification)
		notification->close();
	std::stringstream ss(duration);
	int seconds;
	ss >> seconds;

	QTime time(seconds/3600,seconds%3600/60,seconds%60);
	QString qMessage = i18n("Call disconnected (%1).", time.toString("H:mm:ss"));
	emit signalNotification("callDisconnected", qMessage, false);
}

void KFritzWindow::slotNotification(QString event, QString qMessage, bool persistent) {
	notification = new KNotification (event, this, persistent ? KNotification::Persistent : KNotification::CloseOnTimeout);
	KIcon ico(event == "incomingCall" ? "incoming-call" :
			  event == "outgoingCall" ? "outgoing-call" :
					                    "internet-telephony");
	notification->setTitle(programName);
	notification->setPixmap(ico.pixmap(64, 64));
	notification->setText(qMessage);
	notification->sendEvent();
	connect(notification, SIGNAL(closed()), this, SLOT(notificationClosed()));
}

void KFritzWindow::notificationClosed() {
	notification = NULL;
}

void KFritzWindow::showSettings() {
	KConfigDialog *confDialog = new KConfigDialog(this, "settings", KSettings::self());

	QWidget *frameFritzBox = new KSettingsFritzBox(this);
    confDialog->addPage(frameFritzBox, i18n("Fritz!Box"), "modem", i18n("Configure connection to Fritz!Box"));

    QWidget *frameFonbooks = new KSettingsFonbooks(this);
	confDialog->addPage(frameFonbooks, i18n("Phone books"), "x-office-address-book", i18n("Select phone books to use"));

	connect(confDialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(updateConfiguration(const QString &)));

	confDialog->show();
}

void KFritzWindow::showNotificationSettings() {
    KNotifyConfigWidget::configure(this);
}

void KFritzWindow::updateConfiguration(const QString &dialogName __attribute__((unused))) {
	libFritzInit->setPassword(fbPassword);
	libFritzInit->start();
}

void KFritzWindow::reenterPassword() {
	KWallet::Wallet *wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0);

	bool keepPassword = showPasswordDialog(fbPassword, wallet != NULL);

	if (wallet && keepPassword)
		saveToWallet(wallet);
	updateConfiguration();
}

void KFritzWindow::saveToWallet(KWallet::Wallet *wallet) {
	DBG("Trying to save password...");
	if (wallet->hasFolder(appName) || wallet->createFolder(appName)) {
		wallet->setFolder(appName);
		if (wallet->writePassword(KSettings::hostname(), fbPassword) == 0) {
			INF("Saved password to KWallet.");
		}
	} else {
		ERR("Error accessing KWallet.")
	}
}

bool KFritzWindow::showPasswordDialog(QString &password, bool offerSaving) {
	KPasswordDialog pwd(this, offerSaving ? KPasswordDialog::ShowKeepPassword : KPasswordDialog::NoFlags);
	pwd.setPrompt(i18n("Enter your Fritz!Box password"));
	pwd.exec();
	password = pwd.password();
	return pwd.keepPassword();
}

void KFritzWindow::setupActions() {
	KAction* aShowSettings = new KAction(this);
	aShowSettings->setText(i18n("Configure KFritz..."));
	aShowSettings->setIcon(KIcon("preferences-other"));
	actionCollection()->addAction("showSettings", aShowSettings);
	connect(aShowSettings, SIGNAL(triggered(bool)), this, SLOT(showSettings()));

	KAction* aShowNotifySettings = new KAction(this);
	aShowNotifySettings->setText(i18n("Configure Notifications..."));
	aShowNotifySettings->setIcon(KIcon("preferences-desktop-notification"));
	actionCollection()->addAction("showNotifySettings", aShowNotifySettings);
	connect(aShowNotifySettings, SIGNAL(triggered(bool)), this, SLOT(showNotificationSettings()));

	KAction *aShowLog = new KAction(this);
	aShowLog->setText(i18n("Show log"));
	aShowLog->setIcon(KIcon("text-x-log"));
	actionCollection()->addAction("showLog", aShowLog);
	connect(aShowLog, SIGNAL(triggered(bool)), this, SLOT(showLog()));

	KAction *aDialNumber = new KAction(this);
	aDialNumber->setText(i18n("Dial number"));
	aDialNumber->setIcon(KIcon("internet-telephony"));
	actionCollection()->addAction("dialNumber", aDialNumber);
	connect(aDialNumber, SIGNAL(triggered(bool)), this, SLOT(dialNumber()));

	KAction *aCopyNumber = new KAction(this);
	aCopyNumber->setText(i18n("Copy number to clipboard"));
	aCopyNumber->setIcon(KIcon("edit-copy"));
	actionCollection()->addAction("copyNumber", aCopyNumber);
	connect(aCopyNumber, SIGNAL(triggered(bool)), this, SLOT(copyNumberToClipboard()));

	KAction *aSetDefaultType = new KAction(this);
	aSetDefaultType->setText(i18n("Set as default"));
	aSetDefaultType->setIcon(KIcon("favorites"));
	actionCollection()->addAction("setDefaultType", aSetDefaultType);
	connect(aSetDefaultType, SIGNAL(triggered(bool)), this, SLOT(setDefaultType()));

	//TODO: Set Important

	KAction *aReload = new KAction(this);
	aReload->setText(i18n("Reload"));
	aReload->setIcon(KIcon("view-refresh"));
	actionCollection()->addAction("reload", aReload);
	connect(aReload, SIGNAL(triggered(bool)), this, SLOT(reload()));

	KAction *aReconnectISP = new KAction(this);
	aReconnectISP->setText(i18n("Reconnect to internet"));
	aReconnectISP->setIcon(KIcon("network-workgroup"));
	actionCollection()->addAction("reconnectISP", aReconnectISP);
	connect(aReconnectISP, SIGNAL(triggered(bool)), this, SLOT(reconnectISP()));

	KAction *aGetIP = new KAction(this);
	aGetIP->setText(i18n("Get current IP address"));
	aGetIP->setIcon(KIcon("network-server"));
	actionCollection()->addAction("getIP", aGetIP);
	connect(aGetIP, SIGNAL(triggered(bool)), this, SLOT(getIP()));

	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

//	KStandardAction::find(this, SLOT(find()), actionCollection());
//	KStandardAction::findNext(this, SLOT(findNext()), actionCollection());
//	KStandardAction::findPrev(this, SLOT(findPrev()), actionCollection());
}

void KFritzWindow::initIndicator() {
	QIndicate::Server *iServer = NULL;
	missedCallsIndicator = NULL;
#ifdef INDICATEQT_FOUND
	iServer = QIndicate::Server::defaultInstance();
	iServer->setType("message.irc");
	KService::Ptr service = KService::serviceByDesktopName(appName);
	if (service)
		iServer->setDesktopFile(service->entryPath());
	iServer->show();

	missedCallsIndicator = new QIndicate::Indicator(iServer);
	missedCallsIndicator->setNameProperty(i18n("Missed calls"));
	missedCallsIndicator->setDrawAttentionProperty(true);

	connect(iServer, SIGNAL(serverDisplay()), this, SLOT(showMainWindow()));
	connect(missedCallsIndicator, SIGNAL(display(QIndicate::Indicator*)), this, SLOT(showMissedCalls(QIndicate::Indicator*)));
#endif
}

bool KFritzWindow::queryClose() {
	// don't quit, just minimize to systray
	hide();
	return false;
}

void KFritzWindow::updateMissedCallsIndicator() {
	fritz::CallList *callList = fritz::CallList::getCallList(false);
	size_t missedCallCount = 0;
	if (callList) {
		for (size_t pos = 0; pos < callList->GetSize(fritz::CallEntry::MISSED); pos++)
			if (KSettings::lastKnownMissedCall() < callList->RetrieveEntry(fritz::CallEntry::MISSED, pos)->timestamp)
				missedCallCount++;
	}
#ifdef INDICATEQT_FOUND
	missedCallsIndicator->setCountProperty(missedCallCount);
	if (missedCallCount == 0)
		missedCallsIndicator->hide();
	else
		missedCallsIndicator->show();
#endif
}

void KFritzWindow::showStatusbarBoxBusy(bool b) {
	if (!b)
		setProgressIndicator(i18n("Retrieving data from Fritz!Box..."));
	else
		setProgressIndicator();
}

void KFritzWindow::updateMainWidgets(bool b)
{
	if (!b) {
		if (tabWidget) {
			delete tabWidget;
			treeCallList = NULL;
		}
		tabWidget = new KTabWidget();
		tabWidget->setMovable(true);
		setCentralWidget(tabWidget);
		return;
	}

	// init call list, add to tabWidget
	KCalllistModel *modelCalllist;
	modelCalllist = new KCalllistModel();

	connect(modelCalllist, SIGNAL(updated()), this, SLOT(updateMissedCallsIndicator()));

	treeCallList = new QAdaptTreeView(this);
	treeCallList->setAlternatingRowColors(true);
	treeCallList->setItemsExpandable(false);
	treeCallList->setSortingEnabled(true);
	treeCallList->addAction(actionCollection()->action("dialNumber"));
	treeCallList->addAction(actionCollection()->action("copyNumber"));
	treeCallList->setContextMenuPolicy(Qt::ActionsContextMenu);

	treeCallList->setModel(modelCalllist);
	treeCallList->sortByColumn(1, Qt::DescendingOrder); //sort by Date

	tabWidget->insertTab(0, treeCallList, KIcon("application-x-gnumeric"), 	i18n("Call history"));

	// init fonbooks, add to tabWidget
    fritz::FonbookManager *fm = fritz::FonbookManager::GetFonbookManager();
    std::string first = fm->GetTechId(); //todo: no fb configured
    do {
		KFonbookModel *modelFonbook = new KFonbookModel(fm->GetTechId());

		QAdaptTreeView *treeFonbook = new QAdaptTreeView(this);
		treeFonbook->setAlternatingRowColors(true);
		treeFonbook->setItemsExpandable(true);
		treeFonbook->setSortingEnabled(true);
		treeFonbook->setModel(modelFonbook);
		// TODO: activate editable, depending on writeable flag
		treeFonbook->sortByColumn(0, Qt::AscendingOrder); //sort by Name
		treeFonbook->addAction(actionCollection()->action("dialNumber"));
		treeFonbook->addAction(actionCollection()->action("copyNumber"));
		treeFonbook->addAction(actionCollection()->action("setDefaultType"));
		treeFonbook->setContextMenuPolicy(Qt::ActionsContextMenu);

		tabWidget->insertTab(0, treeFonbook,  KIcon("x-office-address-book"), 	i18n(fm->GetTitle().c_str()));

		fm->NextFonbook();
	} while( first != fm->GetTechId() );
}

void KFritzWindow::find() {
	QStringList findStrings;
	KFindDialog dlg(this, 0, findStrings, true, false);
	if (dlg.exec() != QDialog::Accepted)
		return;

}

void KFritzWindow::findNext() {

}

void KFritzWindow::findPrev() {

}

void KFritzWindow::showMainWindow() {
	this->show();
}

void KFritzWindow::showMissedCalls(QIndicate::Indicator* indicator __attribute__((unused))) {
	tabWidget->setCurrentWidget(treeCallList);
	this->show();
#ifdef INDICATEQT_FOUND
	if (missedCallsIndicator)
		missedCallsIndicator->hide();
#endif
	fritz::CallList *callList = fritz::CallList::getCallList(false);
	KSettings::setLastKnownMissedCall(callList->LastMissedCall());
	KSettings::self()->writeConfig();
	updateMissedCallsIndicator();
}

void KFritzWindow::showLog() {
	logDialog->show();
}

void KFritzWindow::dialNumber() {
	QAdaptTreeView *currentView = static_cast<QAdaptTreeView *>(tabWidget->currentWidget());
	std::string currentNumber;
	if (currentView)
		currentNumber = currentView->currentNumber();
	DialDialog *d = new DialDialog(this, currentNumber);
	d->show();
	// TODO: possible memleak?
}

void KFritzWindow::copyNumberToClipboard() {
	QAdaptTreeView *currentView = static_cast<QAdaptTreeView *>(tabWidget->currentWidget());
	std::string currentNumber;
	if (currentView)
		currentNumber = currentView->currentNumber();
	KApplication::kApplication()->clipboard()->setText(currentNumber.c_str());
}

void KFritzWindow::setDefaultType() {
	QAdaptTreeView *currentView = static_cast<QAdaptTreeView *>(tabWidget->currentWidget());
	KFonbookModel *fonbookModel = static_cast<KFonbookModel *>(currentView->model());
	fonbookModel->setDefaultType(currentView->currentIndex());
}

void KFritzWindow::reload() {
	updateConfiguration();
}

void KFritzWindow::reconnectISP() {
	fritz::FritzClient fc;
	fc.reconnectISP();
	KMessageBox::information(this, i18n("Reconnect successfully initiated."));
}

void KFritzWindow::getIP() {
	fritz::FritzClient fc;
	KMessageBox::information(this, i18n("Current IP is: %1", fc.getCurrentIP().c_str()));
}

void KFritzWindow::setProgressIndicator(QString message) {
	if (!message.length()) {
		if (progressIndicator) {
			statusBar()->removeWidget(progressIndicator);
			delete progressIndicator;
			progressIndicator = NULL;
		}
	}
	else {
		progressIndicator = new QWidget(statusBar());
		new QHBoxLayout(progressIndicator);
		QProgressBar *bar = new QProgressBar(progressIndicator);
		bar->setMaximum(0);
		bar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
		bar->resize(100, 0);
		QLabel *label = new QLabel(message, progressIndicator);
		label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
		progressIndicator->layout()->addWidget(label);
		progressIndicator->layout()->addWidget(bar);
		progressIndicator->layout()->setMargin(0);
		statusBar()->insertPermanentWidget(1, progressIndicator);
	}
}

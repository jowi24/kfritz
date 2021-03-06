/*
 * KFritz
 *
 * Copyright (C) 2010-2012 Joachim Wilke <kfritz@joachim-wilke.de>
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
#include <KSelectAction>
#include <KActionCollection>
#include <KLocale>
#include <KAboutData>
#include <KFilterProxySearchLine>
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

#include "libfritz++/Config.h"
#include "libfritz++/Tools.h"
#include "libfritz++/CallList.h"
#include "libfritz++/FritzClient.h"

#include "ContainerWidget.h"
#include "DialDialog.h"
#include "KCalllistProxyModel.h"
#include "KSettings.h"
#include "KSettingsFonbooks.h"
#include "KSettingsFritzBox.h"
#include "KSettingsMisc.h"
#include "Log.h"
#include "liblog++/Log.h"
#include "libconv++/CharsetConverter.h"
#include "MimeFonbookEntry.h"

KFritzWindow::KFritzWindow()
{
	appName     = KGlobal::mainComponent().aboutData()->appName();
	programName = KGlobal::mainComponent().aboutData()->programName();
	notification = NULL;

	connect(this, SIGNAL(signalNotification(QString, QString, bool)), this, SLOT(slotNotification(QString, QString, bool)));

	dbusIface = new KFritzDbusService(this);

	initIndicator();
	updateMissedCallsIndicator();
	progressIndicator = NULL;

	logDialog = new LogDialog(this);
	KTextEdit *logArea = logDialog->getLogArea();
	logger::Log::setPrefix("kfritz");
    logger::Log::setCustomLogger([logArea](const std::string &msg) { logArea->insertPlainText(msg.c_str()); std::cout << msg; },
                                 [logArea](const std::string &msg) { logArea->insertPlainText(msg.c_str()); std::cout << msg; },
                                 [logArea](const std::string &msg) { logArea->insertPlainText(msg.c_str()); std::cout << msg; });
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

    fbUsername = KSettings::username();
	if (requestPassword)
        savetoWallet = showPasswordDialog(fbUsername, fbPassword, wallet != NULL);

	tabWidget = NULL;

    libFritzInit = new LibFritzInit(fbUsername, fbPassword, this);
    connect(libFritzInit, SIGNAL(ready(bool)),       this, SLOT(showStatusbarBoxBusy(bool)));
	connect(libFritzInit, SIGNAL(invalidPassword()), this, SLOT(reenterPassword()));
	connect(libFritzInit, SIGNAL(ready(bool)),       this, SLOT(updateMainWidgets(bool)));
	libFritzInit->start();

	if (wallet && savetoWallet)
		saveToWallet(wallet);

	setupActions();

	inputCodec  = QTextCodec::codecForName(convert::CharsetConverter::GetDefaultCharset().size() ? convert::CharsetConverter::GetDefaultCharset().c_str() : "UTF-8");

	setupGUI();
	KXmlGuiWindow::stateChanged("NoEdit");
	KXmlGuiWindow::stateChanged("NoFB");


	// remove handbook menu entry
	actionCollection()->action("help_contents")->setVisible(false);
}

KFritzWindow::~KFritzWindow()
{
	// move logging to console
    logger::Log::setCustomLogger([](const std::string &msg) { std::cout << msg; },
                                 [](const std::string &msg) { std::cout << msg; },
                                 [](const std::string &msg) { std::cout << msg; });
	delete dbusIface;
	delete libFritzInit;
}

QString KFritzWindow::toUnicode(const std::string string) const {
	return inputCodec->toUnicode(string.c_str());
}

void KFritzWindow::handleCall(bool outgoing, int connId __attribute__((unused)), std::string remoteNumber, std::string remoteName, fritz::FonbookEntry::eType type, std::string localParty __attribute__((unused)), std::string medium __attribute__((unused)), std::string mediumName)
{
	QString qRemoteName    = toUnicode(remoteName);
	QString qTypeName      = KFonbookModel::getTypeName(type);

	if (qTypeName.size() > 0)
		qRemoteName += " (" + qTypeName + ')';

	//QString qLocalParty = toUnicode(localParty);
	QString qMediumName    = toUnicode(mediumName);
	QString qMessage;
	if (outgoing)
		qMessage=i18n("Outgoing call to <b>%1</b><br/>using %2",   qRemoteName.size() ? qRemoteName : remoteNumber.c_str(),                                    qMediumName);
	else
		qMessage=i18n("Incoming call from <b>%1</b><br/>using %2", qRemoteName.size() ? qRemoteName : remoteNumber.size() ? remoteNumber.c_str() : i18n("unknown"),  qMediumName);

	emit signalNotification(outgoing ? "outgoingCall" : "incomingCall", qMessage, true);
}

void KFritzWindow::handleConnect(int connId __attribute__((unused)))
{
	if (notification)
		notification->close();
	emit signalNotification("callConnected", i18n("Call connected."), false);
}

void KFritzWindow::handleDisconnect(int connId __attribute__((unused)), std::string duration)
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
					                    "internet-telephony"); //krazy:exclude=spelling
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

	QWidget *frameMisc     = new KSettingsMisc(this);
	confDialog->addPage(frameMisc,     i18n("Other"), "preferences-other", i18n("Configure other settings"));

	connect(confDialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(updateConfiguration(const QString &)));

	confDialog->show();
}

void KFritzWindow::showNotificationSettings() {
    KNotifyConfigWidget::configure(this);
}

void KFritzWindow::updateConfiguration(const QString &dialogName __attribute__((unused))) {
	// stop any pending initialization
    libFritzInit->terminate();
    showStatusbarBoxBusy(true);
    // clean up before changing the configuration
	fritz::Config::Shutdown();

    libFritzInit->setCredentials(fbUsername, fbPassword);
	libFritzInit->start();
}

void KFritzWindow::reenterPassword() {
	KWallet::Wallet *wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0);

    bool keepPassword = showPasswordDialog(fbUsername, fbPassword, wallet != NULL);

	if (wallet && keepPassword)
		saveToWallet(wallet);
	updateConfiguration();
}

void KFritzWindow::saveToWallet(KWallet::Wallet *wallet) {
    KSettings::setUsername(fbUsername);
    KSettings::self()->writeConfig();

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

bool KFritzWindow::showPasswordDialog(QString &username, QString &password, bool offerSaving) {
    KPasswordDialog pwd(this, (offerSaving ? KPasswordDialog::ShowKeepPassword : KPasswordDialog::NoFlags) | KPasswordDialog::ShowUsernameLine);
    pwd.setUsername(fbUsername);
    pwd.setPassword(fbPassword);
    pwd.setPrompt(i18n("Enter your Fritz!Box credentials. Leave username empty if you do not have one."));
	pwd.exec();
	password = pwd.password();
    username = pwd.username();
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
	aDialNumber->setIcon(KIcon("internet-telephony")); //krazy:exclude=spelling
	actionCollection()->addAction("dialNumber", aDialNumber);
	connect(aDialNumber, SIGNAL(triggered(bool)), this, SLOT(dialNumber()));

	KAction *aCopyNumber = new KAction(this);
	aCopyNumber->setText(i18n("Copy number to clipboard"));
	aCopyNumber->setIcon(KIcon("edit-copy"));
	actionCollection()->addAction("copyNumber", aCopyNumber);
	connect(aCopyNumber, SIGNAL(triggered(bool)), this, SLOT(copyNumberToClipboard()));

	KAction *aSetDefault = new KAction(this);
	aSetDefault->setText(i18n("Set as default"));
	aSetDefault->setIcon(KIcon("favorites"));
	actionCollection()->addAction("setDefault", aSetDefault);
	connect(aSetDefault, SIGNAL(triggered(bool)), this, SLOT(setDefault()));

	KSelectAction *aSetType = new KSelectAction(this);
	aSetType->setText(i18n("Set type"));
	// WARNING: mapIndexToType() has to be checked if code changes here:
	for (size_t p = fritz::FonbookEntry::TYPE_HOME; p < fritz::FonbookEntry::TYPES_COUNT; p++) {
		aSetType->addAction(KFonbookModel::getTypeName((fritz::FonbookEntry::eType) p));
	}
	actionCollection()->addAction("setType", aSetType);
	connect(aSetType, SIGNAL(triggered(int)), this, SLOT(setType(int)));

	//TODO: Set Important

	KAction *aReload = new KAction(this);
	aReload->setText(i18n("Reload"));
	aReload->setIcon(KIcon("view-refresh"));
	actionCollection()->addAction("reload", aReload);
	connect(aReload, SIGNAL(triggered(bool)), this, SLOT(reload()));

	KAction *aReconnectISP = new KAction(this);
	aReconnectISP->setText(i18n("Reconnect to the Internet"));
	aReconnectISP->setIcon(KIcon("network-workgroup"));
	actionCollection()->addAction("reconnectISP", aReconnectISP);
	connect(aReconnectISP, SIGNAL(triggered(bool)), this, SLOT(reconnectISP()));

	KAction *aGetIP = new KAction(this);
	aGetIP->setText(i18n("Get current IP address"));
	aGetIP->setIcon(KIcon("network-server"));
	actionCollection()->addAction("getIP", aGetIP);
	connect(aGetIP, SIGNAL(triggered(bool)), this, SLOT(getIP()));

	KAction *aAddEntry = new KAction(this);
	aAddEntry->setText(i18n("Add"));
	aAddEntry->setIcon(KIcon("list-add"));
	aAddEntry->setShortcut(Qt::CTRL + Qt::Key_N);
	actionCollection()->addAction("addEntry", aAddEntry);
	connect(aAddEntry, SIGNAL(triggered(bool)), this, SLOT(addEntry()));

	KAction *aDeleteEntry = new KAction(this);
	aDeleteEntry->setText(i18n("Delete"));
	aDeleteEntry->setIcon(KIcon("list-remove"));
	aDeleteEntry->setShortcut(Qt::Key_Delete);
	actionCollection()->addAction("deleteEntry", aDeleteEntry);
	connect(aDeleteEntry, SIGNAL(triggered(bool)), this, SLOT(deleteEntry()));

	KAction *aResolveNumber = new KAction(this);
	aResolveNumber->setText(i18n("Look up number in phone books"));
	actionCollection()->addAction("resolveNumber", aResolveNumber);
	connect(aResolveNumber, SIGNAL(triggered(bool)), this, SLOT(resolveNumber()));

	KAction *aSeparator;
	aSeparator = new KAction(this);
	aSeparator->setSeparator(true);
	actionCollection()->addAction("separator1", aSeparator);
	aSeparator = new KAction(this);
	aSeparator->setSeparator(true);
	actionCollection()->addAction("separator2", aSeparator);

	KStandardAction::save(this, SLOT(save()), actionCollection());
	KStandardAction::cut(this, SLOT(cutEntry()), actionCollection());
	KStandardAction::copy(this, SLOT(copyEntry()), actionCollection());
	KStandardAction::paste(this, SLOT(pasteEntry()), actionCollection());
	KStandardAction::quit(this, SLOT(quit()), actionCollection());
}

void KFritzWindow::initIndicator() {
	missedCallsIndicator = NULL;
#ifdef INDICATEQT_FOUND
	QIndicate::Server *iServer = NULL;
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
	fritz::CallList *callList = fritz::CallList::GetCallList(false);
	if (!callList)
		return;
#ifdef INDICATEQT_FOUND
	size_t missedCallCount = callList->missedCalls(KSettings::lastKnownMissedCall());
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
	treeCallList->addAction(actionCollection()->action("edit_copy"));
	treeCallList->addAction(actionCollection()->action("separator1"));
	treeCallList->addAction(actionCollection()->action("dialNumber"));
	treeCallList->addAction(actionCollection()->action("copyNumber"));
	treeCallList->addAction(actionCollection()->action("resolveNumber"));
	treeCallList->setContextMenuPolicy(Qt::ActionsContextMenu);
	treeCallList->sortByColumn(1, Qt::DescendingOrder); //sort by Date

	// init proxy class for filtering
	KCalllistProxyModel *proxyModelCalllist = new KCalllistProxyModel(this);
	proxyModelCalllist->setSourceModel(modelCalllist);
	treeCallList->setModel(proxyModelCalllist);

	// search line widget
	KFilterProxySearchLine *search = new KFilterProxySearchLine(this);
	search->setProxy(proxyModelCalllist);

	// setup final calllist widget
	ContainerWidget *calllistContainer = new ContainerWidget(this, treeCallList, proxyModelCalllist);
	new QVBoxLayout(calllistContainer);
	calllistContainer->layout()->addWidget(search);
	calllistContainer->layout()->addWidget(treeCallList);
	tabWidget->insertTab(0, calllistContainer, KIcon("application-x-gnumeric"), 	i18n("Call history"));

	// init fonbooks, add to tabWidget
    fritz::FonbookManager *fm = fritz::FonbookManager::GetFonbookManager();
    std::string first = fm->getTechId();
    if (first.length()) {
    	do {
    		KFonbookModel *modelFonbook = new KFonbookModel(fm->getTechId());
    		connect(modelFonbook, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateFonbookState()));

    		QAdaptTreeView *treeFonbook = new QAdaptTreeView(this);
    		treeFonbook->setAlternatingRowColors(true);
    		treeFonbook->setItemsExpandable(true);
    		treeFonbook->setSortingEnabled(true);
    		treeFonbook->setModel(modelFonbook);
    		treeFonbook->sortByColumn(0, Qt::AscendingOrder); //sort by Name
    		treeFonbook->addAction(actionCollection()->action("edit_cut"));
    		treeFonbook->addAction(actionCollection()->action("edit_copy"));
    		treeFonbook->addAction(actionCollection()->action("edit_paste"));
    		treeFonbook->addAction(actionCollection()->action("separator1"));
    		treeFonbook->addAction(actionCollection()->action("addEntry"));
    		treeFonbook->addAction(actionCollection()->action("deleteEntry"));
    		treeFonbook->addAction(actionCollection()->action("separator2"));
    		treeFonbook->addAction(actionCollection()->action("dialNumber"));
    		treeFonbook->addAction(actionCollection()->action("copyNumber"));
    		treeFonbook->addAction(actionCollection()->action("setDefault"));
    		treeFonbook->addAction(actionCollection()->action("setType"));
    		treeFonbook->setContextMenuPolicy(Qt::ActionsContextMenu);
    		connect(treeFonbook->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(updateFonbookContextMenu(const QModelIndex&, const QModelIndex&)));
    		//TODO: also reaction on data changes (dataChanged on model)

    		//TODO: TAB-order by row (not by column) reimplement moveCursor( )

    		ContainerWidget *fonbookContainer = new ContainerWidget(this, treeFonbook, modelFonbook);
    		new QVBoxLayout(fonbookContainer);
    		fonbookContainer->layout()->addWidget(treeFonbook);
    		tabWidget->insertTab(0, fonbookContainer,  KIcon("x-office-address-book"), 	i18n(fm->getTitle().c_str()));

    		fm->nextFonbook();
    	} while( first != fm->getTechId() );
    }
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateActionProperties(int)));
    connect(tabWidget, SIGNAL(currentChanged(int)), statusBar(), SLOT(clearMessage()));
    updateActionProperties(tabWidget->currentIndex());
    connect(treeCallList->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(updateCallListContextMenu(const QModelIndex&, const QModelIndex&)));
}

void KFritzWindow::save() {
	ContainerWidget *container = static_cast<ContainerWidget *>(tabWidget->currentWidget());

	if (container->isFonbook()) {
		std::string techid = container->getFonbookModel()->getFonbook()->getTechId();
		fritz::FonbookManager *fm = fritz::FonbookManager::GetFonbookManager();
		fritz::Fonbooks *fbs = fm->getFonbooks();
		fritz::Fonbook  *fb  = (*fbs)[techid];
		fb->save();
		statusBar()->showMessage(i18n("%1 saved.", toUnicode(fb->getTitle())), 0);
		updateFonbookState();
	}
}

//TODO: quit using systray
void KFritzWindow::quit() {
	// check for pending changes
	fritz::FonbookManager *fm = fritz::FonbookManager::GetFonbookManager();
	std::string first = fm->getTechId();
	if (first.length()) {
		do {
			if (fm->isModified()) {
				switch (KMessageBox::warningYesNoCancel( this, i18n("Save changes to %1?", fm->getTitle().c_str()))) {
				case KMessageBox::Yes :
					fm->save();
					break;
				case KMessageBox::No :
					break;
				default: // cancel
					return;
				}
			}
			fm->nextFonbook();
		} while (first != fm->getTechId());
	}

	QApplication::quit();
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
	fritz::CallList *callList = fritz::CallList::GetCallList(false);
	KSettings::setLastKnownMissedCall(callList->getLastMissedCall());
	KSettings::self()->writeConfig();
	updateMissedCallsIndicator();
}

void KFritzWindow::showLog() {
	logDialog->show();
}

std::string KFritzWindow::getCurrentNumber() {
	ContainerWidget *container = static_cast<ContainerWidget *>(tabWidget->currentWidget());
	QAdaptTreeView *treeView = container->getTreeView();
	if (container->isFonbook()) {
		return container->getFonbookModel()->number(treeView->currentIndex());
	} else if (container->isCalllist()) {
		return container->getCalllistModel()->number(treeView->currentIndex());
	}
	return "";
}

void KFritzWindow::dialNumber() {
	DialDialog d(this, getCurrentNumber());
	d.exec();
}

void KFritzWindow::copyNumberToClipboard() {
	KApplication::kApplication()->clipboard()->setText(getCurrentNumber().c_str());
}

void KFritzWindow::setDefault() {
	ContainerWidget *container = static_cast<ContainerWidget *>(tabWidget->currentWidget());
	QAdaptTreeView *treeView = container->getTreeView();
	if (container->isFonbook()) {
		container->getFonbookModel()->setDefault(treeView->currentIndex());
	}
}

fritz::FonbookEntry::eType KFritzWindow::mapIndexToType(int index) {
	return static_cast<fritz::FonbookEntry::eType>(index+1);
}

void KFritzWindow::setType(int index) {
	ContainerWidget *container = static_cast<ContainerWidget *>(tabWidget->currentWidget());
	QAdaptTreeView *treeView = container->getTreeView();
	if (container->isFonbook()) {
		container->getFonbookModel()->setType(treeView->currentIndex(), mapIndexToType(index));
	}
}

void KFritzWindow::reload() {
	updateConfiguration();  //TODO: pending changes are lost without warning
}

void KFritzWindow::reconnectISP() {
	fritz::FritzClient *fc = fritz::gConfig->fritzClientFactory->create();
	if (fc->reconnectISP())
		KMessageBox::information(this, i18n("Reconnect initiated."));
	else
		KMessageBox::error(this, i18n("Reconnect failed."));
	delete fc;
}

void KFritzWindow::getIP() {
	fritz::FritzClient *fc = fritz::gConfig->fritzClientFactory->create();;
	std::string ip = fc->getCurrentIP();
	delete fc;
	KMessageBox::information(this, i18n("Current IP address is: %1", ip.size() ? ip.c_str() : i18n("unknown")));
}

void KFritzWindow::addEntry(fritz::FonbookEntry *fe) {
	ContainerWidget *container = static_cast<ContainerWidget *>(tabWidget->currentWidget());
	QAdaptTreeView *treeView = container->getTreeView();
	if (container->isFonbook()) {
		KFonbookModel *model = container->getFonbookModel();
		if (fe)
			model->insertFonbookEntry(treeView->currentIndex(), *fe);
		else
			model->insertRows(container->getTreeView()->currentIndex().row(), 1, QModelIndex());
		treeView->scrollTo(container->getTreeView()->currentIndex());
		treeView->setCurrentIndex(model->index(treeView->currentIndex().row()-1,0,QModelIndex()));
	}
}

void KFritzWindow::deleteEntry() {
	ContainerWidget *container = static_cast<ContainerWidget *>(tabWidget->currentWidget());
	QAdaptTreeView *treeView = container->getTreeView();
	if (container->isFonbook()) {
		KFonbookModel *model = container->getFonbookModel();
		size_t newRow = treeView->currentIndex().row();
		if (treeView->currentIndex().row() == model->rowCount()-1)
			newRow--;
		QModelIndex index = model->index(newRow, treeView->currentIndex().column());
		model->removeRows(treeView->currentIndex().row(), 1, QModelIndex());
		treeView->setCurrentIndex(index);
	}
}

void KFritzWindow::cutEntry() {
	copyEntry();
	deleteEntry();
}

void KFritzWindow::copyEntry() {
	ContainerWidget *container = static_cast<ContainerWidget *>(tabWidget->currentWidget());
	QAdaptTreeView *treeView = container->getTreeView();

	if (container->isFonbook()) {
		KFonbookModel *model = container->getFonbookModel();
		const fritz::FonbookEntry *fe = model->retrieveFonbookEntry(treeView->currentIndex());
		if (fe) {
			QMimeData* mimeData = new MimeFonbookEntry(*fe);
			QApplication::clipboard()->setMimeData(mimeData);
		}
	}
	if (container->isCalllist()) {
		KCalllistProxyModel *model = container->getCalllistModel();
		const fritz::CallEntry *ce = model->retrieveCallEntry(treeView->currentIndex());
		if (ce) {
			fritz::FonbookEntry fe(ce->remoteName);
			fe.addNumber(ce->remoteNumber, fritz::FonbookEntry::TYPE_NONE);
			QMimeData* mimeData = new MimeFonbookEntry(fe);
			QApplication::clipboard()->setMimeData(mimeData);
		}
	}
}

void KFritzWindow::pasteEntry() {
	const QMimeData *mime = QApplication::clipboard()->mimeData();
	if (mime) {
		const MimeFonbookEntry *mimeFonbookEntry = qobject_cast<const MimeFonbookEntry *>(mime);
		if (mimeFonbookEntry) {
			fritz::FonbookEntry *fe = mimeFonbookEntry->retrieveFonbookEntry();
			if (fe)
				addEntry(fe);
		}
	}
}

void KFritzWindow::resolveNumber() {
	ContainerWidget *container = static_cast<ContainerWidget *>(tabWidget->currentWidget());
	if (container->isCalllist()) {
		std::string currentNumber = getCurrentNumber();
		fritz::Fonbook::sResolveResult result = fritz::FonbookManager::GetFonbook()->resolveToName(currentNumber);
		if (!result.name.compare(currentNumber)) {
			statusBar()->showMessage(i18n("%1 did not resolve.", toUnicode(currentNumber)), 0);
		} else {
			KCalllistProxyModel *model = container->getCalllistModel();
			for (int pos = 0; pos < model->rowCount(QModelIndex()); pos++) {
				fritz::CallEntry *entry = model->retrieveCallEntry(model->index(pos, 0, QModelIndex()));
				if (entry->matchesRemoteNumber(currentNumber))
					entry->remoteName = result.name;
			}
			statusBar()->showMessage(i18n("%1 resolves to \"%2\".", toUnicode(currentNumber), toUnicode(result.name), 0));
		}
	}
}

void KFritzWindow::updateActionProperties(int tabIndex __attribute__((unused))) {
	KXmlGuiWindow::stateChanged("NoEdit");
	ContainerWidget *container = static_cast<ContainerWidget *>(tabWidget->currentWidget());
	if (container->isFonbook()) {
		if (container->getFonbookModel()->flags(QModelIndex()) & QFlag(Qt::ItemIsEditable))
			KXmlGuiWindow::stateChanged("WriteableFB");
		if (container->getFonbookModel()->getFonbook()->isModified())
			KXmlGuiWindow::stateChanged("DirtyFB");
		else
			KXmlGuiWindow::stateChanged("CleanFB");
	} else {
		KXmlGuiWindow::stateChanged("NoFB");
	}

}

void KFritzWindow::updateCallListContextMenu(const QModelIndex &current, const QModelIndex &previous __attribute__((unused))) {
	ContainerWidget *container = static_cast<ContainerWidget *>(tabWidget->currentWidget());
	if (container->isCalllist()) {
		KCalllistProxyModel *model = container->getCalllistModel();
		bool canResolve = (model->name(current).compare(model->number(current)) == 0);
		actionCollection()->action("resolveNumber")->setEnabled(canResolve);
	}
}

void KFritzWindow::updateFonbookContextMenu(const QModelIndex &current, const QModelIndex &previous __attribute__((unused))) {
	ContainerWidget *container = static_cast<ContainerWidget *>(tabWidget->currentWidget());
	if (container->isFonbook()) {
		KFonbookModel *model = container->getFonbookModel();
		KSelectAction *setTypeAction = static_cast<KSelectAction *>(actionCollection()->action("setType"));
		if (current.column() > 0 && current.column() <= 3) { //XXX((int) fritz::FonbookEntry::MAX_NUMBERS)) {
			const fritz::FonbookEntry *entry = model->retrieveFonbookEntry(current);
			fritz::FonbookEntry::eType type = entry->getType(current.column()-1);
			if (entry->getNumber(current.column()-1).size()) {
				setTypeAction->setCurrentItem(type-1);
				setTypeAction->setEnabled(true);
			} else {
				setTypeAction->setEnabled(false);
			}
		} else {
			setTypeAction->setEnabled(false);
		}
		KSelectAction *copyNumberAction = static_cast<KSelectAction *>(actionCollection()->action("copyNumber"));
		if (model->retrieveFonbookEntry(current)) {
			copyNumberAction->setEnabled(true);
		} else {
			copyNumberAction->setEnabled(false);
		}
	}
}

void KFritzWindow::updateFonbookState() {
	ContainerWidget *container = static_cast<ContainerWidget *>(tabWidget->currentWidget());
	if (container->isFonbook()) {
		if (container->getFonbookModel()->getFonbook()->isModified())
			KXmlGuiWindow::stateChanged("DirtyFB");
		else
			KXmlGuiWindow::stateChanged("CleanFB");
	} else {
		KXmlGuiWindow::stateChanged("NoFB");
	}
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
		statusBar()->insertPermanentWidget(0, progressIndicator);
	}
}


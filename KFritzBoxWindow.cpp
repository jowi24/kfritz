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

#include "KFritzBoxWindow.h"

#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KLocale>
#include <KApplication>
#include <KAction>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <KConfigSkeleton>
#include <KConfigDialog>
#include <KNotifyConfigWidget>
#include <KPasswordDialog>
#include <QTextCodec>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <Config.h>


#include "KSettings.h"
#include "ui_KSettingsFritzBox.h"
#include "Log.h"


KFritzBoxWindow::KFritzBoxWindow()
{
	KTextEdit *logArea = new KTextEdit(this);
	fritz::Config::SetupLogging(LogStream::getLogStream(LogBuf::DEBUG)->setLogWidget(logArea),
							    LogStream::getLogStream(LogBuf::INFO)->setLogWidget(logArea),
					            LogStream::getLogStream(LogBuf::ERROR)->setLogWidget(logArea));

	bool savetoWallet = false;
	bool requestPassword = true;
	QString appName = KGlobal::mainComponent().aboutData()->appName();

	KWallet::Wallet *wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0);
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

	libFritzInit = new LibFritzInit(fbPassword);

	connect(libFritzInit, SIGNAL(invalidPassword()), this, SLOT(reenterPassword()));

	if (wallet && savetoWallet)
		saveToWallet(wallet);

	modelFonbook  = new KFonbookModel();
	connect(libFritzInit, SIGNAL(ready(bool)), modelFonbook, SLOT(libReady(bool)));
	modelCalllist = new KCalllistModel();
	connect(libFritzInit, SIGNAL(ready(bool)), modelCalllist, SLOT(libReady(bool)));

	setupActions();

	treeFonbook = new QAdaptTreeView(this);
	treeFonbook->setAlternatingRowColors(true);
	treeFonbook->setItemsExpandable(false);
	treeFonbook->setSortingEnabled(true);

	treeFonbook->setModel(modelFonbook);
	treeFonbook->sortByColumn(0, Qt::AscendingOrder); //sort by Name

	treeCallList = new QAdaptTreeView(this);
	treeCallList->setAlternatingRowColors(true);
	treeCallList->setItemsExpandable(false);
	treeCallList->setSortingEnabled(true);

	treeCallList->setModel(modelCalllist);
	treeCallList->sortByColumn(1, Qt::DescendingOrder); //sort by Date

	logArea->setReadOnly(true);

	tabWidget = new KTabWidget();
	tabWidget->addTab(treeFonbook,  KIcon("x-office-address-book"), 	i18n("Fonbook"));
	tabWidget->addTab(treeCallList, KIcon("application-x-gnumeric"), 	i18n("Calllist"));
	// this tab has to be removed in the destructor, because of ...?
	tabWidget->addTab(logArea, 		KIcon("text-rtf"), 					i18n("Log"));
	setCentralWidget(tabWidget);

	setupGUI();
}

KFritzBoxWindow::~KFritzBoxWindow()
{
	// move logging to console
	fritz::Config::SetupLogging(&std::clog, &std::cout, &std::cerr);
	tabWidget->removeTab(2);

	fritz::FonbookManager::DeleteFonbookManager();
	fritz::CallList::DeleteCallList();

	delete libFritzInit;
}

void KFritzBoxWindow::showSettings(bool b __attribute__((unused))) {
	KConfigDialog *confDialog = new KConfigDialog(this, "settings", KSettings::self());

	QWidget *frameFritzBox = new QWidget( this );
    Ui_KSettingsFritzBox uiFritzBox;
    uiFritzBox.setupUi(frameFritzBox);
    uiFritzBox.kcfg_Password->setText(fbPassword);
    connect(uiFritzBox.kcfg_Password, SIGNAL(textChanged(const QString &)), confDialog, SLOT(updateButtons()));
	confDialog->addPage(frameFritzBox, "Fritz!Box", "modem", "Configure connection to Fritz!Box");

	confDialog->addPage(new QWidget(this), "Phone books", "x-office-address-book", "Select phone books to use" );

	connect(confDialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(updateConfiguration(const QString &)));

	confDialog->show();
}

void KFritzBoxWindow::showNotificationSettings(bool b __attribute__((unused))) {
    KNotifyConfigWidget::configure(this);
}

void KFritzBoxWindow::updateConfiguration(const QString &dialogName __attribute__((unused))) {
	libFritzInit->setPassword(fbPassword);
	libFritzInit->start();
}

void KFritzBoxWindow::reenterPassword() {
	KWallet::Wallet *wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0);

	bool keepPassword = showPasswordDialog(fbPassword, wallet != NULL);

	if (wallet && keepPassword)
		saveToWallet(wallet);
	updateConfiguration();
}

void KFritzBoxWindow::saveToWallet(KWallet::Wallet *wallet) {
	QString appName = KGlobal::mainComponent().aboutData()->appName();

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

bool KFritzBoxWindow::showPasswordDialog(QString &password, bool offerSaving) {
	KPasswordDialog pwd(NULL, offerSaving ? KPasswordDialog::ShowKeepPassword : KPasswordDialog::NoFlags); //TODO parent
	pwd.setPrompt(i18n("Enter your Fritz!Box password"));
	pwd.exec();
	password = pwd.password();
	return pwd.keepPassword();
}

void KFritzBoxWindow::setupActions() {
	KAction* aShowSettings = new KAction(this);
	aShowSettings->setText(i18n("Configure KFritz..."));
	aShowSettings->setIcon(KIcon("preferences-other"));
	actionCollection()->addAction("showSettings", aShowSettings);
	connect(aShowSettings, SIGNAL(triggered(bool)), this, SLOT(showSettings(bool)));

	KAction* aShowNotifySettings = new KAction(this);
	aShowNotifySettings->setText(i18n("Configure Notifications..."));
	aShowNotifySettings->setIcon(KIcon("preferences-desktop-notification"));
	actionCollection()->addAction("showNotifySettings", aShowNotifySettings);
	connect(aShowNotifySettings, SIGNAL(triggered(bool)), this, SLOT(showNotificationSettings(bool)));

	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
}

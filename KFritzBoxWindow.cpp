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
	fritz::Config::SetupLogging(new LogStream(LogBuf::DEBUG, logArea),
			                    new LogStream(LogBuf::INFO, logArea),
			                    new LogStream(LogBuf::ERROR, logArea));

	// init libfritz++ in background
	libFritzInit = new LibFritzInit();

	modelFonbook  = new KFonbookModel();
	//connect(modelFonbook, SIGNAL(modelReset()), SLOT(modelFonbookReset()));
	connect(libFritzInit, SIGNAL(ready(bool)), modelFonbook, SLOT(libReady(bool)));
	modelCalllist = new KCalllistModel();
	//connect(modelCalllist, SIGNAL(modelReset()), SLOT(modelCalllistReset()));
	connect(libFritzInit, SIGNAL(ready(bool)), modelCalllist, SLOT(libReady(bool)));

	KAction* aShowSettings = new KAction(this);
	aShowSettings->setText(i18n("Configure KFritz"));
	aShowSettings->setIcon(KIcon("preferences-other"));
	actionCollection()->addAction("showSettings", aShowSettings);
	connect(aShowSettings, SIGNAL(triggered(bool)), this, SLOT(showSettings(bool)));

	KAction* aShowNotifySettings = new KAction(this);
	aShowNotifySettings->setText(i18n("Configure Notifications"));
	aShowNotifySettings->setIcon(KIcon("preferences-desktop-notification"));
	actionCollection()->addAction("showNotifySettings", aShowNotifySettings);
	connect(aShowNotifySettings, SIGNAL(triggered(bool)), this, SLOT(showNotificationSettings(bool)));


	//	clearAction->setShortcut(Qt::CTRL + Qt::Key_W);

	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

	treeFonbook = new QAdaptTreeView(this);
	treeFonbook->setAlternatingRowColors(true);
	treeFonbook->setItemsExpandable(false);
	treeFonbook->setRootIsDecorated(false);
	treeFonbook->setSortingEnabled(true);
	treeFonbook->setUniformRowHeights(true);
	treeFonbook->setAcceptDrops(false);
	treeFonbook->setDragEnabled(false);

	treeFonbook->setModel(modelFonbook);
	treeFonbook->sortByColumn(0, Qt::AscendingOrder); //sort by Name

	treeCallList = new QAdaptTreeView(this);
	treeCallList->setAlternatingRowColors(true);
	treeCallList->setItemsExpandable(false);
	treeCallList->setRootIsDecorated(false);
	treeCallList->setSortingEnabled(true);
	treeCallList->setUniformRowHeights(true);
	treeCallList->setAcceptDrops(false);
	treeCallList->setDragEnabled(false);

	treeCallList->setModel(modelCalllist);
	treeCallList->sortByColumn(1, Qt::DescendingOrder); //sort by Date

	logArea->setReadOnly(true);

	tabWidget = new KTabWidget();
	setCentralWidget(tabWidget);
	tabWidget->addTab(treeFonbook,  KIcon("x-office-address-book"), 	i18n("Fonbook"));
	tabWidget->addTab(treeCallList, KIcon("application-x-gnumeric"), 	i18n("Calllist"));
	// this tab has to be removed in the destructor, because of logArea being an 'external' object
	tabWidget->addTab(logArea, 		KIcon("text-rtf"), 					i18n("Log"));

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
	frameFritzBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    Ui_KSettingsFritzBox uiFritzBox;
    uiFritzBox.setupUi(frameFritzBox);

	confDialog->addPage(frameFritzBox, "Fritz!Box", "modem", "Configure connection to Fritz!Box");
	confDialog->addPage(new QWidget(this), "Phone books", "x-office-address-book", "Select phone books to use" );

	connect(confDialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(updateConfiguration(const QString &)));

	confDialog->show();
}

void KFritzBoxWindow::showNotificationSettings(bool b __attribute__((unused))) {
    KNotifyConfigWidget::configure(this);
}

void KFritzBoxWindow::updateConfiguration(const QString &dialogName __attribute__((unused))){
	libFritzInit->start();
}


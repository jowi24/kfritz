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

#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KLocale>
#include <KApplication>
#include <KAction>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <QTextCodec>

#include <FonbookManager.h>
#include <Config.h>
#include "KFritzBoxWindow.h"
#include "Log.h"

KFritzBoxWindow::KFritzBoxWindow()
{
//	logArea = new KTextEdit();

//	fritz::Config::SetupLogging(new LogStream(LogBuf::DEBUG, logArea),
//                    			new LogStream(LogBuf::INFO,  logArea),
//	                      		new LogStream(LogBuf::ERROR, logArea));

	std::vector<std::string> vFonbook;
	vFonbook.push_back("FRITZ");
	vFonbook.push_back("OERT");
	fritz::FonbookManager::CreateFonbookManager(vFonbook, "FRITZ");

	fritz::CallList::CreateCallList();

	tree = new QTreeView(this);
	tree->setAlternatingRowColors(true);
	tree->setItemsExpandable(false);
	tree->setRootIsDecorated(false);
	tree->setSortingEnabled(true);
	tree->setUniformRowHeights(true);
	tree->setAcceptDrops(false);
	tree->setDragEnabled(false);
	//tree->setSizePolicy()
	setCentralWidget(tree);

	modelFonbook  = new KFonbookModel();
	modelCalllist = new KCalllistModel();

	KAction* aShowLog = new KAction(this);
	aShowLog->setText(i18n("Logfile"));
	aShowLog->setIcon(KIcon("text-rtf"));
	actionCollection()->addAction("showLog", aShowLog);
	connect(aShowLog, SIGNAL(triggered(bool)), this, SLOT(showLog(bool)));

	KAction* aShowFonbook = new KAction(this);
	aShowFonbook->setText(i18n("Fonbook"));
	aShowFonbook->setIcon(KIcon("x-office-address-book"));
	actionCollection()->addAction("showFonbook", aShowFonbook);
	connect(aShowFonbook, SIGNAL(triggered(bool)), this, SLOT(showFonbook(bool)));

	KAction* aShowCalllist = new KAction(this);
	aShowCalllist->setText(i18n("Calllist"));
	aShowCalllist->setIcon(KIcon("application-x-gnumeric"));
	actionCollection()->addAction("showCalllist", aShowCalllist);
	connect(aShowCalllist, SIGNAL(triggered(bool)), this, SLOT(showCalllist(bool)));

//	clearAction->setShortcut(Qt::CTRL + Qt::Key_W);
	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

	setupGUI();
}

void KFritzBoxWindow::showFonbook(bool b) {
	tree->setModel(modelFonbook);
}

void KFritzBoxWindow::showCalllist(bool b) {
	tree->setModel(modelCalllist);
}

void KFritzBoxWindow::showLog(bool b) {
//	setCentralWidget(logArea);
}

KFritzBoxWindow::~KFritzBoxWindow()
{
	fritz::FonbookManager::DeleteFonbookManager();
	fritz::CallList::DeleteCallList();
}

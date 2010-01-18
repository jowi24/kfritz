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
#include <KTabWidget>
#include <QTextCodec>

#include <FonbookManager.h>
#include <Config.h>
#include "KFritzBoxWindow.h"
#include "Log.h"

KFritzBoxWindow::KFritzBoxWindow(KTextEdit *logArea)
{
	this->logArea = logArea;

	std::vector<std::string> vFonbook;
	vFonbook.push_back("FRITZ");
	vFonbook.push_back("OERT");
	fritz::FonbookManager::CreateFonbookManager(vFonbook, "FRITZ");

	fritz::CallList::CreateCallList();

	modelFonbook  = new KFonbookModel();
	connect(modelFonbook, SIGNAL(modelReset()), SLOT(modelFonbookReset()));
	modelCalllist = new KCalllistModel();
	connect(modelCalllist, SIGNAL(modelReset()), SLOT(modelCalllistReset()));

//	KAction* aShowLog = new KAction(this);
//	aShowLog->setText(i18n("Logfile"));
//	aShowLog->setIcon(KIcon("text-rtf"));
//	actionCollection()->addAction("showLog", aShowLog);
//	connect(aShowLog, SIGNAL(triggered(bool)), this, SLOT(showLog(bool)));
//
//	KAction* aShowFonbook = new KAction(this);
//	aShowFonbook->setText(i18n("Fonbook"));
//	aShowFonbook->setIcon(KIcon("x-office-address-book"));
//	actionCollection()->addAction("showFonbook", aShowFonbook);
//	connect(aShowFonbook, SIGNAL(triggered(bool)), this, SLOT(showFonbook(bool)));
//
//	KAction* aShowCalllist = new KAction(this);
//	aShowCalllist->setText(i18n("Calllist"));
//	aShowCalllist->setIcon(KIcon("application-x-gnumeric"));
//	actionCollection()->addAction("showCalllist", aShowCalllist);
//	connect(aShowCalllist, SIGNAL(triggered(bool)), this, SLOT(showCalllist(bool)));

//	clearAction->setShortcut(Qt::CTRL + Qt::Key_W);
	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

	treeFonbook = new QTreeView(this);
	treeFonbook->setAlternatingRowColors(true);
	treeFonbook->setItemsExpandable(false);
	treeFonbook->setRootIsDecorated(false);
	treeFonbook->setSortingEnabled(true);
	treeFonbook->setUniformRowHeights(true);
	treeFonbook->setAcceptDrops(false);
	treeFonbook->setDragEnabled(false);

	treeFonbook->setModel(modelFonbook);
	treeFonbook->sortByColumn(0, Qt::AscendingOrder); //sort by Name

	treeCallList = new QTreeView(this);
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

	KTabWidget *w = new KTabWidget();
	setCentralWidget(w);
	w->addTab(treeFonbook,  KIcon("x-office-address-book"), 	i18n("Fonbook"));
	w->addTab(treeCallList, KIcon("application-x-gnumeric"), 	i18n("Calllist"));
	w->addTab(logArea, 		KIcon("text-rtf"), 					i18n("Log"));

	setupGUI();
}

//void KFritzBoxWindow::showFonbook(bool b __attribute__((unused))) {
//}
//
//void KFritzBoxWindow::showCalllist(bool b __attribute__((unused))) {
//}
//
//void KFritzBoxWindow::showLog(bool b __attribute__((unused))) {
//}

KFritzBoxWindow::~KFritzBoxWindow()
{
	fritz::FonbookManager::DeleteFonbookManager();
	fritz::CallList::DeleteCallList();
}

void KFritzBoxWindow::modelFonbookReset() {
	for (int pos=0; pos<modelFonbook->columnCount(QModelIndex()); pos++)
		treeFonbook->resizeColumnToContents(pos);
}

void KFritzBoxWindow::modelCalllistReset() {
	for (int pos=0; pos<modelCalllist->columnCount(QModelIndex()); pos++)
		treeCallList->resizeColumnToContents(pos);
}

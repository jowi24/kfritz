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

#include "KFritzBoxWindow.h"
#include "Log.h"
#include <Config.h>
#include <KApplication>
#include <KAction>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <KListWidget>

KFritzBoxWindow::KFritzBoxWindow()
{
	logArea = new KTextEdit();
	KListWidget *list = new KListWidget();
	list->addItem(new QListWidgetItem("Test"));
	setCentralWidget(list);
	fritz::Config::SetupLogging(new LogStream(LogBuf::DEBUG, logArea),
                    			new LogStream(LogBuf::INFO,  logArea),
	                      		new LogStream(LogBuf::ERROR, logArea));

	KAction* clearAction = new KAction(this);
	clearAction->setText(i18n("Clear"));
	clearAction->setIcon(KIcon("document-new"));
	clearAction->setShortcut(Qt::CTRL + Qt::Key_W);
	actionCollection()->addAction("clear", clearAction);
	connect(clearAction, SIGNAL(triggered(bool)), logArea, SLOT(clear()));

	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());


	// init actions
	KAction* clearAction = new KAction(this);
	clearAction->setText(i18n("Clear"));
	clearAction->setIcon(KIcon("document-new"));
	clearAction->setShortcut(Qt::CTRL + Qt::Key_W);
	actionCollection()->addAction("clear", clearAction);
	connect(clearAction, SIGNAL(triggered(bool)), textArea, SLOT(clear()));

	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
	setupGUI();
}

KFritzBoxWindow::~KFritzBoxWindow()
{
}

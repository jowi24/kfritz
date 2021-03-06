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


#include "LogDialog.h"

#include <KLocale>

LogDialog::LogDialog(QWidget *parent)
:KDialog(parent) {
	setButtons(Close | Reset);
	logArea = new KTextEdit(this);
	logArea->setReadOnly(true);
    logArea->setFontFamily("Courier");
	setMainWidget(logArea);
	connect(this, SIGNAL(resetClicked()), this, SLOT(resetLog()));
	resize(640,480);
	setCaption(i18n("Log"));
	hide();
}

LogDialog::~LogDialog() {
}

void LogDialog::resetLog() {
	logArea->clear();
}

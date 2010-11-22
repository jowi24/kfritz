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


#include <KLocale>
#include <KMessageBox>

#include <FritzClient.h>

#include "DialDialog.h"
#include "Log.h"

DialDialog::DialDialog(QWidget *parent, std::string number)
:KDialog(parent) {
	setButtons(Ok | Cancel);
	numberLine = new KLineEdit(this);
	numberLine->setText(number.c_str());
	setMainWidget(numberLine);
	connect(this, SIGNAL(okClicked()), this, SLOT(dialNumber()));
	resize(300,100);
	setCaption(i18n("Dial number"));
}

DialDialog::~DialDialog() {
}

void DialDialog::dialNumber() {
	DBG("Dialing number = " << numberLine->text().toStdString());
	fritz::FritzClient fc;
	std::string number = numberLine->text().toStdString();
	fc.InitCall(number);
	hide();
	KMessageBox::information(this, i18n("Dialing initiated, pick up your phone now."));
}

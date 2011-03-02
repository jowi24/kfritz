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
#include <QRegExpValidator>

#include <FritzClient.h>

#include "DialDialog.h"
#include "Log.h"

DialDialog::DialDialog(QWidget *parent, std::string number)
:KDialog(parent) {
	setButtons(Ok | Cancel);
	setButtonText(Ok, i18n("Dial"));
	connect(this, SIGNAL(okClicked()), this, SLOT(dialNumber()));
	resize(300,100);
	setCaption(i18n("Dial number"));
	QWidget *widget = new QWidget();
	ui = new Ui::DialDialog();
	ui->setupUi(widget);
	setMainWidget(widget);
	ui->numberLine->setText(number.c_str());
	QRegExp rx("[0-9\\*#]+");
	QRegExpValidator *validator = new QRegExpValidator(rx, this);
	ui->numberLine->setValidator(validator);
	ui->numberLine->setFocus();

	// populate msn combo box
	std::vector<std::string> names = fritz::gConfig->getSipNames();
	std::vector<std::string> msns = fritz::gConfig->getSipMsns();
	ui->msnComboBox->addItem(i18n("Default"), "");
	QString line("*111# - ");
	line.append(i18n("Conventional telephone network"));
	ui->msnComboBox->addItem(line, "*111#");
	for (size_t i=0; i<names.size(); i++) {
		std::stringstream prefix;
		prefix << "*12" << (i+1) << "#";
		std::stringstream line;
		line << prefix.str() << " - " << names[i] << " (" << msns[i] << ")";
		ui->msnComboBox->addItem(line.str().c_str(), prefix.str().c_str());
	}
}

DialDialog::~DialDialog() {
	delete ui;
}

void DialDialog::dialNumber() {
	DBG("Dialing number = " << ui->numberLine->text().toStdString());
	std::string number = ui->numberLine->text().toStdString();
	std::string prefix = ui->msnComboBox->itemData(ui->msnComboBox->currentIndex()).toString().toStdString();
	number.insert(0, prefix);

	fritz::FritzClient fc;
	fc.InitCall(number);
	hide();
	KMessageBox::information(this, i18n("Dialing initiated, pick up your phone now."));
}

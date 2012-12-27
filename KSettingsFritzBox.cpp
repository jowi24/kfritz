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

#include "KSettingsFritzBox.h"

#include <KConfigDialogManager>

KSettingsFritzBox::KSettingsFritzBox(QWidget *parent)
:QWidget(parent) {
	ui = new Ui_KSettingsFritzBox();
	ui->setupUi(this);
	layout()->addWidget(new KMSNListWidget(this, ui->msnList)); // invisible helper widget
}

KSettingsFritzBox::~KSettingsFritzBox() {
	delete ui;
}

KMSNListWidget::KMSNListWidget(QWidget *parent, KEditListBox *msnList)
:QWidget(parent) {
	KConfigDialogManager::changedMap()->insert("KMSNListWidget", SIGNAL(listChanged(const QStringList &)));
	setObjectName(QString::fromUtf8("kcfg_MSNFilter"));
	this->msnList = msnList;

	connect(msnList, SIGNAL(added(const QString &)), this, SLOT(listChangedSlot()));
	connect(msnList, SIGNAL(changed()), this, SLOT(listChangedSlot()));
    connect(msnList, SIGNAL(removed(const QString &)), this, SLOT(listChangedSlot()));
}

KMSNListWidget::~KMSNListWidget() {
}

QStringList KMSNListWidget::getList() const {
	return msnList->items();
}

void KMSNListWidget::setList(QStringList &list) {
	msnList->setItems(list);
}

void KMSNListWidget::listChangedSlot() {
	emit listChanged(getList());
}


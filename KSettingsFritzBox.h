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

#ifndef KSETTINGSFRITZBOX_H
#define KSETTINGSFRITZBOX_H

#include <QWidget>
#include <QListWidgetItem>
#include <KEditListBox>

#include "ui_KSettingsFritzBox.h"

// use this class to add a configuration page to a KConfigDialog

class KSettingsFritzBox: public QWidget {
	Q_OBJECT
private:
	Ui_KSettingsFritzBox *ui;
public:
	KSettingsFritzBox(QWidget *parent);
	virtual ~KSettingsFritzBox();
};

// this is a wrapper class used by KSettingsFritzBox to enable
// auto-management of KEditListBox by KConfigDialog

class KMSNListWidget : public QWidget {
	Q_OBJECT

	Q_PROPERTY(QStringList list READ getList WRITE setList NOTIFY listChanged USER true)
private:
	KEditListBox *msnList;
public:
	KMSNListWidget(QWidget *parent, KEditListBox *msnList);
	virtual ~KMSNListWidget();
	QStringList getList() const;
	void setList(QStringList &list);
public Q_SLOTS:
	void listChangedSlot();
Q_SIGNALS:
	void listChanged(const QStringList &text);
};

#endif /* KSETTINGSFRITZBOX_H_ */

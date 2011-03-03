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

#ifndef KSETTINGSFONBOOKS_H
#define KSETTINGSFONBOOKS_H

#include <QWidget>

#include <FonbookManager.h>

#include "ui_KSettingsFonbooks.h"

// use this class to add a configuration page to a KConfigDialog

class KSettingsFonbooks : public QWidget {
	Q_OBJECT
private:
	Ui_KSettingsFonbooks *ui;
public:
	KSettingsFonbooks(QWidget *parent);
	virtual ~KSettingsFonbooks();
};

// this is a wrapper class used by KSettingsFonbooks to enable
// auto-management of KActionSelector by KConfigDialog

class KFonbooksWidget : public QWidget {
	Q_OBJECT

    Q_PROPERTY(QStringList list READ getList WRITE setList NOTIFY listChanged USER true)
private:
	KActionSelector *actionSelector;
	fritz::Fonbooks *fonbooks;
public:
	KFonbooksWidget(QWidget *parent, KActionSelector *actionSelector);
	virtual ~KFonbooksWidget();
	QStringList getList() const;
	void setList(QStringList &list);
public Q_SLOTS:
	void listChangedSlot();
Q_SIGNALS:
	void listChanged(const QStringList &text);
};


#endif /* KSETTINGSFONBOOKS_H_ */

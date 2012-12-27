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

#ifndef DIALDIALOG_H
#define DIALDIALOG_H

#include <KDialog>

#include "ui_DialDialog.h"

class DialDialog: public KDialog {
	Q_OBJECT
private:
	Ui::DialDialog *ui;
public:
	explicit DialDialog(QWidget *parent, std::string number = "");
	virtual ~DialDialog();
public Q_SLOTS:
	void dialNumber();
};

#endif /* DIALDIALOG_H_ */

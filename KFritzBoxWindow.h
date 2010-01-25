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

#ifndef KFRITZBOXWINDOW_H_
#define KFRITZBOXWINDOW_H_

#include <KXmlGuiWindow>
#include <KTextEdit>
#include <KTabWidget>
#include <KWallet/Wallet>
#include <QTreeView>
#include "KFonbookModel.h"
#include "KCalllistModel.h"
#include "LibFritzInit.h"
#include "QAdaptTreeView.h"

class KFritzBoxWindow : public KXmlGuiWindow
{
	Q_OBJECT
private:
	KFonbookModel *modelFonbook;
	KCalllistModel *modelCalllist;
	QAdaptTreeView *treeFonbook, *treeCallList;
	KTabWidget *tabWidget;
	LibFritzInit *libFritzInit;
	QString fbPassword;
	void saveToWallet(KWallet::Wallet *wallet);
	bool showPasswordDialog(QString &password, bool offerSaving = false);
	void setupActions();
public:
	KFritzBoxWindow();
	virtual ~KFritzBoxWindow();
public Q_SLOTS:
	void showSettings(bool b);
	void showNotificationSettings(bool b);
	void updateConfiguration(const QString &dialogName = QString());
	void reenterPassword();

};

#endif /*KFRITZBOXWINDOW_H_*/

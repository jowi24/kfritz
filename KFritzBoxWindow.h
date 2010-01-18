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
#include <QTreeView>
#include "KFonbookModel.h"
#include "KCalllistModel.h"

class KFritzBoxWindow : public KXmlGuiWindow
{
	Q_OBJECT
private:
	KTextEdit* logArea;
	KFonbookModel *modelFonbook;
	KCalllistModel *modelCalllist;
	QTreeView *treeFonbook, *treeCallList;
public:
	KFritzBoxWindow(KTextEdit *logArea);
	virtual ~KFritzBoxWindow();

private slots:
void modelCalllistReset();
void modelFonbookReset();
//void showFonbook(bool b);
//void showCalllist(bool b);
//void showLog(bool b);

};

#endif /*KFRITZBOXWINDOW_H_*/

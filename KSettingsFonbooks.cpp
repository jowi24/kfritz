/*
 * KFritz
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

#include "KSettingsFonbooks.h"

#include <KConfigDialogManager>
#include <QListWidgetItem>

KSettingsFonbooks::KSettingsFonbooks(QWidget *parent)
:QWidget(parent) {
    ui = new Ui_KSettingsFonbooks();
    ui->setupUi(this);
    layout()->addWidget(new KFonbooksWidget(this, ui->actionSelector));
}

KSettingsFonbooks::~KSettingsFonbooks() {
	delete ui;
}

class KFonbooksWidgetListItem : public QListWidgetItem {
private:
	QString id;
public:
	KFonbooksWidgetListItem(const QString text, const QString id, QListWidget * parent = 0)
	:QListWidgetItem(text, parent) {
		this->id = id;
	}
	QString getId() {
		return id;
	}
};

KFonbooksWidget::KFonbooksWidget(QWidget *parent, KActionSelector *actionSelector)
:QWidget(parent) {
	KConfigDialogManager::changedMap()->insert("KFonbooksWidget", SIGNAL(listChanged(const QStringList &)));
	setObjectName(QString::fromUtf8("kcfg_PhonebookList"));
	this->actionSelector = actionSelector;
    connect(actionSelector, SIGNAL(added(QListWidgetItem *)), this, SLOT(listChangedSlot()));
    connect(actionSelector, SIGNAL(removed(QListWidgetItem *)), this, SLOT(listChangedSlot()));
    connect(actionSelector, SIGNAL(movedUp(QListWidgetItem *)), this, SLOT(listChangedSlot()));
    connect(actionSelector, SIGNAL(movedDown(QListWidgetItem *)), this, SLOT(listChangedSlot()));
    fonbooks = fritz::FonbookManager::GetFonbookManager()->GetFonbooks();
    for (size_t pos=0; pos < fonbooks->size(); pos++)
    	new KFonbooksWidgetListItem((*fonbooks)[pos]->GetTitle().c_str(),
    			                    (*fonbooks)[pos]->GetTechId().c_str(),
    			                    actionSelector->availableListWidget());
}

KFonbooksWidget::~KFonbooksWidget() {
}

QStringList KFonbooksWidget::getList() const {
	QStringList list;
	for (int pos=0; pos < actionSelector->selectedListWidget()->count(); pos++) {
		KFonbooksWidgetListItem *item = static_cast<KFonbooksWidgetListItem*>(actionSelector->selectedListWidget()->item(pos));
		list.append(item->getId());
	}
	return list;
}

void KFonbooksWidget::setList(QStringList &list) {
	// move all to left
	while(actionSelector->selectedListWidget()->count())
		actionSelector->availableListWidget()->addItem(actionSelector->selectedListWidget()->takeItem(0));
	// move selected items to right
	for (int i=0; i < list.count(); i++) {
		for (int j=0; j < actionSelector->availableListWidget()->count(); j++) {
			if (static_cast<KFonbooksWidgetListItem*>(actionSelector->availableListWidget()->item(j))->getId() == list[i]) {
				actionSelector->selectedListWidget()->addItem(actionSelector->availableListWidget()->takeItem(j));
				break;
			}
		}
	}
}

void KFonbooksWidget::listChangedSlot() {
	emit listChanged(getList());
}

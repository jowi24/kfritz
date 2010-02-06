/*
 * KFritz
 *
 * Copyright (C) 2010 Joachim Wilke <vdr@joachim-wilke.de>
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

#include "KFonbookModel.h"

#include <KIcon>
#include <KLocalizedString>

KFonbookModel::KFonbookModel(QString techID) {
	fonbook = NULL;
	this->techID = techID;
}

KFonbookModel::~KFonbookModel() {
}

int KFonbookModel::rowCount(const QModelIndex & parent) const
{
	if (!fonbook)
		return 0;
	if (parent.isValid())
		// the model does not have any hierarchy
		return 0;
	else
		return fonbook->GetFonbookSize();
}

int KFonbookModel::columnCount(const QModelIndex & parent __attribute__((unused))) const
{
	// number of columns is independent of parent, ignoring parameter
	return 3;
}

QVariant KFonbookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	if (orientation == Qt::Horizontal){
		switch (section){
		case 0:
			return i18n("Name");
			break;
		case 1:
			return  i18n("Type");
			break;
		case 2:
			return  i18n("Number");
			break;
		default:
			return QVariant();
		}
	}else {
		return QVariant();
	}

}

QVariant KFonbookModel::data(const QModelIndex & index, int role) const{
	if (!fonbook)
		return QVariant();
	if (role != Qt::DisplayRole)
		return QVariant();

	fritz::FonbookEntry *fe = fonbook->RetrieveFonbookEntry(index.row());
	switch (index.column()) {
	case 0:
		return QVariant(toLocalEncoding(fe->getName()));
		break;
	case 1:
		return QVariant(getTypeName(fe->getType()));
		break;
	case 2:
		return QVariant(toLocalEncoding(fe->getNumber()));
		break;
	default:
		return QVariant();
	}
	return QVariant();
}

QString KFonbookModel::getTypeName(const fritz::FonbookEntry::eType type) {
	switch (type){
	case fritz::FonbookEntry::TYPE_HOME:
		return i18n("Home");
	case fritz::FonbookEntry::TYPE_MOBILE:
		return i18n("Mobile");
	case fritz::FonbookEntry::TYPE_WORK:
		return i18n("Work");
	default:
		return "";
	}
}

void KFonbookModel::sort(int column, Qt::SortOrder order) {
	if (!fonbook)
		return;
	fonbook->Sort((fritz::FonbookEntry::eElements) column, order == Qt::AscendingOrder);
	emit dataChanged(index(0,                       0,                          QModelIndex()),
			index(rowCount(QModelIndex()), columnCount(QModelIndex()), QModelIndex()));
}

void KFonbookModel::libReady(bool isReady) {
	KFritzModel::libReady(isReady);
	if (isReady){
		// get the fonbook resource
		fritz::Fonbooks *books = fritz::FonbookManager::GetFonbookManager()->GetFonbooks();
		fonbook = (*books)[techID.toStdString()];
	}
	else
		fonbook = NULL;
	reset();
}

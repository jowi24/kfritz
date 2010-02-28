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

#include "KFonbookModel.h"

#include <KIcon>
#include <KLocalizedString>

KFonbookModel::KFonbookModel(std::string techID) {
	// get the fonbook resource
	fritz::Fonbooks *books = fritz::FonbookManager::GetFonbookManager()->GetFonbooks();
	fonbook = (*books)[techID];
}

KFonbookModel::~KFonbookModel() {
}

int KFonbookModel::rowCount(const QModelIndex & parent) const
{
	if (parent.isValid())
		// the model does not have any hierarchy
		return 0;
	else
		return fonbook->GetFonbookSize();
}

int KFonbookModel::columnCount(const QModelIndex & parent __attribute__((unused))) const
{
	// number of columns is independent of parent, ignoring parameter
	return 6;
}

QVariant KFonbookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	if (orientation == Qt::Horizontal) {
		switch (section){
		case fritz::FonbookEntry::ELEM_NAME:
			return i18n("Name");
			break;
		case fritz::FonbookEntry::ELEM_TYPE:
			return i18n("Type");
			break;
		case fritz::FonbookEntry::ELEM_NUMBER:
			return i18n("Number");
			break;
		case fritz::FonbookEntry::ELEM_QUICKDIAL:
			return i18n("Quickdial");
			break;
		case fritz::FonbookEntry::ELEM_VANITY:
			return i18n("Vanity");
			break;
		case fritz::FonbookEntry::ELEM_IMPORTANT:
			return i18n("Important");
			break;
		default:
			return QVariant();
		}
	} else {
		return QVariant();
	}

}

QVariant KFonbookModel::data(const QModelIndex & index, int role) const{
	if (role != Qt::DisplayRole)
		return QVariant();

	fritz::FonbookEntry *fe = fonbook->RetrieveFonbookEntry(index.row());
	switch (index.column()) {
	case fritz::FonbookEntry::ELEM_NAME:
		return QVariant(toLocalEncoding(fe->getName()));
		break;
	case fritz::FonbookEntry::ELEM_TYPE:
		return QVariant(getTypeName(fe->getType()));
		break;
	case fritz::FonbookEntry::ELEM_NUMBER:
		return QVariant(toLocalEncoding(fe->getNumber()));
		break;
	case fritz::FonbookEntry::ELEM_QUICKDIAL:
		return QVariant(toLocalEncoding(fe->getQuickdialFormatted()));
		break;
	case fritz::FonbookEntry::ELEM_VANITY:
		return QVariant(toLocalEncoding(fe->getVanityFormatted()));
		break;
	case fritz::FonbookEntry::ELEM_IMPORTANT:
		return QVariant(toLocalEncoding((fe->isImportant()) ? "X": "")); //TODO: make some fancy icon here :-P
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
	fonbook->Sort((fritz::FonbookEntry::eElements) column, order == Qt::AscendingOrder);
	emit dataChanged(index(0,                       0,                          QModelIndex()),
			index(rowCount(QModelIndex()), columnCount(QModelIndex()), QModelIndex()));
}

std::string KFonbookModel::number(const QModelIndex &i) const {
	fritz::FonbookEntry *fe = fonbook->RetrieveFonbookEntry(i.row());
	return fe->getNumber();
}

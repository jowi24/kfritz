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

#include "Log.h"

/**
 * KFonbookModel constructs a hierarchy model for fritz::Fonbooks
 *
 * The top level table contains one row for each fritz::FonbookEntry.
 * Each name cell references to a table containing all numbers of
 * this FonbookEntry.
 *
 * Example - Top level table
 * Row/Column   1
 * 1            Alice
 * 2            Bob
 * 3            Charly
 * ...
 *
 * Example - Details table for (1/1) (Alice)
 * Row/Column   1         2         3      ...
 * 1            Private   555-1234  **711
 * 2            Mobile    555-6789
 * 3            Business  555-0000
 */

enum modelColumns {
	COLUMN_NAME,
	COLUMN_NUMBER,
	COLUMN_QUICKDIAL,
	COLUMN_VANITY,
	COLUMNS_COUNT
};

KFonbookModel::KFonbookModel(std::string techID) {
	// get the fonbook resource
	fritz::Fonbooks *books = fritz::FonbookManager::GetFonbookManager()->GetFonbooks();
	fonbook = (*books)[techID];
}

KFonbookModel::~KFonbookModel() {
}

int KFonbookModel::rowCount(const QModelIndex & parent) const
{
	// The top level table has details table attached to each cell in COLUMN_NAME
	if (parent.isValid() && parent.column() == COLUMN_NAME && !parent.parent().isValid())
		return fritz::FonbookEntry::TYPES_COUNT-1; // all types without TYPE_NONE
	// A details table has no children
	else if (parent.isValid())
		return 0;
	// The top level table has a row for each FonbookEntry
	else
		return fonbook->GetFonbookSize();
}

int KFonbookModel::columnCount(const QModelIndex & parent) const
{
	if (parent.isValid() && parent.parent().isValid())
		return 0;
	return COLUMNS_COUNT;
}

QVariant KFonbookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	if (orientation == Qt::Horizontal) {
		switch (section) {
		case COLUMN_NAME:
			return i18n("Name");
			break;
		case COLUMN_NUMBER:
			return i18n("Number");
			break;
		case COLUMN_QUICKDIAL:
			return i18n("Quickdial");
			break;
		case COLUMN_VANITY:
			return i18n("Vanity");
			break;
		default:
			return QVariant();
		}
	} else
		return QVariant();
}

QVariant KFonbookModel::data(const QModelIndex & index, int role) const {
	// Neither top level nor details table
	if (index.parent().isValid() && index.parent().parent().isValid())
		return QVariant();
	// Details table
	else if (index.parent().isValid()) {

		fritz::FonbookEntry *fe = fonbook->RetrieveFonbookEntry(index.parent().row());

		if (role != Qt::DisplayRole)
			return QVariant();

		// map row to type
		fritz::FonbookEntry::eType type = (fritz::FonbookEntry::eType) (index.row() + 1); // ignore TYPE_NONE

		switch (index.column()) {
		case COLUMN_NAME: {
			QString typeName = getTypeName(type);
			if (fe->getPriority(type) == 1)
				typeName += " " + i18n("(Default)");
			return QVariant(typeName);
		}
		case COLUMN_NUMBER:
			return QVariant(fe->getNumber(type).c_str());
//		case COLUMN_QUICKDIAL:
//			return QVariant(fe->getQuickdialFormatted(type).c_str());
//		case COLUMN_VANITY:
//			return QVariant(fe->getVanityFormatted(type).c_str());
		default:
			return QVariant();
		}
	}
	// Top level table
	else {
		fritz::FonbookEntry *fe = fonbook->RetrieveFonbookEntry(index.row());

		if (role == Qt::DecorationRole && index.column() == COLUMN_NAME)
			return QVariant(fe->isImportant() ? KIcon("emblem-important") : KIcon("x-office-contact"));
		if (role == Qt::ToolTipRole && index.column() == COLUMN_NAME)
			return QVariant(fe->isImportant() ? i18n("Important contact") : "");
		if (role != Qt::DisplayRole)
			return QVariant();

		switch (index.column()) {
		case COLUMN_NAME: {
			//QString name = "<b>" + toLocalEncoding(fe->getName()) + "</b>"; //TODO: formatting?
			QString name = toLocalEncoding(fe->getName());
			return QVariant(name);
		}
		case COLUMN_NUMBER:
			return QVariant(fe->getNumber(fritz::FonbookEntry::TYPE_NONE).c_str());
		case COLUMN_QUICKDIAL:
			return QVariant(fe->getQuickdialFormatted().c_str());
		case COLUMN_VANITY:
			return QVariant(fe->getVanityFormatted().c_str());
		default:
			return QVariant();
		}
	}
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
	fritz::FonbookEntry::eElements element;
	switch (column) {
	case COLUMN_NAME:
		element = fritz::FonbookEntry::ELEM_NAME;
		break;
	case COLUMN_NUMBER:
		return;
	case COLUMN_QUICKDIAL:
		element = fritz::FonbookEntry::ELEM_QUICKDIAL;
		break;
	case COLUMN_VANITY:
		element = fritz::FonbookEntry::ELEM_VANITY;
		break;
	default:
		ERR("Invalid column adressed while sorting.");
	}
	fonbook->Sort(element, order == Qt::AscendingOrder);
	emit dataChanged(index(0,                       0,                          QModelIndex()),
			index(rowCount(QModelIndex()), columnCount(QModelIndex()), QModelIndex()));
}

std::string KFonbookModel::number(const QModelIndex &i) const {
	if (i.parent().isValid() && !i.parent().parent().isValid()) {
		fritz::FonbookEntry *fe = fonbook->RetrieveFonbookEntry(i.parent().row());
		return fe->getNumber((fritz::FonbookEntry::eType) i.row());
	}
	return "";
}

QModelIndex KFonbookModel::index(int row, int column, const QModelIndex &parent) const {
	if (parent.isValid() && parent.parent().isValid())
		return QModelIndex();
	if (parent.isValid()) {
		// index of a details table get its parent row as unique id
		QModelIndex index = createIndex(row, column, parent.row());
		return index;
	}
	else
		// index of top level table get -1 as id
		return createIndex(row, column, -1);
}

QModelIndex KFonbookModel::parent(const QModelIndex &child) const {
	// index of a details table has id >= 0 (see index())
	if (child.internalId() >= 0)
		return index(child.internalId(), 0, QModelIndex());
	return QModelIndex();
}

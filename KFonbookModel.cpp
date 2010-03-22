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
 * KFonbookModel
 */

enum modelColumns {
	COLUMN_NAME,
	COLUMN_NUMBER_HOME,
	COLUMN_NUMBER_MOBILE,
	COLUMN_NUMBER_WORK,
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
	if (parent.isValid())
		return 0;
	return fonbook->GetFonbookSize();
}

int KFonbookModel::columnCount(const QModelIndex & parent) const
{
	if (parent.isValid())
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
		case COLUMN_NUMBER_HOME:
			return i18n("Home");
		case COLUMN_NUMBER_MOBILE:
			return i18n("Mobile");
		case COLUMN_NUMBER_WORK:
			return i18n("Work");
		case COLUMN_QUICKDIAL:
			return i18n("Quickdial");
		case COLUMN_VANITY:
			return i18n("Vanity");
		default:
			return QVariant();
		}
	} else
		return QVariant();
}

QVariant KFonbookModel::data(const QModelIndex & index, int role) const {
	// Neither top level nor details table
	if (index.parent().isValid())
		return QVariant();

	fritz::FonbookEntry *fe = fonbook->RetrieveFonbookEntry(index.row());

	// Indicate important contacts using icon and tooltip
	if (role == Qt::DecorationRole && index.column() == COLUMN_NAME)
		return QVariant(fe->isImportant() ? KIcon("emblem-important") : KIcon("x-office-contact"));
	if (role == Qt::ToolTipRole && index.column() == COLUMN_NAME)
		return QVariant(fe->isImportant() ? i18n("Important contact") : "");

	// Indicate default number using bold font face and tooltip
	if (role == Qt::FontRole || role == Qt::ToolTipRole) {
		bool defaultNumber = false;
		switch(index.column()) {
		case COLUMN_NUMBER_HOME:
			defaultNumber = (fe->getPriority(fritz::FonbookEntry::TYPE_HOME) == 1);
			break;
		case COLUMN_NUMBER_MOBILE:
			defaultNumber = (fe->getPriority(fritz::FonbookEntry::TYPE_MOBILE) == 1);
			break;
		case COLUMN_NUMBER_WORK:
			defaultNumber = (fe->getPriority(fritz::FonbookEntry::TYPE_WORK) == 1);
			break;
		}
		if (defaultNumber) {
			if (role == Qt::FontRole) {
				QFont font;
				font.setBold(true);
				return font;
			}
			if (role == Qt::ToolTipRole)
				return i18n("Default number");
		}
	}

	// Skip all other requests, except for displayed text
	if (role != Qt::DisplayRole)
		return QVariant();

	switch (index.column()) {
	case COLUMN_NAME:
		return QVariant(toLocalEncoding(fe->getName()));
	case COLUMN_NUMBER_HOME:
		if (fe->getNumber(fritz::FonbookEntry::TYPE_NONE).length())
			return QVariant(fe->getNumber(fritz::FonbookEntry::TYPE_NONE).c_str());
		return QVariant(fe->getNumber(fritz::FonbookEntry::TYPE_HOME).c_str());
	case COLUMN_NUMBER_MOBILE:
		return QVariant(fe->getNumber(fritz::FonbookEntry::TYPE_MOBILE).c_str());
	case COLUMN_NUMBER_WORK:
		return QVariant(fe->getNumber(fritz::FonbookEntry::TYPE_WORK).c_str());
	case COLUMN_QUICKDIAL:
		return QVariant(fe->getQuickdialFormatted().c_str());
	case COLUMN_VANITY:
		return QVariant(fe->getVanityFormatted().c_str());
	default:
		return QVariant();
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
	case COLUMN_NUMBER_HOME:
	case COLUMN_NUMBER_MOBILE:
	case COLUMN_NUMBER_WORK:

		return; //TODO: sorting - does this need anybody?
	case COLUMN_QUICKDIAL:
		element = fritz::FonbookEntry::ELEM_QUICKDIAL;
		break;
	case COLUMN_VANITY:
		element = fritz::FonbookEntry::ELEM_VANITY;
		break;
	default:
		ERR("Invalid column adressed while sorting.");
		return;
	}
	fonbook->Sort(element, order == Qt::AscendingOrder);
	emit dataChanged(index(0,              0,                          QModelIndex()),
			index(rowCount(QModelIndex()), columnCount(QModelIndex()), QModelIndex()));
}

std::string KFonbookModel::number(const QModelIndex &i) const {
	if (!i.parent().isValid()) {
		fritz::FonbookEntry *fe = fonbook->RetrieveFonbookEntry(i.row());
		switch (i.column()) {
		case COLUMN_NUMBER_HOME:
			return fe->getNumber(fritz::FonbookEntry::TYPE_HOME);
		case COLUMN_NUMBER_MOBILE:
			return fe->getNumber(fritz::FonbookEntry::TYPE_MOBILE);
		case COLUMN_NUMBER_WORK:
			return fe->getNumber(fritz::FonbookEntry::TYPE_WORK);
		default:
			return "";
		}
	}
	return "";
}

QModelIndex KFonbookModel::index(int row, int column, const QModelIndex &parent) const {
	if (parent.isValid())
		return QModelIndex();
	return createIndex(row, column);
}

QModelIndex KFonbookModel::parent(const QModelIndex &child __attribute__((unused))) const {
	return QModelIndex();
}

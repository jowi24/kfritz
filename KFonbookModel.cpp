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
	COLUMN_NUMBER_1,
	COLUMN_NUMBER_2,
	COLUMN_NUMBER_3,
	COLUMN_QUICKDIAL,
	COLUMN_VANITY,
	COLUMNS_COUNT
};

KFonbookModel::KFonbookModel(std::string techID) {
	// get the fonbook resource
	fritz::Fonbooks *books = fritz::FonbookManager::GetFonbookManager()->GetFonbooks();
	fonbook = (*books)[techID];
	lastRows = 0;
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
		case COLUMN_NUMBER_1:
			return i18n("Number 1");
		case COLUMN_NUMBER_2:
			return i18n("Number 2");
		case COLUMN_NUMBER_3:
			return i18n("Number 3");
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

	const fritz::FonbookEntry *fe = fonbook->RetrieveFonbookEntry(index.row());
	if (!fe) {
		ERR("No FonbookEntry for index row " << index.row());
		return QVariant();
	}

	// Indicate important contacts using icon and tooltip
	if (role == Qt::DecorationRole && index.column() == COLUMN_NAME)
		return QVariant(fe->IsImportant() ? KIcon("emblem-important") : KIcon("x-office-contact"));
	if (role == Qt::ToolTipRole && index.column() == COLUMN_NAME)
		return QVariant(fe->IsImportant() ? i18n("Important contact") : "");

	// Indicate default number using bold font face and tooltip
	if (role == Qt::FontRole || role == Qt::ToolTipRole) {
		bool defaultNumber = false;
		fritz::FonbookEntry::eType type = fritz::FonbookEntry::TYPE_NONE;
		switch(index.column()) {
		case COLUMN_NUMBER_1:
			defaultNumber = (fe->GetPriority(0) == 1);
			type = fe->GetType(0);
			break;
		case COLUMN_NUMBER_2:
			defaultNumber = (fe->GetPriority(1) == 1);
			type = fe->GetType(1);
			break;
		case COLUMN_NUMBER_3:
			defaultNumber = (fe->GetPriority(2) == 1);
			type = fe->GetType(2);
			break;
		}
		QString tooltip = getTypeName(type);
		if (defaultNumber) {
			if (role == Qt::FontRole) {
				QFont font;
				font.setBold(true);
				return font;
			}
			tooltip += " ("+ (i18n("Default number")) + ")";
		}
		if (role == Qt::ToolTipRole)
			return tooltip;
	}

	// Skip all other requests, except for displayed text
	if (role != Qt::DisplayRole && role != Qt::EditRole)
		return QVariant();

	switch (index.column()) {
	case COLUMN_NAME:
		return QVariant(toUnicode(fe->GetName()));
	case COLUMN_NUMBER_1:
		return QVariant(toUnicode(fe->GetNumber(0)));
	case COLUMN_NUMBER_2:
		return QVariant(toUnicode(fe->GetNumber(1)));
	case COLUMN_NUMBER_3:
		return QVariant(toUnicode(fe->GetNumber(2)));
	case COLUMN_QUICKDIAL:
		if (role == Qt::EditRole)
			return QVariant(toUnicode(fe->GetQuickdial()));
		else
			return QVariant(toUnicode(fe->GetQuickdialFormatted()));
	case COLUMN_VANITY:
		if (role == Qt::EditRole)
			return QVariant(toUnicode(fe->GetVanity()));
		else
			return QVariant(toUnicode(fe->GetVanityFormatted()));
	default:
		return QVariant();
	}
}

bool KFonbookModel::setData (const QModelIndex & index, const QVariant & value, int role) {
	if (role == Qt::EditRole) {
		const fritz::FonbookEntry *_fe = fonbook->RetrieveFonbookEntry(index.row());
		fritz::FonbookEntry fe(*_fe);
		switch(index.column()) {
		case COLUMN_NAME:
			fe.SetName(fromUnicode(value.toString()));
			break;
		case COLUMN_NUMBER_1:
			fe.SetNumber(fromUnicode(value.toString()), 0);
			break;
		case COLUMN_NUMBER_2:
			fe.SetNumber(fromUnicode(value.toString()), 1);
			break;
		case COLUMN_NUMBER_3:
			fe.SetNumber(fromUnicode(value.toString()), 2);
			break;
		case COLUMN_QUICKDIAL:
			fe.SetQuickdial(fromUnicode(value.toString()));
			break;
		case COLUMN_VANITY:
			fe.SetVanity(fromUnicode(value.toString()));
			break;
		default:
			return false;
		}
		fonbook->ChangeFonbookEntry(index.row(), fe);
		emit dataChanged(index, index); // we changed one element
		return true;
	}
	return false;
}

void KFonbookModel::setDefault(const QModelIndex &index) {
	switch(index.column()) {
	case COLUMN_NUMBER_1:
		fonbook->SetDefault(index.row(), 0);
		break;
	case COLUMN_NUMBER_2:
		fonbook->SetDefault(index.row(), 1);
		break;
	case COLUMN_NUMBER_3:
		fonbook->SetDefault(index.row(), 2);
		break;
	default:
		return;
	}
	QModelIndex indexLeft = createIndex(index.row(), COLUMN_NUMBER_1);
	QModelIndex indexRight = createIndex(index.row(), COLUMN_NUMBER_3);
	emit dataChanged(indexLeft, indexRight); // we changed up to three elements
}

bool KFonbookModel::insertRows(int row, int count __attribute__((unused)), const QModelIndex &parent) {
	beginInsertRows(parent, row, row);
	fritz::FonbookEntry fe(i18n("New Entry").toStdString());
	fonbook->AddFonbookEntry(fe, row);
	endInsertRows();
	return true;
}

bool KFonbookModel::insertFonbookEntry(const QModelIndex &index, fritz::FonbookEntry &fe) {
	beginInsertRows(QModelIndex(), index.row(), index.row());
	fonbook->AddFonbookEntry(fe, index.row());
	endInsertRows();
	return true;
}

const fritz::FonbookEntry *KFonbookModel::retrieveFonbookEntry(const QModelIndex &index) const {
	return fonbook->RetrieveFonbookEntry(index.row());
}

bool KFonbookModel::removeRows(int row, int count __attribute__((unused)), const QModelIndex &parent) {
	beginRemoveRows(parent,row,row);
	if(fonbook->DeleteFonbookEntry(row)){
		endRemoveRows();
		return true;
	} else
		return false;
}

Qt::ItemFlags KFonbookModel::flags(const QModelIndex & index) const {
	if (fonbook->isWriteable())
		return Qt::ItemFlags(QAbstractItemModel::flags(index) | QFlag(Qt::ItemIsEditable));
	else
		return QAbstractItemModel::flags(index);
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
	case COLUMN_NUMBER_1:
	case COLUMN_NUMBER_2:
	case COLUMN_NUMBER_3:

		return; //TODO: sorting - does this need anybody?
	case COLUMN_QUICKDIAL:
		element = fritz::FonbookEntry::ELEM_QUICKDIAL;
		break;
	case COLUMN_VANITY:
		element = fritz::FonbookEntry::ELEM_VANITY;
		break;
	default:
		ERR("Invalid column addressed while sorting.");
		return;
	}
	fonbook->Sort(element, order == Qt::AscendingOrder);
	emit dataChanged(index(0,              0,                          QModelIndex()),
			index(rowCount(QModelIndex()), columnCount(QModelIndex()), QModelIndex()));
}

std::string KFonbookModel::number(const QModelIndex &i) const {
	if (!i.parent().isValid()) {
		const fritz::FonbookEntry *fe = fonbook->RetrieveFonbookEntry(i.row());
		switch (i.column()) {
		case COLUMN_NUMBER_1:
			return fe->GetNumber(0);
		case COLUMN_NUMBER_2:
			return fe->GetNumber(1);
		case COLUMN_NUMBER_3:
			return fe->GetNumber(2);
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

void KFonbookModel::check() {
	if (lastRows != rowCount(QModelIndex())) {
		reset();
		emit updated();
		// stop timer, because no more changes are expected
		timer->stop();
		lastRows = rowCount(QModelIndex());
	}
}

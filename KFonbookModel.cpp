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

#include "KFonbookModel.h"

#include <QFont>
#include <KIcon>
#include <KLocalizedString>

#include "liblog++/Log.h"

/**
 * KFonbookModel
 */

enum modelColumns {
	COLUMN_NAME,
	COLUMN_NUMBER_FIRST,
	COLUMN_NUMBER_2,
	COLUMN_NUMBER_LAST,
	COLUMN_QUICKDIAL,
	COLUMN_VANITY,
	COLUMNS_COUNT
};

KFonbookModel::KFonbookModel(std::string techID) {
	// get the fonbook resource
    fritz::Fonbooks *books = fritz::FonbookManager::GetFonbookManager()->getFonbooks();
	fonbook = (*books)[techID];
	lastRows = 0;
}

KFonbookModel::~KFonbookModel() {
}

int KFonbookModel::rowCount(const QModelIndex & parent) const
{
	if (parent.isValid())
		return 0;
    return fonbook->getFonbookSize();
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
		case COLUMN_NUMBER_FIRST:
			return i18n("Number 1");
		case COLUMN_NUMBER_2:
			return i18n("Number 2");
		case COLUMN_NUMBER_LAST:
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

    const fritz::FonbookEntry *fe = fonbook->retrieveFonbookEntry(index.row());
	if (!fe) {
		ERR("No FonbookEntry for index row " << index.row());
		return QVariant();
	}

	// Indicate important contacts using icon and tooltip
	if (role == Qt::DecorationRole && index.column() == COLUMN_NAME)
        return QVariant(fe->isImportant() ? KIcon("emblem-important") : KIcon("x-office-contact"));
	if (role == Qt::ToolTipRole && index.column() == COLUMN_NAME)
        return QVariant(fe->isImportant() ? i18n("Important contact") : "");

	// Indicate default number using bold font face and tooltip
	if (role == Qt::FontRole || role == Qt::ToolTipRole) {
		bool defaultNumber = false;
		fritz::FonbookEntry::eType type = fritz::FonbookEntry::TYPE_NONE;
		switch(index.column()) {
		case COLUMN_NUMBER_FIRST ... COLUMN_NUMBER_LAST:
            defaultNumber = (fe->getPriority(index.column() - COLUMN_NUMBER_FIRST) == 1);
            type = fe->getType(index.column() - COLUMN_NUMBER_FIRST);
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
        return QVariant(toUnicode(fe->getName()));
	case COLUMN_NUMBER_FIRST ... COLUMN_NUMBER_LAST:
        return QVariant(toUnicode(fe->getNumber(index.column() - COLUMN_NUMBER_FIRST)));
	case COLUMN_QUICKDIAL:
		if (role == Qt::EditRole)
            return QVariant(toUnicode(fe->getQuickdial()));
		else
            return QVariant(toUnicode(fe->getQuickdialFormatted()));
	case COLUMN_VANITY:
		if (role == Qt::EditRole)
            return QVariant(toUnicode(fe->getVanity()));
		else
            return QVariant(toUnicode(fe->getVanityFormatted()));
	default:
		return QVariant();
	}
}

bool KFonbookModel::setData (const QModelIndex & index, const QVariant & value, int role) {
	if (role == Qt::EditRole) {
        const fritz::FonbookEntry *_fe = fonbook->retrieveFonbookEntry(index.row());
		fritz::FonbookEntry fe(*_fe);
		switch(index.column()) {
		case COLUMN_NAME:
            fe.setName(fromUnicode(value.toString()));
			break;
		case COLUMN_NUMBER_FIRST ... COLUMN_NUMBER_LAST:
            fe.setNumber(fromUnicode(value.toString()), index.column() - COLUMN_NUMBER_FIRST);
			break;
		case COLUMN_QUICKDIAL:
            fe.setQuickdial(fromUnicode(value.toString()));  //TODO: check if unique
			break;
		case COLUMN_VANITY:
            fe.setVanity(fromUnicode(value.toString()));     //TODO: check if unique
			break;
		default:
			return false;
		}
        fonbook->changeFonbookEntry(index.row(), fe);
		emit dataChanged(index, index); // we changed one element
		return true;
	}
	return false;
}

void KFonbookModel::setDefault(const QModelIndex &index) {
	switch(index.column()) {
	case COLUMN_NUMBER_FIRST ... COLUMN_NUMBER_LAST:
        fonbook->setDefault(index.row(), index.column() - COLUMN_NUMBER_FIRST);
		break;
	default:
		return;
	}
	QModelIndex indexLeft = createIndex(index.row(), COLUMN_NUMBER_FIRST);
	QModelIndex indexRight = createIndex(index.row(), COLUMN_NUMBER_LAST);
	emit dataChanged(indexLeft, indexRight); // we changed up to three elements
}

size_t KFonbookModel::mapColumnToNumberIndex(int column) {
	return column-1;
}

void KFonbookModel::setType(const QModelIndex &index, fritz::FonbookEntry::eType type) {
    const fritz::FonbookEntry *_fe = fonbook->retrieveFonbookEntry(index.row());
	fritz::FonbookEntry fe(*_fe);
    fe.setType(type, mapColumnToNumberIndex(index.column()));
    fonbook->changeFonbookEntry(index.row(), fe);
	emit dataChanged(index, index);
}

bool KFonbookModel::insertRows(int row, int count __attribute__((unused)), const QModelIndex &parent) {
	beginInsertRows(parent, row, row);
	fritz::FonbookEntry fe(i18n("New Entry").toStdString());
    fonbook->addFonbookEntry(fe, row);
	endInsertRows();
	return true;
}

bool KFonbookModel::insertFonbookEntry(const QModelIndex &index, fritz::FonbookEntry &fe) {
	beginInsertRows(QModelIndex(), index.row(), index.row());
    fonbook->addFonbookEntry(fe, index.row());
	endInsertRows();
	return true;
}

const fritz::FonbookEntry *KFonbookModel::retrieveFonbookEntry(const QModelIndex &index) const {
    return fonbook->retrieveFonbookEntry(index.row());
}

bool KFonbookModel::removeRows(int row, int count __attribute__((unused)), const QModelIndex &parent) {
	beginRemoveRows(parent,row,row);
    if(fonbook->deleteFonbookEntry(row)){
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
	case COLUMN_NUMBER_FIRST ... COLUMN_NUMBER_LAST:
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
    fonbook->sort(element, order == Qt::AscendingOrder);
	emit dataChanged(index(0,              0,                          QModelIndex()),
			index(rowCount(QModelIndex()), columnCount(QModelIndex()), QModelIndex()));
}

std::string KFonbookModel::number(const QModelIndex &i) const {
	if (!i.parent().isValid()) {
        const fritz::FonbookEntry *fe = fonbook->retrieveFonbookEntry(i.row());
		switch (i.column()) {
		case COLUMN_NUMBER_FIRST ... COLUMN_NUMBER_LAST:
            return fe->getNumber(i.column() - COLUMN_NUMBER_FIRST);
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

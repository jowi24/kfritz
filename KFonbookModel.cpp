/*
 * KFonbookModel.cpp
 *
 *  Created on: Dec 21, 2008
 *      Author: joachim
 */

#include "KFonbookModel.h"

#include <KIcon>

KFonbookModel::KFonbookModel() {
	fonbook = NULL;
}

KFonbookModel::~KFonbookModel() {
	// TODO: somthing to do here?
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
			return "Name";
			break;
		case 1:
			return "Type";
			break;
		case 2:
			return "Number";
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
	if (role == Qt::DecorationRole && index.column() == 0)
		return QVariant(KIcon("document-new"));
	if (role != Qt::DisplayRole)
		return QVariant();

	fritz::FonbookEntry *fe = fonbook->RetrieveFonbookEntry(index.row());
	switch (index.column()) {
	case 0:
		return QVariant(toLocalEncoding(fe->getName()));
		break;
	case 1:
		return QVariant(toLocalEncoding(fe->getTypeName()));
		break;
	case 2:
		return QVariant(toLocalEncoding(fe->getNumber()));
		break;
	default:
		return QVariant();
	}
	return QVariant();
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
		fonbook = fritz::FonbookManager::GetFonbook();
	}
	else
		fonbook = NULL;
}

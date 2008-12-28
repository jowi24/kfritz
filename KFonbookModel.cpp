/*
 * KFonbookModel.cpp
 *
 *  Created on: Dec 21, 2008
 *      Author: joachim
 */

#include <KIcon>
#include "KFonbookModel.h"
#include <Tools.h>
#include <iostream>

KFonbookModel::KFonbookModel() {
	// get the fonbook resource
	fonbook = fritz::FonbookManager::GetFonbook();
	inputCodec  = QTextCodec::codecForName(fritz::CharSetConv::SystemCharacterTable() ? fritz::CharSetConv::SystemCharacterTable() : "UTF-8");
}

KFonbookModel::~KFonbookModel() {
	// TODO: somthing to do here?
}

int KFonbookModel::rowCount(const QModelIndex & parent) const
{
	if (parent.isValid())
		// the model does not have any hierarchy
		return 0;
	else
		return fonbook->GetFonbookSize();
}

int KFonbookModel::columnCount(const QModelIndex & parent) const
{
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

QVariant KFonbookModel::data(const QModelIndex & index, int role) const
{
	if (role == Qt::DecorationRole && index.column() == 0)
		return QVariant(KIcon("document-new"));
	if (role != Qt::DisplayRole)
		return QVariant();

	fritz::FonbookEntry *fe = fonbook->RetrieveFonbookEntry(index.row());
	switch (index.column()) {
	case 0:
		return QVariant(inputCodec->toUnicode(fe->getName().c_str()));
		break;
	case 1:
		return QVariant(inputCodec->toUnicode(fe->getTypeName().c_str()));
		break;
	case 2:
		return QVariant(inputCodec->toUnicode(fe->getNumber().c_str()));
		break;
	default:
		return QVariant();
	}
	return QVariant();
}

QModelIndex KFonbookModel::parent(const QModelIndex & child) const
{
	return QModelIndex();
}

QModelIndex KFonbookModel::index(int row, int column, const QModelIndex & parent) const
{
	if (parent.isValid())
		return QModelIndex();
	else
		return createIndex(row, column);
}











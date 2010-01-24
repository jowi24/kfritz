/*
 * KFritzModel.cpp
 *
 *  Created on: Jan 24, 2010
 *      Author: jo
 */

#include "KFritzModel.h"

#include <Tools.h>

KFritzModel::KFritzModel() {
	lastRows = 0;
	timer = new QTimer();
}

KFritzModel::~KFritzModel() {
	// TODO Auto-generated destructor stub
}

QModelIndex KFritzModel::index(int row, int column, const QModelIndex & parent) const
{
	if (parent.isValid())
		return QModelIndex();
	else
		return createIndex(row, column);
}

QModelIndex KFritzModel::parent(const QModelIndex & child __attribute__((unused))) const
{
	// always returning QModelIndex() == 'child has no parent'; ignoring parameter
	return QModelIndex();
}

QString KFritzModel::toLocalEncoding(const std::string str) const {
	return inputCodec->toUnicode(str.c_str());
}

void KFritzModel::check() {
	if (lastRows != rowCount(QModelIndex())) {
		reset();
		lastRows = rowCount(QModelIndex());
	}
}

void KFritzModel::libReady(bool isReady) {
	if (isReady){
		inputCodec  = QTextCodec::codecForName(fritz::CharSetConv::SystemCharacterTable() ? fritz::CharSetConv::SystemCharacterTable() : "UTF-8");

		connect(timer, SIGNAL(timeout()), SLOT(check()));
		timer->start(1000);
	}
	else {
		timer->stop();
	}
}






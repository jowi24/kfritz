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

#include "KFritzModel.h"

#include <Tools.h>

KFritzModel::KFritzModel() {
	lastRows = 0;
	timer = new QTimer();
}

KFritzModel::~KFritzModel() {
	delete timer;
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
		emit updated();
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






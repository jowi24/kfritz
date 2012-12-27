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

#include "KFritzModel.h"

#include <Tools.h>

KFritzModel::KFritzModel() {
	inputCodec  = QTextCodec::codecForName(fritz::CharSetConv::SystemCharacterTable() ? fritz::CharSetConv::SystemCharacterTable() : "UTF-8");

	// create timer, that periodically calls check() in the derived classes
	// this is used, to refresh connected views in case the library has changed data in the meantime
	// (e.g., new call in call list, init of phone book)
	timer = new QTimer();
	connect(timer, SIGNAL(timeout()), SLOT(check()));
	timer->start(1000);
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

QString KFritzModel::toUnicode(const std::string str) const {
	return inputCodec->toUnicode(str.c_str());
}

std::string KFritzModel::fromUnicode(const QString str) const {
	return inputCodec->fromUnicode(str).data();
}


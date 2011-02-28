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

#include "KCalllistProxyModel.h"
#include "KCalllistModel.h"

KCalllistProxyModel::KCalllistProxyModel(QObject *parent)
: QSortFilterProxyModel(parent) {
}

KCalllistProxyModel::~KCalllistProxyModel() {
}

fritz::CallEntry *KCalllistProxyModel::retrieveCallEntry(const QModelIndex &index) const {
	return static_cast<KCalllistModel *>(sourceModel())->retrieveCallEntry(mapToSource(index));
}

std::string KCalllistProxyModel::number(const QModelIndex &index) const {
	return static_cast<KCalllistModel *>(sourceModel())->number(mapToSource(index));
}

void KCalllistProxyModel::sort(int column, Qt::SortOrder order) {
	static_cast<KCalllistModel *>(sourceModel())->sort(column, order);
}

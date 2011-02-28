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

#ifndef FILTERPROXYMODEL_H_
#define FILTERPROXYMODEL_H_

#include <QSortFilterProxyModel>
#include <CallList.h>

class KCalllistProxyModel : public QSortFilterProxyModel {
public:
	KCalllistProxyModel(QObject *parent);
	virtual ~KCalllistProxyModel();
	virtual fritz::CallEntry *retrieveCallEntry(const QModelIndex &index) const;
	virtual std::string number(const QModelIndex &index) const;
	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
};

#endif /* FILTERPROXYMODEL_H_ */

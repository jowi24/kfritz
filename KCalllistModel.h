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

#ifndef KCALLLISTMODEL_H
#define KCALLLISTMODEL_H

#include "libfritz++/CallList.h"

#include "KFritzModel.h"

class KCalllistModel : public KFritzModel {
	Q_OBJECT
public:
	KCalllistModel();
	virtual ~KCalllistModel();
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual fritz::CallEntry *retrieveCallEntry(const QModelIndex &index) const;
    virtual void sort(int column, Qt::SortOrder order);
    virtual std::string number(const QModelIndex &index) const;
    virtual std::string name(const QModelIndex &index) const;

private:
	fritz::CallList *calllist;
	time_t lastCall;
private Q_SLOTS:
	virtual void check();
};

#endif /* KCALLLISTMODEL_H_ */

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

#ifndef KFONBOOKMODEL_H_
#define KFONBOOKMODEL_H_

#include <FonbookManager.h>

#include "KFritzModel.h"

class KFonbookModel : public KFritzModel  {
	Q_OBJECT
public:
	KFonbookModel(std::string techID);
	virtual ~KFonbookModel();
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    void setDefaultType(const QModelIndex &index);
    virtual Qt::ItemFlags flags(const QModelIndex & index) const;
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    static QString getTypeName(const fritz::FonbookEntry::eType type);
    virtual std::string number(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column,
    		                  const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool insertFonbookEntry(const QModelIndex &index, fritz::FonbookEntry &fe);
    virtual const fritz::FonbookEntry *retrieveFonbookEntry(const QModelIndex &index) const;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    const fritz::Fonbook *getFonbook() const { return fonbook; }
public Q_SLOTS:
	virtual void check();
private:
	fritz::Fonbook *fonbook;
	int lastRows;
};

#endif /* KFONBOOKMODEL_H_ */

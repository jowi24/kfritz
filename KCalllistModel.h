/*
 * KCalllistModel.h
 *
 *  Created on: Dec 22, 2008
 *      Author: joachim
 */

#include <QAbstractItemModel>
#include <QTextCodec>
#include <CallList.h>

#ifndef KCALLLISTMODEL_H_
#define KCALLLISTMODEL_H_

class KCalllistModel : public QAbstractItemModel {
public:
	KCalllistModel();
	virtual ~KCalllistModel();
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column,
    		                  const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual void sort(int column, Qt::SortOrder order);
private:
	fritz::CallList *calllist;
	QTextCodec *inputCodec;
};

#endif /* KCALLLISTMODEL_H_ */

/*
 * KCalllistModel.h
 *
 *  Created on: Dec 22, 2008
 *      Author: joachim
 */

#ifndef KCALLLISTMODEL_H_
#define KCALLLISTMODEL_H_

#include <CallList.h>

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
    virtual void sort(int column, Qt::SortOrder order);

private:
	fritz::CallList *calllist;
private slots:
	void libReady(bool isReady);
};

#endif /* KCALLLISTMODEL_H_ */

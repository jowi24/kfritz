/*
 * KFonbookModel.h
 *
 *  Created on: Dec 21, 2008
 *      Author: joachim
 */

#ifndef KFONBOOKMODEL_H_
#define KFONBOOKMODEL_H_

#include <FonbookManager.h>

#include "KFritzModel.h"

class KFonbookModel : public KFritzModel  {
	Q_OBJECT
public:
	KFonbookModel();
	virtual ~KFonbookModel();
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

private:
	fritz::Fonbook *fonbook;
private slots:
	void libReady(bool isReady);
};

#endif /* KFONBOOKMODEL_H_ */

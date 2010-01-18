/*
 * QAdaptTreeView.h
 *
 *  Created on: Jan 18, 2010
 *      Author: jo
 */

#ifndef QADAPTTREEVIEW_H_
#define QADAPTTREEVIEW_H_

#include <qtreeview.h>

class QAdaptTreeView: public QTreeView {
public:
	QAdaptTreeView();
	virtual ~QAdaptTreeView();
private slots:
	void adaptColumns(const QModelIndex &topleft, const QModelIndex &bottomRight);
};

#endif /* QADAPTTREEVIEW_H_ */

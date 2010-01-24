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
	Q_OBJECT
public:
	QAdaptTreeView(QWidget *parent);
	virtual ~QAdaptTreeView();
	void adaptColumns();
private slots:
	virtual void reset();
};

#endif /* QADAPTTREEVIEW_H_ */

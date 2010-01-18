/*
 * QAdaptTreeView.cpp
 *
 *  Created on: Jan 18, 2010
 *      Author: jo
 */

#include "QAdaptTreeView.h"

QAdaptTreeView::QAdaptTreeView() {
	connect(model, SIGNAL(dataChanged(const QModelIndex &, const
	QModelIndex& ) ), this, SLOT(adaptColumns(const QModelIndex &,
	const
	               QModelIndex&) ) );

}

QAdaptTreeView::~QAdaptTreeView() {
	// TODO Auto-generated destructor stub
}

void QAdaptTreeView::adaptColumns(const QModelIndex &topleft, const QModelIndex &bottomRight) {
    int firstColumn= topleft.column();
    int lastColumn = bottomRight.column();
    // Resize the column to the size of its contents
    do {
           resizeColumnToContents(firstColumn);
           firstColumn++;
    } while (firstColumn < lastColumn);
}

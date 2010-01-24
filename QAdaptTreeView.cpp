/*
 * QAdaptTreeView.cpp
 *
 *  Created on: Jan 18, 2010
 *      Author: jo
 */

#include "QAdaptTreeView.h"

QAdaptTreeView::QAdaptTreeView(QWidget *parent)
:QTreeView(parent) {
//	connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex& )),			SLOT(adaptColumns(const QModelIndex &, const QModelIndex&)));
}

QAdaptTreeView::~QAdaptTreeView() {

}

void QAdaptTreeView::reset() {
	QTreeView::reset();
	adaptColumns();
}

void QAdaptTreeView::adaptColumns() {
    // Resize the column to the size of its contents
    for (int col=0; col < model()->columnCount(QModelIndex()); col++)
           resizeColumnToContents(col);
}

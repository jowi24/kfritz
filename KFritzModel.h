/*
 * KFritzModel.h
 *
 *  Created on: Jan 24, 2010
 *      Author: jo
 */

#ifndef KFRITZMODEL_H_
#define KFRITZMODEL_H_

#include <QAbstractItemModel>
#include <QTextCodec>
#include <QTimer>

class KFritzModel : public QAbstractItemModel {
	Q_OBJECT
public:
	KFritzModel();
	virtual ~KFritzModel();
    virtual QModelIndex index(int row, int column,
    		                  const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    QString toLocalEncoding(const std::string str) const;
protected slots:
	void check();
	void libReady(bool isReady);
private:
	QTextCodec *inputCodec;
	int lastRows;
	QTimer *timer;
};

#endif /* KFRITZMODEL_H_ */

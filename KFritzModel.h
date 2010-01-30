/*
 * KFritzBox
 *
 * Copyright (C) 2010 Joachim Wilke <vdr@joachim-wilke.de>
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
Q_SIGNALS:
	void updated();
protected Q_SLOTS:
	void check();
	void libReady(bool isReady);
private:
	QTextCodec *inputCodec;
	int lastRows;
	QTimer *timer;
};

#endif /* KFRITZMODEL_H_ */

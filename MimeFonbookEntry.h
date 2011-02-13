/*
 * KFritz
 *
 * Copyright (C) 2011 Joachim Wilke <kfritz@joachim-wilke.de>
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

#ifndef MIMEFONBOOKENTRY_H_
#define MIMEFONBOOKENTRY_H_

#include <QMimeData>
#include <QStringList>
#include <Fonbook.h>

class MimeFonbookEntry: public QMimeData {
	Q_OBJECT
private:
	fritz::FonbookEntry *fonbookEntry;
protected:
	virtual QVariant retrieveData(const QString &mimetype,
	                                      QVariant::Type preferredType) const;
public:
	MimeFonbookEntry(const fritz::FonbookEntry &fonbookEntry);
	virtual ~MimeFonbookEntry();
	virtual bool hasFormat(const QString &mimetype) const;
	virtual QStringList formats() const;
	fritz::FonbookEntry *retrieveFonbookEntry() const;
};

#endif /* MIMEFONBOOKENTRY_H_ */

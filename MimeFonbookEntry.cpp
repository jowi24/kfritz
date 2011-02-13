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

#include "MimeFonbookEntry.h"

MimeFonbookEntry::MimeFonbookEntry(const fritz::FonbookEntry &fonbookEntry) {
	this->fonbookEntry = new fritz::FonbookEntry(fonbookEntry);
}

MimeFonbookEntry::~MimeFonbookEntry() {
	delete fonbookEntry;
}

QVariant MimeFonbookEntry::retrieveData(const QString &mimetype, QVariant::Type preferredType) const {
	return QVariant();
}

bool MimeFonbookEntry::hasFormat(const QString &mimetype) const {
	if (mimetype.compare("application/x-kfritz-fonbookentry") == 0) {
		return true;
	} else {
		return false;
	}
}

QStringList MimeFonbookEntry::formats() const {
	QStringList list("application/x-kfritz-fonbookentry");
	return list;
}

fritz::FonbookEntry *MimeFonbookEntry::retrieveFonbookEntry() const {
	return fonbookEntry;
}


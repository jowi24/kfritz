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

#include "KCalllistModel.h"

#include <KIcon>
#include <KLocalizedString>

KCalllistModel::KCalllistModel() {
	calllist = NULL;
}

KCalllistModel::~KCalllistModel() {
	// TODO: somthing to do here?
}

QVariant KCalllistModel::data(const QModelIndex & index, int role) const {
	if (!calllist)
		return QVariant();
	fritz::CallEntry *ce = calllist->RetrieveEntry(fritz::CallEntry::ALL,index.row());
	if (role == Qt::DecorationRole && index.column() == 0)
		return QVariant(KIcon(ce->type == fritz::CallEntry::INCOMING ? "incoming-call" :
				              ce->type == fritz::CallEntry::OUTGOING ? "outgoing-call" :
				              ce->type == fritz::CallEntry::MISSED   ? "missed-call"   : ""));
	if (role == Qt::ToolTipRole && index.column() == 0)
		return QVariant(i18n(ce->type == fritz::CallEntry::INCOMING ? "Incoming call" :
				             ce->type == fritz::CallEntry::OUTGOING ? "Outgoing call" :
				             ce->type == fritz::CallEntry::MISSED   ? "Missed call"   : ""));
	if (role != Qt::DisplayRole)
		return QVariant();


	switch (index.column()) {
		case 1:
			return QVariant(toLocalEncoding((ce->date+" "+ce->time)));
			break;
		case 2:
			if (ce->remoteName.size() == 0)
				if (ce->remoteNumber.size() == 0)
					return QVariant("unknown");
				else
					return QVariant(toLocalEncoding(ce->remoteNumber));
			else
				return QVariant(toLocalEncoding(ce->remoteName));
			break;
		case 3:
			return QVariant(toLocalEncoding(ce->localName));
			break;
		case 4:
			return QVariant(toLocalEncoding(ce->duration));
			break;
		default:
			return QVariant();
	}
	return QVariant();
}

QVariant KCalllistModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role != Qt::DisplayRole)
		return QVariant();
	if (orientation == Qt::Horizontal){
		switch (section) {
		case 0:
			return "Type";
			break;
		case 1:
			return "Date";
			break;
		case 2:
			return "Name / Number";
			break;
		case 3:
			return "Local Device";
			break;
		case 4:
			return "Duration";
			break;
		default:
			return QVariant();
		}
	} else {
		return QVariant();
	}
}

int KCalllistModel::columnCount(const QModelIndex & parent __attribute__((unused))) const {
	// the number of columns is independent of the current parent, ignoring this parameter
	return 5;
}

int KCalllistModel::rowCount(const QModelIndex & parent) const {
	if (!calllist)
		return 0;
	if (parent.isValid())
		// the model does not have any hierarchy
		return 0;
	else
		return calllist->GetSize(fritz::CallEntry::ALL);
}

void KCalllistModel::sort(int column, Qt::SortOrder order) {
	if (!calllist)
		return;
	fritz::CallEntry::eElements element;
	switch (column) {
	case 0:
		element = fritz::CallEntry::ELEM_TYPE;
		break;
	case 1:
		element = fritz::CallEntry::ELEM_DATE;
		break;
	case 2:
		element = fritz::CallEntry::ELEM_REMOTENAME; //TODO: not 100% correct
		break;
	case 3:
		element = fritz::CallEntry::ELEM_LOCALNAME; //TODO: not 100% correct
		break;
	case 4:
		element = fritz::CallEntry::ELEM_DURATION;
		break;
	default:
		std::cout << __FILE__ << "invalid column to sort for in KCalllistModel" << std::endl;
		return;
		break;
	}
	calllist->Sort(element, order == Qt::AscendingOrder);
	emit dataChanged(index(0,                       0,                          QModelIndex()),
			index(rowCount(QModelIndex()), columnCount(QModelIndex()), QModelIndex()));
}

void KCalllistModel::libReady(bool isReady) {
	KFritzModel::libReady(isReady);
	if(isReady){
		// get the calllist resource
		calllist = fritz::CallList::getCallList(false);
	}
	else
		calllist = NULL;
}

void KCalllistModel::check() {
	if (lastCall != calllist->LastCall()) {
		reset();
		emit updated();
		lastCall = calllist->LastCall();
	}
}

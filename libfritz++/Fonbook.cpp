/*
 * libfritz++
 *
 * Copyright (C) 2007-2010 Joachim Wilke <libfritz@joachim-wilke.de>
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

#include <algorithm>
#include "Fonbook.h"
#include "Tools.h"
#include "Config.h"

namespace fritz{

FonbookEntry::FonbookEntry(std::string name, std::string number, eType type) {
	this->name   = name;
	this->number = number;
	this->type   = type;
}

bool FonbookEntry::operator<(const FonbookEntry &fe) const {
	int cresult = this->name.compare(fe.name);
	if (cresult == 0)
		return (this->type < fe.type);
	return (cresult < 0);
}

class FonbookEntrySort {
private:
	bool ascending;
	FonbookEntry::eElements element;
public:
	FonbookEntrySort(FonbookEntry::eElements element = FonbookEntry::ELEM_NAME, bool ascending = true) {
		this->element   = element;
		this->ascending = ascending;
	}
	bool operator() (FonbookEntry fe1, FonbookEntry fe2){
		switch(element) {
		case FonbookEntry::ELEM_NAME:
			return (ascending ? (fe1.getName() < fe2.getName()) : (fe1.getName() > fe2.getName()));
			break;
		case FonbookEntry::ELEM_TYPE:
			return (ascending ? (fe1.getType() < fe2.getType()) : (fe1.getType() > fe2.getType()));
			break;
		case FonbookEntry::ELEM_NUMBER:
			return (ascending ? (fe1.getNumber() < fe2.getNumber()) : (fe1.getNumber() > fe2.getNumber()));
			break;
		default:
			ERR("invalid element given for sorting.");
			return false;
		}
	}
};

Fonbook::Fonbook()
{
	title       = "Phonebook";
	techId      = "BASE";
	displayable = false;
	initialized = false;
	writeable   = false;
}

FonbookEntry &Fonbook::ResolveToName(FonbookEntry &fe) {
	for (unsigned int pos=0; pos < fonbookList.size(); pos++) {
		if (Tools::CompareNormalized(fe.getNumber(), fonbookList[pos].getNumber()) == 0) {
			fe.setName(fonbookList[pos].getName());
			fe.setType(fonbookList[pos].getType());
			return fe;
		}
	}
	fe.setName(fe.getNumber());
	fe.setType(FonbookEntry::TYPE_NONE);
	return fe;
}

FonbookEntry *Fonbook::RetrieveFonbookEntry(size_t id) {
	if (id >= GetFonbookSize())
		return NULL;
	return &fonbookList[id];
}

size_t Fonbook::GetFonbookSize() {
	return fonbookList.size();
}

void Fonbook::Sort(FonbookEntry::eElements element, bool ascending) {
	FonbookEntrySort fes(element, ascending);
	std::sort(fonbookList.begin(), fonbookList.end(), fes);
}

}

/*
 * libfritz++
 *
 * Copyright (C) 2007-2008 Joachim Wilke <vdr@joachim-wilke.de>
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


#include "Fonbooks.h"

namespace fritz{

Fonbooks::Fonbooks()
{
}

Fonbooks::~Fonbooks()
{
}

Fonbook *Fonbooks::operator[](std::string key) {
	for (size_t i=0; i<this->size(); i++) {
		if ((*this)[i]->GetTechId() == key) {
			return ((*this)[i]);
		}
	}
	return NULL;
}

Fonbook *Fonbooks::operator[](size_t i) {
	return (*((std::vector<Fonbook*>*) this))[i];
}

}


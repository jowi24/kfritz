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

#ifndef XMLFONBOOK_H_
#define XMLFONBOOK_H_

#include "Fonbook.h"

namespace fritz {

class XmlFonbook: public fritz::Fonbook {
private:
	std::string ExtractXmlAttributeValue(std::string element, std::string attribute, std::string xml);
	std::string ExtractXmlElementValue(std::string element, std::string xml);
	std::string charset;
protected:
	std::string convertEntities(std::string s);
	void ParseXmlFonbook(std::string *msg);
	std::string SerializeToXml();
public:
	XmlFonbook();
	virtual ~XmlFonbook();
};

}

#endif /* XMLFONBOOK_H_ */

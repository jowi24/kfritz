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

#include "XmlFonbook.h"

#include <string.h>
#include <stdlib.h>

#include "Config.h"
#include "Tools.h"

namespace fritz {

const char *Entities[97][2] = {
	{"&nbsp;",  " "},
	{"&iexcl;", "¡"},
	{"&cent;",  "¢"},
	{"&pound;", "£"},
	{"&curren;","€"},
	{"&yen;",   "¥"},
	{"&brvbar;","Š"},
	{"&sect;",  "§"},
	{"&uml;",   "š"},
	{"&copy;",  "©"},
	{"&ordf;",  "ª"},
	{"&laquo;", "«"},
	{"&not;",   "¬"},
	{"&shy;",   "­"},
	{"&reg;",   "®"},
	{"&macr;",  "¯"},
	{"&deg;",   "°"},
	{"&plusmn;","±"},
	{"&sup2;",  "²"},
	{"&sup3;",  "³"},
	{"&acute;", "Ž"},
	{"&micro;", "µ"},
	{"&para;",  "¶"},
	{"&middot;","·"},
	{"&cedil;", "ž"},
	{"&sup1;",  "¹"},
	{"&ordm;",  "º"},
	{"&raquo;", "»"},
	{"&frac14;","Œ"},
	{"&frac12;","œ"},
	{"&frac34;","Ÿ"},
	{"&iquest;","¿"},
	{"&Agrave;","À"},
	{"&Aacute;","Á"},
	{"&Acirc;", "Â"},
	{"&Atilde;","Ã"},
	{"&Auml;",  "Ä"},
	{"&Aring;", "Å"},
	{"&AElig;", "Æ"},
	{"&Ccedil;","Ç"},
	{"&Egrave;","È"},
	{"&Eacute;","É"},
	{"&Ecirc;", "Ê"},
	{"&Euml;",  "Ë"},
	{"&Igrave;","Ì"},
	{"&Iacute;","Í"},
	{"&Icirc;", "Î"},
	{"&Iuml;",  "Ï"},
	{"&ETH;",   "Ð"},
	{"&Ntilde;","Ñ"},
	{"&Ograve;","Ò"},
	{"&Oacute;","Ó"},
	{"&Ocirc;", "Ô"},
	{"&Otilde;","Õ"},
	{"&Ouml;",  "Ö"},
	{"&times;", "×"},
	{"&Oslash;","Ø"},
	{"&Ugrave;","Ù"},
	{"&Uacute;","Ú"},
	{"&Ucirc;", "Û"},
	{"&Uuml;",  "Ü"},
	{"&Yacute;","Ý"},
	{"&THORN;", "Þ"},
	{"&szlig;", "ß"},
	{"&agrave;","à"},
	{"&aacute;","á"},
	{"&acirc;", "â"},
	{"&atilde;","ã"},
	{"&auml;",  "ä"},
	{"&aring;", "å"},
	{"&aelig;", "æ"},
	{"&ccedil;","ç"},
	{"&egrave;","è"},
	{"&eacute;","é"},
	{"&ecirc;", "ê"},
	{"&euml;",  "ë"},
	{"&igrave;","ì"},
	{"&iacute;","í"},
	{"&icirc;", "î"},
	{"&iuml;",  "ï"},
	{"&eth;",   "ð"},
	{"&ntilde;","ñ"},
	{"&ograve;","ò"},
	{"&oacute;","ó"},
	{"&ocirc;", "ô"},
	{"&otilde;","õ"},
	{"&ouml;",  "ö"},
	{"&divide;","÷"},
	{"&oslash;","ø"},
	{"&ugrave;","ù"},
	{"&uacute;","ú"},
	{"&ucirc;", "û"},
	{"&uuml;",  "ü"},
	{"&yacute;","ý"},
	{"&thorn;", "þ"},
	{"&yuml;",  "ÿ"},
	{"&amp;",   "&"},
};

std::string XmlFonbook::convertEntities(std::string s) {
	if (s.find("&") != std::string::npos) {
		// convert the entities from UTF-8 to current system character table
		CharSetConv *conv = new CharSetConv("UTF-8", CharSetConv::SystemCharacterTable());

		for (int i=0; i<97; i++) {
			std::string::size_type pos = s.find(Entities[i][0]);
			if (pos != std::string::npos) {
				s.replace(pos, strlen(Entities[i][0]), conv->Convert(Entities[i][1]));
				i--; //search for the same entity again
			}
		}
		delete (conv);
	}
	return s;
}


XmlFonbook::XmlFonbook() {
	charset = CharSetConv::SystemCharacterTable() ? CharSetConv::SystemCharacterTable() : "UTF-8";
}

XmlFonbook::~XmlFonbook() {
}


std::string XmlFonbook::ExtractXmlAttributeValue(std::string element, std::string attribute, std::string xml) {
	size_t posStart = xml.find("<"+element);
	if (posStart != std::string::npos) {
		posStart = xml.find(attribute+"=\"", posStart);
		if (posStart != std::string::npos) {
			size_t posEnd = xml.find("\"", posStart + attribute.length() + 2);
			if (posEnd != std::string::npos)
				return xml.substr(posStart + attribute.length() + 2, posEnd - posStart - attribute.length() - 2);
		}
	}
	return "";
}

std::string XmlFonbook::ExtractXmlElementValue(std::string element, std::string xml) {
	size_t posStart = xml.find("<"+element);
	if (posStart != std::string::npos) {
		posStart = xml.find(">", posStart);
		size_t posEnd   = xml.find("</"+element+">");
		if (posEnd != std::string::npos)
			return xml.substr(posStart + 1, posEnd - posStart - 1);
	}
	return "";
}

void XmlFonbook::ParseXmlFonbook(std::string *msg) {
	DBG("Parsing fonbook using xml parser.")
	// determine charset
	size_t pos, posStart, posEnd;
	posStart = msg->find("encoding=\"");
	if (posStart != std::string::npos) {
		posEnd = msg->find("\"", posStart + 10);
		if (posEnd != std::string::npos)
			charset = msg->substr(posStart + 10, posEnd - posStart - 10);
	}
	DBG("using charset " << charset);

	CharSetConv *conv = new CharSetConv(charset.c_str(), CharSetConv::SystemCharacterTable());
	const char *s_converted = conv->Convert(msg->c_str());
	std::string msgConv = s_converted;
	delete (conv);

	pos = msgConv.find("<contact>");
	while (pos != std::string::npos) {
		std::string msgPart = msgConv.substr(pos, msgConv.find("</contact>", pos) - pos + 10);
		std::string category = ExtractXmlElementValue("category", msgPart);
		std::string name     = convertEntities(ExtractXmlElementValue("realName", msgPart));
		FonbookEntry fe(name, category == "1");
		size_t posNumber = msgPart.find("<number");
		while (posNumber != std::string::npos) {
			std::string msgPartofPart = msgPart.substr(posNumber, msgPart.find("</number>", posNumber) - posNumber + 9);
			std::string number    = ExtractXmlElementValue  ("number",              msgPartofPart);
			std::string typeStr   = ExtractXmlAttributeValue("number", "type",      msgPartofPart);
			std::string quickdial = ExtractXmlAttributeValue("number", "quickdial", msgPartofPart);
			std::string vanity    = ExtractXmlAttributeValue("number", "vanity",    msgPartofPart);
			std::string prio      = ExtractXmlAttributeValue("number", "prio",      msgPartofPart);

			if (number.size()) { // the xml may contain entries without a number!
				FonbookEntry::eType type = FonbookEntry::TYPE_NONE;
				if (typeStr == "home")
					type = FonbookEntry::TYPE_HOME;
				if (typeStr == "mobile")
					type = FonbookEntry::TYPE_MOBILE;
				if (typeStr == "work")
					type = FonbookEntry::TYPE_WORK;

				fe.addNumber(number, type, quickdial, vanity, atoi(prio.c_str()));
			}
			posNumber = msgPart.find("<number", posNumber+1);
		}
		fonbookList.push_back(fe);
		pos = msgConv.find("<contact>", pos+1);
	}
	INF("read " << fonbookList.size() << " entries.");
}

std::string XmlFonbook::SerializeToXml() {

	std::stringstream result;
	result << "<?xml version=\"1.0\" encoding=\"" << charset << "\"?>"
			  "<phonebooks>"
			  "<phonebook>";
	for (size_t i = 0; i < fonbookList.size(); i++) {
		FonbookEntry &fe = fonbookList[i];
		result << "<contact>"
			   << "<category>" << (fe.isImportant() ? "1" : "0") << "</category>"
			   << "<person>"
		       << "<realName>" << fe.getName() << "</realName>"
		       << "</person>"
		       << "<telephony>";
		for (int type = 0; type < FonbookEntry::TYPES_COUNT; type++)
			if (fe.getNumber((fritz::FonbookEntry::eType) type).length() > 0) {
				std::string typeName = "";
				switch (type) {
				case FonbookEntry::TYPE_NONE:
				case FonbookEntry::TYPE_HOME:
					typeName="home";
					break;
				case FonbookEntry::TYPE_MOBILE:
					typeName="mobile";
					break;
				case FonbookEntry::TYPE_WORK:
					typeName="work";
					break;
				}
				result << "<number type=\"" << typeName << "\" "
						          "quickdial=\"" << fe.getQuickdial((fritz::FonbookEntry::eType) type) << "\" "
						          "vanity=\""    << fe.getVanity((fritz::FonbookEntry::eType) type)    << "\" "
						          "prio=\""      << fe.getPriority((fritz::FonbookEntry::eType) type)  << "\">"
				       << fe.getNumber((fritz::FonbookEntry::eType) type)
				       << "</number>";
			}

		result << "</telephony>"
			   << "<services/>"
               << "<setup/>"
               << "</contact>";
	}
	result << "</phonebook>"
			  "</phonebooks>";

	CharSetConv *conv = new CharSetConv(CharSetConv::SystemCharacterTable(), charset.c_str());
	const char *result_converted = conv->Convert(result.str().c_str());
	std::string xmlData = result_converted;
	delete (conv);

	return xmlData;
}

}

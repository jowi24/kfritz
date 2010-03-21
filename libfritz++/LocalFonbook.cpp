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

#include <string.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "Tools.h"
#include "LocalFonbook.h"
#include "Config.h"

namespace fritz {

class ReadLine {
private:
  size_t size;
  char *buffer;
public:
  ReadLine(void);
  ~ReadLine();
  char *Read(FILE *f);
  };

ReadLine::ReadLine(void)
{
  size = 0;
  buffer = NULL;
}

ReadLine::~ReadLine()
{
  free(buffer);
}

char *ReadLine::Read(FILE *f)
{
  int n = getline(&buffer, &size, f);
  if (n > 0) {
     n--;
     if (buffer[n] == '\n') {
        buffer[n] = 0;
        if (n > 0) {
           n--;
           if (buffer[n] == '\r')
              buffer[n] = 0;
           }
        }
     return buffer;
     }
  return NULL;
}


LocalFonbook::LocalFonbook() {
	title       = I18N_NOOP("Local phone book");
	techId      = "LOCL";
	displayable = true;
	writeable   = true;
	filePath    = NULL;
}

bool LocalFonbook::Initialize() {
	setInitialized(false);
	fonbookList.clear();

	// first, try xml phonebook
	int ret = asprintf(&filePath, "%s/localphonebook.xml", gConfig->getConfigDir().c_str());
	if (ret <= 0)
		return false;
	if (access(filePath, F_OK) == 0) {
		INF("loading " << filePath);
		std::ifstream file(filePath);
		if (!file.good())
			return false;
		std::string xmlData((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
		ParseXmlFonbook(&xmlData);
		return true;
	} else
		DBG("XML phonebook not found, trying old csv based ones.");

	// try deprecated filenames
	free(filePath);
	filePath = NULL;

	char fileNames[3][20] = {"localphonebook.csv", "localfonbook.csv", "localfonbuch.csv"};
	for (size_t pos = 0; pos < 3; pos++) {
		int ret = asprintf(&filePath, "%s/%s", gConfig->getConfigDir().c_str(), fileNames[pos]);
		if (ret <= 0)
			return false;
		if (access(filePath, F_OK) == 0) {
			if (pos > 0)
				INF("warning, using deprecated file " << filePath << ", please rename to " << fileNames[0] << ".");
			break;
		}
		free(filePath);
		filePath = NULL;
	}
	if (filePath) {
		ParseCsvFonbook(filePath);
		free(filePath);
		// convert to xml when saving
		int res = asprintf(&filePath, "%s/localphonebook.xml", gConfig->getConfigDir().c_str());
		if (res <= 0)
			return false;
		return true;
	} else {
		// file not available -> log preferred filename and location
		ERR("file " << gConfig->getConfigDir().c_str() << "/" << fileNames[0] << " not found.");
		// if no file exists, put the preferred name into filepath (for later usage)
		// convert to xml when saving
		int res = asprintf(&filePath, "%s/localphonebook.xml", gConfig->getConfigDir().c_str());
		if (res <= 0)
			return false;
		return false;
	}
	return false;
}

void LocalFonbook::Reload() {
	Initialize();
}

bool LocalFonbook::AddFonbookEntry(FonbookEntry fe) {
	fonbookList.push_back(fe);
	return true;
}

void LocalFonbook::ParseCsvFonbook(std::string filePath) {
	INF("loading " << filePath);
	FILE *f = fopen(filePath.c_str(), "r");
	if (f) {
		char *s;
		ReadLine ReadLine;
		while ((s = ReadLine.Read(f)) != NULL) {
			if (s[0] == '#') continue;
			char* name_buffer 	= strtok(s, ",;");
			char* type_buffer 	= strtok(NULL, ",;");
			char* number_buffer = strtok(NULL, ",;");
			if (name_buffer && type_buffer && number_buffer) {
				std::string name   		 	= name_buffer;
				FonbookEntry::eType type   = (FonbookEntry::eType) atoi(type_buffer);
				std::string number 			= number_buffer;
				// search for existing fe
				bool feExists = false;
				for (size_t feNr = 0; feNr < fonbookList.size(); feNr++)
					if (fonbookList[feNr].getName() == name) {
						fonbookList[feNr].addNumber(number, type); //TODO: quickdial, vanity and priority not supported here
						feExists = true;
					}
				// add to new fe
				if (!feExists) {
					FonbookEntry fe(name, false); //TODO: important not supported here
					fe.addNumber(number, type);
					AddFonbookEntry(fe);
				}
			}
			else {
				ERR("parse error at " << s);
			}
		}
		setInitialized(true);
		INF("read " << fonbookList.size() << " entries.");
		std::sort(fonbookList.begin(), fonbookList.end());
		fclose(f);
	}
}

void LocalFonbook::Save() {
	DBG("Saving to " << filePath << ".");
	// filePath should always contain a valid content, this is just to be sure
	if (!filePath)
		return;
	// open file
	std::ofstream file(filePath, std::ios_base::trunc);
	if (file.fail())
		return;
	// write all entries to the file
	file << SerializeToXml();
	// close file
	file.close();
	DBG("Saving successful.");
}

}



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


#include <string.h>
#include <algorithm>
#include "Tools.h"
#include "LocalFonbook.h"
#include "Config.h"

namespace fritz{

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
	title = "Local phone book";
	techId = "LOCL";
	displayable = true;
}

LocalFonbook::~LocalFonbook() {
}

bool LocalFonbook::Initialize() {
	setInitialized(false);
	fonbookList.clear();
	char* fileName;
	int ret = asprintf(&fileName, "%s/__FILE__sv", gConfig->getConfigDir().c_str());
	if (ret <= 0)
		return false;
	if (fileName && access(fileName, F_OK) == 0) {
		*isyslog << "loading " << fileName << std::endl;
		FILE *f = fopen(fileName, "r");
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
					FonbookEntry fe(name, number, type);
					fonbookList.push_back(fe);
				}
				else {
					*esyslog << __FILE__ << ": parse error at " << s << std::endl;
				}
			}
			setInitialized(true);
			*isyslog << __FILE__ << ": read " << fonbookList.size() << " entries." << std::endl;
			std::sort(fonbookList.begin(), fonbookList.end());
			return true;
		}
	}
	return false;
}

void LocalFonbook::Reload() {
	Initialize();
}

}



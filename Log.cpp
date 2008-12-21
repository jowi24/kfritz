/*
 * KFritzBox
 *
 * Copyright (C) 2008 Joachim Wilke <vdr@joachim-wilke.de>
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


#include "Log.h"

LogBuf::LogBuf(eLogType type, KTextEdit *widget) {
	const unsigned int BUFFER_SIZE = 1024;
	char	*ptr = new char[BUFFER_SIZE];
	setp(ptr, ptr + BUFFER_SIZE);
	setg(0, 0, 0);
	this->textedit = te;
	this->type = type;
	this->widget = widget;
	connect(this, SIGNAL(signalAppend(QString)), this, SLOT(slotAppend(QString)));
}

void LogBuf::slotAppend(QString m) {
	widget->append(m);
}


void LogBuf::PutBuffer(void)
{
	if (pbase() != pptr())
	{
		int     len = (pptr() - pbase());
		char    *buffer = new char[len + 1];

		strncpy(buffer, pbase(), len);
		buffer[len] = 0;

		switch (type) {
		case INFO:
			emit signalAppend(buffer);
			break;
		case ERROR:
			emit signalAppend(buffer);
			break;
		case DEBUG:
			emit signalAppend(buffer);
			break;
		}

		setp(pbase(), epptr());
		delete [] buffer;
	}
}

int LogBuf::overflow(int c)
{
	PutBuffer();

	if (c != EOF) {
			sputc(c);
	}
	return 0;

}

int LogBuf::sync()
{
	PutBuffer();
	return 0;
}

LogBuf::~LogBuf() {
	sync();
	delete[] pbase();
}

LogStream::LogStream(LogBuf::eLogType type, KTextEdit *te)
:std::ostream(new LogBuf(type, te))
{
}



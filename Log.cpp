/*
 * KFritz
 *
 * Copyright (C) 2010 Joachim Wilke <kfritz@joachim-wilke.de>
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
#include <cstdio>
#include <iostream>

LogBuf::LogBuf(eLogType type) {
	const unsigned int BUFFER_SIZE = 1024;
	char	*ptr = new char[BUFFER_SIZE];
	setp(ptr, ptr + BUFFER_SIZE);
	setg(0, 0, 0);
	this->type = type;
	logWidget = NULL;
	connect(this, SIGNAL(signalAppend(QString)), this, SLOT(slotAppend(QString)));
}

void LogBuf::setLogWidget(KTextEdit *te) {
	logWidget = te;
}


void LogBuf::slotAppend(QString m) {
	if (logWidget)
		logWidget->insertPlainText(m);
	std::cout << m.toStdString();
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
		default:
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

LogStream::LogStream(LogBuf::eLogType type)
:std::ostream(buffer = new LogBuf(type))
{
}

LogStream *LogStream::setLogWidget(KTextEdit *te) {
	buffer->setLogWidget(te);
	return this;
}

LogStream *LogStream::getLogStream(LogBuf::eLogType type) {
	if (!streams[type])
		streams[type] = new LogStream(type);
	return streams[type];
}

LogStream *LogStream::streams[];



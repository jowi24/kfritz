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

#include <iostream>
#include <KTextEdit>

#ifndef LOG_H_
#define LOG_H_

class LogBuf : public QObject, public std::streambuf {
	Q_OBJECT

public:
	enum eLogType {
		DEBUG,
		INFO,
		ERROR,
	};
private:
	void	PutBuffer(void);
	void	PutChar(char c);
	eLogType type;
	KTextEdit *widget;
protected:
	int	overflow(int);
	int	sync();
public:
	LogBuf(eLogType type, KTextEdit *te);
	virtual ~LogBuf();
signals:
	void signalAppend(QString m);
private slots:
	void slotAppend(QString m);
};

class LogStream: public std::ostream {
public:
	LogStream(LogBuf::eLogType type, KTextEdit *te);
	//virtual ~LogStream();
};

#endif /* LOG_H_ */

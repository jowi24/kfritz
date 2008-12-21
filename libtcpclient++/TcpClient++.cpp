/*
 * libtcpclient++
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/poll.h>

#include <string>
#include <sstream>

#include <fstream>

#include "TcpClient++.h"
#include <PThread++.h>

using namespace tcpclient;

// ----- TraceFile ------------------------------------------------------------

class TraceFile {
private:
	std::fstream *trace;
	static TraceFile *traceFile;
	static pthread::Mutex *mutex;
	pthread::tThreadId lastTid;
	bool lastWasWrite;
	TraceFile();
public:
	static TraceFile &getTraceFile();
	virtual ~TraceFile();
	void write(std::string hostname, unsigned int port, bool isWrite, std::string msg);
};

TraceFile *TraceFile::traceFile = NULL;
pthread::Mutex *TraceFile::mutex = new pthread::Mutex();

TraceFile::TraceFile() {
	// libtcpclient's tracing function init
	// this lib writes all socket communication to file /tmp/tcpclient.trace if this file exists
	// to enable tracing, create this file by running 'touch /tmp/tcpclient.trace'
	trace = NULL;
	std::fstream ftest;
	ftest.open("/tmp/tcpclient.trace", std::ios_base::in);
	if (!ftest.fail()) {
		ftest.close();
		trace = new std::fstream();
		trace->open("/tmp/tcpclient.trace", std::ios_base::out | std::ios_base::app);
	}
	lastWasWrite = false;
	lastTid = 0;
}

TraceFile::~TraceFile() {
	if (trace) {
		trace->close();
		delete trace;
	}
}

TraceFile &TraceFile::getTraceFile() {
	mutex->Lock();
	if (traceFile == NULL)
		traceFile = new TraceFile();
	mutex->Unlock();
	return *traceFile;
}

void TraceFile::write(std::string hostname, unsigned int port, bool isWrite, std::string msg) {
	if (trace) {
		mutex->Lock();
		if (isWrite == lastWasWrite && pthread::PThread::ThreadId() == lastTid) {
			*trace << msg << std::flush;
		} else {
			lastTid = pthread::PThread::ThreadId();
			lastWasWrite = isWrite;
			*trace << "\n[" << (isWrite ? "<-" : "->") << hostname << ":" << port << "]\n" << msg << std::flush;
		}
		mutex->Unlock();
	}
}


// ----- TcpClient ------------------------------------------------------------

TcpClient::TcpClient(std::string &hostname, int port) {
	connected = false;
	this->hostname = hostname;
	this->port = port;
	fd = -1;
}

TcpClient::~TcpClient() {
        if (fd >= 0) {
	        shutdown(fd, SHUT_RDWR);
	        close(fd);
	}
}

void TcpClient::Connect() {
	if (connected) {
		close(fd);
		connected = false;
	}

	struct addrinfo *ainfo;

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int res;

	res = getaddrinfo(hostname.c_str(), NULL, &hints, &ainfo);
	if (res != 0) {
		throw TcpException(TcpException::ERR_HOST_NOT_RESOLVABLE);
	}

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		throw TcpException(TcpException::ERR_SOCKET_CREATE);
	((sockaddr_in*)(ainfo->ai_addr))->sin_port = htons(port);
	((sockaddr_in*)(ainfo->ai_addr))->sin_family = AF_INET;

	res = connect(fd, ainfo->ai_addr, sizeof(struct sockaddr));
	if (res < 0) {
		int errCode = errno;
		close(fd);
		freeaddrinfo(ainfo);
		switch (errCode) {
		case EALREADY:
			throw TcpException(TcpException::ERR_ALREADY_TRYING);
		case ECONNREFUSED:
			throw TcpException(TcpException::ERR_CONNECTION_REFUSED);
		case EHOSTUNREACH:
		case ENETUNREACH:
		case ENETDOWN:
			throw TcpException(TcpException::ERR_HOST_NOT_REACHABLE);
		case EINVAL:
			throw TcpException(TcpException::ERR_INVALID_ARGUMENT);
		case EOPNOTSUPP:
			throw TcpException(TcpException::ERR_OPERATION_NOT_SUPPORTED);
		case EPROTOTYPE:
			throw TcpException(TcpException::ERR_PROTOTYPE);
		case EISCONN:
			throw TcpException(TcpException::ERR_ALREADY_CONNECTED);
		case ENOBUFS:
			throw TcpException(TcpException::ERR_OUT_OF_MEMORY);
		case ETIMEDOUT:
			throw TcpException(TcpException::ERR_TIMEOUT);
		default:
			throw TcpException(TcpException::ERR_UNKNOWN);
		}

	}
	freeaddrinfo(ainfo);
	fcntl(fd, F_SETFL, O_NONBLOCK);
	connected = true;
}

void TcpClient::Disconnect() {
	if (connected) {
		close(fd);
		connected = false;
	}
}

// ----- Stuff for reading from socket ----------------------------------------

std::string TcpClient::Receive() {
	int size = 0;
	std::string result;
	do {
		struct pollfd fds[1];
		fds[0].fd = fd;
		fds[0].events = POLLIN | POLLPRI;
		poll(&(fds[0]), 1, 500);
		size = recv(fd, buffer, TCP_BUF_SIZE - 1, 0);
	} while (size == -1 && errno == EAGAIN);
	if (size == 0) 	{
		// connection was orderly closed
		connected = false;
		close(fd);
	} else if (size == -1) {
		// there occured an error
		connected = false;
		close(fd);
		throw TcpException(TcpException::ERR_SOCKET_ERROR);
	}
	buffer[size] = '\0';
	result = buffer;
	// trace socket readings
	TraceFile::getTraceFile().write(hostname, port, false, result);
	return result;
}

std::string TcpClient::Read() {
	if (!connected)
		Connect();
	return Receive();
}

TcpClient &TcpClient::operator>>(std::ostringstream &ss) {
	ss << Read();
	return *this;
}

TcpClient &TcpClient::operator>>(std::string &s) {
	s += Read();
	return *this;
}

// ----- Stuff for writing to socket ------------------------------------------

void TcpClient::Write(std::string s) {
	if (!connected)
		Connect();
	int size = send(fd, s.c_str(), s.length(), 0);
	if (size == -1) {
		connected = false;
		close(fd);
		throw TcpException(TcpException::ERR_CONNECTION_RESET);
	}
	// Trace socket writings
	TraceFile::getTraceFile().write(hostname, port, true, s);
}

TcpClient &TcpClient::operator<<(std::string s) {
	Write(s);
	return *this;
}

TcpClient &TcpClient::operator<<(const char c[]) {
	Write(std::string(c));
	return *this;
}

TcpClient &TcpClient::operator<<(int i) {
	std::ostringstream sData;
	sData << i;
	Write(sData.str());
	return *this;
}

// ----- HttpClient ----------------------------------------------------------

std::string HttpClient::Read() {
	std::string result;
	unsigned int length = 0;
	if (!connected)
		Connect();
	do {
		length = result.length();
		result += Receive();
	} while (result.length() > length);
	return result;
}

HttpClient::HttpClient(std::string &hostname, int port)
:TcpClient(hostname, port) {

}


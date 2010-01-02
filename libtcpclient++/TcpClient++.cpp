/*
 * libtcpclient++
 *
 * Copyright (C) 2007-2009 Joachim Wilke <vdr@joachim-wilke.de>
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
	std::fstream          *trace;
	static TraceFile      *traceFile;
	static pthread::Mutex *mutex;
	pthread::tThreadId    lastTid;
	bool                  lastWasWrite;
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
	lastTid      = 0;
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


// ----- TcpClient/TcpClientBuf -----------------------------------------------

TcpClientBuf::TcpClientBuf(std::string hostname, int port) throw(tcpclient::TcpException) {
	// init controlled output sequence to have space for BUF_SIZE characters
	setp(outputBuffer, outputBuffer + BUF_SIZE);
	// init controlled input sequence to have no data available
	setg(0, 0, 0);

	connected      = false;
	this->hostname = hostname;
	this->port     = port;
	fd             = -1;

	Connect();
}

TcpClientBuf::~TcpClientBuf() {
	Disconnect();
}

TcpClient::~TcpClient() {
	delete ((TcpClientBuf *)rdbuf());
}

void TcpClientBuf::Connect() {
	if (connected) {
		close(fd);
		connected = false;
	}

	struct addrinfo *ainfo;

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_INET;
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
	((sockaddr_in*)(ainfo->ai_addr))->sin_port   = htons(port);
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
	freeaddrinfo(ainfo); //TODO: memleak in case of exception
	fcntl(fd, F_SETFL, O_NONBLOCK);
	connected = true;
}

void TcpClientBuf::Disconnect() {
	if (connected) {
		shutdown(fd, SHUT_RDWR);
		close(fd);
		fd = -1;
		connected = false;
	}
}

// ----- Stuff for reading from socket ----------------------------------------

bool TcpClientBuf::Receive() {
	if (!connected)
		Connect();
	int size = 0;
	do {
		struct pollfd fds[1];
		fds[0].fd = fd;
		fds[0].events = POLLIN | POLLPRI;
		poll(&(fds[0]), 1, 500);
		size = recv(fd, inputBuffer, BUF_SIZE, 0);
	} while (size == -1 && errno == EAGAIN);
	if (size <= 0) 	{
		// connection was closed
		setg(0, 0, 0);
		connected = false;
		if (fd >= 0)
			close(fd);
		if (size == -1) {
			// there occurred an error
			throw TcpException(TcpException::ERR_SOCKET_ERROR);
		}
		return false;
	}
	setg(inputBuffer, inputBuffer, inputBuffer + size);
	// trace socket readings
	TraceFile::getTraceFile().write(hostname, port, false, std::string(inputBuffer, size));
	return true;
}


int TcpClientBuf::underflow() {
	if (Receive())
		return gptr()[0];
	return EOF;
}

// ----- Stuff for writing to socket -------------------------------------------

void TcpClientBuf::Write(std::string &s) {
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

void TcpClientBuf::PutBuffer() {
	// send only, if there is data available
	if (pbase() != pptr()) {
		std::string data(pbase(), pptr() - pbase());
		Write(data);
	}
}

int TcpClientBuf::overflow(int c) {
	PutBuffer();
	if (c != EOF) {
		sputc(c);
	}
	return 0;
}

int TcpClientBuf::sync() {
	PutBuffer();
	return 0;
}

std::iostream& TcpClient::operator>> (std::string &s) {
	// read data that is available from input
	char buffer[BUF_SIZE];
	std::streamsize size = 0;
	peek(); // wait for at least one byte
	if (!good())
		throw TcpException(TcpException::ERR_CONNECTION_RESET);
	size = readsome(buffer, BUF_SIZE);
	s.assign(buffer, size);
	return *this;
}

// ----- HttpClient/HttpClientBuf ---------------------------------------------

std::iostream& HttpClient::operator>> (std::string &s) {
	// read until EOF, return complete string
	s.clear();
	char buffer[BUF_SIZE];
	while (good()) {
		read(buffer, BUF_SIZE);
		s.append(buffer, gcount());
	}
	return *this;
}

int HttpClientBuf::sync() {
	std::string buffer(pbase(), pptr() - pbase());

	switch (this->state) {
	case GET: {
		SetState(PLAIN);
		std::string getUrl = buffer.substr(0, buffer.find('\n'));
		std::string header = buffer.find('\n') != std::string::npos ? buffer.substr(buffer.find('\n')) : "";
		if (header.length() == 0 || header[header.length()-1] != '\n')
			header += "\n";
		std::stringstream result;
		result << "GET "   << getUrl   << " HTTP/1.0\n"
		       << "Host: " << hostname
		       << header
		       << "\n";
		sputn(result.str().c_str(), result.str().size());
		PutBuffer();
		break;
	}
	case POST: {
		SetState(PLAIN);
		std::string postUrl = buffer.substr(0, buffer.find('\n'));
		std::string header = buffer.find('\n') != std::string::npos ? buffer.substr(buffer.find('\n')) : "";
		if (header.length() == 0 || header[header.length()-1] != '\n')
			header += "\n";
		std::stringstream result;
		result << "POST "  << postUrl  << " HTTP/1.0\n"
		       << "Host: " << hostname
		       << header; // only one \n, more header fields to come below
		sputn(result.str().c_str(), result.str().size());
		PutBuffer();
		SetState(POSTDATA);
		break;
	}
	case POSTDATA: {
		SetState(PLAIN);
		std::stringstream result;
		result << "Content-Type: application/x-www-form-urlencoded\n"
		       << "Content-Length: " << buffer.length() << "\n\n"
		       << buffer << std::endl;
		sputn(result.str().c_str(), result.str().size());
		PutBuffer();
		break;
	}
	case PLAIN:
	case HEADER:
		PutBuffer();
		break;
	}
	return 0;
}

void HttpClientBuf::SetState(eState state) {
	this->state = state;
	switch (state) {
	case GET:
	case POST:
	case POSTDATA:
		setp(internalBuffer, internalBuffer + BUF_SIZE);
		break;
	case PLAIN:
	case HEADER:
		setp(outputBuffer, outputBuffer + BUF_SIZE);
		break;
	}
}

std::ostream& tcpclient::get(std::ostream &os) {
	os.flush();
	((HttpClientBuf *)os.rdbuf())->SetState(HttpClientBuf::GET);
	return os;
}

std::ostream& tcpclient::post(std::ostream &os) {
	os.flush();
	((HttpClientBuf *)os.rdbuf())->SetState(HttpClientBuf::POST);
	return os;
}

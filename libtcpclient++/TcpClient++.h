/*
 * libtcpclient++
 *
 * Copyright (C) 2007-2010 Joachim Wilke <libtcpclient@joachim-wilke.de>
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

#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include <string>
#include <sstream>
#include <stdexcept>
#include <PThread++.h>

// BUF_SIZE - buffer size of TcpClient's input/output buffers
// Warning: This also limits maximum size for POST payload
#define BUF_SIZE 1024

namespace tcpclient {

class TcpException : public std::exception {
public:
	enum errorCode {
		ERR_UNKNOWN,
		ERR_HOST_NOT_RESOLVABLE,
		ERR_HOST_NOT_REACHABLE,
		ERR_SOCKET_CREATE,
		ERR_CONNECTION_RESET,
		ERR_CONNECTION_REFUSED,
		ERR_INVALID_DATA,
		ERR_INVALID_ARGUMENT,
		ERR_OPERATION_NOT_SUPPORTED,
		ERR_PROTOTYPE,
		ERR_SOCKET_ERROR,
		ERR_ALREADY_TRYING,
		ERR_ALREADY_CONNECTED,
		ERR_OUT_OF_MEMORY,
		ERR_TIMEOUT,
	} errcode;

	TcpException(errorCode errcode) {
		this->errcode = errcode;
	}
	virtual const char* what() const throw() {
		switch (errcode) {
		case ERR_HOST_NOT_RESOLVABLE:
			return "Host is not resolvable.";
		case ERR_HOST_NOT_REACHABLE:
			return "Host is not reachable.";
		case ERR_SOCKET_CREATE:
			return "Could not create socket.";
		case ERR_CONNECTION_RESET:
			return "Connection was reset.";
		case ERR_CONNECTION_REFUSED:
			return "Connection was refused.";
		case ERR_INVALID_DATA:
			return "Invalid data received.";
		case ERR_INVALID_ARGUMENT:
			return "Invalid argument used.";
		case ERR_OPERATION_NOT_SUPPORTED:
			return "Operation not supported.";
		case ERR_PROTOTYPE:
			return "Prototype error?";
		case ERR_SOCKET_ERROR:
			return "Generic error on socket layer.";
		case ERR_ALREADY_TRYING:
			return "Already trying to connect.";
		case ERR_ALREADY_CONNECTED:
			return "Already connected.";
		case ERR_OUT_OF_MEMORY:
			return "System is out of memory.";
		case ERR_TIMEOUT:
			return "Operation timed out.";
		default:
			return "Unknown TcpException happened.";
		}

	}
};

class TcpClientBuf : public std::streambuf {
private:
	bool connected;
	pthread::Mutex mutex;
	int fd;
	void Write(std::string &s);
	void Connect();
	bool Receive();
protected:
	std::string hostname;
	int port;
	char inputBuffer[BUF_SIZE];
	char outputBuffer[BUF_SIZE];
	void PutBuffer();
	// redefine overflow, underflow and sync from std::streambuf
	int overflow(int c);
	int underflow();
	int	sync();
public:
	TcpClientBuf(std::string hostname, int port) throw(tcpclient::TcpException);
	virtual ~TcpClientBuf();
	void Disconnect();
};

class TcpClient : public std::iostream {
public:
	TcpClient(std::string hostname, int port) throw(tcpclient::TcpException)
	: std::iostream(new TcpClientBuf(hostname, port)) {}
	TcpClient(TcpClientBuf *buf)
	: std::iostream(buf) {}
	virtual ~TcpClient();
	std::iostream& operator>> (std::string &s);
};

class HttpClientBuf : public TcpClientBuf {
public:
	enum eState {
		PLAIN,
		GET,
		POST,
		POSTDATA,
		HEADER
	};
private:
	eState state;
	bool addContentType;
	char internalBuffer[BUF_SIZE];
public:
	HttpClientBuf(std::string hostname, int port);
	void SetState(eState state);
protected:
	int	sync();
};

class HttpClient : public TcpClient {
public:
	HttpClient(std::string hostname, int port = 80)
	: TcpClient(new HttpClientBuf(hostname, port)) {}
	std::iostream& operator>> (std::string &s);
};

std::ostream& get(std::ostream &os);
std::ostream& post(std::ostream &os);

}

#endif /*TCPCLIENT_H_*/

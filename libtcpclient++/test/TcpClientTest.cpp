/*
 * TcpClientTest.cpp
 *
 *  Created on: 29.12.2008
 *      Author: wilke
 */

#include <QtTest>
#include <QtCore>
#include <string>
#include <iostream>
#include <TcpClient++.h>
#include "TcpSendFile.h"

class TcpClientTest : public QObject {
	Q_OBJECT

private slots:
	void Connect();
	void ConnectFail();
};


void TcpClientTest::Connect() {
	// setup of test case infrastructure
	const int port = 8080;
	tcpclient::TcpSendFile sf = tcpclient::TcpSendFile("/proc/cpuinfo", port);

	// actual test
	std::string host = "localhost";
	tcpclient::TcpClient tc = tcpclient::TcpClient(host, port);
	tc << "GET / HTTP/1.1\n\n";
	std::string msg;
	tc >> msg;
	// msg must contain an answer
	QVERIFY(msg.size());
}

void TcpClientTest::ConnectFail() {
	int error_code = 0;
	std::string host = "localhost";
	try{
		tcpclient::TcpClient tc = tcpclient::TcpClient(host, 8081);
		tc << "GET / HTTP/1.1\n\n";
		std::string msg;
		tc >> msg;
	} catch(tcpclient::TcpException exc){
		error_code = exc.errcode;
	}

	// as we did not start any tcp server we except an exception (connection refused)
	QVERIFY(error_code == tcpclient::TcpException::ERR_CONNECTION_REFUSED);
};

QTEST_MAIN(TcpClientTest)
#include "TcpClientTest.moc"

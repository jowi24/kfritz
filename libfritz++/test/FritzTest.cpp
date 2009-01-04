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
#include <TcpSendFile.h>
#include <Config.h>
#include <Tools.h>
#include <iostream>

class FritzTest : public QObject, public fritz::Config {
	Q_OBJECT

public:
	FritzTest();

private slots:
	void GetLocationSettings();
};

FritzTest::FritzTest()
: fritz::Config("localhost", "")
{
	this->mConfig.uiPort = 8080;
	fritz::gConfig = this;
}


void FritzTest::GetLocationSettings() {
	// setup of test case infrastructure
	const int port = 8080;
	tcpclient::TcpSendFile sf = tcpclient::TcpSendFile("29.04.67/de-sipoptionen", port);

	mConfig.lang = "de";

	fritz::Tools::GetLocationSettings(false);

	QVERIFY(mConfig.countryCode == "11");
	QVERIFY(mConfig.regionCode == "12345");
}

QTEST_MAIN(FritzTest)
#include "FritzTest.moc"

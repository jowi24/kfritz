/*
 * fritzeventhandler.cpp
 *
 *  Created on: Apr 1, 2012
 *      Author: jo
 */


#include "gtest/gtest.h"

#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>

#include <QSignalSpy>

#include <KFritzWindow.h>
#include <Listener.h>

namespace test {

class FritzEventHandler : public ::testing::Test {
private:
	KApplication *app;
protected:
	KFritzWindow *window;
	QSignalSpy *spy;
	void SetUp() {
		KAboutData aboutData("kfritz", 0, ki18n("KFritz"), "0");
		char * argv[2];
		asprintf(&argv[0], "%s", "kfritz");
		KCmdLineArgs::init( 1, argv, &aboutData );
		KCmdLineOptions options;
		options.add("p");
		options.add("log-personal-info", ki18n("Log personal information (e.g. passwords, phone numbers, ...)"));
		KCmdLineArgs::addCmdLineOptions(options);

		app = new KApplication();
		window = new KFritzWindow();
		// in the test we don't want that the notification is actually displayed
		window->disconnect(window, SIGNAL(signalNotification(QString, QString, bool)), window, SLOT(slotNotification(QString, QString, bool)));
		spy = new QSignalSpy(window, SIGNAL(signalNotification(QString, QString, bool)));
	}

	void TearDown() {
		delete spy;
		delete window;
		delete app;
	}
};

TEST_F(FritzEventHandler, Ring) {
    window->handleCall(false, 0, "1234", "Huber", fritz::FonbookEntry::TYPE_MOBILE, "1704", "SIP0", "Bestvalue VoIP");

	ASSERT_EQ(spy->count(), 1);
	QList<QVariant> arguments = spy->takeFirst();
	ASSERT_EQ(arguments.at(0).toString().toStdString(), "incomingCall");
	ASSERT_EQ(arguments.at(1).toString().toStdString(), "Incoming call from <b>Huber (Mobile)</b><br/>using Bestvalue VoIP");
	ASSERT_EQ(arguments.at(2).toBool(), true);
}

TEST_F(FritzEventHandler, Call) {
    window->handleCall(true, 0, "3456", "Mayer", fritz::FonbookEntry::TYPE_HOME, "1805", "SIP1", "Trash VoIP");

	ASSERT_EQ(spy->count(), 1);
	QList<QVariant> arguments = spy->takeFirst();
	ASSERT_EQ(arguments.at(0).toString().toStdString(), "outgoingCall");
	ASSERT_EQ(arguments.at(1).toString().toStdString(), "Outgoing call to <b>Mayer (Home)</b><br/>using Trash VoIP");
	ASSERT_EQ(arguments.at(2).toBool(), true);
}

TEST_F(FritzEventHandler, Connect) {
    window->handleConnect(0);

	ASSERT_EQ(spy->count(), 1);
	QList<QVariant> arguments = spy->takeFirst();
	ASSERT_EQ(arguments.at(0).toString().toStdString(), "callConnected");
	ASSERT_EQ(arguments.at(1).toString().toStdString(), "Call connected.");
	ASSERT_EQ(arguments.at(2).toBool(), false);
}

}

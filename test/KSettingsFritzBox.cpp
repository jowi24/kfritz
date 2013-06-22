/*
 * KSettingsFritzBox.cpp
 *
 *  Created on: Apr 5, 2012
 *      Author: jo
 */


#include "gtest/gtest.h"

#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>

#include <QSignalSpy>

#include <KFritzWindow.h>
#include <KSettingsFritzBox.h>
#include <KSettingsFonbooks.h>
#include <KSettings.h>
#include <Listener.h>

namespace test {

class KSettingsFritzBox : public ::testing::Test {
private:
	KApplication *app;
protected:
	KFritzWindow *window;
	void SetUp() {
		KAboutData aboutData("kfritz", 0, ki18n("KFritz"), "0");
		char * argv[2];
		asprintf(&argv[0], "%s", "kfritz");
		KCmdLineArgs::init( 1, argv, &aboutData );
		KCmdLineOptions options;
		options.add("p");
		options.add("log-personal-info", ki18n("Log personal information (e.g. passwords, phone numbers, ...)"));
		KCmdLineArgs::addCmdLineOptions(options);

		KSettings::setHostname("localhost");

		app = new KApplication();
//		window = new KFritzWindow();
	}

	void TearDown() {
//		delete window;
		delete app;

	}
};

TEST_F(KSettingsFritzBox, SettingsFritzBox) {
//	window->showSettings();
	QWidget w;
	::KSettingsFritzBox *fb = new ::KSettingsFritzBox(&w);
	delete fb;
}

TEST_F(KSettingsFritzBox, SettingsFonbooks) {
//	window->showSettings();
	QWidget w;
	::KSettingsFonbooks *fb = new ::KSettingsFonbooks(&w);
	delete fb;
}

}


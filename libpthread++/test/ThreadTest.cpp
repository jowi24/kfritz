/*
 * ThreadTest.cpp
 *
 *  Created on: 02.12.2008
 *      Author: wilke
 */

#include <QtTest>
#include <QtCore>
#include "../PThread++.h"


class cTestThread : public pthread::PThread {
public:
	bool toggle;

	cTestThread()
	: pthread::PThread("thread1")
	{
		toggle = false;
	}

	virtual void Action() {
		toggle = true;
	}

};

class ThreadTest : public QObject {
	Q_OBJECT

private slots:
	void Run();
};


void ThreadTest::Run() {
	cTestThread *t = new cTestThread();
	QVERIFY(t->toggle == false);
	t->Start();
	QVERIFY(t->toggle == true);
	delete t;
}

QTEST_MAIN(ThreadTest)
#include "ThreadTest.moc"


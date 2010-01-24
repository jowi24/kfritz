/*
 * LibFritzInit.h
 *
 *  Created on: Jan 22, 2010
 *      Author: jo
 */

#ifndef LIBFRITZINIT_H_
#define LIBFRITZINIT_H_

#include <QThread>
#include "KEventHandler.h"

class LibFritzInit : public QThread {
	Q_OBJECT;
public:
	LibFritzInit();
	virtual ~LibFritzInit();
	void run();
private:
	QString getBoxPwd();
	KEventHandler *eventHandler;
Q_SIGNALS:
	void ready(bool isReady);
};

#endif /* LIBFRITZINIT_H_ */

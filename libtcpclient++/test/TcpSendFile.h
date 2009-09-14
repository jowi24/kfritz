/*
 * TcpSendFile.h
 *
 *  Created on: Dec 29, 2008
 *      Author: joachim
 */

#ifndef TCPSENDFILE_H_
#define TCPSENDFILE_H_

#include <string>
namespace tcpclient {

class TcpSendFile {
public:
	TcpSendFile(std::string filename, int port);
	virtual ~TcpSendFile();
private:
	int SystemExec(const char *Command, bool Detached);
};

}

#endif /* TCPSENDFILE_H_ */

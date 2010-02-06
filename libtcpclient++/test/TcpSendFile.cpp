/*
 * TcpSendFile.cpp
 *
 *  Created on: Dec 29, 2008
 *      Author: joachim
 */

#include "TcpSendFile.h"
#include <sstream>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <fcntl.h>


namespace tcpclient {

TcpSendFile::TcpSendFile(std::string filename, int port) {
	std::stringstream command;
	command << "nc -l -w 5 -p " << port << "<" << filename << "> /dev/null &";
	SystemExec(command.str().c_str(),false);

}

TcpSendFile::~TcpSendFile() {
}

int TcpSendFile::SystemExec(const char *Command, bool Detached)
{
  pid_t pid;

  if ((pid = fork()) < 0) { // fork failed
     return -1;
     }

  if (pid > 0) { // parent process
     int status = 0;
     if (waitpid(pid, &status, 0) < 0) {
        return -1;
        }
     return status;
     }
  else { // child process
     if (Detached) {
        // Fork again and let first child die - grandchild stays alive without parent
        if (fork() > 0)
           _exit(0);
        // Start a new session
        pid_t sid = setsid();
        // close STDIN and re-open as /dev/null
        int devnull = open("/dev/null", O_RDONLY);
        }
     int MaxPossibleFileDescriptors = getdtablesize();
     for (int i = STDERR_FILENO + 1; i < MaxPossibleFileDescriptors; i++)
         close(i); //close all dup'ed filedescriptors
     if (execl("/bin/sh", "sh", "-c", Command, NULL) == -1) {
        _exit(-1);
        }
     _exit(0);
     }
}

}

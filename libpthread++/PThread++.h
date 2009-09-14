/*
 * thread.h: A simple thread base class
 *
 * See COPYING for copyright information.
 *
 * Bases on thread.h from VDR (www.cadsoft.de/vdr).
 * $Id: thread.h 1.39 2007/02/24 16:13:28 kls Exp $
 */

#ifndef __PTHREADPP_H
#define __PTHREADPP_H

#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <string>
#include <iostream>

namespace pthread {

// objects for logging
extern std::ostream *dsyslog;
extern std::ostream *isyslog;
extern std::ostream *esyslog;

void SetupLogging( std::ostream *dsyslog, std::ostream *isyslog, std::ostream *esyslog );

class CondWait {
private:
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	bool signaled;
public:
	CondWait(void);
	~CondWait();
	static void SleepMs(int TimeoutMs);
	///< Creates a cCondWait object and uses it to sleep for TimeoutMs
	///< milliseconds, immediately giving up the calling thread's time
	///< slice and thus avoiding a "busy wait".
	///< In order to avoid a possible busy wait, TimeoutMs will be automatically
	///< limited to values >2.
	bool Wait(int TimeoutMs = 0);
	///< Waits at most TimeoutMs milliseconds for a call to Signal(), or
	///< forever if TimeoutMs is 0.
	///< \return Returns true if Signal() has been called, false it the given
	///< timeout has expired.
	void Signal(void);
	///< Signals a caller of Wait() that the condition it is waiting for is met.
};

class Mutex;

class CondVar {
private:
	pthread_cond_t cond;
public:
	CondVar(void);
	~CondVar();
	void Wait(Mutex &Mutex);
	bool TimedWait(Mutex &Mutex, int TimeoutMs);
	void Broadcast(void);
};

class RwLock {
private:
	pthread_rwlock_t rwlock;
public:
	RwLock(bool PreferWriter = false);
	~RwLock();
	bool Lock(bool Write, int TimeoutMs = 0);
	void Unlock(void);
};

class Mutex {
	friend class CondVar;
private:
	pthread_mutex_t mutex;
	int locked;
public:
	Mutex(void);
	~Mutex();
	void Lock(void);
	void Unlock(void);
};

typedef pid_t tThreadId;

class PThread {
	friend class ThreadLock;
private:
	bool active;
	bool running;
	pthread_t childTid;
	tThreadId childThreadId;
	Mutex mutex;
	std::string description;
	static void *StartThread(PThread *Thread);
protected:
	void SetPriority(int Priority);
	void Lock(void) { mutex.Lock(); }
	void Unlock(void) { mutex.Unlock(); }
	virtual void Action(void) = 0;
	///< A derived cThread class must implement the code it wants to
	///< execute as a separate thread in this function. If this is
	///< a loop, it must check Running() repeatedly to see whether
	///< it's time to stop.
	bool Running(void) { return running; }
	///< Returns false if a derived cThread object shall leave its Action()
	///< function.
	void Cancel(int WaitSeconds = 0);
	///< Cancels the thread by first setting 'running' to false, so that
	///< the Action() loop can finish in an orderly fashion and then waiting
	///< up to WaitSeconds seconds for the thread to actually end. If the
	///< thread doesn't end by itself, it is killed.
	///< If WaitSeconds is -1, only 'running' is set to false and Cancel()
	///< returns immediately, without killing the thread.
public:
	PThread(std::string Description);
	///< Creates a new thread.
	///< If Description is present, a log file entry will be made when
	///< the thread starts and stops. The Start() function must be called
	///< to actually start the thread.
	virtual ~PThread();
	void SetDescription(std::string Description);
	bool Start(void);
	///< Actually starts the thread.
	///< If the thread is already running, nothing happens.
	bool Active(void);
	///< Checks whether the thread is still alive.
	static tThreadId ThreadId(void);
};

// cMutexLock can be used to easily set a lock on mutex and make absolutely
// sure that it will be unlocked when the block will be left. Several locks can
// be stacked, so a function that makes many calls to another function which uses
// cMutexLock may itself use a cMutexLock to make one longer lock instead of many
// short ones.

class MutexLock {
private:
	Mutex *mutex;
	bool locked;
public:
	MutexLock(Mutex *Mutex = NULL);
	~MutexLock();
	bool Lock(Mutex *Mutex);
};

// cThreadLock can be used to easily set a lock in a thread and make absolutely
// sure that it will be unlocked when the block will be left. Several locks can
// be stacked, so a function that makes many calls to another function which uses
// cThreadLock may itself use a cThreadLock to make one longer lock instead of many
// short ones.

class ThreadLock {
private:
	PThread *thread;
	bool locked;
public:
	ThreadLock(PThread *Thread = NULL);
	~ThreadLock();
	bool Lock(PThread *Thread);
};

#define LOCK_THREAD cThreadLock ThreadLock(this)

// cPipe implements a pipe that closes all unnecessary file descriptors in
// the child process.

class Pipe {
private:
	pid_t pid;
	FILE *f;
public:
	Pipe(void);
	~Pipe();
	operator FILE* () { return f; }
	bool Open(const char *Command, const char *Mode);
	int Close(void);
};

// SystemExec() implements a 'system()' call that closes all unnecessary file
// descriptors in the child process.
// With Detached=true, calls command in background and in a separate session,
// with stdin connected to /dev/null.

class TimeMs {
private:
	uint64_t begin;
public:
	TimeMs(int Ms = 0);
	///< Creates a timer with ms resolution and an initial timeout of Ms.
	static uint64_t Now(void);
	void Set(int Ms = 0);
	bool TimedOut(void);
	uint64_t Elapsed(void);
};


//int SystemExec(const char *Command, bool Detached = false);

}

#endif //__PTHREADPP_H

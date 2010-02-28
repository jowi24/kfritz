/*
 * libpthread++
 *
 * Copyright (C) 2007-2010 Joachim Wilke <libpthread@joachim-wilke.de>
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

/*
 * A simple thread base class
 * Bases on thread.c from VDR (www.cadsoft.de/vdr).
 */

#include "PThread++.h"
#include <errno.h>
#include <linux/unistd.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define NAMESPACE "libpthread++"
#define LOCATOR "[" << NAMESPACE << "/" <<  \
                std::string(__FILE__, std::string(__FILE__).rfind('/') == std::string::npos ? \
                		          0 : std::string(__FILE__).rfind('/')+1, std::string::npos ) \
                << ":" << __LINE__ << "] "
#define DBG(x) *dsyslog << LOCATOR << x << std::endl;
#define INF(x) *isyslog << LOCATOR << x << std::endl;
#define ERR(x) *esyslog << LOCATOR << x << std::endl;

namespace pthread {

std::ostream *dsyslog = &std::clog;
std::ostream *isyslog = &std::cout;
std::ostream *esyslog = &std::cerr;

void SetupLogging(std::ostream *d, std::ostream *i, std::ostream *e)
{
	dsyslog = d;
	isyslog = i;
	esyslog = e;
}


static bool GetAbsTime(struct timespec *Abstime, int MillisecondsFromNow)
{
	struct timeval now;
	if (gettimeofday(&now, NULL) == 0) {           // get current time
		now.tv_usec += MillisecondsFromNow * 1000;  // add the timeout
		while (now.tv_usec >= 1000000) {            // take care of an overflow
			now.tv_sec++;
			now.tv_usec -= 1000000;
		}
		Abstime->tv_sec = now.tv_sec;          // seconds
		Abstime->tv_nsec = now.tv_usec * 1000; // nano seconds
		return true;
	}
	return false;
}

// --- cCondWait -------------------------------------------------------------

CondWait::CondWait(void)
{
	signaled = false;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
}

CondWait::~CondWait()
{
	pthread_cond_broadcast(&cond); // wake up any sleepers
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
}

void CondWait::SleepMs(int TimeoutMs)
{
	CondWait w;
	w.Wait(TimeoutMs > 3 ? TimeoutMs : 3); // making sure the time is >2ms to avoid a possible busy wait
}

bool CondWait::Wait(int TimeoutMs)
{
	pthread_mutex_lock(&mutex);
	if (!signaled) {
		if (TimeoutMs) {
			struct timespec abstime;
			if (GetAbsTime(&abstime, TimeoutMs)) {
				while (!signaled) {
					if (pthread_cond_timedwait(&cond, &mutex, &abstime) == ETIMEDOUT)
						break;
				}
			}
		}
		else
			pthread_cond_wait(&cond, &mutex);
	}
	bool r = signaled;
	signaled = false;
	pthread_mutex_unlock(&mutex);
	return r;
}

void CondWait::Signal(void)
{
	pthread_mutex_lock(&mutex);
	signaled = true;
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);
}

// --- cCondVar --------------------------------------------------------------

CondVar::CondVar(void)
{
	pthread_cond_init(&cond, 0);
}

CondVar::~CondVar()
{
	pthread_cond_broadcast(&cond); // wake up any sleepers
	pthread_cond_destroy(&cond);
}

void CondVar::Wait(Mutex &Mutex)
{
	if (Mutex.locked) {
		int locked = Mutex.locked;
		Mutex.locked = 0; // have to clear the locked count here, as pthread_cond_wait
		// does an implicit unlock of the mutex
		pthread_cond_wait(&cond, &Mutex.mutex);
		Mutex.locked = locked;
	}
}

bool CondVar::TimedWait(Mutex &Mutex, int TimeoutMs)
{
	bool r = true; // true = condition signaled, false = timeout

	if (Mutex.locked) {
		struct timespec abstime;
		if (GetAbsTime(&abstime, TimeoutMs)) {
			int locked = Mutex.locked;
			Mutex.locked = 0; // have to clear the locked count here, as pthread_cond_timedwait
			// does an implicit unlock of the mutex.
			if (pthread_cond_timedwait(&cond, &Mutex.mutex, &abstime) == ETIMEDOUT)
				r = false;
			Mutex.locked = locked;
		}
	}
	return r;
}

void CondVar::Broadcast(void)
{
	pthread_cond_broadcast(&cond);
}

// --- cRwLock ---------------------------------------------------------------

RwLock::RwLock(bool PreferWriter)
{
	pthread_rwlockattr_t attr;
	pthread_rwlockattr_init(&attr);
	pthread_rwlockattr_setkind_np(&attr, PreferWriter ? PTHREAD_RWLOCK_PREFER_WRITER_NP : PTHREAD_RWLOCK_PREFER_READER_NP);
	pthread_rwlock_init(&rwlock, &attr);
}

RwLock::~RwLock()
{
	pthread_rwlock_destroy(&rwlock);
}

bool RwLock::Lock(bool Write, int TimeoutMs)
{
	int Result = 0;
	struct timespec abstime;
	if (TimeoutMs) {
		if (!GetAbsTime(&abstime, TimeoutMs))
			TimeoutMs = 0;
	}
	if (Write)
		Result = TimeoutMs ? pthread_rwlock_timedwrlock(&rwlock, &abstime) : pthread_rwlock_wrlock(&rwlock);
		else
			Result = TimeoutMs ? pthread_rwlock_timedrdlock(&rwlock, &abstime) : pthread_rwlock_rdlock(&rwlock);
			return Result == 0;
}

void RwLock::Unlock(void)
{
	pthread_rwlock_unlock(&rwlock);
}

// --- cMutex ----------------------------------------------------------------

Mutex::Mutex(void)
{
	locked = 0;
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
	pthread_mutex_init(&mutex, &attr);
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&mutex);
}

void Mutex::Lock(void)
{
	pthread_mutex_lock(&mutex);
	locked++;
}

void Mutex::Unlock(void)
{
	if (!--locked)
		pthread_mutex_unlock(&mutex);
}

// --- cThread ---------------------------------------------------------------


PThread::PThread(std::string Description)
{
	active = running = false;
	childTid = 0;
	childThreadId = 0;
	description = Description;
}

PThread::~PThread()
{
	Cancel(); // just in case the derived class didn't call it
}

void PThread::SetPriority(int Priority)
{
	if (setpriority(PRIO_PROCESS, 0, Priority) < 0)
		void(); //LOG_ERROR;
}

void PThread::SetDescription(std::string Description)
{
	description = Description;
}

void *PThread::StartThread(PThread *Thread)
{
	Thread->childThreadId = ThreadId();
	DBG("" << Thread->description << ": thread started (pid=" << getpid() << ", tid=" << Thread->childThreadId << ")");
	Thread->Action();
	DBG("" << Thread->description << ": thread ended (pid=" << getpid() << ", tid=" << Thread->childThreadId << ")");
	Thread->running = false;
	Thread->active = false;
	return NULL;
}

#define THREAD_STOP_TIMEOUT  3000 // ms to wait for a thread to stop before newly starting it
#define THREAD_STOP_SLEEP      30 // ms to sleep while waiting for a thread to stop

bool PThread::Start(void)
{
	if (!running) {
		if (active) {
			// Wait until the previous incarnation of this thread has completely ended
			// before starting it newly:
			TimeMs RestartTimeout;
			while (!running && active && RestartTimeout.Elapsed() < THREAD_STOP_TIMEOUT)
				CondWait::SleepMs(THREAD_STOP_SLEEP);
		}
		if (!active) {
			active = running = true;
			if (pthread_create(&childTid, NULL, (void *(*) (void *))&StartThread, (void *)this) == 0) {
				pthread_detach(childTid); // auto-reap
			}
			else {
				void(); //LOG_ERROR;
				active = running = false;
				return false;
			}
		}
	}
	return true;
}

bool PThread::Active(void)
{
	if (active) {
		//
		// Single UNIX Spec v2 says:
		//
		// The pthread_kill() function is used to request
		// that a signal be delivered to the specified thread.
		//
		// As in kill(), if sig is zero, error checking is
		// performed but no signal is actually sent.
		//
		int err;
		if ((err = pthread_kill(childTid, 0)) != 0) {
			if (err != ESRCH)
				void(); //LOG_ERROR;
			childTid = 0;
			active = running = false;
		}
		else
			return true;
	}
	return false;
}

void PThread::Cancel(int WaitSeconds)
{
	running = false;
	if (active && WaitSeconds > -1) {
		if (WaitSeconds > 0) {
			for (time_t t0 = time(NULL) + WaitSeconds; time(NULL) < t0; ) {
				if (!Active())
					return;
				CondWait::SleepMs(10);
			}
			*esyslog << __FILE__ << "ERROR: " << description << ": thread " << childThreadId << "won't end (waited " << WaitSeconds << " seconds) - canceling it..." << std::endl;
		}
		pthread_cancel(childTid);
		childTid = 0;
		active = false;
	}
}

tThreadId PThread::ThreadId(void)
{
	return syscall(__NR_gettid);
}


// --- cMutexLock ------------------------------------------------------------

MutexLock::MutexLock(Mutex *Mutex)
{
	mutex = NULL;
	locked = false;
	Lock(Mutex);
}

MutexLock::~MutexLock()
{
	if (mutex && locked)
		mutex->Unlock();
}

bool MutexLock::Lock(Mutex *Mutex)
{
	if (Mutex && !mutex) {
		mutex = Mutex;
		Mutex->Lock();
		locked = true;
		return true;
	}
	return false;
}

// --- cThreadLock -----------------------------------------------------------

ThreadLock::ThreadLock(PThread *Thread)
{
	thread = NULL;
	locked = false;
	Lock(Thread);
}

ThreadLock::~ThreadLock()
{
	if (thread && locked)
		thread->Unlock();
}

bool ThreadLock::Lock(PThread *Thread)
{
	if (Thread && !thread) {
		thread = Thread;
		Thread->Lock();
		locked = true;
		return true;
	}
	return false;
}

// --- cPipe -----------------------------------------------------------------

// cPipe::Open() and cPipe::Close() are based on code originally received from
// Andreas Vitting <Andreas@huji.de>

Pipe::Pipe(void)
{
	pid = -1;
	f = NULL;
}

Pipe::~Pipe()
{
	Close();
}

bool Pipe::Open(const char *Command, const char *Mode)
{
	int fd[2];

	if (pipe(fd) < 0) {
		void(); //LOG_ERROR;
		return false;
	}
	if ((pid = fork()) < 0) { // fork failed
		void(); //LOG_ERROR;
		close(fd[0]);
		close(fd[1]);
		return false;
	}

	const char *mode = "w";
	int iopipe = 0;

	if (pid > 0) { // parent process
		if (strcmp(Mode, "r") == 0) {
			mode = "r";
			iopipe = 1;
		}
		close(fd[iopipe]);
		if ((f = fdopen(fd[1 - iopipe], mode)) == NULL) {
			void(); //LOG_ERROR;
			close(fd[1 - iopipe]);
		}
		return f != NULL;
	}
	else { // child process
		int iofd = STDOUT_FILENO;
		if (strcmp(Mode, "w") == 0) {
			mode = "r";
			iopipe = 1;
			iofd = STDIN_FILENO;
		}
		close(fd[iopipe]);
		if (dup2(fd[1 - iopipe], iofd) == -1) { // now redirect
			void(); //LOG_ERROR;
			close(fd[1 - iopipe]);
			_exit(-1);
		}
		else {
			int MaxPossibleFileDescriptors = getdtablesize();
			for (int i = STDERR_FILENO + 1; i < MaxPossibleFileDescriptors; i++)
				close(i); //close all dup'ed filedescriptors
			if (execl("/bin/sh", "sh", "-c", Command, NULL) == -1) {
				void(); //LOG_ERROR_STR(Command);
				close(fd[1 - iopipe]);
				_exit(-1);
			}
		}
		_exit(0);
	}
}

int Pipe::Close(void)
{
	int ret = -1;

	if (f) {
		fclose(f);
		f = NULL;
	}

	if (pid > 0) {
		int status = 0;
		int i = 5;
		while (i > 0) {
			ret = waitpid(pid, &status, WNOHANG);
			if (ret < 0) {
				if (errno != EINTR && errno != ECHILD) {
					void(); //LOG_ERROR;
					break;
				}
			}
			else if (ret == pid)
				break;
			i--;
			CondWait::SleepMs(100);
		}
		if (!i) {
			kill(pid, SIGKILL);
			ret = -1;
		}
		else if (ret == -1 || !WIFEXITED(status))
			ret = -1;
		pid = -1;
	}

	return ret;
}

// --- cTimeMs ---------------------------------------------------------------

TimeMs::TimeMs(int Ms)
{
	Set(Ms);
}

uint64_t TimeMs::Now(void)
{
#if _POSIX_TIMERS > 0 && defined(_POSIX_MONOTONIC_CLOCK)
#define MIN_RESOLUTION 5 // ms
	static bool initialized = false;
	static bool monotonic = false;
	struct timespec tp;
	if (!initialized) {
		// check if monotonic timer is available and provides enough accurate resolution:
		if (clock_getres(CLOCK_MONOTONIC, &tp) == 0) {
			long Resolution = tp.tv_nsec;
			// require a minimum resolution:
			if (tp.tv_sec == 0 && tp.tv_nsec <= MIN_RESOLUTION * 1000000) {
				if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0) {
					DBG("cTimeMs: using monotonic clock (resolution is " << Resolution << " ns)");
					monotonic = true;
				}
				else
					ERR("cTimeMs: clock_gettime(CLOCK_MONOTONIC) failed");
			}
			else
				DBG("cTimeMs: not using monotonic clock - resolution is too bad (" << tp.tv_sec << " s " << tp.tv_nsec << "ns)");
		}
		else
			ERR("cTimeMs: clock_getres(CLOCK_MONOTONIC) failed");
		initialized = true;
	}
	if (monotonic) {
		if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
			return (uint64_t(tp.tv_sec)) * 1000 + tp.tv_nsec / 1000000;
		ERR("cTimeMs: clock_gettime(CLOCK_MONOTONIC) failed");
		monotonic = false;
		// fall back to gettimeofday()
	}
#else
#  warning Posix monotonic clock not available
#endif
	struct timeval t;
	if (gettimeofday(&t, NULL) == 0)
		return (uint64_t(t.tv_sec)) * 1000 + t.tv_usec / 1000;
	return 0;
}

void TimeMs::Set(int Ms)
{
	begin = Now() + Ms;
}

bool TimeMs::TimedOut(void)
{
	return Now() >= begin;
}

uint64_t TimeMs::Elapsed(void)
{
	return Now() - begin;
}

}




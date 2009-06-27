/*
* Copyright 2008 Free Software Foundation, Inc.
*
* This software is distributed under the terms of the GNU Public License.
* See the COPYING file in the main directory for details.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/




#include "Threads.h"
#include "Timeval.h"


using namespace std;




Mutex gStreamLock;		///< Global lock to control access to cout and cerr.

void lockCout()
{
	gStreamLock.lock();
	Timeval entryTime;
	cout << entryTime << " " << pthread_self() << ": ";
}


void unlockCout()
{
	cout << dec << endl << flush;
	gStreamLock.unlock();
}


void lockCerr()
{
	gStreamLock.lock();
	Timeval entryTime;
	cerr << entryTime << " " << pthread_self() << ": ";
}

void unlockCerr()
{
	cerr << dec << endl << flush;
	gStreamLock.unlock();
}







Mutex::Mutex()
{
	assert(!pthread_mutexattr_init(&mAttribs));
	assert(!pthread_mutexattr_settype(&mAttribs,PTHREAD_MUTEX_RECURSIVE));
	assert(!pthread_mutex_init(&mMutex,&mAttribs));
}


Mutex::~Mutex()
{
	pthread_mutex_destroy(&mMutex);
	assert(!pthread_mutexattr_destroy(&mAttribs));
}




/** Block for the signal up to the cancellation timeout. */
void Signal::wait(Mutex& wMutex, unsigned timeout) const
{
	struct timespec waitTime = Timeval(timeout).timespec();
	// FIXME -- With -O3 optimzation in OS X this doesn't block.  See bug #320.
	pthread_cond_timedwait(&mSignal,&wMutex.mMutex,&waitTime);
}


void Thread::start(void *(*task)(void*), void *arg)
{
	assert(mThread==((pthread_t)0));
	assert(!pthread_attr_init(&mAttrib));
	assert(!pthread_attr_setstacksize(&mAttrib, mStackSize));
	assert(!pthread_create(&mThread, &mAttrib, task, arg));
}



// vim: ts=4 sw=4

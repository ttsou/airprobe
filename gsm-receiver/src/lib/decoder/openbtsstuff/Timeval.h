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


#ifndef TIMEVAL_H
#define TIMEVAL_H

#include "sys/time.h"
#include <iostream>
#include <stdint.h>
#include <unistd.h>


inline void msleep(long v) { usleep((v+500)/1000); }


/** A C++ wrapper for struct timeval. */
class Timeval {

	private:

	struct timeval mTimeval;

	public:

	/** Set the value to gettimeofday. */
	void now() { gettimeofday(&mTimeval,NULL); }

	/** Set the value to gettimeofday plus an offset. */
	void future(unsigned offset);

	//@{
	Timeval(unsigned sec, unsigned usec)
	{
		mTimeval.tv_sec = sec;
		mTimeval.tv_usec = usec;
	}

	Timeval(const struct timeval& wTimeval)
		:mTimeval(wTimeval)
	{}

	/**
		Create a Timeval offset into the future.
		@param offset milliseconds
	*/
	Timeval(unsigned offset=0) { future(offset); }
	//@}

	/** Convert to a struct timespec. */
	struct timespec timespec() const;

	/** Return total seconds. */
	double seconds() const;

	uint32_t sec() const { return mTimeval.tv_sec; }
	uint32_t usec() const { return mTimeval.tv_usec; }

	/** Return differnce from other (other-self), in ms. */
	long delta(const Timeval& other) const;

	/** Elapsed time in ms. */
	long elapsed() const { return delta(Timeval()); }

	/** Remaining time in ms. */
	long remaining() const { return -elapsed(); }

	/** Return true if the time has passed, as per gettimeofday. */
	bool passed() const;

};

std::ostream& operator<<(std::ostream& os, const Timeval&);

std::ostream& operator<<(std::ostream& os, const struct timespec&);


#endif
// vim: ts=4 sw=4

#
# Copyright 2001,2002,2004,2005,2006,2007,2008 Free Software Foundation, Inc.
# 
# This file is part of GNU Radio
# 
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 

include $(top_srcdir)/Makefile.common

SUBDIRS = openbtsstuff

noinst_LTLIBRARIES = libdecoder.la

libdecoder_la_LIBADD = \
	openbtsstuff/libopenbtsdecoder.la

libdecoder_la_SOURCES = \
	sch.c		\
	cch.c 		\
	fire_crc.c	\
	gsmstack.c	\
	interleave.c	
#	tch.c		\
#	conv.c

noinst_HEADERS = 	\
	sch.h 		\
	cch.h 		\
	fire_crc.h	\
	gsmstack.h	\
	interleave.h	\
	system.h	\
	gsmtap.h	\
	a5-1-2.h	
#	tch.h		\
#	conv.h		

#
# Copyright 2004,2005,2006,2008,2009 Free Software Foundation, Inc.
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

grinclude_HEADERS =		\
	gsm_receiver_cf.h	\
	gsm_receiver_config.h	\
	viterbi_detector.h

SUBDIRS = decoder

TOP_SWIG_IFILES =		\
	gsm.i

gsm_pythondir_category =	\
	gnuradio

gsm_la_swig_sources = 		\
	gsm_receiver_cf.cc	\
	gsm_receiver_config.cc	\
	viterbi_detector.cc

gsm_la_swig_libadd =		\
	$(LIBOSMOCORE_LIBS)	\
	decoder/libdecoder.la

include $(top_srcdir)/Makefile.swig

BUILT_SOURECS = $(swig_built_sources)

no_dist_file = $(swig_built_sources)

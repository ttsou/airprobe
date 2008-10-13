# $Id: Makefile.am,v 1.1.1.1 2007-06-01 04:26:57 jl Exp $

include $(top_srcdir)/Makefile.common

ourpythondir = $(pythondir)
ourlibdir    = $(pyexecdir)

INCLUDES = $(STD_DEFINES_AND_INCLUDES) $(PYTHON_CPPFLAGS)

SWIGCPPPYTHONARGS = -c++ -python $(PYTHON_CPPFLAGS) -I$(swigincludedir) -I$(grincludedir)

LOCAL_IFILES = gssm.i                         
NON_LOCAL_IFILES = $(GNURADIO_CORE_INCLUDEDIR)/swig/gnuradio.i
ALL_IFILES = $(LOCAL_IFILES) $(NON_LOCAL_IFILES)             

BUILT_SOURCES = gssm_glue.cc gssm.py

ourpython_PYTHON = gssm.py

ourlib_LTLIBRARIES = _gssm.la

# source files that go into the shared library
_gssm_la_SOURCES =					\
			gssm_glue.cc			\
			gssm_sink.cc			\
			bursts.cc			\
			sch.cc				\
			cch.cc				\
			rr_decode.cc			\
			tun.cc				\
			display.cc

_gssm_la_LDFLAGS = -module -avoid-version

_gssm_la_LIBADD = $(PYTHON_LDFLAGS) -lstdc++

gssm_glue.cc gssm.py: gssm.i $(ALL_IFILES)
	$(SWIG) $(SWIGCPPPYTHONARGS) -module gssm -o gssm_glue.cc $<

grinclude_HEADERS = gssm_sink.h

swiginclude_HEADERS = $(LOCAL_IFILES)

MOSTLYCLEANFILES = $(BUILT_SOURCES) *.pyc

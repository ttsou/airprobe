include $(top_srcdir)/Makefile.common

# Install this stuff so that it ends up as the gnuradio.gsm module
# This usually ends up at:
#   ${prefix}/lib/python${python_version}/site-packages/gnuradio

ourpythondir = $(grpythondir)
ourlibdir    = $(grpyexecdir)

INCLUDES = $(STD_DEFINES_AND_INCLUDES) $(PYTHON_CPPFLAGS)

SWIGPYTHONARGS = $(SWIGPYTHONFLAGS) $(SWIGGRFLAGS)

ALL_IFILES = 				\
	$(LOCAL_IFILES)			\
	$(NON_LOCAL_IFILES)		

NON_LOCAL_IFILES =			\
	$(GNURADIO_CORE_INCLUDEDIR)/swig/gnuradio.i


LOCAL_IFILES = 				\
	$(top_srcdir)/src/lib/gsm.i				

# These files are built by SWIG.  The first is the C++ glue.
# The second is the python wrapper that loads the _gsm shared library
# and knows how to call our extensions.

BUILT_SOURCES = 			\
	gsm.cc				\
	gsm.py				

# This gets gsm.py installed in the right place
ourpython_PYTHON =			\
	gsm.py

ourlib_LTLIBRARIES = _gsm.la

# These are the source files that go into the shared library
_gsm_la_SOURCES = 			\
	gsm.cc				\
	common.cc			\
	sch.cc				\
	cch.cc				\
	data_out.c			\
	id_list.c			\
	fire_crc.cc			\
	interleave.cc			\
	gsm_run_bb.cc

# magic flags
_gsm_la_LDFLAGS = $(NO_UNDEFINED) -module -avoid-version

# link the library against some comon swig runtime code and the 
# c++ standard library
_gsm_la_LIBADD = 			\
	$(PYTHON_LDFLAGS)		\
	-lstdc++			

gsm.cc gsm.py: $(LOCAL_IFILES) $(ALL_IFILES)
	$(SWIG) $(SWIGPYTHONARGS) -module gsm -o gsm.cc $(LOCAL_IFILES)

# These headers get installed in ${prefix}/include/gnuradio
grinclude_HEADERS =			\
	gsm_run_bb.h common.h sch.h fire_crc.h cch.h common.h burst_types.h gsm_constants.h interleave.h id_list.h gsm_desc.h data_out.h

# These swig headers get installed in ${prefix}/include/gnuradio/swig
swiginclude_HEADERS = 			\
	$(LOCAL_IFILES)


MOSTLYCLEANFILES = $(BUILT_SOURCES) *.pyc

# Don't distribute output of swig
dist-hook:
	@for file in $(BUILT_SOURCES); do echo $(RM) $(distdir)/$$file; done
	@for file in $(BUILT_SOURCES); do $(RM) $(distdir)/$$file; done

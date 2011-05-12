ACLOCAL_AMFLAGS = -I config

include $(top_srcdir)/Makefile.common

SUBDIRS = config src
DIST_SUBDIRS = config src

EXTRA_DIST = \
	bootstrap configure config.h.in \
	Makefile.swig Makefile.swig.gen.t \
	config.h.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gsm-receiver.pc

# Python plugin loader

loaderdir = $(libdir)/libpeas-1.0/loaders

AM_CPPFLAGS = \
	-I$(top_srcdir)			\
	$(PEAS_CFLAGS)			\
	$(GCOV_CFLAGS)			\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)		\
	$(PYGOBJECT_CFLAGS)		\
	$(PYTHON2_CFLAGS)		\
	-DPEAS_PYEXECDIR=\""$(PYTHON2_PYEXECDIR)"\" \
	-DPEAS_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libpythonloader.la

libpythonloader_la_SOURCES = \
	peas-plugin-loader-python.c 	\
	peas-plugin-loader-python.h

libpythonloader_la_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)		\
	$(GCOV_LDFLAGS)			\
	$(PYTHON2_LDFLAGS)

libpythonloader_la_LIBADD = \
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS)				\
	$(PYGOBJECT_LIBS)			\
	$(PYTHON2_LIBS)

gcov_sources = $(libpythonloader_la_SOURCES)
include $(top_srcdir)/Makefile.gcov

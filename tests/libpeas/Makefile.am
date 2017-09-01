include $(top_srcdir)/tests/Makefile.tests

SUBDIRS = introspection plugins testing

AM_CPPFLAGS = \
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	-I$(srcdir)/../testing-util	\
	$(PEAS_CFLAGS)			\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)

noinst_PROGRAMS = $(TEST_PROGS)

LDADD = \
	$(PEAS_LIBS)				\
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	./introspection/libintrospection-1.0.la	\
	./testing/libtesting.la			\
	../testing-util/libtesting-util.la

TEST_PROGS += \
	engine		\
	extension-c	\
	extension-set	\
	plugin-info

extension_c_LDADD = $(LDADD) $(builddir)/plugins/embedded/libembedded.la

if ENABLE_LUA51
TEST_PROGS               += extension-lua51
extension_lua51_SOURCES   = extension-lua.c
extension_lua51_CFLAGS    = $(LUA51_CFLAGS)
extension_lua51_LDADD     = $(LDADD) $(LUA51_LIBS)
endif

if ENABLE_LUA52
TEST_PROGS               += extension-lua52
extension_lua52_SOURCES   = extension-lua.c
extension_lua52_CFLAGS    = $(LUA52_CFLAGS)
extension_lua52_LDADD     = $(LDADD) $(LUA52_LIBS)
endif

if ENABLE_LUA53
TEST_PROGS               += extension-lua53
extension_lua53_SOURCES   = extension-lua.c
extension_lua53_CFLAGS    = $(LUA53_CFLAGS)
extension_lua53_LDADD     = $(LDADD) $(LUA53_LIBS)
endif

if ENABLE_PYTHON2
TEST_PROGS               += extension-python
extension_python_SOURCES  = extension-py.c
extension_python_CFLAGS   = $(PYGOBJECT_CFLAGS) $(PYTHON2_CFLAGS)
extension_python_LDADD    = $(LDADD) $(PYGOBJECT_LIBS) $(PYTHON2_LIBS)
endif

if ENABLE_PYTHON3
TEST_PROGS                += extension-python3
extension_python3_SOURCES  = extension-py.c
extension_python3_CFLAGS   = $(PYGOBJECT_CFLAGS) $(PYTHON3_CFLAGS)
extension_python3_LDADD    = $(LDADD) $(PYGOBJECT_LIBS) $(PYTHON3_LIBS)
endif

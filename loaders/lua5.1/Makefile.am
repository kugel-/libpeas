# Lua 5.1 plugin loader

SUBDIRS = resources

loaderdir = $(libdir)/libpeas-1.0/loaders

AM_CPPFLAGS = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(GCOV_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

AM_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)		\
	$(GCOV_LDFLAGS)

lua_la_SOURCES = \
	peas-lua-internal.c		\
	peas-lua-internal.h		\
	peas-plugin-loader-lua.c	\
	peas-plugin-loader-lua.h	\
	peas-lua-utils.c		\
	peas-lua-utils.h                \
	resources/peas-lua-resources.c

lua_la_LIBADD = \
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS)

loader_LTLIBRARIES =
if ENABLE_LUA51
loader_LTLIBRARIES += liblua51loader.la
endif
if ENABLE_LUA52
loader_LTLIBRARIES += liblua52loader.la
endif
if ENABLE_LUA53
loader_LTLIBRARIES += liblua53loader.la
endif

liblua51loader_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(LUA51_CFLAGS)

liblua51loader_la_LIBADD = \
	$(lua_la_LIBADD) \
	$(LUA51_LIBS)

liblua51loader_la_SOURCES = $(lua_la_SOURCES)

liblua52loader_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(LUA52_CFLAGS)

liblua52loader_la_LIBADD = \
	$(lua_la_LIBADD) \
	$(LUA52_LIBS)

liblua52loader_la_SOURCES = $(lua_la_SOURCES)

liblua53loader_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(LUA53_CFLAGS)

liblua53loader_la_LIBADD = \
	$(lua_la_LIBADD) \
	$(LUA53_LIBS)

liblua53loader_la_SOURCES = $(lua_la_SOURCES)

gcov_sources = $(lua_la_SOURCES)
include $(top_srcdir)/Makefile.gcov

# Ruby plugin loader

loaderdir = $(libdir)/libpeas-1.0/loaders

AM_CPPFLAGS = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(GCOV_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)	\
	$(RUBY_CFLAGS)

loader_LTLIBRARIES = librbffiloader.la

librbffiloader_la_CFLAGS = $(AM_CFLAGS) -Wno-strict-prototypes

librbffiloader_la_SOURCES = \
	peas-plugin-loader-rbffi.c \
	peas-rbffi-resources.c

librbffiloader_la_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)		\
	$(GCOV_LDFLAGS)

librbffiloader_la_LIBADD = \
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS)				\
	$(RUBY_LIBS)

loader_resources_deps = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/peas-rbffi.gresource.xml)
loader_resources_c_deps = $(srcdir)/peas-rbffi.gresource.xml $(loader_resources_deps)

peas-rbffi-resources.c: $(loader_resources_c_deps)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES)		\
	--target="$@"					\
	--sourcedir="$(srcdir)"				\
	--generate-source				\
	--internal					\
	"$(srcdir)/peas-rbffi.gresource.xml"

EXTRA_DIST = \
	$(loader_resources_c_deps)

gcov_sources = $(librbffiloader_la_SOURCES)
include $(top_srcdir)/Makefile.gcov

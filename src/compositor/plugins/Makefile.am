
pkglibdir = $(UKWM_PLUGIN_DIR)

AM_CPPFLAGS = \
	$(UKWM_CFLAGS)						\
	-I$(top_builddir)/src						\
	-I$(top_srcdir)/src						\
	-I$(top_srcdir)/cogl						\
	-I$(top_builddir)/cogl						\
	-I$(top_builddir)/cogl/cogl					\
	-I$(top_srcdir)/clutter						\
	-I$(top_builddir)/clutter					\
	-I$(top_builddir)/clutter/clutter				\
	-DUKWM_LIBEXECDIR=\"$(libexecdir)\"				\
	-DUKWM_LOCALEDIR=\"$(localedir)\"				\
	-DUKWM_PKGDATADIR=\"$(pkgdatadir)\"				\
	-DUKWM_DATADIR=\"$(datadir)\"					\
	-DG_LOG_DOMAIN=\"ukwm\"					\
	-DSN_API_NOT_YET_FROZEN=1					\
	-DUKWM_PLUGIN_DIR=\"$(UKWM_PLUGIN_DIR)\"

default_la_CFLAGS   = -fPIC
default_la_SOURCES  = default.c
default_la_LDFLAGS  = -module -avoid-version -no-undefined
default_la_LIBADD   = $(CLUTTER_LIBS)

pkglib_LTLIBRARIES = default.la

# post-install hook to remove the .la and .a files we are not interested in
# (There is no way to stop libtool generating static libs locally, and we
# cannot do this globally because of libukwm-private.so).
install-exec-hook:
	-rm -f $(DESTDIR)$(pkglibdir)/*.a
	-rm -f $(DESTDIR)$(pkglibdir)/*.la

# Since we removed the .la file, 'make uninstall' doesn't work properly,
# since it counts on libtool to remove the .la files, so just kill the
# .so file manually.
uninstall-local:
	-rm -f $(DESTDIR)$(pkglibdir)/default.so

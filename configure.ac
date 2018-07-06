AC_PREREQ(2.62)

m4_define([ukwm_major_version], [1])
m4_define([ukwm_minor_version], [1])
m4_define([ukwm_micro_version], [4])

m4_define([ukwm_version],
          [ukwm_major_version.ukwm_minor_version.ukwm_micro_version])

m4_define([ukwm_plugin_api_version], [3])

m4_define([libukwm_api_version], [1])

AC_INIT([ukwm], [ukwm_version],
        [https://github.com/ukui/ukwm/issues])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR(src/core/display.c)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_SUBDIRS([cogl clutter])

AM_INIT_AUTOMAKE([1.11 foreign no-dist-gzip dist-xz tar-ustar subdir-objects])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],)
AM_MAINTAINER_MODE([enable])

AC_GNU_SOURCE

UKWM_MAJOR_VERSION=ukwm_major_version
UKWM_MINOR_VERSION=ukwm_minor_version
UKWM_MICRO_VERSION=ukwm_micro_version
UKWM_VERSION=ukwm_version
UKWM_PLUGIN_API_VERSION=ukwm_plugin_api_version
LIBUKWM_API_VERSION=libukwm_api_version
AC_SUBST(UKWM_MAJOR_VERSION)
AC_SUBST(UKWM_MINOR_VERSION)
AC_SUBST(UKWM_MICRO_VERSION)
AC_SUBST(UKWM_PLUGIN_API_VERSION)
AC_SUBST(UKWM_VERSION)
AC_SUBST(LIBUKWM_API_VERSION)

# Make the ukwm versions visible to the cogl and clutter subdirs
export LIBUKWM_API_VERSION UKWM_VERSION

UKWM_PLUGIN_DIR="$libdir/$PACKAGE/plugins"
AC_SUBST(UKWM_PLUGIN_DIR)

# Honor aclocal flags
AC_SUBST(ACLOCAL_AMFLAGS, "\${ACLOCAL_FLAGS}")

GETTEXT_PACKAGE=ukwm
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[Name of default gettext domain])

AM_GNU_GETTEXT_VERSION([0.19.6])
AM_GNU_GETTEXT([external])

LT_PREREQ([2.2.6])
LT_INIT([disable-static])

# Debian / Ubuntu set this flag to 'no' in libtool, causing linking errors
# (i.e when linking against ukwm-clutter). Not to explicitly redefine such
# deps, we enable this flag for everybody.
link_all_deplibs=yes
link_all_deplibs_CXX=yes

AC_PROG_CC
AC_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_SED
AC_HEADER_STDC
PKG_PROG_PKG_CONFIG([0.21])

# Sets GLIB_GENMARSHAL and GLIB_MKENUMS
AM_PATH_GLIB_2_0([2.53.2])

CANBERRA_GTK=libcanberra-gtk3
CANBERRA_GTK_VERSION=0.26

LIBWACOM_VERSION=0.13

UKWM_PC_MODULES="
   egl
   gtk+-3.0 >= 3.19.8
   gio-unix-2.0 >= 2.35.1
   pango >= 1.2.0
   cairo >= 1.10.0
   gsettings-desktop-schemas >= 3.21.4
   json-glib-1.0
   upower-glib >= 0.99.0
   gnome-desktop-3.0
   xcomposite >= 0.2
   xcursor
   xdamage
   xext
   xfixes
   xi >= 1.6.0
   xkbfile
   xkeyboard-config
   xkbcommon >= 0.4.3
   xkbcommon-x11
   xrender
   x11-xcb
   xcb-randr
   xcb-res
"

GLIB_GSETTINGS

AC_ARG_ENABLE(verbose-mode,
  AC_HELP_STRING([--disable-verbose-mode],
                 [disable ukwm's ability to do verbose logging, for embedded/size-sensitive custom builds]),,
  enable_verbose_mode=yes)

if test x$enable_verbose_mode = xyes; then
    AC_DEFINE(WITH_VERBOSE_MODE,1,[Build with verbose mode support])
fi

AC_ARG_ENABLE(sm,
  AC_HELP_STRING([--disable-sm],
                 [disable ukwm's session management support, for embedded/size-sensitive custom non-GNOME builds]),,
  enable_sm=auto)

AC_ARG_ENABLE(startup-notification,
  AC_HELP_STRING([--disable-startup-notification],
                 [disable ukwm's startup notification support, for embedded/size-sensitive custom non-GNOME builds]),,
  enable_startup_notification=auto)

AC_ARG_WITH(libcanberra,
  AC_HELP_STRING([--without-libcanberra],
                 [disable the use of libcanberra for playing sounds]),,
  with_libcanberra=auto)

AC_ARG_WITH(libwacom,
  AC_HELP_STRING([--without-libwacom],
                 [disable the use of libwacom for advanced tablet management]),,
  with_libwacom=auto)

AC_ARG_WITH(gudev,
  AC_HELP_STRING([--without-gudev],
                 [disable the use of gudev for device type detection]),,
  with_gudev=auto)

AC_ARG_WITH([xwayland-path],
            [AS_HELP_STRING([--with-xwayland-path], [Absolute path for an X Wayland server])],
            [XWAYLAND_PATH="$withval"],
            [XWAYLAND_PATH="$bindir/Xwayland"])

AC_ARG_ENABLE(installed_tests,
              AS_HELP_STRING([--enable-installed-tests],
                             [Install test programs (default: no)]),,
              [enable_installed_tests=no])
AM_CONDITIONAL(BUILDOPT_INSTALL_TESTS, test x$enable_installed_tests = xyes)

## here we get the flags we'll actually use

# Unconditionally use this dir to avoid a circular dep with gnomecc
GNOME_KEYBINDINGS_KEYSDIR="${datadir}/gnome-control-center/keybindings"
AC_SUBST(GNOME_KEYBINDINGS_KEYSDIR)

STARTUP_NOTIFICATION_VERSION=0.7
AC_MSG_CHECKING([Startup notification library >= $STARTUP_NOTIFICATION_VERSION])
if $PKG_CONFIG --atleast-version $STARTUP_NOTIFICATION_VERSION libstartup-notification-1.0; then
   have_startup_notification=yes
else
   have_startup_notification=no
fi
AC_MSG_RESULT($have_startup_notification)

if test x$enable_startup_notification = xyes; then
   have_startup_notification=yes
   echo "startup-notification support forced on"
elif test x$enable_startup_notification = xauto; then
   true
else
   have_startup_notification=no
fi

if test x$have_startup_notification = xyes; then
  echo "Building with libstartup-notification"
  UKWM_PC_MODULES="$UKWM_PC_MODULES libstartup-notification-1.0 >= $STARTUP_NOTIFICATION_VERSION"
  AC_DEFINE(HAVE_STARTUP_NOTIFICATION, , [Building with startup notification support])
else
  echo "Building without libstartup-notification"
fi

have_libcanberra=no
AC_MSG_CHECKING([libcanberra-gtk])
if test x$with_libcanberra = xno ; then
  AC_MSG_RESULT([disabled])
else
  if $PKG_CONFIG --exists $CANBERRA_GTK '>=' $CANBERRA_GTK_VERSION; then
    have_libcanberra=yes
    AC_MSG_RESULT(yes)
    UKWM_PC_MODULES="$UKWM_PC_MODULES $CANBERRA_GTK"
    AC_DEFINE([HAVE_LIBCANBERRA], 1, [Building with libcanberra for playing sounds])
  else
    AC_MSG_RESULT(no)
    if test x$with_libcanberra = xyes ; then
      AC_MSG_ERROR([libcanberra forced and libcanberra-gtk was not found])
    fi
  fi
fi

have_libwacom=no
AC_MSG_CHECKING([libwacom])
if test x$with_libwacom = xno ; then
  AC_MSG_RESULT([disabled])
else
  if $PKG_CONFIG --exists libwacom '>=' $LIBWACOM_VERSION; then
    have_libwacom=yes
    AC_MSG_RESULT(yes)
    UKWM_PC_MODULES="$UKWM_PC_MODULES libwacom"
    AC_DEFINE([HAVE_LIBWACOM], 1, [Building with libwacom for advanced tablet management])
  else
    AC_MSG_RESULT(no)
    if test x$with_libwacom = xyes ; then
      AC_MSG_ERROR([libwacom forced but not found])
    fi
  fi
fi

have_gudev=no
AC_MSG_CHECKING([gudev])
if test x$with_gudev = xno ; then
  AC_MSG_RESULT([disabled])
else
  if $PKG_CONFIG --exists "gudev-1.0 >= 232"; then
    have_gudev=yes
    AC_MSG_RESULT(yes)
    UKWM_PC_MODULES="$UKWM_PC_MODULES gudev-1.0 >= 232"
    AC_DEFINE([HAVE_LIBGUDEV], 1, [Building with gudev for device type detection])
  else
    AC_MSG_RESULT(no)
    if test x$with_gudev = xyes ; then
      AC_MSG_ERROR([gudev forced but not found])
    fi
  fi
fi

AC_ARG_ENABLE(remote-desktop,
  AS_HELP_STRING([--enable-remote-desktop], [enable support for remote desktop and screen cast]),
  enable_remote_desktop=yes,
  enable_remote_desktop=no
)
AS_IF([test "$enable_remote_desktop" = "yes"], [
  UKWM_PC_MODULES="$UKWM_PC_MODULES libpipewire-0.1 >= 0.1.4"
  PKG_CHECK_EXISTS([libpipewire-0.1], [
    pw_micro=`$PKG_CONFIG --modversion libpipewire-0.1 | cut -d. -f3`
    AC_DEFINE_UNQUOTED([PIPEWIRE_VERSION_MICRO],[$pw_micro], [Pipewire micro version used])
  ])
  AC_DEFINE([HAVE_REMOTE_DESKTOP],[1], [Defined if screen cast and remote desktop support is enabled])
])
AM_CONDITIONAL([HAVE_REMOTE_DESKTOP],[test "$enable_remote_desktop" = "yes"])

INTROSPECTION_VERSION=0.9.5
GOBJECT_INTROSPECTION_CHECK([$INTROSPECTION_VERSION])

if test x$found_introspection != xno; then
  AC_DEFINE(HAVE_INTROSPECTION, 1, [Define if GObject introspection is available])
  UKWM_PC_MODULES="$UKWM_PC_MODULES gobject-introspection-1.0"
fi

AC_SUBST(XWAYLAND_PATH)

PKG_CHECK_MODULES(UKWM, $UKWM_PC_MODULES)

UKWM_NATIVE_BACKEND_MODULES="libdrm libsystemd libinput >= 1.4 gudev-1.0 gbm >= 10.3"

AC_ARG_ENABLE(native-backend,
  AS_HELP_STRING([--disable-native-backend], [disable ukwm native (KMS) backend]),,
  enable_native_backend=auto
)
have_native_backend="no"
AS_IF([test "$enable_native_backend" = "yes"], [have_native_backend=yes],
      [test "$enable_native_backend" = "auto"], PKG_CHECK_EXISTS([$UKWM_NATIVE_BACKEND_MODULES], [have_native_backend=yes]))

AS_IF([test "$have_native_backend" = "yes"], [
  PKG_CHECK_MODULES([UKWM_NATIVE_BACKEND], [$UKWM_NATIVE_BACKEND_MODULES])
  AC_DEFINE([HAVE_NATIVE_BACKEND],[1], [Define if you want to enable the native (KMS) backend based on systemd])
])
AM_CONDITIONAL([HAVE_NATIVE_BACKEND],[test "$have_native_backend" = "yes"])

AC_ARG_ENABLE(egl-device,
  AS_HELP_STRING([--enable-egl-device], [enable support for EGLDevice on top of KMS]),
  enable_egl_device=yes,
  enable_egl_device=no
)
AS_IF([test "$enable_egl_device" = "yes"], [
  AC_DEFINE([HAVE_EGL_DEVICE],[1], [Defined if EGLDevice support is enabled])
])

UKWM_WAYLAND_MODULES="wayland-server >= 1.13.0"

AC_ARG_ENABLE(wayland,
  AS_HELP_STRING([--disable-wayland], [disable ukwm on wayland support]),,
  enable_wayland=auto
)
AS_IF([test "$enable_wayland" = "yes"], [have_wayland=yes],
      [test "$enable_wayland" = "auto"], PKG_CHECK_EXISTS([$UKWM_WAYLAND_MODULES], [have_wayland=yes]))

AS_IF([test "$have_wayland" = "yes"], [
  PKG_CHECK_MODULES([UKWM_WAYLAND], [$UKWM_WAYLAND_MODULES])
  AC_PATH_PROG([WAYLAND_SCANNER],[wayland-scanner],[no])
  AS_IF([test $WAYLAND_SCANNER = "no"],
    [AC_MSG_ERROR([Could not find wayland-scanner in your PATH, required for parsing wayland extension protocols])])
  AC_SUBST([WAYLAND_SCANNER])
  AC_DEFINE([HAVE_WAYLAND],[1],[Define if you want to enable Wayland support])

  PKG_CHECK_MODULES(WAYLAND_PROTOCOLS, [wayland-protocols >= 1.9],
		    [ac_wayland_protocols_pkgdatadir=`$PKG_CONFIG --variable=pkgdatadir wayland-protocols`])
  AC_SUBST(WAYLAND_PROTOCOLS_DATADIR, $ac_wayland_protocols_pkgdatadir)
])
AM_CONDITIONAL([HAVE_WAYLAND],[test "$have_wayland" = "yes"])

PKG_CHECK_EXISTS([xi >= 1.6.99.1],
                 AC_DEFINE([HAVE_XI23],[1],[Define if you have support for XInput 2.3 or greater]))

AC_PATH_XTRA

ALL_X_LIBS="$X_LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS"

# Check for Xinerama extension - we only support the "XFree86" style,
# and not the older Solaris-only version; recent Solaris supports the
# XFree86 style.
ukwm_save_cppflags="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $X_CFLAGS"

have_xinerama=yes
AC_CHECK_LIB(Xinerama, XineramaQueryExtension,
    [AC_CHECK_HEADER(X11/extensions/Xinerama.h,
		     [X_EXTRA_LIBS="-lXinerama $X_EXTRA_LIBS"
		     if test -z "`echo $ALL_X_LIBS | grep "\-lXext" 2> /dev/null`"; then
			 X_EXTRA_LIBS="-lXext $X_EXTRA_LIBS"
		     fi],
		     have_xinerama=no,
		     [#include <X11/Xlib.h>])],
    have_xinerama=no, -lXext $ALL_X_LIBS)
AC_MSG_CHECKING(for Xinerama support)
AC_MSG_RESULT($have_xinerama)

CPPFLAGS="$ukwm_save_cppflags"

if test x$have_xinerama = xno; then
   AC_MSG_ERROR([Xinerama extension was not found])
fi

AC_DEFINE_UNQUOTED([XKB_BASE], ["`$PKG_CONFIG --variable xkb_base xkeyboard-config`"],
                               [XKB base dir])

RANDR_LIBS=
found_randr=no
AC_CHECK_LIB(Xrandr, XRRUpdateConfiguration,
               [AC_CHECK_HEADER(X11/extensions/Xrandr.h,
                                RANDR_LIBS=-lXrandr found_randr=yes,,
				[#include <X11/Xlib.h>])],
               , -lXext $ALL_X_LIBS)

if test "x$found_randr" = "xyes"; then
   AC_DEFINE(HAVE_RANDR, , [Have the Xrandr extension library])
   PKG_CHECK_EXISTS([xrandr >= 1.5.0],
                 AC_DEFINE([HAVE_XRANDR15],[1],[Define if you have support for XRandR 1.5 or greater]))
fi

UKWM_LIBS="$UKWM_LIBS $RANDR_LIBS $X_LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS -lm"

found_sm=no
case "$UKWM_LIBS" in
 *-lSM*)
    found_sm=yes
    ;;
 *)
  AC_CHECK_LIB(SM, SmcSaveYourselfDone,
               [AC_CHECK_HEADERS(X11/SM/SMlib.h,
                                UKWM_LIBS="-lSM -lICE $UKWM_LIBS" found_sm=yes)],
               , $UKWM_LIBS)
    ;;
esac

if test x$enable_sm = xno; then
   found_sm=no
fi

if test x$enable_sm = xyes; then
   if test "$found_sm" = "no"; then
      AC_MSG_ERROR([--enable-sm forced and -lSM not found])
      exit 1
   fi
fi

if test "$found_sm" = "yes"; then
   AC_DEFINE(HAVE_SM, , [Building with SM support])
fi

AM_CONDITIONAL(HAVE_SM, test "$found_sm" = "yes")

AC_PATH_PROG(ZENITY, zenity, no)
if test x"$ZENITY" = xno; then
  AC_MSG_ERROR([zenity not found in your path - needed for dialogs])
fi

AC_ARG_ENABLE(debug,
	[  --enable-debug		enable debugging],,
	enable_debug=no)
if test "x$enable_debug" = "xyes"; then
	CFLAGS="$CFLAGS -g -O"
fi

AC_CHECK_DECL([GL_EXT_x11_sync_object],
              [],
              [AC_MSG_ERROR([GL_EXT_x11_sync_object definition not found, please update your GL headers])],
              [#include <GL/glx.h>])

AC_PATH_PROG([CVT],[cvt],[])

#### Warnings (last since -Werror can disturb other tests)

# Stay command-line compatible with the gnome-common configure option. Here
# minimum/yes/maximum are the same, however.
AC_ARG_ENABLE(compile_warnings,
  AS_HELP_STRING([--enable-compile-warnings=@<:@no/minimum/yes/maximum/error@:>@],[Turn on compiler warnings]),,
  enable_compile_warnings=error)

changequote(,)dnl
if test "$enable_compile_warnings" != no ; then
  if test "x$GCC" = "xyes"; then
    case " $CFLAGS " in
    *[\ \	]-Wall[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wall" ;;
    esac

#   case " $CFLAGS " in
#   *[\ \	]-Wshadow[\ \	]*) ;;
#   *) CFLAGS="$CFLAGS -Wshadow" ;;
#   esac

    case " $CFLAGS " in
    *[\ \	]-Wchar-subscripts[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wchar-subscripts" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-Wmissing-declarations[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wmissing-declarations" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-Wmissing-prototypes[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wmissing-prototypes" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-Wnested-externs[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wnested-externs" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-Wpointer-arith[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wpointer-arith" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-Wcast-align[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wcast-align" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-Wsign-compare[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wsign-compare" ;;
    esac

    if test "$enable_compile_warnings" = error; then
      case " $CFLAGS " in
      *[\ \	]-Werror[\ \	]*) ;;
      *) CFLAGS="$CFLAGS -Werror -Wno-error=deprecated-declarations" ;;
      esac
    fi
  fi
fi
changequote([,])dnl

AC_CONFIG_FILES([
Makefile
data/Makefile
doc/Makefile
doc/man/Makefile
src/Makefile
src/libukwm-$LIBUKWM_API_VERSION.pc:src/libukwm.pc.in
src/compositor/plugins/Makefile
src/meta/meta-version.h
po/Makefile.in
])

AC_OUTPUT

# Check that cogl and clutter have their .pc files regenerated with the correct version

COGL_PC_VERSION=$(grep Version: cogl/cogl/ukwm-cogl-$LIBUKWM_API_VERSION.pc|awk '{ print $2; }')
AS_IF([test "x$COGL_PC_VERSION" != "x$UKWM_VERSION"],
      [AC_MSG_ERROR([ukwm-cogl pkg-config file not updated, rerun ./configure])])

CLUTTER_PC_VERSION=$(grep Version: clutter/clutter/ukwm-clutter-$LIBUKWM_API_VERSION.pc|awk '{ print $2; }')
AS_IF([test "x$CLUTTER_PC_VERSION" != "x$UKWM_VERSION"],
      [AC_MSG_ERROR([ukwm-clutter pkg-config file not updated, rerun ./configure])])

if test x$enable_verbose_mode = xno; then
        echo "*** WARNING WARNING WARNING WARNING WARNING"
        echo "*** Building without verbose mode"
        echo "*** This means there's no way to debug ukwm problems."
        echo "*** Please build normal desktop versions of ukwm"
        echo "*** with verbose mode enabled so users can use it when they report bugs."
fi

dnl ==========================================================================
echo "
ukwm-$VERSION

	prefix:                   ${prefix}
	source code location:	  ${srcdir}
	compiler:		  ${CC}

	Startup notification:     ${have_startup_notification}
	libcanberra:              ${have_libcanberra}
	libwacom:                 ${have_libwacom}
	gudev                     ${have_gudev}
	Introspection:            ${found_introspection}
	Session management:       ${found_sm}
	Wayland:                  ${have_wayland}
	Native (KMS) backend:     ${have_native_backend}
	EGLDevice:                ${enable_egl_device}
	Remote desktop:           ${enable_remote_desktop}
"


UKWM_MINOR_VERSION=ukwm_minor_version
if expr $UKWM_MINOR_VERSION % 2 > /dev/null ; then
   stable_version=`expr $UKWM_MINOR_VERSION - 1`
   echo "This is the UNSTABLE branch of ukwm"
   echo -n "Use 3.$stable_version.x for stable "
   echo    "(gnome-3-$stable_version branch in git)"
else
   echo "This is the stable branch of ukwm"
fi

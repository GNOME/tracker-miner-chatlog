AC_INIT([tracker-miner-chatlog],
        [0.1.0],
        [http://bugzilla.gnome.org/],
        [tracker-miner-media],
        [https://wiki.gnome.org/Projects/Tracker])
AC_PREREQ(2.63)

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_INIT_AUTOMAKE([dist-xz no-dist-gzip tar-ustar])
AM_MAINTAINER_MODE([enable])

AC_PROG_CC
AC_PROG_CC_C_O
AC_HEADER_STDC

PKG_PROG_PKG_CONFIG

LT_INIT([])

# i18n support
GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, ["$GETTEXT_PACKAGE"], [Define to the Gettext package name])
AC_SUBST(GETTEXT_PACKAGE)
IT_PROG_INTLTOOL([0.40.0])

# dependencies
PKG_CHECK_MODULES(DEPS, [tracker-miner-1.0 telepathy-glib])

# optional dependencies
AC_ARG_ENABLE(migration,
              AS_HELP_STRING([--enable-migration],
                             [enable migration from telepathy-logger]),
              [enable_migration=$enableval],
              [enable_migration=yes])

if test "x$enable_migration" != "xno" ; then
   PKG_CHECK_MODULES(LOGGER,
		     [telepathy-logger-0.2],
                     [have_logger=yes],
                     [have_logger=no])

   AC_SUBST(LOGGER_CFLAGS)
   AC_SUBST(LOGGER_LIBS)
   AM_CONDITIONAL(HAVE_MIGRATION, test "$have_logger" = "yes")

   if test "$have_logger" != "no" ; then
      AC_DEFINE(HAVE_MIGRATION, [], [Define if we have telepathy-logger migration])
   fi
else
   have_logger=no
fi

AC_CONFIG_FILES([
Makefile
data/Makefile
src/Makefile
po/Makefile.in
])
AC_OUTPUT

echo "
     Telepathy-logger migration: $have_logger
"

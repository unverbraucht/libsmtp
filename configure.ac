dnl Process this file with autoconf to produce a configure script.
AC_INIT([libsmtp], [0.9.0])
AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_HEADER(config.h)

dnl satisfy automake
PACKAGE=libsmtp
CODENAME=None

dnl version number
MAJOR_VERSION=0
MINOR_VERSION=9
MICRO_VERSION=0
EXTRA_VERSION=0

AC_PROG_MAKE_SET

subdirs="smtp"

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB

dnl AM_PROG_LIBTOOL

dnl Checks for libraries
PKG_CHECK_MODULES([glib], [glib-2.0])
dnl AC_CHECK_LIBS

AC_ARG_ENABLE(mime,
	[  --enable-mime         compile libsmtp_mime [default=yes]],
	[ac_cv_enablemime=$enableval], [ac_cv_enablemime=yes])

AM_CONDITIONAL([WITH_MIME], [test "$ac_cv_enablemime" = yes])

if test "$ac_cv_enablemime" = yes; then
	subdirs="$subdirs mime"
	with_mime=yes
	addtl_flags="-DUSE_MIME"
	AC_DEFINE(WITH_MIME, 1, [Define if you want to enable support for MIME])
else
	AC_DEFINE(WITH_MIME, 0, [Define if you want to enable support for MIME])
fi

AC_ARG_ENABLE(debug,
	[  --enable-debug         compile with debug info [default=no]],
	[ac_cv_with_debug=$enableval], [ac_cv_with_debug=no])

AM_CONDITIONAL([LIBSMTP_DEBUG], [test "$ac_cv_with_debug" = yes])
AM_CONDITIONAL([LIBSMTP_MIME_DEBUG], [test "$ac_cv_with_debug" = yes])
if test "$ac_cv_with_debug" = yes; then
	AC_DEFINE([LIBSMTP_DEBUG], 1, [Define if you want to see debug output])
	AC_DEFINE([LIBSMTP_MIME_DEBUG], [1], [Define if you want to see debug output from MIME])
	addtl_flags="-DLIBSMTP_DEBUG -DLIBSMTP_MIME_DEBUG"
else
	AC_DEFINE([LIBSMTP_DEBUG], 0, [Define if you want to see debug output])
	AC_DEFINE([LIBSMTP_MIME_DEBUG], [0], [Define if you want to see debug output from MIME])
fi

subdirs="$subdirs examples"

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.
AC_CHECK_FUNCS(gethostname socket strstr)

AC_SUBST(subdirs)
AC_SUBST(with_mime)
AC_SUBST(addtl_flags)

AC_OUTPUT(mime/Makefile smtp/Makefile examples/Makefile Makefile, echo timestamp > stamp-h)

dnl Process this file with autoconf to produce a configure script.
AC_INIT(confdefs.h)

AC_CONFIG_HEADER(config.h)

dnl satisfy automake
PACKAGE=libsmtp
CODENAME=None

dnl version number
MAJOR_VERSION=0
MINOR_VERSION=7
MICRO_VERSION=9
EXTRA_VERSION=b1

AC_PROG_MAKE_SET

subdirs="smtp"

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB

dnl Checks for libraries.
dnl Replace `main' with a function in -lglib:
dnl AC_CHECK_LIB(glib, main)

AC_ARG_ENABLE(mime,
	[  --enable-mime         compile libsmtp_mime [default=yes]],
	[ac_cv_enablemime=$enableval], [ac_cv_enablemime=yes])

if test "$ac_cv_enablemime" = yes; then
	AC_DEFINE(WITH_MIME)
	subdirs="$subdirs mime"
	with_mime=yes
	addtl_flags="-DUSE_MIME"
fi

AC_ARG_ENABLE(debug,
	[  --enable-debug         compile with debug info [default=no]],
	[ac_cv_with_debug=$enableval], [ac_cv_with_debug=no])

if test "$ac_cv_with_debug" = yes; then
	AC_DEFINE(LIBSMTP_DEBUG, 1, [Define if you want to see debug output])
	AC_DEFINE(LIBSMTP_MIME_DEBUG)
	addtl_flags="-DLIBSMTP_DEBUG -DLIBSMTP_MIME_DEBUG"
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
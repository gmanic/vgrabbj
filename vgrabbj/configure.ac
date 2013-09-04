
dnl Copyright (C) 2000, 2001, 2002 Jens Gecius, Hannover, Germany
dnl eMail: devel@gecius.de
 
dnl Copyright (C) 2013 Ludovic Rousseau <ludovic.rousseau@free.fr>

dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at you option) any later version.
 
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
dnl GNU General Public License for more details.

dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307,
dnl USA  


dnl Process this file with autoconf to produce a configure script.
AC_INIT([vgrabbj], [0.9.6])
AC_CONFIG_SRCDIR([vgrabbj.c])

# For automake
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
AC_ARG_PROGRAM

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Options
AC_ARG_ENABLE(timestamp,
  [  --enable-timestamp      timestamp support])
AC_ARG_ENABLE(ftp,
  [  --enable-ftp            ftp support (automatic upload)])

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AM_PROG_CC_C_O

dnl Use -Wall if we have gcc.
changequote(,)dnl
if test "x$GCC" = "xyes"; then
   case " $CFLAGS " in
   *[\ \ ]-Wall[\ \ ]*) ;;
   *) CFLAGS="$CFLAGS -Wall" ;;
   esac
fi
changequote([,])dnl

dnl Checks for libraries and appropriate headers.

AC_CHECK_LIB(jpeg, jpeg_set_defaults,,
	AC_MSG_ERROR([*** jpeg library not found! Please install the appropriate library ***]))
AC_CHECK_HEADERS(jpeglib.h,, AC_MSG_ERROR([*** jpeglib.h not found! Please install the appropriate header file ***]))

AC_CHECK_LIB(m, main,,
	AC_MSG_ERROR([*** libm library not found! Please install the appropriate library ***]))

AC_CHECK_LIB(png, png_create_info_struct,,
	AC_MSG_ERROR([*** libpng library not found! Please install the appropriate library ***]))
AC_CHECK_HEADERS(png.h,, AC_MSG_ERROR([*** png.h not found! Please install the appropriate header file ***]))

if test "$enable_timestamp" != "no"; then
  AC_CHECK_LIB(ttf, TT_Open_Face,,
	AC_MSG_WARN([*** libttf library not found! Disabled timestamp feature ***]))
  AC_CHECK_HEADERS(freetype/freetype.h freetype1/freetype.h,ttf_h=1;break)
	if test "$ttf_h" != "1"; then
		AC_MSG_WARN([*** freetype.h not found! Timestamp may be disabled ***])
	fi
else
  echo "*** Timestamp disabled"
fi

AC_CHECK_LIB(z, crc32,,
	AC_MSG_ERROR([*** libz library not found! Please install the appropriate library ***]))

if test "$enable_ftp" != "no"; then
  AC_CHECK_LIB(ftp, FtpInit,,
	AC_MSG_WARN([*** ftp library not found! Disabled ftp feature ***]))
  AC_CHECK_HEADERS(ftplib.h,, 
	AC_MSG_WARN([*** ftplib.h not found! Disabled ftp feature ***]))
else
  echo "*** ftp disabled"
fi

AC_CHECK_LIB(z, crc32,,
	AC_MSG_ERROR([*** libz library not found! Please install the appropriate library ***]))


dnl Checks for other header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h syslog.h unistd.h linux/videodev.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_STRUCT_TM

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_STRFTIME

PKG_CHECK_MODULES([LIBV4L1], [libv4l1])

AC_DEFINE([SYSCONF_DIR], [], [default configuration file])
if test \"x${prefix}\" = \"xNONE\"; then
  AC_DEFINE_UNQUOTED(SYSCONF_DIR,"${ac_default_prefix}/etc/vgrabbj.conf")
else
  AC_DEFINE_UNQUOTED(SYSCONF_DIR,"${sysconfdir}/vgrabbj.conf")
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

# -*- autoconf -*-
# Process this file with autoconf to produce a configure script.

# Copyright (C) 1991-2019 Free Software Foundation, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

dnl Written by Jim Meyering and Sergey Sushilin.

AC_PREREQ([2.69])

# Make inter-release version strings look like, e.g., v6.9-219-g58ddd, which
# indicates that it is built from the 219th delta (in _some_ repository)
# following the v6.9 tag, and that 58ddd is a prefix of the commit SHA1.
# The explicit URL can be removed when autoconf > 2.69 is released.
AC_INIT([bfc],
        m4_esyscmd([build-aux/git-version-gen .tarball-version]),
        [],
        [bfc],
        [])

dnl What version of bfc are we building?
AC_MSG_NOTICE([configuring for bfc $PACKAGE_VERSION])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([lib/config.h:lib/config.hin])

AM_INIT_AUTOMAKE([1.11.2 no-dist-gzip dist-xz parallel-tests subdir-objects])
AM_SILENT_RULES([yes]) # make --enable-silent-rules the default.

dnl POSIXCHECK is worthwhile for maintainers, but adds several seconds
dnl (more than 10% execution time) to ./configure, with no benefit for
dnl most users.  Using it to look for bugs requires:
dnl   GNULIB_POSIXCHECK=1 autoreconf -f
dnl   ./configure
dnl   make
dnl   make -C src clean
dnl   make CFLAGS=-DGNULIB_POSIXCHECK=1
dnl FIXME: Once we figure out how to avoid false positives, we should
dnl have 'make my-distcheck' in dist-check.mk exercise this.
m4_syscmd([test "${GNULIB_POSIXCHECK+set}" = set])
m4_if(m4_sysval, [0], [], [dnl
gl_ASSERT_NO_GNULIB_POSIXCHECK])

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
gl_EARLY
gl_INIT

AC_FUNC_FORK

AC_ARG_ENABLE([debug],
              AS_HELP_STRING([--disable-debug],[make compile release version directly]),
              [enable_debug=${enableval}],[enable_debug=yes])

if test "${enable_debug}" == "yes"; then
  AC_DEFINE([_DEBUG], 1, [debug version is enabled])
  CFLAGS="-Og -ggdb"
else
  AC_DEFINE([NDEBUG], 1, [debug version is disabled])
  CFLAGS="$CFLAGS -s -ffunction-sections -fno-asynchronous-unwind-tables"
  if test "x${LDFLAGS}" == "x"; then
    LDFLAGS="-Wl,--gc-sections -Wl,--strip-all"
  else
    LDFLAGS="$LDFLAGS -Wl,--gc-sections -Wl,--strip-all"
  fi
LDFLAGS="$LDFLAGS -Wl,--gc-sections -Wl,--strip-all"
  AC_MSG_NOTICE([NOTE: all debug symbols will be removed])
fi

AC_CONFIG_FILES([Makefile
                 gnulib-tests/Makefile])

AC_OUTPUT

echo ""
echo "$0 finished at `date`"
AC_MSG_NOTICE([
Summary of build options:
  Package:          $PACKAGE_NAME
  Version:          $PACKAGE_VERSION
  Host type:        $host
  Host OS:          $host_os
  Install prefix:   $prefix
  Compiler:         $CC
  CFlags:           $CFLAGS $CPPFLAGS
  LDFlags:          $LDFLAGS
  Libs:             $LIBS])

echo "$0: done.  Now you can run 'make'."

# Make coreutils.					-*-Makefile-*-

# Copyright (C) 1990-2019 Free Software Foundation, Inc.

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

ALL_RECURSIVE_TARGETS =

SUBDIRS = . gnulib-tests

EXTRA_DIST =                            \
  .prev-version                         \
  .version                              \
  bootstrap                             \
  bootstrap.conf                        \
  dist-check.mk                         \
  maint.mk                              \
  thanks-gen

ACLOCAL_AMFLAGS = -I m4

# Shortcut targets to make it easier to run (very) expensive tests.
check-expensive:
	$(MAKE) check RUN_EXPENSIVE_TESTS=yes
check-very-expensive:
	$(MAKE) check-expensive RUN_VERY_EXPENSIVE_TESTS=yes

BUILT_SOURCES = .version
.version:
	$(AM_V_GEN)echo $(VERSION) > $@-t && mv $@-t $@

DISTCLEANFILES =
MAINTAINERCLEANFILES =

noinst_LIBRARIES =
MOSTLYCLEANFILES =
CLEANFILES =
MOSTLYCLEANDIRS =

AM_CPPFLAGS = -Ilib -I$(top_srcdir)/lib -Isrc -I$(top_srcdir)/src

include $(top_srcdir)/lib/local.mk
include $(top_srcdir)/src/local.mk

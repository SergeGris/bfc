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

#AM_CPPFLAGS += -pedantic -pedantic-errors -w -Wextra -Wall -Waggregate-return -Wc++-compat -Wcast-align -Wcast-qual \
-Wchar-subscripts -Wcomment -Wconversion -Wdisabled-optimization \
-Werror-implicit-function-declaration -Wfatal-errors -Wfloat-equal -Wformat -Wformat=2 -Wformat-nonliteral -Wformat-security \
-Wformat-y2k -Wimplicit -Wimplicit-function-declaration -Wimplicit-int -Wimport -Winit-self -Winline \
-Winvalid-pch -Wunsafe-loop-optimizations -Wlong-long -Wmain -Wmissing-braces -Wmissing-field-initializers \
-Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wpadded -Wparentheses -Wpointer-arith \
-Wredundant-decls -Wreturn-type -Wsequence-point -Wshadow -Wsign-compare -Wstack-protector -Wstrict-aliasing \
-Wstrict-aliasing=2 -Wswitch -Wswitch-default -Wswitch-enum -Wsystem-headers -Wtrigraphs -Wundef -Wuninitialized -Wunknown-pragmas \
-Wunreachable-code -Wunused -Wunused-function -Wunused-label -Wunused-parameter -Wunused-value -Wunused-variable -Wvariadic-macros \
-Wvolatile-register-var -Wwrite-strings -Wbad-function-cast -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wold-style-definition \
-Wstrict-prototypes -Wtraditional -Wdeclaration-after-statement -Wpointer-sign

include $(top_srcdir)/lib/local.mk
include $(top_srcdir)/src/local.mk

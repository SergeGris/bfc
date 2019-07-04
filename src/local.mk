# Make ng-coreutils programs.                             -*-Makefile-*-
# This is included by the top-level Makefile.am.

## Copyright (C) 1990-2019 Free Software Foundation, Inc.

## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

AM_CFLAGS = -pipe -march=native -fopenmp $(WERROR_CFLAGS)

EXTRA_PROGRAMS =                \
   $(no_install__progs)         \
   $(build_if_possible__progs)  \
   $(default__progs)

# The user can tweak these lists at configure time.
bin_PROGRAMS = @bin_PROGRAMS@

noinst_HEADERS =  \
  src/system.h

CLEANFILES += $(SCRIPTS)

# Also remove these sometimes-built programs.
# For example, even when excluded, they are built via 'sc_check-AUTHORS'
# or 'dist'.
CLEANFILES += $(no_install__progs)

noinst_LIBRARIES += src/libver.a
nodist_src_libver_a_SOURCES = src/version.c src/version.h

# Tell the linker to omit references to unused shared libraries.
AM_LDFLAGS = $(IGNORE_UNUSED_LIBRARIES_CFLAGS) -export-dynamic

# Extra libraries needed by more than one program.  Will be updated later.
copy_ldadd =
remove_ldadd =

# Sometimes, the expansion of $(LIBINTL) includes -lc which may
# include modules defining variables like 'optind', so libgnu.a
# must precede $(LIBINTL) in order to ensure we use GNU getopt.
# But libgnu.a must also follow $(LIBINTL), since libintl uses
# replacement functions defined in libgnu.a.
LDADD = src/libver.a lib/libgnu.a $(LIBINTL) lib/libgnu.a

src_bfc_LDADD = $(LDADD)

# Get the release year from lib/version-etc.c.
RELEASE_YEAR = \
  `sed -n '/.*COPYRIGHT_YEAR = \([0-9][0-9][0-9][0-9]\) };/s//\1/p' \
    $(top_srcdir)/lib/version-etc.c`

src_bfc_SOURCES  = compile_asm_i386_linux.c compiler.c main.c tokenizer.c
src_bfc_CPPFLAGS = $(AM_CPPFLAGS)

BUILT_SOURCES += src/version.c
src/version.c: Makefile
	$(AM_V_GEN)rm -f $@
	$(AM_V_at)${MKDIR_P} src
	$(AM_V_at)printf '\nconst char *Version = "$(PACKAGE_VERSION)";\n' > $@t
	$(AM_V_at)chmod a-w $@t
	$(AM_V_at)mv $@t $@

BUILT_SOURCES += src/version.h
src/version.h: Makefile
	$(AM_V_GEN)rm -f $@
	$(AM_V_at)${MKDIR_P} src
	$(AM_V_at)printf '\n' > $@t
	$(AM_V_at)printf '#ifndef _VERSION_H\n' >> $@t
	$(AM_V_at)printf '#define _VERSION_H 1\n\n' >> $@t
	$(AM_V_at)printf 'extern const char *Version;\n\n' >> $@t
	$(AM_V_at)printf '#endif /* _VERSION_H */\n' >> $@t
	$(AM_V_at)chmod a-w $@t
	$(AM_V_at)mv $@t $@

DISTCLEANFILES += src/version.c src/version.h
MAINTAINERCLEANFILES += $(BUILT_SOURCES)

all_programs = \
	$(bin_PROGRAMS) \
	$(bin_SCRIPTS) \
	$(EXTRA_PROGRAMS)

INSTALL = install -c

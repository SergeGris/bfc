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

# Have no read-only files in the tarball to allow easy removal.
# Have .tarball-version based versions only in tarball builds.
# Have .timestamp based dates only in tarball builds.
# The perl substitution is to change some key uses of "rm" to "/bin/rm".
# See the rm_subst comment for details.
# The touch avoids a subtle, spurious "make distcheck" failure.
dist-hook: gen-ChangeLog
	$(AM_V_GEN)chmod -R +rw $(distdir)
	$(AM_V_GEN)echo $(VERSION) > $(distdir)/.tarball-version
	$(AM_V_GEN)date +%s > $(distdir)/.timestamp
	$(AM_V_at)perl -pi -e '$(rm_subst)' $(distdir)/Makefile.in
	$(AM_V_at)touch $(distdir)/doc/constants.texi \
	  $(distdir)/doc/ng-coreutils.info

gen_start_ver = 0.4
.PHONY: gen-ChangeLog
gen-ChangeLog:
	$(AM_V_GEN)if test -d .git; then \
	  log_fix="$(srcdir)/build-aux/git-log-fix"; \
	  test -e "$$log_fix" \
	    && amend_git_log="--amend=$$log_fix" \
	    || amend_git_log=; \
	  $(top_srcdir)/build-aux/gitlog-to-changelog $$amend_git_log \
	    -- v$(gen_start_ver)~.. > $(distdir)/cl-t && \
	    { printf '\n\nSee the source repo for older entries\n' \
	      >> $(distdir)/cl-t && \
	      rm -f $(distdir)/ChangeLog && \
	      mv $(distdir)/cl-t $(distdir)/ChangeLog; } \
	fi

ALL_RECURSIVE_TARGETS += distcheck-hook
distcheck-hook: check-ls-dircolors
	$(MAKE) my-distcheck
	$(MAKE) taint-distcheck

DISTCLEANFILES = VERSION
MAINTAINERCLEANFILES = THANKS-to-translators
THANKS-to-translators: po/LINGUAS THANKStt.in
	$(AM_V_GEN)( \
	  cat $(srcdir)/THANKStt.in; \
	  for lang in `cat $(srcdir)/po/LINGUAS`; do \
	    echo https://translationproject.org/team/$$lang.html; \
	  done; \
	) > $@-tmp && mv $@-tmp $@

# Sort in traditional ASCII order, regardless of the current locale;
# otherwise we may get into trouble with distinct strings that the
# current locale considers to be equal.
ASSORT = LC_ALL=C sort

# Extract all lines up to the first one starting with "##".
prologue = perl -ne '/^\#\#/ and exit; print' $(srcdir)/THANKS.in

# FIXME: avoid dependency to build our own 'sort' for 'make dist' ...
# when common platforms have a functional case-folding implementation:
#   $ test 'abácad' = "$(printf '%s\n' 'ab' 'ác' 'ad' \
#                          | LC_ALL=en_US.UTF-8 sort -f \
#                          | tr -d '\n')" && echo GOOD || echo BAD
# Note we don't enable case folding (-f) in the sort below, due to bugs
# in the I18N patch used in many distros (as of 2015).  Also using our
# own src/sort here would induce awkward dependencies for `make dist`.
THANKS: THANKS.in Makefile.am .mailmap thanks-gen .version
	$(AM_V_GEN)rm -f $@-t $@; \
	{ \
	  $(prologue); echo; \
	  { perl -ne '/^$$/.../^$$/ and !/^$$/ and s/  +/\0/ and print' \
	      $(srcdir)/THANKS.in; \
	    git log --pretty=format:'%aN%x00%aE' \
	      | $(ASSORT) -u; \
	  } | $(srcdir)/thanks-gen \
	    | LC_ALL=en_US.UTF-8 sort -k1,1; \
	  echo; \
	  printf ';; %s\n' 'Local Variables:' 'coding: utf-8' End:; \
	} > $@-t && chmod a-w $@-t && mv $@-t $@

# Some of our git hook scripts are supposed to be identical to git's samples.
# See if they are still in sync.
.PHONY: check-git-hook-script-sync
check-git-hook-script-sync:
	@fail=0; \
	t=$$(mktemp -d) \
	  && cd $$t && git init -q && cd .git/hooks \
	  && for i in pre-commit pre-applypatch applypatch-msg; do \
	       diff $(abs_top_srcdir)/scripts/git-hooks/$$i $$i.sample || fail=1; \
	     done; \
	rm -rf $$t; \
	test $$fail = 0

noinst_LIBRARIES =
MOSTLYCLEANFILES =
CLEANFILES =
MOSTLYCLEANDIRS =

AM_CPPFLAGS = -Ilib -I$(top_srcdir)/lib -Isrc -I$(top_srcdir)/src

include $(top_srcdir)/lib/local.mk
include $(top_srcdir)/src/local.mk

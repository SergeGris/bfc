# Make bfc                             -*-Makefile-*-
# This is included by the top-level Makefile.am.

## Copyright (C) 2019 Free Software Foundation, Inc.

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


# ARCH = $(shell uname -m | sed -e 's/i.86/x86/' \
                                -e 's/x86_64/x86/' \
                                -e 's/sun4u/sparc64/' \
                                -e 's/arm.*/arm/' \
                                -e 's/sa110/arm/' \
                                -e 's/ppc.*/powerpc/' \
                                -e 's/mips.*/mips/' \
                                -e 's/aarch64.*/arm64/')

ARCH_DEPENDENT_SOURCES = src/arch/x86-asm.c

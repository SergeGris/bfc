
/* xstrndup.h

   Copyright (C) 2019 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Wriiten by Sergey Sushilin.  */

#ifndef _XSTRNDUP_H
#define _XSTRNDUP_H 1

#include <features.h>

char *xstrndup (const char *, size_t)
  __THROW __attribute_malloc__ __nonnull ((1)) __attribute__ ((__returns_nonnull__));

#endif /* _XSTRNDUP_H */

/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 *
 */

#ifndef CopyrightH
#define CopyrightH

#include "Unicode.h"

/*
 * This is the copyright string that must be displayed by any
 * Unitex program when called with no parameter.
 */
static unichar COPYRIGHT[256];

static int init_copyright() {
//u_sprintf(COPYRIGHT,"This program is part of Unitex 2.1%C version\nCopyright %C 2001-2010 Universit%C Paris-Est Marne-la-Vall%Ce\nContact: <unitex@univ-mlv.fr>\n\n",0x3B2,0xA9,0xE9,0xE9);
u_sprintf(COPYRIGHT,"This program is part of Unitex 2.1\nCopyright %C 2001-2011 Universit%C Paris-Est Marne-la-Vall%Ce\nContact: <unitex@univ-mlv.fr>\n\n",0xA9,0xE9,0xE9);
return 0;
}

static int foo_copyright=init_copyright();

#endif

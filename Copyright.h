/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#define UNITEX_MAJOR_VERSION_NUMBER 3
#define UNITEX_MINOR_VERSION_NUMBER 0

static int init_copyright() {
u_sprintf(COPYRIGHT,"This program is part of Unitex %d.%d%C version\nCopyright %C 2001-2012 Universit%C Paris-Est Marne-la-Vall%Ce\nContact: <unitex@univ-mlv.fr>\n\n",UNITEX_MAJOR_VERSION_NUMBER,UNITEX_MINOR_VERSION_NUMBER,0x3B2,0xA9,0xE9,0xE9);
//u_sprintf(COPYRIGHT,"This program is part of Unitex %d.%d\nCopyright %C 2001-2012 Universit%C Paris-Est Marne-la-Vall%Ce\nContact: <unitex@univ-mlv.fr>\n\n",UNITEX_MAJOR_VERSION_NUMBER,UNITEX_MINOR_VERSION_NUMBER,0xA9,0xE9,0xE9);
return 0;
}

static int foo_copyright=init_copyright();

#endif

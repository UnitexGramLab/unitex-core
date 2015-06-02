/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/*
 * This is the copyright string that must be displayed by any
 * Unitex program when called with no parameter.
 */


#define UNITEX_MAJOR_VERSION_NUMBER 3
#define UNITEX_MINOR_VERSION_NUMBER 1

#define UNITEX_HAVE_SYNCTOOL 1


#define STRINGIZE_COPYRIGHT_2(s) #s
#define STRINGIZE_COPYRIGHT(s) STRINGIZE_COPYRIGHT_2(s)
/*
static int init_copyright() {
u_sprintf(COPYRIGHT,"This program is part of Unitex %d.%d%C version\nCopyright %C 2001-2014 Universit%C Paris-Est Marne-la-Vall%Ce\nContact: <unitex@univ-mlv.fr>\n\n",UNITEX_MAJOR_VERSION_NUMBER,UNITEX_MINOR_VERSION_NUMBER,0x3B2,0xA9,0xE9,0xE9);
//u_sprintf(COPYRIGHT,"This program is part of Unitex %d.%d\nCopyright %C 2001-2014 Universit%C Paris-Est Marne-la-Vall%Ce\nContact: <unitex@univ-mlv.fr>\n\n",UNITEX_MAJOR_VERSION_NUMBER,UNITEX_MINOR_VERSION_NUMBER,0xA9,0xE9,0xE9);
return 0;
}*/

#define BETA_UTF8 "\xce\xb2" // unicode 03be
#define COPYRIGHT_UTF8 "\xc2\xa9" // unicode a9
#define E_ACUTE "\xc3\xa9" // unicode e9

static inline const char* get_copyright_utf8()
{
	return
		"This program is part of Unitex " STRINGIZE_COPYRIGHT(UNITEX_MAJOR_VERSION_NUMBER) "." STRINGIZE_COPYRIGHT(UNITEX_MINOR_VERSION_NUMBER) BETA_UTF8 " version\n"
		"Copyright " COPYRIGHT_UTF8 " 2001-2015 Universit" E_ACUTE " Paris-Est Marne-la-Vall" E_ACUTE "e\nContact: <unitex@univ-mlv.fr>\n\n";
}


static inline void display_copyright_notice() 
{
	unichar str[0x400];
	convert_utf8_to_unichar(str, 0x3ff, NULL, (const unsigned char*)get_copyright_utf8(), strlen(get_copyright_utf8())+1);

	u_printf("%S", str);
}


static inline void get_copyright_notice(unichar*dest, size_t nb_unichar_alloc_walk)
{
	convert_utf8_to_unichar(dest, nb_unichar_alloc_walk, NULL, (const unsigned char*)get_copyright_utf8(), strlen(get_copyright_utf8())+1);
}



} // namespace unitex

#endif

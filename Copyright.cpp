/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "UnusedParameter.h"
#include "Unicode.h"
#include "Copyright.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * get the copyright notice as UTF8 string
 */
const char* get_copyright_utf8()
{
  // the skipped \n allow add line feed in binary file
  return ("\n" COPYRIGHT_NOTICE) + 1;
}


/**
* display the copyright notice
*/
void display_copyright_notice() {
  unichar* str = (unichar*)malloc(sizeof(unichar) * (SIZE_COPYRIGHT_NOTICE_BUFFER));
  if (str == NULL) {
    alloc_error("display_copyright_notice");
    return;
  }

  convert_utf8_to_unichar(str, SIZE_COPYRIGHT_NOTICE_BUFFER - 1, NULL, (const unsigned char*)get_copyright_utf8(), strlen(get_copyright_utf8()) + 1);

  u_printf("%S", str);
  free(str);
}


/**
 * get the copyright notice as unihcar (pre allocated by caller) string
 */
void get_copyright_notice(unichar*dest, size_t nb_unichar_alloc_walk) {
  convert_utf8_to_unichar(dest, nb_unichar_alloc_walk, NULL, (const unsigned char*)get_copyright_utf8(), strlen(get_copyright_utf8()) + 1);
}


} // namespace unitex

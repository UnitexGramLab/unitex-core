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

#include "Thai.h"

/**
 * Returns a non-zero if 'c' is a Thai diacritic.
 */
int is_Thai_diacritic(unichar c) {
return (c>=0x0e47 && c<=0x0e4c);
}


/**
 * Returns a non-zero value if 'c' is a Thai initial vowel.
 */
int is_Thai_initial_vowel(unichar c) {
return (c>=0x0e40 && c<=0x0e44);
}


/**
 * Returns a non-zero value if 'c' is a Thai diacritic that must be
 * ignored for counting displayable characters in a Thai string.
 */
int is_Thai_skipable(unichar c) {
return (c==0x0e31 || (c>=0x0e34 && c<=0x0e3a) || (c>=0x0e47 && c<=0x0e4e));
}


/**
 * Returns the number of displayable characters of the given string,
 * ignoring diacritic signs.
 */
int u_strlen_Thai(const unichar* s) {
int n=0;
int i=0;
while (s[i]!='\0') {
   if (!is_Thai_skipable(s[i])) n++;
   i++;
}
return n;
}

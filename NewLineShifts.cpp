/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdlib.h>
#include "Error.h"
#include "NewLineShifts.h"
#include "UnusedParameter.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This function takes an integer 'a' and an array 't' of size 'n'.
 * It returns the greatest value x so that t[x]<=a.
 */
int find_by_dichotomy(int a,int* t,int n) {
int start_position,middle_position;
if (t==NULL) {
   error("NULL array in find_by_dichotomy\n");
   return 0;
}
if (n==0) {
   return 0;
}
if (a<t[0]) return 0;
if (a>t[n-1]) return n;
n=n-1;
start_position=0;
while (start_position<=n) {
   middle_position=(start_position+n)/2;
   if (t[middle_position]==a) return middle_position;
   if (t[middle_position]<a) {
      start_position=middle_position+1;
   } else {
      n=middle_position-1;
   }
}
return n+1;
}



/**
 * This function takes an integer 'a' and an array 't' of size 'n'.
 * It looks for the greatest value x=3n so that t[x]<=a. Then,
 * it returns t[x+2], which is the snt shift before position 'a'.
 */
int find_snt_shift_by_dichotomy(int a,int* t,int n) {
int start_position,middle_position;
if (t==NULL) {
   fatal_error("NULL array in find_snt_shift_by_dichotomy\n");
   return 0;
}
if (n%3!=0) {
    fatal_error("Invalid array size %d in find_snt_shift_by_dichotomy\n",n);
}
if (n==0) {
   return 0;
}
n=(n/3);
if (a<t[0]) return 0;
if (a>t[(n-1)*3]) return t[(n-1)*3+2];
n=n-1;
start_position=0;
while (start_position<=n) {
   middle_position=(start_position+n)/2;
   if (t[3*middle_position]==a) {
       /* If we have an exact hit for the token at pos #a, then it's not
        * the shift after this token that we have to take into account,
        * but the shift before it */
       return t[3*middle_position+1];
   }
   if (t[3*middle_position]<a) {
      start_position=middle_position+1;
   } else {
       n=middle_position-1;
   }
}
return t[3*n+2];
}


/**
 * Old version: This function takes the number of new lines in the text ('n_enter_char'),
 * the array 'enter_pos' that contains their positions in tokens and a position
 * 'pos'. It returns the number of new lines that occur before 'pos'.
 *
 * New version: uses snt_offsets to know the size in characters of the shift
 * to be added to the given position.
 */
int get_shift(int n_enter_char,int* enter_pos,int pos,vector_int* snt_offsets) {
DISCARD_UNUSED_PARAMETER(enter_pos)
DISCARD_UNUSED_PARAMETER(n_enter_char)
//int res=find_by_dichotomy(pos,enter_pos,n_enter_char);
if (snt_offsets==NULL) return 0;
int res2=find_snt_shift_by_dichotomy(pos,snt_offsets->tab,snt_offsets->nbelems);
return res2;
}

} // namespace unitex

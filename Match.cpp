/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Match.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Returns 1 if a is longer than b; 0 otherwise.
 */
int is_longer_match(Match* a,Match* b) {
if (a->start_pos_in_token>b->start_pos_in_token) return 0;
if (a->start_pos_in_token==b->start_pos_in_token) {
   if (a->start_pos_in_char>b->start_pos_in_char) return 0;
   if (a->start_pos_in_char==b->start_pos_in_char) {
      if (a->start_pos_in_letter>b->start_pos_in_letter) return 0;
   }
}
if (a->end_pos_in_token<b->end_pos_in_token) return 0;
if (a->end_pos_in_token==b->end_pos_in_token) {
   if (a->end_pos_in_char<b->end_pos_in_char) return 0;
   if (a->end_pos_in_char==b->end_pos_in_char) {
      if (a->end_pos_in_letter<b->end_pos_in_letter) return 0;
   }
}
return 1;
}


/**
 * Returns 1 if a ends strictly after b.
 */
int match_end_after(Match* a,Match* b) {
if (a->end_pos_in_token<b->end_pos_in_token) return 0;
if (a->end_pos_in_token>b->end_pos_in_token) return 1;
/* Same end positions in tokens */
if (a->end_pos_in_char<b->end_pos_in_char) return 0;
if (a->end_pos_in_char>b->end_pos_in_char) return 1;
/* Same end positions in chars */
if (a->end_pos_in_letter<=b->end_pos_in_letter) return 0;
return 1;
}


/**
 * Returns 1 if a begins exactly at the same position that b.
 */
int same_start_positions(Match* a,Match* b) {
return a->start_pos_in_token==b->start_pos_in_token
    && a->start_pos_in_char==b->start_pos_in_char
    && a->start_pos_in_letter==b->start_pos_in_letter;
}


/**
 * Returns 1 if a ends exactly at the same position that b.
 */
int same_end_positions(Match* a,Match* b) {
return a->end_pos_in_token==b->end_pos_in_token
    && a->end_pos_in_char==b->end_pos_in_char
    && a->end_pos_in_letter==b->end_pos_in_letter;
}


/**
 * Returns 1 if a and b have the same bounds.
 */
int same_positions(Match* a,Match* b) {
return same_start_positions(a,b) && same_end_positions(a,b);
}


/**
 * Returns 1 if a's start position < b's start position; 0 otherwise
 */
int match_start_before(Match* a,Match* b) {
if (a->start_pos_in_token>b->start_pos_in_token) return 0;
if (a->start_pos_in_token<b->start_pos_in_token) return 1;
if (a->start_pos_in_char>b->start_pos_in_char) return 0;
if (a->start_pos_in_char<b->start_pos_in_char) return 1;
return a->start_pos_in_letter<b->start_pos_in_letter;
}


int b_starts_after_end_of_a(Match* a,Match* b) {
if (b->start_pos_in_token<a->end_pos_in_token) return 0;
if (b->start_pos_in_token>a->end_pos_in_token) return 1;
if (b->start_pos_in_char<a->end_pos_in_char) return 0;
if (b->start_pos_in_char>a->end_pos_in_char) return 1;
return b->start_pos_in_letter>a->end_pos_in_letter;
}


/**
 * Compares a's positions and b's positions.
 */
Overlap compare_matches(Match* a,Match* b) {
if (match_start_before(a,b)) {
   /* a starts before b starts */
   if (b_starts_after_end_of_a(a,b)) return A_BEFORE_B;
   if (match_end_after(b,a)) return A_BEFORE_B_OVERLAP;
   return A_INCLUDES_B;
} else if (same_start_positions(a,b)) {
   /* a and b start at the same position */
   if (same_end_positions(a,b)) return A_EQUALS_B;
   if (match_end_after(b,a)) return B_INCLUDES_A;
   return A_INCLUDES_B;
} else {
   /* a starts after b starts */
   if (match_end_after(b,a)) return B_INCLUDES_A;
   if (same_end_positions(a,b)) return B_INCLUDES_A;
   if (b_starts_after_end_of_a(b,a)) return A_AFTER_B;
   return A_AFTER_B_OVERLAP;
}
}


/**
 * Returns 1 if a starts before b ends; 0 otherwise.
 */
int valid_text_interval_tfst(Match* a,Match* b) {
if (a->start_pos_in_token<b->end_pos_in_token) return 1;
if (a->start_pos_in_token>b->end_pos_in_token) return 0;
return (a->start_pos_in_char<=b->end_pos_in_char);
}

} // namespace unitex

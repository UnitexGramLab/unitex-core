 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
  
#ifndef MatchH
#define MatchH

#include "Unicode.h"

/**
 * This structure is used to define the core information about a match,
 * either for Locate or LocateTfst.
 */
typedef struct {
   int start_pos_in_token;
   int end_pos_in_token;

   /* Those fields are used for Korean
    * xxx_char=offset of the char in the token 
    * xxx_letter=offset of the logical letter in the char */
   int start_pos_in_char;
   int end_pos_in_char;
   int start_pos_in_letter;
   int end_pos_in_letter;
} Match;


/**
 * This enum is used to compare matches.
 */
enum {
   A_BEFORE_B,
   A_BEFORE_B_OVERLAP,
   A_INCLUDES_B,
   A_EQUALS_B,
   B_INCLUDES_A,
   A_AFTER_B_OVERLAP,
   A_AFTER_B
};


int is_longer_match(Match* a,Match* b);
int match_end_after(Match* a,Match* b);
int same_start_positions(Match* a,Match* b);
int same_end_positions(Match* a,Match* b);
int same_positions(Match* a,Match* b);
int match_start_before(Match* a,Match* b);
int compare_matches(Match* a,Match* b);
int valid_text_interval_tfst(Match* a,Match* b);

#endif



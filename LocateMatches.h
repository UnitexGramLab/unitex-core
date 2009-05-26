 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef MatchesH
#define MatchesH

#include "Unicode.h"
#include "LocatePattern.h"
#include "LocateConstants.h"
#include "Match.h"

/**
 * This structure represents a match list. [start;end] is the interval
 * that corresponds to the matched sequence. 'start' and 'end' are absolute
 * positions in tokens. 'output' is the output associated to the match, if any;
 * NULL otherwise.
 */
struct match_list {
   Match m;
   unichar* output;
   struct match_list* next;
};


struct match_list* new_match(int,int,unichar*,struct match_list*);
struct match_list* new_match(int,int,int,int,int,int,unichar*,struct match_list*);
void free_match_list_element(struct match_list*);
void add_match(int,unichar*,struct locate_parameters*);
struct match_list* eliminate_longer_matches(struct match_list*,int,int,unichar*,int*,struct locate_parameters*);
struct match_list* save_matches(struct match_list*,int,U_FILE*,struct locate_parameters*);
struct match_list* load_match_list(U_FILE*,OutputPolicy*);

#endif

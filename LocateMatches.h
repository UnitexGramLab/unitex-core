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

#ifndef MatchesH
#define MatchesH

#include "Unicode.h"
#include "LocateConstants.h"
#include "Match.h"
#include "Vector.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure represents a match list. [start;end] is the interval
 * that corresponds to the matched sequence. 'start' and 'end' are absolute
 * positions in tokens. 'output' is the output associated to the match, if any;
 * NULL otherwise.
 */
struct match_list {
   Match m;
   unichar* output;
   int weight;
   struct match_list* next;
};


struct match_list* new_match(int,int,unichar*,int,struct match_list*,Abstract_allocator prv_alloc=NULL);
struct match_list* new_match(int,int,int,int,int,int,unichar*,int,struct match_list*,Abstract_allocator prv_alloc=NULL);
void free_match_list_element(struct match_list*,Abstract_allocator prv_alloc=NULL);
void free_match_list(struct match_list*,Abstract_allocator prv_alloc=NULL);
struct match_list* load_match_list(U_FILE*,OutputPolicy*,unichar*,Abstract_allocator prv_alloc=NULL);
void filter_unambiguous_outputs(struct match_list* *list,vector_int*);
int are_ambiguous(struct match_list* a,struct match_list* b);

} // namespace unitex

#endif

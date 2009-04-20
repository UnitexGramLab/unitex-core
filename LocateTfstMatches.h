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

#ifndef LocateTfstMatchesH
#define LocateTfstMatchesH

#include "List_int.h"


#define NO_TEXT_TOKEN_WAS_MATCHED -1

/**
 * This structure is used to represent a partial match information while exploring
 * a .tfst. As we explore an automaton, we factorize node information in order to
 * avoid combinatory explosion. This is why we use the 'pointed_by' field that counts
 * how many times a tfst_match is referenced.
 */
struct tfst_match {
   int source_state_text;
   int dest_state_text;
   int grammar_tag_number;
   int dest_state_grammar;
   int pointed_by;
   struct list_int* text_tag_numbers;
   struct tfst_match* next;
};

/**
 * This structure MUST NOT be replaced a list_ptr* because we want to perform
 * reference counting on the above field 'pointed_by'.
 */
struct tfst_match_list {
  struct tfst_match* match;
  struct tfst_match_list* next;
};


struct tfst_match* insert_in_tfst_matches(struct tfst_match*,int,int,int,int);
void free_tfst_match(struct tfst_match*);
struct tfst_match_list* add_match_in_list(struct tfst_match_list*,struct tfst_match*);
void clean_tfst_match_list(struct tfst_match*,struct tfst_match*);

#endif



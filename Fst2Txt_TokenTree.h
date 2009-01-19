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

#ifndef Fst2Txt_TokenTreeH
#define Fst2Txt_TokenTreeH

#include "Unicode.h"
#include "Alphabet.h"
#include "Fst2.h"
#include "Transitions.h"


/**
 * This structure is used to associate lists of transitions with tokens.
 * The string_hash is used to associate an index 'n' to a token 't' and
 * then 'transition_array[n]' contains the transitions associated to 't'.
 * 'capacity' is the maximum size of the array and 'size' is its actual
 * size.
 */
struct fst2txt_token_tree {
   struct string_hash* hash;
   Transition** transition_array;
   int capacity;
   int size;
};



struct fst2txt_token_tree* new_fst2txt_token_tree();
void free_fst2txt_token_tree(struct fst2txt_token_tree*);

void add_tag(unichar*,int,int,struct fst2txt_token_tree*);
Transition* get_matching_tags(unichar*,struct fst2txt_token_tree*,Alphabet*);


#endif

 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef Normalization_transducerH
#define Normalization_transducerH


#include "Unicode.h"
#include "Alphabet.h"
#include "Fst2.h"
#include "Text_tokens.h"
#include "String_hash.h"
#include "List_int.h"
#include "List_ustring.h"

#define EMPTY_TOKEN -4


/**
 * This structure represents a node in a normalization tree.
 * Each node is associated to a list of outputs to be produced when
 * the token sequence corresponding the path to this node is found.
 */
struct normalization_tree {
   struct list_ustring* outputs;
   struct normalization_tree_transition* trans;
};


/**
 * This structure represents a list of transitions in a normalization tree.
 * Note that the transitions can be tagged either by token numbers or strings.
 */
struct normalization_tree_transition {
   union {
      int token;
      unichar* s;
   };
   struct normalization_tree* node;
   struct normalization_tree_transition* next;
};




struct normalization_tree* load_normalization_fst2(char*,Alphabet*,struct text_tokens*);
struct normalization_tree* new_normalization_tree();
void free_normalization_tree(struct normalization_tree*);
struct list_ustring* tokenize_normalization_output(unichar*,Alphabet*);

struct normalization_tree_transition* new_trans_arbre_normalization_string(unichar*);
struct normalization_tree* load_normalization_transducer_string(char*);

#endif

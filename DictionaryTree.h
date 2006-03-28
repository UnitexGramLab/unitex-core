 /*
  * Unitex
  *
  * Copyright (C) 2001-2005 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------
#ifndef DictionaryTreeH
#define DictionaryTreeH
//---------------------------------------------------------------------------

#include "unicode.h"
#include "String_hash.h"
#include "Liste_nombres.h"


/**
 * This structure represents a node of a DELAF dictionary being loaded before
 * being compressed. In the first step, the dictionary is represented as a tree,
 * and then it is compressed into an acylic minimal automaton, using
 * Dominique Revuz's algorithm.
 */
struct dictionary_node {
       struct liste_nombres* arr;
       struct dictionary_tree_transition* trans;
       int offset; // this value will be used to give to this node an adress in the .BIN file
       int n_trans;
       int hash_number;
};


struct dictionary_tree_transition {
       unichar letter;
       struct dictionary_node* node;
       struct dictionary_tree_transition* next;
};


void free_arbre_dico(struct dictionary_node*);
void free_arbre_dico_non_rec(struct dictionary_node*);
void free_arbre_dico_trans(struct dictionary_tree_transition*);
void inserer_entree(unichar*,unichar*,struct dictionary_node*,struct string_hash*);
struct dictionary_node* new_arbre_dico();

#endif

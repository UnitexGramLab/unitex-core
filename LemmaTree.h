 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef LemmaTreeH
#define LemmaTreeH

#include "Unicode.h"
#include "List_ustring.h"

/**
 * These structures are used to build a tree of lemmas. For each lemma,
 * the associated final node contains a list of all the inflected forms
 * that correspond to this lemma. For instance, if the text simple word
 * dictionary contains the following lines:
 * 
 * êtres,être.N:mp
 * suis,être.V:P1s
 * 
 * then the liste "êtres", "suis" will be associated to the lemma "être".
 */
struct lemma_node {
   struct list_ustring* inflected_forms;
   unichar letter;
   struct lemma_node_list* sons;
};


struct lemma_node_list {
  struct lemma_node* node;
  struct lemma_node_list* next;
};


struct lemma_node* new_lemma_node();
void free_lemma_node(struct lemma_node*);
void add_inflected_form_for_lemma(unichar*,unichar*,struct lemma_node*);
struct list_ustring* get_inflected_forms(unichar*,struct lemma_node*);

#endif


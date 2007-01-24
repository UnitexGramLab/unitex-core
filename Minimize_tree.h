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

//---------------------------------------------------------------------------
#ifndef Minimize_treeH
#define Minimize_treeH
//---------------------------------------------------------------------------

#include "DictionaryTree.h"


#define HAUTEUR_MAX 10000

#define MAX_TRANS ((sizeof(void*) > 4) ? 134217728 : 4194304 )
/* a hack to get bigger size on 64bit-architectures */

struct node_list {
   struct dictionary_node_transition* trans;
   struct node_list* suivant;
};

extern struct node_list* tab_by_hauteur[];
extern struct dictionary_node_transition* tab_trans[];


void minimize_tree(struct dictionary_node*);
int compare_nodes(struct dictionary_node_transition*,struct dictionary_node_transition*);
void init_tab_by_hauteur();
void free_tab_by_hauteur();
int sort_by_height(struct dictionary_node*);
void insert_trans_in_tab_by_hauteur(struct dictionary_node_transition*,int);
struct node_list* new_node_list();
int convert_list_to_array(unsigned int);
void quicksort(int, int);
void fusionner(int);

#endif

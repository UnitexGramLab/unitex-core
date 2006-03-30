 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "DLC_optimization.h"
//---------------------------------------------------------------------------


void optimiser_etat_dlc(struct DLC_tree_node* n) {
struct liste_nombres* tmp;
struct DLC_tree_transition* t;
int i;
if (n==NULL) return;
if (n->number_of_patterns!=0) {
  n->array_of_patterns=(int*)malloc(sizeof(int)*n->number_of_patterns);
  i=0;
  while (n->patterns!=NULL) {
    n->array_of_patterns[i++]=n->patterns->n;
    tmp=n->patterns;
    n->patterns=n->patterns->suivant;
    free(tmp);
  }
}
if (n->number_of_transitions!=0) {
  n->destination_tokens=(int*)malloc(sizeof(int)*n->number_of_transitions);
  n->destination_nodes=(struct DLC_tree_node**)malloc(sizeof(struct DLC_tree_node*)*n->number_of_transitions);
  i=0;
  while (n->transitions!=NULL) {
    n->destination_tokens[i]=n->transitions->token;
    n->destination_nodes[i]=n->transitions->node;
    t=n->transitions;
    n->transitions=n->transitions->next;
    optimiser_etat_dlc(n->destination_nodes[i]);
    i++;
    free(t);
  }
}
}



void optimize_dlc(struct DLC_tree_info* DLC_tree) {
optimiser_etat_dlc(DLC_tree->root);
} 


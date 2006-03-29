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
#include <stdio.h>

#include "Minimize_tree.h"
#include "unicode.h"

//---------------------------------------------------------------------------


struct node_list* tab_by_hauteur[HAUTEUR_MAX];
struct dictionary_node_transition* tab_trans[MAX_TRANS];

void minimize_tree(struct dictionary_node* racine) {
printf("Minimizing...                      \n");
init_tab_by_hauteur();
unsigned int H;
H=sort_by_height(racine);
float z;
for (unsigned int k=0;k<=H;k++) {
   int size=convert_list_to_array(k);
   quicksort(0,size-1);
   fusionner(size);
   z=100.0*(float)(k)/(float)H;
   if (z>100.0) z=100.0;
   printf("%2.0f%% completed...    \r",z);
}
printf("Minimization done.                     \n");
free_tab_by_hauteur();
}



//
// this function compares two tree nodes as follow:
// 1) by the unichar that lead to them
// 2) by their hash_number (n° of line in INF file)
// 3) by the transition that get out of them
//
int compare_nodes(struct dictionary_node_transition* a,struct dictionary_node_transition* b) {
if (a==NULL || b==NULL || a->node==NULL || b->node==NULL) {
   fprintf(stderr,"Probleme dans compare_nodes\n");
   getchar();
   exit(1);
}
// then, the hash numbers
if (a->node->single_INF_code_list!=NULL && b->node->single_INF_code_list==NULL) return -1;
if (a->node->single_INF_code_list==NULL && b->node->single_INF_code_list!=NULL) return 1;

if (a->node->single_INF_code_list!=NULL && b->node->single_INF_code_list!=NULL &&
/*if (*/a->node->INF_code != b->node->INF_code)
   return (a->node->INF_code - b->node->INF_code);
// and finally the transitions
a=a->node->trans;
b=b->node->trans;
while(a!=NULL && b!=NULL) {
   // if the unichars are different then nodes are different
   if (a->letter != b->letter) return (a->letter - b->letter);
   // if the unichars are equal and destination nodes are different...
   if (a->node != b->node) return (a->node - b->node);
   a=a->next;
   b=b->next;
}
if (a==NULL && b==NULL) {
   // if the transition lists are equal
   return 0;
}
if (a==NULL) {
   // if the first list is shorter than the second
   return -1;
}
// if the first list is longuer then the second
return 1;
}



void init_tab_by_hauteur() {
unsigned int i;
for (i=0;i<HAUTEUR_MAX;i++) {
   tab_by_hauteur[i]=NULL;
}
}



void free_node_list(struct node_list* l) {
struct node_list* ptr;
while (l!=NULL) {
   ptr=l;
   l=l->suivant;
   free(ptr);
}
}



void free_tab_by_hauteur() {
int i;
for (i=0;i<HAUTEUR_MAX;i++) {
   free_node_list(tab_by_hauteur[i]);
}
}



int sort_by_height(struct dictionary_node* n) {
if (n==NULL) {
   fprintf(stderr,"Probleme dans sort_by_height\n");
   exit(1);
}
if (n->trans==NULL) {
   // if the node is a leaf
   return 0;
}
struct dictionary_node_transition* trans=n->trans;
int res=-1;
int k;
while (trans!=NULL) {
   k=sort_by_height(trans->node);
   if (k>res) res=k;
   insert_trans_in_tab_by_hauteur(trans,k);
   trans=trans->next;
}
return 1+res;
}



void insert_trans_in_tab_by_hauteur(struct dictionary_node_transition* trans,int k) {
  if (k > HAUTEUR_MAX)
    {
      fprintf(stderr, "HAUTEUR_MAX=%u reached: exiting!\n", HAUTEUR_MAX);
      exit(EXIT_FAILURE);
    }
  struct node_list* tmp;
  tmp=new_node_list();
  tmp->trans=trans;
  tmp->suivant=tab_by_hauteur[k];
  tab_by_hauteur[k]=tmp;
}



struct node_list* new_node_list() {
struct node_list* n;
n=(struct node_list*)malloc(sizeof(struct node_list));
n->trans=NULL;
n->suivant=NULL;
return n;
}



int convert_list_to_array(unsigned int k) {
  unsigned int size=0;
  struct node_list* l=tab_by_hauteur[k];
  struct node_list* ptr;
  tab_by_hauteur[k]=NULL;
  while (l!=NULL) {
    if ( size > MAX_TRANS )
    {
      fprintf(stderr, "MAX_TRANS=%u reached: exiting!\n", MAX_TRANS);
      exit(EXIT_FAILURE);
    }
    tab_trans[size++]=l->trans;
    ptr=l;
    l=l->suivant;
    free(ptr);
  }
  return size;
}




//
// on partitionne le tableau t
//
int partition_pour_quicksort(int m, int n) {
struct dictionary_node_transition* pivot;
struct dictionary_node_transition* tmp;
int i = m-1;
int j = n+1;         // indice final du pivot
pivot=tab_trans[(m+n)/2];
while (true) {
  do j--;
  while ((j>(m-1))&&(compare_nodes(pivot,tab_trans[j]) < 0));
  do i++;
  while ((i<n+1)&&(compare_nodes(tab_trans[i],pivot) < 0));
  if (i<j) {
    tmp=tab_trans[i];
    tab_trans[i]=tab_trans[j];
    tab_trans[j]=tmp;
  } else return j;
}
}



void quicksort(int debut, int fin) {
int p;
if (debut<fin) {
  p=partition_pour_quicksort(debut,fin);
  quicksort(debut,p);
  quicksort(p+1,fin);
}
}



void fusionner(int size) {
int i=1;
struct dictionary_node_transition* base=tab_trans[0];
while (i<size) {
   if (compare_nodes(base,tab_trans[i])==0) {
     // if the base trans is equivalent to the current one
     // then we must destroy the current one's destination node
#warning this causes a segfault raised in free_liste_nbre
#warning because of double freeing
#warning   free_arbre_dico(tab_trans[i]->noeud);
#warning the solution
#warning   free_arbre_dico_non_rec(tab_trans[i]->noeud);
#warning is better, but not perfect: there is still a memory leak!
     free_arbre_dico_non_rec(tab_trans[i]->node);
     // and modify the current one's destination node
     tab_trans[i]->node=base->node;
   }
   else {
      base=tab_trans[i];
   }
   i++;
}
}


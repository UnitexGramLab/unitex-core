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

#include "Minimize_tree.h"
#include <stdio.h>
#include "unicode.h"
#include "Error.h"
#include "DictionaryTree.h"


/* The maximum height of the tree corresponds to the length of the
 * longest word in the dictionary */
#define MAXIMUM_HEIGHT 10000

/* We define here the maximum number of transitions for a given height.
 * We use a hack to get bigger size on 64bit-architectures */
#define MAXIMUM_TRANSITIONS ((sizeof(void*) > 4) ? 134217728 : 4194304 )


/**
 * We define here a list of transitions in the dictionary tree.
 * It's here and not in the .h because we don't want to export this
 * internal data structure.
 */
struct transition_list {
   struct dictionary_node_transition* transition;
   struct transition_list* next;
};

int compare_nodes(struct dictionary_node_transition*,struct dictionary_node_transition*);
void init_minimize_arrays(struct transition_list***,struct dictionary_node_transition***);
void free_minimize_arrays(struct transition_list**,struct dictionary_node_transition**);
int sort_by_height(struct dictionary_node*,struct transition_list**);
struct transition_list* new_transition_list(struct dictionary_node_transition*,struct transition_list*);
int convert_list_to_array(unsigned int,struct transition_list**,
                          struct dictionary_node_transition**);
void quicksort(int,int,struct dictionary_node_transition**);
void merge(int,struct dictionary_node_transition**);




/**
 * This function takes a dictionary tree and minimizes it using
 * Dominique Revuz's algorithm.
 */
void minimize_tree(struct dictionary_node* root) {
printf("Minimizing...                      \n");
struct transition_list** transitions_by_height;
struct dictionary_node_transition** transitions;
init_minimize_arrays(&transitions_by_height,&transitions);
unsigned int H=sort_by_height(root,transitions_by_height);
float z;
for (unsigned int k=0;k<=H;k++) {
   int size=convert_list_to_array(k,transitions_by_height,transitions);
   quicksort(0,size-1,transitions);
   merge(size,transitions);
   z=100.0*(float)(k)/(float)H;
   if (z>100.0) z=100.0;
   printf("%2.0f%% completed...    \r",z);
}
printf("Minimization done.                     \n");
free_minimize_arrays(transitions_by_height,transitions);
}



/**
 * This function compares two tree nodes as follow:
 * 1) by the unichar that lead to them
 * 2) by their hash_number (n° of line in INF file)
 * 3) by the transition that get out of them
 */
int compare_nodes(struct dictionary_node_transition* a,struct dictionary_node_transition* b) {
if (a==NULL || b==NULL || a->node==NULL || b->node==NULL) {
   fatal_error("Probleme dans compare_nodes\n");
}
/* If the nodes have not the same INF codes, they are different */
if (a->node->single_INF_code_list!=NULL && b->node->single_INF_code_list==NULL) return -1;
if (a->node->single_INF_code_list==NULL && b->node->single_INF_code_list!=NULL) return 1;
if (a->node->single_INF_code_list!=NULL && b->node->single_INF_code_list!=NULL &&
    a->node->INF_code!=b->node->INF_code)
   return (a->node->INF_code - b->node->INF_code);

/* Then, we compare all the outgoing transitions, two by two */
a=a->node->trans;
b=b->node->trans;
while(a!=NULL && b!=NULL) {
   /* If the 2 current transitions are not tagged by the same 
    * character, then the nodes are different */
   if (a->letter != b->letter) return (a->letter - b->letter);
   /* If the characters are equal and destination nodes are different... */
   if (a->node != b->node) return (a->node - b->node);
   a=a->next;
   b=b->next;
}
if (a==NULL && b==NULL) {
   /* If the transition lists are equal, the nodes are equivalent */
   return 0;
}
if (a==NULL) {
   /* If the first list is shorter than the second */
   return -1;
}
/* If the first list is longer then the second */
return 1;
}


/**
 * We allocate and initialize 2 arrays used by the minimization.
 */
void init_minimize_arrays(struct transition_list** *transitions_by_height,
                          struct dictionary_node_transition** *transitions) {
(*transitions_by_height)=(struct transition_list**)malloc(MAXIMUM_HEIGHT*sizeof(struct transition_list*));
if (*transitions_by_height==NULL) {
   fatal_error("Not enough memory in init_minimize_arrays\n");
}
unsigned int i;
for (i=0;i<MAXIMUM_HEIGHT;i++) {
   (*transitions_by_height)[i]=NULL;
}
(*transitions)=(dictionary_node_transition**)malloc(MAXIMUM_TRANSITIONS*sizeof(struct dictionary_node_transition*));
if (*transitions==NULL) {
   fatal_error("Not enough memory in init_minimize_arrays\n");
}
}


/**
 * Frees all the given transition list.
 */
void free_transition_list(struct transition_list* l) {
struct transition_list* ptr;
while (l!=NULL) {
   ptr=l;
   l=l->next;
   free(ptr);
}
}


/**
 * Frees the 2 arrays used by the minimization.
 */
void free_minimize_arrays(struct transition_list** transitions_by_height,
                          struct dictionary_node_transition** transitions) {
for (int i=0;i<MAXIMUM_HEIGHT;i++) {
   free_transition_list(transitions_by_height[i]);
}
free(transitions_by_height);
/* The 'transitions' array is supposed to be allready cleaned */
free(transitions);
}


/**
 * This function explores the dictionary and puts its transitions into the 
 * 'transitions' array. For a given height, the transitions are not sorted,
 * they will be later in the 'minimize_tree' function.
 * The function returns the height of the given node.
 */
int sort_by_height(struct dictionary_node* n,struct transition_list** transitions_by_height) {
if (n==NULL) {
   fatal_error("NULL error in sort_by_height\n");
}
if (n->trans==NULL) {
   /* If the node is a leaf, we have nothing to do */
   return 0;
}
struct dictionary_node_transition* trans=n->trans;
int height=-1;
int k;
while (trans!=NULL) {
   /* We process recursively the node pointed out by the current transition */
   k=sort_by_height(trans->node,transitions_by_height);
   if (k==MAXIMUM_HEIGHT) {
      fatal_error("Maximum height reached in sort_by_height\n");
   }
   /* We insert the transition in the appropriate list */
   transitions_by_height[k]=new_transition_list(trans,transitions_by_height[k]);
   /* The height of the current node is the maximum of his sons' heights... */
   if (height<k) height=k;
   trans=trans->next;
}
/* ... +1 because of the current node itself */
return 1+height;
}


/**
 * Allocates, initializes and returns a new transition list element.
 */
struct transition_list* new_transition_list(struct dictionary_node_transition* transition,
                                      struct transition_list* next) {
struct transition_list* t;
t=(struct transition_list*)malloc(sizeof(struct transition_list));
if (t==NULL) {
   fatal_error("Not enough memory in new_transition_list\n");
}
t->transition=transition;
t->next=next;
return t;
}


/**
 * We convert the list of transitions corresponding the given height
 * into an array, in order to apply the quicksort.
 */
int convert_list_to_array(unsigned int height,struct transition_list** transitions_by_height,
                          struct dictionary_node_transition** transitions) {
unsigned int size=0;
struct transition_list* l=transitions_by_height[height];
struct transition_list* ptr;
transitions_by_height[height]=NULL;
while (l!=NULL) {
   if (size>MAXIMUM_TRANSITIONS) {
      fatal_error("MAX_TRANS=%u reached: exiting!\n", MAXIMUM_TRANSITIONS);
   }
   transitions[size++]=l->transition;
   ptr=l;
   l=l->next;
   free(ptr);
}
return size;
}


/**
 * Builds a quicksort partition of the given array.
 */
int partition(int start,int end,struct dictionary_node_transition** transitions) {
struct dictionary_node_transition* pivot;
struct dictionary_node_transition* tmp;
int i=start-1;
/* Final pivot index */
int j=end+1;
pivot=transitions[(start+end)/2];
while (true) {
   do j--;
   while ((j>(start-1))&&(compare_nodes(pivot,transitions[j]) < 0));
   do i++;
   while ((i<end+1)&&(compare_nodes(transitions[i],pivot) < 0));
   if (i<j) {
      tmp=transitions[i];
      transitions[i]=transitions[j];
      transitions[j]=tmp;
   } else return j;
}
}


/**
 * Sorts the given transition array, according to Dominique Revuz's criteria.
 */
void quicksort(int start,int end,struct dictionary_node_transition** transitions) {
int p;
if (start<end) {
   p=partition(start,end,transitions);
   quicksort(start,p,transitions);
   quicksort(p+1,end,transitions);
}
}


/**
 * 'transitions' is supposed to be sorted, i.e. equivalent transitions
 * are contiguous. If two transitions point to equivalent nodes, we
 * redirect the second transition on the first's node and we free the
 * node that is not pointed anymore.
 */
void merge(int size,struct dictionary_node_transition** transitions) {
int i=1;
struct dictionary_node_transition* base=transitions[0];
while (i<size) {
   if (compare_nodes(base,transitions[i])==0) {
      /* If the base transition is equivalent to the current one
       * then we must destroy the current one's destination node */
      free_dictionary_node(transitions[i]->node);
      /* We modify the current one's destination node */
      transitions[i]->node=base->node;
      /* And we increase the number of references of the node, in order
       * to know later when it could be freed */
      (base->node->incoming)++;
   }
   else {
      base=transitions[i];
   }
   i++;
}
}


/*
  * Unitex 
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  *
  */

/* Created by Agata Savary (savary@univ-tours.fr)
 * Last modification on July 22 2005
 */
//---------------------------------------------------------------------------

////////////////////////////////////////////////////////////
// Implementation of the management of inflectional tranducers
////////////////////////////////////////////////////////////
// Copy of some functions from Inflect.cpp

#include <stdlib.h>
#include "MF_InflectTransd.h"
#include "Error.h"
#include "FileName.h"

///////////////////////////////
// GLOBAL VARIABLES
///////////////////////////////
///////////////////////////////
// Root of the tree for inflection transducers' names
struct node* root;

///////////////////////////////
// Table of inflection tranducers
Fst2* fst2[N_FST2];

///////////////////////////////
// Number of inflection tranducers
int n_fst2;

///////////////////////////////
// Directory containing the inflection tranducers
//char repertoire[1000] = "/home/agata/UNITEX1.2beta_avec_flex_comp/UNITEX1.2beta_avec_flex_comp_22062005/Polish/Inflection/";
char inflection_directory[FILENAME_MAX];

///////////////////////////////
// ALL FUNCTIONS IN THIS MODULE
int init_transducer_tree();
void free_transducer_tree();
int get_transducer(char* flex);
struct node* new_node();
struct transition* new_transition(char c);
void free_transition(struct transition* t);
void free_node(struct node* n);
struct transition* get_transition(char c,struct transition* t,struct node** n);
int get_node(char* flex,int pos,struct node* n);

///////////////////////////////
//Initiate the tree for inflection transducers' names
//On succes return 0, 1 otherwise
int init_transducer_tree() {
  n_fst2 = 0;
  root=new_node();
  if (!root) {
    error("Tranducer tree could not be initialized in function 'init_transducer_tree'\n");
    return 1;
  }
  return  0;
}

///////////////////////////////
// Free the transducer tree memory
void free_transducer_tree() {
free_node(root);
}

///////////////////////////////
// Try to load the transducer flex and returns its position in the
// 'fst2' array. Returns -1 if the transducer cannot be loaded
int get_transducer(char* flex) {
return get_node(flex,0,root);
}

///////////////////////////////
// Create a new node in the tree
struct node* new_node() {
struct node* n=(struct node*)malloc(sizeof(struct node));
n->final=-1;
n->t=NULL;
return n;
}

///////////////////////////////
// Create a new branch in the tree
struct transition* new_transition(char c) {
struct transition* t=(struct transition*)malloc(sizeof(struct transition));
t->c=c;
t->n=NULL;
t->suivant=NULL;
return t;
}

///////////////////////////////
// Free the branch
void free_transition(struct transition* t) {
struct transition* tmp;
while (t!=NULL) {
    free_node(t->n);
    tmp=t;
    t=t->suivant;
    free(tmp);
}
}

///////////////////////////////
// Free a node
void free_node(struct node* n) {
if (n==NULL) {
  error("NULL error in free_node\n");
  return;
}
free_transition(n->t);
free(n);
}

///////////////////////////////
// Looks for a transition by the char c
// Creates it if it does not exist
struct transition* get_transition(char c,struct transition* t,struct node** n) {
struct transition* tmp;
while (t!=NULL) {
    if (t->c==c) return t;
    t=t->suivant;
}
tmp=new_transition(c);
tmp->suivant=(*n)->t;
tmp->n=NULL;
(*n)->t=tmp;
return tmp;
}

///////////////////////////////
// Look for the path to 'flex', creating it if necessary
// The current node is n, and pos is the position in the flex string
int get_node(char* flex,int pos,struct node* n) {
if (flex[pos]=='\0') {
    // we are at the final node for flex (a leaf)
    if (n->final!=-1) {
        // if the automaton already exists we returns its position in the transducer array (fst2)
        return n->final;
    } 
    else {
        // else we load it
        if (n_fst2==N_FST2) {
            fatal_error("Memory error: too much flexional transducers\n");
        }
        char s[FILENAME_MAX];
        new_file(inflection_directory,flex,s);
        strcat(s,".fst2");
        fst2[n_fst2]=load_fst2(s,1);
        n->final=n_fst2;
        return n_fst2++;
        }
}
// if we are not at the end of the string flex
struct transition* trans=get_transition(flex[pos],n->t,&n);
if (trans->n==NULL) {
    trans->n=new_node();
}
return get_node(flex,pos+1,trans->n);
}

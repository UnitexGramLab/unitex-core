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

#include "utils.h"
#include "ElagFstFilesIO.h"
#include "autalmot.h"
#include "Error.h"
#include "SingleGraph.h"


transition_t * transition_new(int to, symbol_t * label, transition_t * next) {

  transition_t * trans = (transition_t *) xmalloc(sizeof(transition_t));

  trans->to    = to;
  trans->label = dup_symbol(label);
  trans->next  = next;

  return trans;
}


void transition_delete(transition_t * trans) {
  free_symbol(trans->label);
  free(trans);
}


void transitions_delete(transition_t * trans) {
  while (transition_t * tmp = trans) {
    trans = tmp->next;
    transition_delete(tmp);
  }
}


void transitions_concat(transition_t ** t1, transition_t * t2) {
  while (*t1) { t1 = & (*t1)->next; }
  *t1 = t2;
}


inline transition_t * transition_dup(const transition_t * trans) {
  return transition_new(trans->to, trans->label, NULL);
}


transition_t * transitions_dup(const transition_t * trans) {

  transition_t res;
  res.next = NULL;

  for (transition_t * t1 = & res; trans; t1 = t1->next, trans = trans->next) { t1->next = transition_dup(trans); }

  return res.next;
}


/*
 * DEPRECATED
 * 
 * autalmot_t * autalmot_new(unichar * name, int size) {

  autalmot_t * A = (autalmot_t *) xmalloc(sizeof(autalmot_t));

  A->name = u_strdup(name);

  A->nbstates = 0;

  if (size < 1) { size = 1; }

  A->states = (state_t *) xmalloc(size * sizeof(state_t));

  for (int i = 0; i < size; i++) {
    A->states[i].flags = 0;
    A->states[i].trans = NULL;
    A->states[i].defto = -1;
  }

  A->size = size;

  A->initials   = NULL;
  A->nbinitials = 0;

  return A;
}
*/
/**
 * Allocates, initializes and return a new .fst2 automaton. If size<0,
 * the automaton field is set to NULL.
 */
Fst2Automaton* new_Fst2Automaton(unichar* name,int size) {
Fst2Automaton* aut=(Fst2Automaton*)malloc(sizeof(Fst2Automaton));
if (aut==NULL) {
   fatal_error("Not enough memory in new_Fst2Automaton\n");
}
aut->name=u_strdup(name);
if (size>=0) {
   aut->automaton=new_SingleGraph(size);
} else {
   aut->automaton=NULL;
}
return aut;
}


/**
 * Frees all the memory associated to the given automaton, except
 * the symbols.
 */
void free_Fst2Automaton(Fst2Automaton* A) {
if (A==NULL) return;
if (A->name!=NULL) free(A->name);
free_SingleGraph(A->automaton);
free(A);
}


Fst2Automaton * autalmot_dup(const Fst2Automaton * src) {

  Fst2Automaton * res = new_Fst2Automaton(src->name, src->nbstates);

  for (int q = 0; q < src->nbstates; q++) {
    autalmot_add_state(res, src->states[q].flags);
    res->states[q].trans = transitions_dup(src->states[q].trans);
    res->states[q].defto = src->states[q].defto;
  }

  if (src->nbinitials != res->nbinitials) {
    error("autalmot_dup: != initials\n");
    for (int i = 0; i < src->nbinitials; i++) {
      autalmot_set_initial(res, src->initials[i]);
    }
  }

  return res;
}


void autalmot_empty(Fst2Automaton * A) {

  free(A->name);
  A->name = NULL;

  for (int i = 0; i < A->nbstates; i++) { transitions_delete(A->states[i].trans); }
  A->nbstates   = 0;
  A->nbinitials = 0;
}


void autalmot_resize(Fst2Automaton * A, int size) {

  if (size == 0) { size = 1; }

  if (size < A->nbstates) { fatal_error("autalmot_resize: size(=%d) < nbstates(=%d)\n", size, A->nbstates); }

  A->states = (state_t *) xrealloc(A->states, size * sizeof(state_t));

  for (int i = A->size; i < size; i++) {
    A->states[i].flags = 0;
    A->states[i].trans = NULL;
    A->states[i].defto  = -1;
  }

  A->size = size;
}



int autalmot_add_state(Fst2Automaton * A, int flags) {

  if (A->nbstates >= A->size) { autalmot_resize(A, A->size * 2); }

  int res = A->nbstates;

  A->states[res].flags = flags;
  A->states[res].trans = NULL;
  A->states[res].defto = -1;

  A->nbstates++;

  if (flags & AUT_INITIAL) { autalmot_set_initial(A, res); }

  return res;
}



void add_transition(Fst2Automaton * A, int from, symbol_t * label, int to) {


  if (label == SYMBOL_DEF) {
    if (A->states[from].defto != -1) { fatal_error("autalmot add trans: to much <def>\n"); }
    A->states[from].defto = to;
    return;
  }


  while (label) {

    if (label == SYMBOL_DEF) { fatal_error("autalmot_add_trans called with SYMB_DEF\n"); }

    A->states[from].trans = transition_new(to, label, A->states[from].trans);
    label = label->next;
  }
}


/**
 * Adds a transition to 'automaton'.
 */
void add_transition(SingleGraph automaton,struct string_hash_ptr* symbols,int from,
                    symbol_t* label,int to) {
if (label==SYMBOL_DEF) {
   if (automaton->states[from]->default_state!=-1) {
      fatal_error("add_transition: more than one default transition\n");
   }
   automaton->states[from]->default_state=to;
   return;
}
while (label!=NULL) {
   if (label==SYMBOL_DEF) {
      fatal_error("add_transition: unexpected default transition\n");
   }
   /* We build a string representation of the symbol to avoid
    * duplicates in the value array */
   Ustring* u=new_Ustring();
   symbol_to_str(label,u);
   int n=get_value_index(u->str,symbols,INSERT_IF_NEEDED,label);
   free_Ustring(u);
   add_outgoing_transition(automaton->states[from],n,to);
   label=label->next;
}
}




void autalmot_set_initial(Fst2Automaton * A, int q) {

  A->states[q].flags |= AUT_INITIAL;

  for (int i = 0; i < A->nbinitials; i++) { if (A->initials[i] == q) { return; } }

  A->nbinitials++;
  A->initials = (int *) xrealloc(A->initials, A->nbinitials * sizeof(int));

  A->initials[A->nbinitials - 1] = q;
}


void autalmot_unset_initial(Fst2Automaton * A, int q) {

  A->states[q].flags &= ~(AUT_INITIAL);

  for (int i = A->nbinitials - 1; i >= 0; i--) {
    if (A->initials[i] == q) {
      A->initials[i] = A->initials[--A->nbinitials];
    }
  }
}


/**
 * This function saves the given fst2 automaton into a file
 * with the given name. 'type' indicates the kind of automaton
 * (text fst, elag grammar, ...).
 */
void save_automaton(const Fst2Automaton* A,char* name,int type) {
Elag_fst_file_out* fstf=fst_file_out_open(name,type);
if (fstf==NULL) {
   error("Unable to open '%s'\n",name);
   return;
}
fst_file_write(fstf,A);
fst_file_close_out(fstf);
}

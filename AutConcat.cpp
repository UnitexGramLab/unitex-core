/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "AutConcat.h"
#include "Fst2Automaton.h"
#include "Symbol_op.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Returns a list of symbols that corresponds to all the symbols
 * that tag transitions of the given list.
 */
symbol_t* symbols_from_transs(Transition* list) {
symbol_t res;
symbol_t* tmp;
res.next=NULL;
symbol_t* end=&res;
while (list!=NULL) {
   tmp=list->label;
   concat_symbols(end,dup_symbols(tmp),&end);
   list=list->next;
}
return res.next;
}


/**
 * If the state #q of the automaton A has a default transition, this function
 * adds all the explicit transitions that are equivalent to the default one.
 */
void explicit_default_transition(language_t* language,SingleGraph A,int q) {
if (A->states[q]->default_state==-1) {
   /* Nothing to do if there is no default transition */
   return;
}
/* We compute the set of symbols tagging transitions that outgo from q */
symbol_t* s=symbols_from_transs(A->states[q]->outgoing_transitions);
/* and we take the complementary set */
symbol_t* all_but_s=minus_symbols(language,s);
add_all_outgoing_transitions(A->states[q],all_but_s,A->states[q]->default_state);
free_symbols(s);
free_symbols(all_but_s);
}


/**
 * This function concatenates B at the end of A. A is modified.
 */
void elag_concat(language_t* language,SingleGraph A,SingleGraph B) {
int oldnb=A->number_of_states;
int* renumber=(int*)malloc(B->number_of_states*sizeof(int));
if (renumber==NULL) {
   fatal_alloc_error("elag_concat");
}
int q;
/* We copy the states of B into A */
for (q=0;q<B->number_of_states;q++) {
   renumber[q]=A->number_of_states;
   add_state(A);
}
for (q=0;q<B->number_of_states;q++) {
   A->states[renumber[q]]->outgoing_transitions=clone_transition_list(B->states[q]->outgoing_transitions,renumber,dup_symbol);
   A->states[renumber[q]]->default_state=(B->states[q]->default_state!=-1)?renumber[B->states[q]->default_state]:-1;
   if (is_final_state(B->states[q])) {
      set_final_state(A->states[renumber[q]]);
   }
}
/* Then, we concatenate A and B.
 * 1) We replace default transitions that outgo from B's initial states
 *    by explicit transitions */
struct list_int* initials=get_initial_states(B);
for (struct list_int* tmp=initials;tmp!=NULL;tmp=tmp->next) {
   explicit_default_transition(language,A,renumber[tmp->n]);
}
for (q=0;q<oldnb;q++) {
   if (is_final_state(A->states[q])) {
      /* Each final state of A becomes non final. Moreover, we have
       * to explicit its default transition, because if not, the concatenation
       * algorithm will modify the recognized language. */
      unset_final_state(A->states[q]);
      explicit_default_transition(language,A,q);
      for (struct list_int* tmp=initials;tmp!=NULL;tmp=tmp->next) {
         concat(&(A->states[q]->outgoing_transitions),clone_transition_list(A->states[renumber[tmp->n]]->outgoing_transitions,NULL,dup_symbol));
         if (is_final_state(A->states[renumber[tmp->n]])) {
            set_final_state(A->states[q]);
         }
      }
   }
}
free(renumber);
free_list_int(initials);
}

} // namespace unitex


/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "ElagStateSet.h"
#include "ElagDebug.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Determinizes an automaton with transitions tagged with Elag symbols.
 * The given automaton is modified.
 */
void elag_determinize(language_t* language,SingleGraph A,void (*free_elag_symbol)(symbol_t*)) {
SingleGraph res=new_SingleGraph(PTR_TAGS);
state_set_array* ARRAY=new_state_set_array();
state_set* initial_states=new_state_set();
struct list_int* l=get_initial_states(A);
struct list_int* l_tmp=l;
while (l_tmp!=NULL) {
   state_set_add(initial_states,A,l_tmp->n);
   l_tmp=l_tmp->next;
}
free_list_int(l);
/* First, we create a state set containing all the initial states */
state_set_array_add(ARRAY,initial_states);
free_state_set(initial_states);
for (int current_state_set=0;current_state_set<ARRAY->size;current_state_set++) {
   /* Now, we process each state set, corresponding to a new state in the
    * deterministic automaton */
   /* We compute the output transitions of the new state. Those transitions
    * will point to state sets */
   STATE_t* Q=new_STATE_t(language,ARRAY->state_sets[current_state_set]);
   if (current_state_set==0) {
      Q->flags|=AUT_INITIAL;
   }
   SingleGraphState q=add_state(res);
   if (Q->flags & AUT_INITIAL) {
      set_initial_state(q);
   }
   if (Q->flags & AUT_FINAL) set_final_state(q);
   for (TRANS_t* T=Q->transitions;T!=NULL;T=T->next) {
      /* For each outgoing transition, we test if the pointed state set
       * already exists in our state set array */
      int index=state_set_array_lookup(ARRAY,T->destination);
      if (index==-1) {
         /* If we have to create a new state in the result automaton */
         index=state_set_array_add(ARRAY,T->destination);
      }
      if (T->label->next!=NULL) {
    	  fatal_error("elag_determinize: symbol list error should not happen\n");
      }
      add_outgoing_transition(q,T->label,index);
   }
   if (Q->default_transition!=NULL && Q->default_transition->size!=0) {
      /* We deal with the default transition, if any */
      int index=state_set_array_lookup(ARRAY,Q->default_transition);
      if (index==-1) {
         /* If we have to create a new state in the result automaton */
         index=state_set_array_add(ARRAY,Q->default_transition);
      }
      add_outgoing_transition(q,SYMBOL_DEF,index);
   }
   free_STATE_t(Q);
}
free_state_set_array(ARRAY);
/* Now, we empty A's automaton and replace it by res */
move_SingleGraph(A,&res,free_elag_symbol);
}

} // namespace unitex

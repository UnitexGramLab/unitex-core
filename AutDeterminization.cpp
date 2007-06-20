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

#include "ElagStateSet.h"


/**
 * Determinizes an automaton with transitions tagged with Elag symbols.
 * The given automaton is modified.
 */
void elag_determinize(SingleGraph A) {
SingleGraph res=new_SingleGraph();
state_set_array* ARRAY=new_state_set_array();
state_set* initial_states=new_state_set();
struct list_int* l=get_initial_states(A);
while (l!=NULL) {
   state_set_add(initial_states,A,l->n);
   l=l->next;
}
free_list_int(l);
state_set_array_add(ARRAY,initial_states);
free_state_set(initial_states);
for (int current_state_set=0;current_state_set<ARRAY->size;current_state_set++) {
   STATE_t* Q=new_STATE_t(ARRAY->state_sets[current_state_set]);
   SingleGraphState q=add_state(res);
   if (Q->flags & AUT_INITIAL) set_initial_state(q);
   if (Q->flags & AUT_TERMINAL) set_final_state(q);
   for (TRANS_t* T=Q->transitions;T!=NULL;T=T->next) {
      int index=state_set_array_lookup(ARRAY,T->destination);
      if (index==-1) {
         /* If we have to create a new state in the result automaton */
         index=state_set_array_add(ARRAY,T->destination);
      }
      add_outgoing_transition(q,T->label,index);
   }
   if (Q->default_transition!=NULL && Q->default_transition->size) {
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
move_SingleGraph(A,&res,NULL);
}

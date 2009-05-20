 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include "Fst2Automaton.h"
#include "Symbol_op.h"
#include "ElagStateSet.h"
#include "AutComplementation.h"

/* see http://en.wikipedia.org/wiki/Variable_Length_Array . MSVC did not support it 
   see http://msdn.microsoft.com/en-us/library/zb1574zs(VS.80).aspx */
#if defined(_MSC_VER) && (!(defined(NO_C99_VARIABLE_LENGTH_ARRAY)))
#define NO_C99_VARIABLE_LENGTH_ARRAY 1
#endif


/**
 * This function returns a symbol list that matches
 * everything but the symbols matched by the given
 * transition list, or NULL if the transition list
 * matches everything.
 */
symbol_t* LEXIC_minus_transitions(language_t* language,Transition* trans) {
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
symbol_t** POS=(symbol_t**)malloc(sizeof(symbol_t*)*(language->POSs->size));
#else
symbol_t* POS[language->POSs->size];
#endif
int i;
/* First we build an array containing a full symbol for each POS.
 * For instance, we could have POS[0]=<A>, POS[1]=<V>, etc. */
for (i=0;i<language->POSs->size;i++) {
   POS[i]=new_symbol_POS((POS_t*)language->POSs->value[i],-1);
}
symbol_t* tmp;
while (trans!=NULL) {
   tmp=(symbol_t*)trans->label;
   if (tmp->type==S_LEXIC) {
      /* If a transition matches everything, then we can stop
       * and return NULL */
      for (i=0;i<language->POSs->size;i++) {
         free_symbols(POS[i]);
      }
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
	  free(POS);
#endif
      return NULL;
   }
   /* If we have a transition tagged by <A:s>, we have to replace
    * the POS array cell #z that corresponds to <A> by POS[z]-<A:s>,
    * that may give something like POS[z]=<A:p> */
   symbol_t* minus=symbols_minus_symbol(language,POS[tmp->POS->index],tmp);
   free_symbols(POS[tmp->POS->index]);
   POS[tmp->POS->index]=minus;
   trans=trans->next;
}
/* Finally, we concatenate all the symbols we have computed into
 * one symbol list */
symbol_t res;
res.next=NULL;
symbol_t* end=&res;
for (i=0;i<language->POSs->size;i++) {
   concat_symbols(end,POS[i],&end);
}
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
free(POS);
#endif
return res.next;
}


/**
 * Replaces the given automaton by its complement one.
 */
void elag_complementation(language_t* language,SingleGraph A) {
int sink_state_index=A->number_of_states;
SingleGraphState sink_state=add_state(A);
/* The sink state is not final (because finalities will be reversed
 * below), and its default transition loops back on itself */
sink_state->default_state=sink_state_index;
for (int q=0;q<A->number_of_states;q++) {
   /* We reverse the finality of each state */
   if (is_final_state(A->states[q])) {
      unset_final_state(A->states[q]);
   } else {
      set_final_state(A->states[q]);
   }
   if (A->states[q]->default_state==-1) {
      /* If there is no default transition, we create one that is
       * tagged by anything but the non default ones */
      symbol_t* s=LEXIC_minus_transitions(language,A->states[q]->outgoing_transitions);
      if (s!=NULL) {
         add_all_outgoing_transitions(A->states[q],s,sink_state_index);
         /* We have added a single transition tagged by a symbol list. Now
          * we replace it by a list of transitions, each one of them
          * tagged with a single symbol */
         flatten_transition(A->states[q]->outgoing_transitions);
      }
   }
}
}


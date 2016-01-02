/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Symbol.h"
#include "Symbol_op.h"
#include "Fst2Automaton.h"
#include "ElagStateSet.h"
#include "Transitions.h"
#include "AutIntersection.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This function looks for a transition of 'trans' that is tagged
 * with the symbol 's'. If found, the transition is extracted from
 * the list and returned.
 */
Transition* extract_transition(Transition **trans,symbol_t* s) {
if (*trans==NULL) {
   return NULL;
}
if (symbol_compare(s,(*trans)->label)==0) {
   /* If the first transition is the one we want */
   Transition* res=*trans;
   *trans=(*trans)->next;
   return res;
}
for (Transition* t=*trans;t->next!=NULL;t=t->next) {
   if (symbol_compare(s,t->next->label)==0) {
      Transition* res=t->next;
      t->next=t->next->next;
      return res;
   }
}
return NULL;
}


/**
 * Builds the intersection of A's state #q1 and B's state #q2.
 * Returns the index of the corresponding state in the result
 * automaton. This function is supposed to be used when we
 * intersect two elag grammars.
 */
int intersect_states_grammar_grammar(language_t* language,SingleGraph res,const SingleGraph A,int q1,
                     const SingleGraph B,int q2,int** renumber) {
if (renumber[q1][q2]!=-1) {
   /* Nothing to do if the job has already been done */
   return renumber[q1][q2];
}
int q=res->number_of_states;
renumber[q1][q2]=q;
SingleGraphState state=add_state(res);
if (is_initial_state(A->states[q1]) && is_initial_state(B->states[q2])) {
   /* If both q1 and q2 are initial, then the new state must be too */
   set_initial_state(state);
}
if (is_final_state(A->states[q1]) && is_final_state(B->states[q2])) {
   /* If both q1 and q2 are final, then the new state must be too */
   set_final_state(state);
}
/* We clone the transitions of q1 and q2 and we expand them in order to
 * compute their intersection easily */
Transition* transA=clone_transition_list(A->states[q1]->outgoing_transitions,NULL,dup_symbol);
Transition* transB=clone_transition_list(B->states[q2]->outgoing_transitions,NULL,dup_symbol);
expand_transitions(language,transA,transB);
int destination;
Transition* transa;
Transition* transb;
while (transA!=NULL) {
   /* We take one transition from q1's ones */
   transa=transA;
   transA=transA->next;
   /* And we take from q2's ones the same, if any */
   transb=extract_transition(&transB,transa->label);
   if (transb!=NULL) {
      /* If there is such a transition, we merge A and B's transitions */
      destination=intersect_states_grammar_grammar(language,res,A,transa->state_number,B,transb->state_number,renumber);
      add_outgoing_transition(res->states[q],transa->label,destination);
      /* And we can free B's one */
      free_Transition(transb,free_symbol);
   } else {
      /* If A's transition has no equivalent in B... */
      if (B->states[q2]->default_state!=-1) {
         /* ...it can match however with B's default transition, if any */
         destination=intersect_states_grammar_grammar(language,res,A,transa->state_number,B,B->states[q2]->default_state,renumber);
         add_outgoing_transition(res->states[q],transa->label,destination);
      }
   }
   /* We don't need transa anymore */
   free_Transition(transa,free_symbol);
}
if (A->states[q1]->default_state!=-1) {
   /* If q1 has a default transition, it will match
    * with all remaining transitions of B */
   while (transB!=NULL) {
      transb=transB;
      transB=transB->next;
      destination=intersect_states_grammar_grammar(language,res,A,A->states[q1]->default_state,B,transb->state_number,renumber);
      add_outgoing_transition(res->states[q],transb->label,destination);
      free_Transition(transb,free_symbol);
   }
   if (B->states[q2]->default_state!=-1) {
      /* If both q1 and q2 have default transitions */
      res->states[q]->default_state=intersect_states_grammar_grammar(language,res,A,A->states[q1]->default_state,
                                         B,B->states[q2]->default_state,renumber);
   }
} else {
   /* We don't need the remaining transitions from transB */
   free_Transition_list(transB,free_symbol);
}
return q;
}


/**
 * Builds the intersection of A's state #q1 and B's state #q2.
 * Returns the index of the corresponding state in the result
 * automaton. This function is supposed to be used when we
 * intersect an elag grammar with a sentence automaton.
 *
 * A is the text
 * B is the grammar
 */
int intersect_states_text_grammar(SingleGraph res,const SingleGraph A,int q1,
                     const SingleGraph B,int q2,int** renumber) {
if (renumber[q1][q2]!=-1) {
   /* Nothing to do if the job has already been done */
   return renumber[q1][q2];
}
int q=res->number_of_states;
renumber[q1][q2]=q;
SingleGraphState state=add_state(res);
if (is_initial_state(A->states[q1]) && is_initial_state(B->states[q2])) {
   /* If both q1 and q2 are initial, then the new state must be too */
   set_initial_state(state);
}
if (is_final_state(A->states[q1]) && is_final_state(B->states[q2])) {
   /* If both q1 and q2 are final, then the new state must be too */
   set_final_state(state);
}

for (Transition* transA=A->states[q1]->outgoing_transitions;transA!=NULL;transA=transA->next) {
   int found=0;
   for (Transition* transB=B->states[q2]->outgoing_transitions;transB!=NULL;transB=transB->next) {
      if (symbol_in_symbol(transA->label,transB->label)) {
     	 if (found) {
            fatal_error("intersect_states_text_grammar: non deterministic automaton\n");
         }
         found=1;
         int destination=intersect_states_text_grammar(res,A,transA->state_number,B,transB->state_number,renumber);
         add_outgoing_transition(res->states[q],transA->label,destination);
      }
   }
   if (!found && B->states[q2]->default_state!=-1) {
      int destination=intersect_states_text_grammar(res,A,transA->state_number,B,B->states[q2]->default_state,renumber);
      add_outgoing_transition(res->states[q],transA->label,destination);
   }
}
return q;
}



/**
 * Returns the intersection of the two given automata. A and B
 * are supposed to be deterministic. 'type' is used to determine
 * whether we are intersecting 2 elag grammars or 1 elag grammar
 * and 1 sentence automaton.
 */
SingleGraph elag_intersection(language_t* language,const SingleGraph A,const SingleGraph B,int type) {
int initial_A=get_initial_state(A);
int initial_B=get_initial_state(B);
if (initial_A==-2 || initial_B==-2) {
   fatal_error("Non deterministic automaton(a) in elag_intersection\n");
}

if (initial_A==-1 || initial_B==-1) {
   /* If there is no initial in A or B, then the intersection is empty */
   return new_SingleGraph(0,PTR_TAGS);
}
SingleGraph res=new_SingleGraph(A->number_of_states*B->number_of_states,PTR_TAGS);
/* We initialize the renumber matrix */
int** renumber=(int**)malloc(A->number_of_states*sizeof(int*));
if (renumber==NULL) {
   fatal_alloc_error("elag_intersection");
}
int i;
for (i=0;i<A->number_of_states;i++) {
   renumber[i]=(int*)malloc(B->number_of_states*sizeof(int));
   if (renumber[i]==NULL) {
      fatal_alloc_error("elag_intersection");
   }
   for (int j=0;j<B->number_of_states;j++) {
      renumber[i][j]=-1;
   }
}
if (type==GRAMMAR_GRAMMAR) {
   intersect_states_grammar_grammar(language,res,A,initial_A,B,initial_B,renumber);
} else if (type==TEXT_GRAMMAR) {
   intersect_states_text_grammar(res,A,initial_A,B,initial_B,renumber);
} else {
   fatal_error("Invalid type in elag_intersection\n");
}
resize(res);
/* And we free the renumber matrix */
for (i=0;i<A->number_of_states;i++) {
   free(renumber[i]);
}
free(renumber);
return res;
}

} // namespace unitex



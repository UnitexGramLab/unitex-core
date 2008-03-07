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
#include "symbol.h"
#include "symbol_op.h"
#include "autalmot.h"
#include "ElagStateSet.h"



/**
 * This function looks for a transition of 'trans' that is tagged
 * with the symbol 's'. If found, the transition is extracted from
 * the list and returned.
 */
Transition* extract_transition(Transition **trans,symbol_t* s) {
if (*trans==NULL) {
   return NULL;
}
if (symbol_compare(s,(symbol_t*)(*trans)->label)==0) {
   /* If the first transition is the one we want */
   Transition* res=*trans;
   *trans=(*trans)->next;
   return res;
}
for (Transition* t=*trans;t->next!=NULL;t=t->next) {
   if (symbol_compare(s,(symbol_t*)t->next->label)==0) {
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
 * automaton.
 */
int intersect_states(SingleGraph res,const SingleGraph A,int q1,
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
Transition* transA=clone_transition_list(A->states[q1]->outgoing_transitions,NULL,(void*(*)(void*))dup_symbol);
Transition* transB=clone_transition_list(B->states[q2]->outgoing_transitions,NULL,(void*(*)(void*))dup_symbol);
expand_transitions(transA,transB);
int destination;
Transition* transa;
Transition* transb;
while (transA!=NULL) {
   /* We take one transition from q1's ones */
   transa=transA;
   transA=transA->next;
   /* And we take from q2's ones the same, if any */
   transb=extract_transition(&transB,(symbol_t*)transa->label);
   if (transb!=NULL) {
      /* If there is such a transition, we merge A and B's transitions */
      destination=intersect_states(res,A,transa->state_number,B,transb->state_number,renumber);
      add_outgoing_transition(res->states[q],transa->label,destination);
      /* We NULL transa so that we can free transa without affecting
       * the transition we have just added to q */
      transa->label=NULL; 
      /* And we can free B's one */
      free_Transition(transb,(void(*)(void*))free_symbol);
   } else {
      /* If A's transition has no equivalent in B... */
      if (B->states[q2]->default_state!=-1) {
         /* ...it can match however with B's default transition, if any */
         destination=intersect_states(res,A,transa->state_number,B,B->states[q2]->default_state,renumber);
         add_outgoing_transition(res->states[q],transa->label,destination);
         /* See above */
         transa->label=NULL; 
      }
   }
   /* We don't need transa anymore */
   free_Transition(transa);
}
if (A->states[q1]->default_state!=-1) {
   /* If q1 has a default transition, it will match
    * with all remaining transitions of B */
   while (transB!=NULL) {
      transb=transB;
      transB=transB->next;
      destination=intersect_states(res,A,A->states[q1]->default_state,B,transb->state_number,renumber);
      add_outgoing_transition(res->states[q],transb->label,destination);
      /* See above */
      transb->label=NULL;
      free_Transition(transb);
   }
   if (B->states[q2]->default_state!=-1) {
      /* If both q1 and q2 have default transitions */
      res->states[q]->default_state=intersect_states(res,A,A->states[q1]->default_state,
                                         B,B->states[q2]->default_state,renumber);
   }
} else {
   /* We don't need the remaining transitions from transB */
   free_Transition_list(transB,(void (*)(void*))free_symbol);
}
return q;
}


/**
 * Returns the intersection of the two given automata. A and B
 * are supposed to be deterministic.
 */
SingleGraph elag_intersection(const SingleGraph A,const SingleGraph B) {
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
   fatal_error("Not enough memory in elag_intersection\n");
}
int i;
for (i=0;i<A->number_of_states;i++) {
   renumber[i]=(int*)malloc(B->number_of_states*sizeof(int));
   if (renumber[i]==NULL) {
      fatal_error("Not enough memory in elag_intersection\n");
   }
   for (int j=0;j<B->number_of_states;j++) {
      renumber[i][j]=-1;
   }
}
intersect_states(res,A,initial_A,B,initial_B,renumber);
resize(res);
/* And we free the renumber matrix */
for (i=0;i<A->number_of_states;i++) {
   free(renumber[i]);
}
free(renumber);
return res;
}




static int interStateAtom(Fst2Automaton * res, const Fst2Automaton * A, int q1, const Fst2Automaton * B,
                          int q2, int ** corresp) {

  if (corresp[q1][q2] != -1) { return corresp[q1][q2]; }

  int q = corresp[q1][q2] = autalmot_add_state(res);

  if ((A->states[q1].flags & AUT_INITIAL) && (B->states[q2].flags & AUT_INITIAL)) {
    autalmot_set_initial(res, q);
  }

  if ((A->states[q1].flags & AUT_TERMINAL) && (B->states[q2].flags & AUT_TERMINAL)) {
    autalmot_set_terminal(res, q);
  }


  for (transition_t * t1 = A->states[q1].trans; t1; t1 = t1->next) {

    //debug("process :"); symbol_dump(t1->label); endl();

    if (t1->label->POS->ignorable) { // skip ignorable tokens
      //debug("skip ignorable :"); symbol_dump(t1->label); endl();
      //debug("IGNORABLE\n");
      int to = interStateAtom(res, A, t1->to, B, q2, corresp);
      add_transition(res, q, t1->label, to);
      continue;
      //debug("CONTINUE\n");
    }
 

    bool found = false;

    for (transition_t * t2 = B->states[q2].trans; t2 && ! found; t2 = t2->next) {

      //debug("t2="); symbol_dump(t2->label); endl();

      if (symbol_in_symbol(t1->label, t2->label)) {
        //debug("  symbols matches\n");
	if (found) {
	  error("interStateAtom: non deterministic automaton\n");
	}

	found = true;

	int to = interStateAtom(res, A, t1->to, B, t2->to, corresp);
	add_transition(res, q, t1->label, to);
      } //else { debug("  DONT MATCH\n"); }
    }

//#warning "should no have def trans ???"
    if (! found && B->states[q2].defto != -1) {   
      int to = interStateAtom(res, A, t1->to, B, B->states[q2].defto, corresp);
      add_transition(res, q, t1->label, to);
    }
  }

  return q;
}


Fst2Automaton * interAutAtome(const Fst2Automaton * A, const Fst2Automaton * B) {

  if ((A->nbinitials > 1) || (B->nbinitials > 1)) {
    error("a nbstates=%d & b->nbstates=%d\n", A->nbinitials, B->nbinitials);
    fatal_error("autalmot_interAutAtome: non deterministic auto\n");
  }

  Fst2Automaton * res = new_Fst2Automaton(A->name, A->nbstates * B->nbstates);

  if (A->nbinitials == 0 || B->nbinitials == 0) {
    error("interAutAtome: auto is void\n");
    return res;
  }

  int ** corresp = (int **) xmalloc(A->nbstates * sizeof(int *));

  int i;
  for (i = 0; i < A->nbstates; i++) {

    corresp[i] = (int *) xmalloc(B->nbstates * sizeof(int));

    for (int j = 0; j < B->nbstates; j++) { corresp[i][j] = -1; }
  }


  interStateAtom(res, A, A->initials[0], B, B->initials[0], corresp);

  autalmot_resize(res);

  for (i = 0; i < A->nbstates; i++) { free(corresp[i]); }

  free(corresp);

  //  debug("out of interAutAtom:\n"); autalmot_dump(res);
  //  autalmot_dump_dot_fname(res, "interAutAtom.out");

  return res;				     
}

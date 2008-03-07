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
#include "symbol_op.h"
#include "Error.h"
#include "ElagStateSet.h"



/**
 * Allocates, initializes and returns a new state id.
 */
state_id* new_state_id(SingleGraph A, int n,state_id* next) {
state_id* id=(state_id*)malloc(sizeof(state_id));
if (id==NULL) {
   fatal_error("Not enough memory in new_state_id\n");
}
id->automaton=A;
id->state_number=n;
id->next=next;
return id;
}


/**
 * Frees the given single state_id.
 */
void free_state_id(state_id* id) {
if (id==NULL) return;
free(id);
}


/**
 * Allocates, initializes and returns a new state set.
 */
state_set* new_state_set() {
state_set* res=(state_set*)malloc(sizeof(state_set));
if (res==NULL) {
   fatal_error("Not enough memory in new_state_set\n");
}
res->state_list=NULL;
res->size=0;
return res;
}


/**
 * Frees the given state set.
 */
void free_state_set(state_set* s) {
if (s==NULL) return;
while (s->state_list!=NULL) {
   state_id* next=s->state_list->next;
   free_state_id(s->state_list);
   s->state_list=next;
}
free(s);
}


/**
 * Returns a copy of the given state set.
 */
state_set* clone_state_set(const state_set* s) {
if (s==NULL) return NULL;
state_set* res=new_state_set();
for (state_id* id=s->state_list;id!=NULL;id=id->next) {
   state_set_add(res,id->automaton,id->state_number);
}
return res;
}


/**
 * Adds the given state to the given state set.
 */
void state_set_add(state_set* S,SingleGraph A,int state_number) {
/* If the state set was empty */
if (S->size==0) {
   S->state_list=new_state_id(A,state_number,NULL);
   S->size++;
   return;
}
/* If the state number is the first of the list, nothing to do */
if (state_number==S->state_list->state_number) {
   return;
}
/* If we must insert the state at the first position */
if (state_number<S->state_list->state_number) {
   S->state_list=new_state_id(A,state_number,S->state_list);
} else {
   state_id* id;
   for (id=S->state_list;id->next!=NULL && (id->next->state_number<state_number);id=id->next);
   if (id->next!=NULL && (id->next->state_number==state_number)) {
      /* If the state is already in the set, nothing to do */
      return;
   }
   id->next=new_state_id(A,state_number,id->next);
}
S->size++;
}


/**
 * Tests if the two given state sets are identical.
 */
bool state_set_equals(state_set* S1,state_set* S2) {
if (S1==NULL || S2==NULL) {
   fatal_error("NULL error in state_set_equals\n");
}
if (S1==S2) {
   return true;
}
if (S1->size!=S2->size) {
   return false;
}
state_id* id1;
state_id* id2;
for (id1=S1->state_list,id2=S2->state_list;id1!=NULL && id2!=NULL;id1=id1->next,id2=id2->next) {
   if ((id1->automaton!=id2->automaton) || (id1->state_number!=id2->state_number)) {
      return false;
   }
}
return (id1==id2);
}


/**
 * Allocates, initializes and returns a new TRANS_t.
 */
TRANS_t* new_TRANS_t(symbol_t* s,TRANS_t* next) {
TRANS_t* T=(TRANS_t*)malloc(sizeof(TRANS_t));
if (T==NULL) {
   fatal_error("Not enough memory in new_TRANS_t\n");
}
T->label=s;
T->destination=new_state_set();
T->next=next;
return T;
}


/**
 * Frees the memory associated to the given single transition.
 * Note that we don't free the symbol that tags it since
 * it is shared with other objects.
 */
void free_TRANS_t(TRANS_t* T) {
if (T==NULL) return;
free_state_set(T->destination);
free(T);
}


/**
 * Frees the memory associated to the given transition list.
 */
void free_TRANS_t_list(TRANS_t* T) {
while (T!=NULL) {
   TRANS_t* tmp=T->next;
    free_TRANS_t(T);
    T=tmp;
}
}


/**
 * This function looks if the given symbol tags a transition
 * of the given transition list. It returns the transition
 * or NULL if not found.
 */
TRANS_t* TRANS_t_lookup(TRANS_t* T,symbol_t* s) {
while (T!=NULL) {
   if (symbol_compare(T->label,s)==0) {
      break;
   }
   T=T->next;
}
return T;
}


/**
 * Allocates, initializes and returns a state set array of the given capacity.
 */
state_set_array* new_state_set_array(int capacity) {
state_set_array* s=(state_set_array*)malloc(sizeof(state_set_array));
if (s==NULL) {
   fatal_error("Not enough memory in new_state_set_array\n");
}
if (capacity<=0) {
   capacity=1;
}
s->state_sets=(state_set**)malloc(sizeof(state_set*)*capacity);
if (s->state_sets==NULL) {
   fatal_error("Not enough memory in new_state_set_array\n");
}
s->capacity=capacity;
s->size=0;
return s;
}


/**
 * Frees all the memory associated to the given state set array.
 */
void free_state_set_array(state_set_array* s) {
if (s==NULL) return;
for (int i=0;i<s->size;i++) {
   free_state_set(s->state_sets[i]);
}
free(s->state_sets);
free(s);
}


/**
 * Adds a copy of the given state set to the given state set array.
 * The function returns the index of the state set.
 */
int state_set_array_add(state_set_array* ARRAY,state_set* s) {
if (ARRAY->size==ARRAY->capacity) {
   /* If necessary, we enlarge the array doubling its capacity */
   ARRAY->capacity=ARRAY->capacity*2;
   ARRAY->state_sets=(state_set**)realloc(ARRAY->state_sets,ARRAY->capacity*sizeof(state_set*));
   if (ARRAY->state_sets==NULL) {
      fatal_error("Not enough memory in state_set_array_add\n");
   }
}
ARRAY->state_sets[ARRAY->size++]=clone_state_set(s);
return ARRAY->size-1;
}


/**
 * This function tests if the given state set belongs to the given
 * state set array. It returns its index, or -1 if not found.
 */
int state_set_array_lookup(state_set_array* ARRAY,state_set* s) {
for (int i=0;i<ARRAY->size;i++) {
   if (state_set_equals(ARRAY->state_sets[i],s)) {
      return i;
   }
}
return -1;
}


/**
 * Replaces the first symbol of the symbol list a with the symbol list b.
 */
void replace_symbol(symbol_t* a,symbol_t* b) {
symbol_t* next=a->next;
empty_symbol(a);
copy_symbol(a,b);
free_symbol(b);
concat_symbols(a,next);
}


/**
 * Compares the symbols a and b, expanding them if necessary.
 * When a symbol is expanded, it is replaced by a list of distinct
 * symbols, so that the union of these symbols is equivalent to the
 * original symbol.
 */
void compare_and_expand_symbol_with_symbol(symbol_t* a, symbol_t* b) {
if (symbol_compare(a,b)==0) {
   /* Nothing to do if the symbols are identical */
   return;
}
symbol_t* i=symbol_inter_symbol(a,b);
if (i==NULL) {
   /* Nothing to do if the intersection is empty */
   return;
}
if (i->next!=NULL) {
   fatal_error("compare_and_expand_symbol_with_symbol: i->next not NULL\n");
}
symbol_t* aminusb=symbol_minus_symbol(a,i);
symbol_t* bminusa=symbol_minus_symbol(b,i);
if (aminusb==NULL) {
   if (symbol_compare(a,i)) {
      fatal_error("compare_and_expand_symbol_with_symbol: A!=I and A\\I=null\n");
   }
}
if (bminusa==NULL) {
   if (symbol_compare(b,i)) {
      fatal_error("compare_and_expand_symbol_with_symbol: B!=I and B\\I=null\n");
   }
}
symbol_t* copy_of_i=dup_symbol(i);
/* We replace a with (a^b) v (a-b) */
concat_symbols(copy_of_i,aminusb);
replace_symbol(a,copy_of_i);
/* We replace b with (a^b) v (b-a) */
concat_symbols(i,bminusa);
replace_symbol(b,i);
}


/**
 * Compares the symbol a with the symbol list b. Then, for
 * each pair (a,b[i]), it compares the two symbols, expanding them
 * if necessary.
 */
void compare_and_expand_symbol_with_symbols(symbol_t * a, symbol_t * b) {
while (b!=NULL) {
   compare_and_expand_symbol_with_symbol(a,b);
   b=b->next;
}
}


/**
 * Compares each symbol of a with each one of b, expanding
 * them if necessary.
 */
void expand_symbols(symbol_t* a,symbol_t* b) {
while (a!=NULL) {
   compare_and_expand_symbol_with_symbols(a,b);
   a=a->next;
}
}


/**
 * This function takes a symbol list and compares each element with
 * its follower. Then, it expands them if necessary.
 */
void expand_symbols(symbol_t* s) {
if (s==NULL) {
   return;
}
while (s->next!=NULL) {
   compare_and_expand_symbol_with_symbols(s,s->next);
   s=s->next;
}
}


/**
 * This function takes a transition list, where each transition
 * can be tagged with a symbol list, and it modifies it so that 
 * we will have exactly one transition per symbol.
 */
void flatten_transition(Transition* trans) {
while (trans!=NULL) {
   if (trans->label==NULL) {
      fatal_error("flatten_transition: NULL tagged transition\n");
   }
   symbol_t* symbol=(symbol_t*)trans->label;
   if (symbol->next!=NULL) {
      trans->next=new_Transition(symbol->next,trans->state_number,trans->next);
      symbol->next=NULL;
   }
   trans=trans->next;
}
}


/**
 * This function looks if the given symbol tags a transition
 * of the given transition list. It returns the transition
 * or NULL if not found.
 */
Transition* Transition_lookup(Transition* trans,symbol_t* s) {
while (trans!=NULL) {
   if (symbol_compare((symbol_t*)trans->label,s)==0) {
      break;
   }
}
return trans;
}


/**
 * Expands the two given transition lists.
 */
void expand_transitions(Transition* _t1,Transition* _t2) {
for (Transition* t1=_t1;t1!=NULL;t1=t1->next) {
   for (Transition* t2=_t2;t2!=NULL;t2=t2->next) {
      expand_symbols((symbol_t*)(t1->label),(symbol_t*)(t2->label));
   }
}
flatten_transition(_t1);
flatten_transition(_t2);
}



/**
 * Expands the given transition list, so that all its symbols
 * with have empty intersection between them.
 */
void expand_transitions(Transition* trans) {
Transition* t1;
Transition* t2;
for (t1=trans;t1!=NULL;t1=t1->next) {
   expand_symbols((symbol_t*)t1->label);
}
for (t1=trans;t1!=NULL;t1=t1->next) {
   for (t2=t1->next;t2!=NULL;t2=t2->next) {
      expand_symbols((symbol_t*)(t1->label),(symbol_t*)(t2->label));
   }
}
flatten_transition(trans);
}


/**
 * This function takes two states and expands their transitions, including
 * the default ones, if any.
 */
void expand_transitions(SingleGraphState q1,SingleGraphState q2) {
expand_transitions(q1->outgoing_transitions,q2->outgoing_transitions);
if (q1->default_state!=-1) {
   /* If q1 has a default transition, then for each
    * symbol s that tags a transition from q2 but not from 
    * q1, we add a transition q1 --s--> q1's default state */
   for (Transition* t=q2->outgoing_transitions;t!=NULL;t=t->next) {
      if (Transition_lookup(q1->outgoing_transitions,(symbol_t*)t->label)==NULL) {
         add_outgoing_transition(q1,t->label,q1->default_state);
      }
   }
}
if (q2->default_state!=-1) {
   /* If q2 has a default transition, the same as above */
   for (Transition* t=q1->outgoing_transitions;t!=NULL;t=t->next) {
      if (Transition_lookup(q2->outgoing_transitions,(symbol_t*)t->label)==NULL) {
         add_outgoing_transition(q2,t->label,q2->default_state);
      }
   }
}
}


/**
 * Expands all the transitions in the original states that belong
 * to state set of the given new state.
 */
void expand_transitions(STATE_t* Q) {
for (state_id* id=Q->original_state_set->state_list;id!=NULL;id=id->next) {
   /* First, we expand the transitions of the original states of Q */
   SingleGraphState q=id->automaton->states[id->state_number];
   expand_transitions(q->outgoing_transitions);
}
for (state_id* id1=Q->original_state_set->state_list;id1!=NULL;id1=id1->next) {
   for (state_id* id2=id1->next;id2!=NULL;id2=id2->next) {
      SingleGraphState q1=id1->automaton->states[id1->state_number];
      SingleGraphState q2=id2->automaton->states[id2->state_number];
      expand_transitions(q1,q2);
    }
  }
}


/**
 * This function builds the transitions field of
 * the given STATE_t. Note that transitions must have been
 * expanded before doing this.
 */
void build_TRANS_t(STATE_t* Q) {
for (state_id* id=Q->original_state_set->state_list;id!=NULL;id=id->next) {
   /* We examine each state of the state set */
   SingleGraphState q=id->automaton->states[id->state_number];
   if (q->default_state!=-1) {
      state_set_add(Q->default_transition,id->automaton,q->default_state);
   }
   for (Transition* t=q->outgoing_transitions;t!=NULL;t=t->next) {
      TRANS_t* T;
      if ((T=TRANS_t_lookup(Q->transitions,(symbol_t*)t->label))==NULL) {
         /* If we have to create a new symbol */
         T=new_TRANS_t((symbol_t*)t->label,Q->transitions);
         Q->transitions=T;
      }
      state_set_add(T->destination,id->automaton,t->state_number);
   }
}
}


/**
 * Allocates, initializes and returns a new STATE_t from the
 * given state set.
 */
STATE_t* new_STATE_t(state_set* s) {
STATE_t* res=(STATE_t *)malloc(sizeof(STATE_t));
if (res==NULL) {
   fatal_error("Not enough memory in new_STATE_t\n");
}
res->original_state_set=clone_state_set(s);
res->transitions=NULL;
res->default_transition=new_state_set();
/* Flags stuffs:
 * are states are initial ones => Q is an initial one
 * one state is a final one    => Q is a final one
 */
res->flags=AUT_INITIAL;
for (state_id* id=res->original_state_set->state_list;id!=NULL;id=id->next) {
   if (!is_initial_state(id->automaton->states[id->state_number])) {
      res->flags&=~(AUT_INITIAL);
   }
   if (is_final_state(id->automaton->states[id->state_number])) {
      res->flags|=AUT_TERMINAL;
   }
}
/* We expand the transitions of the original automaton */
expand_transitions(res);
/* And we build the transitions of our STATE_t */
build_TRANS_t(res);
return res;
}


/**
 * Frees all the memory associated to the given STATE_t.
 */
void free_STATE_t(STATE_t* Q) {
if (Q==NULL) return;
free_state_set(Q->original_state_set);
free_TRANS_t_list(Q->transitions);
free_state_set(Q->default_transition);
free(Q);
}





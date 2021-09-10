/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "SymbolAlphabet.h"
#include "AutMinimization.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure defines a collection of transitions to be
 * used by the minimization algorithm. It's an internal data
 * structure that must not be visible outside this library.
 */
typedef struct TransitionCollection_ {
   int tag_number;
   int state_number;
   /* This color indication is used to identify state partitions */
   int destination_color;
   struct TransitionCollection_* next;
} TransitionCollection;



/**
 * Allocates, initializes and returns a new transition.
 */
TransitionCollection* new_TransitionCollection(int tag_number,int state_number,
                                               int destination_color,
                                               TransitionCollection* next=NULL) {
TransitionCollection* res=(TransitionCollection*)malloc(sizeof(TransitionCollection));
if (res==NULL) {
   fatal_alloc_error("new_TransitionCollection");
}
res->tag_number=tag_number ;
res->state_number=state_number;
res->destination_color=destination_color;
res->next=next;
return res;
}


/**
 * Frees the memory associated to the given transition list.
 */
void free_TransitionCollection(TransitionCollection* trans) {
while (trans!=NULL) {
   TransitionCollection* next=trans->next;
   free(trans);
   trans=next;
}
}


/**
 * Inserts the given element at the correct place in the transition
 * list of state #s. The sort is done by increasing tag numbers.
 * Note that the element is supposed not to be in the list before, what
 * should be true if the input automaton was a deterministic one.
 */
void sorted_insert(TransitionCollection* element,TransitionCollection **list,int s) {
TransitionCollection* t;
TransitionCollection* previous=NULL;
for (t=list[s];t!=NULL;t=t->next) {
   if (element->tag_number<t->tag_number) {
      /* If we have to insert the element before the end of the list */
      element->next=t;
      if (previous!=NULL) {
         previous->next=element;
      } else {
         list[s]=element;
      }
      return;
   }
   previous=t;
}
/* If the element must be inserted at the end of the list */
element->next=NULL;
if (previous) {
   previous->next=element;
} else {
   list[s]=element;
}
}


/**
 * For each state, builds the sorted list of outgoing transitions.
 */
TransitionCollection** build_transition_collections(SingleGraph A,SymbolAlphabet* alph) {
TransitionCollection** trans=(TransitionCollection**)malloc(A->number_of_states*sizeof(TransitionCollection*));
if (trans==NULL) {
   fatal_alloc_error("build_transition_collections");
}
for (int e=0;e<A->number_of_states;e++) {
   trans[e]=NULL;
   for (Transition* t=A->states[e]->outgoing_transitions;t!=NULL;t=t->next) {
      TransitionCollection* temp=new_TransitionCollection(alphabet_lookup(alph,(symbol_t*)t->label),t->state_number,0);
      sorted_insert(temp,trans,e);
   }
   if (A->states[e]->default_state!=-1) {
      /* If there is a default transition */
      TransitionCollection* temp=new_TransitionCollection(alphabet_lookup(alph,SYMBOL_DEF),A->states[e]->default_state,0);
      sorted_insert(temp,trans,e);
   }
}
return trans;
}


/**
 * Returns 0 if the given transitions list are identical; any non null
 * value otherwise.
 */
int compare_transitions(TransitionCollection* t1,TransitionCollection* t2) {
while (t1!=NULL && t2!=NULL) {
   if ((t1->tag_number!=t2->tag_number) || (t1->destination_color!=t2->destination_color)) {
      return 1;
   }
   t1=t1->next;
   t2=t2->next;
}
return (t1!=t2);
}


/**
 * Allocates, initializes and returns an array that associates
 * a color (0 or 1) to each state of 'A', making sure that the
 * state #0 will be colored with 0. '*nbColors' will be set to
 * the number of colors that have been used (1 if all states
 * have the same finality; 2 otherwise).
 */
int* init_colors(SingleGraph A,int *nbColors) {
int* color=(int*)calloc(A->number_of_states,sizeof(int));
if (color==NULL) {
   fatal_alloc_error("init_colors");
}
/* bicolor will indicate if all states are of the same color (finality) or
 * not */
bool bicolor=false;
if (is_final_state(A->states[0])) {
   /* We distinguish two cases (initial state final or not), just
    * to ensure that the color of the initial state #0 will be 0 */
   for (int e=0;e<A->number_of_states;e++) {
      color[e]=is_final_state(A->states[e])?0:(bicolor=true,1);
   }
} else {
   for (int e=0;e<A->number_of_states;e++) {
      color[e]=is_final_state(A->states[e])?(bicolor=true,1):0;
   }
}
(*nbColors)=(bicolor?2:1);
return color;
}


/**
 * Updates the color of the transitions' destination states.
 */
void update_colors(TransitionCollection** transitions,int* colors,int nbStates) {
for (int s=0;s<nbStates;s++) {
   for (TransitionCollection* t=transitions[s];t!=NULL;t=t->next) {
      t->destination_color=colors[t->state_number];
   }
}
}


/**
 * Returns the shade of the state #s, updating '*nbShades' if this is
 * a new shade. The state #s is compared with all the states of the
 * same color. Note that the states with the same color than 's' are
 * supposed to be in the range [color(s);s].
 */
int get_shade(int s,TransitionCollection** trans,int* color,int* shade,int *nbShades) {
for (int i=color[s];i<s;i++) {
   if (color[i]==color[s]) {
      if (compare_transitions(trans[s],trans[i])==0) {
         return shade[i];
      }
   }
}
/* If we have to create a new shade */
(*nbShades)++;
return (*nbShades)-1;
}


/**
 * For each color, a state of this color is chosen to represent the color.
 * The chosen number is >= its color number.
 */
int* choose_states(int* color,int nbColors,int nbStates) {
int* chosen=(int*)malloc(nbColors*sizeof(int));
if (chosen==NULL) {
   fatal_alloc_error("choose_states");
}
for (int c=0;c<nbColors;c++) {
   bool found=false;
   for (int s=c;!found && s<nbStates;s++) {
      if (color[s]==c) {
         chosen[c]=s;
         found=true;
      }
   }
   if (!found) {
      fatal_error("choose_states: color %d not found!\n",c);
   }
}
return chosen;
}


/**
 * Removes all transitions whose destination state is n.
 */
Transition* clean_transitions(Transition* trans, int n) {
while (trans!=NULL && trans->state_number==n) {
   Transition* next=trans->next;
   free_Transition_list(trans,free_symbol);
   trans=next;
}
for (Transition* t=trans;t!=NULL;t=t->next) {
   while (t->next!=NULL && t->next->state_number==n) {
      Transition* next=t->next->next;
      free_Transition_list(t->next,free_symbol);
      t->next=next;
   }
}
return trans;
}


/**
 * This function removes transitions that take the same way than the
 * default ones.
 */
void compact_default_transitions(SingleGraph g) {
for (int q=0;q<g->number_of_states;q++) {
   if (g->states[q]->default_state!=-1) {
      g->states[q]->outgoing_transitions=clean_transitions(g->states[q]->outgoing_transitions,g->states[q]->default_state);
   }
}
}


/**
 * This function minimizes the given automaton. Note
 * that it must be deterministic. For more information,
 * see comments in this library's .h file.
 */
void elag_minimize(SingleGraph automaton,int level) {
struct list_int* initials=get_initial_states(automaton);
if (initials==NULL) {
   /* No initial state should mean 'empty automaton' */
   if (automaton->number_of_states!=0) {
      /* If not, we fail */
      fatal_error("No initial state in non empty automaton in elag_minimize\n");
   }
   return;
}
if (initials->next!=NULL) {
   fatal_error("Non-deterministic automaton in elag_minimize\n");
}
free_list_int(initials);
if (level>0) {
   /* If necessary, we remove transitions that are included in the
    * default ones */
   compact_default_transitions(automaton);
}
SymbolAlphabet* alph=build_symbol_alphabet(automaton);
TransitionCollection** transitions=build_transition_collections(automaton,alph);
/* Now that we have numbered transitions, we don't need the symbol
 * alphabet anymore */
free_SymbolAlphabet(alph);
int nbColors;
int nbShades;
int* color=(int*)calloc(automaton->number_of_states,sizeof(int));
if (color==NULL) {
   fatal_alloc_error("elag_minimize");
}
int* shade=init_colors(automaton,&nbShades);
do {
   int s;
   /* We copy the shades into the color array */
   for (s=0;s<automaton->number_of_states;s++) {
      color[s]=shade[s];
   }
   nbColors=nbShades;
   nbShades=0;
   /* We update the colors of the transitions' destination states */
   update_colors(transitions,color,automaton->number_of_states);
   /* Now, for each state #s, we look for its shade, comparing it with
    * all the states #i so that i<s */
   for (s=0;s<automaton->number_of_states;s++) {
      shade[s]=get_shade(s,transitions,color,shade,&nbShades);
   }
   /* We stop when no more shades have been introduced */
} while (nbColors!=nbShades);
int* chosen=choose_states(color,nbColors,automaton->number_of_states);
for (int i=0;i<automaton->number_of_states;i++) {
   free_TransitionCollection(transitions[i]);
}
free(transitions);
free(shade);
/* We allocate the resulting automaton */
SingleGraph result=new_SingleGraph(nbColors,PTR_TAGS);
for (int c=0;c<nbColors;c++) {
   SingleGraphState state=add_state(result);
   SingleGraphState original=automaton->states[chosen[c]];
   /* We set the initiality and finality of the state */
   state->control=original->control;
   state->outgoing_transitions=original->outgoing_transitions;
   original->outgoing_transitions=NULL;
   /* We renumber the transitions' destination states */
   for (Transition* t1=state->outgoing_transitions;t1!=NULL;t1=t1->next) {
      t1->state_number=color[t1->state_number];
   }
   state->default_state=original->default_state;
}
/* Now we have to replace the old automaton by the new one */
move_SingleGraph(automaton,&result,free_symbol);
/* And we don't need these arrays anymore */
free(color);
free(chosen);
}

} // namespace unitex


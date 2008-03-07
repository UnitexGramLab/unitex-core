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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "SingleGraph.h"
#include "Error.h"
#include "BitMasks.h"
#include "List_int.h"
#include "BitArray.h"
#include "HashTable.h"
#include "FIFO.h"

#define DEFAULT_STATE_ARRAY_SIZE 256

/*
 * These constants are declared here instead of in the .h because
 * we don't want people to write tests like (e->control & FINAL_STATE_BIT_MASK)
 * We prefer writing: is_final_state(e)
 */
#define FINAL_STATE_BIT_MASK 1
#define INITIAL_STATE_BIT_MASK 2
#define ACCESSIBILITY_BIT_MASK 4
#define CO_ACCESSIBILITY_BIT_MASK 8



/**
 * Allocates, initializes and returns a new SingleGraph.
 * 'size' specifies the initial size of the state array.
 */
SingleGraph new_SingleGraph(int size,TagType type) {
SingleGraph g =(SingleGraph)malloc(sizeof(struct single_graph_));
if (g==NULL) {
   fatal_error("Not enough memory in new_SingleGraph\n");
}
g->number_of_states=0;
g->capacity=0;
g->states=NULL;
set_state_array_capacity(g,size);
g->tag_type=type;
return g;
}


/**
 * Allocates, initializes and returns a new SingleGraph.
 */
SingleGraph new_SingleGraph(int size) {
return new_SingleGraph(size,INT_TAGS);
}


/**
 * Allocates, initializes and returns a new SingleGraph.
 */
SingleGraph new_SingleGraph(TagType type) {
return new_SingleGraph(DEFAULT_STATE_ARRAY_SIZE,type);
}


/**
 * Allocates, initializes and returns a new SingleGraph.
 */
SingleGraph new_SingleGraph() {
return new_SingleGraph(DEFAULT_STATE_ARRAY_SIZE);
}


/**
 * Frees a SingleGraph
 */
void free_SingleGraph(SingleGraph g,void(*free_tag)(void*)) {
if (g==NULL) return;
if (g->tag_type!=PTR_TAGS && free_tag!=NULL) {
   error("Unexpected free function in free_SingleGraph\n");
   free_tag=NULL;
}
if (g->states!=NULL) {
   for (int i=0;i<g->number_of_states;i++) {
      free_SingleGraphState(g->states[i],free_tag);
   }
   free(g->states);
}
free(g);
}


/**
 * Tests if a SingleGraph state is final.
 */
int is_final_state(SingleGraphState e) {
return is_bit_mask_set(e->control,FINAL_STATE_BIT_MASK);
}


/**
 * Marks a SingleGraph state as final.
 */
void set_final_state(SingleGraphState e) {
set_bit_mask(&(e->control),FINAL_STATE_BIT_MASK);
}


/**
 * Marks a SingleGraph state as non final.
 */
void unset_final_state(SingleGraphState e) {
unset_bit_mask(&(e->control),FINAL_STATE_BIT_MASK);
}


/**
 * Tests if a SingleGraph state is initial.
 */
int is_initial_state(SingleGraphState e) {
return is_bit_mask_set(e->control,INITIAL_STATE_BIT_MASK);
}


/**
 * Marks a SingleGraph state as initial.
 */
void set_initial_state(SingleGraphState e) {
set_bit_mask(&(e->control),INITIAL_STATE_BIT_MASK);
}


/**
 * Marks a SingleGraph state as non initial.
 */
void unset_initial_state(SingleGraphState e) {
unset_bit_mask(&(e->control),INITIAL_STATE_BIT_MASK);
}


/**
 * Tests if a SingleGraph state is accessible.
 */
int is_accessible_state(SingleGraphState e) {
return is_bit_mask_set(e->control,ACCESSIBILITY_BIT_MASK);
}


/**
 * Marks a SingleGraph state as accessible.
 */
void set_accessible_state(SingleGraphState e) {
set_bit_mask(&(e->control),ACCESSIBILITY_BIT_MASK);
}


/**
 * Tests if a SingleGraph state is co-accessible.
 */
int is_co_accessible_state(SingleGraphState e) {
return is_bit_mask_set(e->control,CO_ACCESSIBILITY_BIT_MASK);
}


/**
 * Marks a SingleGraph state as co-accessible.
 */
void set_co_accessible_state(SingleGraphState e) {
set_bit_mask(&(e->control),CO_ACCESSIBILITY_BIT_MASK);
}


/**
 * Returns 1 if the state is not both accessible and
 * co-accessible; 0 otherwise.
 */
int is_useless_state(SingleGraphState e) {
return !(is_accessible_state(e) && is_co_accessible_state(e));
}


/**
 *  Allocates, initializes and returns a new SingleGraph state.
 */
SingleGraphState new_SingleGraphState() {
SingleGraphState s;
s=(SingleGraphState)malloc(sizeof(struct single_graph_state_));
if (s==NULL) {
   fatal_error("Not enough memory in new_SingleGraphState\n");
}
s->control=0;
s->outgoing_transitions=NULL;
s->reverted_incoming_transitions=NULL;
s->default_state=-1;
return s;
}


/**
 * Frees a SingleGraph state and all its transitions. The states
 * pointed by its transitions are not modified.
 */
void free_SingleGraphState(SingleGraphState s,void (*free_tag)(void*)) {
if (s==NULL) return;
free_Transition_list(s->outgoing_transitions,free_tag);
/* We don't use free_tag on the reverted transitions, because there
 * would be double free problems */
free_Transition_list(s->reverted_incoming_transitions);
free(s);
}


/**
 * Creates and adds an outgoing transition to the given state. No test
 * is performs to check whether the transition already exists.
 * Note that it is the responsability to the caller to deal with
 * the corresponding reverted incoming transition, if needed.
 */
void add_outgoing_transition(SingleGraphState state,int tag_number,int state_number) {
state->outgoing_transitions=new_Transition(tag_number,state_number,state->outgoing_transitions);
}


/**
 * Creates and adds an outgoing transition to the given state. No test
 * is performs to check whether the transition already exists.
 * Note that it is the responsability to the caller to deal with
 * the corresponding reverted incoming transition, if needed.
 */
void add_outgoing_transition(SingleGraphState state,void* label,int state_number) {
state->outgoing_transitions=new_Transition(label,state_number,state->outgoing_transitions);
}


/**
 * Creates and adds an incoming transition into the given state. No test
 * is performs to check whether the transition already exists.
 * Note that it is the responsability to the caller to deal with
 * the corresponding reverted incoming transition, if needed.
 */
void add_incoming_transition(SingleGraphState state,int tag_number,int state_number) {
state->reverted_incoming_transitions=new_Transition(tag_number,state_number,state->reverted_incoming_transitions);
}


/**
 * Resize the state array of a SingleGraph.
 */
void set_state_array_capacity(SingleGraph g,int new_capacity) {
if (g->states==NULL && new_capacity==0) return;
g->states=(SingleGraphState*)realloc(g->states,new_capacity*sizeof(SingleGraphState));
if (g->states==NULL) {
  fatal_error("Not enough memory in set_state_array_capacity\n");
}
g->capacity=new_capacity;
}


/**
 * Resizes g's state array to its exact number of states.
 */
void resize(SingleGraph g) {
set_state_array_capacity(g,g->number_of_states);
}


/**
 * Adds a new state to a SingleGraph. The function returns 
 * the new state.
 */
SingleGraphState add_state(SingleGraph g) {
SingleGraphState s=new_SingleGraphState();
if (g->capacity==g->number_of_states) {
   /* If necessary, we resize the state array, doubling its capacity */
   if (g->capacity==0) {
      g->capacity=1;
   }
   set_state_array_capacity(g,2*(g->capacity));
}
g->states[g->number_of_states]=s;
(g->number_of_states)++;
return s;
}


/**
 * Moves a SingleGraph src to the SingleGraph dest.
 *
 * dest is freed before copying content of src to it,
 * src is freed and nulled after.
 */
void move_SingleGraph(SingleGraph dest,SingleGraph *src,void (*free_tag)(void*)) {
/* We free dest */
if (dest->tag_type!=PTR_TAGS && free_tag!=NULL) {
   error("Unexpected free function in move_SingleGraph\n");
   free_tag=NULL;
}
if (dest->states!=NULL) {
   for (int i=0;i<dest->number_of_states;i++) {
      free_SingleGraphState(dest->states[i],free_tag);
   }
   free(dest->states);
}
/* We copy src into dest */
dest->capacity=(*src)->capacity;
dest->number_of_states=(*src)->number_of_states;
dest->states=(*src)->states;
/* And we free *src */
free(*src);
*src=NULL;
}


/**
 * This function creates for each transition of the graph
 * the corresponding reverse one.
 */
void compute_reverse_transitions(SingleGraph graph) {
int n_states=graph->number_of_states;
for (int i=0;i<n_states;i++) {
   if (graph->states[i]!=NULL) {
      Transition* t=graph->states[i]->outgoing_transitions;
      while (t!=NULL) {
         Transition* tmp=clone_transition(t,NULL);
         tmp->state_number=i;
         tmp->next=graph->states[t->state_number]->reverted_incoming_transitions;
         graph->states[t->state_number]->reverted_incoming_transitions=tmp;
         t=t->next;
      }
      if (graph->states[i]->default_state!=-1) {
         /* If there is a default transition from i to q, we
          * must add a reverse transition from q to i. As we
          * we only need the information that i and q and linked, we
          * can arbitrarily tag this transition with 0. */
         add_incoming_transition(graph->states[graph->states[i]->default_state],0,i);
      }
   }
}
}


/**
 * Marks the current state as being accessible. All the states reachable
 * from this one will recursively be marked as being accessible.
 */
void check_accessibility(SingleGraphState* states,int state_number) {
if (states[state_number]==NULL) {
   fatal_error("Internal NULL error in check_accessibility\n");
}
if (is_accessible_state(states[state_number])) {
   /* There is nothing to do if the state has already been processed */
   return;
}
/* Otherwise, we mark it */
set_accessible_state(states[state_number]);
Transition* t=states[state_number]->outgoing_transitions;
/* And we explore the states that are reachable from the current one */
while (t!=NULL) {
   check_accessibility(states,t->state_number);
   t=t->next;
}
int q;
if ((q=states[state_number]->default_state)!=-1) {
   /* If there is a default transition, we explore it */
   check_accessibility(states,q);
}
}


/**
 * Marks the current state as being co-accessible. All the states that have a
 * transition pointing on this one will recursively be marked as being
 * co-accessible.
 */
void check_co_accessibility(SingleGraphState* states,int state_number) {
if (states[state_number]==NULL) {
   fatal_error("Internal NULL error in check_co_accessibility\n");
}
if (is_co_accessible_state(states[state_number])) {
   /* There is nothing to do if the state has already been processed */
   return;
}
/* Otherwise, we mark it */
set_co_accessible_state(states[state_number]);
/* And we explore the states that point to the current one */
Transition* t=states[state_number]->reverted_incoming_transitions;
while (t!=NULL) {
   check_co_accessibility(states,t->state_number);
   t=t->next;
}
}


/**
 * This function adds the given closure to the one of the given state.
 */
void add_closure(struct list_int** closures,int state_number,struct list_int* closure_to_add) {
if (closure_to_add==NULL) {
   /* If there is nothing to add, we return */
   return;
}
while (closure_to_add!=NULL) {
   closures[state_number]=sorted_insert(closure_to_add->n,closures[state_number]);
   closure_to_add=closure_to_add->next;
}
}


/**
 * This function computes the epsilon closure of the given state. It updates the
 * 'closures' array and it returns the epsilon closure of the state.
 */
struct list_int* get_epsilon_closure(struct list_int** closures,SingleGraphState* states,
                                     int state_number,struct bit_array* mark) {
if (!get_value(mark,state_number)) {
   /* If the state has not already been processed, we start by marking it */
   set_value(mark,state_number,1);
   /* A state is in its own closure */
   closures[state_number]=sorted_insert(state_number,closures[state_number]);
   Transition* t=states[state_number]->outgoing_transitions;
   while (t!= NULL) {
      if ((t->tag_number==0) && (t->state_number!=state_number)) {
         /* If the transition is tagged by epsilon and if it doesn't
          * point on the current state, then we can add the epsilon
          * closure of its destination state to the one of the current
          * state. */
         add_closure(closures,state_number,get_epsilon_closure(closures,states,t->state_number,mark));
      }
      t=t->next;
   }
}
return closures[state_number];
}


/**
 * This function builds and returns an array of integer lists that represent
 * for each state the states that can be reached via epsilon transitions.
 */
struct list_int** get_epsilon_closures(SingleGraph graph) {
struct list_int** closures=(struct list_int**)malloc(graph->number_of_states*sizeof(struct list_int*));
if (closures==NULL) {
   fatal_error("Not enough memory in get_epsilon_closures\n");
}
/* We use a bit array to mark states. We don't use a bit mask with the
 * control byte of the state, since having been processed in a function
 * is not a real property for a state, like being final or accessible. */
struct bit_array* mark=new_bit_array(graph->number_of_states,ONE_BIT);
int i;
/* First, we initialize the closures. Note that 'mark' was already initialized
 * when it was created */
for (i=0;i<graph->number_of_states;i++) {
   closures[i]=NULL;    
}
/* Then, for each state, we compute the epsilon closure */
for (i=0;i<graph->number_of_states;i++) {
   /* If the state was not already processed, we compute its epsilon closure */
   if (!get_value(mark,i)) {
      get_epsilon_closure(closures,graph->states,i,mark);
   }
}
/* Finally we free the bit array */
free_bit_array(mark);
return closures;
}


/**
 * This function removes all the epsilon transitions from the given graph.
 * Note that it is the responsability of the caller to have computed the
 * epsilon closures before, in order to add later non-epsilon transitions
 * to preserve the graph language.
 */
void delete_epsilon_transitions(SingleGraph graph) {
Transition* ptr;
Transition* non_epsilon_transitions;
Transition* temp;
for (int i=0;i<graph->number_of_states;i++) {
   /* For each state, we look all the outgoing transitions */
   ptr=graph->states[i]->outgoing_transitions;
   non_epsilon_transitions=NULL;
   while (ptr!=NULL) {
      /* For each transition */
      temp=ptr;
      ptr=ptr->next;
      if (temp->tag_number==0) {
         /* If we have an epsilon transition, we free it */
         free(temp); 
      }
      else {
         /* Otherwise, we add the transition to the list of good ones */
         temp->next=non_epsilon_transitions;
         non_epsilon_transitions=temp;
      }
   }
   graph->states[i]->outgoing_transitions=non_epsilon_transitions;
   /* Then, we look all the incoming transitions in the same way */
   ptr=graph->states[i]->reverted_incoming_transitions;
   non_epsilon_transitions = NULL;
   while (ptr!=NULL) {
      /* For each transition, we do the same than above */
      temp=ptr;
      ptr=ptr->next;
      if (temp->tag_number==0) {
         free(temp); 
      }
      else {
         temp->next=non_epsilon_transitions;
         non_epsilon_transitions=temp;
      }
   }
   graph->states[i]->reverted_incoming_transitions=non_epsilon_transitions;
}
}


/**
 * This function adds to the given graph the transitions required by the given
 * epsilon closure in order to preserve the initial graph language.
 */
void add_transitions_according_to_epsilon_closure(struct list_int** closures,SingleGraph graph) {
for (int i=0;i<graph->number_of_states;i++) {
   /* For each state */
   struct list_int* closure=closures[i];
   while (closure!=NULL) {
      /* For each state that can be reached from the current one... */
      if (closure->n!=i) {
         /* ...that is not the current one */
         SingleGraphState e=graph->states[closure->n];
         if (is_final_state(e)) {
            /* We set the current one final if we can reached a final state
             * from it via epsilon transitions */
            set_final_state(graph->states[i]);
         }
         /* And we add to the current state all the transitions that outgo
          * from the reachable one */
         Transition* tr=e->outgoing_transitions;
         while (tr!=NULL) {
            add_outgoing_transition(graph->states[i],tr->tag_number,tr->state_number);
            add_incoming_transition(graph->states[tr->state_number],tr->tag_number,i);
            tr=tr->next;
         }
      }
      closure=closure->next;
   }
}
}


/**
 * Removes epsilon transitions from the given graph, adding the transitions
 * that are necessary not to modify the language recognized by the graph. 
 */
void remove_epsilon_transitions(SingleGraph graph) {
/* We compute the epsilon closure for each state */
struct list_int** closures=get_epsilon_closures(graph);
/* Then we remove all the epsilon transitions */
delete_epsilon_transitions(graph);
/* And we add transitions in order to preserve the graph language. For instance,
 * if we have the following transitions:
 * 
 * A --epsilon--> B
 * B --XXX--> C
 * 
 * we have to remove the first one and then to add the following new one:
 * 
 * A --XXX--> C
 */
add_transitions_according_to_epsilon_closure(closures,graph);
/* Finally, we free the closures */
for (int i=0;i<graph->number_of_states;i++) {
   free_list_int(closures[i]);
}
free(closures);
}


/**
 * Takes a list of transitions and removes those that point to useless states.
 */
Transition* remove_transitions_to_useless_states(Transition* transitions,
                                                    SingleGraphState* states) {
Transition* tmp;
Transition* tmp2;
Transition* tmp_old;
tmp=transitions;
while (tmp!=NULL) {
   if ((((states[tmp->state_number]->control)&4)==0)||(((states[tmp->state_number]->control)&8)==0)) {
      tmp2=tmp->next;
      if (tmp==transitions) {
         transitions=tmp2;
      } else {
         tmp_old->next=tmp2;
      }
      free(tmp);
      tmp=tmp2;
   } else {
      tmp_old=tmp;
      tmp=tmp->next;
   }
}
return transitions;
}


/**
 * This function renumbers all the transitions that point to 'old_state_number',
 * making them point to 'new_state_number'. Both outgoing and incoming
 * transitions are updated, in all states that are concerned.
 */
void renumber_transitions(SingleGraphState* states,int old_state_number,int new_state_number) {
/* First, we deal with the transitions that go out from 'old_state_number' */
Transition* t=states[old_state_number]->outgoing_transitions;
while (t!=NULL) {
   if (t->state_number!=old_state_number) {
      /* If we have a transition of the form:
       * 
       * old_state_number ----> X
       * 
       * then we must renumber the incoming transitions of X. */
      renumber_transitions(states[t->state_number]->reverted_incoming_transitions,
                           old_state_number,new_state_number);
   }
   else {
      /* If we have a transition like:
       * 
       * old_state_number ----> old_state_number
       * 
       * then we must replace change its destination to new_state_number. */
      t->state_number=new_state_number;
   }
   t=t->next;
}
/* Then, we do exactly the same with incoming transitions */
t=states[old_state_number]->reverted_incoming_transitions;
while (t!=NULL) {
   if (t->state_number!=old_state_number) {
      /* If we have a transition of the form:
       * 
       * old_state_number <---- X
       * 
       * then we must renumber the outging transitions of X. */
      renumber_transitions(states[t->state_number]->outgoing_transitions,
                           old_state_number,new_state_number);
   }
   else {
      /* If we have a transition like:
       * 
       * old_state_number <---- old_state_number
       * 
       * then we must replace change its destination to new_state_number. */
      t->state_number=new_state_number;
   }
   t=t->next;
}
}


/**
 * This function removes useless states, that is to say states that are
 * not accessible and/or not co-accessible. Note that states must have
 * been previously marked with 'check_accessibility' and 'check_co_accessibility'.
 * Of course, all transitions related to useless states are removed.
 */
void remove_useless_states(SingleGraph graph) {
int i;
for (i=0;i<graph->number_of_states;i++) {
   /* For each state, we remove transitions that go to/come from a state
    * to be removed */
   graph->states[i]->outgoing_transitions=remove_transitions_to_useless_states(graph->states[i]->outgoing_transitions,graph->states);
   graph->states[i]->reverted_incoming_transitions=remove_transitions_to_useless_states(graph->states[i]->reverted_incoming_transitions,graph->states);
}
int last_state=graph->number_of_states-1;
i=0;
do {
   /* We look for the first non NULL state, starting from the end of the
    * state array */
   while ((last_state>=0) && (graph->states[last_state]==NULL)) {
      last_state--;
   }
   /* We free all the states that we can */
   while ((last_state>=0) && is_useless_state(graph->states[last_state])) {
      free_SingleGraphState(graph->states[last_state]);
      graph->states[last_state]=NULL;
      last_state--;
   }
   if (last_state==-1) {
      /* If we have removed all the states, we return */
      graph->number_of_states=0;
      return;
   }
   /* Then, we look for the first state to remove from the beginning
    * of the state array */
   while ((i<last_state) && !is_useless_state(graph->states[i])) {
      i++;
   }
   if (i==last_state) {
      /* If there is no more state to remove, we have finished */
      graph->number_of_states=last_state+1;
      return;
   }
   /* Otherwise, the state number 'i' is to be removed. In that case, we
    * swap it with the state number 'last_state' which must not be removed.
    * To do that correctly, we must update the transitions that point
    * to 'last_state' and make them point to 'i'. */
   renumber_transitions(graph->states,last_state,i);
   SingleGraphState tmp=graph->states[i];
   graph->states[i]=graph->states[last_state];
   graph->states[last_state]=tmp;
   free_SingleGraphState(graph->states[last_state]);
   graph->states[last_state]=NULL;
   last_state--;
} while (i<last_state);
graph->number_of_states=last_state+1;
}


/**
 * This function removes useless states, that is to say states that are
 * not accessible and/or not co-accessible. All the related transitions are
 * also removed.
 * No epsilon removal is done.
 */
void trim(SingleGraph graph) {
compute_reverse_transitions(graph);
for (int h=0;h<graph->number_of_states;h++) {
   if (is_final_state(graph->states[h])) {
      /* We start the co_accessibility check from every final state */
      check_co_accessibility(graph->states,h);
   }
}
for (int h=0;h<graph->number_of_states;h++) {
   if (is_initial_state(graph->states[h])) {
      /* We start the accessibility check from every initial state */
      check_accessibility(graph->states,h);
   }
}
remove_useless_states(graph);
}


/**
 * This function reverses a graph, that is to say:
 * 1) it reverses all its transitions
 * 2) each initial state becomes final and each final state becomes initial
 * 
 * Note that we assume that reversed transitions have been computed before.
 * 
 * The function raises a fatal error in case of NULL or empty automaton.
 */
void reverse(SingleGraph graph) {
if (graph==NULL) {
   fatal_error("NULL error in reverse\n");
}
if (graph->number_of_states==0) {
   /* If the automaton is empty */
   fatal_error("Trying to reverse an empty automaton\n");
}
SingleGraphState s;
for (int i=0;i<graph->number_of_states;i++) {
   s=graph->states[i];
   if (s==NULL) {
      fatal_error("Internal error in reverse\n");
   }
   /* As we have reversed transitions, we just have to swap
    * incoming and outgoing transitions */
   Transition* t=s->outgoing_transitions;
   s->outgoing_transitions=s->reverted_incoming_transitions;
   s->reverted_incoming_transitions=t;
   /* Then we modify the initiality and the finality of the state */
   if (is_final_state(s) && is_initial_state(s)) {
      /* No need to do nothing if the state has both properties */
      continue;
   }
   if (is_final_state(s)) {
      /* A final state becomes initial and non final */
      unset_bit_mask(&(s->control),FINAL_STATE_BIT_MASK);
      set_initial_state(s);
   }
   else if (is_initial_state(s)) {
      /* An initial state becomes final and non initial */
      unset_bit_mask(&(s->control),INITIAL_STATE_BIT_MASK);
      set_final_state(s);
   }
}
}


/**
 * Returns a copy of the given automaton.
 */
SingleGraph clone(SingleGraph src,void*(*clone_tag_label)(void*)) {
if (src==NULL) return NULL;
SingleGraph dest=new_SingleGraph(src->number_of_states,src->tag_type);
SingleGraphState src_state;
SingleGraphState dest_state;
for (int i=0;i<src->number_of_states;i++) {
   src_state=src->states[i];
   dest_state=add_state(dest);
   dest_state->default_state=src_state->default_state;
   if (is_initial_state(src_state)) set_initial_state(dest_state);
   if (is_final_state(src_state)) set_final_state(dest_state);
   dest_state->outgoing_transitions=clone_transition_list(src_state->outgoing_transitions,NULL,clone_tag_label);
}
return dest;
}


/**
 * Takes a graph and returns a sorted list that contains the number
 * of the initial states of the graph; NULL if none is found.
 */
struct list_int* get_initial_states(SingleGraph graph) {
struct list_int* initial_states=NULL;
for (int i=0;i<graph->number_of_states;i++) {
   if (is_initial_state(graph->states[i])) {
      initial_states=sorted_insert(i,initial_states);
   }
}
return initial_states;
}


/**
 * Returns the number of the single initial state of the given graph, or
 * -1 if there is none
 * -2 if there are more than one
 */
int get_initial_state(SingleGraph A) {
struct list_int* l=get_initial_states(A);
if (l==NULL) return -1;
int n=(l->next!=NULL)?-2:l->n;
free_list_int(l);
return n;
}


/**
 * This function looks for the given state set in the given hash table. If it
 * find it, the corresponding state number is returned. Otherwise, we add a state
 * to the given graph and we associate its number to the state set. This number
 * is returned. (*created) will be set to 1 if we need to create the state; to 0
 * otherwise.
 */
int get_state_number(SingleGraph graph,struct hash_table* hash,struct list_int* state_set,
                     int *created) {
int code;
struct any* value=get_value(hash,(void*)state_set,HT_INSERT_IF_NEEDED,&code);
if (code==HT_KEY_ALREADY_THERE) {
   (*created)=0;
   return value->_int;
}
(*created)=1;
value->_int=graph->number_of_states;
add_state(graph);
return value->_int;
}


/**
 * This function looks for the given state set in the given hash table. If it
 * finds it, the corresponding state is returned. Otherwise, we add a state
 * to the given graph and we associate its number to the state set.
 * (*created) will contain a non null value if and only if the state did
 * not already exist.
 */
SingleGraphState get_state(SingleGraph graph,struct hash_table* hash,struct list_int* state_set,
                           int *created) {
return graph->states[get_state_number(graph,hash,state_set,created)];
}


/**
 * This function looks for the given state set in the given hash table. If it
 * finds it, the corresponding state is returned. Otherwise, we add a state
 * to the given graph and we associate its number to the state set.
 */
SingleGraphState get_state(SingleGraph graph,struct hash_table* hash,struct list_int* state_set) {
int i;
return get_state(graph,hash,state_set,&i);
}


/**
 * This function returns 1 if a state of the set is final; 0 otherwise.
 */
int is_final_state_set(SingleGraph graph,struct list_int* state_set) {
while (state_set!=NULL) {
   if (state_set->n<0 || state_set->n>=graph->number_of_states) {
      fatal_error("Invalid state number %d in is_final_state_set (should be in [0;%d])\n",state_set->n,graph->number_of_states-1);
   }
   if (is_final_state(graph->states[state_set->n])) {
      return 1;
   }
   state_set=state_set->next;
}
return 0;
}


/**
 * This function looks in 'hash' for the linked list associated to the
 * transition's tag number. Then, it adds the transition's destination
 * state number to this list, if not already present.
 */
void process_transition(Transition* t,struct hash_table* hash) {
int code;
struct any* value=get_value(hash,t->tag_number,HT_INSERT_IF_NEEDED,&code);
if (code==HT_KEY_ADDED) {
   /* If there was no entry for the given tag number, then we must
    * create its empty list */
    value->_ptr=NULL;
}
/* Then, we insert the state number */
value->_ptr=sorted_insert(t->state_number,(struct list_int*)value->_ptr);
}


/**
 * This function takes a graph and determinizes it. We assume that the graph
 * does not contain any epsilon transition. However, we make no hypothesis about
 * its initial state(s).
 * 
 * Note that, after the determinization, the new reversed transitions have not been
 * computed.
 * 
 * The function raises a fatal error in case of NULL or empty automaton.
 */
void determinize(SingleGraph graph) {
if (graph==NULL) {
   fatal_error("NULL error in determinize\n");
}
if (graph->number_of_states==0) {
   /* If the automaton is empty */
   fatal_error("Trying to determinize an empty automaton\n");
}
SingleGraph new_graph=new_SingleGraph();
SingleGraphState state;
Transition* transition;
/* NOTE about structures to be used:
 * 
 * - hash: a hash table used to associate numbers to set of states
 * - fifo: a FIFO used to deal with the state sets to be processed
 * - transition_hash: a hash table reinitialized for each state set. We use
 *      it to associate to a given tag number the list of the states that can
 *      be reached.
 */
struct hash_table* hash=new_hash_table((unsigned int (*)(void*))hash_list_int,(int (*)(void*, void*))equal_list_int,(void (*)(void*))free_list_int);
struct fifo* fifo=new_fifo();
struct hash_table* transition_hash=new_hash_table();
/* We start by creating the initial state of the new graph and inserting it
 * in the hash table as well as in the FIFO */
struct list_int* current_state_set=get_initial_states(graph);
if (current_state_set==NULL) {
   /* If the automaton has no initial state */
   fatal_error("Trying to determinize an automaton with no initial state\n");
}
/* The initial state has the number 0 */
state=get_state(new_graph,hash,current_state_set);
set_initial_state(state);
put_ptr(fifo,current_state_set);
/* And now, we loop until we have processed all the state sets */
while (!is_empty(fifo)) {
   current_state_set=(struct list_int*)take_ptr(fifo);
   /* We get the state in the new graph that correspond to
    * the current state set */
   int created;
   state=get_state(new_graph,hash,current_state_set,&created);
   if (created) {
      /* We assume that a state set in the FIFO actually corresponds to
       * a state in the new graph */
      fatal_error("Internal error in determinize's FIFO\n");
   }
   if (is_final_state_set(graph,current_state_set)) {
      /* If at least one of the states of the set if final,
       * the new state must be final */
      set_final_state(state);
   }
   /* We process all the outgoing transitions of all the states of the set */
   struct list_int* tmp=current_state_set;
   while (tmp!=NULL) {
      transition=graph->states[tmp->n]->outgoing_transitions;
      while (transition!=NULL) {
         process_transition(transition,transition_hash);
         transition=transition->next;
      }
      tmp=tmp->next;
   }
   /* Now, we look for all the pairs (tag number,destination state numbers).
    * We use a counter 'p' in order to avoid looking in all the table if
    * have already seen all its elements. */
   int elements_to_process=transition_hash->number_of_elements;
   for (unsigned int i=0;i<transition_hash->capacity && elements_to_process!=0;i++) {
      struct hash_list* list=transition_hash->table[i];
      while (list!=NULL) {
         elements_to_process--;
         /* We get the number of the state that corresponds to the
          * state set */
         int created;
         int dest_state=get_state_number(new_graph,hash,(struct list_int*)(list->value._ptr),&created);
         if (created) {
            /* If the destination state has not already been processed, we add it to
             * the FIFO */
            put_ptr(fifo,list->value._ptr);
         } else {
            /* If the state set has already an equivalent in the hash table,
             * then we can free this useless one. */
            free_list_int((struct list_int*)list->value._ptr);
         }
         /* And we add a transition to our current state */
         state->outgoing_transitions=new_Transition(list->int_key,dest_state,state->outgoing_transitions);
         /* Finally, we can remove the current element from the transition hash list.
          * IMPORTANT: we must just free the hash_list cell and not its '_ptr' field because:
          *            1) the corresponding state set was used to add a new state in the hash
          *               table, so freeing it would destroy some content of the hash table
          *         or 2) it was already freed
          */
         struct hash_list* tmp=list;
         list=list->next;
         free(tmp);
      }
      transition_hash->table[i]=NULL;
   }
   /* And we don't forget to clean the transition hash table for the next step */
   clear_hash_table(transition_hash);
}
free_hash_table(hash);
free_hash_table(transition_hash);
free_fifo(fifo);
/* Finally, we replace the old graph by the new one */
move_SingleGraph(graph,&new_graph);
}


/**
 * Minimize the given graph, using Brzozowski's algorithm: 
 * reverse, determinize, reverse, determinize again.
 * If 'compute_reversed_transitions' parameter is non null,
 * then the function will first compute the reversed transitions
 * of the graph; otherwise, it will assume that this is already
 * done.
 * 
 * Note that, after the minimization, the new reversed transitions
 * have not been computed.
 * 
 * The function raises a fatal error in case of NULL or empty automaton,
 * or if the automaton is emptied during the minimization, for example if
 * it has no final state.
 */
void minimize(SingleGraph graph,int compute_reversed_transitions) {
if (graph==NULL) {
   fatal_error("NULL error in minimize\n");
}
if (graph->number_of_states==0) {
   /* If the automaton is empty */
   fatal_error("Trying to minimize an empty automaton\n");
}
if (compute_reversed_transitions) {
   compute_reverse_transitions(graph);
}
reverse(graph);
determinize(graph);
compute_reverse_transitions(graph);
reverse(graph);
determinize(graph);
}


/**
 * Saves the given graph to a .fst2 file. 'graph_number' is supposed to be
 * a negative integer representing the graph. 'graph_name' is supposed to
 * be the graph name without path and extension, like "Det". In some particular
 * case, this can be replaced by a string value. For instance, the graph name is
 * replaced by the content of a sentence, when a .fst2 represents a text automaton.
 */
void save_fst2_subgraph(FILE* fst2,SingleGraph graph,int graph_number,unichar* graph_name) {
/* We print the graph header made of its negative number and its name */
u_fprintf(fst2,"%d %S\n", graph_number, graph_name);
/* If we have an empty graph, we represent it by a single state with
 * no transition. This is a pure convention in order to avoid troubles
 * when loading .fst2 files. */
if (graph->number_of_states==0) {
   u_fprintf(fst2, ": \n");
}
/* We print all the states */
for (int i=0;i<graph->number_of_states;i++) {
   if (graph->states[i]==NULL) {
      fatal_error("NULL graph state in save_graph\n");
   }
   if (is_final_state(graph->states[i])) {
      u_fputc((unichar)'t',fst2);
   } else {
      u_fputc((unichar)':',fst2);
   }
   Transition* ptr=graph->states[i]->outgoing_transitions;
   while (ptr!=NULL) {
      u_fprintf(fst2," %d %d",ptr->tag_number,ptr->state_number);
      ptr=ptr->next;
   }
   u_fputc((unichar)' ',fst2);
   u_fputc((unichar)'\n',fst2);
}
/* Finally, we mark the end of the graph */
u_fprintf(fst2,"f \n");
}


/**
 * This function does a topological sort on the given automaton.
 * After the sort, we are sure that for each transition from
 * a state #x to a state #y, we have x<y. Note that the given automaton
 * must be an acyclic automaton.
 */
void topological_sort(SingleGraph graph) {
/* First, we compute for each state its number of incoming transitions */
int* incoming=(int*)malloc(graph->number_of_states*sizeof(int));
if (incoming==NULL) {
   fatal_error("Not enough memory in topological_sort\n");
}
int i;
for (i=0;i<graph->number_of_states;i++) {
   incoming[i]=0;
}
for (i=0;i<graph->number_of_states;i++) {
   for (Transition* t=graph->states[i]->outgoing_transitions;t!=NULL;t=t->next) {
      incoming[t->state_number]++;
   }
   if (graph->states[i]->default_state!=-1) {
      incoming[graph->states[i]->default_state]++;
   }
}
/* Then we allocate an array used to know that the state #x will
 * be renumber into the state #y */
int* renumber=(int*)malloc(graph->number_of_states*sizeof(int));
if (renumber==NULL) {
   fatal_error("Not enough memory in topological_sort\n");
}
int q;
for (q=0;q<graph->number_of_states;q++) {
   int old=0;
   /* At each step #q of the algorithm, we look for the first state with no
    * incoming transition, and we renumber as the new state #q */
   while (incoming[old]!=0) {
      old++;
   }
   renumber[old]=q;
   incoming[old]=-1;
   /* Then we decrease the number of incoming transitions of the
    * states that can directly be reached from the current one */
   for (Transition* t=graph->states[old]->outgoing_transitions;t!=NULL;t=t->next) {
      incoming[t->state_number]--;
   }
}
/* Finally, we create an array with the new states, renumbering
 * their transitions' destination state numbers */
SingleGraphState* new_states=(SingleGraphState*)malloc(graph->number_of_states*sizeof(SingleGraphState));
if (new_states==NULL) {
   fatal_error("Not enough memory in topological_sort\n");
}
for (q=0;q<graph->number_of_states;q++) {
   new_states[q]=new_SingleGraphState();
}
for (q=0;q<graph->number_of_states;q++) {
   new_states[renumber[q]]->outgoing_transitions=graph->states[q]->outgoing_transitions;
   /* We set to NULL in order to have a clean free later */
   graph->states[q]->outgoing_transitions=NULL;
   for (Transition* t=new_states[renumber[q]]->outgoing_transitions;t!=NULL;t=t->next) {
      t->state_number=renumber[t->state_number];
   }
   int default_state=graph->states[q]->default_state;
   new_states[renumber[q]]->default_state=(default_state==-1)?-1:renumber[default_state];
   new_states[renumber[q]]->control=graph->states[q]->control;
}
/* We free the previous state array */
for (q=0;q<graph->number_of_states;q++) {
   free_SingleGraphState(graph->states[q]);
}
free(graph->states);
graph->states=new_states;
}


/**
 * This function takes a .fst2 and return the automaton corresponding to its
 * subgraph #n. Note that the resulting automaton considers state numbers
 * from 0 to n_states-1 and not global state numbers as in a Fst2 structure.
 */
SingleGraph get_subgraph(Fst2* fst2,int n) {
if (n<=0 || n>fst2->number_of_graphs) {
   fatal_error("Invalid subgraph number %d in get_subgraph: should be in [1;%d]\n",n,fst2->number_of_graphs);
}
SingleGraph graph=new_SingleGraph(fst2->number_of_states_per_graphs[n]);
int initial_state=fst2->initial_states[n];
int max_states=fst2->number_of_states_per_graphs[n]+initial_state;
for (int i=initial_state;i<max_states;i++) {
   SingleGraphState state=add_state(graph);
   if (i==initial_state) {
      set_initial_state(state);
   }
   if (is_final_state(fst2->states[i])) {
      set_final_state(state);
   }
   Transition* t=fst2->states[i]->transitions;
   while (t!=NULL) {
      add_outgoing_transition(state,t->tag_number,t->state_number-initial_state);
      t=t->next;
   }
}
return graph;
}


/**
 * This function computes the number of paths between the states q1 and q2.
 * After its execution:
 * min_path_length[q1] will contain the length of the shortest path from q1 to q2
 * max_path_length[q1] will contain the length of the longest path from q1 to q2
 * number_of_paths[q1] will contain the number of paths to go from q1 to q2
 */
void count_paths(SingleGraph graph,int q1,int q2,int* min_path_length,int* max_path_length,
                 int* number_of_paths) {
if (q1==q2) {
   min_path_length[q1]=0;
   max_path_length[q1]=0;
   number_of_paths[q1]=1;
}
if (number_of_paths[q1]!=-1) {
   /* If we have already counted paths from the state q1, we don't
    * need to do it again */
   return;
}
number_of_paths[q1]=0;
min_path_length[q1]=INT_MAX;
max_path_length[q1]=0;
for (Transition* t=graph->states[q1]->outgoing_transitions;t!=NULL;t=t->next) {
   count_paths(graph,t->state_number,q2,min_path_length,max_path_length,number_of_paths);
   number_of_paths[q1]=number_of_paths[q1]+number_of_paths[t->state_number];
   /* We use +1 because we must count the transition from q1 to t->state_number */
   if ((min_path_length[t->state_number]+1)<min_path_length[q1]) {
      min_path_length[q1]=min_path_length[t->state_number]+1;
   }
   if ((max_path_length[t->state_number]+1)>max_path_length[q1]) {
      max_path_length[q1]=max_path_length[t->state_number]+1;
   }
}
}


/**
 * This function takes an acylic automaton that is supposed to represent
 * a sentence automaton and it returns an estimation of its
 * ambiguity rate, that represents the average number of hypothesis for
 * each word of the sentence. The lengths of the shortest and longest paths
 * are respectively stored into '*min' and '*max'.
 */
double evaluate_ambiguity(SingleGraph graph,int *min,int *max) {
/* If min and or max are NULL */
int dumbmax;
int dumbmin;
if (max==NULL) {
   max=&dumbmax;
}
if (min==NULL) {
   min=&dumbmin;
}
topological_sort(graph);
/* We create and initialize a matrix to know, for each couple of state
 * (x,y) if there is a direct transition from x to y. */
struct bit_array* direct[graph->number_of_states];
for (int i=0;i<graph->number_of_states;i++) {
   direct[i]=new_bit_array(graph->number_of_states,ONE_BIT);
}
int q;
for (q=0;q<graph->number_of_states;q++) {
   for (Transition* t=graph->states[q]->outgoing_transitions;t!=NULL;t=t->next) {
      set_value(direct[q],t->state_number,1);
   }
   if (graph->states[q]->default_state!=-1) {
      set_value(direct[q],graph->states[q]->default_state,1);
   }
}
/* Now, we look for factorizing states, i.e. states so that
 * every path of the automaton cross them */
char factorizing[graph->number_of_states];
for (q=0;q<graph->number_of_states;q++) {
   factorizing[q]=1;
}
for (int i=0;i<graph->number_of_states;i++) {
   for (int j=1;j<graph->number_of_states;j++) {
      if (get_value(direct[i],j)) {
         for (int k=i+1;k<j;k++) {
            /* We can do this only because we have performed a
             * topological sort before */
            factorizing[k]=0;
         }
      }
   }
}
/* We can free the matrix */
for (int i=0;i<graph->number_of_states;i++) {
   free_bit_array(direct[i]);
}
/* Now, we will count the number of paths between all the couple
 * of states q_i and q_i+1, where q_i is the factorizing state #i. 
 * We do that because the combinatory explosion makes impossible to
 * count directly the number of paths of the whole automaton. */
double ambiguity_rate=0;
*max=0;
*min=0;
int min_path_length[graph->number_of_states];
int max_path_length[graph->number_of_states];
int number_of_paths[graph->number_of_states];
for (int i=0;i<graph->number_of_states;i++) {
   min_path_length[i]=-1;
   max_path_length[i]=-1;
   number_of_paths[i]=-1;
}
int q1,q2;
/* By definition, the initial state #0 is a factorizing one */
q2=0;
while (q2<graph->number_of_states-1) {
   q1=q2;
   q2++;
   while (q2<graph->number_of_states-1 && !factorizing[q2]) {
      q2++;
   }
   number_of_paths[q1]=-1;
   count_paths(graph,q1,q2,min_path_length,max_path_length,number_of_paths);
   *max=(*max)+max_path_length[q1];
   *min=(*min)+min_path_length[q1];
   ambiguity_rate=ambiguity_rate+log((double)number_of_paths[q1]);
}
return ambiguity_rate;
}


/**
 * Replaces A by the union of the two given automata. B is freed.
 * Note that the result may not be deterministic.
 */
void build_union(SingleGraph A,SingleGraph B) {
set_state_array_capacity(A,A->number_of_states+B->number_of_states);
int shift=A->number_of_states;
int q;
for (q=0;q<B->number_of_states;q++) {
   SingleGraphState state=add_state(A);
   state->control=B->states[q]->control;
}
for (q=0;q<B->number_of_states;q++) {
   A->states[shift+q]->outgoing_transitions=shift_destination_states(B->states[q]->outgoing_transitions,shift);
   B->states[q]->outgoing_transitions=NULL;
   int def=B->states[q]->default_state;
   A->states[shift+q]->default_state=(def!=-1)?(shift+def):-1;
}
/* We free B */
free_SingleGraph(B,NULL);
}

 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "SingleGraph.h"
#include "Error.h"
#include "BitMasks.h"
#include "List_int.h"
#include "BitArray.h"

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
 */
SingleGraph new_SingleGraph() {
SingleGraph g =(SingleGraph)malloc(sizeof(struct single_graph_));
if (g==NULL) {
   fatal_error("Not enough memory in new_SingleGraph\n");
}
g->number_of_states=0;
g->capacity=0;
g->states=NULL;
set_state_array_capacity(g,DEFAULT_STATE_ARRAY_SIZE);
return g;
}


/**
 * Frees a SingleGraph
 */
void free_SingleGraph(SingleGraph g) {
if (g==NULL) return;
if (g->states!=NULL) {
   for (int i=0;i<g->number_of_states;i++) {
      free_SingleGraphState(g->states[i]);
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
return s;
}


/**
 * Frees a SingleGraph state and all its transitions. The states
 * pointed by its transitions are not modified.
 */
void free_SingleGraphState(SingleGraphState s) {
if (s==NULL) return;
free_Fst2Transition(s->outgoing_transitions);
free_Fst2Transition(s->reverted_incoming_transitions);
free(s);
}


/**
 * Adds a transition to a transition list. No test
 * is performs to check whether the transition allready exists. 
 * The function returns the new head of the list.
 */
Fst2Transition add_transition(Fst2Transition list,int tag_number,int state_number) {
Fst2Transition transition=new_Fst2Transition();
transition->tag_number=tag_number;
transition->state_number=state_number;
transition->next=list;
return transition;
}


/**
 * Creates and adds an outgoing transition to the given state. No test
 * is performs to check whether the transition allready exists.
 * Note that it is the responsability to the caller to deal with
 * the corresponding reverted incoming transition, if needed.
 */
void add_outgoing_transition(SingleGraphState state,int tag_number,int state_number) {
state->outgoing_transitions=add_transition(state->outgoing_transitions,
                                           tag_number,state_number);
}


/**
 * Creates and adds an incoming transition to the given state. No test
 * is performs to check whether the transition allready exists.
 */
void add_incoming_transition(SingleGraphState state,int tag_number,int state_number) {
state->reverted_incoming_transitions=add_transition(state->reverted_incoming_transitions,
                                                    tag_number,state_number);
}


/**
 * Resize the state array of a SingleGraph.
 */
void set_state_array_capacity(SingleGraph g,int new_capacity) {
g->states=(SingleGraphState*)realloc(g->states,new_capacity*sizeof(SingleGraphState));
if (g->states==NULL) {
  fatal_error("Not enough memory in set_state_array_capacity\n");
}
g->capacity=new_capacity;
}


/**
 * Adds a new state to a SingleGraph.
 */
SingleGraphState add_state(SingleGraph g) {
SingleGraphState s=new_SingleGraphState();
if (g->capacity==g->number_of_states) {
   /* If necessary, we resize the state array, doubling its capacity */
   set_state_array_capacity(g,2*(g->capacity));
}
g->states[g->number_of_states]=s;
(g->number_of_states)++;
return s;
}


/**
 * Move a SingleGraph src to the SingleGraph dest.
 *
 * dest is freed before copying content of src to it,
 * src is freed and nulled after.
 */
void move_SingleGraph(SingleGraph dest,SingleGraph *src) {
/* We free dest */
if (dest->states!=NULL) {
   for (int i=0;i<dest->number_of_states;i++) {
      free_SingleGraphState(dest->states[i]);
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
for (int i=0; i<n_states; i++) {
   if (graph->states[i]!=NULL) {
      Fst2Transition t=graph->states[i]->outgoing_transitions;
      while (t!=NULL) {
         add_incoming_transition(graph->states[t->state_number],t->tag_number,i);
         t=t->next;
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
   /* There is nothing to do if the state has allready been processed */
   return;
}
/* Otherwise, we mark it */
set_accessible_state(states[state_number]);
Fst2Transition t=states[state_number]->outgoing_transitions;
/* And we explore the states that are reachable from the current one */
while (t!=NULL) {
   check_accessibility(states,t->state_number);
   t=t->next;
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
   /* There is nothing to do if the state has allready been processed */
   return;
}
/* Otherwise, we mark it */
set_co_accessible_state(states[state_number]);
/* And we explore the states that point to the current one */
Fst2Transition t=states[state_number]->reverted_incoming_transitions;
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
   /* If the state has not allready been processed, we start by marking it */
   set_value(mark,state_number,1);
   /* A state is in its own closure */
   closures[state_number]=sorted_insert(state_number,closures[state_number]);
   Fst2Transition t=states[state_number]->outgoing_transitions;
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
/* First, we initialize the closures. Note that 'mark' was allready initialized
 * when it was created */
for (i=0;i<graph->number_of_states;i++) {
   closures[i]=NULL;    
}
/* Then, for each state, we compute the epsilon closure */
for (i=0;i<graph->number_of_states;i++) {
   /* If the state was not allready processed, we compute its epsilon closure */
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
Fst2Transition ptr,non_epsilon_transitions,temp;
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
         Fst2Transition tr=e->outgoing_transitions;
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

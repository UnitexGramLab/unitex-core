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

#include "GrfCheck.h"
#include "Error.h"
#include "BitMasks.h"
#include "Transitions.h"

int look_for_recursion(int,struct list_int*,Fst2*,int*);


#define NO_LEFT_RECURSION 1
#define LEFT_RECURSION 0

#define TMP_LOOP_MARK 8
#define VISITED_MARK 16

#define UNCONDITIONAL_E_MATCH 32
#define CONDITIONAL_E_MATCH 64
#define DOES_NOT_MATCH_E 128

#define E_IS_MATCHED 1
#define E_IS_NOT_MATCHED 2
#define DOES_NOT_KNOW_IF_E_IS_MATCHED 3


/**
 * To determine if a graph can match the empty word <E>, we must check if
 * there is an empty path from its initial state to a final state. However,
 * such a path can be made of calls to subgraphs that match <E>. So, we use
 * conditions to represents these dependencies. Conditions are associated to
 * each state in the form of a list of condition. Each list corresponds to a
 * path from this state to a final state. It contains the numbers of the subgraphs
 * that are in the path. For instance, if we have the following case:
 * 
 * state s --------.....-------> graph 45 -------......-------> graph 12 ----> FINAL STATE
 *            |
 *            |
 *            |----------.....--------> graph 27 --------.....------> FINAL STATE
 *
 * the state s will have two conditions:
 * - condition 1: {45, 12}
 * - condition 2: {27}
 */
struct condition_list {
  struct list_int* condition;
  struct condition_list* next;
};
typedef struct condition_list* ConditionList;


/**
 * Frees all the memory associated to the given condition list.
 */
void free_ConditionList(ConditionList l) {
ConditionList tmp;
while (l!=NULL) {
   free_list_int(l->condition);
   tmp=l;
   l=l->next;
   free(tmp);
}
}


/**
 * Returns a copy of the given condition list.
 */
ConditionList clone(ConditionList l) {
ConditionList tmp;
if (l==NULL) return NULL;
tmp=(ConditionList)malloc(sizeof(struct condition_list));
if (tmp==NULL) {
   fatal_error("Not enough memory in clone\n");
}
tmp->condition=clone(l->condition);
tmp->next=clone(l->next);
return tmp;
}


/**
 * Inserts the graph number 'n' in the given condition list.
 */
void insert_graph_in_conditions(int n,ConditionList* l) {
ConditionList tmp;
if (*l==NULL) {
   /* If the condition list is empty, we create one */
   tmp=(ConditionList)malloc(sizeof(struct condition_list));
   if (tmp==NULL) {
      fatal_error("Not enough memory in insert_graph_in_conditions\n");
   }
   tmp->next=NULL;
   tmp->condition=new_list_int(n);
   *l=tmp;
   return;
}
/* Otherwise, we insert the graph number in all the conditions of the list */
tmp=*l;
while (tmp!=NULL) {
   tmp->condition=sorted_insert(n,tmp->condition);
   tmp=tmp->next;
}
}


/**
 * Appends the condition list 'd' to the condition list 'c'.
 */
void merge_condition_lists(ConditionList *c,ConditionList d) {
ConditionList tmp;
if (*c==NULL) {
   *c=d;
   return;
}
if ((*c)->next==NULL) {
   (*c)->next=d;
   return;
}
tmp=*c;
while (tmp->next!=NULL) tmp=tmp->next;
tmp->next=d;
}



/**
 * Takes a fst2 tag and puts its control byte to 1 if it can
 * match the empty word <E>.
 * 
 * WARNING: <E> problem detection does not take contexts into account!
 */
void check_epsilon_tag(Fst2Tag e) {
if (!u_strcmp(e->input,"<E>")) e->control=1;
else if (e->input[0]=='$' && e->input[1]!='\0') {
   /* If we have a variable mark */
   e->control=1;
}
else e->control=0;
}


/**
 * Prints the graph call sequence that leads to the graph #n.
 */
void print_reversed_list(struct list_int* l,int n,unichar** graph_names) {
if (l->n==n) {
   error("ERROR: %S",graph_names[l->n]);
   return;
}
print_reversed_list(l->next,n,graph_names);
error(" calls %S that",graph_names[l->n]);
}


/**
 * Returns 1 if we can match <E> from the current state, with or without
 * conditions; 0 otherwise.
 */
int graph_matches_E(int initial_state,int current_state,Fst2State* states,Fst2Tag* tags,
                      int current_graph,unichar** graph_names,
                      ConditionList conditions_for_states[],
                      ConditionList *graph_conditions) {
Transition* l;
Fst2State s;
int ret_value=0;
int ret;
*graph_conditions=NULL;
s=states[current_state];
if (is_final_state(s)) {
   /* If we are arrived in a final state, then the graph matches <E> */
   set_bit_mask(&(s->control),UNCONDITIONAL_E_MATCH);
   return 1;
}
if (is_bit_mask_set(s->control,TMP_LOOP_MARK)) {
   /* If we have a loop, we do nothing, because they will be
    * dealt with later. */
   return 0;
}
if (is_bit_mask_set(s->control,VISITED_MARK)) {
   /* If we are in state that has already been visited */
   if (is_bit_mask_set(s->control,UNCONDITIONAL_E_MATCH)) {
      /* If this state can match <E> without conditions, then we have finished */
      return 1;
   }
   if (is_bit_mask_set(s->control,CONDITIONAL_E_MATCH)) {
      /* If this state can match <E> with conditions, then we have finished, but
       * we copy the necessary conditions in 'graph_conditions'. */
      *graph_conditions=clone(conditions_for_states[current_state-initial_state]);
      return 1;
   }
   /* If the state has been visited and if it does not match <E>, then we return OK */
   return 0;
}
set_bit_mask(&(s->control),VISITED_MARK);
set_bit_mask(&(s->control),TMP_LOOP_MARK);
l=s->transitions;
/* We look all the outgoing transitions */
while (l!=NULL) {
   if (l->tag_number<0) {
      /* If we have a subgraph, we test if it matches <E> */
      *graph_conditions=NULL;
      ret=graph_matches_E(initial_state,l->state_number,states,tags,current_graph,graph_names,conditions_for_states,graph_conditions);
      if (ret==1) {
         /* If the subgraph matches <E>, we say that the current state matches
          * <E>, modulo the conditions to be verified */
         set_bit_mask(&(s->control),CONDITIONAL_E_MATCH);
         /* We insert the new condition in first position... */
         insert_graph_in_conditions(-(l->tag_number),graph_conditions);
         /* ...and we merge the new conditions with the existing ones for this state */
         merge_condition_lists(&conditions_for_states[current_state-initial_state],*graph_conditions);
         *graph_conditions=NULL;
      }
      ret_value=ret_value|ret;
   }
   else if (tags[l->tag_number]->control&1) {
      /* If we have an <E> transition, we explore the rest of the graph from it */
      *graph_conditions=NULL;
      ret=graph_matches_E(initial_state,l->state_number,states,tags,current_graph,graph_names,conditions_for_states,graph_conditions);
      if (ret==1) {
         /* If we can match <E> from the <E>-transition's destination state, then
          * we can match it from the current state. */
         if (*graph_conditions==NULL) {
            /* If there is no condition */
            set_bit_mask(&(s->control),UNCONDITIONAL_E_MATCH);
         }
         else {
            /* Otherwise, we add the condition to the existing ones */
            set_bit_mask(&(s->control),CONDITIONAL_E_MATCH);
            merge_condition_lists(&conditions_for_states[current_state-initial_state],*graph_conditions);
            *graph_conditions=NULL;
         }
      }
      ret_value=ret_value|ret;
   }
   l=l->next;
}
unset_bit_mask(&(s->control),TMP_LOOP_MARK);
*graph_conditions=clone(conditions_for_states[current_state-initial_state]);
return ret_value;
}


/**
 * This function takes a condition, i.e. a list of graph numbers. Then, it
 * tests if all the corresponding graphs match <E>. In that case, it sets
 * '*matches_E' to E_IS_MATCHED.
 */
struct list_int* resolve_simple_condition(struct list_int* c,Fst2State* states,
                                    int* initial_states,int *modification,
                                    int *matches_E) {
struct list_int* tmp;
*matches_E=E_IS_MATCHED;
if (c==NULL) return NULL;
/* First, we try to solve the rest of the condition */
tmp=resolve_simple_condition(c->next,states,initial_states,modification,matches_E);
if ((*matches_E)==E_IS_NOT_MATCHED) {
   /* If at least one element of the rest of the condition does not
    * matches <E>, we can delete the current element (the rest has
    * already been freed). */
   free(c);
   (*modification)++;
   return NULL;
}
if ((*matches_E)==E_IS_MATCHED) {
   /* If all the elements of the rest of the condition match <E> */
   c->next=tmp;
   if (is_bit_mask_set(states[initial_states[c->n]]->control,UNCONDITIONAL_E_MATCH)) {
      /* If the current one also matches <E>, then we can return */
      return c;
   }
   if (is_bit_mask_set(states[initial_states[c->n]]->control,CONDITIONAL_E_MATCH) && 
       !is_bit_mask_set(states[initial_states[c->n]]->control,DOES_NOT_MATCH_E)) {
      /* If we don't know if the current element matches <E> or not */
      *matches_E=DOES_NOT_KNOW_IF_E_IS_MATCHED;
      return c;
   }
   /* If the current element does not match <E>, we can free the rest of the condition */
   free_list_int(c);
   *matches_E=E_IS_NOT_MATCHED;
   (*modification)++;
   return NULL;
}
/* If we don't know if the rest of the condition match <E> */
c->next=tmp;
if (is_bit_mask_set(states[initial_states[c->n]]->control,UNCONDITIONAL_E_MATCH)) {
   /* If the current element matches <E>, we still cannot decide */
   *matches_E=DOES_NOT_KNOW_IF_E_IS_MATCHED;
   return c;
}
if (is_bit_mask_set(states[initial_states[c->n]]->control,DOES_NOT_MATCH_E)) {
   /* If the current element does not matches <E>, the condition is not verified */
   free_list_int(c);
   *matches_E=E_IS_NOT_MATCHED;
   (*modification)++;
   return NULL;
}
/* If we still don't know, we do nothing */
*matches_E=DOES_NOT_KNOW_IF_E_IS_MATCHED;
return c;
}


/**
 * This function takes a condition list and checks if at least of the
 * conditions is verified, i.e. there is at least one path to match <E>.
 * It returns the modified condition list. If there is at least one modification,
 * then '*modification' is set to 1. See 'resolve_simple_condition' for
 * the values to be taken by '*matches_E'.
 */
ConditionList resolve_condition_list(ConditionList l,Fst2State* states,
                               int* initial_states,int *modification,int *matches_E) {
ConditionList tmp;
if (l==NULL) return NULL;
if (l->condition==NULL) {
   error("NULL internal error in resolve_condition_list\n");
   return NULL;
}
l->condition=resolve_simple_condition(l->condition,states,initial_states,modification,matches_E);
if (*matches_E==E_IS_MATCHED) {
   /* If condition is verified, then the graph matches <E>, so we return */
   return l;
}
if (l->condition==NULL) {
   /* If we have removed the whole condition, we delete it and we
    * clean the rest of the condition list. */
   tmp=l->next;
   free(l);
   return resolve_condition_list(tmp,states,initial_states,modification,matches_E);
}
/* Otherwise, we clean the rest of the condition list. */
l->next=resolve_condition_list(l->next,states,initial_states,modification,matches_E);
return l;
}


/**
 * This function tries to resolve the conditions of the graph #n.
 * It returns 1 if at least one condition was resolved.
 */
int resolve_conditions_for_one_graph(int n,ConditionList* conditions,
                                     Fst2State* states,int* initial_states,
                                     unichar** graph_names) {
int modification=0;
int matches_E=DOES_NOT_KNOW_IF_E_IS_MATCHED;
if (conditions[n]==NULL) {
   error("NULL internal error in resolve_conditions_for_one_graph for graph %d\n",n);
   return modification;
}
conditions[n]=resolve_condition_list(conditions[n],states,initial_states,&modification,&matches_E);
if (matches_E==E_IS_MATCHED) {
   /* If at least one condition was verified */
   set_bit_mask(&(states[initial_states[n]]->control),UNCONDITIONAL_E_MATCH);
   /* We can free the conditions */
   free_ConditionList(conditions[n]);
   conditions[n]=NULL;
   return 1;
}
if (conditions[n]==NULL) {
   /* If the graph has no more condition, then we mark it as unable to match <E> */
   set_bit_mask(&(states[initial_states[n]]->control),DOES_NOT_MATCH_E);
   free_ConditionList(conditions[n]);
   conditions[n]=NULL;
   return 1;
}
return modification;
}


/**
 * This function looks for each graph if it can resolve a condition.
 * 
 * It returns a non zero value if some conditions have been resolved, 
 * even just one; 0 otherwise.
 */
int resolve_conditions(ConditionList* conditions,int n_graphs,
                       Fst2State* states,int* initial_states,
                       unichar** graph_names) {
int modification=0;
for (int i=1;i<n_graphs+1;i++) {
   if (!is_bit_mask_set(states[initial_states[i]]->control,
                        UNCONDITIONAL_E_MATCH|DOES_NOT_MATCH_E)) {
      /* If we don't already know the status of the current graph */
      if (!is_bit_mask_set(states[initial_states[i]]->control,CONDITIONAL_E_MATCH)) {
         /* If there is no conditionnal match, then we say that it does
          * not match <E> */
         set_bit_mask(&(states[initial_states[i]]->control),DOES_NOT_MATCH_E);
         modification++;
      } else {
         /* Otherwise, we try to solve the conditions of the graph */
         modification=modification+resolve_conditions_for_one_graph(i,conditions,states,initial_states,graph_names);
      }
   }
}
return modification;
}


/**
 * Cleans the control bytes of the fst2's states. If 'graphs_matching_E'
 * is not null, graphs_matching_E[i] is set to 1 if the graph #i matches <E>;
 * 0 otherwise.
 */
void clean_controls(Fst2* fst2,int* graphs_matching_E) {
int i;
if (graphs_matching_E!=NULL) {
   for (i=1;i<fst2->number_of_graphs+1;i++) {
      if (is_bit_mask_set(fst2->states[fst2->initial_states[i]]->control,UNCONDITIONAL_E_MATCH))
         graphs_matching_E[i]=1;
      else graphs_matching_E[i]=0;
   }
}
int ALL_MASKS=CONDITIONAL_E_MATCH|UNCONDITIONAL_E_MATCH|DOES_NOT_MATCH_E|TMP_LOOP_MARK|VISITED_MARK;
for (i=0;i<fst2->number_of_states;i++) {
   unset_bit_mask(&(fst2->states[i]->control),ALL_MASKS);
}
}


/**
 * Returns 1 if the given state has already been visited; 0 otherwise.
 */
int look_for_E_loop_in_state(int state_number,Fst2* fst2,int* graphs_matching_E) {
Transition* l;
Fst2State e=fst2->states[state_number];
if (is_bit_mask_set(e->control,TMP_LOOP_MARK)) {
   /* If we have already visited this state */
   unset_bit_mask(&(e->control),TMP_LOOP_MARK);
   return 1;
}
set_bit_mask(&(e->control),TMP_LOOP_MARK);
l=e->transitions;
while (l!=NULL) {
   if (l->tag_number<0) {
      /* If we have a graph call */
      if (graphs_matching_E[-l->tag_number]) {
         /* And if we can cross it because it matches <E> */
         if (look_for_E_loop_in_state(l->state_number,fst2,graphs_matching_E)) {
            /* And if we have reached the current state via an <E> loop */
            unset_bit_mask(&(e->control),TMP_LOOP_MARK);
            return 1;
         }
      }
   }
   else if (fst2->tags[l->tag_number]->control==1) {
      /* If we have a tag that can match <E> */
      if (look_for_E_loop_in_state(l->state_number,fst2,graphs_matching_E)) {
         /* And if we have reached the current state via an <E> loop */
         unset_bit_mask(&(e->control),TMP_LOOP_MARK);
         return 1;
      }
   }
  l=l->next;
}
unset_bit_mask(&(e->control),TMP_LOOP_MARK);
return 0;
}


/**
 * Returns 1 and prints an error message if an <E> loop is found the graph #n;
 * returns 0 otherwise.
 */
int look_for_E_loops(int n,Fst2* fst2,int* graphs_matching_E) {
int first_state=fst2->initial_states[n];
for (int i=0;i<fst2->number_of_states_per_graphs[n];i++) {
   if (look_for_E_loop_in_state(first_state+i,fst2,graphs_matching_E)) {
      error("ERROR: <E> loop in the graph %S\n",fst2->graph_names[n]);
      return 1;
   }
}
return 0;
}


/**
 * Explores the transitions that outgo from the given state.
 * Returns 1 if a recursion is found; 0 otherwise.
 */
int explore_state(int state_number,struct list_int* l,Fst2* fst2,int* graphs_matching_E) {
Fst2State s=fst2->states[state_number];
int ret=0;
if (s==NULL) return 0;
if (is_bit_mask_set(s->control,TMP_LOOP_MARK|VISITED_MARK)) {
   /* If this state has already been processed */
   return 0;
}
set_bit_mask(&(s->control),TMP_LOOP_MARK|VISITED_MARK);
Transition* list=s->transitions;
while (list!=NULL) {
   if (list->tag_number<0) {
      /* If we have a subgraph call */
      if (look_for_recursion(-(list->tag_number),l,fst2,graphs_matching_E)) {
         /* If there is a recursion */
         return 1;
      }
      if (graphs_matching_E[-list->tag_number] && explore_state(list->state_number,l,fst2,graphs_matching_E)) {
         /* If the graph matches <E> */
         return 1;
      }
   } else if (fst2->tags[list->tag_number]->control==1) {
      /* If we have a transition that can match <E> */
      if (explore_state(list->state_number,l,fst2,graphs_matching_E)) {
         return 1;
      }
   }
   list=list->next;
}
unset_bit_mask(&(s->control),TMP_LOOP_MARK);
return ret;
}


/**
 * Returns 1 and prints an error message if a recursion is found in graph #n;
 * returns 0 otherwise.
 */
int look_for_recursion(int n,struct list_int* l,Fst2* fst2,int* graphs_matching_E) {
if (is_in_list(n,l)) {
   /* If we find a graph that has already been visited */
   print_reversed_list(l,n,fst2->graph_names);
   error(" recalls the graph %S\n",fst2->graph_names[n]);
   return 1;
}
l=new_list_int(n,l);
int ret=explore_state(fst2->initial_states[n],l,fst2,graphs_matching_E);
delete_head(&l);
return ret;
}


/**
 * Returns 1 if the given .fst2 is OK; 0 otherwise. OK means:
 * 1) no left recursion
 * 2) no loop that can recognize the empty word (<E> with an output or subgraph
 *    that can match the empty word).
 */
int grf_OK(char* name) {
ConditionList* conditions;
ConditionList* conditions_for_state;
int i,j;
int ERROR=0;
Fst2* fst2=load_fst2(name,1);
if (fst2==NULL) {
	fatal_error("Cannot load graph %s\n",name);
}
u_printf("Recursion detection started\n");
int* graphs_matching_E=(int*)malloc(sizeof(int)*(fst2->number_of_graphs+1));
conditions=(ConditionList*)malloc(sizeof(ConditionList)*(fst2->number_of_graphs+1));
if (graphs_matching_E==NULL || conditions==NULL) {
   fatal_error("Not enough memory in grf_OK\n");
}
for (i=0;i<fst2->number_of_graphs+1;i++) {
   graphs_matching_E[i]=0;
   conditions[i]=NULL;
}
/* First, we look for tags that match the empty word <E> */
for (i=0;i<fst2->number_of_tags;i++) {
   check_epsilon_tag(fst2->tags[i]);
}
/* Then, we look for graphs that match <E> with or without conditions */
for (i=1;i<=fst2->number_of_graphs;i++) {
   conditions_for_state=(ConditionList*)malloc(sizeof(ConditionList)*fst2->number_of_states_per_graphs[i]);
   if (conditions_for_state==NULL) {
      fatal_error("Not enough memory in grf_OK\n");
   }
   for (j=0;j<fst2->number_of_states_per_graphs[i];j++) {
      conditions_for_state[j]=NULL;
   }
   graphs_matching_E[i]=graph_matches_E(fst2->initial_states[i],fst2->initial_states[i],
  								      fst2->states,fst2->tags,i,fst2->graph_names,
  								      conditions_for_state,&conditions[i]);
   /* If any, we remove the temp conditions */                        
   if (conditions[i]!=NULL) free_ConditionList(conditions[i]);
   /* And we way that the conditions for the current graph are its initial
    * state's ones. */
   conditions[i]=conditions_for_state[0];
   /* Then we perform cleaning */
   conditions_for_state[0]=NULL;
   for (j=1;j<fst2->number_of_states_per_graphs[i];j++) {
      free_ConditionList(conditions_for_state[j]);
   }
   free(conditions_for_state); 
}
/* Then, we use all our condition lists to determine which graphs match <E>.
 * We iterate until we find a fixed point. If some conditions remain non null
 * after this loop, it means that there are <E> dependencies between graphs
 * and this case will be dealt with later. */
u_printf("Resolving <E> conditions\n");
while (resolve_conditions(conditions,fst2->number_of_graphs,
							fst2->states,fst2->initial_states,fst2->graph_names));
if (is_bit_mask_set(fst2->states[fst2->initial_states[1]]->control,UNCONDITIONAL_E_MATCH)) {
   /* If the main graph matches <E> */
   error("ERROR: the main graph %S recognizes <E>\n",fst2->graph_names[1]);
   ERROR=1;
}
if (!ERROR) {
   for (i=1;i<fst2->number_of_graphs+1;i++) {
      if (is_bit_mask_set(fst2->states[fst2->initial_states[i]]->control,UNCONDITIONAL_E_MATCH)) {
         /* If the graph matches <E> */
         error("WARNING: the graph %S recognizes <E>\n",fst2->graph_names[i]);
      }
   }
}
clean_controls(fst2,graphs_matching_E);
if (!ERROR) {
   u_printf("Looking for <E> loops\n");
   for (i=1;!ERROR && i<fst2->number_of_graphs+1;i++) {
      ERROR=look_for_E_loops(i,fst2,graphs_matching_E);
   }
}
clean_controls(fst2,NULL);
if (!ERROR) {
   u_printf("Looking for infinite recursions\n");
   for (i=1;!ERROR && i<fst2->number_of_graphs+1;i++) {
      ERROR=look_for_recursion(i,NULL,fst2,graphs_matching_E);
   }
}
for (i=1;i<fst2->number_of_graphs+1;i++) {
   free_ConditionList(conditions[i]);
}
free_Fst2(fst2);
u_printf("Recursion detection completed\n");
if (ERROR) return LEFT_RECURSION;
return NO_LEFT_RECURSION;
}









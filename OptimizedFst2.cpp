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

#include "List_int.h"
#include "LocatePattern.h"
#include "OptimizedFst2.h"
#include "LocateConstants.h"
#include "Error.h"
#include "BitMasks.h"
#include "TransductionVariables.h"


/**
 * Allocates, initializes and returns a new optimized graph call.
 */
struct opt_graph_call* new_opt_graph_call(int graph_number) {
struct opt_graph_call* g;
g=(struct opt_graph_call*)malloc(sizeof(struct opt_graph_call));
if (g==NULL) {
   fatal_error("Not enough memory in new_opt_graph_call\n");
}
g->graph_number=graph_number;
g->transition=NULL;
g->next=NULL;
return g;
}


/**
 * Frees the whole memory associate to the given graph call list.
 */
void free_opt_graph_call(struct opt_graph_call* list) {
struct opt_graph_call* tmp;
while (list!=NULL) {
   free_Fst2Transition(list->transition);
   tmp=list;
   list=list->next;
   free(tmp);
}
}


/**
 * This function adds a transition to the given transition list, if not
 * already present.
 */
void add_transition_if_not_present(Fst2Transition *list,int tag_number,int state_number) {
Fst2Transition ptr;
ptr=*list;
/* We look for a transition with the same properties */
while (ptr!=NULL && !(ptr->state_number==state_number && ptr->tag_number==tag_number)) {
   ptr=ptr->next;
}
if (ptr==NULL) {
   /* If we have not found one, we add a new transition at the head of the list */
   ptr=new_Fst2Transition(tag_number,state_number);
   ptr->next=*list;
   *list=ptr;
}
}


/**
 * This function adds the given graph call to given graph call list.
 */
void add_graph_call(Fst2Transition transition,struct opt_graph_call** graph_calls) {
int graph_number=-(transition->tag_number);
struct opt_graph_call* ptr=*graph_calls;
/* We look for a graph call for the same graph number */
while (ptr!=NULL && ptr->graph_number!=graph_number) {
   ptr=ptr->next;
}
if (ptr==NULL) {
   /* If we have found none, we create one */
   ptr=new_opt_graph_call(graph_number);
   ptr->next=*graph_calls;
   *graph_calls=ptr;
}
/* Then, we add the transition to the graph call */
add_transition_if_not_present(&(ptr->transition),transition->tag_number,transition->state_number);
}


/**
 * Allocates, initializes and returns a new opt_pattern.
 */
struct opt_pattern* new_opt_pattern(int pattern_number,int negation) {
struct opt_pattern* p;
p=(struct opt_pattern*)malloc(sizeof(struct opt_pattern));
if (p==NULL) {
   fatal_error("Not enough memory in new_opt_pattern\n");
}
p->pattern_number=pattern_number;
p->negation=negation;
p->transition=NULL;
p->next=NULL;
return p;
}


/**
 * Frees the whole memory associated to the given optimized pattern list.
 */
void free_opt_pattern(struct opt_pattern* list) {
struct opt_pattern* tmp;
while (list!=NULL) {
   free_Fst2Transition(list->transition);
   tmp=list;
   list=list->next;
   free(tmp);
}
}


/**
 * This function adds the given pattern number to the given pattern list.
 */
void add_pattern(int pattern_number,Fst2Transition transition,struct opt_pattern** pattern_list,int negation) {
struct opt_pattern* ptr=*pattern_list;
/* We look for a pattern with the same properties */
while (ptr!=NULL && !(ptr->pattern_number==pattern_number && ptr->negation==negation)) {
   ptr=ptr->next;
}
if (ptr==NULL) {
   /* If we have not found an equivalent pattern, we create one */
   ptr=new_opt_pattern(pattern_number,negation);
   ptr->next=*pattern_list;
   *pattern_list=ptr;
}
/* Finally, we add the transition to the pattern */
add_transition_if_not_present(&(ptr->transition),transition->tag_number,transition->state_number);
}


/**
 * Allocates, initializes and returns a new optimized token.
 */
struct opt_token* new_opt_token(int token_number) {
struct opt_token* t;
t=(struct opt_token*)malloc(sizeof(struct opt_token));
if (t==NULL) {
   fatal_error("Not enough memory in new_opt_token\n");
}
t->token_number=token_number;
t->transition=NULL;
t->next=NULL;
return t;
}


/**
 * Frees all the memory associated to the given token list.
 */
void free_opt_token(struct opt_token* list) {
struct opt_token* tmp;
while (list!=NULL) {
   free_Fst2Transition(list->transition);
   tmp=list;
   list=list->next;
   free(tmp);
}
}


/**
 * This function add the given token in the given 'token_list', if not already present.
 * '*number_of_tokens' is increased of 1 if the token was not present.
 * 
 * Note that this function must perform a sorted insert, since the resulting
 * list will be supposed to be sorted at the time of converting it into an array
 * in 'token_list_2_token_array'.
 */
void add_token(int token_number,Fst2Transition transition,struct opt_token** token_list,
               int *number_of_tokens) {
struct opt_token* ptr;
if (*token_list==NULL) {
   /* If the list is empty, we add the token */
   (*token_list)=new_opt_token(token_number);
   add_transition_if_not_present(&((*token_list)->transition),transition->tag_number,transition->state_number);
   (*number_of_tokens)++;
   return;
}
/* If we must insert before the head of the list */
if (token_number<(*token_list)->token_number) {
   /* If the list is empty, we add the token */
   ptr=new_opt_token(token_number);
   add_transition_if_not_present(&(ptr->transition),transition->tag_number,transition->state_number);
   ptr->next=(*token_list);
   (*token_list)=ptr;
   (*number_of_tokens)++;
   return;
}
/* If we must update the head of the list */
if (token_number==(*token_list)->token_number) {
   add_transition_if_not_present(&((*token_list)->transition),transition->tag_number,transition->state_number);
   return;
}
/* Otherwise, we look for the exact token, or the last token that is lower than
 * the one we look for */
ptr=(*token_list);
while (ptr->next!=NULL && (ptr->next->token_number<token_number)) {
   ptr=ptr->next;
}
if (ptr->next==NULL || ptr->next->token_number>token_number) {
   /* If we are at the end of list or before a greater token, then we must create the
    * token and insert it after 'ptr'. */
   struct opt_token* tmp=new_opt_token(token_number);
   add_transition_if_not_present(&(tmp->transition),transition->tag_number,transition->state_number);
   tmp->next=ptr->next;
   ptr->next=tmp;
   (*number_of_tokens)++;
   return;
}
/* Otherwise, 'ptr->next' points on the token we look for */
add_transition_if_not_present(&(ptr->next->transition),transition->tag_number,transition->state_number);
}


/**
 * This function adds all the tokens contained in the given 'token_list' into
 * 'list' that contains all the tokens matched by an optimized state.
 * '*number_of_tokens' is updated.
 */
void add_token_list(struct list_int* token_list,Fst2Transition transition,
                    struct opt_token** list,int *number_of_tokens) {
while (token_list!=NULL) {
   add_token(token_list->n,transition,list,number_of_tokens);
   token_list=token_list->next;
}
}


/**
 * Allocates, initializes and returns a new optimized meta.
 */
struct opt_meta* new_opt_meta(enum meta_symbol meta,int negation) {
struct opt_meta* m;
m=(struct opt_meta*)malloc(sizeof(struct opt_meta));
if (m==NULL) {
   fatal_error("Not enough memory in new_opt_meta\n");
}
m->meta=meta;
m->negation=negation;
m->transition=NULL;
m->next=NULL;
return m;
}


/**
 * Frees all the memory associated to the given meta list.
 */
void free_opt_meta(struct opt_meta* list) {
struct opt_meta* tmp;
while (list!=NULL) {
   free_Fst2Transition(list->transition);
   tmp=list;
   list=list->next;
   free(tmp);
}
}


/**
 * This function adds the given meta to the given meta list.
 */
void add_meta(enum meta_symbol meta,Fst2Transition transition,struct opt_meta** meta_list,int negation) {
struct opt_meta* ptr=*meta_list;
/* We look for a meta with the same properties */
while (ptr!=NULL && !(ptr->meta==meta && ptr->negation==negation)) {
   ptr=ptr->next;
}
if (ptr==NULL) {
   /* If we have found none, we create one */
   ptr=new_opt_meta(meta,negation);
   ptr->next=*meta_list;
   *meta_list=ptr;
}
/* Then, we add the transition to the meta */
add_transition_if_not_present(&(ptr->transition),transition->tag_number,transition->state_number);
}


/**
 * Allocates, initializes and returns a new optimized variable.
 */
struct opt_variable* new_opt_variable(int variable_number,Fst2Transition transition) {
struct opt_variable* v;
v=(struct opt_variable*)malloc(sizeof(struct opt_variable));
if (v==NULL) {
   fatal_error("Not enough memory in new_opt_variable\n");
}
v->variable_number=variable_number;
v->transition=NULL;
add_transition_if_not_present(&(v->transition),transition->tag_number,transition->state_number);
v->next=NULL;
return v;
}


/**
 * Frees all the memory associated to the given variable list.
 */
void free_opt_variable(struct opt_variable* list) {
struct opt_variable* tmp;
while (list!=NULL) {
   free_Fst2Transition(list->transition);
   tmp=list;
   list=list->next;
   free(tmp);
}
}


/**
 * This function adds the given variable to the given variable list.
 * No tests is done to check if there is already a transition with the
 * given variable, because it cannot happen if the grammar is deterministic.
 */
void add_variable(unichar* variable,Fst2Transition transition,struct opt_variable** variable_list) {
int n=get_transduction_variable_indice(variable);
struct opt_variable* v=new_opt_variable(n,transition);
v->next=(*variable_list);
(*variable_list)=v;
}


/**
 * Allocates, initializes and returns a new optimized context structure.
 */
struct opt_contexts* new_opt_contexts() {
struct opt_contexts* c=(struct opt_contexts*)malloc(sizeof(struct opt_contexts));
if (c==NULL) {
   fatal_error("Not enough memory in new_opt_contexts\n");
}
c->positive_mark=NULL;
c->negative_mark=NULL;
c->end_mark=NULL;
c->reacheable_states_from_positive_context=NULL;
c->reacheable_states_from_negative_context=NULL;
return c;
}


/**
 * Frees all the memory associated to the given optimized context structure.
 */
void free_opt_contexts(struct opt_contexts* c) {
if (c==NULL) return;
free_Fst2Transition(c->positive_mark);
free_Fst2Transition(c->negative_mark);
free_Fst2Transition(c->end_mark);
free_Fst2Transition(c->reacheable_states_from_positive_context);
free_Fst2Transition(c->reacheable_states_from_negative_context);
free(c);
}


/**
 * This function looks for context mark ends in the given fst2.
 */
void look_for_closing_context_mark(Fst2* fst2,int state,Fst2Transition *list,
                                   struct bit_array* marker,int nesting_level) {
if (get_value(marker,state)) {
   /* Nothing to do if this state has already been visited */
   return;
}
/* Otherwise, we mark the state */
set_value(marker,state,1);
/* And we look all its outgoing transitions */
Fst2Transition ptr=fst2->states[state]->transitions;
while (ptr!=NULL) {
   if (ptr->tag_number>=0) {
      /* If we have a tag, we check if it is a context mark */
      Fst2Tag tag=fst2->tags[ptr->tag_number];
      switch (tag->type) {
         /* If we have a context start mark, we go on with an increased nesting level */
         case BEGIN_POSITIVE_CONTEXT_TAG: 
         case BEGIN_NEGATIVE_CONTEXT_TAG: look_for_closing_context_mark(fst2,ptr->state_number,list,marker,nesting_level+1);
                                          break;
         /* If we have a context end mark */
         case END_CONTEXT_TAG: if (nesting_level==0) {
                                  /* If we are at the top nesting level, we have found a transition
                                   * to add to our list */
                                  add_transition_if_not_present(list,ptr->tag_number,ptr->state_number);
                               } else {
                                  /* Otherwisen we on with a decreased nesting level */
                                  look_for_closing_context_mark(fst2,ptr->state_number,list,marker,nesting_level-1);
                               }
                               break;
         /* If we an another type of transition, we follow it */
         default: look_for_closing_context_mark(fst2,ptr->state_number,list,marker,nesting_level);
      }
   }
   else {
      /* If we have a graph call, we follow it */
      look_for_closing_context_mark(fst2,ptr->state_number,list,marker,nesting_level);
   }
   ptr=ptr->next;
}
}


/**
 * This function explores the given fst2 from the given state and looks for
 * transition tagged by the closing context mark "$]". Such transitions are added to
 * the given list, but only if they are at the same nesting level that the original
 * state. For instance, if we find the following tag sequence:
 * 
 * <MOT> $[ <ADV> $] $]
 * 
 * we will stop on the second "$]", since the first corresponds to a different 
 * context start mark than ours.
 */
void get_reachable_closing_context_marks(Fst2* fst2,int state,Fst2Transition *list) {
/* we declare a bit array in order to mark states that have already been visited.
 * Note that we could use a bit array with a smaller length, since the only states
 * that will be explored are in the same subgraph that the one containing the
 * given start state. */
struct bit_array* marker=new_bit_array(fst2->number_of_states,ONE_BIT);
look_for_closing_context_mark(fst2,state,list,marker,0);
free_bit_array(marker);
}


/**
 * Adds a positive context to the given state or raises a fatal error if
 * there is already one, because it would mean that the fst2 is not
 * deterministic. As a side effect, this function looks for all the closing
 * context marks reachable from this positive context mark and stores them into
 * 'reacheable_states_from_positive_context'. If there is no reachable context
 * end mark, the function emit an error message and ignores this "$[" mark.
 */
void add_positive_context(Fst2* fst2,OptimizedFst2State state,Fst2Transition transition) {
int created=0;
if (state->contexts==NULL) {
   created=1;
   state->contexts=new_opt_contexts();
}
if (state->contexts->positive_mark!=NULL) {
   fatal_error("Duplicate positive context mark\n");
}
state->contexts->positive_mark=new_Fst2Transition(transition->tag_number,transition->state_number);
get_reachable_closing_context_marks(fst2,transition->state_number,&(state->contexts->reacheable_states_from_positive_context));
if (state->contexts->reacheable_states_from_positive_context==NULL) {
   error("Positive context with no end\n");
   free_Fst2Transition(state->contexts->positive_mark);
   state->contexts->positive_mark=NULL;
   if (created) {
      free(state->contexts);
      state->contexts=NULL;
   }
}
}


/**
 * Adds a negative context to the given state or raises a fatal error if
 * there is already one, because it would mean that the fst2 is not
 * deterministic. As a side effect, this function looks for all the closing
 * context marks reachable from this negative context mark and stores them into
 * 'reacheable_states_from_negative_context'. If there is no reachable context
 * end mark, the function emit an error message and ignores this "$![" mark.
 */
void add_negative_context(Fst2* fst2,OptimizedFst2State state,Fst2Transition transition) {
int created=0;
if (state->contexts==NULL) {
   created=1;
   state->contexts=new_opt_contexts();
}
if (state->contexts->negative_mark!=NULL) {
   fatal_error("Duplicate negative context mark\n");
}
state->contexts->negative_mark=new_Fst2Transition(transition->tag_number,transition->state_number);
get_reachable_closing_context_marks(fst2,transition->state_number,&(state->contexts->reacheable_states_from_negative_context));
if (state->contexts->reacheable_states_from_negative_context==NULL) {
   error("Negative context with no end\n");
   free_Fst2Transition(state->contexts->negative_mark);
   state->contexts->negative_mark=NULL;
   if (created) {
      free(state->contexts);
      state->contexts=NULL;
   }
}
}


/**
 * Adds a context end to the given state or raises a fatal error if
 * there is already one, because it would mean that the fst2 is not
 * deterministic.
 */
void add_end_context(OptimizedFst2State state,Fst2Transition transition) {
if (state->contexts==NULL) {
   state->contexts=new_opt_contexts();
}
if (state->contexts->end_mark!=NULL) {
   fatal_error("Duplicate end context mark\n");
}
state->contexts->end_mark=new_Fst2Transition(transition->tag_number,transition->state_number);
}


/**
 * This function optimizes the given transition.
 */
void optimize_transition(Fst2* fst2,Fst2Transition transition,OptimizedFst2State state,Fst2Tag* tags) {
if (transition->tag_number<0) {
   /* If the transition is a graph call */
   add_graph_call(transition,&(state->graph_calls));
   return;
}
Fst2Tag tag=tags[transition->tag_number];
if (tag==NULL) {
   fatal_error("NULL transition tag in optimize_transition\n");
}
int negation=is_bit_mask_set(tag->control,NEGATION_TAG_BIT_MASK);
/* First, we look if there is a compound pattern associated to this tag */
if (tag->compound_pattern!=NO_COMPOUND_PATTERN) {
   add_pattern(tag->compound_pattern,transition,&(state->compound_patterns),negation);
}
/* Then, we look the possible kind of transitions */
switch (tag->type) {
   case TOKEN_LIST_TAG: add_token_list(tag->matching_tokens,transition,&(state->token_list),&(state->number_of_tokens));
                        return;
   case PATTERN_NUMBER_TAG: add_pattern(tag->pattern_number,transition,&(state->patterns),negation);
                            return;
   case META_TAG: add_meta(tag->meta,transition,&(state->metas),negation);
                           return;
   case BEGIN_VAR_TAG: add_variable(tag->variable,transition,&(state->variable_starts));
                       return;
   case END_VAR_TAG: add_variable(tag->variable,transition,&(state->variable_ends));
                     return;
   case BEGIN_POSITIVE_CONTEXT_TAG: add_positive_context(fst2,state,transition); return;
   case BEGIN_NEGATIVE_CONTEXT_TAG: add_negative_context(fst2,state,transition); return;
   case END_CONTEXT_TAG: add_end_context(state,transition); return;
   default: fatal_error("Unexpected transition tag type in optimize_transition\n");
}
}


/**
 * This function builds an array containing the token numbers stored
 * in the token list of the given state. As the token list is supposed
 * to be sorted, the array will be sorted. The function frees the token
 * list.
 */
void token_list_2_token_array(OptimizedFst2State state) {
int i;
struct opt_token* l;
struct opt_token* tmp;
if (state->number_of_tokens==0) {
   /* Nothing to do if there is no token in the list */
   return;
}
state->tokens=(int*)malloc(sizeof(int)*state->number_of_tokens);
state->token_transitions=(Fst2Transition*)malloc(sizeof(Fst2Transition)*state->number_of_tokens);
if (state->tokens==NULL || state->token_transitions==NULL) {
   fatal_error("Not enough memory in token_list_2_token_array\n");
}
i=0;
l=state->token_list;
while (l!=NULL) {
   state->tokens[i]=l->token_number;
   state->token_transitions[i]=l->transition;
   i++;
   tmp=l;
   l=l->next;
   /* We must NOT free 'tmp->transition' since it is referenced now
    * in 'state->token_transitions[i]' */
   free(tmp);
}
if (i!=state->number_of_tokens) {
   fatal_error("Internal error in token_list_2_token_array\n");
}
state->token_list=NULL;
}


/**
 * Allocates, initializes and returns a new optimized state.
 */
OptimizedFst2State new_optimized_state() {
OptimizedFst2State state=(OptimizedFst2State)malloc(sizeof(struct optimizedFst2State));
if (state==NULL) {
   fatal_error("Not enough memory in new_optimized_state\n");
}
state->control=0;
state->graph_calls=NULL;
state->metas=NULL;
state->patterns=NULL;
state->compound_patterns=NULL;
state->token_list=NULL;
state->tokens=NULL;
state->token_transitions=NULL;
state->number_of_tokens=0;
state->variable_starts=NULL;
state->variable_ends=NULL;
state->contexts=NULL;
return state;
}


/**
 * Frees the whole memory associated to the given optimized state.
 */
void free_optimized_state(OptimizedFst2State state) {
if (state==NULL) return;
free_opt_graph_call(state->graph_calls);
free_opt_meta(state->metas);
free_opt_pattern(state->patterns);
free_opt_pattern(state->compound_patterns);
free_opt_token(state->token_list);
free_opt_variable(state->variable_starts);
free_opt_variable(state->variable_ends);
free_opt_contexts(state->contexts);
if (state->tokens!=NULL) free(state->tokens);
if (state->token_transitions!=NULL) {
   for (int i=0;i<state->number_of_tokens;i++) {
      free_Fst2Transition(state->token_transitions[i]);
   }
   free(state->token_transitions);
}
free(state);
}


/**
 * This function looks all the transitions that outgo from the given state
 * and returns an equivalent optimized state, or NULL if the given state
 * was NULL.
 */
OptimizedFst2State optimize_state(Fst2* fst2,Fst2State state,int state_number,Fst2Tag* tags) {
if (state==NULL) return NULL;
OptimizedFst2State new_state=new_optimized_state();
new_state->control=state->control;
struct fst2Transition* ptr=state->transitions;
while (ptr!=NULL) {
   optimize_transition(fst2,ptr,new_state,tags);
   ptr=ptr->next;
}
token_list_2_token_array(new_state);
return new_state;
}


/**
 * This function takes a fst2 and returns an array containing the corresponding
 * optimized states.
 */
OptimizedFst2State* build_optimized_fst2_states(Fst2* fst2) {
OptimizedFst2State* optimized_states=(OptimizedFst2State*)malloc(fst2->number_of_states*sizeof(OptimizedFst2State));
if (optimized_states==NULL) {
   fatal_error("Not enough memory in build_optimized_fst2_states\n");
}
for (int i=0;i<fst2->number_of_states;i++) {
   optimized_states[i]=optimize_state(fst2,fst2->states[i],i,fst2->tags);
}
return optimized_states;
}


/**
 * Frees the whole memory associated to the given optimized state array.
 */
void free_optimized_states(OptimizedFst2State* states,int size) {
if (states==NULL) return;
for (int i=0;i<size;i++) {
   free_optimized_state(states[i]);
}
free(states);
}


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

#include "Contexts.h"
#include "Error.h"
#include "BitArray.h"


/**
 * This function looks for context mark ends in the given fst2.
 */
void look_for_closing_context_mark(Fst2* fst2,int state,Transition** list,
                                   struct bit_array* marker,int nesting_level) {
if (get_value(marker,state)) {
   /* Nothing to do if this state has already been visited */
   return;
}
/* Otherwise, we mark the state */
set_value(marker,state,1);
/* And we look all its outgoing transitions */
Transition* ptr=fst2->states[state]->transitions;
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
void get_reachable_closing_context_marks(Fst2* fst2,int state,Transition** list) {
/* we declare a bit array in order to mark states that have already been visited.
 * Note that we could use a bit array with a smaller length, since the only states
 * that will be explored are in the same subgraph that the one containing the
 * given start state. */
struct bit_array* marker=new_bit_array(fst2->number_of_states,ONE_BIT);
(*list)=NULL;
look_for_closing_context_mark(fst2,state,list,marker,0);
free_bit_array(marker);
}


/**
 * Allocates, initializes and returns a new optimized context structure.
 */
struct opt_contexts* new_opt_contexts() {
struct opt_contexts* c=(struct opt_contexts*)malloc(sizeof(struct opt_contexts));
if (c==NULL) {
   fatal_alloc_error("new_opt_contexts");
}
c->positive_mark=NULL;
c->size_positive=0;
c->negative_mark=NULL;
c->size_negative=0;
c->end_mark=NULL;
return c;
}


/**
 * Frees all the memory associated to the given optimized context structure.
 */
void free_opt_contexts(struct opt_contexts* c) {
if (c==NULL) return;
for (int i=0;i<c->size_positive;i++) {
   free_Transition_list(c->positive_mark[i]);
}
if (c->positive_mark!=NULL) free(c->positive_mark);
for (int i=0;i<c->size_negative;i++) {
   free_Transition_list(c->negative_mark[i]);
}
if (c->negative_mark!=NULL) free(c->negative_mark);
free_Transition_list(c->end_mark);
free(c);
}


/**
 * Adds a positive context to the given state or raises a fatal error if
 * there is already one, because it would mean that the fst2 is not
 * deterministic. As a side effect, this function looks for all the closing
 * context marks reachable from this positive context mark and stores them into
 * 'reacheable_states_from_positive_context'. If there is no reachable context
 * end mark, the function emit an error message and ignores this "$[" transition.
 */
void add_positive_context(Fst2* fst2,struct opt_contexts* *contexts,Transition* transition) {
int created=0;
if (*contexts==NULL) {
   created=1;
   (*contexts)=new_opt_contexts();
}
if ((*contexts)->positive_mark!=NULL) {
   fatal_error("Duplicate positive context mark\n");
}
int n=(*contexts)->size_positive;
(*contexts)->size_positive=(*contexts)->size_positive+2;
(*contexts)->positive_mark=(Transition**)realloc((*contexts)->positive_mark,(*contexts)->size_positive*sizeof(Transition*));
if ((*contexts)->positive_mark==NULL) {
   fatal_alloc_error("add_positive_context");
}
(*contexts)->positive_mark[n]=new_Transition(transition->tag_number,transition->state_number);
get_reachable_closing_context_marks(fst2,transition->state_number,&((*contexts)->positive_mark[n+1]));
if ((*contexts)->positive_mark[n+1]==NULL) {
   error("Positive context with no end\n");
   free_Transition_list((*contexts)->positive_mark[n]);
   if (n==0) {
      free((*contexts)->positive_mark);
      (*contexts)->positive_mark=NULL;
   }
   if (created) {
      free((*contexts));
      (*contexts)=NULL;
   }
}
}


/**
 * Adds a negative context to the given state or raises a fatal error if
 * there is already one, because it would mean that the fst2 is not
 * deterministic. As a side effect, this function looks for all the closing
 * context marks reachable from this negative context mark and stores them into
 * 'reacheable_states_from_negative_context'. If there is no reachable context
 * end mark, the function emit an error message and ignores this "$![" transition.
 */
void add_negative_context(Fst2* fst2,struct opt_contexts* *contexts,Transition* transition) {
int created=0;
if (*contexts==NULL) {
   created=1;
   (*contexts)=new_opt_contexts();
}
int n=(*contexts)->size_negative;
(*contexts)->size_negative=(*contexts)->size_negative+2;
(*contexts)->negative_mark=(Transition**)realloc((*contexts)->negative_mark,(*contexts)->size_negative*sizeof(Transition*));
if ((*contexts)->negative_mark==NULL) {
   fatal_alloc_error("add_negative_context");
}
(*contexts)->negative_mark[n]=new_Transition(transition->tag_number,transition->state_number);
get_reachable_closing_context_marks(fst2,transition->state_number,&((*contexts)->negative_mark[n+1]));
if ((*contexts)->negative_mark[n+1]==NULL) {
   error("Negative context with no end\n");
   free_Transition_list((*contexts)->negative_mark[n]);
   if (n==0) {
      free((*contexts)->negative_mark);
      (*contexts)->negative_mark=NULL;
   }
   if (created) {
      free((*contexts));
      (*contexts)=NULL;
   }
}
}


/**
 * Computes contexts for all states of the given fst2.
 */
struct opt_contexts** compute_contexts(Fst2* fst2) {
struct opt_contexts** contexts=(struct opt_contexts**)calloc(fst2->number_of_states,sizeof(struct opt_contexts*));
if (contexts==NULL) {
   fatal_alloc_error("compute_contexts");
}
for (int i=0;i<fst2->number_of_states;i++) {
   Transition* t=fst2->states[i]->transitions;
   while (t!=NULL) {
      if (t->tag_number>0) {
         /* Subgraph calls and epsilon are not to be considered */
         Fst2Tag tag=fst2->tags[t->tag_number];
         if (tag->type==BEGIN_POSITIVE_CONTEXT_TAG) {
            add_positive_context(fst2,&(contexts[i]),t);
         } else if (tag->type==BEGIN_NEGATIVE_CONTEXT_TAG) {
            add_negative_context(fst2,&(contexts[i]),t);
         } 
      }
      t=t->next;
   }
}
return contexts;
}


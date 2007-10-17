 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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



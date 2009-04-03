 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <string.h>
#include "Transitions.h"
#include "Error.h"


/**
 * Creates, initializes and returns a transition tagged by an integer.
 */
Transition* new_Transition(int tag_number,int state_number,Transition* next) {
Transition* transition;
transition=(Transition*)malloc(sizeof(Transition));
if (transition==NULL) {
  fatal_alloc_error("new_Transition");
}
transition->tag_number=tag_number;
transition->state_number=state_number;
transition->next=next;
return transition;
}


/**
 * Creates, initializes and returns a transition tagged by an integer.
 */
Transition* new_Transition(int tag_number,int state_number) {
return new_Transition(tag_number,state_number,NULL);
}


/**
 * Creates, initializes and returns a transition tagged by a pointer.
 */
Transition* new_Transition(symbol_t* label,int state_number,Transition* next) {
Transition* transition;
transition=(Transition*)malloc(sizeof(Transition));
if (transition==NULL) {
  fatal_alloc_error("new_Transition");
}
transition->label=dup_symbol(label);
transition->state_number=state_number;
transition->next=next;
return transition;
}


/**
 * The same than above, except that it does not duplicate the given symbol.
 */
Transition* new_Transition_no_copy(symbol_t* label,int state_number,Transition* next) {
Transition* transition;
transition=(Transition*)malloc(sizeof(Transition));
if (transition==NULL) {
  fatal_alloc_error("new_Transition");
}
transition->label=label;
transition->state_number=state_number;
transition->next=next;
return transition;
}


/**
 * Creates, initializes and returns a transition tagged by a pointer.
 */
Transition* new_Transition(symbol_t* label,int state_number) {
return new_Transition(label,state_number,NULL);
}


/**
 * Frees a transition list.
 */
void free_Transition_list(Transition* t,void(*free_elag_tag)(symbol_t*)) {
Transition* tmp;
while (t!=NULL) {
   tmp=t;
   t=t->next;
   if (free_elag_tag!=NULL && tmp->label!=NULL) {
      free_elag_tag(tmp->label);
   }
   free(tmp);
}
}


/**
 * Frees a transition list.
 */
void free_Transition_list(Transition* t) {
free_Transition_list(t,NULL);
}


/**
 * Frees a single transition.
 */
void free_Transition(Transition* t,void(*free_elag_tag)(symbol_t*)) {
if (t==NULL) return;
if (free_elag_tag!=NULL && t->label!=NULL) {
   free_elag_tag(t->label);
}
free(t);
}


/**
 * This function adds a transition to the given transition list, if not
 * already present.
 */
void add_transition_if_not_present(Transition** list,int tag_number,int state_number) {
Transition* ptr;
ptr=*list;
/* We look for a transition with the same properties */
while (ptr!=NULL && !(ptr->state_number==state_number && ptr->tag_number==tag_number)) {
   ptr=ptr->next;
}
if (ptr==NULL) {
   /* If we have not found one, we add a new transition at the head of the list */
   *list=new_Transition(tag_number,state_number,*list);
}
}


/**
 * This function adds a transition to the given transition list, if not
 * already present.
 */
void add_transition_if_not_present(Transition** list,symbol_t* label,int state_number) {
Transition* ptr;
ptr=*list;
/* We look for a transition with the same properties */
while (ptr!=NULL && !(ptr->state_number==state_number && ptr->label==label)) {
   ptr=ptr->next;
}
if (ptr==NULL) {
   /* If we have not found one, we add a new transition at the head of the list */
   *list=new_Transition(label,state_number,*list);
}
}


/**
 * Returns a clone of the given transition, whatever it's tagged by
 * an integer or a pointer. If 'clone_tag_label' is not NULL,
 * this function is used to duplicate t's pointer label.
 */
Transition* clone_transition(Transition* t,symbol_t*(*clone_elag_symbol)(const symbol_t*)) {
Transition* transition;
transition=(Transition*)malloc(sizeof(Transition));
if (transition==NULL) {
  fatal_alloc_error("clone_transition");
}
if (clone_elag_symbol==NULL) {
   memcpy(transition,t,sizeof(Transition));
} else {
   transition->label=clone_elag_symbol(t->label);
   transition->state_number=t->state_number;
   transition->next=t->next;
}
return transition;
}


/**
 * Clones the given transition list. If 'renumber' is not NULL, it
 * is used to renumber destination states on the fly. If 
 * 'clone_tag_label' is not NULL, it is used to clone the pointer
 * labels. If NULL, transitions are rawly copied with a memcpy.
 */
Transition* clone_transition_list(Transition* list,int* renumber,symbol_t*(*clone_elag_symbol)(const symbol_t*)) {
if (list==NULL) return NULL;
Transition* result=clone_transition(list,clone_elag_symbol);
result->next=NULL;
if (renumber!=NULL) {
   result->state_number=renumber[result->state_number];
}
list=list->next;
Transition* tmp=result;
while (list!=NULL) {
   tmp->next=clone_transition(list,clone_elag_symbol);
   tmp->next->next=NULL;
   if (renumber!=NULL) {
      tmp->next->state_number=renumber[tmp->next->state_number];
   }
   list=list->next;
   tmp=tmp->next;
}
return result;
}


/**
 * Concatenates 'src' at the end of 'dest'. 'dest' is modified.
 */
void concat(Transition** dest,Transition* src) {
while ((*dest)!=NULL) {
   dest=&(*dest)->next;
}
*dest=src;
}


/**
 * Takes a list of transitions, and replaces in them 'old_state_number' by
 * 'new_state_number'.
 */
void renumber_transitions(Transition* list,int old_state_number,int new_state_number) {
while (list!=NULL) {
   if (list->state_number==old_state_number) {
      list->state_number=new_state_number;
   }
   list=list->next;
}
}


/**
 * Increases all the destination states of the given transitions
 * with 'shift'.
 */
Transition* shift_destination_states(Transition* trans,int shift) {
for (Transition* t=trans;t!=NULL;t=t->next) {
   t->state_number=t->state_number+shift;
}
return trans;
}


/**
 * Adds all the transitions of 'src' to '*dest', if not already present.
 */
void add_transitions_int(Transition* src,Transition** dest) {
while (src!=NULL) {
   add_transition_if_not_present(dest,src->tag_number,src->state_number);
   src=src->next;
}
}


/**
 * Adds all the transitions of 'src' to '*dest', if not already present.
 */
void add_transitions_ptr(Transition* src,Transition** dest) {
while (src!=NULL) {
   add_transition_if_not_present(dest,src->label,src->state_number);
   src=src->next;
}
}

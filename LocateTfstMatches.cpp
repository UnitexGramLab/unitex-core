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

#include "LocateTfstMatches.h"
#include "Error.h"
#include <stdlib.h>


/**
 * Allocates, initializes and returns a new tfst_match.
 */
struct tfst_match* new_tfst_match(int source_state_text,
                                  int dest_state_text,
                                  int grammar_tag_number,
                                  int text_tag_number) {
struct tfst_match* match=(struct tfst_match*)malloc(sizeof(struct tfst_match));
if (match==NULL) {
   fatal_alloc_error("new_tfst_match");
}
match->source_state_text=source_state_text;
match->dest_state_text=dest_state_text;
match->grammar_tag_number=grammar_tag_number;
match->text_tag_numbers=sorted_insert(text_tag_number,NULL);
match->next=NULL;
match->pointed_by=0;
return match;
}


/**
 * Inserts a tfst_match corresponding to the given information, if not already present.
 */
struct tfst_match* insert_in_tfst_matches(struct tfst_match* list,
                                          int source_state_text,
                                          int dest_state_text,
                                          int grammar_tag_number,
                                          int text_tag_number) {
if (list==NULL) {
   return new_tfst_match(source_state_text,dest_state_text,
                         grammar_tag_number,text_tag_number);
}
if (list->source_state_text==source_state_text
    && list->dest_state_text==dest_state_text
    && list->grammar_tag_number==grammar_tag_number) {
    list->text_tag_numbers=sorted_insert(text_tag_number,list->text_tag_numbers);
    return list;
}
list->next=insert_in_tfst_matches(list->next,
                         source_state_text,dest_state_text,
                         grammar_tag_number,text_tag_number);
return list;
}


/**
 * Frees the given tfst_match.
 */
void free_tfst_match(struct tfst_match* match) {
if (match==NULL) {
   fatal_error("NULL error in free_tfst_match");
}
free_list_int(match->text_tag_numbers);
free(match);
}


/**
 * Allocates, initializes and returns a tfst_match_list.
 */
struct tfst_match_list* new_tfst_match_list() {
struct tfst_match_list* l=(struct tfst_match_list*)malloc(sizeof(struct tfst_match_list*));
if (l==NULL) {
   fatal_alloc_error("new_tfst_match_list");
}
l->match=NULL;
l->next=NULL;
return l;
}


/**
 * Adds a tfst_match to the given list, increasing its reference counter.
 */
struct tfst_match_list* add_match_in_list(struct tfst_match_list* list,
                                          struct tfst_match* match) {
struct tfst_match_list* l=new_tfst_match_list();
l->match=match;
l->next=list;
(match->pointed_by)++;
return l;
}


/**
 * This function takes a list of match elements and free the first elements
 * until they have a non null pointed_by field. In other words, this function
 * releases all the first elements that are not involved anymore in any match.
 * The limit parameter refers to a stop element. In fact, when we want to
 * clean a list of match elements associated to a subgraph call, we do not want
 * to remove the elements that do not depend on this subgraph call.
 */
void clean_tfst_match_list(struct tfst_match* list,struct tfst_match* limit) {
struct tfst_match* tmp;
while (list!=NULL && list!=limit && list->pointed_by==0) {
   /* We have an element that we can free */
   if (list->next!=NULL) {
      /* If there is a next element, then we say
       * that it is now pointed by one element less */
      (list->next->pointed_by)--;
   }
   tmp=list->next;
   free_tfst_match(list);
   list=tmp;
}
}


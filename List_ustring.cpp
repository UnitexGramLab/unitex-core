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

//---------------------------------------------------------------------------
#include <stdlib.h>
#include "List_ustring.h"
#include "Error.h"
//---------------------------------------------------------------------------


/**
 * Allocates, initializes and returns a new string list element.
 */
struct list_ustring* new_list_ustring(unichar* string,struct list_ustring* following) {
if (string==NULL) {
   fatal_error("NULL string argument in new_list_ustring\n");
}
struct list_ustring* l;
l=(struct list_ustring*)malloc(sizeof(struct list_ustring));
if (l==NULL) {
   fatal_error("Not enough memory in new_list_ustring\n");
}
l->string=u_strdup(string);
l->next=following;
return l;
}


/**
 * Frees a whole unicode string list.
 */
void free_list_ustring(struct list_ustring* head) {
struct list_ustring* tmp;
while (head!=NULL) {
   tmp=head;
   head=head->next;
   if (tmp->string!=NULL) {
      /* This case should always happen */
      free(tmp->string);
   }
   free(tmp);
}
}


/**
 * Inserts a value in a sorted list, if not already present. The
 * element that contains the value is returned.
 * 
 * NOTE: in the general case, a struct list_ustring is not supposed
 *       to be sorted.
 */
struct list_ustring* sorted_insert(unichar* value,struct list_ustring* l) {
if (value==NULL) {
   fatal_error("NULL string argument in sorted_insert\n");
}
struct list_ustring* tmp;
if (l==NULL) {
   tmp=new_list_ustring(value);
   return tmp;
}
int res=u_strcmp(value,l->string);
if (!res) return l;
if (res<0) {
   tmp=new_list_ustring(value);
   tmp->next=l;
   return tmp;
}
l->next=sorted_insert(value,l->next);
return l;
}


/**
 * Inserts an element at the head of the list.
 */
struct list_ustring* head_insert(unichar* value,struct list_ustring* old_head) {
if (value==NULL) {
   fatal_error("NULL string argument in head_insert\n");
}
struct list_ustring* new_head=new_list_ustring(value);
new_head->next=old_head;
return new_head;
}


/**
 * Returns 1 if the given value is in the list; 0 otherwise.
 */
int is_in_list(unichar* value,struct list_ustring* l) {
if (value==NULL) {
   fatal_error("NULL string argument in is_in_list\n");
}
while (l!=NULL) {
  if (!u_strcmp(l->string,value)) return 1;
  l=l->next;
}
return 0;
}


/**
 * Returns 1 if a is the same than b, i.e. they are
 * both NULL or they contain the same elements in the
 * same order.
 */
int equal(struct list_ustring* a,struct list_ustring* b) {
if (a==NULL) {
   if (b==NULL) return 1;
   else return 0;
}
if (b==NULL) {
   return 0;
}
if (u_strcmp(a->string,b->string)) {
   return 0;
}
return equal(a->next,b->next);
}


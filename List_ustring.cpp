/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdlib.h>
#include "List_ustring.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates, initializes and returns a new string list element.
 */
struct list_ustring* new_list_ustring(const unichar* string,struct list_ustring* following,Abstract_allocator prv_alloc) {
if (string==NULL) {
   fatal_error("NULL string argument in new_list_ustring\n");
}
struct list_ustring* l;
l=(struct list_ustring*)malloc_cb(sizeof(struct list_ustring),prv_alloc);
if (l==NULL) {
   fatal_alloc_error("new_list_ustring");
}
l->string=u_strdup(string,prv_alloc);
l->next=following;
return l;
}


/**
 * Allocates, initializes and returns a new string list element.
 */
struct list_ustring* new_list_ustring(const unichar* string,Abstract_allocator prv_alloc) {
return new_list_ustring(string,NULL,prv_alloc);
}


/**
 * Frees a whole unicode string list.
 */
void free_list_ustring(struct list_ustring* head,Abstract_allocator prv_alloc) {
struct list_ustring* tmp;
while (head!=NULL) {
   tmp=head;
   head=head->next;
   if (tmp->string!=NULL) {
      /* This case should always happen */
      free_cb(tmp->string,prv_alloc);
   }
   free_cb(tmp,prv_alloc);
}
}


/**
 * Frees a unicode string list element.
 */
void free_list_ustring_element(struct list_ustring* element,Abstract_allocator prv_alloc) {
if (element==NULL) return;
if (element->string!=NULL) free_cb(element->string,prv_alloc);
free_cb(element,prv_alloc);
}


/**
 * Inserts a value in a sorted list, if not already present. The
 * element that contains the value is returned.
 * 
 * NOTE: in the general case, a struct list_ustring is not supposed
 *       to be sorted.
 */
struct list_ustring* sorted_insert(const unichar* value,struct list_ustring* l,Abstract_allocator prv_alloc) {
if (value==NULL) {
   fatal_error("NULL string argument in sorted_insert\n");
}
if (l==NULL) {
   return new_list_ustring(value,prv_alloc);
}

list_ustring* baselist = l;
int res=u_strcmp(value,l->string);
if (!res) {
   return l;
}
if (res<0) {
   return new_list_ustring(value,l,prv_alloc);
}

for (;;) {
    list_ustring* lnext = l->next;
    if (lnext==NULL)
    {
        l->next=new_list_ustring(value,prv_alloc);
        return baselist;
    }
    int res2=u_strcmp(value,lnext->string);
    if (res2==0)
        return baselist;
    if (res2<0)
    {
        l->next=new_list_ustring(value,lnext,prv_alloc);
        return baselist;
    }
    l=lnext;
}

}


/**
 * Inserts an element at the head of the list.
 */
struct list_ustring* head_insert(const unichar* value,struct list_ustring* old_head,Abstract_allocator prv_alloc) {
if (value==NULL) {
   fatal_error("NULL string argument in head_insert\n");
}
struct list_ustring* new_head=new_list_ustring(value,prv_alloc);
new_head->next=old_head;
return new_head;
}


/**
 * Inserts an element at the end of a list.
 */
struct list_ustring* insert_at_end_of_list(const unichar* s,struct list_ustring* l,Abstract_allocator prv_alloc) {
if (l==NULL) return new_list_ustring(s,prv_alloc);
struct list_ustring* l_browse = l;
while (l_browse->next != NULL) l_browse = l_browse->next;
l_browse->next=new_list_ustring(s, prv_alloc);
return l;
}


/**
* Inserts an element at the end of a list maintaing pointer to latest item.
*/
struct list_ustring* insert_at_end_of_list_with_latest(const unichar* s, struct list_ustring* l, struct list_ustring** latest, Abstract_allocator prv_alloc) {
  if ((*latest) == NULL) {
    *latest = new_list_ustring(s, prv_alloc);
    return *latest;
  }	
  (*latest)->next = new_list_ustring(s, prv_alloc);
  *latest = (*latest)->next;
  return l;
}


/**
 * Returns 1 if the given value is in the list; 0 otherwise.
 */
int is_in_list(const unichar* value,const struct list_ustring* l) {
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
int equal(const struct list_ustring* a,const struct list_ustring* b) {
while ((a!=NULL) && (b!=NULL)) {
   if (u_strcmp(a->string,b->string)) {
      return 0;
   }
   a=a->next;
   b=b->next;
}

if ((a==NULL) && (b==NULL)) {
   return 1;
}
return 0;
}


/**
 * Returns a clone of the list.
 */
struct list_ustring* clone(const struct list_ustring* list,Abstract_allocator prv_alloc) {
if (list==NULL) return NULL;
list_ustring* result=new_list_ustring(list->string,NULL,prv_alloc);
list=list->next;
list_ustring* tmp=result;
while (list!=NULL) {
   tmp->next=new_list_ustring(list->string,NULL,prv_alloc);
   tmp->next->next=NULL;
   list=list->next;
   tmp=tmp->next;
}

return result;
}

/**
 * Returns the length of the list.
 */
int length(const struct list_ustring* list) {
int n=0;
while (list!=NULL) {
   list=list->next;
   n++;
}
return n;
}

} // namespace unitex

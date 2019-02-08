/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "List_int.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates, initializes and returns a new int list element.
 */
struct list_int* new_list_int(int value,struct list_int* next,Abstract_allocator prv_alloc) {
struct list_int* l;
l=(struct list_int*)malloc_cb(sizeof(struct list_int),prv_alloc);
if (l==NULL) {
   fatal_alloc_error("new_list_int");
}
l->n=value;
l->next=next;
return l;
}


/**
 * Allocates, initializes and returns a new int list element.
 */
struct list_int* new_list_int(int value,Abstract_allocator prv_alloc) {
return new_list_int(value,NULL,prv_alloc);
}


/**
 * Frees a whole int list.
 */
void free_list_int(struct list_int* head,Abstract_allocator prv_alloc) {
struct list_int* tmp;
while (head!=NULL) {
   tmp=head;
   head=head->next;
   free_cb(tmp,prv_alloc);
}
}

/**
 * Frees a whole int list.
 */
void free_list_int(struct list_int* head) {
free_list_int(head,STANDARD_ALLOCATOR);
}


/**
 * Inserts a value in a sorted list, if not already present. The
 * element that contains the value is returned.
 *
 * NOTE: in the general case, a struct list_int is not supposed
 *       to be sorted.
 *
 * Time-critical function: the iterative implementation is faster!
 */
struct list_int* sorted_insert(int value,struct list_int* l,Abstract_allocator prv_alloc) {

register struct list_int* tmp;
struct list_int* tmp2;
struct list_int* last = NULL;

if (l==NULL)  /* empty list */
   return new_list_int(value,prv_alloc);

for (tmp=l; tmp!=NULL; tmp=tmp->next) {
  if (value==tmp->n) /* is in list */
     return l;
  if (value<tmp->n) { /* smaller than element tmp */
    tmp2=new_list_int(value,tmp,prv_alloc);
    if (last==NULL) /* tmp was the first element: tmp2 will get the
                       first */
      l = tmp2;
    else
      last->next = tmp2;
    return l;
  }
  last=tmp;
}
/* value not found in the list and there is no bigger element in the
   list: insert at the end of the list */
tmp2=new_list_int(value,prv_alloc);
last->next = tmp2;
return l;
}
/*
struct list_int* sorted_insert(int value,struct list_int* l) {
struct list_int* tmp;
if (l==NULL) {
   tmp=new_list_int(value);
   return tmp;
}
if (value==l->n)
  return l;
if (value<l->n) {
   tmp=new_list_int(value);
   tmp->next=l;
   return tmp;
}
l->next=sorted_insert(value,l->next);
return l;
}
*/


/**
 * Inserts an element at the head of the list.
 */
struct list_int* head_insert(int value,struct list_int* old_head,Abstract_allocator prv_alloc) {
struct list_int* new_head=new_list_int(value,prv_alloc);
new_head->next=old_head;
return new_head;
}


/**
 * Returns 1 if the given value is in the list; 0 otherwise.
 */
int is_in_list(int value,const struct list_int* l) {
while (l!=NULL) {
  if (l->n==value) return 1;
  l=l->next;
}
return 0;
}


/**
 * Returns 1 if a is the same than b, i.e. they are
 * both NULL or they contain the same elements in the
 * same order.
 */
int equal_list_int(const struct list_int* a,const struct list_int* b) {
while ((a!=NULL) && (b!=NULL)) {
   if (a->n != b->n) {
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
 * This function sums the elements of the list and returns it as an hash code.
 */
unsigned int hash_list_int(const struct list_int* list) {
int n=0;
while (list!=NULL) {
   n=n+list->n;
   list=list->next;
}
return n;
}


/**
 * Returns the length of the list.
 */
int length(const struct list_int* list) {
int n=0;
while (list!=NULL) {
   list=list->next;
   n++;
}
return n;
}


/**
 * This function returns a list that is the sorted merge of the two given lists.
 * Duplicate elements are freed, if any.
 */
struct list_int* destructive_sorted_merge(struct list_int* a,struct list_int* b,Abstract_allocator prv_alloc) {
struct list_int* tmp;
while (b!=NULL) {
   a=sorted_insert(b->n,a,prv_alloc);
   tmp=b;
   b=b->next;
   free_cb(tmp,prv_alloc);
}
return a;
}


/**
 * This function puts in 'a' the merge of 'a' and 'b'. The merge result
 * is returned. 'b' is not modified.
 */
struct list_int* sorted_merge(struct list_int* a,struct list_int* b,Abstract_allocator prv_alloc) {
while (b!=NULL) {
   a=sorted_insert(b->n,a,prv_alloc);
   b=b->next;
}
return a;
}


/**
 * This function removes the head of the given list.
 */
void delete_head(struct list_int* *l,Abstract_allocator prv_alloc) {
if (*l==NULL) return;
struct list_int* tmp=*l;
*l=(*l)->next;
free_cb(tmp,prv_alloc);
}


/**
 * This function removes the tail of the given list.
 */
void delete_tail(struct list_int* *l,Abstract_allocator prv_alloc) {
struct list_int* previous;
if ((*l)==NULL) return;
if ((*l)->next==NULL) {
  free_cb(*l,prv_alloc);
  *l=NULL;
  return;
}
previous=*l;
while (previous->next->next!=NULL) {
   previous=previous->next;
}
free_cb(previous->next,prv_alloc);
previous->next=NULL;
}


/**
 * Returns a copy of the given list.
 */
struct list_int* clone(const struct list_int* list,Abstract_allocator prv_alloc) {
if (list==NULL) return NULL;
list_int* result=new_list_int(list->n,NULL,prv_alloc);
list=list->next;
list_int* tmp=result;
while (list!=NULL) {
   tmp->next=new_list_int(list->n,NULL,prv_alloc);
   tmp->next->next=NULL;
   list=list->next;
   tmp=tmp->next;
}

return result;
}


/**
 * Allocates, initializes and returns an integer array that contains
 * the elements of the given list. '*size' is set to the size of this
 * array. Note that passing an empty list will return NULL.
 */
int* dump(struct list_int* list,int *size,Abstract_allocator prv_alloc) {
*size=0;
if (list==NULL) return NULL;
struct list_int* tmp=list;
/* We count the number of elements */
while (tmp!=NULL) {
   (*size)++;
   tmp=tmp->next;
}
int* result=(int*)malloc_cb((*size)*sizeof(int),prv_alloc);
if (result==NULL) {
   fatal_alloc_error("dump");
}
tmp=list;
for (int i=0;i<(*size);i++) {
   result[i]=tmp->n;
   tmp=tmp->next;
}
return result;
}


/**
 * Tries to remove the first occurrence of the value 'n' in
 * the given list. Returns 1 if the element was there; 0 otherwise.
 */
int remove(int n,struct list_int** l,Abstract_allocator prv_alloc) {
struct list_int* tmp;
if (*l==NULL) return 0;
if ((*l)->n==n) {
   tmp=*l;
   *l=(*l)->next;
   free_cb(tmp,prv_alloc);
   return 1;
}
tmp=*l;
while (tmp->next!=NULL && tmp->next->n!=n) {
   tmp=tmp->next;
}
if (tmp->next==NULL) return 0;
struct list_int* tmp2=tmp->next;
tmp->next=tmp2->next;
free_cb(tmp2,prv_alloc);
return 1;
}

} // namespace unitex

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

#include <stdlib.h>
#include "List_int.h"
#include "Error.h"


/**
 * Allocates, initializes and returns a new int list element.
 */
struct list_int* new_list_int(int value,struct list_int* next) {
struct list_int* l;
l=(struct list_int*)malloc(sizeof(struct list_int));
if (l==NULL) {
   fatal_error("Not enough memory in new_list_int\n");
}
l->n=value;
l->next=next;
return l;
}


/**
 * Allocates, initializes and returns a new int list element.
 */
struct list_int* new_list_int(int value) {
return new_list_int(value,NULL);
}


/**
 * Frees a whole int list.
 */
void free_list_int(struct list_int* head) {
struct list_int* tmp;
while (head!=NULL) {
   tmp=head;
   head=head->next;
   free(tmp);
}
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
struct list_int* sorted_insert(int value,struct list_int* l) {

register struct list_int* tmp;
struct list_int* tmp2;
struct list_int* last = NULL;

if (l==NULL)  /* empty list */
   return new_list_int(value);

for (tmp=l; tmp!=NULL; tmp=tmp->next) {
  if (value==tmp->n) /* is in list */
     return l;
  if (value<tmp->n) { /* smaller than element tmp */
    tmp2=new_list_int(value,tmp);
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
tmp2=new_list_int(value);
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
struct list_int* head_insert(int value,struct list_int* old_head) {
struct list_int* new_head=new_list_int(value);
new_head->next=old_head;
return new_head;
}


/**
 * Returns 1 if the given value is in the list; 0 otherwise.
 */
int is_in_list(int value,struct list_int* l) {
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
int equal_list_int(struct list_int* a,struct list_int* b) {
if (a==NULL) {
   if (b==NULL) return 1;
   else return 0;
}
if (b==NULL) {
   return 0;
}
if (a->n!=b->n) {
   return 0;
}
return equal_list_int(a->next,b->next);
}


/**
 * This function sums the elements of the list and returns it as an hash code.
 */
unsigned int hash_list_int(struct list_int* list) {
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
int length(struct list_int* list) {
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
struct list_int* destructive_sorted_merge(struct list_int* a,struct list_int* b) {
struct list_int* tmp;
while (b!=NULL) {
   a=sorted_insert(b->n,a);
   tmp=b;
   b=b->next;
   free(tmp);
}
return a;
}


/**
 * This function puts in 'a' the merge of 'a' and 'b'. The merge result 
 * is returned. 'b' is not modified.
 */
struct list_int* sorted_merge(struct list_int* a,struct list_int* b) {
struct list_int* tmp;
while (b!=NULL) {
   a=sorted_insert(b->n,a);
   tmp=b;
   b=b->next;
}
return a;
}



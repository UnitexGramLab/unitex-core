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

#ifndef _LIST_H_
#define _LIST_H_


typedef struct list_node {

  void * data;
  struct list_node * next;

} list_node;


typedef struct list_t {

  list_node * first;
  int size;

} list_t;



list_t * list_new() {

  list_t * res = (list_t *) xmalloc(sizeof(list_t));

  res->first = NULL;
  res->size  = 0;

  return res;
}


void list_delete(list_t * list) {

  list_node * it;

  while (list->first) {
    it = list->first->next;
    free(list->first);
    list->first = it;
  }
  free(list);
}


void list_push_front(list_t * list, void * data) {

  list_node * it = (list_node *) xmalloc(sizeof(list_node));
  it->data = data;

  it->next = list->first;
  list->first = it;
  list->size++;
}


void * list_front(list_t * list) {

  if (list->first == NULL) { return NULL; }
  return list->first->data;
}


#endif

 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "FIFO.h"
#include "Error.h"


/**
 * Allocates, initializes and returns a new FIFO.
 */
struct fifo* new_fifo() {
struct fifo* f=(struct fifo*)malloc(sizeof(struct fifo));
if (f==NULL) {
   fatal_error("Not enough memory in new_fifo\n");
}
f->input=NULL;
f->output=NULL;
return f;
}

/**
 * Allocates, initializes and returns a new FIFO list.
 */
struct fifo_list* new_fifo_list(struct any value) {
struct fifo_list* l=(struct fifo_list*)malloc(sizeof(struct fifo_list));
if (l==NULL) {
   fatal_error("Not enough memory in new_fifo_list\n");
}
l->value=value;
l->next=NULL;
return l;
}


/**
 * Frees the internal linked list of a FIFO.

 * WARNING: if some elements have pointer values, these pointers
 * are not freed, so that it is the responsability of the user to free this
 * memory.
 */
void free_fifo_list(struct fifo_list* list) {
struct fifo_list* tmp;
while (list!=NULL) {
   tmp=list;
   list=list->next;
   free(tmp);
}
}


/**
 * Frees a FIFO. If it still contains elements, its linked list is freed.
 * 
 * WARNING: if the remaining elements have pointer values, these pointers
 * are not freed, so that it is the responsability of the user to free this
 * memory.
 */
void free_fifo(struct fifo* f) {
if (f==NULL) {
   return;
}
free_fifo_list(f->output);
free(f);
}


/**
 * Returns a non null value if the given FIFO is empty.
 */
int is_empty(struct fifo* f) {
return (f->output==NULL);
}


/**
 * Puts a generic value in the given FIFO.
 */
void put_any(struct fifo* f,struct any value) {
if (f->input==NULL) {
   /* If the FIFO was empty, we add a new element and we make
    * output points on it */
   f->input=new_fifo_list(value);
   f->output=f->input;
   return;
}
/* Otherwise, we put the element in the input side of the list */
f->input->next=new_fifo_list(value);
/* And we update the input */
f->input=f->input->next;
}


/** 
 * Takes a generic value from the given FIFO, or raises a fatal error
 * if the FIFO is empty.
 */
struct any take_any(struct fifo* f) {
if (f->output==NULL) {
   fatal_error("Empty FIFO in take\n");
}
struct fifo_list* tmp=f->output;
/* We get the value */
struct any value=tmp->value;
/* And we remove the element from the FIFO */
f->output=f->output->next;
tmp->next=NULL;
free_fifo_list(tmp);
if (f->output==NULL) {
   /* If we have emptied the FIFO, we must update it */
   f->input=NULL;
}
return value;
}


/**
 * Puts a pointer value in the given FIFO.
 */
void put_ptr(struct fifo* f,void* value) {
struct any tmp;
tmp._ptr=value;
put_any(f,tmp);
}


/** 
 * Takes a pointer value from the given FIFO, or raises a fatal error
 * if the FIFO is empty.
 */
void* take_ptr(struct fifo* f) {
struct any tmp=take_any(f);
return tmp._ptr;
}


/**
 * Puts an integer pointer value in the given FIFO.
 */
void put_int(struct fifo* f,int value) {
struct any tmp;
tmp._int=value;
put_any(f,tmp);
}


/** 
 * Takes an integer value from the given FIFO, or raises a fatal error
 * if the FIFO is empty.
 */
int take_int(struct fifo* f) {
struct any tmp=take_any(f);
return tmp._int;
}


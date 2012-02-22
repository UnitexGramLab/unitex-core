/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "List_pointer.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates, initializes and returns a pointer list cell.
 */
struct list_pointer* new_list_pointer(void* pointer,struct list_pointer* next,Abstract_allocator prv_alloc) {
struct list_pointer* p=(struct list_pointer*)malloc_cb(sizeof(struct list_pointer),prv_alloc);
if (p==NULL) {
   fatal_alloc_error("new_list_pointer");
}
p->pointer=pointer;
p->next=next;
return p;
}


/**
 * Frees the whole memory associated to the given list. We free the pointers
 * using the given 'free_pointer' function, if not NULL.
 */
void free_list_pointer(struct list_pointer* list,void (*free_pointer)(void*),Abstract_allocator prv_alloc) {
struct list_pointer* tmp;
while (list!=NULL) {
   tmp=list;
   list=list->next;
   if (free_pointer!=NULL) free_pointer(tmp->pointer);
   free_cb(tmp,prv_alloc);
}
}


/**
 * Frees the memory associated to the given list, but not the pointers it
 * contains.
 */
void free_list_pointer(struct list_pointer* list,Abstract_allocator prv_alloc) {
free_list_pointer(list,NULL,prv_alloc);
}

} // namespace unitex

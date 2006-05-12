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

#include "Context.h"
#include "Error.h"


/**
 * Allocates, initializes and returns a new context.
 */
struct context* new_context(unsigned char context_mode,
							int continue_position,
							unichar* stack,
							int stack_pointer,
							struct liste_num** l,
							int n_matches,
							struct context* next) {
struct context* c=(struct context*)malloc(sizeof(struct context));
if (c==NULL) {
	fatal_error("Not enough memory in new_context\n");
}
c->context_mode=context_mode;
c->continue_position=continue_position;
if (stack==NULL) {
	fatal_error("NULL stack error in new_context\n");
}
u_strcpy(c->stack,stack);
c->stack_pointer=stack_pointer;
c->list_of_matches=l;
c->number_of_matches=n_matches;
c->next=next;
return c;
}


/**
 * Remove the first element of the given context list and returns
 * the remaining list.
 */
struct context* remove_context(struct context* list) {
if (list==NULL) {
	return NULL;
}
struct context* tmp=list->next;
free(list);
return tmp;
}


/**
 * Frees the given context list.
 */
void free_context_list(struct context* list) {
while (list!=NULL) {
	list=remove_context(list);
}
}


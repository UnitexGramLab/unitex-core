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

struct context* new_context(unsigned char context_mode,
                            int continue_position,
                            unichar* stack,
                            int stack_pointer,
                            struct liste_num** l,
                            int n_matches,
                            struct context* next) {
struct context* c=(struct context*)malloc(sizeof(struct context));
if (c==NULL) {
   fatal_error("malloc error in new_context",1);
}
c->contextMode=context_mode;
c->continue_position=continue_position;
if (stack==NULL) {
   fatal_error("NULL stack error in new_context",1);
}
u_strcpy(c->stack,stack);
c->stack_pointer=stack_pointer;
c->list_of_matches=l;
c->n_matches=n_matches;
c->next=next;
return c;
}



struct context* remove_context(struct context* c) {
if (c==NULL) return NULL;
struct context* tmp=c->next;
free(c);
return tmp;
}


void free_context_list(struct context* c) {
while (c!=NULL) {
   c=remove_context(c);
}
}


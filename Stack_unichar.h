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

#ifndef Stack_unicharH
#define Stack_unicharH

#include "Unicode.h"

/**
 * This structure represents a stack of unicode characters.
 */
struct stack_unichar {
   unichar* stack;
   /* Pointer on the top element, -1 if the stack is empty */
   int stack_pointer;
   /* Maximum capacity of the stack */
   int capacity;
};


struct stack_unichar* new_stack_unichar(int);
void free_stack_unichar(struct stack_unichar*);
int is_empty(struct stack_unichar*);
void empty(struct stack_unichar* stack);
int is_full(struct stack_unichar*);
void push(struct stack_unichar*,unichar);
void push_array(struct stack_unichar*,const unichar*,unsigned int);
unichar pop(struct stack_unichar*);

#endif

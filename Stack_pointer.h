/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef Stack_pointerH
#define Stack_pointerH


/**
 * This structure represents a stack of pointers.
 */
struct stack_pointer {
   void** stack;
   /* Pointer on the top element, -1 if the stack is empty */
   int stack_pointer_m;
   /* Maximum capacity of the stack */
   int capacity;
};


struct stack_pointer* new_stack_pointer(int);
void free_stack_pointer(struct stack_pointer*);
int is_empty(const struct stack_pointer*);
void empty(struct stack_pointer* stack);
int is_full(const struct stack_pointer*);
void push(struct stack_pointer*,void*);
void* pop(struct stack_pointer*);

#endif

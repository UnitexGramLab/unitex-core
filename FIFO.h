/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef FIFOH
#define FIFOH

#include "Any.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This is the generic linked list used to encode FIFOs.
 */
struct fifo_list {
   struct any value;
   struct fifo_list* next;
};


/**
 * This structure is used to handle generic FIFO. We represent it with a
 * linked list as follows:
 *
 * output -->  ......... --> input --> NULL
 *
 */
struct fifo {
   struct fifo_list* input;
   struct fifo_list* output;
};


struct fifo* new_fifo();
void free_fifo(struct fifo*);
int is_empty(struct fifo*);

void put_any(struct fifo*,struct any);
struct any take_any(struct fifo*);

void put_ptr(struct fifo*,void*);
void* take_ptr(struct fifo*);

void put_int(struct fifo*,int);
int take_int(struct fifo*);

} // namespace unitex

#endif


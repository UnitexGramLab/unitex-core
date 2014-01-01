/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef List_pointerH
#define List_pointerH

#include "AbstractAllocator.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This is a simple structure for manipulating pointer lists.
 */
struct list_pointer {
   void* pointer;
   struct list_pointer* next;
};


struct list_pointer* new_list_pointer(void*,struct list_pointer*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_list_pointer(struct list_pointer*,void (*)(void*),Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_list_pointer(struct list_pointer*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);

} // namespace unitex

#endif


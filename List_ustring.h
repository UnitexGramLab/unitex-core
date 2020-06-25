/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef List_ustringH
#define List_ustringH

#include "Unicode.h"
#include "AbstractAllocator.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

// list pointer
typedef struct list_ustring* list_ustring_ptr;
// string compare function between a Unicode and C string
typedef int (*unichar_char_comparator)(const unichar* lhs, const char* rhs);
// string compare function between two Unicode strings
typedef int (*unichar_unichar_comparator)(const unichar* lhs, const unichar* rhs);

/**
 * This is a simple structure for manipulating unicode string lists.
 */
struct list_ustring {
   unichar* string;
   struct list_ustring* next;
};

struct list_ustring* new_list_ustring(const char*,struct list_ustring*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct list_ustring* new_list_ustring(const unichar*,struct list_ustring*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct list_ustring* new_list_ustring(const char*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct list_ustring* new_list_ustring(const unichar*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_list_ustring(struct list_ustring*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_list_ustring_element(struct list_ustring*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct list_ustring* sorted_insert(const unichar*,struct list_ustring*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct list_ustring* head_insert(const unichar*,struct list_ustring*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct list_ustring* insert_at_end_of_list(const char*,struct list_ustring*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct list_ustring* insert_at_end_of_list(const unichar*,struct list_ustring*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct list_ustring* insert_at_end_of_list_with_latest(const unichar*,struct list_ustring*,struct list_ustring**,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct list_ustring* insert_at_end_of_list_with_latest(const char*,struct list_ustring*,struct list_ustring**,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
int is_in_list(const unichar*, const struct list_ustring*, unichar_unichar_comparator compare = unitex::u_strcmp);
int is_in_list(const char*, const struct list_ustring*, unichar_char_comparator compare = unitex::u_strcmp);
int equal(const struct list_ustring*, const struct list_ustring*, unichar_unichar_comparator compare = unitex::u_strcmp);
struct list_ustring* clone(const struct list_ustring*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
int length(const struct list_ustring*);
} // namespace unitex

#endif


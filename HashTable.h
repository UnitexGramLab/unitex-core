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

#ifndef HashTableH
#define HashTableH

#include "Any.h"
#include "AbstractAllocator.h"

namespace unitex {

/* Values used to set the behavior if the key is not in the table */
#define HT_INSERT_IF_NEEDED 0
#define HT_DONT_INSERT 1

/* Values used to indicate if a key was created or not during a 'get_value'
 * operation */
#define HT_KEY_ADDED 0
#define HT_KEY_ALREADY_THERE 1


/**
 * This linked list is used to represent the list of elements that are
 * associated to a cell of the hash table in case of collision. 'value' is
 * the value associated to the element. It is the responsability of the user
 * to know if 'value' must must interpreted as an int, a pointer, etc.
 * 
 * Note that the order of the elements in the list is undefined and may vary
 * when resizing the hash table.
 */
struct hash_list {
   union {
      /* Keys can be either integers or pointers */
      int int_key;
      void* ptr_key;
   };
   struct hash_list* next;
   struct any value;
};

typedef unsigned int (*HASH_FUNCTION)(const void*);
typedef int (*EQUAL_FUNCTION)(const void*,const void*);
typedef void (*FREE_FUNCTION)(void*);
typedef void* (*KEYCOPY_FUNCTION)(const void*);


/**
 * This structure represents a generic hash table used to associate values to elements.
 * The elements can be either int or pointer.
 */
struct hash_table {
   struct hash_list** table;
   /* The capacity is the maximum number of elements of the element array */
   unsigned int capacity;
   /* This is the actual number of element in the hash table (not in the array,
    * since one cell of the array can contain several elements). */
   int number_of_elements;
   /* When number_of_elements>(capacity*ratio), we must resize the table and rehash all its
    * elements. This value must be between 0 and 1. By default, the ratio is 0.75. */
   float ratio;
   /* The hash function that returns a positive integer for a given element. A NULL
    * hash function indicates that keys are integers. */
   HASH_FUNCTION hash;
   /* The equal function must return a non null value if the two elements are
    * identical; 0 otherwise. */
   EQUAL_FUNCTION equal;
   /* The function to use for freeing a key of the hash table */
   FREE_FUNCTION free_key;
   /* The function to use for freeing the pointers contained in the hash list cell any values */
   FREE_FUNCTION free_ptr_value;
   /* The function to use to duplicate the given key when it has to be inserted
    * in the table. This is useful when we look for a key that may already be in the table:
    * in such a case, we don't want to malloc a key that finally will have to be freed. */
   KEYCOPY_FUNCTION keycopy;
   Abstract_allocator allocator_hash_list;
};


struct hash_table* new_hash_table(int,float,HASH_FUNCTION,EQUAL_FUNCTION,FREE_FUNCTION,FREE_FUNCTION,KEYCOPY_FUNCTION);
struct hash_table* new_hash_table(HASH_FUNCTION,EQUAL_FUNCTION,FREE_FUNCTION,FREE_FUNCTION,KEYCOPY_FUNCTION);
struct hash_table* new_hash_table(int,float);
struct hash_table* new_hash_table();
void free_hash_table(struct hash_table*);
void clear_hash_table(struct hash_table*);
struct any* get_value(struct hash_table*,void*,int,int*);
struct any* get_value(struct hash_table*,int,int,int*);
struct any* get_value(struct hash_table*,void*,int);
struct any* get_value(struct hash_table*,int,int);
void free_any_ptr(void*);

} // namespace unitex

#endif


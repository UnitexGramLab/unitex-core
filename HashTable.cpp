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

#include "HashTable.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define DEFAULT_HASH_SIZE 1024
#define DEFAULT_RATIO 0.75f


/**
 * set the capacity of hash table, verify we have a power of two
 */
static void set_hash_capacity(struct hash_table* h,unsigned int capacity_set) {
	h->capacity = 1;
	while (h->capacity < capacity_set)
		h->capacity *= 2;
}


/**
 * Allocates, initializes and return a new hash table for pointer elements.
 */
struct hash_table* new_hash_table(int capacity,float ratio,HASH_FUNCTION hash,
                                  EQUAL_FUNCTION equal,FREE_FUNCTION free_key,
                                  FREE_FUNCTION free_ptr_value,KEYCOPY_FUNCTION keycopy) {
if (ratio<=0 || ratio >=1) {
   fatal_error("Invalid ratio in new_hash_table\n");
}
struct hash_table* h=(struct hash_table*)malloc(sizeof(struct hash_table));
if (h==NULL) {
   fatal_alloc_error("new_hash_table");
}
set_hash_capacity(h,capacity);
h->ratio=ratio;
h->table=(struct hash_list**)malloc(((size_t)(h->capacity))*sizeof(struct hash_list*));
if (h->table==NULL) {
   fatal_alloc_error("new_hash_table");
}
/* We don't forget to initialize the table */
for (int i=0;i<(int)(h->capacity);i++) {
   h->table[i]=NULL;
}
if (hash==NULL) {
   fatal_error("NULL hash function error in new_hash_table\n");
}
h->hash=hash;
if (equal==NULL) {
   fatal_error("NULL equal function error in new_hash_table\n");
}
h->equal=equal;
if (free_key==NULL) {
   fatal_error("NULL free function error in new_hash_table\n");
}
h->free_key=free_key;
h->free_ptr_value=free_ptr_value;
h->keycopy=keycopy;
h->number_of_elements=0;
h->allocator_hash_list=create_abstract_allocator("new_hash_table",
                                                 AllocatorCreationFlagAutoFreePrefered | AllocatorFreeOnlyAtAllocatorDelete,
                                                 sizeof(struct hash_list),NULL);
return h;
}


/**
 * Allocates, initializes and return a new hash table for pointer elements with
 * default ratio and capacity.
 */
struct hash_table* new_hash_table(int capacity,HASH_FUNCTION hash,EQUAL_FUNCTION equal,
                                  FREE_FUNCTION free_key,FREE_FUNCTION free_ptr_value,
                                  KEYCOPY_FUNCTION keycopy) {
return new_hash_table(capacity,DEFAULT_RATIO,hash,equal,free_key,free_ptr_value,keycopy);
}


/**
 * Allocates, initializes and return a new hash table for pointer elements with
 * default ratio and capacity.
 */
struct hash_table* new_hash_table(HASH_FUNCTION hash,EQUAL_FUNCTION equal,
                                  FREE_FUNCTION free_key,FREE_FUNCTION free_ptr_value,
                                  KEYCOPY_FUNCTION keycopy) {
return new_hash_table(DEFAULT_HASH_SIZE,DEFAULT_RATIO,hash,equal,free_key,free_ptr_value,keycopy);
}


/**
 * Allocates, initializes and return a new hash table for integer elements.
 */
struct hash_table* new_hash_table(int capacity,float ratio) {
if (ratio<=0 || ratio >=1) {
   fatal_error("Invalid ratio in new_hash_table\n");
}
struct hash_table* h=(struct hash_table*)malloc(sizeof(struct hash_table));
if (h==NULL) {
   fatal_alloc_error("new_hash_table");
}
set_hash_capacity(h,capacity);
h->ratio=ratio;
h->table=(struct hash_list**)malloc(((size_t)(h->capacity))*sizeof(struct hash_list*));
if (h->table==NULL) {
   fatal_alloc_error("new_hash_table");
}
/* We don't forget to initialize the table */
for (int i=0;i<(int)(h->capacity);i++) {
   h->table[i]=NULL;
}
h->hash=NULL;
h->equal=NULL;
h->free_key=NULL;
h->free_ptr_value=NULL;
h->number_of_elements=0;
h->allocator_hash_list=create_abstract_allocator("new_hash_table",
                                                 AllocatorCreationFlagAutoFreePrefered | AllocatorFreeOnlyAtAllocatorDelete,
                                                 sizeof(struct hash_list),NULL);
return h;
}


/**
 * Allocates, initializes and return a new hash table for integer elements,
 * with default capacity and ratio.
 */
struct hash_table* new_hash_table(int capacity) {
return new_hash_table(capacity,DEFAULT_RATIO);
}


/**
 * Allocates, initializes and return a new hash table for integer elements,
 * with default capacity and ratio.
 */
struct hash_table* new_hash_table() {
return new_hash_table(DEFAULT_HASH_SIZE,DEFAULT_RATIO);
}


/**
 * Allocates, initializes and returns a new hash_list with an integer key.
 */
struct hash_list* new_hash_list(int key,struct hash_list* next,struct hash_table* h) {
struct hash_list* list=(struct hash_list*)malloc_cb(sizeof(struct hash_list),h->allocator_hash_list);
if (list==NULL) {
   fatal_alloc_error("new_hash_list");
}
list->int_key=key;
list->next=next;
return list;
}


/**
 * Allocates, initializes and returns a new hash_list with a pointer key.
 */
struct hash_list* new_hash_list(KEYCOPY_FUNCTION keycopy,void* key,struct hash_list* next,struct hash_table* h) {
struct hash_list* list=(struct hash_list*)malloc_cb(sizeof(struct hash_list),h->allocator_hash_list);
if (list==NULL) {
   fatal_alloc_error("new_hash_list");
}
if (keycopy!=NULL) {
   key=keycopy(key);
}
list->ptr_key=key;
list->next=next;
return list;
}


static inline void free_hash_list_only_key(struct hash_list* list, void(*free_key)(void*)) {
while (list != NULL) {
  struct hash_list* tmp = list;
  list = list->next;
  if (free_key != NULL) {
    /* If we have pointer elements, we must free them */
    free_key(tmp->ptr_key);
  }
}
}

/**
 * Frees a list of elements.
 */
static inline void free_hash_list(struct hash_list* list,void (*free_key)(void*),void (*free_ptr_value)(void*),
                                  int free_hash_list_struct,struct hash_table* h) {
if ((free_ptr_value == NULL) && (!free_hash_list_struct)) {
   free_hash_list_only_key(list, free_key);
   return;
}
while (list!=NULL) {
   struct hash_list* tmp=list;
   list=list->next;
   if (free_key!=NULL) {
      /* If we have pointer elements, we must free them */
      free_key(tmp->ptr_key);
   }
   if (free_ptr_value!=NULL) {
      /* If we have pointer values in elements, we must free them */
      free_ptr_value(tmp->value._ptr);
   }
   if (free_hash_list_struct) {
       free_cb(tmp,h->allocator_hash_list);
   }
}
}
 
 
/**
 * Frees all the memory associated to the given hash table.
 */
void free_hash_table(struct hash_table* h) {
if (h==NULL) return;
int free_hash_list_struct=(get_allocator_cb_flag(h->allocator_hash_list) & AllocatorGetFlagAutoFreePresent) ? 0 : 1;
if (h->table!=NULL) {
   /* This case should always happen, but we never know... */
   for (unsigned int i=0;i<h->capacity;i++) {
      free_hash_list(h->table[i],h->free_key,h->free_ptr_value,free_hash_list_struct,h);
   }
   free(h->table);
}
close_abstract_allocator(h->allocator_hash_list);
free(h);
}


/**
 * Removes all the elements of the given hash table.
 */
void clear_hash_table(struct hash_table* h) {
if (h==NULL) return;
int free_hash_list_struct=(get_allocator_cb_flag(h->allocator_hash_list) & AllocatorGetFlagAutoFreePresent) ? 0 : 1;

if ((h->table!=NULL) && (h->free_ptr_value==NULL) && (h->free_key==NULL) && (!free_hash_list_struct)) {
   memset(h->table, 0, sizeof(struct hash_list*)*h->capacity);
}
else
if (h->table!=NULL) {
   /* This case should always happen, but we never know... */
   for (unsigned int i=0;i<h->capacity;i++) {
      free_hash_list(h->table[i],h->free_key,h->free_ptr_value,free_hash_list_struct,h);
      h->table[i]=NULL;
   }
}

close_abstract_allocator(h->allocator_hash_list);
h->allocator_hash_list=create_abstract_allocator("new_hash_table",
                                                 AllocatorCreationFlagAutoFreePrefered | AllocatorFreeOnlyAtAllocatorDelete,
                                                 sizeof(struct hash_list),NULL);
h->number_of_elements=0;
}


/**
 * This function doubles the capacity of the hash table, rehashing all its
 * elements.
 */
static void resize(struct hash_table* h) {
unsigned int i;
int new_cell_index;
struct hash_list* tmp;
unsigned int new_capacity=h->capacity*2;
unsigned int new_capacity_and_mask=new_capacity-1;
struct hash_list** new_table=(struct hash_list**)malloc(new_capacity*sizeof(struct hash_list*));
if (new_table==NULL) {
   fatal_alloc_error("resize");
}
/* We don't forget to initialize the new table */
for (i=0;i<new_capacity;i++) {
   new_table[i]=NULL;
}
/* Now, we rehash all the elements in order to store them in the new table */
for (i=0;i<h->capacity;i++) {
   while (h->table[i]!=NULL) {
      /* We get the hash_list element that is at the head */
      tmp=h->table[i];
      /* We remove it from the original hash_list */
      h->table[i]=h->table[i]->next;
      /* Then, we compute its new hash code */
      if (h->hash==NULL) {
         /* If keys are integers */
         new_cell_index=tmp->int_key & new_capacity_and_mask;
      }
      else {
         /* If keys are pointers */
         new_cell_index=h->hash(tmp->ptr_key) & new_capacity_and_mask;
      }
      /* And we insert it in the new table */
      tmp->next=new_table[new_cell_index];
      new_table[new_cell_index]=tmp;
   }
}
/* Now, the original table has been emptied, so that we can free it and replace
 * it by the new one */
free(h->table);
h->table=new_table;
h->capacity=new_capacity;
}


/**
 * This function looks in the given list if it contains the given pointer
 * key. In that case, it returns a pointer on its associated value; NULL
 * otherwise.
 */
struct any* get_value_(void* key,struct hash_list* list,struct hash_table* h) {
while (list!=NULL) {
   if (h->equal(key,list->ptr_key)) {
      /* If we have found an equal key, we return a pointer on its value */
      return &(list->value);
   }
   /* Otherwise, we look the next element */
   list=list->next;
}
/* If we have not found the key, we return NULL */
return NULL;
}


/**
 * This function inserts a key in the given table, resizing it if needed.
 * It returns a pointer on the virgin value associated to the key. No
 * test is performed to check if the key is already present in the table.
 * 
 * Note that it is the responsability of the caller to initialize
 * properly this value.
 */
static struct any* insert_key(struct hash_table* h,void* key) {
if (h->number_of_elements>=(h->ratio*h->capacity)) {
   /* If necessary, we resize the hash table */
   resize(h);
}
/* Then we add the new key */
int cell_index=h->hash(key) & ( h->capacity-1);
h->table[cell_index]=new_hash_list(h->keycopy,key,h->table[cell_index],h);
h->number_of_elements++;
return &(h->table[cell_index]->value);
}


/**
 * Looks for the value associated to the given key in the given hash table.
 * If 'insert_policy' is set to HT_INSERT_IF_NEEDED, the key will be added
 * to the table if necessary. In that case, (*ret) will contain HT_KEY_ADDED
 * and the returned value will be a virgin one to be set by the caller.
 * If the key is already in the table, (*ret) is set to HT_KEY_ALREADY_THERE.
 * 
 * Returns a pointer on the struct any that contains the value, or NULL if
 * the key is not in the table and if 'insert_policy' is set to HT_DONT_INSERT.
 * 
 * Note that you can associate a NULL value to a key. In that case, the returned
 * struct any will have its _ptr field set to NULL.
 */

struct any* get_value(struct hash_table* h,void* key,int insert_policy,int *ret) {
if (h==NULL) {
   fatal_error("NULL hash table error in get_value\n");
}
if (key==NULL) {
   fatal_error("NULL key error in get_value\n");
}
if (h->hash==NULL) {
   fatal_error("NULL hash function error in get_value\n");
}
int cell_index=h->hash(key) & (h->capacity-1);
struct any* value=get_value_(key,h->table[cell_index],h);
if (value!=NULL) {
   /* If the key is in the table we have finished */
   (*ret)=HT_KEY_ALREADY_THERE;
   return value;
}
/* If the key is not in the table, we must act according to 'insert_policy' */
if (insert_policy==HT_DONT_INSERT) {
   return NULL;
}
/* If we must insert, then we do it */
value=insert_key(h,key);
(*ret)=HT_KEY_ADDED;
return value;
}


/**
 * This function looks in the given list if it contains the given integer
 * key. In that case, it returns a pointer on its associated value; NULL
 * otherwise.
 */
struct any* get_value_(int key,struct hash_list* list) {
while (list!=NULL) {
   if (key==list->int_key) {
      /* If we have found an equal key, we return a pointer on its value */
      return &(list->value);
   }
   /* Otherwise, we look the next element */
   list=list->next;
}
/* If we have not found the key, we return NULL */
return NULL;
}


/**
 * This function inserts a key in the given table, resizing it if needed.
 * It returns a pointer on the virgin value associated to the key. No
 * test is performed to check if the key is already present in the table.
 * 
 * Note that it is the responsability of the caller to initialize
 * properly this value.
 */
static struct any* insert_key(struct hash_table* h,int key) {
if (h->number_of_elements>=(h->ratio*h->capacity)) {
   /* If necessary, we resize the hash table */
   resize(h);
}
/* Then we add the new key */
int cell_index=key & (h->capacity-1);
h->table[cell_index]=new_hash_list(key,h->table[cell_index],h);
h->number_of_elements++;
return &(h->table[cell_index]->value);
}


/**
 * Looks for the value associated to the given key in the given hash table.
 * If 'insert_policy' is set to HT_INSERT_IF_NEEDED, the key will be added
 * to the table if necessary. In that case, (*ret) will contain HT_KEY_ADDED
 * and the returned value will be a virgin one to be set by the caller.
 * If the key is already in the table, (*ret) is set to HT_KEY_ALREADY_THERE.
 * 
 * Returns a pointer on the struct any that contains the value, or NULL if
 * the key is not in the table and if 'insert_policy' is set to HT_DONT_INSERT.
 * 
 * Note that you can associate a NULL value to a key. In that case, the returned
 * struct any will have its _ptr field set to NULL.
 */
struct any* get_value(struct hash_table* h,int key,int insert_policy,int *ret) {
if (h==NULL) {
   fatal_error("NULL hash table error in get_value\n");
}
int cell_index=key & (h->capacity-1);
struct any* value=get_value_(key,h->table[cell_index]);
if (value!=NULL) {
   /* If the key is the table we have finished */
   (*ret)=HT_KEY_ALREADY_THERE;
   return value;
}
/* If the key is not in the table, we must act according to 'insert_policy' */
if (insert_policy==HT_DONT_INSERT) {
   return NULL;
}
/* If we must insert, then we do it */
value=insert_key(h,key);
(*ret)=HT_KEY_ADDED;
return value;
}


/**
 * Looks for the value associated to the given key in the given hash table.
 * If 'insert_policy' is set to HT_INSERT_IF_NEEDED, the key will be added
 * to the table if necessary. In that case, the returned value will be a virgin
 * one to be set by the caller.
 * 
 * Returns a pointer on the struct any that contains the value, or NULL if
 * the key is not in the table and if 'insert_policy' is set to HT_DONT_INSERT.
 * 
 * Note that you can associate a NULL value to a key. In that case, the returned
 * struct any will have its _ptr field set to NULL.
 */

struct any* get_value(struct hash_table* h,void* key,int insert_policy) {
int i;
return get_value(h,key,insert_policy,&i);
}


/**
 * Looks for the value associated to the given key in the given hash table.
 * If 'insert_policy' is set to HT_INSERT_IF_NEEDED, the key will be added
 * to the table if necessary. In that case, the returned value will be a virgin
 * one to be set by the caller.
 * 
 * Returns a pointer on the struct any that contains the value, or NULL if
 * the key is not in the table and if 'insert_policy' is set to HT_DONT_INSERT.
 * 
 * Note that you can associate a NULL value to a key. In that case, the returned
 * struct any will have its _ptr field set to NULL.
 */

struct any* get_value(struct hash_table* h,int key,int insert_policy) {
int i;
return get_value(h,key,insert_policy,&i);
}


/**
 * Frees a pointer any structure applying 'free' to the pointer field.
 */
void free_any_ptr(void* any_ptr) {
if (any_ptr==NULL) return;
struct any* a=(struct any*)any_ptr;
free(a->_ptr);
free(a);
}

} // namespace unitex

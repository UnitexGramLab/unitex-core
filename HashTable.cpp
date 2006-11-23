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
#include "HashTable.h"
#include "Error.h"


#define DEFAULT_HASH_SIZE 1024
#define DEFAULT_RATIO 0.75f


/**
 * Allocates, initializes and return a new hash table for pointer elements.
 */
struct hash_table* new_hash_table(int capacity,float ratio,unsigned int (*hash)(void*),
                                  int (*equal)(void*,void*),void (*free_element)(void*)) {
if (ratio<=0 || ratio >=1) {
   fatal_error("Invalid ratio in new_hash_table\n");
}
struct hash_table* h=(struct hash_table*)malloc(sizeof(struct hash_table));
if (h==NULL) {
   fatal_error("Not enough memory in new_hash_table\n");
}
h->capacity=capacity;
h->ratio=ratio;
h->table=(struct hash_list**)malloc(capacity*sizeof(struct hash_list*));
if (h->table==NULL) {
   fatal_error("Not enough memory in new_hash_table\n");
}
/* We don't forget to initialize the table */
for (int i=0;i<capacity;i++) {
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
if (free==NULL) {
   fatal_error("NULL free function error in new_hash_table\n");
}
h->free=free_element;
h->number_of_elements=0;
return h;
}


/**
 * Allocates, initializes and return a new hash table for pointer elements with
 * default ratio and capacity.
 */
struct hash_table* new_hash_table(unsigned int (*hash)(void*),int (*equal)(void*,void*),
                                  void (*free_element)(void*)) {
return new_hash_table(DEFAULT_HASH_SIZE,DEFAULT_RATIO,hash,equal,free_element);
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
   fatal_error("Not enough memory in new_hash_table\n");
}
h->capacity=capacity;
h->ratio=ratio;
h->table=(struct hash_list**)malloc(capacity*sizeof(struct hash_list*));
if (h->table==NULL) {
   fatal_error("Not enough memory in new_hash_table\n");
}
/* We don't forget to initialize the table */
for (int i=0;i<capacity;i++) {
   h->table[i]=NULL;
}
h->hash=NULL;
h->equal=NULL;
h->free=NULL;
h->number_of_elements=0;
return h;
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
struct hash_list* new_hash_list(int key,struct hash_list* next) {
struct hash_list* list=(struct hash_list*)malloc(sizeof(struct hash_list));
if (list==NULL) {
   fatal_error("Not enough memory in new_hash_list\n");
}
list->int_key=key;
list->next=next;
return list;
}


/**
 * Allocates, initializes and returns a new hash_list with a pointer key.
 */
struct hash_list* new_hash_list(void* key,struct hash_list* next) {
struct hash_list* list=(struct hash_list*)malloc(sizeof(struct hash_list));
if (list==NULL) {
   fatal_error("Not enough memory in new_hash_list\n");
}
list->ptr_key=key;
list->next=next;
return list;
}


/**
 * Frees a list of elements.
 */
void free_hash_list(struct hash_list* list,void (*free_element)(void*)) {
while (list!=NULL) {
   struct hash_list* tmp=list;
   list=list->next;
   if (free_element!=NULL) {
      /* If we have pointer elements, we must free them */
      free_element(tmp->ptr_key);
   }
   free(tmp);
}
}
 
 
/**
 * Frees all the memory associated to the given hash table.
 */
void free_hash_table(struct hash_table* h) {
if (h==NULL) return;
if (h->table!=NULL) {
   /* This case should always happen, but we never know... */
   for (unsigned int i=0;i<h->capacity;i++) {
      free_hash_list(h->table[i],h->free);
   }
}
free(h);
}


/**
 * Removes all the elements of the given hash table.
 */
void clear_hash_table(struct hash_table* h) {
if (h==NULL) return;
if (h->table!=NULL) {
   /* This case should always happen, but we never know... */
   for (unsigned int i=0;i<h->capacity;i++) {
      free_hash_list(h->table[i],h->free);
      h->table[i]=NULL;
   }
}
h->number_of_elements=0;
}


/**
 * This function doubles the capacity of the hash table, rehashing all its
 * elements.
 */
void resize(struct hash_table* h) {
unsigned int i;
int new_cell_index;
struct hash_list* tmp;
unsigned int new_capacity=h->capacity*2;
struct hash_list** new_table=(struct hash_list**)malloc(new_capacity*sizeof(struct hash_list*));
if (new_table==NULL) {
   fatal_error("Not enough memory in resize\n");
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
         new_cell_index=tmp->int_key%new_capacity;
      }
      else {
         /* If keys are pointers */
         new_cell_index=h->hash(tmp->ptr_key)%new_capacity;
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
struct any* insert_key(struct hash_table* h,void* key) {
if (h->number_of_elements>=(h->ratio*h->capacity)) {
   /* If necessary, we resize the hash table */
   resize(h);
}
/* Then we add the new key */
int cell_index=h->hash(key)%h->capacity;
h->table[cell_index]=new_hash_list(key,h->table[cell_index]);
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
int cell_index=h->hash(key)%h->capacity;
struct any* value=get_value_(key,h->table[cell_index],h);
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
 * This function looks in the given list if it contains the given integer
 * key. In that case, it returns a pointer on its associated value; NULL
 * otherwise.
 */
struct any* get_value_(int key,struct hash_list* list,struct hash_table* h) {
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
struct any* insert_key(struct hash_table* h,int key) {
if (h->number_of_elements>=(h->ratio*h->capacity)) {
   /* If necessary, we resize the hash table */
   resize(h);
}
/* Then we add the new key */
int cell_index=key%h->capacity;
h->table[cell_index]=new_hash_list(key,h->table[cell_index]);
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
int cell_index=key%h->capacity;
struct any* value=get_value_(key,h->table[cell_index],h);
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


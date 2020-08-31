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

#include <stdlib.h>
#include "OptimizedTfstTagMatching.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define DEFAULT_CACHE_SIZE 4096


/**
 * This list structure is used to store a match result.
 */
struct element {
   int result;
   /* old_XXX = positions before matching */
   int old_pos_kr_fst2_tag;
   int old_pos_kr_tfst_tag;
   /* new_XXX = positions after matching */
   int new_pos_kr_fst2_tag;
   int new_pos_kr_tfst_tag;
   struct element* next;
};


static int estimate_number_of_tfs_tags(int);
static struct element* new_element(int,int,int,int,int,struct element*);
static void free_element_list(struct element*);
static struct element** new_element_array(int);
static void free_element_array(struct element**,int);


/**
 * Creates a cache with default size.
 */
LocateTfstTagMatchingCache* new_LocateTfstTagMatchingCache(int n_fst2_tags) {
return new_LocateTfstTagMatchingCache(DEFAULT_CACHE_SIZE,n_fst2_tags);
}


/**
 * Creates a cache with a size that is estimated with the formula:
 *
 * f(n_sentences*4):
 *    4 is empirically a good value
 *    f(x) = closer power of 2 greater than x
 */
LocateTfstTagMatchingCache* new_LocateTfstTagMatchingCache(int n_sentences,int n_fst2_tags) {
int size=estimate_number_of_tfs_tags(n_sentences);
LocateTfstTagMatchingCache* cache=(LocateTfstTagMatchingCache*)malloc(sizeof(LocateTfstTagMatchingCache));
if (cache==NULL) {
   fatal_alloc_error("new_LocateTfstTagMatchingCache");
}
cache->n_fst2_tags=n_fst2_tags;
cache->table=new_hash_table(size,0.75f,(HASH_FUNCTION)hash_unichar,(EQUAL_FUNCTION)u_equal,
      (FREE_FUNCTION)free,NULL,(KEYCOPY_FUNCTION)keycopy);
cache->elements=new_vector_ptr(size);
cache->cached_tfst_tags_index=NULL;
return cache;
}


/**
 * Estimates the number of TfstTag.
 */
static int estimate_number_of_tfs_tags(int n) {
n=n*4;
int j=1;
while (j<n) {
   j=j*2;
}
return j;
}

/**
 * Frees the cache.
 */
void free_LocateTfstTagMatchingCache(LocateTfstTagMatchingCache* cache) {
if (cache==NULL) return;
free_hash_table(cache->table);
/* We free all the element arrays */
for (int i=0;i<cache->elements->nbelems;i++) {
   free_element_array((struct element**)(cache->elements->tab[i]),cache->n_fst2_tags);
}
free_vector_ptr(cache->elements,NULL);
free(cache->cached_tfst_tags_index);
free(cache);
}


/**
 * We (re)creates the array used to cache tfst tags indices, because
 * hashing them is expensive.
 */
void prepare_cache_for_new_sentence(LocateTfstTagMatchingCache* cache,int n_tfst_tags) {
free(cache->cached_tfst_tags_index);
cache->cached_tfst_tags_index=(int*)malloc(n_tfst_tags*sizeof(int));
if (cache->cached_tfst_tags_index==NULL) {
   fatal_alloc_error("prepare_cache_for_new_sentence");
}
for (int i=0;i<n_tfst_tags;i++) {
   cache->cached_tfst_tags_index[i]=-1;
}
}


/**
 * Builds a new element.
 */
static struct element* new_element(int result,int old_pos_fst2,int old_pos_tfst,
                                   int new_pos_fst2,int new_pos_tfst,struct element* next) {
struct element* e=(struct element*)malloc(sizeof(struct element));
if (e==NULL) {
   fatal_alloc_error("new_element");
}
e->result=result;
e->old_pos_kr_fst2_tag=old_pos_fst2;
e->old_pos_kr_tfst_tag=old_pos_tfst;
e->new_pos_kr_fst2_tag=new_pos_fst2;
e->new_pos_kr_tfst_tag=new_pos_tfst;
e->next=next;
return e;
}


/**
 * Frees a whole element list.
 */
static void free_element_list(struct element* list) {
struct element* tmp;
while (list!=NULL) {
   tmp=list->next;
   free(list);
   list=tmp;
}
}


/**
 * Builds an element array, initialized with NULL values.
 */
static struct element** new_element_array(int n) {
struct element** array=(struct element**)calloc(n,sizeof(struct element*));
if (array==NULL) {
   fatal_alloc_error("new_element_array");
}
return array;
}


/**
 * Frees all the memory associated to the given element array.
 */
static void free_element_array(struct element** array,int size) {
if (array==NULL) return;
for (int i=0;i<size;i++) {
   free_element_list(array[i]);
}
free(array);
}


/**
 * Returns the index of the given tag in the cache.
 */
static int get_tfst_tag_index(unichar* s,LocateTfstTagMatchingCache* cache) {
int ret;
struct any* value=get_value(cache->table,s,HT_INSERT_IF_NEEDED,&ret);
if (ret==HT_KEY_ADDED) {
   /* If we had not already this tag, we insert it */
   value->_int=cache->elements->nbelems;
   vector_ptr_add(cache->elements,new_element_array(cache->n_fst2_tags));
}
return value->_int;
}


/**
 * Returns the cached result for the given match, or UNKNOWN_MATCH_STATUS
 * if it has not been computed yet.
 */
int get_cached_result(LocateTfstTagMatchingCache* cache,
                      unichar* tfst_tag,int fst2_tag_index,
                      int tfst_tag_index,
                      int old_pos_fst2,int old_pos_tfst,
                      int *new_pos_fst2,int *new_pos_tfst) {
int tfst_tag_cache_index=cache->cached_tfst_tags_index[tfst_tag_index];
if (tfst_tag_cache_index==-1) {
   tfst_tag_cache_index=get_tfst_tag_index(tfst_tag,cache);
   cache->cached_tfst_tags_index[tfst_tag_index]=tfst_tag_cache_index;
}
struct element** array=(struct element**)(cache->elements->tab[tfst_tag_cache_index]);
if (array==NULL) {
   fatal_error("Unexpected NULL array in get_cached_result\n");
}
struct element* list=array[fst2_tag_index];
while (list!=NULL) {
   if (list->old_pos_kr_fst2_tag==old_pos_fst2 && list->old_pos_kr_tfst_tag==old_pos_tfst) {
      /* If we have a result for the match, we return it */
      (*new_pos_fst2)=list->new_pos_kr_fst2_tag;
      (*new_pos_tfst)=list->new_pos_kr_tfst_tag;
      return list->result;
   }
   list=list->next;
}
return UNKNOWN_MATCH_STATUS;
}


/**
 * Sets the given result in the cache. This function is not supposed to be called
 * if the result is already in the cache.
 */
void set_cached_result(LocateTfstTagMatchingCache* cache,
                      int tfst_tag_index,int fst2_tag_index,
                      int old_pos_fst2,int old_pos_tfst,int result,
                      int new_pos_fst2,int new_pos_tfst) {
int tfst_tag_cache_index=cache->cached_tfst_tags_index[tfst_tag_index];
if (tfst_tag_cache_index==-1) {
   fatal_error("Unexpected -1 tfst tag cache index in set_cached_result\n");
}
struct element** array=(struct element**)(cache->elements->tab[tfst_tag_cache_index]);
if (array==NULL) {
   fatal_error("Unexpected NULL array in set_cached_result\n");
}
array[fst2_tag_index]=new_element(result,old_pos_fst2,old_pos_tfst,new_pos_fst2,new_pos_tfst,array[fst2_tag_index]);
}

} // namespace unitex

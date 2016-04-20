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

#ifndef OptimizedTfstTagMatchingH
#define OptimizedTfstTagMatchingH

#include "HashTable.h"
#include "Vector.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This library provides a data structure to optimize TfstTag/Fst2Tag matching,
 * using a cache.
 */

/* See comment in LocateTfst_lib.h */
#define UNKNOWN_MATCH_STATUS -1

typedef struct {
   struct hash_table* table;
   vector_ptr* elements;
   int n_fst2_tags;
   int* cached_tfst_tags_index;
} LocateTfstTagMatchingCache;


LocateTfstTagMatchingCache* new_LocateTfstTagMatchingCache(int);
LocateTfstTagMatchingCache* new_LocateTfstTagMatchingCache(int,int);
void free_LocateTfstTagMatchingCache(LocateTfstTagMatchingCache*);

void prepare_cache_for_new_sentence(LocateTfstTagMatchingCache*,int);
int get_cached_result(LocateTfstTagMatchingCache* cache,
                      unichar* tfst_tag,int fst2_tag_index,int tfst_tag_index,
                      int old_pos_fst2,int old_pos_tfst,int *new_pos_fst2,int *new_pos_tfst);
void set_cached_result(LocateTfstTagMatchingCache* cache,
                      int tfst_tag_index,int fst2_tag_index,
                      int old_pos_fst2,int old_pos_tfst,int new_pos_fst2,int new_pos_tfst,int result);


} // namespace unitex

#endif

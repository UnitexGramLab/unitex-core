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

#ifndef CompoundWordHashTableH
#define CompoundWordHashTableH

/**
 * 
 * This library provides functions for manipulating compound words seen as
 * token sequences. Such compound words are stored into a hash table.
 * 
 * Author: Alexis Neme
 * Modified by: Sébastien Paumier
 */
 
#include "Unicode.h"

#define TCT_HASH_SIZE 16384
#define TCT_DEFAULT_HASH_BLOCK_SIZE 4

/**
 * This structure represents a cell of a hash table. It is made of a token
 * array with a size of 'size'. 'length' represents
 * the number of cells that are actually used in this token array.
 * 'token_array' contains integer sequences that represent compound word.
 * Each sequence is ended by -1 followed by the priority of the compound word.
 * For instance, if a block contains sequences for the compound words
 * "black box" and "black humor", respectively with priorities 2 and 3, the
 * token array may be:
 * 
 * (40,2,13,-1,2,40,2,125,-1,3)
 */
struct tct_hash_block {
   int length;
   int size;
   int* token_array;
};


/**
 * This structure represents a hash table containing compound words.
 */
struct tct_hash {
   /* The size of the table, i.e. the size of the 'hash_blocks' array */
   int size;
   
   /* The array of hash blocks */
   struct tct_hash_block*	hash_blocks;
   int* token_array_base_memory_alloc;
   int token_array_standard_base_memory_nb_item_for_each_block;
};


struct tct_hash* new_tct_hash();  
struct tct_hash* new_tct_hash(int,int);  
void free_tct_hash(struct tct_hash*);
int was_already_in_tct_hash(int*,struct tct_hash*,int);
int build_token_sequence(unichar*,struct text_tokens*,int*);
void add_tct_token_sequence(int* token_seq,struct tct_hash* hash_table,int priority);

#endif

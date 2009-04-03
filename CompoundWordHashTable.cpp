 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "CompoundWordHashTable.h"
#include "Error.h"
#include "Text_tokens.h"
#include "Tokenization.h"
#include "List_ustring.h"
#include "LocateConstants.h"



/**
 * This function initializes the blocks of the given hash table.
 */
void initialize_hash_blocks(struct tct_hash* hash_table,int block_size) {
struct tct_hash_block* block;
int size=hash_table->size;
for (int i=0;i<size;i++) {
   block=(struct tct_hash_block*)malloc(sizeof(struct tct_hash_block));
   if (block==NULL) {
      fatal_alloc_error("initialize_hash_blocks");
   }
   block->size=block_size;
   block->token_array=(int*)malloc(block_size*sizeof(int));
   if (block->token_array==NULL) {
      fatal_alloc_error("initialize_hash_blocks");
   }
   block->length=0;
   hash_table->hash_blocks[i]=block;
}
}


/**
 * Allocates, initializes and returns a new hash table.
 * 'size' is the number of entry in the hash table.
 */
struct tct_hash* new_tct_hash(int size,int tct_hash_block_size) {
struct tct_hash* hash_table;
hash_table=(struct tct_hash*)malloc(sizeof(struct tct_hash));
if (hash_table==NULL) {
   fatal_alloc_error("new_tct_hash");
}
hash_table->size=size;
hash_table->hash_blocks=(struct tct_hash_block**)malloc(size*sizeof(struct tct_hash_block*));
if (hash_table->hash_blocks==NULL) {
   fatal_alloc_error("new_tct_hash");
}
/* We initialize the block array */
initialize_hash_blocks(hash_table,tct_hash_block_size);
return hash_table;
}


/**
 * Allocates, initializes and returns a new hash table, using the default
 * sizs.
 */
struct tct_hash* new_tct_hash() {
return new_tct_hash(TCT_HASH_SIZE,TCT_DEFAULT_HASH_BLOCK_SIZE);
}


/**
 * Frees all the memory associated to the given table.
 */
void free_tct_hash(struct tct_hash* hash_table){
if (hash_table==NULL) return;
for (int j=0;j<hash_table->size;j++) {
  free(hash_table->hash_blocks[j]->token_array);
  free(hash_table->hash_blocks[j]);
}
free(hash_table->hash_blocks);
free(hash_table);
}


/**
 * Returns 1+the length in token of the compound word coded by
 * the given token array.
 * Example: tct_length(574,1,5,1,575,-1) is 6 (we count the -1)
 */
int tct_length(int* token_sequence) {
int i;
for (i=0;token_sequence[i]!=-1;i++);
return i+1; 
}


/**
 * Returns the hash code associated to the given token sequence.
 * For convenience reasons, the code is given modulo the size
 * of the hash table.
 */
int tct_hash(int* token_sequence,int hash_table_size){
unsigned long int hash_code=0;
for (int i=0;token_sequence[i]!=-1;i++) {
   hash_code=hash_code+(token_sequence[i]<<i)+1357;
}
return abs(hash_code)%hash_table_size;
}





/**
 * Resizes the token array of the given block so that the token array can
 * contain 'new_number_of_elements' elements. The function doubles the size
 * of the array as many times as needed. If the array has already a sufficient
 * capacity, the function does nothing.
 */
void realloc_tct_hash_block(struct tct_hash_block* block,int new_number_of_elements) {
int factor=1;
while (block->size*factor < new_number_of_elements) {
   factor=factor*2;
}
block->size=block->size*factor;
if (factor==1) return;
block->token_array=(int*)realloc(block->token_array,block->size*sizeof(int));
if (block->token_array==NULL) {
   fatal_alloc_error("realloc_tct_hash_block");
}
}


/**
 * Adds a token sequence ended by -1 to the given hash table.
 * An integer representing the given priority is added after the -1 in
 * the token sequence stored in the hash table.
 * 
 * Example: (574,1,5,1,575,-1) 2 => the (574,1,5,1,575,-1,2) array will be stored
 */
void add_tct_token_sequence(int* token_seq,struct tct_hash* hash_table,int priority) {
int hash_code=tct_hash(token_seq,hash_table->size);
struct tct_hash_block* block=hash_table->hash_blocks[hash_code];
int old_length=block->length;
/* We compute the number of integers required to encode the given token
 * sequence. We add 1 because of the priority. */
int size=tct_length(token_seq)+1;
/* We enlarge the token array if needed */
realloc_tct_hash_block(block,size+block->length);
/* Then we copy the token sequence at the end of the block's token array */
int i;
int* tokens=block->token_array;
for (i=0;token_seq[i]!=-1;i++) {
   tokens[old_length+i]=token_seq[i];
}
/* We add the ending -1 and the priority */
tokens[old_length+i]=-1;
i++;
tokens[old_length+i]=priority;
i++;
/* Finally, we update the length of the block's token array */
block->length=block->length+i;
}


/**
 * Looks for the given token sequence inside the token array of the
 * given block. Returns -1 if the sequence is not found; the offset
 * of its first token otherwise.
 */
int tct_match(struct tct_hash_block* block,int* token_sequence) {
int i=0;
int j=0;
int block_length=block->length;
int* tokens=block->token_array; 
while (i<block_length) {
   j=0;
   while (i+j<block_length && tokens[i+j]==token_sequence[j]) {
      if (token_sequence[j]==-1) return i; 
      j++;
   }
   /* If the current sequence in the token array has not matched,
    * we look for the next sequence. The -2 is there because we want
    * to put 'i' on the first cell after the -1 and priority of the previous
    * token sequence. We increase 'i' of 1 so that i-2 is always >=0 */
   i=i+1;
   do {
      i++;
   } while (i<block_length && tokens[i-2]!=-1);
} 
return -1;
}


/**
 * Looks for the given token sequence in the hash table. Returns 0 if the
 * compound word is not found; its priority (1, 2 or 3) otherwise.
 */
int was_already_in_tct_hash(int* token_sequence,struct tct_hash* hash_table,int priority){
int hash_code=tct_hash(token_sequence,hash_table->size);
struct tct_hash_block* block=hash_table->hash_blocks[hash_code];
/* We look for the token sequence in the token array of the correct block */
int offset=tct_match(block,token_sequence);
if (offset!=-1) {
   /* If the token sequence is present, we look for its priority */
   int offset_priority=offset+tct_length(token_sequence);
   return block->token_array[offset_priority];
}
/* Otherwise, we add the token sequence to the hash table */
add_tct_token_sequence(token_sequence,hash_table,priority);
return 0;
}


/**
 * This function takes a compound word and tokenizes it according to
 * the given text tokens. The result is an integer sequence that is
 * stored in 'token_sequence'. Each integer represents a token number,
 * and the sequence is ended by -1.
 * 
 * Example: "sans raison" may be turned into (121,1,1643,-1)
 * 
 * WARNING: every token of the compound word is supposed to be present
 *          in the given text tokens.
 */
void build_token_sequence(unichar* compound_word,struct text_tokens* tokens,int* token_sequence) {
struct list_ustring* list=tokenize(compound_word,WORD_BY_WORD_TOKENIZATION,NULL);
struct list_ustring* tmp;
int i=0;
while (list!=NULL) {
   token_sequence[i]=get_token_number(list->string,tokens);
   if (token_sequence[i]==-1) {
      fatal_error("Unknown token in build_token_sequence\n");
   }
   i++;
   tmp=list;
   list=list->next;
   free_list_ustring_element(tmp);
}
/* We put the final -1 */
token_sequence[i]=-1;
}


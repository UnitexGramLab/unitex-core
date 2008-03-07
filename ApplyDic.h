 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef ApplyDicH
#define ApplyDicH

#include "Unicode.h"
#include "Alphabet.h"
#include "Text_tokens.h"
#include "DELA.h"
#include "CompoundWordHashTable.h"
#include "Buffer.h"
#include "BitArray.h"


/**
 * This structure is used to represent a list of offsets in the current
 * .bin dictionary. For each offset, 'content' contains the sequence that
 * leads to the node with this offset. For instance, if we are in the .bin 
 * dictionary at the position 5487 corresponding to "black-eyed", we will have:
 * 
 * offset=5487   content="black-eyed"
 * 
 * This is used to cache information when looking for compound words.
 * 'next' is the next element in the list.
 */
struct offset_list {
  int offset;
  unichar* content;
  struct offset_list* next;
};


/**
 * This structure represents a list of transitions from one tree node to another.
 * 'token_number' is the number of the token that tags the current transition and
 * 'node' is its destination node. 'next' is the next element in the list.
 */
struct word_transition {
   int token_number;
   struct word_struct* node;
   struct word_transition* next;
};


/**
 * This structure represents a tree node. 'trans' is the list of
 * its output branches. 'list' represent the offsets in the current
 * .bin dictionary.
 */
struct word_struct {
   struct offset_list* list;
   struct word_transition* trans;
};


/**
 * This structure is used to store information about the structure
 * of words. element[i] is a tree that provides information about words 
 * whose first token has the number i. N is the size of the array.
 */
struct word_struct_array {
   struct word_struct** element;
   int N;
};


/**
 * This structure is used to store various information needed for the
 * application of dictionaries to a text.
 */
struct dico_application_info {
   /* Info about the text files */
   FILE* text_cod;
   struct text_tokens* tokens;
   FILE* dlf;
   FILE* dlc;
   FILE* err;
   /* The buffer to use to read the text.cod file */
   struct buffer* buffer;
   /* The alphabet to use */
   Alphabet* alphabet;
   /* The dictionary to use */
   unsigned char* bin;
   struct INF_codes* inf;
   /* Information about the recognized words:
    * - word_array is a tree that contains information about the
    *   structure of words
    * - part_of_a_word is an array used to know if a token is part of
    *   a word or not
    * - simple_word is an array used to know if a token has already been
    *   matched as a simple word or not, and if it is the case, we use this array
    *   to know the priority of the dictionary that matched this token
    * - n_occurrences is an array used to count the number of occurrences of each token
    * - tct_h is a hash table that contains the recognized compound words
    */
   struct word_struct_array* word_array;
   struct bit_array* part_of_a_word;
   struct bit_array* simple_word;
   int* n_occurrences;
   struct tct_hash* tct_h;
   /* Total number of simple, compound and unknown word occurrences in the text
    * WARNING: these are NOT the number of lines of the dlf, dlc and err files */
   int SIMPLE_WORDS;
   int COMPOUND_WORDS;
   int UNKNOWN_WORDS;
};

#define BUFFER_SIZE 200000


struct dico_application_info* init_dico_application(struct text_tokens*,FILE*,FILE*,FILE*,FILE*,Alphabet*);
void dico_application(char*,struct dico_application_info*,int);
void free_dico_application(struct dico_application_info*);
void count_token_occurrences(struct dico_application_info*);
void save_unknown_words(struct dico_application_info*);

/* Added by Alexis Neme: FST Functionality of Dico */
int merge_dic_locate_results(struct dico_application_info*,char*,int);

#endif

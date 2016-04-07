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

#ifndef ApplyDicH
#define ApplyDicH

#include "Unicode.h"
#include "Alphabet.h"
#include "Text_tokens.h"
#include "DELA.h"
#include "AbstractDelaLoad.h"
#include "CompoundWordHashTable.h"
#include "BitArray.h"
#include "LocateMatches.h"
#include "CompressedDic.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

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
  /* The base and output fields are required when we deal with .bin2 dictionaries */
  int base;
  unichar* output;
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
   ABSTRACTMAPFILE* map_text_cod;
   const int* text_cod_buf;
   int text_cod_size_nb_int;
   struct text_tokens* tokens;
   U_FILE* dlf;
   U_FILE* dlc;
   U_FILE* err;
   U_FILE* tags_err;
   U_FILE* morpho;
   char tags_ind[FILENAME_MAX];
   /* Used to know the current dic being applied when we are in simplified mode */
   char dic_name[FILENAME_MAX];
   /* The buffer to use to read the text.cod file */
   //struct buffer* buffer;
   /* The alphabet to use */
   Alphabet* alphabet;
   /* The dictionary to use */
   Dictionary* d;
#if 0
   const unsigned char* bin;
   const struct INF_codes* inf;
#endif

   struct BIN_free_info bin_free;
   struct INF_free_info inf_free;
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
   /* part_of_a_word is used to mark tokens that have been matched by dlf/dlc */
   struct bit_array* part_of_a_word;
   /* part_of_a_word2 is used to mark tokens that have been matched by tags.ind entries */
   struct bit_array* part_of_a_word2;
   struct bit_array* simple_word;
   int* n_occurrences;

   /* tct_h is a hash table used to associate a priority to each token
    * sequence matched when applying a .bin dictionary. Keys are sequences
    * of token numbers. */
   struct tct_hash* tct_h;
   /* tct_h_tags_ind is a hash table used to associate a priority to each token
    * sequence matched when applying a .fst2 dictionary.
    * IMPORTANT: unlike tct_h, keys are couple of offsets [start;end], because
    *            .fst2 matching are contextual */
   struct tct_hash* tct_h_tags_ind;

   /* Total number of simple, compound and unknown word occurrences in the text
    * WARNING: these are NOT the number of lines of the dlf, dlc and err files */
   int SIMPLE_WORDS;
   int COMPOUND_WORDS;
   int UNKNOWN_WORDS;

   /* The following field define a pointer array used to store tag sequences in order
    * to sort them before saving them into the "tags.ind" file */
   struct match_list** tag_sequences;
   int n_tag_sequences;
   int tag_sequences_capacity;
   VersatileEncodingConfig vec;
};


struct dico_application_info* init_dico_application(struct text_tokens*,U_FILE*,U_FILE*,U_FILE*,U_FILE*,
                                                    U_FILE*,const char*,const char*,Alphabet*,
                                                    const VersatileEncodingConfig*);
int dico_application(const VersatileEncodingConfig*,const char*,struct dico_application_info*,int);
int dico_application_simplified(const VersatileEncodingConfig*,const unichar*,const char*,struct dico_application_info*);
void free_dico_application(struct dico_application_info*);
void count_token_occurrences(struct dico_application_info*);
void save_unknown_words(struct dico_application_info*);

/* Added by Alexis Neme: FST Functionality of Dico */
int merge_dic_locate_results(struct dico_application_info*,const char*,int,int);

void save_and_sort_tag_sequences(struct dico_application_info*);

} // namespace unitex

#endif

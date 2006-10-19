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

//---------------------------------------------------------------------------
#ifndef Dico_applicationH
#define Dico_applicationH
//---------------------------------------------------------------------------
#include "unicode.h"
#include "Alphabet.h"
#include "Text_tokens.h"
#include "DELA.h"
#include "Table_complex_token_hash.h"


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
    * - has_been_processed is an array used to know if a token has already been
    *   processed or not, and if it is the case, we use this array to know the priority
    *   of the dictionary that matched this token
    * - n_occurrences is an array used to count the number of occurrences of each token
    * - token_tree_root*/
   struct word_struct_array* word_array;
   unsigned char* part_of_a_word;
   unsigned char* has_been_processed;
   int* n_occurrences;
   struct tct_hash* tct_h;
   struct buffer* buffer;
};


#define BUFFER_SIZE 200000

extern struct word_struct_array* word_array;
extern FILE* DLF;
extern FILE* DLC;
extern FILE* ERR;
extern Alphabet* ALPH;
extern unsigned char* BIN;
extern struct INF_codes* INF;
extern FILE* TEXT;
extern struct text_tokens* TOK;
extern int buffer[BUFFER_SIZE];
//extern struct token_tree_node* token_tree_root;
extern unsigned char* part_of_a_word;
extern unsigned char* has_been_processed;
extern int COMPOUND_WORDS;
extern int* n_occur;
extern int WORDS_HAVE_BEEN_COUNTED;
extern int ERRORS;
extern int SIMPLE_WORDS;
extern int CURRENT_BLOCK;

extern struct tct_hash* tct_h;				// *** Alexis to access from Dico (for the FST)

void liberer_word_struct(struct word_struct*);
void dico_application(unsigned char*,struct INF_codes*,
                             FILE*,struct text_tokens*,Alphabet*,
                             FILE*,FILE*,FILE*,int);
void init_dico_application(struct text_tokens*,FILE* dlf,FILE* dlc,FILE* err,FILE* text,Alphabet* alph);
void free_dico_application();


void sauver_mots_inconnus();

// Added by Alexis Neme: FST Functionality of Dico

//  coumputing the  coumpounds forms
void set_part_of_a_word(int n,int priority);
int is_part_of_a_word(int n) ;
void set_has_been_processed(int n,int priority) ;
int token_has_been_processed(int n) ;


// Alexis - management of the Global Variable file DLF,DLC, ERR
void assign_text_DICO (FILE* dlf,FILE* dlc,FILE* err) ;
void free_text_DICO () ;

#endif

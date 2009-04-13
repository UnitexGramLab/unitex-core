 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef TfstH
#define TfstH

/**
 * This library provides tools for manipulating text automata.
 */

#include <stdio.h>
#include "Ustring.h"
#include "Vector.h"
#include "SingleGraph.h"

#define NO_SENTENCE_LOADED -1

/**
 * This structure represents a text automaton. This structure
 * is meant to manipulate one sentence automaton at a time.
 */
typedef struct {

   /* Number of sentences */
   int N;

   /* .tfst and .tind files */
   U_FILE* tfst;
   U_FILE* tind;

   /* Number of the current sentence */
   int current_sentence;

   /* The text of the current sentence */
   unichar* text;

   /* The tokens of the current sentence and their sizes in characters */
   vector_int* tokens;
   vector_int* token_sizes;

   /* Offsets of the current sentence in the text */
   int offset_in_tokens;
   int offset_in_chars;

   /* Current sentence automaton */
   SingleGraph automaton;

   /* The tags of the current sentence automaton */
   vector_ptr* tags;

} Tfst;



/**
 * Here are the types of transitions supported in a Tfst.
 */
typedef enum {
   /* The empty word transition */
   T_EPSILON,
   /* The normal transition */
   T_STD
} TfstTagType;



/**
 * Here is the description of a Tfst tag.
 */
typedef struct {
   /* Type of the transition */
   TfstTagType type;

   /*-- The following fields only concern tag transitions --*/

   /* The content of the tag */
   unichar* content;

   /* The boundings of the sentence area covered by the tag */
   int start_pos_token;
   int start_pos_char;
   int end_pos_token;
   int end_pos_char;

} TfstTag;

Tfst* new_Tfst(U_FILE* tfst,U_FILE* tind,int N);
Tfst* open_text_automaton(char* tfst);
void close_text_automaton(Tfst* tfst);
void load_sentence(Tfst* tfst,int n);
void save_current_sentence(Tfst* tfst,U_FILE* out_tfst,U_FILE* tind,unichar** tags,int n_tags);

TfstTag* new_TfstTag(TfstTagType);
void free_TfstTag(TfstTag*);
void TfstTag_to_string(TfstTag*,unichar*);

#endif


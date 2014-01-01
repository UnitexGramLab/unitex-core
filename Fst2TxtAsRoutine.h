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

#ifndef Fst2TxtAsRoutineH
#define Fst2TxtAsRoutineH

#include "Alphabet.h"
#include "Buffer.h"
#include "Fst2.h"
#include "AbstractFst2Load.h"
#include "Fst2Txt_TokenTree.h"
#include "LocateConstants.h"
#include "ParsingInfo.h"
#include "TransductionVariables.h"
#include "Unicode.h"
#include "Stack_unichar.h"
#include "Offsets.h"
#include "Vector.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define MAX_OUTPUT_LENGTH 10000


/**
 * This structure represents the parameters required by Fst2Txt.
 */
struct fst2txt_parameters {
   char* text_file;
   char* temp_file;
   char* fst_file;
   char* alphabet_file;

   U_FILE* f_input;
   U_FILE* f_output;
   Fst2* fst2;
   Alphabet* alphabet;

   OutputPolicy output_policy;
   TokenizationPolicy tokenization_policy;
   SpacePolicy space_policy;

   struct fst2txt_token_tree** token_tree;
   /* n_token_trees corresponds to the number of states in the fst2, but
    * we cache it here, in order to avoid problems if the fst2 is freed
    * before 'token_tree'. */
   int n_token_trees;
   Variables* variables;
   /* Here are the text buffer and the current origin in it */
   struct buffer* text_buffer;
   int current_origin;
   /* It's just a shortcut to the data array */
   unichar* buffer;
   /* This is the absolute offset of the first character in the buffer */
   int absolute_offset;
   /* Here are the stack and string used to deal with outputs */
   struct stack_unichar* stack;
   unichar output[MAX_OUTPUT_LENGTH];

   /* Used to know how long was the input that has been matched */
   int input_length;

   VersatileEncodingConfig vec;

   /* Offset management:
    * v_in_offsets=input offsets
    * f_out_offsets=file where to store outputs offsets
    * insertions=vector used to store insertion positions in MERGE mode. Note that we
    *            don't need it in REPLACE mode, since in REPLACE mode, we just have
    *            to consider a single replacement over the whole input sequence
    * current_insertions=the temp vector used when exploring graphs
    */
   vector_offset* v_in_offsets;
   vector_offset* v_out_offsets;
   U_FILE* f_out_offsets;
   vector_int* insertions;
   vector_int* current_insertions;
   int CR_shift;
   int new_absolute_origin;
   int last_offset_index;
};

struct fst2txt_parameters* new_fst2txt_parameters();
void free_fst2txt_parameters(struct fst2txt_parameters*);
int main_fst2txt(struct fst2txt_parameters*);

} // namespace unitex

#endif

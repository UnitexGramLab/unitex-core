 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef FST2TXTASROUTINE_H
#define FST2TXTASROUTINE_H

#include "Alphabet.h"
#include "Buffer.h"
#include "Fst2.h"
#include "Fst2Txt_TokenTree.h"
#include "LocateConstants.h"
#include "ParsingInfo.h"
#include "TransductionVariables.h"
#include "Unicode.h"


/**
 * This structure represents the parameters required by Fst2Txt.
 */
struct fst2txt_parameters {
   char* text_file;
   char* temp_file;
   char* fst_file;
   char* alphabet_file;

   FILE* input;
   FILE* output;
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
};

struct fst2txt_parameters* new_fst2txt_parameters();
void free_fst2txt_parameters(struct fst2txt_parameters*);
int main_fst2txt(struct fst2txt_parameters*);

#endif

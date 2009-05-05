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

#ifndef TextAutomatonH
#define TextAutomatonH


#include "Unicode.h"
#include "Text_tokens.h"
#include "Alphabet.h"
#include "DELA_tree.h"
#include "String_hash.h"
#include "NormalizationFst2.h"
#include "Matches.h"
#include "LanguageDefinition.h"

#define MAX_TOKENS_IN_SENTENCE 2000


void build_sentence_automaton(int*,int,struct text_tokens*,
                              struct DELA_tree*,
                              Alphabet*,U_FILE*,U_FILE*,int,int,
                              struct normalization_tree*,
                              struct match_list**,int,int,
                              language_t*);

void build_korean_sentence_automaton(int* buffer,int length,struct text_tokens* tokens,
                               Alphabet* alph,U_FILE* out_tfst,U_FILE* out_tind,
                               int sentence_number,
                               int we_must_clean,
                               int current_global_position_in_tokens,
                               int current_global_position_in_chars,
                               char* phrase_cod,char* jamoTable,
                               char* jamoFst2);

#endif

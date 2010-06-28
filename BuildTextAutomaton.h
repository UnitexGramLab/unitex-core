/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "LocateMatches.h"
#include "LanguageDefinition.h"
#include "Korean.h"
#include "HashTable.h"

#define MAX_TOKENS_IN_SENTENCE 2000


void build_sentence_automaton(int*,int,struct text_tokens*,
                              struct DELA_tree*,
                              Alphabet*,U_FILE*,U_FILE*,int,int,
                              struct normalization_tree*,
                              struct match_list**,int,int,
                              language_t*,Korean* korean,
                              struct hash_table* form_frequencies);

#endif

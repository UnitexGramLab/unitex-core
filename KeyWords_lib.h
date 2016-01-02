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

#ifndef KeyWords_libH
#define KeyWords_libH

#include "Unicode.h"
#include "Vector.h"
#include "FileEncoding.h"
#include "String_hash.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * We represent a keyword with a single or multi-word unit
 * sequence associated to a weight information.
 */
typedef struct keyword {
	int weight;
	unichar* sequence;
	char lemmatized;
	struct keyword* next;
} KeyWord;


KeyWord* new_KeyWord(double weight,unichar* sequence,KeyWord* next);
void free_KeyWord(KeyWord* k);
void free_KeyWord_list(KeyWord* k);
struct string_hash_ptr* load_tokens_by_freq(char* name,VersatileEncodingConfig* vec);
void load_compound_words(char* name,VersatileEncodingConfig* vec,
		struct string_hash_ptr* keywords);
void filter_non_letter_keywords(struct string_hash_ptr* keywords,Alphabet* alphabet);
void filter_keywords_with_dic(struct string_hash_ptr* keywords,char* name,
						VersatileEncodingConfig* vec,Alphabet* alphabet);
void merge_case_equivalent_unknown_words(struct string_hash_ptr* keywords,Alphabet* alphabet);
vector_ptr* sort_keywords(struct string_hash_ptr* keywords);
void dump_keywords(vector_ptr* keywords,U_FILE* f);
struct string_hash* compute_forbidden_lemmas(struct string_hash_ptr* keywords,unichar* code);
void remove_keywords_with_forbidden_lemma(struct string_hash_ptr* keywords,
					struct string_hash* lemmas);
} // namespace unitex

#endif


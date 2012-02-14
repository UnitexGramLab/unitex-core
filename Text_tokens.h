/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef Text_tokensH
#define Text_tokensH


#include "List_int.h"
#include "String_hash.h"
#include "Unicode.h"
#include "Alphabet.h"
#include "DELA.h"
#include "AbstractAllocator.h"
#include "LoadInf.h"
#include "CompressedDic.h"

namespace unitex {

/*
 * This is the structure that holds 
 *  - The strings that each token represent (unichar **token)
 *  - The biggest token id                  (int N)
 *  - The token id of the sentence marker   (int SENTENCE_MARKER)
 *  - The token id of the stop marker       (int STOP_MARKER)
 *  - The token id of the space             (int SPACE)
 */

struct text_tokens {
   unichar** token;
   int N;
   int SENTENCE_MARKER;
   int STOP_MARKER;
   int SPACE;
};


struct text_tokens* load_text_tokens(const VersatileEncodingConfig* vec,const char*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct string_hash* load_text_tokens_hash(const char*, const VersatileEncodingConfig* vec,int*,int*,int*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_text_tokens(struct text_tokens*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct list_int* get_token_list_for_sequence(const unichar*,const Alphabet*,struct string_hash*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
int get_token_number(const unichar*,struct text_tokens*);
int is_a_digit_token(const unichar* s);
void extract_semantic_codes_from_tokens(const struct string_hash*,struct string_hash*,Abstract_allocator prv_alloc);
void extract_semantic_codes_from_morpho_dics(Dictionary**,int,struct string_hash*,Abstract_allocator prv_alloc);

} // namespace unitex

//---------------------------------------------------------------------------
#endif



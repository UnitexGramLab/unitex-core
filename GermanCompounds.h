/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef GermanCompoundsH
#define GermanCompoundsH


#include "Unicode.h"
#include "Alphabet.h"
#include "DELA.h"
#include "String_hash.h"
#include "LoadInf.h"
#include "CompressedDic.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

struct german_word_decomposition {
   int n_parts;
   unichar decomposition[2000];
   unichar dela_line[2000];
};

struct german_word_decomposition_list {
   struct german_word_decomposition* element;
   struct german_word_decomposition_list* suivant;
};


void analyse_german_compounds(const Alphabet*,Dictionary*,U_FILE*,U_FILE*,U_FILE*,U_FILE*);
void check_valid_right_component_german(char*,const struct INF_codes*);
void check_valid_left_component_german(char*,const struct INF_codes*);
char check_valid_left_component_for_an_INF_line_german(const struct list_ustring*);
char check_valid_left_component_for_one_INF_code_german(const unichar*);
char check_valid_right_component_for_an_INF_line_german(const struct list_ustring*);
char check_valid_right_component_for_one_INF_code_german(const unichar*);
void analyse_german_word_list(Dictionary*,U_FILE*,U_FILE*,U_FILE*,U_FILE*,const char*,const char*,const Alphabet*);
int analyse_german_word(const unichar*,U_FILE*,U_FILE*,const char*,const char*,const Alphabet*,Dictionary*);
void get_first_sia_code_german(int,unichar*,const struct INF_codes*);

struct german_word_decomposition* new_german_word_decomposition();
void free_german_word_decomposition(struct german_word_decomposition*);
struct german_word_decomposition_list* new_german_word_decomposition_list();
void free_german_word_decomposition_list(struct german_word_decomposition_list*);

} // namespace unitex

#endif

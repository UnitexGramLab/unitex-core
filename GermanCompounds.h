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

#ifndef GermanCompoundsH
#define GermanCompoundsH


#include "Unicode.h"
#include "Alphabet.h"
#include "DELA.h"
#include "String_hash.h"


struct german_word_decomposition {
   int n_parts;
   unichar decomposition[2000];
   unichar dela_line[2000];
};

struct german_word_decomposition_list {
   struct german_word_decomposition* element;
   struct german_word_decomposition_list* suivant;
};


void analyse_german_compounds(Alphabet*,const unsigned char*,const struct INF_codes*,U_FILE*,U_FILE*,U_FILE*,U_FILE*);
void check_valid_right_component_german(char*,const struct INF_codes*);
void check_valid_left_component_german(char*,const struct INF_codes*);
char check_valid_left_component_for_an_INF_line_german(struct list_ustring*);
char check_valid_left_component_for_one_INF_code_german(unichar*);
char check_valid_right_component_for_an_INF_line_german(struct list_ustring*);
char check_valid_right_component_for_one_INF_code_german(unichar*);
void analyse_german_word_list(const unsigned char*,const struct INF_codes*,U_FILE*,U_FILE*,U_FILE*,U_FILE*,char*,char*,Alphabet*);
int analyse_german_word(unichar*,U_FILE*,U_FILE*,char*,char*,const struct INF_codes*,Alphabet*,const unsigned char*);
void get_first_sia_code_german(int,unichar*,const struct INF_codes*);

struct german_word_decomposition* new_german_word_decomposition();
void free_german_word_decomposition(struct german_word_decomposition*);
struct german_word_decomposition_list* new_german_word_decomposition_list();
void free_german_word_decomposition_list(struct german_word_decomposition_list*);
void explore_state_german(int,unichar*,int,unichar*,int,unichar*,unichar*,
      struct german_word_decomposition_list**,int,char*,char*,const struct INF_codes*,Alphabet*,const unsigned char*);

#endif

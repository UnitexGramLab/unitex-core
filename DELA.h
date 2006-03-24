 /*
  * Unitex
  *
  * Copyright (C) 2001-2005 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef DELAH
#define DELAH
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "String_hash.h"

// maximum size of a DELA line
#define DIC_LINE_SIZE 2000
// maximum size of a word (word form or lemma)
#define DIC_WORD_SIZE (DIC_LINE_SIZE/2)
// maximum number of semantic codes (Hum,Conc,z1,...) per line
#define MAX_SEMANTIC_CODES 20
// maximum number of flexional codes (ms,Kf,W,...) per line
#define MAX_FLEXIONAL_CODES 64
/* for languages with rich inflection this number must be considerably high,
   because words with (almost) no inflection and many homonymic forms may occur.
   e.g. Russian adjectives have max. 31 forms
        (long forms: 6*case * 4*gender/number + 2*animacy in acc.
         + 4 short forms + 1 comparative = 31)
        German adjectives have 147 forms
        ((4*case * 4*gender/number * 3 declension + 1 predicative) * 3 comparation)
        but for indeclinable adjectives (like "lila") there is no comparation,
        so 49 forms is the maximum.
*/
        

#define NOT_IN_DICTIONARY -1


struct dic_entry_ {
  unichar* inflected;
  unichar* lemma;
  unsigned char n_semantic_codes;  /* number of semantic codes
                                      the first may be the grammatical code */
  unsigned char n_flexional_codes; /* number of flexional codes */
  unichar* semantic_codes[MAX_SEMANTIC_CODES];
  unichar* flexional_codes[MAX_FLEXIONAL_CODES];
};

typedef struct dic_entry_ dic_entry;



struct token_list {
  unichar* token;
  struct token_list* suivant;
};


struct INF_codes {
   struct token_list** tab;
   int N;
};


int read_DELA_line(FILE*,unichar*);
dic_entry* tokenize_DELA_line(unichar*);
dic_entry* tokenize_tag_token(unichar*);
void get_compressed_line(dic_entry*,unichar*);
struct token_list* tokenize_compressed_info(unichar*);
void free_token_list(struct token_list*);
void uncompress_entry(unichar*,unichar*,unichar*);
struct INF_codes* load_INF_file(char*);
void free_INF_codes(struct INF_codes*);
unsigned char* load_BIN_file(char*);
void rebuild_dictionary(unsigned char*,struct INF_codes*,FILE*);
void tokenize_DELA_line_into_inflected_and_code(unichar*,unichar*,unichar*);
void extract_semantic_codes(char*,struct string_hash*);
void tokenize_DELA_line_into_3_parts(unichar*,unichar*,unichar*,unichar*);
void tokenize_tag_token_into_3_parts(unichar*,unichar*,unichar*,unichar*);
void check_DELAS_line(unichar*,FILE*,int,char*,struct string_hash*,struct string_hash*);
void check_DELAF_line(unichar*,FILE*,int,char*,struct string_hash*,struct string_hash*);
int warning_on_code(unichar*,unichar*);
int contains_unprotected_equal_sign(unichar*);
void replace_unprotected_equal_sign(unichar*,unichar);
void unprotect_equal_signs(unichar*);
void free_dic_entry(dic_entry*);
void tokenize_inflectional_codes(unichar*,int*,unichar**);
int check_tag_token(unichar*);

int dic_entry_contain_gram_code(dic_entry*,unichar*);
int dic_entry_contain_flex_code(dic_entry*,unichar*);

int get_INF_number(unichar*,unsigned char*,Alphabet*);

#endif


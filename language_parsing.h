 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef _LANG_PARSE_H_
#define _LANG_PARSE_H_

#include <stdio.h>

#include "unicode.h"

enum {
  TOK_STR = 0, TOK_ANGLE, TOK_NAME, TOK_POS, TOK_END, TOK_DISCR, TOK_FLEX, TOK_CAT, TOK_COMPLET, TOK_EQUAL, TOK_BLANK
};


typedef struct token_t {
  int type;
  unichar * str;
  token_t * next;
} token_t;


typedef struct tokens_list {
  token_t * tokens;
  struct tokens_list * next;
} tokens_list;



#define PART_FLEX  0
#define PART_DISCR 1
#define PART_CAT   2
#define PART_COMP  3
#define PART_NUM   4

typedef struct pos_section_t {

  unichar * name;
  tokens_list * parts[PART_NUM];
  struct pos_section_t * next;

} pos_section_t;


typedef struct language_tree_t {
  unichar   * name;
  pos_section_t * pos_secs;
} language_tree_t;




void token_dump(token_t * tok, FILE * f = stderr);
void tokens_dump(token_t * toks, FILE * f = stderr);
void tokens_list_dump(tokens_list * tlist, FILE * f = stderr);
void pos_section_dump(pos_section_t * sec, FILE * f = stderr);
void language_tree_dump(language_tree_t * tree, FILE * f = stderr);


language_tree_t * language_parse(FILE * f);
language_tree_t * language_parse(char * fname);

void language_tree_delete(language_tree_t * tree);

#endif

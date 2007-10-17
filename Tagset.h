 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef TagsetH
#define TagsetH

/**
 * This library provides tools for loading tagset definition files.
 * 
 * Author: Olivier Blanc
 */

#include <stdio.h>
#include "Unicode.h"


/**
 * Here are the possible token types.
 */
enum {
  TOK_STR=0, TOK_ANGLE, TOK_NAME, TOK_POS, TOK_END, TOK_DISCR, TOK_FLEX, TOK_CAT, TOK_COMPLET, TOK_EQUAL, TOK_BLANK, TOK_IGNORE
};


/**
 * This structure defines a token list.
 */
typedef struct token_t {
   /* Type of the token */
   int type;
   
   /* Token's content */
   unichar* str;
   
   /* Next token in the list */
   token_t* next;
} token_t;


/**
 * This is a list of token lists.
 */
typedef struct tokens_list {
   token_t* tokens;
   struct tokens_list* next;
} tokens_list;



#define PART_FLEX  0
#define PART_DISCR 1
#define PART_CAT   2
#define PART_COMP  3
#define PART_NUM   4


/**
 * This structure describes a POS definition list.
 */
typedef struct pos_section_t {
   /* POS name like "N" or "ADV" */
   unichar * name;
   
   /* This field indicates whether this POS can be ignored or not during elag operations */
   bool ignore;
   
   /* This array gives for each POS definition part the lines it contains, in the form
    * of token lists. */
   tokens_list* parts[PART_NUM];
   
   /* The next POS definition */
   struct pos_section_t * next;
} pos_section_t;


/**
 * This structure is used to load the content of the tagset definition file.
 */
typedef struct tagset_t {
   /* Language name */
   unichar* name;
   
   /* POSs defined in the tagset file */
   pos_section_t* pos_sections;
} tagset_t;



tagset_t* load_tagset(FILE*);
void free_tagset_t(tagset_t*);

#endif

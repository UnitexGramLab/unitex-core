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

#ifndef ParsingInfoH
#define ParsingInfoH

#include "Unicode.h"
#include "TransductionVariables.h"
#include "DicVariables.h"
#include "DELA.h"


/**
 * This structure is used to store information during the parsing of a text.
 * It represents the state of the parsing when arriving at the final state of
 * a subgraph. Such information is represented as a list.
 */
struct parsing_info {
   /* Current position in the text, i.e. position in the text when the
    * final state of the subgraph was reached. */
   int position;

   /* This field is used in the morphological mode to know where the
    * matches ends in the current token. -1 means "end of the token". */
   int pos_in_token;

   /* This field is used in the morphological mode to know the
    * number of the state pointed by the $> transition that ends the
    * morphological mode. -1 if not used. */
   int state_number;


   /* Content of the stack */
   unichar* stack;
   /* Stack pointer */
   int stack_pointer;

   /* A copy of the variable ranges */
   int* variable_backup;

   /* A copy of the DELAF entry variables */
   struct dic_variable* dic_variable_backup;

   /* In morphological mode, we may want to save the matching DELAF entry
    * when we have a pattern like <V:W>. To do that, we use this field to
    * save the address of a struct dela_entry. */
   struct dela_entry* dic_entry;

   /* The same as in locate_parameters */
   int left_ctx_shift;
   int left_ctx_base;

   /* Indication for morphological mode in Korean */
   unichar* jamo;
   int pos_in_jamo;

   /* The next element of the list */
   struct parsing_info* next;
};



struct parsing_info* new_parsing_info(int,int,int,int,unichar*,Variables*,struct dic_variable*,
		                              int,int,unichar*,int);
void free_parsing_info(struct parsing_info*);
struct parsing_info* insert_if_absent(int,int,int,struct parsing_info*,int,unichar*,Variables*,
		                              struct dic_variable*,int,int,unichar*,int);
struct parsing_info* insert_if_different(int,int,int,struct parsing_info*,int,unichar*,Variables*,
		                                 struct dic_variable*,int,int,unichar*,int);
struct parsing_info* insert_morphological_match(int pos,int pos_in_token,int state,
                                                struct parsing_info* list,struct dela_entry*,
                                                unichar* jamo,int pos_in_jamo);

#endif

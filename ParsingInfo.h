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

#ifndef ParsingInfoH
#define ParsingInfoH

#include "Unicode.h"
#include "TransductionVariables.h"


/**
 * This structure is used to store information during the parsing of a text.
 * It represents the state of the parsing when arriving at the final state of
 * a subgraph. Such information is represented as a list.
 */
struct parsing_info {
   /* Current position in the text, i.e. position in the text when the
    * final state of the subgraph was reached. */
   int position;
   
   /* Content of the stack */
   unichar* stack;
   /* Stack pointer */
   int stack_pointer;
   
   /* A copy of the variable ranges */
   int* variable_backup;
   
   /* The next element of the list */
   struct parsing_info* next;
};



struct parsing_info* new_parsing_info(int,int,unichar*,Variables*);
void free_parsing_info(struct parsing_info*);
struct parsing_info* insert_if_absent(int,struct parsing_info*,int,unichar*,Variables*);
struct parsing_info* insert_if_different(int,struct parsing_info*,int,unichar*,Variables*);


#endif

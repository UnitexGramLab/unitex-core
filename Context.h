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

//---------------------------------------------------------------------------
#ifndef ContextH
#define ContextH
//---------------------------------------------------------------------------
#include "TransductionStack.h"
#include "unicode.h"
#include "Liste_num.h"


#define INSIDE_POSITIVE_CONTEXT 0
#define INSIDE_NEGATIVE_CONTEXT 1
#define FAILED_IN_NEGATIVE_CONTEXT 2
#define NEGATIVE_CONTEXT_HAS_MATCHED 3

struct context {
   unsigned char contextMode;
   int continue_position;
   unichar stack[STACK_SIZE];
   int stack_pointer;
   struct liste_num** list_of_matches;
   int n_matches;
   int depth;
   struct context* next;
};


struct context* new_context(unsigned char,int,unichar*,int,
                            struct liste_num**,int,struct context*);
struct context* remove_context(struct context*);
void free_context_list(struct context*);


#endif


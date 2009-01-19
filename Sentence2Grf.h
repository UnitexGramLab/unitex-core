 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef Sentence_to_grfH
#define Sentence_to_grfH


#include "Unicode.h"
#include "Fst2.h"
#include "List_int.h"


/**
 * This structure represents a box of a grf file. For the moment, the Y position
 * is not there because it is not used.
 */
struct grf_state {
   unichar* content;
   int pos_X;
   int rank;
   /* The transition list is supposed to be sorted by increasing order */
   struct list_int* l;
};



void sentence_to_grf(Fst2*,int,char*,FILE*);
int width_of_tag(Fst2Tag);
void write_grf_header(int,int,int,char*,FILE*);

#endif

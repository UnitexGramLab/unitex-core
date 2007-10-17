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

#ifndef LocateFst2TagsH
#define LocateFst2TagsH

#include "Fst2.h"
#include "Alphabet.h"
#include "String_hash.h"
#include "LocatePattern.h"
#include "LemmaTree.h"


void process_tags(int*,
                  struct string_hash*,
                  int*,int*,
                  int*,struct locate_parameters*);

void optimize_pattern_tags(Alphabet*,struct lemma_node*,struct locate_parameters*);

#endif

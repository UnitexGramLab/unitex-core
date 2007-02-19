 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#ifndef Loading_dicH
#define Loading_dicH
//---------------------------------------------------------------------------

#include "unicode.h"
#include "DELA.h"
#include "Alphabet.h"
#include "String_hash.h"
#include "Text_tokens.h"
#include "List_int.h"
#include "LocatePattern.h"
#include "CompoundWordTree.h"
#include "LocateConstants.h"
#include "LemmaTree.h"


void load_dic_for_locate(char*,Alphabet*,int,int,int,int,struct lemma_node*,struct locate_parameters*);
void check_patterns_for_tag_tokens(Alphabet*,int,struct lemma_node*,struct locate_parameters*);
int is_a_simple_word(unichar*,Alphabet*,int);

#endif

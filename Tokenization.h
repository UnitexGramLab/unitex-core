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

#ifndef TokenizationH
#define TokenizationH

/**
 * This library provides functions for tokenizing text.
 * 
 */

#include "unicode.h"
#include "Alphabet.h"
#include "List_ustring.h"
#include "LocateConstants.h"

struct list_ustring* tokenize(unichar*,TokenizationPolicy,Alphabet*);
struct list_ustring* tokenize_char_by_char(unichar*);
struct list_ustring* tokenize_word_by_word(unichar*,Alphabet*);
int is_a_simple_token(unichar*,TokenizationPolicy,Alphabet*);
int is_a_simple_word(unichar*,TokenizationPolicy,Alphabet*);

#endif














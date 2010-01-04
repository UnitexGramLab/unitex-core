 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef GeneralDerivationH
#define GeneralDerivationH


#include "Unicode.h"
#include "Alphabet.h"
#include "DELA.h"
#include "String_hash.h"

const int DDEBUG = 0; // 1 for additional debugging information
                      // 2 plus information about rule matching

// some definitions
const int MAX_COMPOSITION_RULE_LENGTH = 100;
const int MAX_NUMBER_OF_COMPOSITION_RULES = 50; // per lexicon entry
const int MAX_DICT_LINE_LENGTH = DIC_LINE_SIZE;
const int MAX_WORD_LENGTH = 500; // in the web frequency list there are very long words
const int MAX_CODE_LENGTH = 20;


struct _tags {
  char PREFIX[MAX_CODE_LENGTH];
  char SUFFIX[MAX_CODE_LENGTH];
  char RULE[MAX_CODE_LENGTH];
};
typedef _tags tags;
struct utags {
  unichar PREFIX[MAX_CODE_LENGTH];
  unichar SUFFIX[MAX_CODE_LENGTH];
  unichar RULE[MAX_CODE_LENGTH];
};
struct utags init_utags (tags);

void analyse_compounds(Alphabet*, unsigned char*, struct INF_codes*, U_FILE*, U_FILE*, U_FILE*, U_FILE*,
                       struct utags);

#endif

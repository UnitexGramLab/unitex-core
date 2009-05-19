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

#ifndef AlphabetH
#define AlphabetH

#include "Unicode.h"


struct alphabet_ {
  unichar* t[0x10000]; // t['e']= "E{E+'}" where {E+'} stands for the unicode
                       // character representing the E with accent
  char t2[0x10000];
       // t2['E'] = 1 -> 'E' is an uppercase letter
       // t2['e'] = 2 -> 'e' is a lowercase letter
       // t2['?'] = (1 & 2) -> '?' is a non variable letter (Thai, Chinese, ...)
       // t2['?'] = 0 -> '?' is not a letter
  
  /* This array is only used for Korean alphabets, because it is useful to
   * know for a given Chinese character its Hangul syllab equivalent */
  unichar* korean_equivalent_syllab;
};

typedef struct alphabet_ Alphabet;

Alphabet* load_alphabet(const char*);
Alphabet* load_alphabet(const char*,int);
void free_alphabet(Alphabet*);
int is_upper_of(unichar,unichar,Alphabet*);
int is_equal_ignore_case(unichar,unichar,Alphabet*);
int is_equal_or_uppercase(unichar,unichar,Alphabet*);
int is_equal_or_uppercase(unichar*,unichar*,Alphabet*);
int is_lower(unichar,Alphabet*);
int is_upper(unichar,Alphabet*);
int is_letter(unichar,Alphabet*);
int is_letter2(unichar,Alphabet*);
int is_sequence_of_lowercase_letters(unichar*,Alphabet*);
int is_sequence_of_uppercase_letters(unichar*,Alphabet*);
int is_sequence_of_letters(unichar*,Alphabet*);
int is_equal_ignore_case_and_quotes(unichar*,unichar*,Alphabet*);
void turn_portuguese_sequence_to_lowercase(unichar*);
void replace_letter_by_letter_set(Alphabet*,unichar*,unichar*);
int get_longuest_prefix_ignoring_case(unichar*,unichar*,Alphabet*);

#endif

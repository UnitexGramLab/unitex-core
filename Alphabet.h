/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
	
  //unichar* t[0x10000]; // obsolete
  // t['e']= "E{E+'}" where {E+'} stands for the unicode
                       // character representing the E with accent
  //char t2[0x10000]; // obsolete
  
  int i_last_array_pos_used;
  int i_nb_array_pos_allocated;
  unichar** t_array_collection;
  // t_array_collection[pos_in_represent_list['e'] = "E{E+'}" where {E+'} stands for the unicode
                       // character representing the E with accent
  uint16_t pos_in_represent_list[0x10000];

  unsigned char array_case_flags[(0x10000*2)/8];

  // for a char c, IS_UPPER_MACRO(c,alphabet) != 0
  // tCaseFlags[(c >> 2)] & (1 << ((c & 3)*2)) != 0 -> c is an uppercase letter
  // (by example, c=='E')
  //
  // for a char c, IS_LOWER_MACRO(c,alphabet) != 0
  // tCaseFlags[(c >> 2)] & (1 << (((c & 3)*2)+1)) != 0 -> c is an lowercase letter
  // (by example, c=='e')
  //
  // CASE_FLAG_MACRO(c,alphabet) == 3
  // ((tCaseFlags[(c >> 2)] & (1 << ((c & 3)*2)) != 0) &&
  //  (tCaseFlags[(c >> 2)] & (1 << (((c & 3)*2)+1)) != 0)) -> c is a non variable letter (Thai, Chinese, ...)
  //
  // CASE_FLAG_MACRO(c,alphabet) == 3
  // ((tCaseFlags[(c >> 2)] & (1 << ((c & 3)*2)) == 0) &&
  //  (tCaseFlags[(c >> 2)] & (1 << (((c & 3)*2)+1)) == 0)) -> c is a non letter
  //
  // remember : (c >> 2) == (c / 4)
  //            (c & 3)  == (c % 4)
  //            (x << 1) == (x * 2)
  
  /* This array is only used for Korean alphabets, because it is useful to
   * know for a given Chinese character its Hangul syllable equivalent */
  unichar* korean_equivalent_syllable;
};

typedef struct alphabet_ Alphabet;

#define ARRAY_ITEM(c) ((c) >> 2)
#define SHIFT_BIT(c) (((c) & 3) << 1)

#define SET_CASE_FLAG_MACRO(c,alphabet,value) \
	            ((alphabet)->array_case_flags[ARRAY_ITEM(c)] |= ((value) << SHIFT_BIT(c)))

#define CASE_FLAG_MACRO(c,alphabet) \
	            ((((alphabet)->array_case_flags[ARRAY_ITEM(c)]) >> SHIFT_BIT(c)) & 3)

#define IS_LOWER_MACRO(c,alphabet) \
	            ((CASE_FLAG_MACRO(c,alphabet)) & 2)

#define IS_UPPER_MACRO(c,alphabet) \
	            ((CASE_FLAG_MACRO(c,alphabet)) & 1)

Alphabet* load_alphabet(const char*);
Alphabet* load_alphabet(const char*,int);
void free_alphabet(Alphabet*);
int is_upper_of(unichar,unichar,const Alphabet*);
int is_equal_ignore_case(unichar,unichar,const Alphabet*);
int is_equal_or_uppercase(unichar,unichar,const Alphabet*);
int is_equal_or_uppercase(const unichar*,const unichar*,const Alphabet*);
int is_lower(unichar,const Alphabet*);
int is_upper(unichar,const Alphabet*);
int is_letter(unichar,const Alphabet*);
int is_sequence_of_lowercase_letters(const unichar*,const Alphabet*);
int is_sequence_of_uppercase_letters(const unichar*,const Alphabet*);
int is_sequence_of_letters(const unichar*,const Alphabet*);
int is_equal_ignore_case_and_quotes(const unichar*,const unichar*,const Alphabet*);
void turn_portuguese_sequence_to_lowercase(unichar*);
void replace_letter_by_letter_set(const Alphabet*,unichar*,const unichar*);
int get_longuest_prefix_ignoring_case(const unichar*,const unichar*,const Alphabet*);

#endif

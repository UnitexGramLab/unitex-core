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

#ifndef KoreanH
#define KoreanH

#include "Unicode.h"
#include "Alphabet.h"
#include "jamoCodage.h"
#include "state_machine_fst2.h"


#define MAX_LETTERS_IN_A_SYLLAB 5
#define KR_SYLLAB_BOUND 0x318D
#define KR_EMPTY_INITIAL_CONSONANT 0x110B


/**
 * This object is used to convert a Jamo letter sequence into
 * a Hangul syllab sequence.
 */
class Jamo2Syl : public state_machine, public jamoCodage
{
public:
	Jamo2Syl(){};
    ~Jamo2Syl(){};

	/**
	 * fst2 is supposed to be the path of the uneSyl.fst2 transducer included
	 * in Korean resources.
	 */
	void init(char* jamo_table,char* fst2) {
		loadJamoMap(jamo_table);
		setStrToVal(GetChangeStrContext(),sylMarkStr,sylMark);
		init_machine(fst2,2);
	}

} ;


int syllabToLetters_HCJ(unichar,unichar*);
void convert_Korean_text(unichar* src,unichar* dest,jamoCodage* jamo,Alphabet* alphabet);
void compatibility_jamo_to_standard_jamo(unichar c,unichar* dest,jamoCodage* jamo);
void convert_jamo_to_hangul(unichar* src,unichar* dest,Jamo2Syl* jamo2syl);
int get_length_in_jamo(unichar hangul,jamoCodage* jamo,Alphabet* alphabet);

#endif

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

#ifndef KoreanH
#define KoreanH

#include "Unicode.h"
#include "Alphabet.h"
#include "jamoCodage.h"
#include "state_machine_fst2.h"


#define MAX_LETTERS_IN_A_SYLLAB 5
#define KR_EMPTY_INITIAL_CONSONANT 0x110B
#define KR_SYLLAB_BOUND 0x318D


class Korean {

   /* This array contains the lines of the Jamo configuration file. Index are
    * line numbers */
   unichar* orderTableJamo[256];

   /* This array is another indirection for converting Unicode Jamos to Jamos.
    * If we have a Unicode Jamo #Z, CjamoUjamoTable[Z-0x3130] will point
    * to a unichar array containing equivalent Jamos */
   unichar* CjamoUjamoTable[256];

public:

   Korean(Alphabet* alph) {
      for(int i=0;i<256;i++) {
         orderTableJamo[i]=NULL;
         CjamoUjamoTable[i]=NULL;
      }
      initJamoMap();
      alphabet=alph;
   };

   ~Korean() {
      for(int i=0;i<JAMO_SIZE;i++) {
         if (orderTableJamo[i]!=NULL) {
            delete [] orderTableJamo[i];
         }
      }
   };

   int sylToJamo(unichar syl,unichar* output,int pos);
   int jamoToSJamo(unichar jamo,unichar* output,int pos);
   int convertSylToJamo(unichar* input,unichar* output,int size,int limit);
   int convertSyletCjamoToJamo(unichar* input,unichar* output,int size,int limit);
   int convHJAtoHAN(unichar* src,unichar* dest);


private:
   void initJamoMap();
   Alphabet* alphabet;
};


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
	void init(char* fst2) {
		setStrToVal(GetChangeStrContext(),sylMarkStr,KR_SYLLAB_BOUND);
		init_machine(fst2,2);
	}

} ;


int syllabToLetters_HCJ(unichar,unichar*);
void compatibility_jamo_to_standard_jamo(unichar c,unichar* dest,Korean* korean);
void convert_jamo_to_hangul(unichar* src,unichar* dest,Jamo2Syl* jamo2syl);
void convert_Korean_text(unichar* src,unichar* dest,Korean* korean,Alphabet* alphabet);
int get_length_in_jamo(unichar hangul,Korean* korean,Alphabet* alphabet);

#endif

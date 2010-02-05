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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "IOBuffer.h"
#include "Unicode.h"
#include "Copyright.h"
#include "Fst2.h"
#include "Error.h"
#include "AbstractFst2Load.h"
#include "Korean.h"
#include "Alphabet.h"


void kprintf(unichar* s) {
error("s=_%S_\n",s);
for (int i=0;s[i];i++) {
	error("%C (%X) ",s[i],s[i]);
}
error("\n");
}


/**
 * This program is designed for test purpose only.
 */
int main(int /*argc*/,char * /* argv*/ []) {
setBufferMode();

/* Korean tests */
Alphabet* alph=load_alphabet("/home/igm/unitex/KoreanJeeSun/Alphabet.txt",1);
Korean* korean=new Korean(alph);
unichar out[256];

Jamo2Syl* jamo2Syl=new Jamo2Syl();
jamo2Syl->init("/home/igm/unitex/KoreanJeeSun/Decoding/uneSyl.fst2");


unichar z[]={0x3134,'a',0x71B9,0x1104,0};

jamoCodage* jamo=new jamoCodage();
//jamo->loadJamoMap("/home/igm/unitex/KoreanJeeSun/jamoTable.txt");
//jamo->convertSyletCjamoToJamo(z,out,u_strlen(z),1024);
delete(jamo);
Hanguls_to_Jamos(z,out,korean);
error("input=: ");
kprintf(out);
error("\n");
return 0;


//unichar s[]={KR_SYLLABLE_BOUND,HCJ_KIYEOK,0x314F,0};
unichar s[]={KR_SYLLABLE_BOUND,0x1100,0x1104,0x1161,0x1105,'A',KR_SYLLABLE_BOUND,0x1161,0};

error("input=: ");
kprintf(s);
error("\n");


convert_jamo_to_hangul(s,out,jamo2Syl,korean);
error("avant: ");
kprintf(out);

korean->Jamos_to_Hangul(s,out);
error("apres: ");
kprintf(out);

return 0;
}





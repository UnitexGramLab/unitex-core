 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unicode.cpp"
#include "Korean.cpp"

//---------------------------------------------------------------------------
int main(int argc, char **argv) {
unichar res[MAX_LETTERS_IN_A_SYLLAB+1];

FILE* src=u_fopen("e:\\test_korean.txt",U_READ);
FILE* dest=u_fopen("e:\\syllab.txt",U_WRITE);
int c;
while ((c=u_fgetc(src))!=EOF) {
   if (syllabToLetters((unichar)c,res)) {
      u_fprints(res,dest);
   } else {u_fputc((unichar)c,dest);}
}
u_fclose(src);
u_fclose(dest);
return 0;
}


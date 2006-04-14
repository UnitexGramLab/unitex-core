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

#include "Compounds.h"


//unsigned char* tableau_bin;
char* tableau_correct_left_component;
char* tableau_correct_right_component;
FILE* debug_file;
FILE* result_file;
struct INF_codes* inf_codes;


//
// this function reads a token in a file and returns 1 on success, 0 else
//
/*int next_word(FILE* words,unichar* s) {
int c;
// we jump all the separators
while ((c=u_fgetc(words))!=EOF && (c==' ' || c=='\n' || c=='\t'));
if (c==EOF) {
   // if we are at the end of the file, we return
   return 0;
}
int i=0;
while (c!=EOF && c!=' ' && c!='\n' && c!='\t') {
   s[i++]=(unichar)c;
   c=u_fgetc(words);
}
s[i]='\0';
return 1;
}*/

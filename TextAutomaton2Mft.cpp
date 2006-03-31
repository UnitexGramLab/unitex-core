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

//--------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unicode.h"
#include "FileName.h"
#include "Fst2.h"
#include "Copyright.h"
#include "Fst2_to_Mft.h"
#include "String_hash.h"
#include "IOBuffer.h"

//
// "c:\unitex-visiteur\english\corpus\ivanhoe_snt\text.fst2"
//

//---------------------------------------------------------------------------
void usage() {
printf("%s",COPYRIGHT);
printf("Usage: TextAutomaton2Mft <fst2>\n");
printf("     <fst2> : fst2 file representing the text automaton to\n");
printf("              convert to a unicode .mft file.\n\n");
printf("Converts a Unitex text automaton into a Unicode Intex one.\n");
}



int main(int argc, char **argv) {
setBufferMode();

if (argc!=2) {
   usage();
   return 0;
}
printf("Loading text automaton...\n");
Automate_fst2* fst2=load_fst2(argv[1],0);
if (fst2==NULL) {
   fprintf(stderr,"Cannot load text automaton %s\n",argv[1]);
   return 1;
}
char temp[2000];
name_without_extension(argv[1],temp);
strcat(temp,".mft");
FILE* f=u_fopen(temp,U_WRITE);
if (f==NULL) {
   fprintf(stderr,"Cannot create %s\n",temp);
   free_fst2(fst2);
   return 1;
}
printf("Making %s...\n",temp);
convert_fst2_to_mft(fst2,f);
u_fclose(f);
free_fst2(fst2);
printf("Done.\n");
return 0;
}






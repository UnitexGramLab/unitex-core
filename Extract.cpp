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
#include "unicode.h"
#include "FileName.h"
#include "Text_tokens.h"
#include "Extract_units.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"

//---------------------------------------------------------------------------
void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Extract yes/no <text> <concordance> <result>\n");
printf("       yes/no : yes tells the program extract all matching units\n");
printf("                no extracts unmatching units\n");
printf("       <text> : the .snt text to extract from the units\n");
printf("       <concordance> : the .ind file that describes the concordance\n");
printf("       <result> : the text file where the units will be stored\n");
printf("\nExtract all the units that contain (or not) any part of a utterance. The\n");
printf("units are supposed to be separated by the symbol {S}.\n");
}


int main(int argc, char **argv) {
setBufferMode();

if (argc!=5) {
   usage();
   return 0;
}
char yes_no;
if (!strcmp(argv[1],"yes")) yes_no=1;
else if (!strcmp(argv[1],"no")) yes_no=0;
else {
   error("Invalid parameter %s: must be yes or no\n",argv[1]);
   return 1;
}

char temp[2000];
get_snt_path(argv[2],temp);
strcat(temp,"text.cod");
FILE* text=fopen(temp,"rb");
if (text==NULL) {
   error("Cannot open %s\n",temp);
   return 1;
}
get_snt_path(argv[2],temp);
strcat(temp,"tokens.txt");
struct text_tokens* tok=load_text_tokens(temp);
if (tok==NULL) {
   error("Cannot load token list %s\n",temp);
   fclose(text);
   return 1;
}
if (tok->SENTENCE_MARKER==-1) {
   error("The text does not contain any sentence marker {S}\n");
   fclose(text);
   free_text_tokens(tok);
   return 1;
}
FILE* concord=u_fopen(argv[3],U_READ);
if (concord==NULL) {
   error("Cannot open concordance %s\n",argv[3]);
   fclose(text);
   free_text_tokens(tok);
   return 1;
}
FILE* result=u_fopen(argv[4],U_WRITE);
if (result==NULL) {
   error("Cannot write output file %s\n",argv[4]);
   fclose(text);
   u_fclose(concord);
   free_text_tokens(tok);
   return 1;
}
extract_units(yes_no,text,tok,concord,result);
fclose(text);
u_fclose(concord);
u_fclose(result);
free_text_tokens(tok);
printf("Done.\n");
return 0;
}
//---------------------------------------------------------------------------

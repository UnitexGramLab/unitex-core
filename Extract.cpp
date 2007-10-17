 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "File.h"
#include "Text_tokens.h"
#include "ExtractUnits.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"
#include "Snt.h"


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Extract yes/no <text> <concordance> <result>\n");
u_printf("       yes/no : yes tells the program extract all matching units\n");
u_printf("                no extracts unmatching units\n");
u_printf("       <text> : the .snt text to extract from the units\n");
u_printf("       <concordance> : the .ind file that describes the concordance\n");
u_printf("       <result> : the text file where the units will be stored\n");
u_printf("\nExtract all the units that contain (or not) any part of a utterance. The\n");
u_printf("units are supposed to be separated by the symbol {S}.\n");
}


int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

if (argc!=5) {
   usage();
   return 0;
}
char extract_matching_units;
if (!strcmp(argv[1],"yes")) extract_matching_units=1;
else if (!strcmp(argv[1],"no")) extract_matching_units=0;
else {
   error("Invalid parameter %s: must be yes or no\n",argv[1]);
   return 1;
}
struct snt_files* snt_files=new_snt_files(argv[2]);
FILE* text=fopen(snt_files->text_cod,"rb");
if (text==NULL) {
   error("Cannot open %s\n",snt_files->text_cod);
   return 1;
}
struct text_tokens* tok=load_text_tokens(snt_files->tokens_txt);
if (tok==NULL) {
   error("Cannot load token list %s\n",snt_files->tokens_txt);
   fclose(text);
   return 1;
}
if (tok->SENTENCE_MARKER==-1) {
   error("The text does not contain any sentence marker {S}\n");
   fclose(text);
   free_text_tokens(tok);
   return 1;
}
/* Warning: here we don't use a concordance name built from argv[2],
 *          since the concordance file can be elsewhere and can have 
 *          another name than "concord.ind" */
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
free_snt_files(snt_files);
extract_units(extract_matching_units,text,tok,concord,result);
fclose(text);
u_fclose(concord);
u_fclose(result);
free_text_tokens(tok);
u_printf("Done.\n");
return 0;
}


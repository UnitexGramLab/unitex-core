 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "DELA.h"
#include "String_hash.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"


void print_TAGML2_header(FILE*);


#warning hamorniser les formats de traits flexionnels entre tagml, tagset.def et les fichiers de MultiFlex
void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: ExportDic <dela> <out>\n");
u_printf("     <dela> : name of the unicode text dictionary\n");
u_printf("     <out> : name of the TAGML2 output dictionary\n");
u_printf("\nExports the given French DELA into a UTF8 TAGML2 dictionary.\n");
}


int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();
if (argc!=3) {
   usage();
   return 0;
}
FILE* dic=u_fopen(argv[1],U_READ);
if (dic==NULL) {
	fatal_error("Cannot open dictionary %s\n",argv[1]);
}
FILE* out=u_fopen(UTF8,argv[2],U_WRITE);
if (out==NULL) {
   u_fclose(dic);
   fatal_error("Cannot open output file %s\n",argv[2]);
}
print_TAGML2_header(out);
unichar line[10000];
int line_number=0;
while (EOF!=u_fgets(line,dic)) {
   line_number++;
   if ((line_number%10000)==0) {
      u_printf("%d lines read \r",line_number);
   }
	if (line[0]!='\0' && line[0]!='/') {
		/* If a line starts with '/', it is a commment line, so
		 * we ignore it, as we ignore the empty lines */
      struct dela_entry* entry=tokenize_DELAF_line(line);
      if (entry->n_inflectional_codes==0) {
         /* If the entry has no inflectional code */
         u_fprintf(UTF8,out,"  <morph lex=\"%S\">\n",entry->inflected);
         u_fprintf(UTF8,out,"    <lemmaref cat=\"%S\" name=\"%S\"/>\n",entry->semantic_codes[0],entry->lemma);
         u_fprintf(UTF8,out,"  </morph>\n");
      }
      else for (int i=0;i<entry->n_inflectional_codes;i++) {
         /* Otherwise, we produce one TAGML2 entry per inflectional code */
         u_fprintf(UTF8,out,"  <morph lex=\"%S\">\n",entry->inflected);
         u_fprintf(UTF8,out,"    <fs>\n");
         unichar* s=entry->inflectional_codes[i];
         for (int j=0;s[j]!='\0';j++) {
            switch (s[j]) {
               case '1': u_fprintf(UTF8,out,"      <f name=\"pers\"><sym value=\"1\"/></f>\n");
                         break;
               case '2': u_fprintf(UTF8,out,"      <f name=\"pers\"><sym value=\"2\"/></f>\n");
                         break;
               case '3': u_fprintf(UTF8,out,"      <f name=\"pers\"><sym value=\"3\"/></f>\n");
                         break;
               case 's': u_fprintf(UTF8,out,"      <f name=\"num\"><sym value=\"sing\"/></f>\n");
                         break;
               case 'p': u_fprintf(UTF8,out,"      <f name=\"num\"><sym value=\"plur\"/></f>\n");
                         break;
               case 'm': u_fprintf(UTF8,out,"      <f name=\"gender\"><sym value=\"masc\"/></f>\n");
                         break;
               case 'f': u_fprintf(UTF8,out,"      <f name=\"gender\"><sym value=\"fem\"/></f>\n");
                         break;
               case 'P': u_fprintf(UTF8,out,"      <f name=\"mod\"><sym value=\"ind\"/></f>\n");
                         u_fprintf(UTF8,out,"      <f name=\"tense\"><sym value=\"present\"/></f>\n");
                         break;
               case 'I': u_fprintf(UTF8,out,"      <f name=\"mod\"><sym value=\"ind\"/></f>\n");
                         u_fprintf(UTF8,out,"      <f name=\"tense\"><sym value=\"imparfait\"/></f>\n");
                         break;
               case 'S': u_fprintf(UTF8,out,"      <f name=\"mod\"><sym value=\"subj\"/></f>\n");
                         u_fprintf(UTF8,out,"      <f name=\"tense\"><sym value=\"present\"/></f>\n");
                         break;
               case 'T': u_fprintf(UTF8,out,"      <f name=\"mod\"><sym value=\"subj\"/></f>\n");
                         u_fprintf(UTF8,out,"      <f name=\"tense\"><sym value=\"imparfait\"/></f>\n");
                         break;
               case 'Y': u_fprintf(UTF8,out,"      <f name=\"mod\"><sym value=\"imp\"/></f>\n");
                         u_fprintf(UTF8,out,"      <f name=\"tense\"><sym value=\"present\"/></f>\n");
                         break;
               case 'C': u_fprintf(UTF8,out,"      <f name=\"mod\"><sym value=\"cond\"/></f>\n");
                         u_fprintf(UTF8,out,"      <f name=\"tense\"><sym value=\"present\"/></f>\n");
                         break;
               case 'J': u_fprintf(UTF8,out,"      <f name=\"mod\"><sym value=\"ind\"/></f>\n");
                         u_fprintf(UTF8,out,"      <f name=\"tense\"><sym value=\"passe\"/></f>\n");
                         break;
               case 'W': u_fprintf(UTF8,out,"      <f name=\"mod\"><sym value=\"inf\"/></f>\n");
                         break;
               case 'G': u_fprintf(UTF8,out,"      <f name=\"mod\"><sym value=\"part\"/></f>\n");
                         u_fprintf(UTF8,out,"      <f name=\"tense\"><sym value=\"present\"/></f>\n");
                         break;
               case 'K': u_fprintf(UTF8,out,"      <f name=\"mod\"><sym value=\"part\"/></f>\n");
                         u_fprintf(UTF8,out,"      <f name=\"tense\"><sym value=\"passe\"/></f>\n");
                         break;
               case 'F': u_fprintf(UTF8,out,"      <f name=\"mod\"><sym value=\"ind\"/></f>\n");
                         u_fprintf(UTF8,out,"      <f name=\"tense\"><sym value=\"futur\"/></f>\n");
                         break;
            }
         }
         u_fprintf(UTF8,out,"    </fs>\n");
         u_fprintf(UTF8,out,"    <lemmaref cat=\"%S\" name=\"%S\"/>\n",entry->semantic_codes[0],entry->lemma);
         u_fprintf(UTF8,out,"  </morph>\n");
      }
      free_dela_entry(entry);
   }
}
u_fclose(dic);
u_fprintf(UTF8,out,"</tagml>\n");
u_fclose(out);
u_printf("%d line%s read \r",line_number,(line_number>1)?"s":"");
u_printf("\nDone.\n");
return 0;
}


/**
 * Prints the header of a TAGML2 file.
 */
void print_TAGML2_header(FILE* f) {
u_fprintf(UTF8,f,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
u_fprintf(UTF8,f,"<!DOCTYPE tagml SYSTEM \"tag.dtd\">\n");
u_fprintf(UTF8,f,"\n");
u_fprintf(UTF8,f,"<tagml>\n");
}

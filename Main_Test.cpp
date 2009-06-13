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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "IOBuffer.h"
#include "Pattern.h"
#include "PatternTree.h"
#include "Unicode.h"
#include "String_hash.h"
#include "DELA.h"
#include "List_pointer.h"
#include "StringParsing.h"
#include "Copyright.h"
#include "Fst2.h"
#include "Error.h"
#include "Tfst.h"

/**
 * This program is designed for test purpose only.
 */
int main(int argc,char *argv[]) {
setBufferMode();

U_FILE* input=u_fopen(UTF16_LE,argv[1],U_READ);
if (input==NULL) {
	fatal_error("Cannot open %s\n",argv[1]);
}
U_FILE* output=u_fopen(UTF16_LE,argv[2],U_WRITE);
if (output==NULL) {
	fatal_error("Cannot open %s\n",argv[2]);
}
unichar line[4096];
while (u_fgets(line,input)!=EOF) {
   int i=0;
   while (line[i]!=',') {
	   u_fprintf(output,"%C",line[i++]);
   }
   if (line[i]!=',') {
	   fatal_error("Oops\n");
   }
   i+=2;
   u_fprintf(output,",");
   while (line[i]!=',') {
	   u_fprintf(output,"%C",line[i++]);
   }
   int j=u_strlen(line);
   do {
	   j--;
   } while (line[j+1]!=',');
   line[j+1]='\0';
   do {
	   j--;
   } while (line[j]>='0' && line[j]<='9');
   u_fprintf(output,"%S",line+j+1);
   int last_was_comma=0;
   while (i!=j-1) {
	   if (line[i]==',') {
		   if (!last_was_comma) {
			   i++;
			   last_was_comma=1;
		   } else {
			   i++;
		   }
	   } else {
		   if (last_was_comma) {
			   u_fprintf(output,"+");
			   last_was_comma=0;
		   }
		   u_fprintf(output,"%C",line[i++]);
	   }
   }
   u_fprintf(output,"\n");
}

u_fclose(input);
u_fclose(output);
return 0;
}





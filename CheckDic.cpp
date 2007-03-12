 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Universit� de Marne-la-Vall�e <unitex@univ-mlv.fr>
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
#include "FileName.h"
#include "DELA.h"
#include "String_hash.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"



void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: CheckDic <dela> <type>\n");
u_printf("     <dela> : name of the unicode text dictionary (must be a full path)\n");
u_printf("     <type> : dictionary type. Two values are possible:\n");
u_printf("              DELAS : check any non inflected dictionary\n");
u_printf("              DELAF : check any inflected dictionary\n");
u_printf("\nChecks the format of <dela> and produces a file named CHECK_DIC.TXT\n");
u_printf("that contains check result informations. This file is stored in the\n");
u_printf("<dela> directory.\n");
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
FILE* dic;
FILE* out;
int is_a_DELAF;
if (!strcmp(argv[2],"DELAS")) {
	is_a_DELAF=0;
}
else if (!strcmp(argv[2],"DELAF")) {
		is_a_DELAF=1;
}
else {
	fatal_error("Invalid dictionary type %s\n",argv[2]);
}
dic=u_fopen(argv[1],U_READ);
if (dic==NULL) {
	fatal_error("Cannot open dictionary %s\n",argv[1]);
}
char output_filename[2048];
get_path(argv[1],output_filename);
strcat(output_filename,"CHECK_DIC.TXT");
out=u_fopen(output_filename,U_WRITE);
if (out==NULL) {
	u_fclose(dic);
	fatal_error("Cannot create %s\n",output_filename);
}
u_printf("Checking %s...\n",argv[1]);
unichar line[10000];
int line_number=1;
/*
 * We declare and initialize an array in order to know which
 * letters are used in the dictionary.
 */
char alphabet[MAX_NUMBER_OF_UNICODE_CHARS];
for (int i=0;i<MAX_NUMBER_OF_UNICODE_CHARS;i++) {
	alphabet[i]=0;
}
/*
 * We use two structures for the storage of the codes found in the
 * dictionary. Note that 'semantic_codes' is used to store both grammatical and
 * semantic codes.
 */
struct string_hash* semantic_codes=new_string_hash();
struct string_hash* inflectional_codes=new_string_hash();
/*
 * We read all the lines and check them.
 */
while (EOF!=u_fgets(line,dic)) {
	if (line[0]=='\0') {
		/* If we have an empty line, we print a unicode error message
		 * into the output file */
		u_fprintf(out,"Line %d: empty line\n",line_number);
	}
	else if (line[0]=='/') {
		/* If a line starts with '/', it is a commment line, so
		 * we ignore it */
	}
	else {
		/* If we have a line to check, we check it according to the
		 * dictionary type */
		check_DELA_line(line,out,is_a_DELAF,line_number,alphabet,semantic_codes,inflectional_codes);
	}
	/* At regular intervals, we display a message on the standard
	 * output to show that the program is working */
	if (line_number%10000==0) {
		u_printf("%d lines read...\r",line_number);
	}
	line_number++;
}
u_printf("%d lines read\n",line_number-1);
u_fclose(dic);
/*
 * Once we have checked all the lines, we print some informations
 * in the output file.
 * 
 * First, we print the list of the characters that are used, with
 * their unicode numbers shown in hexadecimal. This can be useful
 * to detect different characters that are graphically identical
 * like 'A' (upper of latin 'a' or upper of greek alpha ?).
 */
u_fprintf(out,"-----------------------------------\n");
u_fprintf(out,"----  All chars used in forms  ----\n");
u_fprintf(out,"-----------------------------------\n");
unichar r[4];
unichar r2[7];
r[1]=' ';
r[2]='(';
r[3]='\0';
r2[5]='\n';
r2[6]='\0';
for (int i=0;i<MAX_NUMBER_OF_UNICODE_CHARS;i++) {
	if (alphabet[i]) {
      u_fprintf(out,"%C (%04X)\n",i,i);
	}
}
/*
 * Then we print the list of all grammatical and semantic codes used in the
 * dictionary. If a code contains a non ASCII character, a space or a tabulation,
 * we print a warning.
 */
u_fprintf(out,"-------------------------------------------------------------\n");
u_fprintf(out,"----  %3d grammatical/semantic code%s",semantic_codes->size,(semantic_codes->size>1)?"s used in dictionary  ----\n":" used in dictionary  -----\n");
u_fprintf(out,"-------------------------------------------------------------\n");
unichar comment[2000];
for (int i=0;i<semantic_codes->size;i++) {
	/* We print the code, followed if necessary by a warning */
	u_fprintf(out,"%S",semantic_codes->value[i]);
	if (warning_on_code(semantic_codes->value[i],comment)) {
		u_fprintf(out," %S",comment);
	}
	u_fprintf(out,"\n");
}
/*
 * Finally, we print the list of inflectional codes,
 * with warnings in the case of non ASCII letters, spaces
 * or tabulations.
 */
u_fprintf(out,"-----------------------------------------------------\n");
u_fprintf(out,"----  %3d inflectional code%s",inflectional_codes->size,(inflectional_codes->size>1)?"s used in dictionary  ----\n":" used in dictionary  -----\n");
u_fprintf(out,"-----------------------------------------------------\n");
for (int i=0;i<inflectional_codes->size;i++) {
	u_fprintf(out,"%S",inflectional_codes->value[i]);
	if (warning_on_code(inflectional_codes->value[i],comment)) {
		u_fprintf(out," %S",comment);
	}
	u_fprintf(out,"\n");
}
u_fclose(out);
u_printf("Done.\n");
return 0;
}


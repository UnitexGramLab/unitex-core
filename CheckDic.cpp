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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unicode.h"
#include "FileName.h"
#include "DELA.h"
#include "String_hash.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"



void usage() {
printf("%s",COPYRIGHT);
printf("Usage: CheckDic <dela> <type>\n");
printf("     <dela> : name of the unicode text dictionary (must be a full path)\n");
printf("     <type> : dictionary type. Two values are possible:\n");
printf("              DELAS : check any non inflected dictionary\n");
printf("              DELAF : check any inflected dictionary\n");
printf("\nChecks the format of <dela> and produces a file named CHECK_DIC.TXT\n");
printf("that contains check result informations. This file is stored in the\n");
printf("<dela> directory.\n");
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
printf("Checking %s...\n",argv[1]);
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
while (u_read_line(dic,line)) {
	if (line[0]=='\0') {
		/* If we have an empty line, we print a unicode error message
		 * into the output file */
		char temp[1000];
		sprintf(temp,"Line %d: empty line\n",line_number);
		u_fprints_char(temp,out);
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
		printf("%d lines read...\r",line_number);
	}
	line_number++;
}
printf("%d lines read\n",line_number-1);
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
u_fprints_char("-----------------------------------\n",out);
u_fprints_char("----  All chars used in forms  ----\n",out);
u_fprints_char("-----------------------------------\n",out);
unichar r[4];
unichar r2[7];
r[1]=' ';
r[2]='(';
r[3]='\0';
r2[5]='\n';
r2[6]='\0';
for (int i=0;i<MAX_NUMBER_OF_UNICODE_CHARS;i++) {
	if (alphabet[i]) {
		r[0]=(unichar)i;
		u_char_to_hexa((unichar)i,r2);
		r2[4]=')';
		u_fprints(r,out);
		u_fprints(r2,out);
	}
}
/*
 * Then we print the list of all grammatical and semantic codes used in the
 * dictionary. If a code contains a non ASCII character, a space or a tabulation,
 * we print a warning.
 */
u_fprints_char("-------------------------------------------------------------\n",out);
char tmp[1000];
sprintf(tmp,"----  %3d grammatical/semantic code%s",semantic_codes->N,(semantic_codes->N>1)?"s used in dictionary  ----\n":" used in dictionary  -----\n");
u_fprints_char(tmp,out);
u_fprints_char("-------------------------------------------------------------\n",out);
unichar comment[2000];
for (int i=0;i<semantic_codes->N;i++) {
	/* We print the code, followed if necessary by a warning */
	u_fprints(semantic_codes->tab[i],out);
	if (warning_on_code(semantic_codes->tab[i],comment)) {
		u_fprints_char(" ",out);
		u_fprints(comment,out);
	}
	u_fprints_char("\n",out);
}
/*
 * Finally, we print the list of inflectional codes,
 * with warnings in the case of non ASCII letters, spaces
 * or tabulations.
 */
u_fprints_char("-----------------------------------------------------\n",out);
sprintf(tmp,"----  %3d inflectional code%s",inflectional_codes->N,(inflectional_codes->N>1)?"s used in dictionary  ----\n":" used in dictionary  -----\n");
u_fprints_char(tmp,out);
u_fprints_char("-----------------------------------------------------\n",out);
for (int i=0;i<inflectional_codes->N;i++) {
	u_fprints(inflectional_codes->tab[i],out);
	if (warning_on_code(inflectional_codes->tab[i],comment)) {
		u_fprints_char(" ",out);
		u_fprints(comment,out);
	}
	u_fprints_char("\n",out);
}
u_fclose(out);
printf("Done.\n");
return 0;
}


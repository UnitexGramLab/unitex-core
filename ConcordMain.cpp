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
#include "ConcordMain.h"
#include "unicode.h"
#include "Text_tokens.h"
#include "String_hash.h"
#include "Liste_nombres.h"
#include "Alphabet.h"
#include "Matches.h"
#include "Concordance.h"
#include "FileName.h"
#include "Copyright.h"
#include "LocatePattern.h"
#include "Error.h"


/* Maximum number of new lines in a text. New lines are encoded in
 * 'enter.pos' files. Those files will disappear in the futures */
#define MAX_ENTER_CHAR 1000000
int enter_pos[MAX_ENTER_CHAR];


/* 
 * This function behaves in the same way that a main one, except that it does
 * not invoke the setBufferMode function and that it does not print the
 * synopsis.
 */
int main_Concord(int argc, char **argv) {
if (argc!=9 && argc!=10 && argc!=11) {
	return 0;
}
int fontsize;
if (1!=sscanf(argv[3],"%d",&fontsize)) {
	error("Invalid font size parameter %s\n",argv[3]);
	return 1;
}
FILE* concor=u_fopen(argv[1],U_READ);
if (concor==NULL) {
   error("Cannot open file %s\n",argv[1]);
   return 1;
}
/* We initialize the ..._snt directory with the directory where we found 'concord.ind' */
char snt_dir[2000];
get_filename_path(argv[1],snt_dir);
char text_cod[2000];
char tokens_txt[2000];
char enter[2000];
/* By default, we are not dealing with a Thai concordance */
int thai_mode=0;
if (argc!=9) {
	/* We look for the -thai and <snt_dir> optional parameters */
    if (strcmp(argv[9],"-thai")) {
    	/* If there is an extra parameter that is not -thai, then it is the <snt_dir> parameter */
    	strcpy(snt_dir,argv[9]);
    }
	else {
		/* If there is -thai */
		thai_mode=1;
		if (argc==11) {
			/* If there is another extra parameter, then it is the <snt_dir> parameter */
			strcpy(snt_dir,argv[10]);
		}
	}
}
strcpy(text_cod,snt_dir);
strcat(text_cod,"text.cod");
strcpy(tokens_txt,snt_dir);
strcat(tokens_txt,"tokens.txt");
strcpy(enter,snt_dir);
strcat(enter,"enter.pos");
FILE* text=fopen(text_cod,"rb");
if (text==NULL) {
	error("Cannot open file %s\n",text_cod);
	u_fclose(concor);
	return 1;
}
struct text_tokens* tok=load_text_tokens(tokens_txt);
if (tok==NULL) {
	error("Cannot load text token file %s\n",tokens_txt);
	u_fclose(concor);
	fclose(text);
	return 1;
}
FILE* f_enter=fopen(enter,"rb");
int n_enter_char;
if (f_enter==NULL) {
	error("Cannot open file %s\n",enter);
	n_enter_char=0;
}
else {
	n_enter_char=fread(&enter_pos,sizeof(int),MAX_ENTER_CHAR,f_enter);
	fclose(f_enter);
}
int left_context;
int right_context;
if (1!=sscanf(argv[4],"%d",&left_context)) {
	error("Invalid left context length %s\n",argv[4]);
	u_fclose(concor);
	fclose(text);
	free_text_tokens(tok);
	return 1;
}
if (1!=sscanf(argv[5],"%d",&right_context)) {
	error("Invalid right context length %s\n",argv[5]);
	u_fclose(concor);
	fclose(text);
	free_text_tokens(tok);
	return 1;
}
int sort_mode;
if (!strcmp(argv[6],"TO") || !strcmp(argv[6],"NULL"))
	sort_mode=TEXT_ORDER;
else if (!strcmp(argv[6],"LC"))
	sort_mode=LEFT_CENTER;
else if (!strcmp(argv[6],"LR"))
	sort_mode=LEFT_RIGHT;
else if (!strcmp(argv[6],"CL"))
	sort_mode=CENTER_LEFT;
else if (!strcmp(argv[6],"CR"))
	sort_mode=CENTER_RIGHT;
else if (!strcmp(argv[6],"RL"))
	sort_mode=RIGHT_LEFT;
else if (!strcmp(argv[6],"RC"))
	sort_mode=RIGHT_CENTER;
else {
	error("Invalid sort mode %s\n",argv[6]);
	u_fclose(concor);
	fclose(text);
	free_text_tokens(tok);
	return 1;
}
char working_directory[2000];
get_filename_path(argv[1],working_directory);
/* Once we have setted all the parameters, we call the function that
 * will actually create the concordance. */
create_concordance(concor,text,tok,sort_mode,left_context,right_context,argv[2],
					argv[3],working_directory,argv[7],argv[8],n_enter_char,enter_pos,thai_mode);
u_fclose(concor);
fclose(text);
free_text_tokens(tok);
printf("Done.\n");
return 0;
}


 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Unicode.h"
#include "Text_tokens.h"
#include "String_hash.h"
#include "List_int.h"
#include "Alphabet.h"
#include "Matches.h"
#include "Concordance.h"
#include "File.h"
#include "Copyright.h"
#include "LocatePattern.h"
#include "Error.h"
#include "Snt.h"


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
/* We compute the name of the files associated to the text */
struct snt_files* snt_files=NULL;
/* By default, we are not dealing with a Thai concordance */
struct conc_opt option;
option.thai_mode = 0;
option.left_context_until_eos = 0;
option.right_context_until_eos = 0;
if (argc!=9) {
	/* We look for the -thai and <snt_dir> optional parameters */
    if (strcmp(argv[9],"-thai")) {
    	/* If there is an extra parameter that is not -thai, then it is the <snt_dir> parameter */
    	snt_files=new_snt_files_from_path(argv[9]);
    }
	else {
		/* If there is -thai */
		option.thai_mode=1;
		if (argc==11) {
			/* If there is another extra parameter, then it is the <snt_dir> parameter */
			snt_files=new_snt_files_from_path(argv[10]);
		}
	}
}
if (snt_files==NULL) {
   char text_snt[FILENAME_MAX];
   get_path(argv[1],text_snt);
   snt_files=new_snt_files_from_path(text_snt);
}
FILE* text=fopen(snt_files->text_cod,"rb");
if (text==NULL) {
	error("Cannot open file %s\n",snt_files->text_cod);
	u_fclose(concor);
	return 1;
}
struct text_tokens* tok=load_text_tokens(snt_files->tokens_txt);
if (tok==NULL) {
	error("Cannot load text token file %s\n",snt_files->tokens_txt);
	u_fclose(concor);
	fclose(text);
	return 1;
}
FILE* f_enter=fopen(snt_files->enter_pos,"rb");
int n_enter_char;
if (f_enter==NULL) {
	error("Cannot open file %s\n",snt_files->enter_pos);
	n_enter_char=0;
}
else {
	n_enter_char=fread(&enter_pos,sizeof(int),MAX_ENTER_CHAR,f_enter);
   fclose(f_enter);
}
char test='\0';
if (1!=sscanf(argv[4],"%d%c",&option.left_context,&test))
	option.left_context_until_eos=1; /* "80s" means: 80 characters
	                                    context, but stop at "{S}" */
else if (1!=sscanf(argv[4],"%d",&option.left_context)) {
	error("Invalid left context length %s\n",argv[4]);
	u_fclose(concor);
	fclose(text);
	free_text_tokens(tok);
	return 1;
}
if (1!=sscanf(argv[5],"%d%c",&option.right_context,&test))
	option.right_context_until_eos=1; /* dito */
else if (1!=sscanf(argv[5],"%d",&option.right_context)) {
	error("Invalid right context length %s\n",argv[5]);
	u_fclose(concor);
	fclose(text);
	free_text_tokens(tok);
	return 1;
}
if (!strcmp(argv[6],"TO") || !strcmp(argv[6],"NULL"))
	option.sort_mode=TEXT_ORDER;
else if (!strcmp(argv[6],"LC"))
	option.sort_mode=LEFT_CENTER;
else if (!strcmp(argv[6],"LR"))
	option.sort_mode=LEFT_RIGHT;
else if (!strcmp(argv[6],"CL"))
	option.sort_mode=CENTER_LEFT;
else if (!strcmp(argv[6],"CR"))
	option.sort_mode=CENTER_RIGHT;
else if (!strcmp(argv[6],"RL"))
	option.sort_mode=RIGHT_LEFT;
else if (!strcmp(argv[6],"RC"))
	option.sort_mode=RIGHT_CENTER;
else {
	error("Invalid sort mode %s\n",argv[6]);
	u_fclose(concor);
	fclose(text);
	free_text_tokens(tok);
	return 1;
}
option.working_directory[0] = '\0';
get_path(argv[1],option.working_directory);
option.fontname=argv[2];
option.fontsize=argv[3];
option.result_mode=argv[7];
if (!strcmp(option.result_mode,"index") 
    || !strcmp(option.result_mode,"uima")
    || !strcmp(option.result_mode,"axis")) {
   /* We force some options for index, uima and axis files */
   option.left_context=0;
   option.right_context=0;
   option.sort_mode=TEXT_ORDER;
}
option.sort_alphabet=argv[8];

/* Once we have setted all the parameters, we call the function that
 * will actually create the concordance. */
create_concordance(concor,text,tok,n_enter_char,enter_pos,option);
u_fclose(concor);
fclose(text);
free_text_tokens(tok);
u_printf("Done.\n");
return 0;
}


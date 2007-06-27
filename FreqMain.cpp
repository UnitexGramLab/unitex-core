
 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "FreqMain.h"
#include "Unicode.h"
#include "Text_tokens.h"
#include "String_hash.h"
#include "List_int.h"
#include "Alphabet.h"
#include "Matches.h"
#include "Frequency.h"
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
 * This function behaves in the same way as an int main(), except that it does
 * not invoke the setBufferMode function and that it does not print the
 * usage.
 */
int main_Freq(int argc, char **argv) {

if (argc!=2 && argc!=3 && argc!=4) {
	return 0;
}

struct freq_opt option;
option.thai_mode = 0;    /* By default, we are not dealing with Thai */
option.words_only = 0;   /* By default, we are not restricting ourselves only to word tokens */
option.token_limit = 10; /* By default, the context limit is +/-10 tokens */

if (argc!=2) {
	/* We look for -thai */
    if (! strcmp(argv[2],"-thai")) {
		option.thai_mode=1;
	}
    else if (! strcmp(argv[2],"-wordsonly")) {
		option.words_only=1;
	}
	else {
		u_printf( "Invalid option %s\n\n", argv[2] );
		return 1;
	}

	if (argc!=3) {
		/* We look for -thai */
	    if (! strcmp(argv[3],"-thai")) {
			option.thai_mode=1;
		}
	    else if (! strcmp(argv[3],"-wordsonly")) {
			option.words_only=1;
		}
		else {
			u_printf( "Invalid option %s\n\n", argv[3] );
			return 1;
		}
	}
}

/* open snt files we're going to need */
struct snt_files* snt_files=NULL;

char text_snt[FILENAME_MAX];
get_path ( argv[1], text_snt );
snt_files = new_snt_files_from_path(text_snt);

FILE* freq=u_fopen(snt_files->freq,U_WRITE); // the output file
if (freq==NULL) {
   error("Cannot open file %s\n",argv[1]);
   return 1;
}

FILE* text=fopen(snt_files->text_cod,"rb");
if (text==NULL) {
	error("Cannot open file %s\n",snt_files->text_cod);
	u_fclose(freq);
	return 1;
}

FILE* ind=u_fopen(snt_files->concord_ind,"rb");
if (ind==NULL) {
	error("Cannot open file %s\n",snt_files->concord_ind);
	u_fclose(freq);
	fclose(text);
	return 1;
}

struct text_tokens* tok=load_text_tokens(snt_files->tokens_txt);
if (tok==NULL) {
	error("Cannot load text token file %s\n",snt_files->tokens_txt);
	u_fclose(freq);
	u_fclose(ind);
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

/*
 * Once we've parsed all the arguments, we call the function to create the 
 * frequency table 
 */

create_freqtable(freq,text,ind,tok,option);
u_fclose(freq);
fclose(text);
u_fclose(ind);
free_text_tokens(tok);
u_printf("Done.\n");

return 0;
}


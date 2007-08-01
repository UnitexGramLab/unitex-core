 
 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>  *
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
  
#include "Frequency.h"
#include "Unicode.h"
#include "Matches.h"
#include "SortTxtMain.h"
#include "Error.h"
#include "Buffer_ng.h"
#include "StringParsing.h"
#include "Fst2.h"
#include "Stack_int.h"
#include "Snt.h"
#include "Text_tokens.h"

/* Maximum number of new lines in a text. New lines are encoded in
 * 'enter.pos' files. Those files will disappear in the futures */
#define MAX_ENTER_CHAR 1000000
static int enter_pos[MAX_ENTER_CHAR];

/* todo: 
	- order by frequency
	- count the frequency on 
*/

struct stack_int *stack;

int print_freqtable(struct snt_files *snt, struct freq_opt option) {

	FILE* text=fopen(snt->text_cod,"rb");
	if (text==NULL) {
		error("Error: Cannot open file %s\n",snt->text_cod);
		return 1;
	}
	
	FILE* ind=u_fopen(snt->concord_ind,"rb");
	if (ind==NULL) {
		error("Error: Cannot open file %s\n",snt->concord_ind);
		fclose(text);
		return 1;
	}
	
	FILE* fst2=NULL;
	if (option.automata) {
		fst2=u_fopen(snt->text_fst2,"rb");
		if (fst2==NULL) {
			error("Error: Cannot open file %s\n",snt->text_fst2);
			u_fclose(text);
			return 1;
		}
	}
	
	struct text_tokens* tok=load_text_tokens(snt->tokens_txt);
	if (tok==NULL) {
		error("Error: Cannot load text token file %s\n",snt->tokens_txt);
		u_fclose(ind);
		u_fclose(text);
		if (fst2) u_fclose(fst2);
		return 1;
	}
	
	FILE* f_enter=fopen(snt->enter_pos,"rb");
	int n_enter_char;
	if (f_enter==NULL) {
		error("Error: Cannot open file %s\n",snt->enter_pos);
		n_enter_char=0;
	}
	else {
		n_enter_char=fread(&enter_pos,sizeof(int),MAX_ENTER_CHAR,f_enter);
		fclose(f_enter);
	}

	judy freqs = create_freqtable(text,ind,fst2,tok,option);

	if (freqs == NULL) {
		u_fprintf(stderr,"There was a fatal problem while computing frequencies\n");
		return 1;
	}	
	else {
		/* show the results */
		u_printf("Token\tOccurrences\n"
				 "-------------------\n");
	
		Word_t j=0;
		Pvoid_t f;
		
		JLF(f, freqs, j);
		while (f) {
			if ((*(unsigned*)f) >= option.threshold) {
				u_printf("%S\t%d\n",tok->token[j], *(unsigned*)f ); 
			}
			JLN(f, freqs, j);
		}
		
		free_text_tokens(tok);

	}
	
	return 0;
} 


judy create_freqtable( FILE *text,              
                       FILE *ind,
                       FILE *fst2,            
                       struct text_tokens *tok, 
                       struct freq_opt option   ) {

#define INDBUFSIZE 1024
#define RECORDLENGTH 4

	int text_size;
	unichar indbuf[INDBUFSIZE];
	int first_token, last_token;
	unsigned *cod;

	Pvoid_t keys=(Pvoid_t)NULL;
	Pvoid_t freqs=(Pvoid_t)NULL; // judy array
	Word_t  j;                   //      index
	Word_t  l=option.clength;    //      key length
	Pvoid_t f;                   //      iterator

	if (option.automata == 0) {
		/* First, we allocate a buffer and read the "text.cod" file */
		fseek(text,0,SEEK_END); 
		text_size=ftell(text)/RECORDLENGTH;
		fseek(text,0,SEEK_SET);
	
		buffer_ng buf[1];
		buffer_init(buf, 65536, text);
	
		/* We then read the concord.ind file, to get the central tokens */
		rewind(ind);
		u_fgets(indbuf, INDBUFSIZE-1, ind); // the first line does not have any token information
		u_fgets(indbuf, INDBUFSIZE-1, ind);
	
		unsigned *p;
		int i;
	
		stack=new_stack_int(option.clength);
	
		while (! feof(ind) ) {
			u_sscanf(indbuf,"%d %d",&first_token, &last_token);
			cod = (unsigned*)buffer_set_mid(buf,first_token *RECORDLENGTH);
	
			/* count tokens to the left of the central token set */
			for (i=0,p=cod-1; i < option.token_limit; p=(unsigned*)buffer_prev(buf,RECORDLENGTH,RECORDLENGTH) ) {
				if (! p ) {
					break;
				}
				if ( option.sentence_only && *p == (unsigned)tok->SENTENCE_MARKER ) {
					break;
				} 
				if ( (option.words_only && u_is_word(tok->token[*p])) || (! option.words_only) ) {	
					JLI(f,freqs,*p);
					(*(unsigned*)f)++;
					u_printf("i %d: %d\n", *p, (*(unsigned*)f) );
					i++;
				}
			}
			
			/* count tokens to the right of the central token set */
			for (i=0,p=cod+(last_token-first_token)+1; i < option.token_limit; p=(unsigned*)buffer_next(buf,RECORDLENGTH,RECORDLENGTH) ) {
				if (! p ) {
					break;	
				}
				if ( option.sentence_only && *p == (unsigned)tok->SENTENCE_MARKER ) {
					break;
				} 
				if ( (option.words_only && u_is_word(tok->token[*p])) || (! option.words_only) ) {
					JLI(f,freqs,*p);
					(*(unsigned*)f)++;
					u_printf("i %d: %d\n", *p, (*(unsigned*)f) );
					i++;
				}
			}
	
			u_fgets(indbuf, INDBUFSIZE-1, ind);
		}
	}
	else {
		// load_fst2()
	}
	
	return freqs;
	
}



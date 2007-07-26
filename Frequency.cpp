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
  
#include <Judy.h>

#include "Frequency.h"
#include "Unicode.h"
#include "Matches.h"
#include "SortTxtMain.h"
#include "Error.h"
#include "Buffer_ng.h"
#include "StringParsing.h"
#include "Thai.h"
#include "Stack_int.h"

/* todo: 
	- order by frequency
*/

struct stack_int *stack;

void create_freqtable( FILE *freq,              
                       FILE *text,              
                       FILE *ind,               
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
		for (i=0,p=cod-1; i < option.token_limit; p--) {
			if ( ((byte*)p) <  buf->beg ) {
				break;
			}

			if ( option.sentence_only && *p == tok->SENTENCE_MARKER ) {
				break;
			} 
			if ( (option.words_only && u_is_word(tok->token[*p])) || (! option.words_only) ) {	
				JLI(f,freqs,*p);
				(*(unsigned*)f)++;
				i++;
			}
		}
		
		/* count tokens to the right of the central token set */
		for (i=0,p=cod+(last_token-first_token)+1; i < option.token_limit; p++) {
			if ( ((byte*)p) >= buf->end ) {
				break;	
			}

			if ( option.sentence_only && *p == tok->SENTENCE_MARKER ) {
				break;
			} 
			if ( (option.words_only && u_is_word(tok->token[*p])) || (! option.words_only) ) {
				JLI(f,freqs,*p);
				(*(unsigned*)f)++;
				i++;
			}
		}

		u_fgets(indbuf, INDBUFSIZE-1, ind);
	}

	/* show the results */
	u_printf("Token\tOccurrences\n"
			 "-------------------\n");

	j=0;
	JLF(f, freqs, j);
	while (f) {
		if ((*(unsigned*)f) >= option.threshold) {
			u_printf("%S\t%d\n",tok->token[j], *(unsigned*)f ); 
		}
		JLN(f, freqs, j);
	}

}



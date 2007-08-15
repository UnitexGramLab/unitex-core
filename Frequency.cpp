 
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

#include <Judy.h>
typedef Pvoid_t judy;

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
#include "custom_malloc.h"

static freq_entry *new_freq_entry( unsigned freq, int token, unichar *text ) {
	freq_entry *retval;

	custom_malloc( freq_entry, 1, retval );
	
	retval->token = token;
	retval->freq = freq;

	int strlen = u_strlen(text) + 1;
	custom_malloc( unichar, strlen, retval->text );
	u_strcpy( retval->text, text );
	
	return retval;
}

static void free_freq_entry( freq_entry **freq ) {
	if ( freq ) {
		if ( *freq ) {
			if ( (*freq)->text ) free( (*freq)->text );
			free( *freq );
			*freq=NULL;
		}
	}
}

int print_freqtable(judy freqs, unsigned threshold) {
	
	if (! freqs ) {
		u_fprintf(stderr,"There was a fatal problem while computing frequencies\n");
		return 1;
	}
	else {
		/* show the results.
		 * one can do Freq | sort -n to get a sorted list.
		 */
		u_fprintf(stderr,"Frequency\tToken\n"
				         "---------------------\n");

		Word_t j=0;
		Pvoid_t f;
		freq_entry **freq;

		JLF(f, freqs, j); freq=(freq_entry**)(f);
		while (freq) {
			if ((*freq)->freq >= threshold) {
				u_printf("%d\t%S\n", (*freq)->freq, (*freq)->text ); 
			}
			JLN(f, freqs, j); freq=(freq_entry**)(f);
		}
	}

	return 0;
}


judy create_freqtable( struct snt_files *snt, freq_opt option ) {

#define INDBUFSIZE 1024
#define RECORDLENGTH 4

	FILE* text=fopen(snt->text_cod,"rb");
	if (! text) {
		error("Error: Cannot open file %s\n",snt->text_cod);
		return NULL;
	}
	
	FILE* ind=u_fopen(snt->concord_ind,"rb");
	if (! ind) {
		error("Error: Cannot open file %s\n",snt->concord_ind);
		fclose(text);
		return NULL;
	}
	
	FILE* ffst2=NULL;
	if (option.automata) {
		ffst2=u_fopen(snt->text_fst2,"rb");
		if (! ffst2) {
			error("Error: Cannot open file %s\n",snt->text_fst2);
			u_fclose(text);
			return NULL;
		}
	}
	
	struct text_tokens* tok=load_text_tokens(snt->tokens_txt);
	if (! tok) {
		error("Error: Cannot load text token file %s\n",snt->tokens_txt);
		u_fclose(ind);
		u_fclose(text);
		if (ffst2) u_fclose(ffst2);
		return NULL;
	}

	judy freqs=(Pvoid_t)NULL; // judy array
	Pvoid_t f;                //      iterator

	if ( option.automata == 0 ) {
		int text_size;
		unichar indbuf[INDBUFSIZE];
		int first_token, last_token;

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
		freq_entry **freq;

		while (! feof(ind) ) {
			u_sscanf(indbuf,"%d %d",&first_token, &last_token);
			u_printf("%d %d\n", first_token, last_token);
	
			/* count tokens to the left of the central token set */
			p = (unsigned*)buffer_set_mid(buf, (first_token-1) *RECORDLENGTH);
			for (i=0; ; p=(unsigned*)buffer_prev(buf,RECORDLENGTH,RECORDLENGTH) ) {
				if (! p ) { // end of file
					break;
				}
				if ( option.sentence_only && *p == (unsigned)tok->SENTENCE_MARKER ) { // end of sentence
					break;
				} 
				if ( (option.words_only && u_is_word(tok->token[*p])) || (! option.words_only) ) {	
					JLI(f,freqs,*p); freq=(freq_entry**)(f);
					if (! *freq ) { 
						(*freq)=new_freq_entry(0,*p,tok->token[*p]);
					}
					(*freq)->freq++;
					if (++i == option.token_limit) {
						break;
					}
				}
			}
			
			/* count tokens to the right of the central token set */
			p = (unsigned*)buffer_set(buf, (last_token+1)*RECORDLENGTH,RECORDLENGTH);
			for (i=0; ; p=(unsigned*)buffer_next(buf,RECORDLENGTH,RECORDLENGTH) ) {
				if (! p ) { // end of file
					break;	
				}
				if ( option.sentence_only && *p == (unsigned)tok->SENTENCE_MARKER ) { // end of sentence
					break;
				} 
				if ( (option.words_only && u_is_word(tok->token[*p])) || (! option.words_only) ) {
					JLI(f,freqs,*p); freq=(freq_entry**)(f);
					if (! *freq ) {
						(*freq)=new_freq_entry(0,*p,tok->token[*p]);
					}
					(*freq)->freq++;
					if (++i == option.token_limit) {
						break;
					}
				}
			}

			u_fgets(indbuf, INDBUFSIZE-1, ind);
		}
	}
	else {
		Fst2 *sfst2;
		Fst2State state;
		Transition *tran;
		int i,j;
		int ret;
		
		ret=load_fst2_from_file(ffst2,0,&sfst2);
		if ( ret ) {
			u_fprintf(stderr,"Error %d in %s:%d. Fst2 file could not be loaded.\n",ret, __FILE__,__LINE__);
			return 0;
		}

		for (i=1; i<=sfst2->number_of_graphs; i++) { // for every sentence
			j = sfst2->initial_states[i];
			state = sfst2->states[j];
			tran = state->transitions;

			while (state) { // FIXME: we need a way to associate states with token ids.
				if (tran) {
					state=sfst2->states[tran->state_number];
					if (tran->next) u_printf( "[ " );
					u_printf( "%S ", sfst2->tags[tran->tag_number]->input );

					while (tran->next) {
						tran=tran->next;
						u_printf( " | %S", sfst2->tags[tran->tag_number]->input );
						if (! tran->next) u_printf( " ] " );
					}

					tran=state->transitions;
				}
				else {
					state=0;
					u_printf( "\n" );
				}
			}
		}
	}

	free_text_tokens(tok);

	return freqs;
	
}



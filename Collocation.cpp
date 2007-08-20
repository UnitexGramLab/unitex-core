
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

/*
 * Author: Burak Arslan (arslan@univ-mlv.fr, plq@gsulinux.org)
 *         This File contains the collocation extraction api implementation.
 */

#include <time.h>

#include <Judy.h>

#include "Collocation.h"
#include "Buffer_ng.h"
#include "Stack_int.h"
#include "Unicode.h"
#include "defines.h"
#include "Error.h"
#include "Fst2.h"

#define KEYLENGTH 1024

typedef struct {
	unichar can[32];    // canonical form
	unichar gscode[32]; // grammatical or semantic code
	unichar infl[32];   // inflectional information
} tag_t;

static void parse_tag_string ( tag_t *tag, unichar *str ) {
	unichar *p=str, *q=str;
	size_t len;
	if (tag) {
		while (*p) {
			if ( *p == ',' ) {
				if ( *(p+1) == '.' ) {
					len=p-q;
					tag->can[len]=0;
					len*=sizeof(unichar);
					memcpy( tag->can, q, len );
				}
				q=p+1;
			}
			if ( *p == '.' ) {
				len=p-q;
				tag->can[len]=0;
				len*=sizeof(unichar);
				memcpy( tag->can, q, len );
				q=p+1;
			}
			if ( *p == ':' ) {
				len=p-q;
				tag->gscode[len]=0;
				len*=sizeof(unichar);
				memcpy( tag->gscode, q, len );
				q=p+1;
			}
			if ( *p == '}' ) {
				len=p-q;
				tag->infl[len]=0;
				len*=sizeof(unichar);
				memcpy( tag->infl, q, len );
				q=p+1;
			}

			p++;
		}
	}
}

static void comb_l2( Word_t start, struct stack_int *stack, struct stack_int *stack_l1, PPvoid_t retval) {
	if (! stacki_is_full((struct stack_int*)stack) ) {
		PPvoid_t nodes   = (PPvoid_t)stack_l1->stack[start]; // this array is contained in another judy array.
		                         // that's why it's declared as PPvoid_t (which is just a void**) and used 
                                 // always like *nodes.
		Pvoid_t  nodesI  = NULL; // iterator (ie pointer to an element, as in c++ map's iterator->second)
		Pvoid_t  nodesK  = NULL; // key      (   pointer to an element's key, as in c++ map's iterator->first)
		Pvoid_t  nodesS  = NULL; // state of the current iteration. should be NULLified everytime a new
		                         // iteration is attempted.
		Word_t   nodesSL = 0;    // state length. is filled when the state is destroyed, so not of much use.
		Word_t   nodesKL = 0;    // keylength
		Word_t   nodesAL = 0;    // arraylength. is filled when the array is destroyed, so not of much use.

		JHSIF(nodesI, *nodes, nodesS, nodesK, nodesKL);
		while (nodesI)	{
			stacki_push( stack, (Word_t)nodesK);
			comb_l2( start+1, stack, stack_l1, retval);
			stacki_pop( stack );

			JHSIN(nodesI, *nodes, nodesS, nodesK, nodesKL);
		}
        JHSFI(nodesSL, nodesS);
	}
	else { 
		int i=0;
		unichar key[KEYLENGTH], *pkey=key;
		Pvoid_t  retvalI  = NULL;
		Pvoid_t  retvalK  = NULL;
		Word_t   retvalKL = 0;
		
		for (i=0; i<stack->capacity; i++) {
			pkey+=u_sprintf( pkey, "%S ", (unichar *)stack->stack[i] ); // FIXME: need u_snprintf, this may overflow.
		}
		retvalK  = key; 
		retvalKL = pkey-key+1; retvalKL*=sizeof(unichar);
		
		JHSI( retvalI, *retval , retvalK, retvalKL );
		(*((Word_t *)retvalI))++;
	}		
}

static void comb_l1( Word_t arrayK, struct stack_int *stack, Pvoid_t array, PPvoid_t retval ) {

	if (! stacki_is_full((struct stack_int*)stack) ) {
		Pvoid_t arrayI=NULL;

		JLF(arrayI, array, arrayK);
		while (arrayI) {
			stacki_push( stack, (Word_t)arrayI);
			comb_l1  ( arrayK+1, stack, array, retval);
			stacki_pop( stack );
			JLN(arrayI, array, arrayK);
		}
	}
	else {
		struct stack_int *stack_l2=new_stack_int(stack->capacity);

		comb_l2(0, stack_l2, stack, retval);
		
		free_stack_int(stack_l2);
	}
}

int colloc_print(Pvoid_t array, unsigned threshold) {

	if (! array ) {
		u_fprintf(stderr,"%s() in %s:%d was passed a null pointer.\n", __FUNCTION__, __FILE__, __LINE__ );
		return 1;
	}
	else {
		/* show the results.
		 * one can pipe the output to "sort -n" to get a sorted list.
		 */
		Pvoid_t  arrayI  = NULL;
		Pvoid_t  arrayK  = NULL;
		Pvoid_t  arrayS  = NULL;		                        
		Word_t   arraySL = 0;
		Word_t   arrayKL = 0;
		Word_t   arrayAL = 0;

		u_fprintf(stderr,"Frequency\tCollocation\n"
		                 "---------------------------\n");

		JHSIF(arrayI, array, arrayS, arrayK, arrayKL);
		while (arrayI)	{
			if ( (*((Word_t*)arrayI)) >= threshold ) {
				u_printf("%9d\t%S\n", *((Word_t*)arrayI), (unichar*)arrayK );
			}

			JHSIN(arrayI, array, arrayS, arrayK, arrayKL);
		}
        JHSFI(arraySL, arrayS);

	}

	return 0;
}

Pvoid_t colloc_generate_candidates( struct snt_files *snt, colloc_opt option ) {

	Fst2 *sfst2=NULL;
	Fst2State state;
	Transition *tran=NULL;
	Pvoid_t retval=NULL;

	struct stack_int *stack=new_stack_int(2); // start by generating combinations of 2. we'll then try to combine them.

	unichar *input, *c, *d;

	int i,j,index=0,prev_i;
	int ret;

	FILE* ffst2=NULL;
	ffst2=u_fopen(snt->text_fst2,"rb");
	if (! ffst2) {
		error( "Error: Cannot open file %s\n", snt->text_fst2 );
		return NULL;
	}
	
	u_printf("Loading %s ...\n", snt->text_fst2);
	ret=load_fst2_from_file(ffst2,0,&sfst2);
	if ( ret ) {
		u_fprintf(stderr,"Error %d in %s:%d. Fst2 file could not be loaded.\n", ret, __FILE__,__LINE__);
		return 0;
	}

	Pvoid_t  sentence   = NULL;
	Pvoid_t  sentenceI  = NULL;
	Word_t   sentenceK  = 0;
	int      sentenceAL = 0;

	PPvoid_t nodes   = NULL;
	Pvoid_t  nodesI  = NULL;
	Pvoid_t  nodesK  = NULL;
	Pvoid_t  nodesS  = NULL;
	Word_t   nodesSL = 0;
	Word_t   nodesKL = 0;
	Word_t   nodesAL = 0;

	u_fprintf(stderr,"Generating collocation candidates...\n");
	time_t ptime=0,ctime,stime;

	ctime=time(&stime);

	for (i=1; i<=sfst2->number_of_graphs; i++) { // for every sentence
		j = sfst2->initial_states[i];
		state = sfst2->states[j];
		tran = state->transitions;

		sentenceK=0;

		if ( (time(&ctime)-ptime) ) {
			u_fprintf(stderr,"Sentence %8d/%d, %4.3f sentences per second, still ~%8.3f seconds to go.\r",
			               i, sfst2->number_of_graphs, ((float)i) / (ctime-stime) , 
			               (ctime-stime) * (sfst2->number_of_graphs -i) / ((float)i)
			         );
			ptime=ctime;
			prev_i=i;
		}

		while (state) { // here we try to group transitions whose terminal states are the same. FIXME: need confirmation :-)
			if (tran) {
				state=sfst2->states[tran->state_number];
				JLI(sentenceI, sentence, sentenceK );
				sentenceK++;
				nodes=(PPvoid_t)sentenceI;

				while (tran) { 
					input=sfst2->tags[tran->tag_number]->input;
					if (input[0]=='{') {
						tag_t tag;
						parse_tag_string( &tag, input+1 );

						c=u_strchr( input, ',' )+1;
						d=u_strchr(     c, '.' );

						if ( c == d ) c =input +1;
						d=u_strchr(     c, '}' );

						if (d) {
							nodesKL=d-c; nodesKL++; nodesKL*=sizeof(unichar);
							nodesK=(Pvoid_t)c;
							*d=0;
						}
						else {
							u_fprintf (stderr, "format error in %s, state %d transition %d: %S\n", 
							                   snt->text_fst2, tran->state_number, tran->tag_number, input );
							exit(1);
						}
					}
					else {
						nodesKL=(u_strlen(input)+1)*sizeof(unichar);
						nodesK=(Pvoid_t)input;
					}

					if ( (! option.wonly) || (option.wonly && u_is_word((unichar *)nodesK)) ) { // FIXME: u_isword not working as expected, need something more elaborate in order to do the filtering here. 
						JHSI( nodesI, *nodes , nodesK, nodesKL );
						if (! (*((int*)nodesI)) ) *((int*)nodesI)=++index;
					}

					if (d) { // fixing the above null termination
						*d='}';
						d=NULL;
					}					

					tran=tran->next;
				}

				tran=state->transitions;
			}
			else {
				state=0;
			}
		}

		comb_l1( 0, stack, sentence, &retval );
		// loop that frees the judy array
		sentenceK=0;
		JLF(sentenceI, sentence, sentenceK);
		while (sentenceI) {
			nodes=(PPvoid_t)sentenceI;
			JHSFA( nodesAL, *nodes);

			JLN(sentenceI, sentence, sentenceK);
		}
		JLFA( sentenceAL, sentence );
	}

	u_fprintf(stderr, "Sentence %9d/%d, %4.3f sentences per second.                                     \n",
	                  i-1, sfst2->number_of_graphs, ((float)i-1) / (time(&ctime)-stime)
	         );

	free_stack_int(stack);
	return retval;

}


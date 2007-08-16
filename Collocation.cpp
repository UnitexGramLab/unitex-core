
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
 *         This File contains the collocation extraction api functions.
 * 
 */


#include <Judy.h>

#include "Collocation.h"
#include "Buffer_ng.h"
#include "Stack_int.h"
#include "Unicode.h"
#include "defines.h"
#include "Error.h"
#include "Fst2.h"

typedef Pvoid_t judy;

judy colloc_candidates( struct snt_files *snt, colloc_opt option ) {

	Fst2 *sfst2=NULL;
	Fst2State state;
	Transition *tran=NULL;
	judy retval=NULL;

	struct stack_int *stack=new_stack_int(option.clength);

	unichar *input, *c, *d;

	int i,j,index=0;
	int ret;

	FILE* ffst2=NULL;
	ffst2=u_fopen(snt->text_fst2,"rb");
	if (! ffst2) {
		error( "Error: Cannot open file %s\n", snt->text_fst2 );
		return NULL;
	}
	
	ret=load_fst2_from_file(ffst2,0,&sfst2);
	if ( ret ) {
		u_fprintf(stderr,"Error %d in %s:%d. Fst2 file could not be loaded.\n", ret, __FILE__,__LINE__);
		return 0;
	}

	Pvoid_t  sentence   = NULL;
	Pvoid_t  sentenceI  = NULL;
	Word_t   sentenceK  = 0;
	int      sentenceAL = 0;

	PPvoid_t nodes   = NULL; // this array is contained in another judy array. that's why it's declared
	                         // as PPvoid_t (which is just a void**) and used by dereferencing once.
	Pvoid_t  nodesI  = NULL; // iterator (ie pointer to an element, as in c++ map's iterator->second)
	Pvoid_t  nodesK  = NULL; // key      (   pointer to an element's key, as in c++ map's iterator->first)
	Pvoid_t  nodesS  = NULL; // state of the current iteration. should be NULLified everytime a new
	                         // iteration is attempted.
	Word_t   nodesSL = 0;    // state length. is filled when the state is destroyed, so not of much use.
	Word_t   nodesKL = 0;    // keylength
	int      nodesAL = 0;    // arraylength. is filled when the array is destroyed, so not of much use.
	
	for (i=1; i<=sfst2->number_of_graphs; i++) { // for every sentence
		j = sfst2->initial_states[i];
		state = sfst2->states[j];
		tran = state->transitions;

		sentenceK=0;

		while (state) { // here we try to group transitions that  common step.
			if (tran) {
				state=sfst2->states[tran->state_number];
				JLI(sentenceI, sentence, sentenceK );
				sentenceK++;
				nodes=(PPvoid_t)sentenceI;

				while (tran) { 
					input=sfst2->tags[tran->tag_number]->input;
					if (input[0]=='{') {
						c=u_strchr( input, ',' )+1;
						d=u_strchr(     c, '.' );

						if (c == d) c =input +1;
						d=u_strchr(     c, '}' );						

						if (d) {
							nodesKL=d-c; nodesKL++; nodesKL*=sizeof(unichar);
							nodesK=(Pvoid_t)c;
							*d=0;
						}
						else {
							u_printf ( "format error in %s, state %d transition %d: %S\n", 
							           snt->text_fst2, tran->state_number, tran->tag_number, input );
							exit(1);
						}
					}
					else {
						nodesKL=(u_strlen(input)+1)*sizeof(unichar);
						nodesK=(Pvoid_t)input;
					}

					JHSI( nodesI, *nodes , nodesK, nodesKL );
					if (! (*((int*)nodesI)) ) *((int*)nodesI)=++index;

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

		sentenceK=0;
		JLF(sentenceI, sentence, sentenceK);
		while (sentenceI) {
			u_printf("%9lu: ", sentenceK);

			nodes=(PPvoid_t)sentenceI;
			nodesKL = 0;
			nodesS  = NULL;

			JHSIF(nodesI, *nodes, nodesS, nodesK, nodesKL);
			while (nodesI)	{
				u_printf("%S ", (unichar *)nodesK );

				JHSIN(nodesI, *nodes, nodesS, nodesK, nodesKL);
			}
	        JHSFI(nodesSL, nodesS);
			u_printf("\n");

			JLN(sentenceI, sentence, sentenceK);
		}

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

	free_stack_int(stack);

	return retval;

}


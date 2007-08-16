
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

/*
 * Author: Burak Arslan (arslan@univ-mlv.fr, plq@gsulinux.org)
 *         This File contains the collocation extraction api functions.
 * 
 */


#include <Judy.h>

#include "custom_malloc.h"
#include "Collocation.h"
#include "Buffer_ng.h"
#include "Unicode.h"
#include "Error.h"
#include "Fst2.h"

typedef Pvoid_t judy;

judy colloc_candidates( struct snt_files *snt, colloc_opt option ) {

	Fst2 *sfst2=NULL;
	Fst2State state;
	Transition *tran=NULL;
	judy retval=NULL;

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

	Pvoid_t  sentence=NULL;
	Pvoid_t  sentenceI;
	Word_t   sentenceK;  
	Word_t   sentenceAL;

	PPvoid_t nodes=NULL;
	Pvoid_t  nodesI;  // iterator
	Pvoid_t  nodesK;  // key
	Pvoid_t  nodesS;  // state
	Word_t   nodesSL; // state length
	Word_t   nodesKL; // keylength
	Word_t   nodesAL; // arraylength
	

	for (i=1; i<=sfst2->number_of_graphs; i++) { // for every sentence
		j = sfst2->initial_states[i];
		state = sfst2->states[j];
		tran = state->transitions;

		sentenceK=0;

		while (state) { 
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
			nodesKL=0;
			nodesS=NULL;

			JHSIF(nodesI, *nodes, nodesS, nodesK, nodesKL);
			while (nodesI)	{
				u_printf("%S ", (unichar *)nodesK );

				JHSIN(nodesI, *nodes, nodesS, nodesK, nodesKL);
			}
			u_printf("\n");

			JLN(sentenceI, sentence, sentenceK);
		}

		// freeing loop
		sentenceK=0;
//		u_printf ("freed ");
		JLF(sentenceI, sentence, sentenceK);
		while (sentenceI) {
			nodes=(PPvoid_t)sentenceI;
			JHSFA( nodesAL, *nodes);
//			u_printf("%lu ", nodesAL);
			
			JLN(sentenceI, sentence, sentenceK);
		}
		JLFA( sentenceAL, sentence );
//		u_printf("%lu bytes.\n", sentenceAL);

		u_printf(" - -- --- ==== ===== ==== --- -- - \n");
		if (i>3) break;
	}

//u_printf("%d\n", index); exit(0);

/*
	Pvoid_t iter     = NULL;  // JudyHS iterator
	Pvoid_t Index2   = NULL;  // JudyHS key: line
	Word_t Length2   =    0;  // length of key: start at 0
	Pvoid_t PValue2;          // pointer to value
	Word_t IterBytes;         // size of iterator

	JHSIF(PValue2, retval, iter, Index2, Length2);
	while (PValue2)	{
		u_printf("%8d: %S\n", *((int*)PValue2), (unichar *)Index2 );

		JHSIN(PValue2, retval, iter, Index2, Length2);
	}
*/

	return retval;

}


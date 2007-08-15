
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

#include "custom_malloc.h"
#include "Collocation.h"
#include "Buffer_ng.h"
#include "Unicode.h"
#include "Error.h"
#include "Fst2.h"

typedef Pvoid_t judy;

#define KEYLENGTH 256

judy colloc_candidates( struct snt_files *snt, colloc_opt option ) {

	Fst2 *sfst2=NULL;
	Fst2State state;
	Transition *tran=NULL;
	judy retval=NULL;

	Pvoid_t   PValue;                   // Judy array element pointer.

	unichar *input, *c, *d, key[KEYLENGTH], *fkey;

	int i,j,l,index=0;
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

	for (i=1; i<=sfst2->number_of_graphs; i++) { // for every sentence
		j = sfst2->initial_states[i];
		state = sfst2->states[j];
		tran = state->transitions;

		while (state) { 
			if (tran) {
				state=sfst2->states[tran->state_number];

				while (tran) {
					input=sfst2->tags[tran->tag_number]->input;
					if (input[0]=='{') {
						c=u_strchr( input, ',' )+1;
						d=u_strchr(     c, '.' );

						if (c == d) c =input +1;
						d=u_strchr(     c, '}' );						

						if (d) {
							l=d-c;
							if ( l >= KEYLENGTH ) {
								u_printf( "key string too long in %s, state %d transition %d: %S\n",
								          snt->text_fst2, tran->state_number, tran->tag_number, input );
								exit(1);
							}
							else {
								memcpy(key, c, sizeof(unichar) * l); 
								key[l]=0;
								fkey=key;
							}
						}
						else {
							u_printf ( "format error in %s, state %d transition %d: %S\n", 
							           snt->text_fst2, tran->state_number, tran->tag_number, input );
							exit(1);
						}
					}
					else {
						l=u_strlen(input);
						fkey=input;
					}
					
					JHSI( PValue, retval, fkey, sizeof(unichar) * (l+1) ); 
					if (! (*((int*)PValue)) ) *((int*)PValue)=++index;
					
//					u_printf( "%S ", fkey );
					tran=tran->next;
				}
//				u_printf( "\n" );

				tran=state->transitions;
			}
			else {
				state=0;
			}
		}
//		u_printf("\n");
	}

//u_printf("%d\n", index); exit(0);

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


	return retval;

}


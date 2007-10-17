
 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include <Judy.h>
#include <time.h>
#include <unistd.h>

#include "Collocation.h"
#include "Buffer_ng.h"
#include "Stack_int.h"
#include "defines.h"
#include "Error.h"
#include "Fst2.h"

#define KEYLENGTH 1024

typedef struct {
	unichar can[32];        // canonical form
	unichar gscode[32][32]; // grammatical or semantic code
	unichar infl[32];       // inflectional information
} tag_t;

static Word_t cnum=0,cnumu=0,thrash=0;

unichar *readfile(char *name) {
	FILE *f=u_fopen (name, "r");
	unichar *retval;
	size_t size;

	if (f) {
		fseek(f, 0, SEEK_END);
		size=ftell(f);
		fseek(f, 2, SEEK_SET);
		
		custom_malloc( unichar, size, retval );
		memset( retval, 0, size );
		fread( retval, 1, size, f );

		fclose(f);
	}
	else {
		retval=NULL;
	}

	return retval;
}

static void parse_tag_string ( tag_t *tag, unichar *str ) { // TODO: do boundary checks.
	unichar *p=str, *q=str;
	size_t len;
	Word_t col=0,free=1,s=0;

	if (tag) {
		while (*p) {
			if ( *p == ',' && free ) {
				if ( *(p+1) == '.' ) {
					len=p-q;
					tag->can[len]=0;
					len*=sizeof(unichar);
					memcpy( tag->can, q, len );
				}
				q=p+1;
			}
			if ( *p == '.' && free ) {
				len=p-q;
				tag->can[len]=0;
				len*=sizeof(unichar);
				memcpy( tag->can, q, len );
				q=p+1;
			}
			if ( (*p == '+' || *p == ':') && free ) {
				len=p-q;
				tag->gscode[s][len]=0;
				len*=sizeof(unichar);
				memcpy( tag->gscode[s], q, len );
				q=p+1;

				col=1;
				s++;
			}
			if ( *p == '}' && free ) {
				len=p-q;
	
				if (col) {
					tag->infl[len]=0;
					len*=sizeof(unichar);
					memcpy( tag->infl, q, len );
				}
				else {
					tag->gscode[s][len]=0;
					len*=sizeof(unichar);
					memcpy( tag->gscode[s], q, len );
					*tag->infl = 0;
				}
				q=p+1;
			}
			if ( *p == '\\' ) {
				free=0;
			}
			else {
				free=1;
			}

			p++;
		}
	}
}

static void comb_l2( Word_t start, struct stack_int *stack, struct stack_int *stack_l1, Parray_t retval) {
	if (! stacki_is_full((struct stack_int*)stack) ) {
		PPvoid_t nodes=(PPvoid_t)stack_l1->stack[start]; 
		JUDYHSH(nodes);

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
		size_t retvalKL, retvalDL=sizeof(Word_t);
		Word_t *retvalD, value;
		int ret;

		if ( u_strcmp((unichar *)stack->stack[0], (unichar *)stack->stack[1] ) ) { 
			pkey+=u_sprintf( pkey, "%S ", (unichar *)stack->stack[1] ); // FIXME: need u_snprintf, this may overflow.
			pkey+=u_sprintf( pkey, "%S",  (unichar *)stack->stack[0] ); // FIXME: need u_snprintf, this may overflow.

			retvalKL  = pkey-key+1; 
			retvalKL *= sizeof(unichar);
			if ( array_get(retval, key, retvalKL, (void**)(&retvalD), &retvalDL ) ) {
				pkey =key;
				pkey+=u_sprintf( pkey, "%S ", (unichar *)stack->stack[0] ); // FIXME: need u_snprintf, this may overflow.
				pkey+=u_sprintf( pkey, "%S",  (unichar *)stack->stack[1] ); // FIXME: need u_snprintf, this may overflow.
				if ( array_get(retval, key, retvalKL, (void**)(&retvalD), &retvalDL ) ) {
					cnumu++;
					retvalD=&value;
					value=0;
				}
			}
 
			(*retvalD)++;
			array_set( retval, key, retvalKL, (void*)(retvalD), retvalDL );
			cnum++;
		}
	}
}

static void comb_l1( Word_t arrayK, struct stack_int *stack, Pvoid_t array, Parray_t retval ) {

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

int colloc_print(array_t array, colloc_opt option) {

	if (! array ) {
		u_fprintf(stderr,"%s() in %s:%d was passed a null pointer.\n", __FUNCTION__, __FILE__, __LINE__ );
		return 1;
	}
	else { /* one can pipe this output to "sort -n" to get a sorted list. */
		if (! option.quiet) {
			u_fprintf(stderr,"Frequency\tCollocation\n"
			                 "---------------------------\n");
		}
		arrayI_t arrayK,arrayD;
		foreach (array, arrayK, arrayD) {
			if ( (*((Word_t*)arrayD.data)) > option.threshold ) {
#ifdef DEBUG_STAT
				u_printf("[outp]\t%9d\t%S\n", 
#else
				u_printf("%9d\t%S\n", 
#endif

					*((Word_t*)(arrayD.data)), (unichar*)(arrayK.data) 
				);
			}
			else { 
				thrash++;
			}
		} end_foreach(array, arrayK, arrayD);

		if (! option.quiet) u_fprintf(stderr,"\n%d out of %d were below the threshold (%d).\n", thrash, cnumu, option.threshold);

	}

	return 0;
}

int colloc_compact(Parray_t array, unsigned threshold, int quiet) {

	if (! ( array && *array) ) {
		u_fprintf(stderr,"%s() in %s:%d was passed a null pointer.\n", __FUNCTION__, __FILE__, __LINE__ );
		return 1;
	}
	else { /* one can pipe this output to "sort -n" to get a sorted list. */
		arrayI_t arrayD, arrayK;
		foreach (*array, arrayK, arrayD) {
			if ( (*((Word_t*)(arrayD.data))) <= threshold ) {
				thrash++;
				array_del( array, arrayK.data, arrayK.size );
			} 
		} end_foreach(*array, arrayK, arrayD);

		if (! quiet) u_fprintf(stderr,"\n%d out of %d were below the threshold (%d).\n", thrash, cnumu, threshold);

	}

	return 0;
}

#define print_status \
	u_fprintf(stderr,"Snt %8d/%d, %4.3f snt/s, %8d/%8d comb. so far. still ~%8.3f sec. to go. \r", \
					i, sfst2->number_of_graphs, ((float)(i-prev_i)) / ((float)(ctime-ptime)) , cnum, cnumu, \
					(ctime-stime) * (end -i) / ((float)i) \
				) 

array_t colloc_generate_candidates( struct snt_files *snt, colloc_opt option ) {

	Fst2 *sfst2=NULL;
	Fst2State state;
	Transition *tran=NULL;

	struct stack_int *stack=new_stack_int(2); // start by generating combinations of 2. we'll then try to combine them.
	                                          // if this changes, comb_l2 should be adjusted accordingly.
	unichar *input, *c, *d,key[KEYLENGTH];

	int i,j,index=0,prev_i;
	int ret;

	if (option.rstart > option.rend) {
		error("Error: option.rstart > option.rend. \n");
		return NULL;
	}

	FILE* ffst2=NULL;
	ffst2=u_fopen(snt->text_fst2,"rb");
	if (! ffst2) {
		error( "Error: Cannot open file %s\n", snt->text_fst2 );
		return NULL;
	}
	
	if (! option.quiet) u_fprintf(stderr,"Loading %s ...\n", snt->text_fst2);
	ret=load_fst2_from_file(ffst2,0,&sfst2);
	if ( ret ) {
		u_fprintf(stderr,"Error %d in %s:%d. Fst2 file could not be loaded.\n", ret, __FILE__, __LINE__);
		return 0;
	}

	Pvoid_t sentence=NULL;
	JUDYLH(sentence);

	PPvoid_t nodes=NULL;
	JUDYHSH(nodes);

	array_t retval;
	array_init(&retval);

	if ( option.compact && (! option.threshold) ) return NULL;

	if (! option.quiet) u_fprintf(stderr,"Generating collocation candidates...\n");
	time_t ptime=0,ctime,stime;

	unsigned start, end;

	ctime=time(&stime);
	if (option.rstart) {
		start = option.rstart;
		if (option.rend > sfst2->number_of_graphs ) {
			end = sfst2->number_of_graphs;
		}
		else {
			end = option.rend;
		}
	}
	else {
		start = 1;
		end   = sfst2->number_of_graphs;
	}
	
	for (i=start; i<=end; i++) { // for every sentence
		j = sfst2->initial_states[i];
		state = sfst2->states[j];
		tran = state->transitions;

		sentenceK=0;
#ifdef DEBUG_STAT
		u_printf("[stat]\t%9d\t%9d\t%9d\n", i, cnum, cnumu );
#endif
		if ( (time(&ctime)-ptime) && (! option.quiet) ) {
			print_status;

			ptime=ctime;
			prev_i=i;
		}

		while (state) { // here we try to group transitions whose terminal states are the same.
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

						unichar **spos = option.spos;
						if (spos) {
							while( *spos ) { 
								if (! u_strcmp( *spos, tag.gscode[0] ) ) break; // TODO: should compare other labels as well
								spos++;
							}
						}

						unichar **sword = option.swords;
						if ( (! spos) || (spos && (! *spos) ) )  { // check if it does not match above
							if (sword) {
								while( *sword ) { 
									if (! u_strcmp( *sword, tag.can ) ) break;
									sword++;
								}
							}
						}

						if ( (sword && *sword) || (spos && *spos) ) {
							nodesK=NULL;
						}
						else {
							c=u_strchr( input, ',' ) + 1;
							d=u_strchr(     c, '.' );

							if ( c == d ) c=input+1;
							if (option.level == 0) {
								d=u_strchr( c, '.' );
								if (! d) fatal_error( "format error in %s, state %d transition %d: %S\n",
								                       snt->text_fst2, tran->state_number, tran->tag_number, input );
							}
							if ( (! d) || option.level == 1 ) d=u_strchr( c, '+' );
							if ( (! d) || option.level == 2 ) d=u_strchr( c, ':' );
							if ( (! d) || option.level >= 3 ) d=u_strchr( c, '}' );

							if (d) {
								nodesKL=0;
								while (c!=d) {
									if (*c != ',') key[nodesKL++] = *c;

									if (nodesKL==KEYLENGTH) {
										fatal_error( "Node key is too long (>%d) in %s:%d. bailing out.\n", KEYLENGTH, __FILE__, __LINE__ );									
									}
									c++;
								}
								key[nodesKL++]=0;
								nodesKL*=sizeof(unichar);

								nodesK=(Pvoid_t)key;
							}
							else {
								fatal_error( "format error in %s, state %d transition %d: %S\n",
								              snt->text_fst2, tran->state_number, tran->tag_number, input );								
							}
						}
					}
					else {
						if ( u_is_word(input) ) {
							unichar **sword = option.swords;
							if (sword) {
								while( *sword ) { 
									if (! u_strcmp( *sword, input ) ) break;
									sword++;
								}
							}

							if ( sword && *sword ) {
								nodesK=NULL;
							}
							else {
								nodesKL=(u_strlen(input)+1)*sizeof(unichar);
								nodesK=(Pvoid_t)input;
							}
						}
						else {
							nodesK=NULL;
						}

					}

					if (nodesK) {
						c=u_strchr( (unichar *)nodesK, '.');
						if (c) *c=0;
						if ( (! u_is_word((unichar *)nodesK)) || u_strchr((unichar*)nodesK, ' ') ) {

						}
						else {
							if (c) *c='.'; 
							
							JHSI( nodesI, *nodes, nodesK, nodesKL );
							if (! (*((int*)nodesI)) ) *((int*)nodesI)=++index;
						}
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

		// loop that frees the temporary judy array
		sentenceK=0;
		JLF(sentenceI, sentence, sentenceK);
		while (sentenceI) {
			nodes=(PPvoid_t)sentenceI;
			JHSFA( nodesAL, *nodes);

			JLN(sentenceI, sentence, sentenceK);
		}
		JLFA( sentenceAL, sentence );

		if (option.compact) {
			if (! (i % option.compact) ) {
				print_status;

				if (! option.quiet) u_fprintf (stderr, "\ncompacting...");
				colloc_compact( &retval, (i-start) * option.threshold/end, option.quiet );

				ptime=ctime;
				prev_i=i;
			}
		}
	}

	if (! option.quiet) {
		u_fprintf(stderr, "Sentence %9d/%d, %4.3f sentences per second, %d combinations processed, of which %d unique.       \n",
		                  i-1, sfst2->number_of_graphs, ((float)i-1) / (time(&ctime)-stime), cnum, cnumu
		         );
	}

	free_stack_int(stack);
	return retval;
}


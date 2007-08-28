
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

/*
 * tip: think that the code is confusing with all those ifdefs around? try
 *      gcc -E Collocation.cpp [ -DBDB ] | less -S  
 */

#include <Judy.h>
#include <time.h>

#include "Collocation.h"
#include "Buffer_ng.h"
#include "Stack_int.h"
#include "defines.h"
#include "Error.h"
#include "Fst2.h"

#define KEYLENGTH 1024

// TODO: a function to free the collocation candidates frequency table

typedef struct {
	unichar can[32];        // canonical form
	unichar gscode[32][32]; // grammatical or semantic code
	unichar infl[32];       // inflectional information
} tag_t;

static Word_t cnum=0,cnumu=0,thrash=0;

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

#ifdef BDB
		DBT retvalK, retvalD;
		Word_t value=0;
		int ret;
#define retvalI (retvalD.data)

#else
		JUDYHSH(retval);
#endif

		// concats here may still cause problems in case of merging 2+ sessions of computation output. must merge with care.
		if ( u_strcmp((unichar *)stack->stack[0], (unichar *)stack->stack[1] ) ) { 
			pkey+=u_sprintf( pkey, "%S ", (unichar *)stack->stack[0] ); // FIXME: need u_snprintf, this may overflow.
			pkey+=u_sprintf( pkey, "%S ", (unichar *)stack->stack[1] ); // FIXME: need u_snprintf, this may overflow.

#ifdef BDB
			/* Zero out the DBTs before using them. */
			memset( &retvalK, 0, sizeof(DBT) );
			retvalK.data = key;
			retvalK.size = pkey-key+1; retvalK.size*=sizeof(unichar);

			memset( &retvalD, 0, sizeof(DBT) );
			retvalD.data = &value;
			retvalD.size = sizeof(value);

			if (ret) {
#else
			retvalK  = key; 
			retvalKL = pkey-key+1; retvalKL*=sizeof(unichar);

			JHSG( retvalI, *retval , retvalK, retvalKL );

			if (! retvalI) { 
#endif
				pkey =key;
				pkey+=u_sprintf( pkey, "%S ", (unichar *)stack->stack[1] ); // FIXME: need u_snprintf, this may overflow.
				pkey+=u_sprintf( pkey, "%S ", (unichar *)stack->stack[0] ); // FIXME: need u_snprintf, this may overflow.
#ifdef BDB
				ret = (*retval)->get(*retval, NULL, &retvalK, &retvalD, 0);
				if (ret) {
					memset( &retvalD, 0, sizeof(DBT) );
					value=0;
					retvalD.data = &value;
					retvalD.size = sizeof(value);

					(*retval)->put(*retval, NULL, &retvalK, &retvalD, 0);
				}
#else
				JHSI( retvalI, *retval, retvalK, retvalKL );
#endif
			}

            if (!(*((Word_t *)retvalI))) cnumu++;
			(*((Word_t *)retvalI))++;

#ifdef BDB
			(*retval)->put( *retval, NULL, &retvalK, &retvalD, 0 );

#undef retvalI
#endif

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

int colloc_print(array_t array, unsigned threshold) {

	if (! array ) {
		u_fprintf(stderr,"%s() in %s:%d was passed a null pointer.\n", __FUNCTION__, __FILE__, __LINE__ );
		return 1;
	}
	else { /* one can pipe this output to "sort -n" to get a sorted list. */
#ifdef BDB
		DBC *arrayC; // database cursor
		DBT arrayKey, arrayData;
		int ret;
//		memset(&arrayKey, 0, sizeof(arrayKey));

		arrayKey.flags=0;
		arrayData.flags=0;
#define arrayI (arrayData.data)
#define arrayK (arrayKey.data)

#else
		JUDYHSH(array);
#endif

		u_fprintf(stderr,"Frequency\tCollocation\n"
		                 "---------------------------\n");
#ifdef BDB
		array->cursor( array, NULL, &arrayC, 0);

		/* Iterate over the database, retrieving each record in turn. */
		while ((ret = arrayC->get(arrayC, &arrayKey, &arrayData, DB_NEXT)) == 0) {
#else
		JHSIF(arrayI, array, arrayS, arrayK, arrayKL);
		while (arrayI)	{
#endif

			if ( (*((Word_t*)arrayI)) > threshold ) {
				u_printf("%9d\t%S\n", *((Word_t*)arrayI), (unichar*)arrayK );
			}
			else { 
				thrash++;
			}
#ifndef BDB
			JHSIN(arrayI, array, arrayS, arrayK, arrayKL);
#endif
		}

#ifdef BDB
		if (ret != DB_NOTFOUND) {
			u_printf("There was a problem with the bdb in %s() %s:%d. Bailing out.\n", __FUNCTION__, __FILE__, __LINE__); 
			exit(1);
		}

		/* Cursors must be closed */
		if (arrayC) arrayC->close(arrayC);
#undef arrayK 
#undef arrayI

#else
        JHSFI(arraySL, arrayS);

#endif

		u_fprintf(stderr,"\n%d were below the threshold (%d).\n", thrash, threshold);

	}

	return 0;
}

int colloc_compact(array_t array, unsigned threshold) {

	if (! array ) {
		u_fprintf(stderr,"%s() in %s:%d was passed a null pointer.\n", __FUNCTION__, __FILE__, __LINE__ );
		return 1;
	}
	else { /* one can pipe this output to "sort -n" to get a sorted list. */
#ifdef BDB
		DBC *arrayC; // database cursor
		DBT arrayKey, arrayData;
		int ret;

		arrayKey.flags=0;
		arrayData.flags=0;
#define arrayI (arrayData.data)
#define arrayK (arrayKey.data)

#else
		JUDYHSH(array);
#endif

#ifdef BDB
		array->cursor( array, NULL, &arrayC, 0);

		/* Iterate over the database, retrieving each record in turn. */
		while ((ret = arrayC->get(arrayC, &arrayKey, &arrayData, DB_NEXT)) == 0) {
#else
		JHSIF(arrayI, array, arrayS, arrayK, arrayKL);
		while (arrayI)	{
#endif

			if ( (*((Word_t*)arrayI)) <= threshold ) {
				thrash++;
#ifdef BDB
			    array->del(array, NULL, &arrayKey, 0);
#else
				JHSD(arrayR, array, arrayK, arrayKL);
#endif

			}
#ifndef BDB
			JHSIN(arrayI, array, arrayS, arrayK, arrayKL);
#endif
		}

#ifdef BDB
		if (ret != DB_NOTFOUND) {
			u_printf("There was a problem with the bdb in %s() %s:%d. Bailing out.\n", __FUNCTION__, __FILE__, __LINE__); 
			exit(1);
		}

		/* Cursors must be closed */
		if (arrayC) arrayC->close(arrayC);
#undef arrayK 
#undef arrayI

#else
        JHSFI(arraySL, arrayS);

#endif

		u_fprintf(stderr,"\n%d were below the threshold (%d).\n", thrash, threshold);

	}

	return 0;
}



array_t colloc_generate_candidates( struct snt_files *snt, colloc_opt option ) {

	Fst2 *sfst2=NULL;
	Fst2State state;
	Transition *tran=NULL;

	struct stack_int *stack=new_stack_int(2); // start by generating combinations of 2. we'll then try to combine them.
	                                          // if this changes, comb_l2 should be adjusted accordingly.
	unichar *input, *c, *d,key[KEYLENGTH];

	int i,j,index=0,prev_i;
	int ret;

    /*
	 * array initialization
	 */
#ifdef BDB
    /* Initialize our handles */
    DB *retval = NULL;
    DB_ENV *retvalE = NULL;
    DB_MPOOLFILE *retvalPF = NULL;

    int ret_t; 
    const char *db_name = "db";
    u_int32_t open_flags;

/* Create the environment */
    ret = db_env_create(&retvalE, 0);
    if (ret != 0) {
        fprintf(stderr, "Error creating environment handle: %s\n",
            db_strerror(ret));
        exit(1);
    }

    open_flags =
      DB_CREATE     |  /* Create the environment if it does not exist */
      DB_INIT_MPOOL |  /* Initialize the memory pool (in-memory cache) */
      DB_PRIVATE;      /* Region files are not backed by the filesystem. 
                        * Instead, they are backed by heap memory.  */

    /* 
     * Specify the size of the in-memory cache. 
     */
    ret = retvalE->set_cachesize(retvalE, 0, 256 * 1024 * 1024, 1);
    if (ret != 0) {
        fprintf(stderr, "Error increasing the cache size: %s\n",
            db_strerror(ret));
        exit(1);
    }

    /* 
     * Now actually open the environment. Notice that the environment home
     * directory is NULL. This is required for an in-memory only
     * application. 
     */
    ret = retvalE->open(retvalE, NULL, open_flags, 0);
    if (ret != 0) {
        fprintf(stderr, "Error opening environment: %s\n",
            db_strerror(ret));
        exit(1);
    }


   /* Initialize the DB handle */
    ret = db_create(&retval, retvalE, 0);
    if (ret != 0) {
         retvalE->err(retvalE, ret,
                "Attempt to create db handle failed.");
        exit(1);
    }


    /* 
     * Set the database open flags. Autocommit is used because we are 
     * transactional. 
     */
    open_flags = DB_CREATE | DB_TRUNCATE;
    ret = retval->open(retval, /* Pointer to the database */
             NULL,             /* Txn pointer */
             "/var/tmp/oha",   /* File name -- Must be NULL for inmemory! */
             db_name,          /* Logical db name */
             DB_HASH,          /* Database type (using btree) */
             open_flags,       /* Open flags */
             0);               /* File mode. Using defaults */

    if (ret != 0) {
         retvalE->err(retvalE, ret,
                "Attempt to open db failed.");
        exit(1);
    }

	u_printf("Colloc using the bdb backend.\n");

#else
	Pvoid_t retval=NULL;
	u_printf("Colloc using the Judy backend.\n");
#endif

	if (option.rstart > option.rend) {
		error("option.rstart > option.rend. \n");
		return NULL;
	}

	FILE* ffst2=NULL;
	ffst2=u_fopen(snt->text_fst2,"rb");
	if (! ffst2) {
		error( "Error: Cannot open file %s\n", snt->text_fst2 );
		return NULL;
	}
	
	u_fprintf(stderr,"Loading %s ...\n", snt->text_fst2);
	ret=load_fst2_from_file(ffst2,0,&sfst2);
	if ( ret ) {
		u_fprintf(stderr,"Error %d in %s:%d. Fst2 file could not be loaded.\n", ret, __FILE__, __LINE__);
		return 0;
	}

	Pvoid_t sentence=NULL;
	JUDYLH(sentence);

	PPvoid_t nodes=NULL;
	JUDYHSH(nodes);

	u_fprintf(stderr,"Generating collocation candidates...\n");
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

		if ( (time(&ctime)-ptime) ) {
			u_fprintf(stderr,"Snt %8d/%d, %4.3f snt/s, %8d/%8d comb. so far. still ~%8.3f sec. to go. \r",
			               i, sfst2->number_of_graphs, ((float)(i-prev_i)) / ((float)(ctime-ptime)) , cnum, cnumu,
			               (ctime-stime) * (end -i) / ((float)i)
			         );
			ptime=ctime;
			prev_i=i;
		}


		while (state) { // here we try to group transitions whose terminal states are the same. FIXME: need confirmation about this.
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
						if (! *spos) {
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
							d=u_strchr(     c, '}' );

							if (d) {
								nodesKL=0;
								while (c!=d) {
									if (*c != ',') key[nodesKL++] = *c;

									if (nodesKL==KEYLENGTH) {
										u_fprintf( stderr, "Node key is too long in %s:%d. bailing out.\n", __FILE__, __LINE__ );
										exit(1);
									}
									c++;
								}
								key[nodesKL++]=0;
								nodesKL*=sizeof(unichar);

								nodesK=(Pvoid_t)key;
							}
							else {
								u_fprintf( stderr, "format error in %s, state %d transition %d: %S\n",
								                   snt->text_fst2, tran->state_number, tran->tag_number, input );
								exit(1);
							}
						}
					}
					else {
						if ( (   *input == ','  || *input == '.' || *input == '?' || *input == '!' 
					          || *input == '\'' || *input == '_' || *input == '-' || *input == ':' 
						      || *input == ')'  || *input == '(' || *input == '"' || *input == ';'
						      || *input == '%'  || *input == '/' ) && option.spunc) { // FIXME: do we have a list of punctuations? they differ from language to language.
							nodesK=NULL;
						}
						else {
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
					}

					if (nodesK) {
						JHSI( nodesI, *nodes, nodesK, nodesKL );
						if (! (*((int*)nodesI)) ) *((int*)nodesI)=++index;
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

		if (option.compact) {
			if (! (i % option.compact) ) {
				u_fprintf (stderr, "\ncompacting...                                                                     \n");
				colloc_compact( retval, option.threshold/2 );
			}
		}
	}

	u_fprintf(stderr, "Sentence %9d/%d, %4.3f sentences per second, %d combinations processed, of which %d unique.       \n",
	                  i-1, sfst2->number_of_graphs, ((float)i-1) / (time(&ctime)-stime), cnum, cnumu
	         );

	free_stack_int(stack);
	return retval;

}



#if 0
	// this code needs to go to the free_array function

    /* Close our database handle, if it was opened. */
    if (retval != NULL) {
        ret_t = retval->close(retval, 0);
        if (ret_t != 0) {
            fprintf(stderr, "%s database close failed.\n",
                db_strerror(ret_t));
            ret = ret_t;
        }
    }

    /* Close our environment, if it was opened. */
    if (retvalE != NULL) {
        ret_t = retvalE->close(retvalE, 0);
        if (ret_t != 0) {
            fprintf(stderr, "environment close failed: %s\n",
                db_strerror(ret_t));
                ret = ret_t;
        }
    }
#endif


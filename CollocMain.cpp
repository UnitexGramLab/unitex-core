
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
  
#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Snt.h"
#include "Error.h"
#include "Unicode.h"
#include "defines.h"
#include "Alphabet.h"
#include "Copyright.h"
#include "CollocMain.h"
#include "Collocation.h"
#include "Text_tokens.h"

#ifndef DEFAULT_CLENGTH 
#define DEFAULT_CLENGTH 2
#endif

#ifndef DEFAULT_LWIDTH  
#define DEFAULT_LWIDTH  0
#endif

static void usage(int header) {

	if (header) {
		u_printf("%S",COPYRIGHT);
		u_printf(
			"This is a tool that is aimed to be used to extract collocations. "
			"It works on the normalized text automata.\n"
			"\n"
		);
	}

	u_printf(
		"Usage: Colloc [OPTIONS] <snt directory>\n"
		"\n"
		"Parameters:\n"
		"     -?, --help                    Shows this message\n"
		"     -c, --combination-length=LEN  TODO: The length of word combinations. The p in C(n,p)\n"
		"     -l, --linear-width=LEN        TODO: The limit in which the token combinations are formed. FIXME: Do we need this, actually?\n"
		"                                   This the n in C(n,p).\n"
		"     -p, --no-strip-punctuations      Strip punctuations like , . ! etc.\n"
		"     -t, --strip-tag=TAG1,TAG2,... POS tags to strip from the collocation candidates.\n"
		"\n"
	); 
}

/* 
 * This function behaves in the same way as an int main(), except that it does
 * not invoke the setBufferMode function.
 */

int main_Colloc(int argc, char **argv) {

	char ch;
	int option_index = 0;
	
	const struct option longopts[] = { 
		 {                  "help",       no_argument, NULL, '?' }
		,{    "combination-length", required_argument, NULL, 'c' }
		,{          "linear-width", required_argument, NULL, 'l' } 
		,{            "strip-tags", required_argument, NULL, 't' }
		,{ "no-strip-punctuations",       no_argument, NULL, 'p' }	
		,{NULL, 0, NULL, 0}
	};

	colloc_opt option;
	option.clength = DEFAULT_CLENGTH; 
	option.lwidth  = DEFAULT_LWIDTH; 
	option.spunc   = 1;
	option.spos    = NULL;
	
	while ((ch = getopt_long(argc, argv, "?c:l:t:p", longopts, &option_index)) != -1) {
		switch (ch) {
	
		case '?':
			usage(1);
			exit(EXIT_SUCCESS);
			break;	
	
		case 'c':
			STRINGINT(optarg, option.clength);
			if (option.clength <= 0) {
				u_printf("combination length must be > 0\n\n");
				exit (EXIT_FAILURE);
			}

			break;

		case 'l':
			STRINGINT(optarg, option.lwidth);
			if (option.lwidth <= 0) {
				u_printf("linear context width must be > 0\n\n");
				exit (EXIT_FAILURE);
			}
			break;

		case 't': {
			char *p=optarg, *q=optarg;
			int len;
			int size=1;

			while (1) {
				if ( *p==',' || *p==0 ) {
					len = p-q;
					if (len) {
						size++;
					}
					q=p+1;
				}

				if (! *p) break;
				p++;
			}
			
			custom_malloc( unichar*, size, option.spos );

			p=optarg;
			q=optarg;
			size=0;
			while (1) {
				if ( *p==',') {
					len = p-q;
					if (len) {
						custom_malloc( unichar, (len+1), option.spos[size] );
						*p=0; u_sprintf( option.spos[size], "%s", q ); *p=',';
						size++;
					}
					q=p+1;
				}

				if (! *p) {
					len = p-q;
					if (len) {
						custom_malloc( unichar, (len+1), option.spos[size] );
						u_sprintf( option.spos[size], "%s", q );
						size++;
					}
					break;
				}
				p++;
			}

			option.spos[size]=0;
			break;
			
		}
		case 'p': {
			option.spunc=1;
			break;
		}
		
		default:
			exit (EXIT_FAILURE);
		
		}
	}
	
	
	char text_snt[FILENAME_MAX];
	if (optind < argc) {
		if (strlen (argv[optind]) > FILENAME_MAX) {
			u_fprintf(stderr, "`%s' is too long for a file name (max=%d)", argv[optind], FILENAME_MAX);
			exit (EXIT_FAILURE);
		}
		else {
			get_path ( argv[optind], text_snt );
	    }
	}
	else { 
		usage(1);
		u_fprintf(stderr, "Error: no snt directory specified\n\n");
		exit(EXIT_FAILURE);
	}
	
	struct snt_files* snt_files=NULL;
	snt_files = new_snt_files_from_path(text_snt);

	Pvoid_t candidates=NULL;
	
	candidates=colloc_generate_candidates(snt_files, option);
	colloc_print(candidates, 2);
	
	u_printf("Done.\n");
	
	return 0;
}


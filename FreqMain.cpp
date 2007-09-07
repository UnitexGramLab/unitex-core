
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
 *         This File contains the real main function of Freq executable.
 */
  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>
#include "FreqMain.h"
#include "Unicode.h"
#include "Text_tokens.h"
#include "String_hash.h"
#include "List_int.h"
#include "Alphabet.h"
#include "Matches.h"
#include "Frequency.h"
#include "File.h"
#include "Copyright.h"
#include "LocatePattern.h"
#include "Error.h"
#include "Snt.h"
#include "defines.h"

typedef Pvoid_t judy;

static void usage(int header) {

	if (header) {
		u_printf("%S",COPYRIGHT);
		u_printf(
			"Creates the frequency table of words that were in the vicinity of the given word (in the .ind file)\n\n"
		);
	}

	u_printf(
		"Usage:\n"
		"     Freq [OPTIONS] <snt_directory>\n"
		"\n"
		"Parameters:\n"
		"     -?, --help                  Shows this message\n"
		"     -t, --threshold=LIMIT       Words with values below LIMIT won't be displayed.\n"
        "                                 Default: 2\n"
		"     -a, --text-automata         FIXME: Work on text automata instead of linear text.\n"
		"                                 (Which should be used for Thai) Implies -s.\n"
		"     -o, --words-only            Tokens that are not words are ignored.\n"
		"     -w, --context-width=SIZE    The size of the window the frequency values are\n" 
        "                                 computed for. Default: 10\n"
		"     -s, --sentence-only         When counting tokens, don't go beyond sentence\n" 
        "                                 boundaries\n"
		"\n"
		"\n"
	); 
}

/* 
 * This function behaves in the same way as an int main(), except that it does
 * not invoke the setBufferMode function.
 */

int main_Freq(int argc, char **argv) {

	char ch;
	int option_index = 0;
	
	const struct option longopts[] = {
		{"help",               no_argument,       NULL, '?'},
		{"threshold",          required_argument, NULL, 't'},
		{"text-automata",      no_argument,       NULL, 'a'},
		{"words-only",         no_argument,       NULL, 'o'},
		{"context-width",      required_argument, NULL, 'w'},
		{"sentence-only",      no_argument,       NULL, 's'},
		{NULL, 0, NULL, 0}
	};

	freq_opt option;
	option.automata      =  0; /* By default, we work on surface string */
	option.words_only    =  0; /* By default, we are not restricting ourselves only to word tokens */
	option.token_limit   = 10; /* By default, the context limit is +/-10 tokens */
	option.threshold     =  2; /* By default, frequency limit for displaying tokens is 2 */
	option.sentence_only =  0; /* By default, we go beyond sentence limits when counting frequencies */
	
	while ((ch = getopt_long(argc, argv, "?t:aow:sc:", longopts, &option_index)) != -1) {
		switch (ch) {
	
		case '?':
			usage(1);
			exit(EXIT_SUCCESS);
			break;	
	
		case 't':
			STRINGINT(optarg, option.threshold);
			break;
		
		case 'a':
			option.automata=1;
			break;
		
		case 'o':
			option.words_only=1;
			break;
		
		case 'w':
			STRINGINT(optarg, option.token_limit);
			if (option.token_limit < 1) {
				u_printf("context width must be > 0\n\n");
				exit (EXIT_FAILURE);
			}
			break;
	
		case 's': 
			option.sentence_only=1;
			break;
		
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
	judy freqtable;
	
	freqtable=create_freqtable(snt_files, option);
	print_freqtable(freqtable, option.threshold);
	
	u_printf("Done.\n");
	
	return 0;
}


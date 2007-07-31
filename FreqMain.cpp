
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


/* Maximum number of new lines in a text. New lines are encoded in
 * 'enter.pos' files. Those files will disappear in the futures */
#define MAX_ENTER_CHAR 1000000
int enter_pos[MAX_ENTER_CHAR];

#define STRINGINT(_string, _int) { \
  char *_tmp; \
  long _number = strtol (_string, &_tmp, 0); \
  errno = 0; \
  if ((errno != 0 && _number == 0) || _string == _tmp || \
      (errno == ERANGE && (_number == LONG_MAX || _number == LONG_MIN))) \
    { \
      u_fprintf (stderr,"`%s' out of range", _string);; \
      exit (EXIT_FAILURE); \
    } \
  else \
  _int = (int) _number; \
}

void usage(int header) {

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
		"     -h, --thai                  Thai mode on\n" 
		"                                 --== Doesn't yet have any effect ==--\n"
		"     -o, --words-only            Tokens that are not words are ignored.\n"
		"     -w, --context-width=SIZE    The size of the window the frequency values are\n" 
        "                                 computed for. Default: 10\n"
		"     -s, --sentence-only         When counting tokens, don't go beyond sentence\n" 
        "                                 boundaries\n"
        "     -c, --combination-length=N  Treat N-word combinations of all words that are\n"
        "                                 within context as one single token. (Which is\n" 
        "                                 different from N-grams as the words don't need\n"
        "                                 to follow each other.\n" 
        "                                 --== Not implemented yet ==--\n"
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
	{"thai",               no_argument,       NULL, 'h'},
	{"words-only",         no_argument,       NULL, 'o'},
	{"context-width",      required_argument, NULL, 'w'},
	{"sentence-only",      no_argument,       NULL, 's'},
	{"combination-length", required_argument, NULL, 'c'},
	{NULL, 0, NULL, 0}
};
struct freq_opt option;
option.thai_mode = 0;    /* By default, we are not dealing with Thai */
option.words_only = 0;   /* By default, we are not restricting ourselves only to word tokens */
option.token_limit = 10; /* By default, the context limit is +/-10 tokens */
option.threshold = 2;    /* By default, frequency limit for displaying tokens is 2 */
option.sentence_only=0;  /* By default, we go beyond sentence limits when counting frequencies */
option.clength=1;        /* clength short for combination length. 
                          * By default, we look for simple words (and not MWEs). */

while ((ch = getopt_long(argc, argv, "?t:how:sc:", longopts, &option_index)) != -1) {
	switch (ch) {

	case '?':
		usage(1);
		exit(EXIT_SUCCESS);
		break;	

	case 't':
		STRINGINT(optarg, option.threshold);
		break;
	
	case 'h':
		option.thai_mode=1;
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
	
	case 'c':
		STRINGINT(optarg, option.clength);
		if (option.clength < 1) {
			u_printf("combination length must be > 0\n\n");
			exit (EXIT_FAILURE);
		}
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

/* open snt files we're going to need */
struct snt_files* snt_files=NULL;
snt_files = new_snt_files_from_path(text_snt);

FILE* freq=u_fopen(snt_files->freq,U_WRITE); // the output file
if (freq==NULL) {
	error("Error: Cannot open file %s\n",argv[1]);
	return 1;
}

FILE* text=fopen(snt_files->text_cod,"rb");
if (text==NULL) {
	error("Error: Cannot open file %s\n",snt_files->text_cod);
	u_fclose(freq);
	return 1;
}

FILE* ind=u_fopen(snt_files->concord_ind,"rb");
if (ind==NULL) {
	error("Error: Cannot open file %s\n",snt_files->concord_ind);
	u_fclose(freq);
	fclose(text);
	return 1;
}

struct text_tokens* tok=load_text_tokens(snt_files->tokens_txt);
if (tok==NULL) {
	error("Error: Cannot load text token file %s\n",snt_files->tokens_txt);
	u_fclose(freq);
	u_fclose(ind);
	fclose(text);
	return 1;
}

FILE* f_enter=fopen(snt_files->enter_pos,"rb");
int n_enter_char;
if (f_enter==NULL) {
	error("Error: Cannot open file %s\n",snt_files->enter_pos);
	n_enter_char=0;
}
else {
	n_enter_char=fread(&enter_pos,sizeof(int),MAX_ENTER_CHAR,f_enter);
	fclose(f_enter);
}

/*
 * Once we've parsed all the arguments, we call the function to create the 
 * frequency table 
 */

create_freqtable(freq,text,ind,tok,option);
u_fclose(freq);
fclose(text);
u_fclose(ind);
free_text_tokens(tok);
u_printf("Done.\n");

return 0;
}


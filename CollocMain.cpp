
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

#include "Collocation.h"

#include "Unicode.h"
#include "Text_tokens.h"
#include "String_hash.h"
#include "Alphabet.h"
#include "Matches.h"
#include "File.h"
#include "Copyright.h"
#include "LocatePattern.h"
#include "Error.h"
#include "Snt.h"


/* Maximum number of new lines in a text. New lines are encoded in
 * 'enter.pos' files. Those files will disappear in the futures */
#define MAX_ENTER_CHAR 1000000
static int enter_pos[MAX_ENTER_CHAR];

/* 
 * This function behaves in the same way as an int main(), except that it does
 * not invoke the setBufferMode function and that it does not print the
 * usage.
 */

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


int main_Colloc(int argc, char **argv) {

char ch;
int option_index = 0;


const struct option longopts[] =
    {
{"words-only", no_argument, NULL, 'w'},
{NULL, 0, NULL, 0}
    };
struct colloc_opt option;
option.words_only = 0;   /* By default, we are not restricting ourselves only to word tokens */

while ((ch = getopt_long(argc, argv, "w", longopts, &option_index)) != -1) {
	switch (ch) {
	
	case 'w':
		option.words_only=1;
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
else { /* If only version was requested then exit now */
	u_fprintf(stderr, "no snt directory specified");
	exit(EXIT_FAILURE);
}

/* open snt files we're going to need */
struct snt_files* snt_files=NULL;
snt_files = new_snt_files_from_path(text_snt);

u_printf("Done.\n");

return 0;
}


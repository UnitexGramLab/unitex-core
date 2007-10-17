 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Unicode.h"
#include "File.h"
#include "Copyright.h"
#include "String_hash.h"
#include "IOBuffer.h"
#include "Error.h"
#include "NormalizeAsRoutine.h"


void usage() {
	u_printf("%S",COPYRIGHT);
	u_printf("Usage: Normalize <text> [-no_CR] [-f=EQUIV]\n");
	u_printf("     <text>   : text file to be normalized\n");
	u_printf("     -no_CR   : this optional parameter indicates that every separator\n");
	u_printf("                sequence will be turned into a single space\n\n");
	u_printf("     -f=EQUIV : this optional parameter specifies a configuration file\n");
	u_printf("                EQUIV that contains replacement instructions in the form\n");
	u_printf("                of lines like: input_sequence tab output_sequence\n");
	u_printf("                By default, the program only replace { and } by [ and ]\n");
	u_printf("Turns every sequence of separator chars (space, tab, new line) into one.\n");
	u_printf("If a separator sequence contains a new line char, it is turned to a single new\n");
	u_printf("line (except with -no_CR); if not, it is turned into a single space. As\n");
	u_printf("a side effect, new line sequences are converted into the Windows style: \\r\\n.\n");
	u_printf("If you specifies replacement rules with -f, they will be applied prior\n");
	u_printf("to the separator normalization, so you have to take care if you manipulate\n");
	u_printf("separators in your replacement rules.\n");
	u_printf("The result is stored in a file named file_name.snt.\n");
}


int main(int argc, char **argv) {
	/* Every Unitex program must start by this instruction,
	 * in order to avoid display problems when called from
	 * the graphical interface */
	setBufferMode();

	if (argc<2 || argc >4) {
		usage();
		return 0;
	}

	int mode=KEEP_CARRIDGE_RETURN;
	char *rules = NULL;

	/* We check the parameters */
	if (argc==3) {
		if (strcmp(argv[2],"-no_CR")) {
			if (argv[2][0]=='-' && argv[2][1]=='f' && argv[2][2]=='=') { rules = &(argv[2][3]); }
			else {
				error("Wrong parameter: %s\n",argv[2]);
				return 1;
			}
		}
		else { mode=REMOVE_CARRIDGE_RETURN; }
	}

	if (argc==4) {
		if (strcmp(argv[2],"-no_CR")) {
			error("Wrong parameter: %s\n",argv[2]);
			return 1;
		}

		mode=REMOVE_CARRIDGE_RETURN;

		if (argv[3][0]=='-' && argv[3][1]=='f' && argv[3][2]=='=') { rules = &(argv[3][3]); }
		else {
			error("Wrong parameter: %s\n", argv[3]);
			return 1;
		}
	}

	char tmp_file[FILENAME_MAX];
	get_extension(argv[1],tmp_file);
	if (!strcmp(tmp_file, ".snt")) {
		/* If the file to process has allready the .snt extension, we temporary rename it to
		 * .snt.normalizing */
		strcpy(tmp_file, argv[1]);
		strcat(tmp_file, ".normalizing");
		rename(argv[1], tmp_file);
	} else { strcpy(tmp_file, argv[1]); }

	/* We set the destination file */
	char dest_file[FILENAME_MAX];
	remove_extension(argv[1], dest_file);
	strcat(dest_file, ".snt");

	u_printf("Normalizing %s...\n",argv[1]);
	normalize(tmp_file, dest_file, mode, rules);
	u_printf("\n");

	/* If we have used a temporary file, we delete it */
	if (tmp_file != argv[1]) { remove(tmp_file); }

	return 0;
}




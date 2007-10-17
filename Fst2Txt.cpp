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


#include "Copyright.h"
#include "Error.h"
#include "File.h"
#include "Fst2TxtAsRoutine.h"
#include "IOBuffer.h"
#include "LocateConstants.h"


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Fst2Txt <text> <fst2> <alphabet> <MODE> [-char_by_char|-char_by_char_with_space]\n"
       "     <text> : the unicode text file to be parsed\n"
       "     <fst2> : the grammar to be applied to the text\n"
       "     <alphabet> : the alphabet file for the current language.\n"
       "     <MODE> : the parsing can be done in merge mode or replace mode, using\n"
       "              -merge or -replace\n"
       "     -char_by_char : force the program to parse char by char. This\n"
       "                     option is useful for languages like Thai.\n"
       "     -char_by_char_with_space : force the program to parse char by char, allowing\n"
       "                     the matching of expressions beginning by spaces.\n\n"
       "Applies a grammar to a text. The text file is modified.\n");
}



int main(int argc, char **argv) {
	/* Every Unitex program must start by this instruction,
	 * in order to avoid display problems when called from
	 * the graphical interface */
	setBufferMode();

	if (argc<5) {
	   usage();
	   return 0;
	}
	struct fst2txt_parameters* p=new_fst2txt_parameters();

	char *tmp;
	tmp = strdup(argv[1]);
	remove_extension(argv[1], tmp);
	strcat(tmp, ".tmp");

	p->text_file=argv[1];
	p->temp_file=tmp;
	p->fst_file=argv[2];
	p->alphabet_file=argv[3];

	if (!strcmp(argv[4],"-merge")) {
		p->output_policy=MERGE_OUTPUTS;
	}
	else if (!strcmp(argv[4],"-replace")) {
		p->output_policy=REPLACE_OUTPUTS;
	} else {
		error("Invalid parameter %s : the mode must be -merge or -replace\n",argv[4]);
		return 1;
	}

	if (argc>=6) {
		if (!strcmp(argv[5],"-char_by_char")) {
			p->parsing_mode=PARSING_CHAR_BY_CHAR;
		} 
		else if (!strcmp(argv[5],"-char_by_char_with_space")) {
			p->parsing_mode=PARSING_CHAR_BY_CHAR_WITH_SPACE;
		} 
		else {
			error("Invalid parameter: %s\n",argv[5]);
			return 1;
		}
	}

	return main_fst2txt(p);
}

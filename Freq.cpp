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
#include "Unicode.h"
#include "Copyright.h"
#include "FreqMain.h"
#include "IOBuffer.h"


void usage(int argc, char ** argv) {
	u_printf("%S",COPYRIGHT);
	u_printf(
		"Creates the frequency table of words that were in the vicinity of the given word (in the ind file)\n"
		"Usage:\n"
		"\t%s <snt> [-thai] [-wordsonly]\n\n", argv[0]
	);
}

int main(int argc, char **argv) {
	/* Every Unitex program must start by this instruction,
	 * in order to avoid display problems when called from
	 * the graphical interface */
	setBufferMode();
	if (argc!=2 && argc!=3 && argc!=4) {
		usage(argc, argv);
		return 0;
	}
	/* We call an artificial main function located in 'ConcordMain'. This
	 * trick allows to use the functionalities of the 'Concord' program
	 * without having to launch an external process.
	 */
	return main_Freq(argc,argv);
}


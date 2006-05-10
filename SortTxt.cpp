 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Copyright.h"
#include "IOBuffer.h"
#include "SortTxtMain.h"


void usage() {
printf("%s",COPYRIGHT);
printf("Usage: SortTxt <text file> [OPTIONS]\n");
printf("     <text file> : any unicode text file\n\n");
printf("The options can be a combinaison of the following:\n");
printf("     -y : remove duplicates\n");
printf("     -n : do not remove duplicates\n");
printf("     -r : reverse the sort order\n");
printf("     -o <char order> : use a file describing the char order for sorting\n");
printf("     -l <file> : saves the resulting number of lines in <file>\n");
printf("     -thai : sort thai text\n\n");
printf("By default, the sort is done according the Unicode char order, removing\n");
printf("the duplicates.\n\n");
}


int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();
if (argc<2) {
	usage();
	return 0;
}
/* We call an artificial main function located in 'SortTxtMain'. This
 * trick allows to use the functionalities of the 'SortTxt' program
 * without having to launch an external process.
 */
return main_SortTxt(argc,argv);
}

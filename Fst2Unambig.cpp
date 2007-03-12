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
#include "Fst2.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "LinearAutomaton2Txt.h"
#include "Error.h"


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Fst2Unambig <fst2> <txt>\n");
u_printf("     <fst2> : fst2 file representing the text automaton\n");
u_printf("     <txt>  : output unicode text file\n\n");
u_printf("Converts a linear Unitex text automaton into a text file. If\n");
u_printf("the automaton is not linear, the process is aborted.\n");
}



int main(int argc, char **argv) {
setBufferMode();

if (argc!=3) {
   usage();
   return 0;
}
u_printf("Loading text automaton...\n");
Fst2* fst2=load_fst2(argv[1],0);
if (fst2==NULL) {
   error("Cannot load text automaton %s\n",argv[1]);
   return 1;
}

int res=isLinearAutomaton(fst2);
if (res!=LINEAR_AUTOMATON) {
   error("Error: the text automaton is not linear in sentence %d\n",res);
   free_Fst2(fst2);
   return 1;
}

FILE* f=u_fopen(argv[2],U_WRITE);
if (f==NULL) {
   error("Cannot create %s\n",argv[2]);
   free_Fst2(fst2);
   return 1;
}
u_printf("Converting linear automaton into text...\n");
convertLinearAutomaton(fst2,f);
u_fclose(f);
free_Fst2(fst2);
u_printf("Done.\n");
return 0;
}






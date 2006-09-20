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
#include "unicode.h"
#include "Fst2.h"
#include "Grf2Fst2_lib.h"
#include "FlattenFst2.h"
#include "Liste_nombres.h"
#include "IOBuffer.h"

//---------------------------------------------------------------------------
//
// "E:\My Unitex\French\Graphs\These\Insert_Vaux_GN\gn.fst2" RTN 1
//
// "E:\My Unitex\French\Graphs\cEstAdjAnne.fst2" RTN 1
//
//---------------------------------------------------------------------------
void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Flatten <fst2> <type> [depth]\n");
printf("     <fst2> : compiled grammar to flatten;\n");
printf("     <type> : this parameter specifies the type of the resulting grammar\n");
printf("              The 2 possibles values are:\n");
printf("              FST : if the grammar is not a finite-state one, the program\n");
printf("                    makes a finite-state approximation of it. The resulting\n");
printf("                    FST2 will contain only one graph.\n");
printf("              RTN : the grammar will be flattened according to the depth limit.\n");
printf("                    The resulting grammar may not be finite-state.\n");
printf("     [depth] : maximum subgraph depth to be flattened. If this parameter is\n");
printf("               not precised, the value 10 is taken by default.\n");
printf("\n\n");
printf("Flattens a FST2 grammar into a finite state transducer in the limit of\n");
printf("a recursion depth. The grammar <fst2> is replaced by its flattened equivalent.\n");
printf("If the flattening process is complete, the resulting grammar contains only one\n");
printf("graph.\n");
}

int main(int argc, char **argv) {
setBufferMode();

if ((argc<3) || (argc>4)) {
   usage();
   return 0;
}
int RTN;
if (!strcmp(argv[2],"RTN")) {
   RTN=1;
}
else if (!strcmp(argv[2],"FST")) {
   RTN=0;
}
else {
   fprintf(stderr,"Invalid parameter: %s\n",argv[2]);
   return 1;
}
int depth=10;
if (argc==4) {
   if (1!=sscanf(argv[3],"%d",&depth) || (depth<1)) {
      fprintf(stderr,"Invalid depth parameter %s\n",argv[3]);
      return 1;
   }
}
printf("Loading %s...\n",argv[1]);
Fst2* origin=load_fst2(argv[1],1);
if (origin==NULL) {
   fprintf(stderr,"Cannot load %s\n",argv[1]);
   return 1;
}

char temp[2000];
strcpy(temp,argv[1]);
strcat(temp,".tmp.fst2");

switch (flatten_fst2(origin,depth,temp,RTN)) {
   case EQUIVALENT_FST: printf("The resulting grammar is an equivalent finite-state transducer.\n");
                        break;
   case APPROXIMATIVE_FST: printf("The resulting grammar is a finite-state approximation.\n");
                        break;
   case EQUIVALENT_RTN: printf("The resulting grammar is an equivalent FST2 (RTN).\n");
                        break;
   default:;
}
free_Fst2(origin);
remove(argv[1]);
rename(temp,argv[1]);
return 0;
}
//---------------------------------------------------------------------------

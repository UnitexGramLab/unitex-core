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
#include "LocateAsRoutine.h"
/* $CD$ begin */
#include "GF_lib.h"
/* $CD$ end   */

//---------------------------------------------------------------------------


void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Locate <text> <fst2> <alphabet> <sla> <imr> <n> [<dynamicSntDir>] [-thai] [-space]\n");
printf("     <text>: the text file\n");
printf("     <fst2> : the grammar to be applied\n");
printf("     <alphabet> : the language alphabet file\n");
printf("     <sla> : defines the matching mode: s=shortest matches\n");
printf("                                        l=longuest matches\n");
printf("                                        a=all matches\n");
printf("     <imr> : defines the transduction mode: i=ignore outputs\n");
printf("                                            m=merge outputs\n");
printf("                                            r=replace outputs\n");
printf("     <n> : defines the search limitation:\n");
printf("                                        all=compute all matches\n");
printf("                                        N=stop after N matches\n");
printf("     [<dynamicSntDir>] : optional dynamic Snt Objects directory ((back)slash terminated)\n");
printf("     [-thai] : option to use to work on thai\n");
printf("     [-space] : enables morphological use of space\n");
printf("\nApplies a grammar to a text, and saves the matching sequence index in a\n");
printf("file named \"concord.ind\" stored in the text directory. A result info file\n");
printf("named \"concord.n\" is also saved in the same directory.\n");
}



int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();
if (argc!=7 && argc!=8 && argc!=9) {
   usage();
   #ifdef DO_NOT_USE_TRE_LIBRARY
   fprintf(stderr,"\n\nWARNING: morphological filters are disabled\n");
   #else
   #ifndef TRE_WCHAR
   fprintf(stderr,"\n\nWARNING: on this system, morphological filters will not be taken into account,\n");
   fprintf(stderr,"         because wide characters are not supported\n");
   #endif
   #endif
   return 0;
}
#ifdef DO_NOT_USE_TRE_LIBRARY
   fprintf(stderr,"WARNING: morphological filters are disabled\n");
#else
#ifndef TRE_WCHAR
   fprintf(stderr,"WARNING: on this system, morphological filters will not be taken into account,\n");
   fprintf(stderr,"         because wide characters are not supported\n");
#endif
#endif
/* We call an artificial main function located in 'LocateAsRoutine'. This
 * trick allows to use the functionalities of the 'Locate' program
 * without having to launch an external process.
 */
return main_Locate(argc,argv);
}

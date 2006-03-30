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

#include "unicode.h"
#include "Alphabet.h"
#include "DELA.h"
#include "String_hash.h"
#include "LocatePattern.h"
#include "AutomateFst2.h"
#include "Text_tokens.h"
#include "Liste_nombres.h"
#include "Grammatical_codes.h"
#include "Flexional_codes.h"
#include "Facteurs_interdits.h"
#include "Loading_dic.h"
#include "CompoundWordTree.h"
#include "Fst2_tags_optimization.h"
#include "DLC_optimization.h"
#include "Optimized_fst2.h"
#include "Pattern_transitions.h"
#include "Text_parsing.h"
#include "Matches.h"
#include "TransductionVariables.h"
#include "TransductionStack.h"
#include "Liste_num.h"
#include "FileName.h"
#include "Copyright.h"
/* $CD$ begin */
#include "GF_lib.h"
/* $CD$ end   */
#include "IOBuffer.h"

//---------------------------------------------------------------------------

//
// "E:\My Unitex\Flammand\Corpus\NRC.EXP3.snt" "E:\My Unitex\Flammand\hebben.fst2" "E:\My Unitex\Flammand\Alphabet.txt" l i 200
//
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
/* $CD$ begin + overall */
char staticSntDir[2000], dynamicSntDir[2000];
/* $CD$ end */

char tokens_txt[2000];
char text_cod[2000];
char dlf[2000];
char dlc[2000];
char err[2000];


get_snt_path(argv[1],staticSntDir);

strcpy(tokens_txt,staticSntDir);
strcat(tokens_txt,"tokens.txt");

strcpy(text_cod,staticSntDir);
strcat(text_cod,"text.cod");

strcpy(dlf,staticSntDir);
strcat(dlf,"dlf");

strcpy(dlc,staticSntDir);
strcat(dlc,"dlc");

strcpy(err,staticSntDir);
strcat(err,"err");

int mode;
int output_mode;
if (!strcmp(argv[4],"s")) {
   mode=SHORTEST_MATCHES;
} else if (!strcmp(argv[4],"a")) {
          mode=ALL_MATCHES;
       }
  else if (!strcmp(argv[4],"l")) {
          mode=LONGUEST_MATCHES;
       }
  else {
     fprintf(stderr,"Invalid parameter %s\n",argv[4]);
     return 1;
  }
if (!strcmp(argv[5],"i")) {
   output_mode=IGNORE_TRANSDUCTIONS;
} else if (!strcmp(argv[5],"m")) {
          output_mode=MERGE_TRANSDUCTIONS;
       }
  else if (!strcmp(argv[5],"r")) {
          output_mode=REPLACE_TRANSDUCTIONS;
       }
  else {
     fprintf(stderr,"Invalid parameter %s\n",argv[5]);
     return 1;
  }
if (!strcmp(argv[6],"all")) {
   SEARCH_LIMITATION=-1;
}
else {
   if (!sscanf(argv[6],"%d",&SEARCH_LIMITATION)) {
      fprintf(stderr,"Invalid parameter %s\n",argv[6]);
      return 1;
   }
}


/* $CD$ begin */
switch (argc) {

    case 7: // 6 arguments: pas de dynamic, pas de -thai, pas de -space

        strcpy(dynamicSntDir, staticSntDir);
        CHAR_BY_CHAR=OCCIDENTAL;
        GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
        break;


    case 8: // 7 arguments: soit dynamic, soit -thai, soit -space

        if (!strcmp(argv[7], "-thai")) {
            strcpy(dynamicSntDir, staticSntDir);
            CHAR_BY_CHAR=THAI;
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
            }
        else if (!strcmp(argv[7], "-space")) {
            strcpy(dynamicSntDir, staticSntDir);
            CHAR_BY_CHAR=OCCIDENTAL;
            GESTION_DE_L_ESPACE=MODE_MORPHO;
            }
        else {
            strcpy(dynamicSntDir, argv[7]);
            CHAR_BY_CHAR=OCCIDENTAL;
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
            }
        break; 

    
    case 9: // 8 arguments: 7 = dynamic, 8 = soit -thai, soit -space
    
        strcpy(dynamicSntDir, argv[7]);
        
        if (strcmp(argv[8], "-thai") && strcmp(argv[8], "-space")) {
            fprintf(stderr, "Invalid parameter %s\n", argv[8]);
            return 1;
            }
        
        if (!strcmp(argv[8], "-thai"))
            CHAR_BY_CHAR=THAI;
        else
            CHAR_BY_CHAR=OCCIDENTAL;
        
        if (!strcmp(argv[8], "-space"))
            GESTION_DE_L_ESPACE=MODE_MORPHO;
        else
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
        break; 

    
    case 10: // 9 arguments: 7 = dynamic, 8 = soit -thai, soit -space
             //                           9 = soit -space, soit -thai
    
        strcpy(dynamicSntDir, argv[7]);
        
        if (strcmp(argv[8], "-thai") && strcmp(argv[8], "-space")) {
            fprintf(stderr, "Invalid parameter %s\n", argv[8]);
            return 1;
            }
        
        if (!strcmp(argv[8], "-thai"))
            CHAR_BY_CHAR=THAI;
        else
            CHAR_BY_CHAR=OCCIDENTAL;
        
        if (!strcmp(argv[8], "-space"))
            GESTION_DE_L_ESPACE=MODE_MORPHO;
        else
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
        
        if (strcmp(argv[9], "-thai") && strcmp(argv[9], "-space")) {
            fprintf(stderr, "Invalid parameter %s\n", argv[9]);
            return 1;
            }
        
        if (!strcmp(argv[9], "-thai"))
            CHAR_BY_CHAR=THAI;
        else
            CHAR_BY_CHAR=OCCIDENTAL;
        
        if (!strcmp(argv[9], "-space"))
            GESTION_DE_L_ESPACE=MODE_MORPHO;
        else
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
        break;
    
    } 
/* $CD$ end */
int OK=locate_pattern(text_cod,tokens_txt,argv[2],dlf,dlc,err,argv[3],mode,output_mode,
               dynamicSntDir);
if (OK == 1) {
    return 0;
}
else {
return 1;
}
}
//---------------------------------------------------------------------------

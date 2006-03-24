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
//  The purpose is to call locate  with one Fst from dico.exe
//  by Alexis Neme 15/11/2005
//

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
#include "Arbre_mots_composes.h"
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
#include "LocateAsRoutine.h"



int Call_Locate_Prog(int aargc, char **aargv) {  
// replicates the main of locate.cpp
setBufferMode();

if (aargc!=7 && aargc!=8 && aargc!=9) {
   //usage();
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


get_snt_path(aargv[1],staticSntDir);

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
if (!strcmp(aargv[4],"s")) {
   mode=SHORTEST_MATCHES;
} else if (!strcmp(aargv[4],"a")) {
          mode=ALL_MATCHES;
       }
  else if (!strcmp(aargv[4],"l")) {
          mode=LONGUEST_MATCHES;
       }
  else {
     fprintf(stderr,"Invalid parameter %s\n",aargv[4]);
     return 1;
  }
if (!strcmp(aargv[5],"i")) {
   output_mode=IGNORE_TRANSDUCTIONS;
} else if (!strcmp(aargv[5],"m")) {
          output_mode=MERGE_TRANSDUCTIONS;
       }
  else if (!strcmp(aargv[5],"r")) {
          output_mode=REPLACE_TRANSDUCTIONS;
       }
  else {
     fprintf(stderr,"Invalid parameter %s\n",aargv[5]);
     return 1;
  }
if (!strcmp(aargv[6],"all")) {
   SEARCH_LIMITATION=-1;
}
else {
   if (!sscanf(aargv[6],"%d",&SEARCH_LIMITATION)) {
      fprintf(stderr,"Invalid parameter %s\n",aargv[6]);
      return 1;
   }
}


/* $CD$ begin */
switch (aargc) {

    case 7: // 6 arguments: pas de dynamic, pas de -thai, pas de -space

        strcpy(dynamicSntDir, staticSntDir);
        CHAR_BY_CHAR=OCCIDENTAL;
        GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
        break;


    case 8: // 7 arguments: soit dynamic, soit -thai, soit -space

        if (!strcmp(aargv[7], "-thai")) {
            strcpy(dynamicSntDir, staticSntDir);
            CHAR_BY_CHAR=THAI;
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
            }
        else if (!strcmp(aargv[7], "-space")) {
            strcpy(dynamicSntDir, staticSntDir);
            CHAR_BY_CHAR=OCCIDENTAL;
            GESTION_DE_L_ESPACE=MODE_MORPHO;
            }
        else {
            strcpy(dynamicSntDir, aargv[7]);
            CHAR_BY_CHAR=OCCIDENTAL;
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
            }
        break; 

    
    case 9: // 8 arguments: 7 = dynamic, 8 = soit -thai, soit -space
    
        strcpy(dynamicSntDir, aargv[7]);
        
        if (strcmp(aargv[8], "-thai") && strcmp(aargv[8], "-space")) {
            fprintf(stderr, "Invalid parameter %s\n", aargv[8]);
            return 1;
            }
        
        if (!strcmp(aargv[8], "-thai"))
            CHAR_BY_CHAR=THAI;
        else
            CHAR_BY_CHAR=OCCIDENTAL;
        
        if (!strcmp(aargv[8], "-space"))
            GESTION_DE_L_ESPACE=MODE_MORPHO;
        else
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
        break; 

    
    case 10: // 9 arguments: 7 = dynamic, 8 = soit -thai, soit -space
             //                           9 = soit -space, soit -thai
    
        strcpy(dynamicSntDir, aargv[7]);
        
        if (strcmp(aargv[8], "-thai") && strcmp(aargv[8], "-space")) {
            fprintf(stderr, "Invalid parameter %s\n", aargv[8]);
            return 1;
            }
        
        if (!strcmp(aargv[8], "-thai"))
            CHAR_BY_CHAR=THAI;
        else
            CHAR_BY_CHAR=OCCIDENTAL;
        
        if (!strcmp(aargv[8], "-space"))
            GESTION_DE_L_ESPACE=MODE_MORPHO;
        else
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
        
        if (strcmp(aargv[9], "-thai") && strcmp(aargv[9], "-space")) {
            fprintf(stderr, "Invalid parameter %s\n", aargv[9]);
            return 1;
            }
        
        if (!strcmp(aargv[9], "-thai"))
            CHAR_BY_CHAR=THAI;
        else
            CHAR_BY_CHAR=OCCIDENTAL;
        
        if (!strcmp(aargv[9], "-space"))
            GESTION_DE_L_ESPACE=MODE_MORPHO;
        else
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
        break;
    
    } 
/* $CD$ end */
if (locate_pattern(text_cod,tokens_txt,		                     // the text representations
	               aargv[2],					                 // FST
				   dlf,dlc,err,				                     // the text dictionaries
				   aargv[3],					                 // Alphabet
				   mode,					                     // SLA 
				   output_mode,				                     // IMR
                   dynamicSntDir) == 1) {
    return 0;
}
else {
return 1;
}
}

void LaunchLocateAsRoutine(char **argv, int fst_index){     
// maps  the right arguments of dico for the locate call 
// and launch the  locate program within dico.exe

	char** aargv;
	aargv=(char**)malloc(9*sizeof(char*));
 														
	aargv[0] = strdup("LaunchLocateAsRoutine");		     // if needed : just to know that the call come from here if necessary
	aargv[1] = strdup(argv[1]) ;						  //Text.snt ;

	char fst_name[1000], fst_ext[100];                  // argv[fst_index] name of the fst file  
    name_without_extension(argv [fst_index], fst_name);
	file_name_extension(argv [fst_index],fst_ext);
    strcat(fst_name,fst_ext);
   	aargv[2] = strdup(fst_name)  ;						

	aargv[3] = strdup(argv[2])  ;						//Alphabet.txt
	aargv[4] = strdup( "l");							// longest match
	aargv[5] = strdup("m");								// Merge mode
	aargv[6] = strdup("-1");							// search all occurences
	aargv[7] = strdup("" );								//  empty dynamic - directory 
	aargv[8] = strdup("");

	Call_Locate_Prog(7,aargv);							// argc must be 7 ; in LocateAsRoutine.cpp
}


//---------------------------------------------------------------------------


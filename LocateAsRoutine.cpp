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
#include "Fst2.h"
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
#include "LocateAsRoutine.h"
#include "Error.h"


/* 
 * This function behaves in the same way that a main one, except that it does
 * not invoke the setBufferMode function and that it does not print the
 * synopsis.
 */
int main_Locate(int argc, char **argv) {
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

int tokenization_mode;

/* $CD$ begin */
switch (argc) {

    case 7: // 6 arguments: pas de dynamic, pas de -thai, pas de -space

        strcpy(dynamicSntDir, staticSntDir);
        tokenization_mode=WORD_BY_WORD_TOKENIZATION;
        GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
        break;


    case 8: // 7 arguments: soit dynamic, soit -thai, soit -space

        if (!strcmp(argv[7], "-thai")) {
            strcpy(dynamicSntDir, staticSntDir);
            tokenization_mode=CHAR_BY_CHAR_TOKENIZATION;
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
            }
        else if (!strcmp(argv[7], "-space")) {
            strcpy(dynamicSntDir, staticSntDir);
            tokenization_mode=WORD_BY_WORD_TOKENIZATION;
            GESTION_DE_L_ESPACE=MODE_MORPHO;
            }
        else {
            strcpy(dynamicSntDir, argv[7]);
            tokenization_mode=WORD_BY_WORD_TOKENIZATION;
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
            tokenization_mode=CHAR_BY_CHAR_TOKENIZATION;
        else
            tokenization_mode=WORD_BY_WORD_TOKENIZATION;
        
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
            tokenization_mode=CHAR_BY_CHAR_TOKENIZATION;
        else
            tokenization_mode=WORD_BY_WORD_TOKENIZATION;
        
        if (!strcmp(argv[8], "-space"))
            GESTION_DE_L_ESPACE=MODE_MORPHO;
        else
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
        
        if (strcmp(argv[9], "-thai") && strcmp(argv[9], "-space")) {
            fprintf(stderr, "Invalid parameter %s\n", argv[9]);
            return 1;
            }
        
        if (!strcmp(argv[9], "-thai"))
            tokenization_mode=CHAR_BY_CHAR_TOKENIZATION;
        else
            tokenization_mode=WORD_BY_WORD_TOKENIZATION;
        
        if (!strcmp(argv[9], "-space"))
            GESTION_DE_L_ESPACE=MODE_MORPHO;
        else
            GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
        break;
    
    } 
/* $CD$ end */
int OK=locate_pattern(text_cod,tokens_txt,argv[2],dlf,dlc,err,argv[3],mode,output_mode,
               dynamicSntDir,tokenization_mode);
if (OK == 1) {
    return 0;
}
else {
return 1;
}
}


/**
 * Launches the Locate main function with the appropriate arguments.
 * This function is used to apply a .fst2 as dictionary in the Dico
 * program.
 * 
 * @author Alexis Neme
 * Modified by Sébastien Paumier
 */
void launch_locate_as_routine(char* text_snt,char* fst2,char* alphabet) {
char tmp[FILENAME_SIZE];
char tmp2[FILENAME_SIZE];
get_filename_path(alphabet,tmp);
name_without_extension(tmp,tmp2);
/* We test if we are working on Thai, on the basis of the alphabet file */
char path[FILENAME_SIZE];
char lang[FILENAME_SIZE];
get_filename_path(alphabet,path);
path[strlen(path)-1]='\0';
name_without_path(path,lang);
int thai=0;
if (!strcmp(lang,"Thai")) {
   thai=1;
}
char** argv;
argv=(char**)malloc((7+thai)*sizeof(char*));
if (argv==NULL) {
   fatal_error("Not enough memory in launch_locate_as_routine\n");
}
/* If needed: just to know that the call come from here if necessary */
argv[0]=strdup("launch_locate_as_routine");
argv[1]=strdup(text_snt);
argv[3]=strdup(alphabet);
/* We work in longuest match mode */
argv[4]=strdup("l");
/* And in merge mode */
argv[5]=strdup("m");
/* We look for all the occurrences */
argv[6]=strdup("all");
/* If needed, we add the -thai option */
if (thai) {
   argv[7]=strdup("-thai");
}
/* Finally, we call the main function of Locate */
main_Locate(7+thai,argv);
}


//---------------------------------------------------------------------------


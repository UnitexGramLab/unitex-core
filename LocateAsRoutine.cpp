 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Unicode.h"
#include "Alphabet.h"
#include "DELA.h"
#include "String_hash.h"
#include "LocatePattern.h"
#include "Fst2.h"
#include "Text_tokens.h"
#include "List_int.h"
#include "CompoundWordTree.h"
#include "Text_parsing.h"
#include "Matches.h"
#include "TransductionVariables.h"
#include "TransductionStack.h"
#include "ParsingInfo.h"
#include "File.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "LocateAsRoutine.h"
#include "Error.h"


/* 
 * This function behaves in the same way that a main one, except that it does
 * not invoke the setBufferMode function and that it does not print the
 * synopsis.
 */
int main_Locate(int argc,char* argv[]) {
/* $CD$ begin + overall */
char staticSntDir[FILENAME_MAX], dynamicSntDir[FILENAME_MAX];
/* $CD$ end */

char tokens_txt[FILENAME_MAX];
char text_cod[FILENAME_MAX];
char dlf[FILENAME_MAX];
char dlc[FILENAME_MAX];
char err[FILENAME_MAX];
char* morpho_dic=NULL;

get_snt_path((const char*)argv[1],staticSntDir);

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

MatchPolicy match_policy;
OutputPolicy output_policy;

if (!strcmp(argv[4],"s")) {
   match_policy=SHORTEST_MATCHES;
} else if (!strcmp(argv[4],"a")) {
          match_policy=ALL_MATCHES;
       }
  else if (!strcmp(argv[4],"l")) {
          match_policy=LONGEST_MATCHES;
       }
  else {
     error("Invalid parameter %s\n",argv[4]);
     return 1;
  }
if (!strcmp(argv[5],"i")) {
   output_policy=IGNORE_OUTPUTS;
} else if (!strcmp(argv[5],"m")) {
          output_policy=MERGE_OUTPUTS;
       }
  else if (!strcmp(argv[5],"r")) {
          output_policy=REPLACE_OUTPUTS;
       }
  else {
     error("Invalid parameter %s\n",argv[5]);
     return 1;
  }
int search_limit;
if (!strcmp(argv[6],"all")) {
   search_limit=NO_MATCH_LIMIT;
}
else {
   if (!sscanf(argv[6],"%d",&search_limit)) {
      error("Invalid parameter %s\n",argv[6]);
      return 1;
   }
}

TokenizationPolicy tokenization_policy;
SpacePolicy space_policy;

/* $CD$ begin */
switch (argc) {

    case 7: // 6 arguments: pas de dynamic, pas de -thai, pas de -space

        strcpy(dynamicSntDir, staticSntDir);
        tokenization_policy=WORD_BY_WORD_TOKENIZATION;
        space_policy=DONT_START_WITH_SPACE;
        break;


    case 8: // 7 arguments: soit dynamic, soit -thai, soit -space, soit -md=XXX

        if (!strcmp(argv[7], "-thai")) {
            strcpy(dynamicSntDir, staticSntDir);
            tokenization_policy=CHAR_BY_CHAR_TOKENIZATION;
            space_policy=DONT_START_WITH_SPACE;
            }
        else if (!strcmp(argv[7], "-space")) {
            strcpy(dynamicSntDir, staticSntDir);
            tokenization_policy=WORD_BY_WORD_TOKENIZATION;
            space_policy=START_WITH_SPACE;
            }
        else if (strstr(argv[7],"-md=")==argv[7]) {
            morpho_dic=argv[7]+4;
            strcpy(dynamicSntDir, staticSntDir);
            tokenization_policy=WORD_BY_WORD_TOKENIZATION;
            space_policy=DONT_START_WITH_SPACE;
            }
        else {
            strcpy(dynamicSntDir, argv[7]);
            tokenization_policy=WORD_BY_WORD_TOKENIZATION;
            space_policy=DONT_START_WITH_SPACE;
            }
        break; 

    
    case 9: // 8 arguments: 7 = dynamic, 8 = soit -thai, soit -space, soit -md=XXX
    
        strcpy(dynamicSntDir, argv[7]);
        
        if (strcmp(argv[8], "-thai") && strcmp(argv[8], "-space")
            && strstr(argv[8],"-md=")!=argv[8]) {
            error("Invalid parameter %s\n", argv[8]);
            return 1;
        }
        if (!strcmp(argv[8], "-thai"))
            tokenization_policy=CHAR_BY_CHAR_TOKENIZATION;
        else
            tokenization_policy=WORD_BY_WORD_TOKENIZATION;
        
        if (!strcmp(argv[8], "-space"))
            space_policy=START_WITH_SPACE;
        else
            space_policy=DONT_START_WITH_SPACE;
        if (strstr(argv[8],"-md=")==argv[8]) {
           morpho_dic=argv[8]+4;
        }
        break; 

    
    case 10: // 9 arguments: 7 = dynamic, 8 = or -thai, or -space, or -md=XXX
             //                           9 = or -space, or -thai, or -md=XXX
    
        strcpy(dynamicSntDir, argv[7]);
        
        if (strcmp(argv[8], "-thai") && strcmp(argv[8], "-space")
            && strstr(argv[8],"-md=")!=argv[8]) {
            error("Invalid parameter %s\n", argv[8]);
            return 1;
        }
        
        if (!strcmp(argv[8], "-thai"))
            tokenization_policy=CHAR_BY_CHAR_TOKENIZATION;
        else
            tokenization_policy=WORD_BY_WORD_TOKENIZATION;
        
        if (!strcmp(argv[8], "-space"))
            space_policy=START_WITH_SPACE;
        else
            space_policy=DONT_START_WITH_SPACE;
        if (strstr(argv[8],"-md=")==argv[8]) {
           morpho_dic=argv[8]+4;
        }
        if (strcmp(argv[9], "-thai") && strcmp(argv[9], "-space")
            && strstr(argv[9],"-md=")!=argv[9]) {
           error("Invalid parameter %s\n", argv[9]);
           return 1;
        }
        
        if (!strcmp(argv[9], "-thai"))
            tokenization_policy=CHAR_BY_CHAR_TOKENIZATION;
        else
            tokenization_policy=WORD_BY_WORD_TOKENIZATION;
        
        if (!strcmp(argv[9], "-space"))
            space_policy=START_WITH_SPACE;
        else
            space_policy=DONT_START_WITH_SPACE;
        if (strstr(argv[9],"-md=")==argv[9]) {
           morpho_dic=argv[9]+4;
        }
        break;

    case 11: // 10 arguments: 7 = dynamic, 8 = -thai, 9 = -space, and 10 = -md=XXX
    
        strcpy(dynamicSntDir, argv[7]);
        
        if (strcmp(argv[8], "-thai")) {
            error("Invalid parameter %s\n", argv[8]);
            return 1;
        }
        tokenization_policy=CHAR_BY_CHAR_TOKENIZATION;
        if (strcmp(argv[9], "-space")) {
           error("Invalid parameter %s\n", argv[9]);
           return 1;
        }
        space_policy=START_WITH_SPACE;
        if (strstr(argv[10],"-md=")!=argv[10]) {
           error("Invalid parameter %s\n", argv[10]);
        } 
        morpho_dic=argv[10]+4;
        break;
    
    } 
/* $CD$ end */
int OK=locate_pattern(text_cod,tokens_txt,argv[2],dlf,dlc,err,argv[3],match_policy,output_policy,
               dynamicSntDir,tokenization_policy,space_policy,search_limit,morpho_dic);
return (!OK);
}


/**
 * Launches the Locate main function with the appropriate arguments.
 * This function is used to apply a .fst2 as dictionary in the Dico
 * program.
 * 
 * @author Alexis Neme
 * Modified by Sébastien Paumier
 */
void launch_locate_as_routine(char* text_snt,char* fst2,char* alphabet,
                              OutputPolicy output_policy,char* morpho_dic) {
char tmp[FILENAME_MAX];
char tmp2[FILENAME_MAX];
get_path(alphabet,tmp);
remove_extension(tmp,tmp2);
/* We test if we are working on Thai, on the basis of the alphabet file */
char path[FILENAME_MAX];
char lang[FILENAME_MAX];
get_path(alphabet,path);
path[strlen(path)-1]='\0';
remove_path(path,lang);
int thai=0;
if (!strcmp(lang,"Thai")) {
   thai=1;
}
int md=0;
if (morpho_dic!=NULL) {
   md=1;
}
char** argv;
argv=(char**)malloc((7+thai+md)*sizeof(char*));
if (argv==NULL) {
   fatal_error("Not enough memory in launch_locate_as_routine\n");
}
/* If needed: just to know that the call come from here if necessary */
argv[0]=strdup("launch_locate_as_routine");
argv[2]=strdup(fst2);
argv[1]=strdup(text_snt);
argv[3]=strdup(alphabet);
/* We work in longuest match mode */
argv[4]=strdup("l");
/* We set the output policy */
switch (output_policy) {
   case MERGE_OUTPUTS: argv[5]=strdup("m"); break;
   case REPLACE_OUTPUTS: argv[5]=strdup("r"); break;
   default: argv[5]=strdup("i"); break;
}
/* We look for all the occurrences */
argv[6]=strdup("all");
/* If needed, we add the -thai option */
int index=7;
if (thai) {
   argv[index++]=strdup("-thai");
}
if (md) {
   char tmp[FILENAME_MAX];
   strcpy(tmp,"-md=");
   strcat(tmp,morpho_dic);
   argv[index++]=tmp;
}
/* Finally, we call the main function of Locate */
main_Locate(index,argv);
}


//---------------------------------------------------------------------------


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
#include <string.h>
#include <stdlib.h>
#include "Unicode.h"
#include "Fst2.h"
#include "GrfCheck.h"
#include "Copyright.h"
#include "Grf2Fst2_lib.h"
#include "Alphabet.h"
#include "IOBuffer.h"
#include "FileName.h"
#include "LocateConstants.h"
#include "Error.h"


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage : Grf2Fst2 <grf> [y/n] [ALPH] [-d <pckgPath>]\n"
         "      <grf> : main graph of grammar (must be an absolute path)\n"
         "      [y/n] : enable or not the loops/left-recursion detection\n"
         "      [ALPH] : name of the alphabet file to use for tokenizing\n"
         "               lexical units. If ALPH=char_by_char, lexical units\n"
         "               will be single letters. If this parameter is omitted,\n"
         "               lexical units will be sequences of any unicode letters.\n"
         "      <pckgPath> : path of the root dir of all grammar packages\n"
         "Compiles the grammar <grf> and saves the result in a FST2 file\n"
         "stored in the same directory as <grf>.\n");
}



int main(int argc,char *argv[]) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();
if(argc<2 || argc>6) {
   usage();
   return 0;
}
struct compilation_info* infos=new_compilation_info();
if(argc>=6) {
   if(!strcmp(argv[argc-2],"-d")){      
      strcpy(infos->repository,argv[argc-1]);
      argc -= 2;
   }
}
int check_recursion=0;
int index=0;
if (argc>=3) {
   if (!strcmp(argv[2],"y") || !strcmp(argv[2],"n")) {
      if (!strcmp(argv[2],"y")) check_recursion=1;
      if (argc==4) {index=3;}
   }
   else {
      if (argc==3) {
         index=2;
      } else {
         error("Extra parameter: %s\n",argv[2]);
         return 1;
      }
  }
}
if (index!=0) {
   if (!strcmp(argv[index],"char_by_char")) {
      infos->tokenization_policy=CHAR_BY_CHAR_TOKENIZATION;
   } else {
      infos->alphabet=load_alphabet(argv[index]);
      if (infos->alphabet==NULL) {
         error("Cannot load alphabet file %s\n",argv[index]);
         return 1;
      }
      infos->tokenization_policy=WORD_BY_WORD_TOKENIZATION;
   }
}
char fst2_file_name[FILENAME_MAX];
remove_extension(argv[1],fst2_file_name);
strcat(fst2_file_name,".fst2");
if ((infos->fst2=u_fopen(fst2_file_name,U_WRITE))==NULL) {
   error("Cannot open file %s\n",fst2_file_name);
   return 1;
}
u_fprintf(infos->fst2,"0000000000\n");
int result=compile_grf(argv[1],infos);
if (result==0) {
   error("Compilation has failed\n");
   free_compilation_info(infos);
   u_fclose(infos->fst2);
   return 1;
}
free_alphabet(infos->alphabet);
write_tags(infos->fst2,infos->tags);
u_fclose(infos->fst2);
write_number_of_graphs(fst2_file_name,infos->graph_names->size-1);
if (check_recursion) {
   if (!grf_OK(fst2_file_name)) {
      return 1;
   }
}
free_compilation_info(infos);
u_printf("Compilation has succeeded\n");
return 0;
}


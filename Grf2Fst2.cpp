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
#include "VerifierRecursion.h"
#include "Copyright.h"
#include "Grf2Fst2_lib.h"
#include "Alphabet.h"
#include "IOBuffer.h"
#include "FileName.h"


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

  setBufferMode();

  FILE *fs_comp;

  if(argc<2 || argc>6) {
    usage();
    return 0;
  }
  int TOKENIZATION_MODE=DEFAULT_TOKENIZATION;
  pckg_path[0] = '\0';
  if(argc >= 4){
    if(!strcmp(argv[argc - 2],"-d")){      
      u_strcpy(pckg_path,argv[argc - 1]);
    }
    argc -= 2;
  }
  
  int index=0;
  if (argc>=3) {
     if (!strcmp(argv[2],"y") || !strcmp(argv[2],"n")) {
        if (argc==4) {index=3;}
     }
     else {
        if (argc==3) {
           index=2;
        }
        else {
           error("Extra parameter: %s\n",argv[2]);
           return 1;
        }
     }
  }

  Alphabet* alph=NULL;
  if (index!=0) {
     if (!strcmp(argv[index],"char_by_char")) {
        TOKENIZATION_MODE=CHAR_BY_CHAR_TOKENIZATION;
     }
     else {
        alph=load_alphabet(argv[index]);
        if (alph==NULL) {
           error("Cannot load alphabet file %s\n",argv[index]);
           return 1;
        }
        TOKENIZATION_MODE=ALPHABET_TOKENIZATION;
     }
  }
  char fst2_file_name[FILENAME_MAX];
  remove_extension(argv[1],fst2_file_name);
  strcat(fst2_file_name,".fst2");

  if((fs_comp = u_fopen(fst2_file_name,U_WRITE)) == NULL)
    {
      error("Cannot open file %s\n",fst2_file_name);
      return 1;
   }

  donnees=(struct donnees_comp *) malloc(sizeof(struct donnees_comp));
  init_generale_comp();
  init_arbres_comp();

  u_fprintf(fs_comp,"0000000000\n");

  int result = compilation(argv[1],TOKENIZATION_MODE,alph,fs_comp);
  if (result == 0)
  {
    error("Compilation has failed\n");
    libere_arbres_comp();
    free(donnees);
    u_fclose(fs_comp);
    return 1;
  }

  if (alph!=NULL) {
     free_alphabet(alph);
  }

  sauvegarder_etiquettes_comp(fs_comp);

  libere_arbres_comp();
  free(donnees);
  u_fclose(fs_comp);
  ecrire_fichier_sortie_nb_graphes(fst2_file_name,fs_comp);
  if (argc>2 && (!strcmp(argv[2],"y"))) {
    if (!pas_de_recursion(fst2_file_name)) {
      free(fst2_file_name);
      return 1;
    }
  }
  u_printf("Compilation has succeeded\n");
  return 0;
 }




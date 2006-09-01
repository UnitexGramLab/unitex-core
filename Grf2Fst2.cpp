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
#include <string.h>
#include <stdlib.h>

#include "unicode.h"
#include "Fst2.h"
#include "VerifierRecursion.h"
#include "Copyright.h"
#include "Grf2Fst2_lib.h"
#include "Alphabet.h"
#include "IOBuffer.h"



//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////
/////////PROGRAMME PRINCIPAL ////////////////////////////////
/////////////////////////////////////////////////////////////


void usage() {
printf("%s",COPYRIGHT);
printf("Usage : Grf2Fst2 <grf> [y/n] [ALPH] [-d <pckgPath>]\n");
printf("      <grf> : main graph of grammar (must be an absolute path)\n");
printf("      <pckgPath> : path of the root dir of all grammar packages\n");
printf("      [y/n] : enable or not the loops/left-recursion detection\n");
printf("      [ALPH] : name of the alphabet file to use for tokenizing\n");
printf("               lexical units. If ALPH=char_by_char, lexical units\n");
printf("               will be single letters. If this parameter is omitted,\n");
printf("               lexical units will be sequences of any unicode letters.\n");
printf("Compile the grammar and saves the result in a FST2 file stored\n");
printf("in the same directory that <grf>.\n");
}



int main(int argc,char *argv[]) {
setBufferMode();

  char temp[TAILLE_MOT_GRAND_COMP];
  char temp1[TAILLE_MOT_GRAND_COMP];
  int l;

  if(argc<2 || argc>6) {
    usage();
    return 0;
  }
  int TOKENIZATION_MODE=DEFAULT_TOKENIZATION;
  pckg_path[0] = '\0';
  if(argc >= 4){
    if(!strcmp(argv[argc - 2],"-d")){      
      u_strcpy_char(pckg_path,argv[argc - 1]);
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
           fprintf(stderr,"Extra parameter: %s\n",argv[2]);
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
           fprintf(stderr,"Cannot load alphabet file %s\n",argv[index]);
           return 1;
        }
        TOKENIZATION_MODE=ALPHABET_TOKENIZATION;
     }
  }
  donnees=(struct donnees_comp *) malloc_comp(sizeof(struct donnees_comp));
  init_generale_comp();
  init_arbres_comp();
  strcpy(temp,argv[1]);

  if(ouverture_fichier_sortie(temp) == 0)
  {
    free_comp(donnees);
    libere_arbres_comp();
    fprintf(stderr,"Cannot open file %s\n",temp);
    return 1;
   }
   u_fprints_char("0000000000\n",fs_comp);
   int result=compilation(temp,TOKENIZATION_MODE,alph);
  if (result == 0)
  {
    fprintf(stderr,"Compilation has failed\n");
    libere_arbres_comp();
    free_comp(donnees);
    u_fclose(fs_comp);
    return 1;
  }
  if (alph!=NULL) {
     free_alphabet(alph);
  }
  sauvegarder_etiquettes_comp();
  libere_arbres_comp();
  free_comp(donnees);
  u_fclose(fs_comp);
  strcpy(temp1,temp);
  l = strlen(temp1);
  temp1[l-4] = '\0';
  strcat(temp1,".fst2");
  ecrire_fichier_sortie_nb_graphes(temp1);
  if (argc>=2 && (!strcmp(argv[2],"y"))) {
    if (!pas_de_recursion(temp1)) {
      return 1;
    }
  }
  printf("Compilation has succeeded\n");
  return 0;
 }




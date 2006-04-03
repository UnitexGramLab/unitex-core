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
#include "Fst2.h"
#include "LiberationFst2.h"
#include "Sentence_to_grf.h"
#include "FileName.h"
#include "Liste_nombres.h"
#include "Copyright.h"
#include "IOBuffer.h"

//
// "e:\my unitex\english\corpus\ivanhoe_snt\text.fst2" 1
//

void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Fst2Grf <text automata> <sentence> [<output>] [-f=<font>]\n");
printf("     <text automata> :  the FST2 file that contains the text automata.\n");
printf("     <sentence> :       the number of the sentence to be converted.\n");
printf("     <output> :         name GRF file as <output>.grf and the TXT one as <output>.txt (default cursentence)\n"); 
printf("     -f=<font> :        use the font <font> in the output .grf (Times new Roman by default).\n\n"); 
printf("Converts a sentence automaton into a GRF file that can be viewed. The\n");
printf("resulting file, named cursentence.grf, is stored in the same directory\n");
printf("that <text automata>. The text of the sentence is saved in the same\n");
printf("directory, in a file named cursentence.txt.\n");
}



int main(int argc, char **argv) {
setBufferMode();

  if (argc < 3 || argc > 5) {
    usage();
    return 0;
  }

  int SENTENCE;
  char nom_grf[2000];
  char nom_txt[2000];
  char* fontname=NULL;
  FILE * txt, * f;


  if (!sscanf(argv[2],"%d",&SENTENCE)) {
    fprintf(stderr,"Invalid sentence number %s\n",argv[2]);
    return 1;
  }

  get_filename_path(argv[1],nom_grf);
  get_filename_path(argv[1],nom_txt);

  switch (argc){
  case 3:
    strcat(nom_grf,"cursentence.grf");
    strcat(nom_txt,"cursentence.txt");
    break;
  case 4:
    if (argv[3][0]=='-' && argv[3][1]=='f' && argv[3][2]=='=') {
       fontname=&(argv[3][3]);
       strcat(nom_grf,"cursentence.grf");
       strcat(nom_txt,"cursentence.txt");
    } else {
        strcat(nom_grf, argv[3]);
        strcat(nom_grf, ".grf");

        strcat(nom_txt, argv[3]);
        strcat(nom_txt, ".txt");
    }
    break; 
  case 5:
       if (argv[4][0]=='-' && argv[4][1]=='f' && argv[4][2]=='=') {
          fontname=&(argv[4][3]);
       } else {
          fprintf(stderr,"Wrong parameter: %s\n",argv[4]);
       }  
        strcat(nom_grf, argv[3]);
        strcat(nom_grf, ".grf");
    
        strcat(nom_txt, argv[3]);
        strcat(nom_txt, ".txt");
        break;
  }
#ifdef OLD
  } else {
    if (argv[3][0]=='-' && argv[3][1]=='f' && argv[3][2]=='=') {
       fontname=&(argv[3][3]);
    }
    else {
    strcat(nom_grf, argv[3]);
    strcat(nom_grf, ".grf");

    strcat(nom_txt, argv[3]);
    strcat(nom_txt, ".txt");
    if (argc==5) {
       if (argv[4][0]=='-' && argv[4][1]=='f' && argv[4][2]=='=') {
          fontname=&(argv[4][3]);
       }
       else {
          fprintf(stderr,"Wrong parameter: %s\n",argv[4]);
       }
    }
    }
  }
#endif


  f = u_fopen(nom_grf,U_WRITE);
  if (f==NULL) {
    fprintf(stderr,"Cannot write file %s\n",nom_grf);
    return 1;
  }

  txt = u_fopen(nom_txt,U_WRITE);
  if (txt==NULL) {
    fprintf(stderr,"Cannot write file %s\n",nom_txt);
    u_fclose(f);
    return 1;
  }



Fst2* automate;
printf("Loading %s...\n",argv[1]);
automate=load_one_sentence_of_fst2(argv[1],SENTENCE,txt);
if (automate==NULL) {
   fprintf(stderr,"Cannot load text automata file %s\n",argv[1]);
   u_fclose(f);
   u_fclose(txt);
   return 1;
}
printf("Creating GRF...\n");
sentence_to_grf(automate,SENTENCE,fontname,f);
u_fclose(f);
u_fclose(txt);
free_fst2(automate);
printf("Done.\n");
return 0;
}
//---------------------------------------------------------------------------

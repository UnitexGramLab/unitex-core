 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "Fst2.h"
#include "Sentence2Grf.h"
#include "File.h"
#include "List_int.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Fst2Grf <text automata> <sentence> [<output>] [-f=<font>]\n");
u_printf("     <text automata> :  the FST2 file that contains the text automata.\n");
u_printf("     <sentence> :       the number of the sentence to be converted.\n");
u_printf("     <output> :         name GRF file as <output>.grf and the TXT one as <output>.txt (default cursentence)\n"); 
u_printf("     -f=<font> :        use the font <font> in the output .grf (Times new Roman by default).\n\n"); 
u_printf("Converts a sentence automaton into a GRF file that can be viewed. The\n");
u_printf("resulting file, named cursentence.grf, is stored in the same directory\n");
u_printf("that <text automata>. The text of the sentence is saved in the same\n");
u_printf("directory, in a file named cursentence.txt.\n");
}



int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

if (argc<3 || argc>5) {
   usage();
   return 0;
}
int SENTENCE;
char grf_name[FILENAME_MAX];
char txt_name[FILENAME_MAX];
char* fontname=NULL;
if (!sscanf(argv[2],"%d",&SENTENCE)) {
   error("Invalid sentence number %s\n",argv[2]);
   return 1;
}
get_path(argv[1],grf_name);
get_path(argv[1],txt_name);
switch (argc){
   case 3:
      strcat(grf_name,"cursentence.grf");
      strcat(txt_name,"cursentence.txt");
      break;
      
   case 4:
      if (argv[3][0]=='-' && argv[3][1]=='f' && argv[3][2]=='=') {
         fontname=&(argv[3][3]);
         strcat(grf_name,"cursentence.grf");
         strcat(txt_name,"cursentence.txt");
      } else {
         strcat(grf_name,argv[3]);
         strcat(grf_name,".grf");
         strcat(txt_name,argv[3]);
         strcat(txt_name,".txt");
      }
      break; 
      
   case 5:
      if (argv[4][0]=='-' && argv[4][1]=='f' && argv[4][2]=='=') {
         fontname=&(argv[4][3]);
      } else {
         error("Wrong parameter: %s\n",argv[4]);
      }  
      strcat(grf_name,argv[3]);
      strcat(grf_name,".grf");
      strcat(txt_name,argv[3]);
      strcat(txt_name,".txt");
      break;
}
FILE* f=u_fopen(grf_name,U_WRITE);
if (f==NULL) {
   error("Cannot file %s\n",grf_name);
   return 1;
}
FILE* txt=u_fopen(txt_name,U_WRITE);
if (txt==NULL) {
   error("Cannot file %s\n",txt_name);
   u_fclose(f);
   return 1;
}
u_printf("Loading %s...\n",argv[1]);
Fst2* fst2=load_one_sentence_from_fst2(argv[1],SENTENCE);
if (fst2==NULL) {
   error("Cannot load text automata file %s\n",argv[1]);
   u_fclose(f);
   u_fclose(txt);
   return 1;
}
u_fprintf(txt,"%S\n",fst2->graph_names[SENTENCE]);
u_fclose(txt);
u_printf("Creating GRF...\n");
sentence_to_grf(fst2,SENTENCE,fontname,f);
u_fclose(f);
free_Fst2(fst2);
u_printf("Done.\n");
return 0;
}


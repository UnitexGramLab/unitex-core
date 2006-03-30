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
  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ConcordMain.h"
#include "unicode.h"
#include "Text_tokens.h"
#include "String_hash.h"
#include "Liste_nombres.h"
#include "Alphabet.h"
#include "Matches.h"
#include "Concordance.h"
#include "FileName.h"
#include "Copyright.h"
#include "LocatePattern.h"

// for handling of line breaks (raw text concordance and when modifying text):
#define MAX_ENTER_CHAR 1000000
int enter_pos[MAX_ENTER_CHAR];
int n_enter_char;

int main_concord_cpp(int argc, char **argv) {
if (argc!=9 && argc!=10 && argc!=11) {
   return 0;
}
int fontsize;
if (1!=sscanf(argv[3],"%d",&fontsize)) {
   fprintf(stderr,"Invalid font size parameter %s\n",argv[3]);
   return 1;
}
FILE* concor=u_fopen(argv[1],U_READ);
if (concor==NULL) {
   fprintf(stderr,"Cannot open file %s\n",argv[1]);
   return 1;
}

char snt_dir[2000];
get_filename_path(argv[1],snt_dir); // we initialize the snt dir with the dir where we found concord.ind

char text_cod[2000];
char tokens_txt[2000];
char enter[2000];

int thai_mode=0;
if (argc!=9) {
    // we look for the -thai and <snt_dir> optional parameters
    if (strcmp(argv[9],"-thai")) {
       // if there is an extra parameter that is not -thai, then it is the <snt_dir> parameter
       strcpy(snt_dir,argv[9]);
    }
    else {
       // if there is -thai
       thai_mode=1;
       if (argc==11) {
          strcpy(snt_dir,argv[10]);
       }
    }
}
strcpy(text_cod,snt_dir);
strcat(text_cod,"text.cod");
strcpy(tokens_txt,snt_dir);
strcat(tokens_txt,"tokens.txt");
strcpy(enter,snt_dir);
strcat(enter,"enter.pos");

FILE* text=fopen(text_cod,"rb");
if (text==NULL) {
   fprintf(stderr,"Cannot open file %s\n",text_cod);
   u_fclose(concor);
   return 1;
}
struct text_tokens* tok=load_text_tokens(tokens_txt);
if (tok==NULL) {
   fprintf(stderr,"Cannot load text token file %s\n",tokens_txt);
   u_fclose(concor);
   fclose(text);
   return 1;
}
FILE* f_enter=fopen(enter,"rb");
if (f_enter==NULL) {
   fprintf(stderr,"Cannot open file %s\n",enter);
   n_enter_char=0;
}
else {
   n_enter_char=fread(&enter_pos,sizeof(int),MAX_ENTER_CHAR,f_enter);
   fclose(f_enter);
}

int longueur_avant;
int longueur_apres;
if (1!=sscanf(argv[4],"%d",&longueur_avant)) {
   fprintf(stderr,"Invalid left context length %s\n",argv[4]);
   u_fclose(concor);
   fclose(text);
   free_text_tokens(tok);
   return 1;
}
if (1!=sscanf(argv[5],"%d",&longueur_apres)) {
   fprintf(stderr,"Invalid right context length %s\n",argv[5]);
   u_fclose(concor);
   fclose(text);
   free_text_tokens(tok);
   return 1;
}
int sort_mode;
if (!strcmp(argv[6],"TO") || !strcmp(argv[6],"NULL"))
   sort_mode=TEXT_ORDER;
else if (!strcmp(argv[6],"LC"))
   sort_mode=LEFT_CENTER;
else if (!strcmp(argv[6],"LR"))
   sort_mode=LEFT_RIGHT;
else if (!strcmp(argv[6],"CL"))
   sort_mode=CENTER_LEFT;
else if (!strcmp(argv[6],"CR"))
   sort_mode=CENTER_RIGHT;
else if (!strcmp(argv[6],"RL"))
   sort_mode=RIGHT_LEFT;
else if (!strcmp(argv[6],"RC"))
   sort_mode=RIGHT_CENTER;
else {
   fprintf(stderr,"Invalid sort mode %s\n",argv[6]);
   u_fclose(concor);
   fclose(text);
   free_text_tokens(tok);
   return 1;
}
char f[2000];
get_filename_path(argv[1],f);
char program_path[2000];
get_filename_path(argv[0],program_path);
create_concordance(concor,text,tok,sort_mode,longueur_avant,longueur_apres,argv[2],
                   argv[3],f,argv[7],argv[8],n_enter_char,enter_pos,program_path,
                   thai_mode);
u_fclose(concor);
fclose(text);
free_text_tokens(tok);
printf("Done.\n");
return 0;
}


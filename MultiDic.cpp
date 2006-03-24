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

#include "MultiDic.h"


//
// loads a collection of dictionaries
//
struct multi_dic* init_multi_dic_from_parameters(char** argv,int argc) {
char nom_inf[1024];
char extension[128];
unsigned char* bin;
struct INF_codes* INF;
Automate_fst2* fst2;
struct multi_dic* M=(struct multi_dic*)malloc(sizeof(struct multi_dic));

M->tab=(struct dic**)malloc((argc-FIRST_DIC_PARAMETER)*sizeof(struct dic*));
M->N=0;

for (int priority=DELAF_HIGH_PRIORITY;priority<=DELAF_LOW_PRIORITY;priority++) {
   for (int i=FIRST_DIC_PARAMETER;i<argc;i++) {
      // we do a loop to apply several dictionaries
      get_file_name_extension_ignoring_case(argv[i],extension);
      if (!strcmp(extension,"BIN")) {
         // we are dealing with a .dic dictionary
         name_without_extension(argv[i],nom_inf);
         char C=nom_inf[strlen(nom_inf)-1];
         if ((priority==DELAF_HIGH_PRIORITY && C=='-') ||
             (priority==DELAF_NORMAL_PRIORITY && C!='-' && C!='+') ||
             (priority==DELAF_LOW_PRIORITY && C=='+')) {
             // the dictionary has the correct priority
             printf("Loading %s...\n%s",argv[i],CR);
             strcat(nom_inf,".inf");
             INF=load_INF_file(nom_inf);
             if (INF==NULL) {
                fprintf(stderr,"Cannot load %s\n",nom_inf);
             }
             else {
                bin=load_BIN_file(argv[i]);
                if (bin==NULL) {
                   free_INF_codes(INF);
                   fprintf(stderr,"Cannot load %s\n",argv[i]);
                }
                else {
                   // the dictionary has been loaded correctly
                   M->tab[M->N]=(struct dic*)malloc(sizeof(struct dic));
                   M->tab[M->N]->type=DELAF_BIN;
                   M->tab[M->N]->priority=(char)priority;
                   M->tab[M->N]->bin=bin;
                   M->tab[M->N]->INF=INF;
                   (M->N)++;
                }
             }
         }
      }
      else if (!strcmp(extension,"FST2")) {
         // we are dealing with a .fst2 grammar dictionary
         name_without_extension(argv[i],nom_inf);
         char C=nom_inf[strlen(nom_inf)-1];
         if ((priority==DELAF_HIGH_PRIORITY && C=='-') ||
             (priority==DELAF_NORMAL_PRIORITY && C!='-' && C!='+') ||
             (priority==DELAF_LOW_PRIORITY && C=='+')) {
             // the dictionary has the correct priority
             printf("Loading %s...\n%s",argv[i],CR);
             fst2=load_fst2(argv[i],0);
             if (fst2==NULL) {
                fprintf(stderr,"Cannot load %s\n",argv[i]);
             }
             else {
                // the dictionary has been loaded correctly
                M->tab[M->N]=(struct dic*)malloc(sizeof(struct dic));
                M->tab[M->N]->type=DELAF_FST2;
                M->tab[M->N]->priority=(char)priority;
                M->tab[M->N]->fst2=fst2;
                (M->N)++;
             }
         }
      }
      else if (priority==1) {
         // it's not necessary to print this error message more than once
         fprintf(stderr,"Bad dictionary extension: %s\n",argv[i]);
      }
   }
}
return M;
}


//
// frees a collection of dictionaries
//
void free_multi_dic(struct multi_dic* M) {
if (M==NULL) return;
for (int i=0;i<M->N;i++) {
   if (M->tab[i]->type==DELAF_BIN) {
      free(M->tab[i]->bin);
      free_INF_codes(M->tab[i]->INF);
   }
   else {
      free_fst2(M->tab[i]->fst2);
   }
   free(M->tab[i]);
}
}


//---------------------------------------------------------------------------

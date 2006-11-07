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
#include "Alphabet.h"
#include "DELA.h"
#include "FileName.h"
#include "Copyright.h"
#include "Matches.h"
#include "PortugueseNormalization.h"
#include "String_hash.h"
#include "Sentence_to_grf.h"
#include "Fst2.h"
#include "Normalization_transducer.h"
#include "Text_tokens.h"
#include "List_int.h"
#include "IOBuffer.h"



void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Reconstrucao <alph> <list> <root> <dic> <pro> <nasalpro> <res>\n");
printf("   <alph> : the alphabet file to use\n");
printf("   <list> : the match list that describes the forms to be normalized. This\n");
printf("            list must have been computed in MERGE or REPLACE mode.\n");
printf("   <root> : the .bin dictionary containing the radical forms\n");
printf("   <dic> : the .bin dictionary containing the complete forms\n");
printf("   <pro> : the .fst2 grammar describing pronoun rewriting rules\n");
printf("   <nasalpro> : the .fst2 grammar describing nasal pronoun rewriting rules\n");
printf("   <res> :  the name of the .grf graph to be generated\n\n");
printf("Takes a list of multi-part verbs and creates an apropriate normalization\ngrammar.\n");
}



int main(int argc, char **argv) {
setBufferMode();

if (argc!=8) {
   usage();
   return 0;
}
printf("Loading alphabet...\n");
Alphabet* alph=load_alphabet(argv[1]);
if (alph==NULL) {
   fprintf(stderr,"Cannot load alphabet file %s\n",argv[1]);
   return 1;
}
printf("Loading match list...\n");
FILE* f_list=u_fopen(argv[2],U_READ);
if (f_list==NULL) {
   fprintf(stderr,"Cannot load match list %s\n",argv[2]);
   free_alphabet(alph);
   return 1;
}
int TRANSDUCTION_MODE;
struct liste_matches* list=load_match_list(f_list,&TRANSDUCTION_MODE);
u_fclose(f_list);
if (TRANSDUCTION_MODE==IGNORE_TRANSDUCTIONS) {
   fprintf(stderr,"Invalid match list %s\n",argv[2]);
   free_alphabet(alph);
   return 1;
}
printf("Loading radical form dictionary...\n");
unsigned char* root_bin=load_BIN_file(argv[3]);
if (root_bin==NULL) {
   fprintf(stderr,"Cannot load radical form dictionary %s\n",argv[3]);
   free_alphabet(alph);
   return 1;
}
char root_inf_file[2000];
name_without_extension(argv[3],root_inf_file);
strcat(root_inf_file,".inf");
struct INF_codes* root_inf=load_INF_file(root_inf_file);
if (root_bin==NULL) {
   fprintf(stderr,"Cannot load radical form dictionary %s\n",root_inf_file);
   free_alphabet(alph);
   free(root_bin);
   return 1;
}
printf("Loading inflected form dictionary...\n");
unsigned char* inflected_bin=load_BIN_file(argv[4]);
if (inflected_bin==NULL) {
   fprintf(stderr,"Cannot load inflected form dictionary %s\n",argv[4]);
   free_alphabet(alph);
   free(root_bin);
   free_INF_codes(root_inf);
   return 1;
}
char inflected_inf_file[2000];
name_without_extension(argv[4],inflected_inf_file);
strcat(inflected_inf_file,".inf");
struct INF_codes* inflected_inf=load_INF_file(inflected_inf_file);
if (inflected_inf==NULL) {
   fprintf(stderr,"Cannot load inflected form dictionary %s\n",inflected_inf_file);
   free_alphabet(alph);
   free(root_bin);
   free(inflected_bin);
   free_INF_codes(root_inf);
   return 1;
}
printf("Loading pronoun rewriting rule grammar...\n");
struct noeud_arbre_normalization* rewriting_rules=load_normalization_transducer_string(argv[5]);
if (rewriting_rules==NULL) {
   fprintf(stderr,"Cannot load pronoun rewriting grammar %s\n",argv[5]);
   free_alphabet(alph);
   free(root_bin);
   free(inflected_bin);
   free_INF_codes(root_inf);
   free_INF_codes(inflected_inf);
   return 1;
}
printf("Loading nasal pronoun rewriting rule grammar...\n");
struct noeud_arbre_normalization* nasal_rewriting_rules=load_normalization_transducer_string(argv[6]);
if (rewriting_rules==NULL) {
   fprintf(stderr,"Cannot load nasal pronoun rewriting grammar %s\n",argv[6]);
   free_alphabet(alph);
   free(root_bin);
   free(inflected_bin);
   free_INF_codes(root_inf);
   free_INF_codes(inflected_inf);
   free_noeud_arbre_normalization(rewriting_rules);
   return 1;
}


printf("Constructing normalization grammar...\n");
build_portuguese_normalization_grammar(alph,list,root_bin,root_inf,inflected_bin,inflected_inf,argv[7],
                                       rewriting_rules,nasal_rewriting_rules);

free_alphabet(alph);
free(root_bin);
free_INF_codes(root_inf);
free(inflected_bin);
free_INF_codes(inflected_inf);
free_noeud_arbre_normalization(rewriting_rules);
free_noeud_arbre_normalization(nasal_rewriting_rules);
return 0;
}
//---------------------------------------------------------------------------



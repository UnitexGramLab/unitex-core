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
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "Alphabet.h"
#include "DELA.h"
#include "FileName.h"
#include "Copyright.h"
#include "Matches.h"
#include "PortugueseNormalization.h"
#include "String_hash.h"
#include "Sentence2Grf.h"
#include "Fst2.h"
#include "NormalizationFst2.h"
#include "Text_tokens.h"
#include "List_int.h"
#include "IOBuffer.h"
#include "Error.h"



void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Reconstrucao <alph> <list> <root> <dic> <pro> <nasalpro> <res>\n");
u_printf("   <alph> : the alphabet file to use\n");
u_printf("   <list> : the match list that describes the forms to be normalized. This\n");
u_printf("            list must have been computed in MERGE or REPLACE mode.\n");
u_printf("   <root> : the .bin dictionary containing the radical forms\n");
u_printf("   <dic> : the .bin dictionary containing the complete forms\n");
u_printf("   <pro> : the .fst2 grammar describing pronoun rewriting rules\n");
u_printf("   <nasalpro> : the .fst2 grammar describing nasal pronoun rewriting rules\n");
u_printf("   <res> :  the name of the .grf graph to be generated\n\n");
u_printf("Takes a list of multi-part verbs and creates an apropriate normalization\ngrammar.\n");
}



int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */setBufferMode();

if (argc!=8) {
   usage();
   return 0;
}
u_printf("Loading alphabet...\n");
Alphabet* alph=load_alphabet(argv[1]);
if (alph==NULL) {
   fatal_error("Cannot load alphabet file %s\n",argv[1]);
   return 1;
}
u_printf("Loading match list...\n");
FILE* f_list=u_fopen(argv[2],U_READ);
if (f_list==NULL) {
   error("Cannot load match list %s\n",argv[2]);
   free_alphabet(alph);
   return 1;
}
OutputPolicy output_policy;
struct match_list* list=load_match_list(f_list,&output_policy);
u_fclose(f_list);
if (output_policy==IGNORE_OUTPUTS) {
   error("Invalid match list %s\n",argv[2]);
   free_alphabet(alph);
   return 1;
}
u_printf("Loading radical form dictionary...\n");
unsigned char* root_bin=load_BIN_file(argv[3]);
if (root_bin==NULL) {
   error("Cannot load radical form dictionary %s\n",argv[3]);
   free_alphabet(alph);
   return 1;
}
char root_inf_file[FILENAME_MAX];
remove_extension(argv[3],root_inf_file);
strcat(root_inf_file,".inf");
struct INF_codes* root_inf=load_INF_file(root_inf_file);
if (root_bin==NULL) {
   error("Cannot load radical form dictionary %s\n",root_inf_file);
   free_alphabet(alph);
   free(root_bin);
   return 1;
}
u_printf("Loading inflected form dictionary...\n");
unsigned char* inflected_bin=load_BIN_file(argv[4]);
if (inflected_bin==NULL) {
   error("Cannot load inflected form dictionary %s\n",argv[4]);
   free_alphabet(alph);
   free(root_bin);
   free_INF_codes(root_inf);
   return 1;
}
char inflected_inf_file[FILENAME_MAX];
remove_extension(argv[4],inflected_inf_file);
strcat(inflected_inf_file,".inf");
struct INF_codes* inflected_inf=load_INF_file(inflected_inf_file);
if (inflected_inf==NULL) {
   error("Cannot load inflected form dictionary %s\n",inflected_inf_file);
   free_alphabet(alph);
   free(root_bin);
   free(inflected_bin);
   free_INF_codes(root_inf);
   return 1;
}
u_printf("Loading pronoun rewriting rule grammar...\n");
struct normalization_tree* rewriting_rules=load_normalization_transducer_string(argv[5]);
if (rewriting_rules==NULL) {
   error("Cannot load pronoun rewriting grammar %s\n",argv[5]);
   free_alphabet(alph);
   free(root_bin);
   free(inflected_bin);
   free_INF_codes(root_inf);
   free_INF_codes(inflected_inf);
   return 1;
}
u_printf("Loading nasal pronoun rewriting rule grammar...\n");
struct normalization_tree* nasal_rewriting_rules=load_normalization_transducer_string(argv[6]);
if (rewriting_rules==NULL) {
   error("Cannot load nasal pronoun rewriting grammar %s\n",argv[6]);
   free_alphabet(alph);
   free(root_bin);
   free(inflected_bin);
   free_INF_codes(root_inf);
   free_INF_codes(inflected_inf);
   free_normalization_tree(rewriting_rules);
   return 1;
}
u_printf("Constructing normalization grammar...\n");
build_portuguese_normalization_grammar(alph,list,root_bin,root_inf,inflected_bin,inflected_inf,argv[7],
                                       rewriting_rules,nasal_rewriting_rules);
free_alphabet(alph);
free(root_bin);
free_INF_codes(root_inf);
free(inflected_bin);
free_INF_codes(inflected_inf);
free_normalization_tree(rewriting_rules);
free_normalization_tree(nasal_rewriting_rules);
return 0;
}



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

#include "LocatePattern.h"
#include "Error.h"
#include "LemmaTree.h"
#include "Pattern.h"
#include "PatternTree.h"
#include "LocateFst2Tags.h"
#include "BitMasks.h"


/**
 * Allocates, initializes and returns a new locate_parameters structure.
 */
struct locate_parameters* new_locate_parameters() {
struct locate_parameters* p=(struct locate_parameters*)malloc(sizeof(struct locate_parameters));
if (p==NULL) {
   fatal_error("Not enough memory in new_locate_parameters\n");
}
p->token_control=NULL;
p->matching_patterns=NULL;
p->current_compound_pattern=0;
p->pattern_tree_root=NULL;
/* We use -1 because there may be no space, {S} or {STOP} in the text */
p->SPACE=-1;
p->SENTENCE=-1;
p->STOP=-1;
p->tag_token_list=NULL;
#ifdef TRE_WCHAR
p->filters=NULL;
p->filter_match_index=NULL;
#endif
p->DLC_tree=NULL;
p->optimized_states=NULL;
p->fst2=NULL;
p->tokens=NULL;
p->current_origin=-1;
return p;
}


/**
 * Frees a locate_parameters structure.
 */
void free_locate_parameters(struct locate_parameters* p) {
if (p==NULL) return;
free(p);
}


int locate_pattern(char* text,char* tokens,char* fst2_name,char* dlf,char* dlc,char* err,
                   char* alphabet,int mode,int output_mode, char* dynamicDir,
                   int tokenization_mode) {
struct locate_parameters* parameters=new_locate_parameters();

FILE* text_file;
FILE* out;
FILE* info;
long int text_size=u_file_size(text)/4;
text_file=fopen(text,"rb");
if (text_file==NULL) {
   error("Cannot load %s\n",text);
   return 0;
}
char concord[1000];
char concord_info[1000];

strcpy(concord,dynamicDir);
strcat(concord,"concord.ind");

strcpy(concord_info,dynamicDir);
strcat(concord_info,"concord.n");

out=u_fopen(concord,U_WRITE);
if (out==NULL) {
   error("Cannot write %s\n",concord);
   fclose(text_file);
   return 0;
}
info=u_fopen(concord_info,U_WRITE);
if (info==NULL) {
   error("Cannot write %s\n",concord_info);
}
switch(output_mode) {
   case IGNORE_TRANSDUCTIONS: u_fprints_char("#I\n",out); break;
   case MERGE_TRANSDUCTIONS: u_fprints_char("#M\n",out); break;
   case REPLACE_TRANSDUCTIONS: u_fprints_char("#R\n",out); break;
}
printf("Loading alphabet...\n");
Alphabet* alph=load_alphabet(alphabet);
if (alph==NULL) {
   error("Cannot load alphabet file %s\n",alphabet);
   return 0;
}
struct string_hash* semantic_codes=new_string_hash();
extract_semantic_codes(dlf,semantic_codes);
extract_semantic_codes(dlc,semantic_codes);
printf("Loading fst2...\n");
parameters->fst2=load_fst2(fst2_name,1);
if (parameters->fst2==NULL) {
   error("Cannot load grammar %s\n",fst2_name);
   free_alphabet(alph);
   free_string_hash(semantic_codes);
   return 0;
}
parameters->tags=parameters->fst2->tags;
#ifdef TRE_WCHAR
parameters->filters=new_FilterSet(parameters->fst2,alph);
if (parameters->filters==NULL) {
   error("Cannot compile filter(s)\n");
   free_alphabet(alph);
   free_string_hash(semantic_codes);
   return 0;
}
#endif

printf("Loading token list...\n");
parameters->tokens=load_text_tokens_hash(tokens,&(parameters->SENTENCE),&(parameters->STOP));
if (parameters->tokens==NULL) {
   error("Cannot load token list %s\n",tokens);
   free_alphabet(alph);
   free_string_hash(semantic_codes);
   free_Fst2(parameters->fst2);
   return 0;
}

#ifdef TRE_WCHAR
parameters->filter_match_index=new_FilterMatchIndex(parameters->filters,parameters->tokens);
if (parameters->filter_match_index==NULL) {
   error("Cannot optimize filter(s)\n");
   free_alphabet(alph);
   free_string_hash(semantic_codes);
   free_string_hash(parameters->tokens);
   free_Fst2(parameters->fst2);
   return 0;
}
#endif

extract_semantic_codes_from_tokens(parameters->tokens,semantic_codes);
parameters->token_control=(unsigned char*)malloc(NUMBER_OF_TEXT_TOKENS*sizeof(unsigned char));
if (parameters->token_control==NULL) {
   fatal_error("Not enough memory in locate_pattern\n");
}
parameters->matching_patterns=(struct bit_array**)malloc(NUMBER_OF_TEXT_TOKENS*sizeof(struct bit_array*));
if (parameters->matching_patterns==NULL) {
   fatal_error("Not enough memory in locate_pattern\n");
}
for (int i=0;i<NUMBER_OF_TEXT_TOKENS;i++) {
  parameters->token_control[i]=0;
  parameters->matching_patterns[i]=NULL;
}
compute_token_controls(alph,err,tokenization_mode,parameters);
int number_of_patterns,is_DIC,is_CDIC,is_SDIC;
parameters->pattern_tree_root=new_pattern_node();
printf("Computing fst2 tags...\n");
process_tags(&number_of_patterns,semantic_codes,&is_DIC,&is_CDIC,&is_SDIC,parameters);
parameters->current_compound_pattern=number_of_patterns;
parameters->DLC_tree=new_DLC_tree(parameters->tokens->size);
struct lemma_node* root=new_lemma_node();
printf("Loading dlf...\n");
load_dic_for_locate(dlf,alph,number_of_patterns,is_DIC,is_CDIC,
				is_SDIC,tokenization_mode,root,parameters);
printf("Loading dlc...\n");
load_dic_for_locate(dlc,alph,number_of_patterns,is_DIC,is_CDIC,
				is_SDIC,tokenization_mode,root,parameters);
/* We look if tag tokens like "{today,.ADV}" verify some patterns */
check_patterns_for_tag_tokens(alph,number_of_patterns,tokenization_mode,root,parameters);
printf("Optimizing fst2 pattern tags...\n");
optimize_pattern_tags(alph,tokenization_mode,root,parameters);
printf("Optimizing compound word dictionary...\n");
optimize_DLC(parameters->DLC_tree);
free_string_hash(semantic_codes);
init_transduction_variable_index(parameters->fst2->variables);
printf("Optimizing fst2...\n");
parameters->optimized_states=build_optimized_fst2_states(parameters->fst2);
#warning to replace by simple lists of integers
printf("Optimizing patterns...\n");
init_pattern_transitions(parameters->tokens);
convert_pattern_lists(parameters->tokens);

printf("Working...\n");
launch_locate(text_file,mode,out,output_mode,text_size,info,parameters);
free_transduction_variable_index();
fclose(text_file);
if (info!=NULL) u_fclose(info);
u_fclose(out);
printf("Freeing memory...\n");
free_optimized_states(parameters->optimized_states,parameters->fst2->number_of_states);
free_DLC_tree(parameters->DLC_tree);
free_Fst2(parameters->fst2);
/* We don't free 'parameters->tags' because it was just a link on 'parameters->fst2->tags' */
free_alphabet(alph);
free_string_hash(parameters->tokens);
free_list_int(parameters->tag_token_list);
free_lemma_node(root);
free(parameters->token_control);
for (int i=0;i<NUMBER_OF_TEXT_TOKENS;i++) {
   free_bit_array(parameters->matching_patterns[i]);
}
free(parameters->matching_patterns);
#ifdef TRE_WCHAR
free_FilterSet(parameters->filters);
free_FilterMatchIndex(parameters->filter_match_index);
#endif
free_locate_parameters(parameters);
printf("Done.\n");
return 1;
}





/**
 * Returns a control byte that represents the characteristics of the given token.
 */
unsigned char get_control_byte(unichar* token,Alphabet* alph,struct string_hash* err,int tokenization_mode) {
int i;
int tmp;
unsigned char c=0;
if (token==NULL || token[0]=='\0') {
   fatal_error("NULL or empty token in get_control_byte\n");
}
/* We consider that a token starting with a letter is a word */
if (is_letter(token[0],alph)) {
   set_bit_mask(&c,MOT_TOKEN_BIT_MASK);
   /* If a token is a word, we check if it is in the 'err' word list
    * in order to answer the question <!DIC> */
   if (err!=NULL && get_value_index(token,err,DONT_INSERT)!=-1) {
      set_bit_mask(&c,NOT_DIC_TOKEN_BIT_MASK);
   }
   if (is_upper(token[0],alph)) {
      set_bit_mask(&c,PRE_TOKEN_BIT_MASK);
      i=0;
      tmp=0;
      while (token[i]!='\0') {
         if (is_lower(token[i],alph)) {
            tmp=1;
            break;
         }
         i++;
      }
      if (!tmp) {
         set_bit_mask(&c,MAJ_TOKEN_BIT_MASK);
      }
      return c;
   }
   i=0;
   tmp=0;
   while (token[i]!='\0') {
      if (is_upper(token[i],alph)) {
         tmp=1;
         break;
      }
      i++;
   }
   if (!tmp) {
      set_bit_mask(&c,MIN_TOKEN_BIT_MASK);
   }
   return c;
}
/* If the token doesn't start with a letter, we start with 
 * checking if it is a tag like {today,.ADV} */
if (token[0]=='{' && u_strcmp_char(token,"{S}") && u_strcmp_char(token,"{STOP}")) {
   /* Anyway, such a tag is classed as verifying <MOT> and <DIC> */
   set_bit_mask(&c,MOT_TOKEN_BIT_MASK|DIC_TOKEN_BIT_MASK);
   struct dela_entry* temp=tokenize_tag_token(token);
   if (is_upper(temp->inflected[0],alph)) {
      set_bit_mask(&c,PRE_TOKEN_BIT_MASK);
      i=0;
      tmp=0;
      while (temp->inflected[i]!='\0') {
         if (is_letter(temp->inflected[i],alph) && is_lower(temp->inflected[i],alph)) {
            tmp=1;
            break;
         }
         i++;
      }
      if (!tmp) {
         set_bit_mask(&c,MAJ_TOKEN_BIT_MASK);
      }
   }
   else {
      i=0;
      tmp=0;
      while (temp->inflected[i]!='\0') {
         if (is_letter(temp->inflected[i],alph) && is_upper(temp->inflected[i],alph)) {
            tmp=1;
            break;
         }
         i++;
      }
      if (!tmp) {
         set_bit_mask(&c,MIN_TOKEN_BIT_MASK);
      }
   }
   if (!is_a_simple_word(temp->inflected,alph,tokenization_mode)) {
      /* If the tag is a compound word, we say that it verifies the <CDIC> pattern */
      set_bit_mask(&c,CDIC_TOKEN_BIT_MASK);
   }
   free_dela_entry(temp);
}
return c;
}





void compute_token_controls(Alphabet* alph,char* err,int tokenization_mode,
                            struct locate_parameters* p) {
struct string_hash* ERR=load_key_list(err);
int n=p->tokens->size;
for (int i=0;i<n;i++) {
   p->token_control[i]=get_control_byte(p->tokens->value[i],alph,ERR,tokenization_mode);
}
free_string_hash(ERR);
}


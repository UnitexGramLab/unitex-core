 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
#include "Tokenization.h"
#include "File.h"


void load_dic_for_locate(char*,int,Alphabet*,int,int,int,struct lemma_node*,struct locate_parameters*);
void check_patterns_for_tag_tokens(Alphabet*,int,struct lemma_node*,struct locate_parameters*);
void load_morphological_dictionaries(char* morpho_dic_list,struct locate_parameters* p);


/**
 * Allocates, initializes and returns a new locate_parameters structure.
 */
struct locate_parameters* new_locate_parameters(U_FILE* fileread) {
struct locate_parameters* p=(struct locate_parameters*)malloc(sizeof(struct locate_parameters));
if (p==NULL) {
   fatal_alloc_error("new_locate_parameters");
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
p->absolute_offset=0;
p->current_origin=-1;
p->token_buffer=new_buffer_for_file(INTEGER_BUFFER,fileread);
p->buffer=p->token_buffer->int_buffer;
p->tokenization_policy=WORD_BY_WORD_TOKENIZATION;
p->space_policy=DONT_START_WITH_SPACE;
p->matching_units=0;
p->match_policy=LONGEST_MATCHES;
p->output_policy=IGNORE_OUTPUTS;
p->ambiguous_output_policy=ALLOW_AMBIGUOUS_OUTPUTS;
p->variable_error_policy=IGNORE_VARIABLE_ERRORS;
p->match_list=NULL;
p->number_of_matches=0;
p->number_of_outputs=0;
p->start_position_last_printed_match=-1;
p->end_position_last_printed_match=-1;
p->search_limit=0;
p->variables=NULL;
p->stack=new_stack_unichar(TRANSDUCTION_STACK_SIZE);
p->alphabet=NULL;
p->morpho_dic_inf=NULL;
p->morpho_dic_inf_free=NULL;
p->morpho_dic_bin=NULL;
p->morpho_dic_bin_free=NULL;
p->n_morpho_dics=0;
p->dic_variables=NULL;
p->left_ctx_shift=0;
p->left_ctx_base=0;
p->protect_dic_chars=0;
p->jamo=NULL;
p->jamo_tags=NULL;
p->jamo2syl=NULL;
p->mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
return p;
}


/**
 * Frees a locate_parameters structure.
 */
void free_locate_parameters(struct locate_parameters* p) {
if (p==NULL) return;
free(p);
}


/**
 * Returns an array containing the jamo versions of all the given tokens.
 */
unichar** create_jamo_tags(jamoCodage* jamo,struct string_hash* tokens,Alphabet* alphabet) {
unichar** res=(unichar**)malloc(tokens->size*sizeof(unichar*));
unichar foo[128];
for (int i=0;i<tokens->size;i++) {
	if (!u_strcmp(tokens->value[i],"{S}")) {
		res[i]=u_strdup("{S}");
	} else {
	   convert_Korean_text(tokens->value[i],foo,jamo,alphabet);
	   res[i]=u_strdup(foo);
	}
}
return res;
}


int locate_pattern(char* text,char* tokens,char* fst2_name,char* dlf,char* dlc,char* err,
                   char* alphabet,MatchPolicy match_policy,OutputPolicy output_policy,
                   Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input,
                   char* dynamicDir,TokenizationPolicy tokenization_policy,
                   SpacePolicy space_policy,int search_limit,char* morpho_dic_list,
                   AmbiguousOutputPolicy ambiguous_output_policy,
                   VariableErrorPolicy variable_error_policy,int protect_dic_chars,
                   char* jamo,char* korean_fst2) {

U_FILE* text_file;
U_FILE* out;
U_FILE* info;
text_file=u_fopen(BINARY,text,U_READ);
if (text_file==NULL) {
   error("Cannot load %s\n",text);
   return 0;
}
long save_pos=ftell(text_file);
fseek(text_file,0,SEEK_END);
long text_size=ftell(text_file)/sizeof(int);
fseek(text_file,save_pos,SEEK_SET);

struct locate_parameters* p=new_locate_parameters(text_file);
p->match_policy=match_policy;
p->tokenization_policy=tokenization_policy;
p->space_policy=space_policy;
p->output_policy=output_policy;
p->search_limit=search_limit;
p->ambiguous_output_policy=ambiguous_output_policy;
p->variable_error_policy=variable_error_policy;
p->protect_dic_chars=protect_dic_chars;
p->mask_encoding_compatibility_input = mask_encoding_compatibility_input;

char concord[FILENAME_MAX];
char concord_info[FILENAME_MAX];

strcpy(concord,dynamicDir);
strcat(concord,"concord.ind");

strcpy(concord_info,dynamicDir);
strcat(concord_info,"concord.n");

out=u_fopen_versatile_encoding(encoding_output,bom_output,mask_encoding_compatibility_input,concord,U_WRITE);
if (out==NULL) {
   error("Cannot write %s\n",concord);
   u_fclose(text_file);
   return 0;
}
info=u_fopen_versatile_encoding(encoding_output,bom_output,mask_encoding_compatibility_input,concord_info,U_WRITE);
if (info==NULL) {
   error("Cannot write %s\n",concord_info);
}
switch(output_policy) {
   case IGNORE_OUTPUTS: u_fprintf(out,"#I\n"); break;
   case MERGE_OUTPUTS: u_fprintf(out,"#M\n"); break;
   case REPLACE_OUTPUTS: u_fprintf(out,"#R\n"); break;
}
if (alphabet!=NULL && alphabet[0]!='\0') {
   u_printf("Loading alphabet...\n");
   p->alphabet=load_alphabet(alphabet,jamo!=NULL);
   if (p->alphabet==NULL) {
      error("Cannot load alphabet file %s\n",alphabet);
      return 0;
   }
}
struct string_hash* semantic_codes=new_string_hash();
extract_semantic_codes(dlf,semantic_codes);
extract_semantic_codes(dlc,semantic_codes);
u_printf("Loading fst2...\n");
p->fst2=load_abstract_fst2(fst2_name,1,NULL);
if (p->fst2==NULL) {
   error("Cannot load grammar %s\n",fst2_name);
   free_alphabet(p->alphabet);
   free_string_hash(semantic_codes);
   return 0;
}
p->tags=p->fst2->tags;
#ifdef TRE_WCHAR
p->filters=new_FilterSet(p->fst2,p->alphabet);
if (p->filters==NULL) {
   error("Cannot compile filter(s)\n");
   free_alphabet(p->alphabet);
   free_string_hash(semantic_codes);
   return 0;
}
#endif
u_printf("Loading token list...\n");
int n_text_tokens=0;
p->tokens=load_text_tokens_hash(tokens,mask_encoding_compatibility_input,&(p->SENTENCE),&(p->STOP),&n_text_tokens);
if (p->tokens==NULL) {
   error("Cannot load token list %s\n",tokens);
   free_alphabet(p->alphabet);
   free_string_hash(semantic_codes);
   free_abstract_Fst2(p->fst2,NULL);
   return 0;
}

#ifdef TRE_WCHAR
p->filter_match_index=new_FilterMatchIndex(p->filters,p->tokens);
if (p->filter_match_index==NULL) {
   error("Cannot optimize filter(s)\n");
   free_alphabet(p->alphabet);
   free_string_hash(semantic_codes);
   free_string_hash(p->tokens);
   free_abstract_Fst2(p->fst2,NULL);
   return 0;
}
#endif

extract_semantic_codes_from_tokens(p->tokens,semantic_codes);
u_printf("Loading morphological dictionaries...\n");
load_morphological_dictionaries(morpho_dic_list,p);
extract_semantic_codes_from_morpho_dics(p->morpho_dic_inf,p->n_morpho_dics,semantic_codes);
p->token_control=(unsigned char*)malloc(n_text_tokens*sizeof(unsigned char));
if (p->token_control==NULL) {
   fatal_alloc_error("locate_pattern");
}
p->matching_patterns=(struct bit_array**)malloc(n_text_tokens*sizeof(struct bit_array*));
if (p->matching_patterns==NULL) {
   fatal_alloc_error("locate_pattern");
}
for (int i=0;i<n_text_tokens;i++) {
  p->token_control[i]=0;
  p->matching_patterns[i]=NULL;
}
compute_token_controls(p->alphabet,err,p);
int number_of_patterns,is_DIC,is_CDIC,is_SDIC;
p->pattern_tree_root=new_pattern_node();
u_printf("Computing fst2 tags...\n");
process_tags(&number_of_patterns,semantic_codes,&is_DIC,&is_CDIC,&is_SDIC,p);
p->current_compound_pattern=number_of_patterns;
p->DLC_tree=new_DLC_tree(p->tokens->size);
struct lemma_node* root=new_lemma_node();
u_printf("Loading dlf...\n");
load_dic_for_locate(dlf,mask_encoding_compatibility_input,p->alphabet,number_of_patterns,is_DIC,is_CDIC,root,p);
u_printf("Loading dlc...\n");
load_dic_for_locate(dlc,mask_encoding_compatibility_input,p->alphabet,number_of_patterns,is_DIC,is_CDIC,root,p);
/* We look if tag tokens like "{today,.ADV}" verify some patterns */
check_patterns_for_tag_tokens(p->alphabet,number_of_patterns,root,p);
u_printf("Optimizing fst2 pattern tags...\n");
optimize_pattern_tags(p->alphabet,root,p);
u_printf("Optimizing compound word dictionary...\n");
optimize_DLC(p->DLC_tree);
free_string_hash(semantic_codes);
p->variables=new_Variables(p->fst2->variables);
u_printf("Optimizing fst2...\n");
p->optimized_states=build_optimized_fst2_states(p->variables,p->fst2);
if (jamo!=NULL && jamo[0]!='\0') {
	p->jamo=new jamoCodage();
	p->jamo->loadJamoMap(jamo);
	/* We also initializes the Chinese -> Hangul table */
	p->jamo->cloneHJAMap(p->alphabet->korean_equivalent_syllab);
	p->jamo_tags=create_jamo_tags(p->jamo,p->tokens,p->alphabet);
	p->jamo2syl=new Jamo2Syl();
	p->jamo2syl->init(jamo,korean_fst2);
}

u_printf("Working...\n");
launch_locate(text_file,out,text_size,info,p);

free_buffer(p->token_buffer);
free_Variables(p->variables);
u_fclose(text_file);
if (info!=NULL) u_fclose(info);
u_fclose(out);
free_optimized_states(p->optimized_states,p->fst2->number_of_states);
free_stack_unichar(p->stack);
/** Too long to free the DLC tree if it is big
 * free_DLC_tree(p->DLC_tree);
 */
free_pattern_node(p->pattern_tree_root);
free_abstract_Fst2(p->fst2,NULL);

/* We don't free 'parameters->tags' because it was just a link on 'parameters->fst2->tags' */
free_alphabet(p->alphabet);
if (p->jamo!=NULL) {
	delete p->jamo;
}
if (p->jamo_tags!=NULL) {
	/* jamo tags must be freed before tokens, because we need to know how
	 * many jamo tags there are, and this number is the number of tokens */
	for (int i=0;i<p->tokens->size;i++) {
		free(p->jamo_tags[i]);
	}
	free(p->jamo_tags);
}
if (p->jamo2syl!=NULL) {
   delete p->jamo2syl;
}
free_string_hash(p->tokens);
free_list_int(p->tag_token_list);
free_lemma_node(root);
free(p->token_control);
for (int i=0;i<n_text_tokens;i++) {
   free_bit_array(p->matching_patterns[i]);
}
free(p->matching_patterns);
#ifdef TRE_WCHAR
free_FilterSet(p->filters);
free_FilterMatchIndex(p->filter_match_index);
#endif
for (int i=0;i<p->n_morpho_dics;i++) {
   free_abstract_INF(p->morpho_dic_inf[i],&(p->morpho_dic_inf_free[i]));
   free_abstract_BIN(p->morpho_dic_bin[i],&(p->morpho_dic_bin_free[i]));
}
free(p->morpho_dic_inf);
free(p->morpho_dic_inf_free);
free(p->morpho_dic_bin);
free(p->morpho_dic_bin_free);
#if (defined(UNITEX_LIBRARY) || defined(UNITEX_RELEASE_MEMORY_AT_EXIT))
free_DLC_tree(p->DLC_tree);
#endif
free_locate_parameters(p);
u_printf("Done.\n");
return 1;
}


int count_semi_colons(char* s) {
int n=0;
for (int i=0;s[i]!='\0';i++) {
   if (s[i]==';') n++;
}
return n;
}


/**
 * Takes a string containing .bin names separated with semi-colons and
 * loads the corresponding dictionaries.
 */
void load_morphological_dictionaries(char* morpho_dic_list,struct locate_parameters* p) {
if (morpho_dic_list==NULL || morpho_dic_list[0]=='\0') {
   return;
}
p->n_morpho_dics=1+count_semi_colons(morpho_dic_list);
p->morpho_dic_bin=(unsigned char**)malloc(p->n_morpho_dics*sizeof(unsigned char*));
p->morpho_dic_bin_free=(struct BIN_free_info*)malloc(p->n_morpho_dics*sizeof(struct BIN_free_info));
p->morpho_dic_inf=(struct INF_codes**)malloc(p->n_morpho_dics*sizeof(struct INF_codes*));
p->morpho_dic_inf_free=(struct INF_free_info*)malloc(p->n_morpho_dics*sizeof(struct INF_free_info));
if (p->morpho_dic_bin==NULL || p->morpho_dic_inf==NULL || p->morpho_dic_bin_free==NULL || p->morpho_dic_inf_free==NULL) {
   fatal_alloc_error("load_morphological_dictionaries");
}
char bin[FILENAME_MAX];
int pos;
for (int i=0;i<p->n_morpho_dics;i++) {
   pos=0;
   while (*morpho_dic_list!='\0' && *morpho_dic_list!=';') {
      bin[pos++]=*morpho_dic_list;
      morpho_dic_list++;
   }
   bin[pos]='\0';
   if (*morpho_dic_list==';') {
      morpho_dic_list++;
   }
   p->morpho_dic_bin[i]=load_abstract_BIN_file(bin,&(p->morpho_dic_bin_free[i]));
   p->morpho_dic_inf[i]=NULL;
   if (p->morpho_dic_bin[i]!=NULL) {
      char inf[FILENAME_MAX];
      remove_extension(bin,inf);
      strcat(inf,".inf");
      p->morpho_dic_inf[i]=load_abstract_INF_file(inf,&(p->morpho_dic_inf_free[i]));
      if (p->morpho_dic_inf[i]==NULL) {
         free_abstract_BIN(p->morpho_dic_bin[i],&(p->morpho_dic_bin_free[i]));
         p->morpho_dic_bin[i]=NULL;
      }
   }
}
}



/**
 * Returns a control byte that represents the characteristics of the given token.
 */
unsigned char get_control_byte(unichar* token,Alphabet* alph,struct string_hash* err,TokenizationPolicy tokenization_policy) {
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
    * in order to answer the question <!DIC>. We perform this test in order
    * to avoid taking "priori" as an unknown word if the compound "a priori"
    * is in the text. */
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
if (token[0]=='{' && u_strcmp(token,"{S}") && u_strcmp(token,"{STOP}")) {
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
   if (!is_a_simple_word(temp->inflected,tokenization_policy,alph)) {
      /* If the tag is a compound word, we say that it verifies the <CDIC> pattern */
      set_bit_mask(&c,CDIC_TOKEN_BIT_MASK);
   }
   free_dela_entry(temp);
}
return c;
}


/**
 * For each token of the text, we compute its associated control byte.
 * We use the unknown word file 'err' in order to determine if a token
 * must be matched by <!DIC>
 */
void compute_token_controls(Alphabet* alph,char* err,struct locate_parameters* p) {
struct string_hash* ERR=load_key_list(err,p->mask_encoding_compatibility_input);
int n=p->tokens->size;
for (int i=0;i<n;i++) {
   p->token_control[i]=get_control_byte(p->tokens->value[i],alph,ERR,p->tokenization_policy);
}
free_string_hash(ERR);
}


/**
 * This function loads a DLF or a DLC. It computes information about tokens
 * that will be used during the Locate operation. For instance, if we have the
 * following line:
 *
 *   extended,.A
 *
 * and if the .fst2 to be applied to the text contains the pattern <A> with,
 * number 456, then the function will mark the "extended" token to be matched
 * by the pattern 456. Moreover, all case variations will be taken into account,
 * so that the "Extended" and "EXTENDED" tokens will also be updated.
 *
 * The two parameters 'is_DIC_pattern' and 'is_CDIC_pattern'
 * indicate if the .fst2 contains the corresponding patterns. For instance, if
 * the pattern "<CDIC>" is used in the grammar, it means that any token sequence that is a
 * compound word must be marked as be matched by this pattern.
 */
void load_dic_for_locate(char* dic_name,int mask_encoding_compatibility_input,Alphabet* alphabet,
                         int number_of_patterns,int is_DIC_pattern,
                         int is_CDIC_pattern,
                         struct lemma_node* root,struct locate_parameters* parameters) {
struct string_hash* tokens=parameters->tokens;
U_FILE* f;
unichar line[DIC_LINE_SIZE];
f=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,dic_name,U_READ);
if (f==NULL) {
   error("Cannot open dictionary %s\n",dic_name);
   return;
}
/* We parse all the lines */
int lines=0;
char name[FILENAME_MAX];
remove_path(dic_name,name);
while (EOF!=u_fgets(line,f)) {
   lines++;
   if (lines%10000==0) {
      u_printf("%s: %d lines loaded...                          \r",name,lines);
   }
   if (line[0]=='/') {
      /* NOTE: DLF and DLC files are not supposed to contain comment
       *       lines, but we test them, just in the case */
      continue;
   }
   struct dela_entry* entry=tokenize_DELAF_line(line,1);
   if (entry==NULL) {
      /* This case should never happen */
      error("Invalid dictionary line in load_dic_for_locate\n");
      continue;
   }
   /* We add the inflected form to the list of forms associated to the lemma.
    * This will be used to replace patterns like "<be>" by the actual list of
    * forms that can be matched by it, for optimization reasons */
   add_inflected_form_for_lemma(entry->inflected,entry->lemma,root);
   /* We get the list of all tokens that can be matched by the inflected form of this
    * this entry, with regards to case variations (see the "extended" example above). */
   struct list_int* ptr=get_token_list_for_sequence(entry->inflected,alphabet,tokens);
   /* We save the list pointer to free it later */
   struct list_int* ptr_copy=ptr;
   /* Here, we will deal with all simple words */
   while (ptr!=NULL) {
      int i=ptr->n;
      /* If the current token can be matched, then it can be recognized by the "<DIC>" pattern */
      parameters->token_control[i]=(unsigned char)(get_control_byte(tokens->value[i],alphabet,NULL,parameters->tokenization_policy)|DIC_TOKEN_BIT_MASK);
      if (number_of_patterns) {
         /* We look for matching patterns only if there are some */
         struct list_pointer* list=get_matching_patterns(entry,parameters->pattern_tree_root);
         if (list!=NULL) {
            /* If we have some patterns to add */
            if (parameters->matching_patterns[i]==NULL) {
               /* We allocate the pattern bit array, if needed */
               parameters->matching_patterns[i]=new_bit_array(number_of_patterns,ONE_BIT);
            }
            struct list_pointer* tmp=list;
            while (tmp!=NULL) {
               /* Then we add all the pattern numbers to the bit array */
               set_value(parameters->matching_patterns[i],((struct constraint_list*)(tmp->pointer))->pattern_number,1);
               tmp=tmp->next;
            }
            /* Finally, we free the constraint list */
            free_list_pointer(list);
         }
      }
      ptr=ptr->next;
   }
   /* Finally, we free the token list */
   free_list_int(ptr_copy);
   if (!is_a_simple_word(entry->inflected,parameters->tokenization_policy,alphabet)) {
      /* If the inflected form is a compound word */
      if (is_DIC_pattern || is_CDIC_pattern) {
         /* If the .fst2 contains "<DIC>" and/or "<CDIC>", then we
          * must note that all compound words can be matched by them */
         add_compound_word_with_no_pattern(entry->inflected,alphabet,tokens,parameters->DLC_tree,parameters->tokenization_policy);
      }
      if (number_of_patterns) {
         /* We look for matching patterns only if there are some */
         /* We look if the compound word can be matched by some patterns */
         struct list_pointer* list=get_matching_patterns(entry,parameters->pattern_tree_root);
         struct list_pointer* tmp=list;
         while (tmp!=NULL) {
            /* If the word is matched by at least one pattern, we store it. */
            int pattern_number=((struct constraint_list*)(tmp->pointer))->pattern_number;
            add_compound_word_with_pattern(entry->inflected,pattern_number,alphabet,tokens,parameters->DLC_tree,parameters->tokenization_policy);
            tmp=tmp->next;
         }
         free_list_pointer(list);
      }
   }
   free_dela_entry(entry);
}
if (lines>10000) {
   u_printf("\n");
}
u_fclose(f);
}


/**
 * This function checks for each tag token like "{extended,extend.V:K}"
 * if it verifies some patterns. Its behaviour is very similar to the one
 * of the load_dic_for_locate function. However, as a side effect, this
 * function fills 'tag_token_list' with the list of tag token numbers.
 * This list is later used during Locate preprocessings.
 */
void check_patterns_for_tag_tokens(Alphabet* alphabet,int number_of_patterns,
                           struct lemma_node* root,struct locate_parameters* parameters) {
struct string_hash* tokens=parameters->tokens;
for (int i=0;i<tokens->size;i++) {
   if (tokens->value[i][0]=='{' && u_strcmp(tokens->value[i],"{S}")  && u_strcmp(tokens->value[i],"{STOP}")) {
      /* If the token is tag like "{today,.ADV}", we add its number to the tag token list */
      parameters->tag_token_list=head_insert(i,parameters->tag_token_list);
      /* And we look for the patterns that can match it */
      struct dela_entry* entry=tokenize_tag_token(tokens->value[i]);
      if (entry==NULL) {
         /* This should never happen */
         fatal_error("Invalid tag token in function check_patterns_for_tag_tokens\n");
      }
      /* We add the inflected form to the list of forms associated to the lemma.
      * This will be used to replace patterns like "<be>" by the actual list of
      * forms that can be matched by it, for optimization reasons */
      add_inflected_form_for_lemma(tokens->value[i],entry->lemma,root);
      parameters->token_control[i]=(unsigned char)(get_control_byte(tokens->value[i],alphabet,NULL,parameters->tokenization_policy)|DIC_TOKEN_BIT_MASK);
      if (number_of_patterns) {
         /* We look for matching patterns only if there are some */
         struct list_pointer* list=get_matching_patterns(entry,parameters->pattern_tree_root);
         if (list!=NULL) {
            if (parameters->matching_patterns[i]==NULL) {
               /* We allocate the bit array if needed */
               parameters->matching_patterns[i]=new_bit_array(number_of_patterns,ONE_BIT);
            }
            struct list_pointer* tmp=list;
            while (tmp!=NULL) {
               set_value(parameters->matching_patterns[i],((struct constraint_list*)(tmp->pointer))->pattern_number,1);
               tmp=tmp->next;
            }
            free_list_pointer(list);
         }
      }
      /* At the opposite of DLC lines, a compound word tag like "{all around,.ADV}"
       * does not need to be put in the compound word tree, since the tag is already
       * characterized by its token number. */
      free_dela_entry(entry);
   }
}
}

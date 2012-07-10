/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "UserCancelling.h"
#include "LocateTrace.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

void load_dic_for_locate(const char*, const VersatileEncodingConfig*,Alphabet*,int,int,int,struct lemma_node*,struct locate_parameters*);
void check_patterns_for_tag_tokens(Alphabet*,int,struct lemma_node*,struct locate_parameters*,Abstract_allocator);
void load_morphological_dictionaries(const VersatileEncodingConfig*,const char* morpho_dic_list,struct locate_parameters* p);
void load_morphological_dictionaries(const VersatileEncodingConfig*,const char* morpho_dic_list,struct locate_parameters* p,const char* local_morpho_dic);


/**
 * Allocates, initializes and returns a new locate_parameters structure.
 */
struct locate_parameters* new_locate_parameters() {
struct locate_parameters* p=(struct locate_parameters*)malloc(sizeof(struct locate_parameters));
if (p==NULL) {
   fatal_alloc_error("new_locate_parameters");
}
p->tilde_negation_operator=1;
p->useLocateCache=1;
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
p->max_count_call=0;
p->max_count_call_warning=0;
p->buffer=NULL;
p->tokenization_policy=WORD_BY_WORD_TOKENIZATION;
p->space_policy=DONT_START_WITH_SPACE;
p->matching_units=0;
p->match_policy=LONGEST_MATCHES;
p->real_output_policy=IGNORE_OUTPUTS;
p->output_policy=IGNORE_OUTPUTS;
p->ambiguous_output_policy=ALLOW_AMBIGUOUS_OUTPUTS;
p->variable_error_policy=IGNORE_VARIABLE_ERRORS;
p->match_list=NULL;
p->number_of_matches=0;
p->number_of_outputs=0;
p->start_position_last_printed_match=-1;
p->end_position_last_printed_match=-1;
p->search_limit=0;
p->input_variables=NULL;
p->output_variables=NULL;
p->nb_output_variables=0;
p->stack=new_stack_unichar(TRANSDUCTION_STACK_SIZE);
p->alphabet=NULL;
p->morpho_dic=NULL;
p->morpho_dic_inf_free=NULL;
p->morpho_dic_bin_free=NULL;
p->n_morpho_dics=0;
p->dic_variables=NULL;
p->left_ctx_shift=0;
p->left_ctx_base=0;
p->protect_dic_chars=0;
p->graph_depth=0;
p->korean=NULL;
p->jamo_tags=NULL;
p->recyclable_wchart_buffer=(unichar_regex*)malloc(sizeof(unichar_regex)*SIZE_RECYCLABLE_WCHAR_T_BUFFER);
if (p->recyclable_wchart_buffer==NULL) {
   fatal_alloc_error("new_locate_parameters");
}
p->recyclable_unichar_buffer=(unichar*)malloc(sizeof(unichar)*SIZE_RECYCLABLE_UNICHAR_BUFFER);
if (p->recyclable_unichar_buffer==NULL) {
   fatal_alloc_error("new_locate_parameters");
}
p->size_recyclable_unichar_buffer = SIZE_RECYCLABLE_UNICHAR_BUFFER;
p->failfast=NULL;
p->match_cache_first=NULL;
p->match_cache_last=NULL;
p->match_cache=NULL;
p->prv_alloc=NULL;
p->prv_alloc_recycle=NULL;
p->token_error_ctx.last_length=0;
p->token_error_ctx.last_start=0;
p->token_error_ctx.n_errors=0;
p->token_error_ctx.n_matches_at_token_pos__locate=0;
p->token_error_ctx.n_matches_at_token_pos__morphological_locate=0;
p->counting_step.count_call=0;
p->counting_step.count_cancel_trying=0;
p->explore_depth=0;
p->backup_memory_reserve=NULL;
p->cached_match_vector=new_vector_ptr(16);
p->fnc_locate_trace_step=NULL;
p->private_param_locate_trace=NULL;
memset(&(p->arabic),0,sizeof(ArabicTypoRules));
p->is_in_cancel_state = 0;
p->is_in_trace_state = 0;
p->counting_step_count_cancel_trying_real_in_debug_or_trace = 0;
p->debug=0;
p->weight=-1;
return p;
}


/**
 * Frees a locate_parameters structure.
 */
void free_locate_parameters(struct locate_parameters* p) {
if (p==NULL) return;
if (p->recyclable_wchart_buffer!=NULL) {
    free(p->recyclable_wchart_buffer);
}
if (p->recyclable_unichar_buffer!=NULL) {
    free(p->recyclable_unichar_buffer);
}
free_vector_ptr(p->cached_match_vector,NULL);
free(p);
}


/**
 * Returns an array containing the jamo versions of all the given tokens.
 */
unichar** create_jamo_tags(Korean* korean,struct string_hash* tokens) {
unichar** res=(unichar**)malloc(tokens->size*sizeof(unichar*));
unichar foo[1024];
for (int i=0;i<tokens->size;i++) {
	if (!u_strcmp(tokens->value[i],"{S}")) {
		res[i]=u_strdup("{S}");
	} else {
	   Hanguls_to_Jamos(tokens->value[i],foo,korean,0);
	   res[i]=u_strdup(foo);
	   /*error("<%S> (%x) => \n",tokens->value[i],tokens->value[i][0]);
	   for (int j=0;foo[j]!=0;j++) {
		   error("[ %C %x] ",foo[j],foo[j]);
	   }
	   error("\n");*/
	}
}
return res;
}



int locate_pattern(const char* text_cod,const char* tokens,const char* fst2_name,const char* dlf,const char* dlc,const char* err,
                   const char* alphabet,MatchPolicy match_policy,OutputPolicy output_policy,
                   const VersatileEncodingConfig* vec,
                   const char* dynamicDir,TokenizationPolicy tokenization_policy,
                   SpacePolicy space_policy,int search_limit,const char* morpho_dic_list,
                   AmbiguousOutputPolicy ambiguous_output_policy,
                   VariableErrorPolicy variable_error_policy,int protect_dic_chars,
                   int is_korean,int max_count_call,int max_count_call_warning,
                   char* arabic_rules,int tilde_negation_operator,int useLocateCache,int allow_trace,
                   vector_ptr* injected_vars) {

U_FILE* out;
U_FILE* info;
struct locate_parameters* p=new_locate_parameters();
p->text_cod=af_open_mapfile(text_cod,MAPFILE_OPTION_READ,0);
p->buffer=(int*)af_get_mapfile_pointer(p->text_cod);
long text_size=(long)af_get_mapfile_size(p->text_cod)/sizeof(int);
p->buffer_size=(int)text_size;
p->tilde_negation_operator=tilde_negation_operator;
p->useLocateCache=useLocateCache;
if (max_count_call == -1) {
   max_count_call = (int)text_size;
}
if (max_count_call_warning == -1) {
   max_count_call_warning = (int)text_size;
}
p->match_policy=match_policy;
p->tokenization_policy=tokenization_policy;
p->space_policy=space_policy;
p->real_output_policy=p->output_policy=output_policy;
p->search_limit=search_limit;
p->ambiguous_output_policy=ambiguous_output_policy;
p->variable_error_policy=variable_error_policy;
p->protect_dic_chars=protect_dic_chars;
p->max_count_call = max_count_call;
p->max_count_call_warning = max_count_call_warning;
p->token_filename = tokens;
char concord[FILENAME_MAX];
char concord_info[FILENAME_MAX];

strcpy(concord,dynamicDir);
strcat(concord,"concord.ind");

strcpy(concord_info,dynamicDir);
strcat(concord_info,"concord.n");

char morpho_bin[FILENAME_MAX];
strcpy(morpho_bin,dynamicDir);
strcat(morpho_bin,"morpho.bin");
if (arabic_rules!=NULL && arabic_rules[0]!='\0') {
	load_arabic_typo_rules(vec,arabic_rules,&(p->arabic));
}
out=u_fopen(vec,concord,U_WRITE);
if (out==NULL) {
   error("Cannot write %s\n",concord);
   af_release_mapfile_pointer(p->text_cod,p->buffer);
   af_close_mapfile(p->text_cod);
   free_stack_unichar(p->stack);
   free_locate_parameters(p);
   u_fclose(out);
   return 0;
}
info=u_fopen(vec,concord_info,U_WRITE);
if (info==NULL) {
   error("Cannot write %s\n",concord_info);
}
if (alphabet!=NULL && alphabet[0]!='\0') {
   u_printf("Loading alphabet...\n");
   p->alphabet=load_alphabet(vec,alphabet,is_korean);
   if (p->alphabet==NULL) {
      error("Cannot load alphabet file %s\n",alphabet);
      af_release_mapfile_pointer(p->text_cod,p->buffer);
      af_close_mapfile(p->text_cod);
      free_stack_unichar(p->stack);
      free_locate_parameters(p);
      if (info!=NULL) u_fclose(info);
      u_fclose(out);
      return 0;
   }
}
struct string_hash* semantic_codes=new_string_hash();
extract_semantic_codes(vec,dlf,semantic_codes);
extract_semantic_codes(vec,dlc,semantic_codes);

if (is_cancelling_requested() != 0) {
	   error("user cancel request.\n");
	   free_alphabet(p->alphabet);
	   free_string_hash(semantic_codes);
       af_release_mapfile_pointer(p->text_cod,p->buffer);
       af_close_mapfile(p->text_cod);
       free_stack_unichar(p->stack);
       free_locate_parameters(p);
       if (info!=NULL) u_fclose(info);
       u_fclose(out);
	   return 0;
	}

u_printf("Loading fst2...\n");
struct FST2_free_info fst2load_free;
Fst2* fst2load=load_abstract_fst2(vec,fst2_name,1,&fst2load_free);
if (fst2load==NULL) {
   error("Cannot load grammar %s\n",fst2_name);
   free_alphabet(p->alphabet);
   free_string_hash(semantic_codes);
   af_release_mapfile_pointer(p->text_cod,p->buffer);
   af_close_mapfile(p->text_cod);
   free_stack_unichar(p->stack);
   free_locate_parameters(p);
   if (info!=NULL) u_fclose(info);
   u_fclose(out);
   return 0;
}
if (fst2load->debug) {
	/* If Locate uses a debug fst2, we force the output mode to MERGE,
	 * we allow ambiguous outputs and we write graph names into the
	 * concordance file */
	p->output_policy=MERGE_OUTPUTS;
	p->ambiguous_output_policy=ALLOW_AMBIGUOUS_OUTPUTS;
	p->debug=1;
	u_fprintf(out,"#D\n");
	u_fprintf(out,"%d\n",fst2load->number_of_graphs);
	for (int i=0;i<fst2load->number_of_graphs;i++) {
		u_fprintf(out,"%S\n",fst2load->graph_names[i+1]);
	}
}
switch(p->real_output_policy) {
   case IGNORE_OUTPUTS: u_fprintf(out,"#I\n"); break;
   case MERGE_OUTPUTS: u_fprintf(out,"#M\n"); break;
   case REPLACE_OUTPUTS: u_fprintf(out,"#R\n"); break;
   default:break;
}

Abstract_allocator locate_abstract_allocator=create_abstract_allocator("locate_pattern",AllocatorCreationFlagAutoFreePrefered);


p->fst2=new_Fst2_clone(fst2load,locate_abstract_allocator);
free_abstract_Fst2(fst2load,&fst2load_free);

if (is_cancelling_requested() != 0) {
   error("User cancel request..\n");
   free_alphabet(p->alphabet);
   free_string_hash(semantic_codes);
   free_Fst2(p->fst2,locate_abstract_allocator);
   close_abstract_allocator(locate_abstract_allocator);
   af_release_mapfile_pointer(p->text_cod,p->buffer);
   af_close_mapfile(p->text_cod);
   free_stack_unichar(p->stack);
   free_locate_parameters(p);
   if (info!=NULL) u_fclose(info);
   u_fclose(out);
   return 0;
}
	
p->tags=p->fst2->tags;
#ifdef TRE_WCHAR
p->filters=new_FilterSet(p->fst2,p->alphabet);
if (p->filters==NULL) {
   error("Cannot compile filter(s)\n");
   free_alphabet(p->alphabet);
   free_string_hash(semantic_codes);
   free_Fst2(p->fst2,locate_abstract_allocator);
   close_abstract_allocator(locate_abstract_allocator);
   free_stack_unichar(p->stack);
   free_locate_parameters(p);
   af_release_mapfile_pointer(p->text_cod,p->buffer);
   af_close_mapfile(p->text_cod);
   if (info!=NULL) u_fclose(info);
   u_fclose(out);
   return 0;
}
#endif
u_printf("Loading token list...\n");
int n_text_tokens=0;

p->tokens=load_text_tokens_hash(tokens,vec,&(p->SENTENCE),&(p->STOP),&n_text_tokens);
if (p->tokens==NULL) {
   error("Cannot load token list %s\n",tokens);
   free_alphabet(p->alphabet);
   free_string_hash(semantic_codes);
   free_Fst2(p->fst2,locate_abstract_allocator);
   close_abstract_allocator(locate_abstract_allocator);
   free_locate_parameters(p);
   af_release_mapfile_pointer(p->text_cod,p->buffer);
   af_close_mapfile(p->text_cod);
   if (info!=NULL) u_fclose(info);
   u_fclose(out);
   return 0;
}
Abstract_allocator locate_work_abstract_allocator = locate_abstract_allocator;

p->match_cache=(LocateCache*)malloc_cb(p->tokens->size * sizeof(LocateCache),locate_work_abstract_allocator);
memset(p->match_cache,0,p->tokens->size * sizeof(LocateCache));
if (p->match_cache==NULL) {
	fatal_alloc_error("locate_pattern");
}

#ifdef TRE_WCHAR
p->filter_match_index=new_FilterMatchIndex(p->filters,p->tokens);
if (p->filter_match_index==NULL) {
   error("Cannot optimize filter(s)\n");
   free_alphabet(p->alphabet);
   free_string_hash(semantic_codes);
   free_string_hash(p->tokens);
   close_abstract_allocator(locate_abstract_allocator);
   free_locate_parameters(p);
   af_release_mapfile_pointer(p->text_cod,p->buffer);
   af_close_mapfile(p->text_cod);
   if (info!=NULL) u_fclose(info);
   u_fclose(out);
   return 0;
}
#endif

if (allow_trace!=0) {
   open_locate_trace(p,&p->fnc_locate_trace_step,&p->private_param_locate_trace);
}
extract_semantic_codes_from_tokens(p->tokens,semantic_codes,locate_abstract_allocator);
u_printf("Loading morphological dictionaries...\n");
load_morphological_dictionaries(vec,morpho_dic_list,p,morpho_bin);
extract_semantic_codes_from_morpho_dics(p->morpho_dic,p->n_morpho_dics,semantic_codes,locate_abstract_allocator);
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
compute_token_controls(vec,p->alphabet,err,p);
int number_of_patterns,is_DIC,is_CDIC,is_SDIC;
p->pattern_tree_root=new_pattern_node(locate_abstract_allocator);
u_printf("Computing fst2 tags...\n");
process_tags(&number_of_patterns,semantic_codes,&is_DIC,&is_CDIC,&is_SDIC,p,locate_abstract_allocator);
p->current_compound_pattern=number_of_patterns;
p->DLC_tree=new_DLC_tree(p->tokens->size);
struct lemma_node* root=new_lemma_node();
u_printf("Loading dlf...\n");
load_dic_for_locate(dlf,vec,p->alphabet,number_of_patterns,is_DIC,is_CDIC,root,p);
u_printf("Loading dlc...\n");
load_dic_for_locate(dlc,vec,p->alphabet,number_of_patterns,is_DIC,is_CDIC,root,p);
/* We look if tag tokens like "{today,.ADV}" verify some patterns */
check_patterns_for_tag_tokens(p->alphabet,number_of_patterns,root,p,locate_abstract_allocator);
u_printf("Optimizing fst2 pattern tags...\n");
optimize_pattern_tags(p->alphabet,root,p,locate_abstract_allocator);
u_printf("Optimizing compound word dictionary...\n");
optimize_DLC(p->DLC_tree);
free_string_hash(semantic_codes);
int nb_input_variable=0;
p->input_variables=new_Variables(p->fst2->input_variables,&nb_input_variable);
p->output_variables=new_OutputVariables(p->fst2->output_variables,&p->nb_output_variables,injected_vars);

Abstract_allocator locate_recycle_abstract_allocator=NULL;
locate_recycle_abstract_allocator=create_abstract_allocator("locate_pattern_recycle",
                                 AllocatorFreeOnlyAtAllocatorDelete|AllocatorTipOftenRecycledObject,
                                 get_prefered_allocator_item_size_for_nb_variable(nb_input_variable));

u_printf("Optimizing fst2...\n");
p->optimized_states=build_optimized_fst2_states(p->input_variables,p->output_variables,p->fst2,locate_abstract_allocator);
if (is_korean) {
	p->korean=new Korean(p->alphabet);
	p->jamo_tags=create_jamo_tags(p->korean,p->tokens);
}
p->failfast=new_bit_array(n_text_tokens,ONE_BIT);

u_printf("Working...\n");
p->prv_alloc=locate_work_abstract_allocator;
p->prv_alloc_recycle=locate_recycle_abstract_allocator;
launch_locate(out,text_size,info,p);
if (allow_trace!=0) {
   close_locate_trace(p,p->fnc_locate_trace_step,p->private_param_locate_trace);
}
free_bit_array(p->failfast);
free_Variables(p->input_variables);
free_OutputVariables(p->output_variables);
af_release_mapfile_pointer(p->text_cod,p->buffer);
af_close_mapfile(p->text_cod);
if (info!=NULL) u_fclose(info);
u_fclose(out);

if (p->match_cache!=NULL) {
	for (int i=0;i<p->tokens->size;i++) {
		free_LocateCache(p->match_cache[i],locate_work_abstract_allocator);
	}
	free_cb(p->match_cache,locate_work_abstract_allocator);
}
int free_abstract_allocator_item=(get_allocator_cb_flag(locate_abstract_allocator) & AllocatorGetFlagAutoFreePresent) ? 0 : 1;

if (free_abstract_allocator_item) {
  free_optimized_states(p->optimized_states,p->fst2->number_of_states,locate_abstract_allocator);
}
free_stack_unichar(p->stack);
/** Too long to free the DLC tree if it is big
 * free_DLC_tree(p->DLC_tree);
 */
if (free_abstract_allocator_item) {
  free_pattern_node(p->pattern_tree_root,locate_abstract_allocator);
  free_Fst2(p->fst2,locate_abstract_allocator);
  free_list_int(p->tag_token_list,locate_abstract_allocator);
}
close_abstract_allocator(locate_abstract_allocator);
close_abstract_allocator(locate_recycle_abstract_allocator);
locate_recycle_abstract_allocator=locate_abstract_allocator=NULL;

/* We don't free 'parameters->tags' because it was just a link on 'parameters->fst2->tags' */
free_alphabet(p->alphabet);
if (p->korean!=NULL) {
	delete p->korean;
}
if (p->jamo_tags!=NULL) {
	/* jamo tags must be freed before tokens, because we need to know how
	 * many jamo tags there are, and this number is the number of tokens */
	for (int i=0;i<p->tokens->size;i++) {
		free(p->jamo_tags[i]);
	}
	free(p->jamo_tags);
}
free_string_hash(p->tokens);
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
	free_Dictionary(p->morpho_dic[i]);
   /*free_abstract_INF(p->morpho_dic_inf[i],&(p->morpho_dic_inf_free[i]));
   free_abstract_BIN(p->morpho_dic_bin[i],&(p->morpho_dic_bin_free[i]));*/
}
free(p->morpho_dic);
free(p->morpho_dic_inf_free);
free(p->morpho_dic_bin_free);
#if (defined(UNITEX_LIBRARY) || defined(UNITEX_RELEASE_MEMORY_AT_EXIT))
free_DLC_tree(p->DLC_tree);
#endif
free_locate_parameters(p);
u_printf("Done.\n");
return 1;
}


int count_semi_colons(const char* s) {
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
void load_morphological_dictionaries(const VersatileEncodingConfig* vec,const char* morpho_dic_list,struct locate_parameters* p) {
if (morpho_dic_list==NULL || morpho_dic_list[0]=='\0') {
   return;
}
p->n_morpho_dics=1+count_semi_colons(morpho_dic_list);
p->morpho_dic=(Dictionary**)malloc(p->n_morpho_dics*sizeof(Dictionary*));
p->morpho_dic_bin_free=(struct BIN_free_info*)malloc(p->n_morpho_dics*sizeof(struct BIN_free_info));
p->morpho_dic_inf_free=(struct INF_free_info*)malloc(p->n_morpho_dics*sizeof(struct INF_free_info));
if (p->morpho_dic==NULL || p->morpho_dic_bin_free==NULL || p->morpho_dic_inf_free==NULL) {
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
   char inf[FILENAME_MAX];
   remove_extension(bin,inf);
   strcat(inf,".inf");
   p->morpho_dic[i]=new_Dictionary(vec,bin,inf);
}
}


/**
 * Takes a string containing .bin names separated with semi-colons and
 * loads the corresponding dictionaries.
 */
void load_morphological_dictionaries(const VersatileEncodingConfig* vec,const char* morpho_dic_list,struct locate_parameters* p,
                                     const char* local_morpho_dic) {
if (fexists(local_morpho_dic)) {
   if (morpho_dic_list!=NULL && morpho_dic_list[0]!='\0') {
      /* If we have both local and non-local dictionaries */
      char* temp;
      /* +2 because we have a ';' to insert */
      temp=(char*)malloc(strlen(local_morpho_dic)+strlen(morpho_dic_list)+2);
      if (temp==NULL) {
         fatal_alloc_error("load_morphological_dictionaries");
      }
      sprintf(temp,"%s;%s",local_morpho_dic,morpho_dic_list);
      load_morphological_dictionaries(vec,temp,p);
      free(temp);
      return;
   } else {
      /* We just have the local one */
      return load_morphological_dictionaries(vec,local_morpho_dic,p);
   }
}
/* We have no local dictionary*/
load_morphological_dictionaries(vec,morpho_dic_list,p);
}



/**
 * Returns a control byte that represents the characteristics of the given token.
 */
unsigned char get_control_byte(const unichar* token,const Alphabet* alph,struct string_hash* err,TokenizationPolicy tokenization_policy) {
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
         if (!is_upper(token[i],alph)) {
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
      if (!is_lower(token[i],alph)) {
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
   set_bit_mask(&c,MOT_TOKEN_BIT_MASK|DIC_TOKEN_BIT_MASK|TDIC_TOKEN_BIT_MASK);
   struct dela_entry* temp=tokenize_tag_token(token,1);
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
void compute_token_controls(const VersatileEncodingConfig* vec,Alphabet* alph,const char* err,struct locate_parameters* p) {
struct string_hash* ERR=load_key_list(vec,err);
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
void load_dic_for_locate(const char* dic_name, const VersatileEncodingConfig* vec,Alphabet* alphabet,
                         int number_of_patterns,int is_DIC_pattern,
                         int is_CDIC_pattern,
                         struct lemma_node* root,struct locate_parameters* parameters) {
struct string_hash* tokens=parameters->tokens;
U_FILE* f;
f=u_fopen(vec,dic_name,U_READ);
if (f==NULL) {
   return;
}
/* We parse all the lines */
int lines=0;
char name[FILENAME_MAX];
remove_path(dic_name,name);
Ustring* line=new_Ustring(DIC_LINE_SIZE);
while (EOF!=readline(line,f)) {
   lines++;
   if (lines%10000==0) {
      u_printf("%s: %d lines loaded...                          \r",name,lines);
   }
   if (line->str[0]=='/') {
      /* NOTE: DLF and DLC files are not supposed to contain comment
       *       lines, but we test them, just in the case */
      continue;
   }
   struct dela_entry* entry=tokenize_DELAF_line(line->str,1);
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
free_Ustring(line);
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
                           struct lemma_node* root,struct locate_parameters* parameters,Abstract_allocator prv_alloc) {
struct string_hash* tokens=parameters->tokens;
for (int i=0;i<tokens->size;i++) {
   if (tokens->value[i][0]=='{' && u_strcmp(tokens->value[i],"{S}")  && u_strcmp(tokens->value[i],"{STOP}")) {
      /* If the token is tag like "{today,.ADV}", we add its number to the tag token list */
      parameters->tag_token_list=head_insert(i,parameters->tag_token_list,prv_alloc);
      /* And we look for the patterns that can match it */
      struct dela_entry* entry=tokenize_tag_token(tokens->value[i],1);
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

} // namespace unitex


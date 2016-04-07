/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "UnusedParameter.h"
#include "Fst2.h"
#include "AbstractFst2Load.h"
#include "DELA.h"
#include "Pattern.h"
#include "LocateTfst_lib.h"
#include "File.h"
#include "Korean.h"
#include "Contexts.h"
#include "List_int.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/* see http://en.wikipedia.org/wiki/Variable_Length_Array . MSVC did not support it 
   see http://msdn.microsoft.com/en-us/library/zb1574zs(VS.80).aspx */
#if defined(_MSC_VER) && (!(defined(NO_C99_VARIABLE_LENGTH_ARRAY)))
#define NO_C99_VARIABLE_LENGTH_ARRAY 1
#endif


/* During a locate operation starting on a given tfst state, we consider that
 * more than MAX_VISITS_PER_TFST_STATE visits in a state means that there is
 * a combinatorial explosion */
#define MAX_VISITS_PER_TFST_STATE 128


void explore_tfst(int* visits,Tfst* tfst,int current_state_in_tfst,
		          int current_state_in_fst2,int graph_depth,
		          struct tfst_match* match_element_list,
                struct tfst_match_list** LIST,
                struct locate_tfst_infos* infos,
                int pos_pending_in_fst2_tag,
                int pos_pending_in_tfst_tag,
                Transition* current_pending_fst2_transition,
                Transition* current_pending_tfst_transition,
                struct list_context* ctx, int tilde_negation_operator);
void init_Korean_stuffs(struct locate_tfst_infos* infos,int is_korean);
void free_Korean_stuffs(struct locate_tfst_infos* infos);
void compute_jamo_tfst_tags(struct locate_tfst_infos* infos);
int match_between_text_and_grammar_tags(Tfst* tfst,TfstTag* text_tag,Fst2Tag grammar_tag,
                                        int tfst_tag_index,int fst2_tag_index,
                                        struct locate_tfst_infos* infos,
                                        int *pos_pending_fst2_tag,int *pos_pending_tfst_tag, int tilde_negation_operator);
int real_match_between_text_and_grammar_tags(Tfst* tfst,TfstTag* text_tag,Fst2Tag grammar_tag,
                                        int tfst_tag_index,int fst2_tag_index,
                                        struct locate_tfst_infos* infos,
                                        int *pos_pending_fst2_tag,int *pos_pending_tfst_tag, int tilde_negation_operator);
struct pattern* tokenize_grammar_tag(unichar* tag,int *negation,int tilde_negation_operator);
int is_space_on_the_left_in_tfst(Tfst* tfst,TfstTag* tag);
int morphological_filter_is_ok(const unichar* content,Fst2Tag grammar_tag,const struct locate_tfst_infos* infos);


/**
 * This function computes the debug output associated to the given partial
 * match, obtained within a context. As in a context, we don't care of
 * all possible paths (knowing that the context has matched is enough),
 * we compute only one output, even if there are several paths.
 */
Ustring* get_debug_output_in_context(struct tfst_match* match_element_list,struct locate_tfst_infos* infos) {
if (match_element_list==NULL) {
	return new_Ustring();
}
Ustring* s=get_debug_output_in_context(match_element_list->next,infos);
Transition* t=match_element_list->fst2_transition;
u_strcat(s,infos->fst2->tags[t->tag_number]->output);
struct list_int* l=match_element_list->text_tag_numbers;
if (l!=NULL) {
	TfstTag* tag=(TfstTag*)(infos->tfst->tags->tab[l->n]);
	if (tag->content[0]!='{' || tag->content[1]=='\0') {
		/* Untagged token ? */
		u_strcat(s,tag->content);
	} else {
		/* Real tag ? We only take the inflected form */
		struct dela_entry* d=tokenize_tag_token(tag->content,1);
		u_strcat(s,d->inflected);
		free_dela_entry(d);
	}
}
return s;
}


/**
 * This function applies the given grammar to the given text automaton.
 * It returns 1 in case of success; 0 otherwise.
 */
int locate_tfst(const char* text,const char* grammar,const char* alphabet,const char* output,
                const VersatileEncodingConfig* vec,
                MatchPolicy match_policy,
		          OutputPolicy output_policy,AmbiguousOutputPolicy ambiguous_output_policy,
		          VariableErrorPolicy variable_error_policy,int search_limit,int is_korean,
		          int tilde_negation_operator,vector_ptr* injected_vars,int tagging,
		          int single_tags_only,int match_word_boundaries) {
Tfst* tfst=open_text_automaton(vec,text);
if (tfst==NULL) {
	return 0;
}
struct FST2_free_info fst2_free;
struct locate_tfst_infos infos;
infos.fst2=load_abstract_fst2(vec,grammar,1,&fst2_free);
if (infos.fst2==NULL) {
	close_text_automaton(tfst);
	return 0;
}
infos.tagging=tagging;
infos.tfst=tfst;
infos.number_of_matches=0;
infos.alphabet=NULL;
infos.single_tags_only=single_tags_only;
infos.match_word_boundaries=match_word_boundaries;
if (alphabet!=NULL && alphabet[0]!='\0') {
   /* We want to allow undefined alphabets (see comments above 'is_letter'
    * in Alphabet.cpp) */
   infos.alphabet=load_alphabet(vec,alphabet,is_korean);
   if (infos.alphabet==NULL) {
	   close_text_automaton(tfst);
	   free_abstract_Fst2(infos.fst2,&fst2_free);
	   error("Cannot load alphabet file: %s\n",alphabet);
	   return 0;
   }
}
infos.output=u_fopen(vec,output,U_WRITE);
if (infos.output==NULL) {
	close_text_automaton(tfst);
	free_abstract_Fst2(infos.fst2,&fst2_free);
	free_alphabet(infos.alphabet);
	error("Cannot open %s\n",output);
	return 0;
}
infos.real_output_policy=output_policy;
infos.output_policy=output_policy;
infos.debug=0;
if (infos.fst2->debug) {
	/* If LocateTfst uses a debug fst2, we force the output mode to MERGE,
	 * we allow ambiguous outputs and we write graph names into the
	 * concordance file */
	if (tagging) {
		fatal_error("--tagging option cannot be used with a debug-compiled .fst2\n");
	}
	infos.output_policy=MERGE_OUTPUTS;
	infos.ambiguous_output_policy=ALLOW_AMBIGUOUS_OUTPUTS;
	infos.debug=1;
	u_fprintf(infos.output,"#D\n");
	u_fprintf(infos.output,"%d\n",infos.fst2->number_of_graphs);
	for (int i=0;i<infos.fst2->number_of_graphs;i++) {
		u_fprintf(infos.output,"%S\n",infos.fst2->graph_names[i+1]);
	}
}
if (tagging) {
	u_fprintf(infos.output,"#X\n");
} else {
	switch (infos.real_output_policy) {
	case IGNORE_OUTPUTS: u_fprintf(infos.output,"#I\n"); break;
	case MERGE_OUTPUTS: u_fprintf(infos.output,"#M\n"); break;
	case REPLACE_OUTPUTS: u_fprintf(infos.output,"#R\n"); break;
	default: break;
}
}
#ifdef REGEX_FACADE_ENGINE
infos.filters=new_FilterSet(infos.fst2,infos.alphabet);
infos.matches=NULL;
if (infos.filters==NULL) {
	close_text_automaton(tfst);
	free_abstract_Fst2(infos.fst2,&fst2_free);
	free_alphabet(infos.alphabet);
	u_fclose(infos.output);
    error("Cannot compile filter(s)\n");
   return 0;
}
#endif
infos.match_policy=match_policy;
infos.ambiguous_output_policy=ambiguous_output_policy;
infos.variable_error_policy=variable_error_policy;
infos.input_variables=new_Variables(infos.fst2->input_variables);
infos.output_variables=new_OutputVariables(infos.fst2->output_variables,NULL,injected_vars);
infos.dic_variables=NULL;
infos.number_of_outputs=0;
infos.start_position_last_printed_match_token=-1;
infos.end_position_last_printed_match_token=-1;
infos.start_position_last_printed_match_char=-1;
infos.end_position_last_printed_match_char=-1;
infos.start_position_last_printed_match_letter=-1;
infos.end_position_last_printed_match_letter=-1;

infos.search_limit=search_limit;
init_Korean_stuffs(&infos,is_korean);
infos.cache=new_LocateTfstTagMatchingCache(tfst->N,infos.fst2->number_of_tags);
infos.contexts=compute_contexts(infos.fst2);
/* We launch the matching for each sentence */
for (int i=1;i<=tfst->N && infos.number_of_matches!=infos.search_limit;i++) {
   if (i%100==0) {
		u_printf("\rSentence %d/%d...",i,tfst->N);
	}
   load_sentence(tfst,i);
	compute_token_contents(tfst);
	if (infos.korean!=NULL) {
	   compute_jamo_tfst_tags(&infos);
	}
	infos.matches=NULL;
	prepare_cache_for_new_sentence(infos.cache,tfst->tags->nbelems);
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
	int* visits=(int*)malloc(sizeof(int)*(1+tfst->automaton->number_of_states));
#else
	int visits[tfst->automaton->number_of_states];
#endif
	/* Within a sentence graph, we try to match from any state */
	for (int j=0;j<tfst->automaton->number_of_states;j++) {
	   for (int k=0;k<tfst->automaton->number_of_states;k++) {
	      visits[k]=0;
	   }
	   explore_tfst(visits,tfst,j,infos.fst2->initial_states[1],0,NULL,NULL,&infos,-1,-1,NULL,NULL,NULL,tilde_negation_operator);
	}
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
	free(visits);
#endif
	save_tfst_matches(&infos);
	clear_dic_variable_list(&(infos.dic_variables));
}
u_printf("\rDone.                                    \n");
/* We save some infos */
char concord_tfst_n[FILENAME_MAX];
get_path(output,concord_tfst_n);
strcat(concord_tfst_n,"concord_tfst.n");
U_FILE* f=u_fopen(vec,concord_tfst_n,U_WRITE);
if (f==NULL) {
	error("Cannot save information in %s\n",concord_tfst_n);
} else {
	u_fprintf(f,"%d match%s",infos.number_of_matches,(infos.number_of_matches<=1)?"":"es");
	if ((infos.number_of_outputs != infos.number_of_matches)
	       && (infos.number_of_outputs != 0))
	     {
	       u_fprintf(f,"(%d output%s)\n",infos.number_of_outputs,(infos.number_of_outputs<=1)?"":"s");
	     }
	u_fprintf(f,"\n");
	u_fclose(f);
}
u_printf("%d match%s",infos.number_of_matches,(infos.number_of_matches<=1)?"":"es");
if ((infos.number_of_outputs != infos.number_of_matches)
       && (infos.number_of_outputs != 0))
     {
       u_printf("(%d output%s)\n",infos.number_of_outputs,(infos.number_of_outputs<=1)?"":"s");
     }
u_printf("\n");
/* And we free things */
#ifdef REGEX_FACADE_ENGINE
free_FilterSet(infos.filters);
#endif
u_fclose(infos.output);
free_alphabet(infos.alphabet);
free_Variables(infos.input_variables);
free_OutputVariables(infos.output_variables);
free_Korean_stuffs(&infos);
free_LocateTfstTagMatchingCache(infos.cache);
for (int i=0;i<infos.fst2->number_of_states;i++) {
   free_opt_contexts(infos.contexts[i]);
}
free(infos.contexts);
free_abstract_Fst2(infos.fst2,&fst2_free);
close_text_automaton(tfst);
return 1;
}


/**
 * If we must deal with a Korean .tfst, we compute the jamo version of all fst2 tags.
 */
void init_Korean_stuffs(struct locate_tfst_infos* infos,int is_korean) {
if (infos->alphabet==NULL) {
   fatal_error("init_Korean_stuffs cannot be invoked with a NULL alphabet\n");
}
if (!is_korean) {
   infos->korean=NULL;
   infos->n_jamo_fst2_tags=0;
   infos->jamo_fst2_tags=NULL;
   infos->n_jamo_tfst_tags=0;
   infos->jamo_tfst_tags=NULL;
   return;
}
infos->korean=new Korean(infos->alphabet);
infos->n_jamo_tfst_tags=0;
infos->jamo_tfst_tags=NULL;
/* We also initializes the Chinese -> Hangul table */ 
infos->n_jamo_fst2_tags=infos->fst2->number_of_tags;
infos->jamo_fst2_tags=(unichar**)malloc(sizeof(unichar*)*infos->n_jamo_fst2_tags);
if (infos->jamo_fst2_tags==NULL) {
   fatal_alloc_error("init_Korean_stuffs");
}
unichar tmp[4096];
for (int i=0;i<infos->n_jamo_fst2_tags;i++) {
   Fst2Tag t=infos->fst2->tags[i];
   if (t->input[0]=='{' && t->input[1]!='\0') {
      /* If we have a tag like {today,.ADV}, we compute the jamo version
       * of its inflected form */
      struct dela_entry* entry=tokenize_tag_token(t->input,1);
      if (entry==NULL) {
         fatal_error("NULL dela_entry in init_Korean_stuffs\n");
      }
      if (!u_strcmp(entry->inflected,"<E>")) {
         /* <E> is a special inflected form indicating that the surface form
          * is empty. In order to be careful, we prefer not to convert it */
         u_strcpy(tmp,"<E>");
      } else {
         Hanguls_to_Jamos(entry->inflected,tmp,infos->korean,0);
      }
      free_dela_entry(entry);
   } else {
      if (i!=0) {
         Hanguls_to_Jamos(t->input,tmp,infos->korean,0);
      } else {
         /* We don't want to convert the epsilon transition tag */
         u_strcpy(tmp,"<E>");
      }
   }
   infos->jamo_fst2_tags[i]=u_strdup(tmp);
   if (infos->jamo_fst2_tags[i]==NULL) {
      fatal_alloc_error("init_Korean_stuffs");
   }
}
}


/**
 * Frees memory associated to Korean stuffs.
 */
void free_Korean_stuffs(struct locate_tfst_infos* infos) {
if (infos->korean==NULL) {
   return;
}
delete infos->korean;
for (int i=0;i<infos->n_jamo_fst2_tags;i++) {
   free(infos->jamo_fst2_tags[i]);
}
free(infos->jamo_fst2_tags);
if (infos->jamo_tfst_tags!=NULL) {
   for (int i=0;i<infos->n_jamo_tfst_tags;i++) {
      free(infos->jamo_tfst_tags[i]);
   }
   free(infos->jamo_tfst_tags);
}
}


/**
 * Computes the Jamo versions of the current sentence automaton tags, freeing the previous
 * ones if any.
 */
void compute_jamo_tfst_tags(struct locate_tfst_infos* infos) {
if (infos->jamo_tfst_tags!=NULL) {
   for (int i=0;i<infos->n_jamo_tfst_tags;i++) {
      free(infos->jamo_tfst_tags[i]);
   }
   free(infos->jamo_tfst_tags);
}
infos->n_jamo_tfst_tags=infos->tfst->tags->nbelems;
infos->jamo_tfst_tags=(unichar**)malloc(sizeof(unichar*)*infos->n_jamo_tfst_tags);
if (infos->jamo_tfst_tags==NULL) {
   fatal_alloc_error("compute_jamo_tfst_tags");
}
unichar tmp[4096];
for (int i=0;i<infos->n_jamo_tfst_tags;i++) {
   TfstTag* t=(TfstTag*)(infos->tfst->tags->tab[i]);
   if (t->content[0]=='{' && t->content[1]!='\0') {
      /* If we have a tag like {today,.ADV}, we compute the jamo version
       * of its inflected form */
      struct dela_entry* entry=tokenize_tag_token(t->content,1);
      if (entry==NULL) {
         fatal_error("NULL dela_entry in compute_jamo_tfst_tags\n");
      }
      if (!u_strcmp(entry->inflected,"<E>")) {
         /* <E> is a special inflected form indicating that the surface form
          * is empty. In order to be careful, we prefer not to convert it */
         u_strcpy(tmp,"<E>");
      } else {
         Hanguls_to_Jamos(entry->inflected,tmp,infos->korean,0);
      }
      free_dela_entry(entry);
   } else {
      if (!u_strcmp(t->content,"<E>")) {
         /* <E> is a special tag that should not be used by LocateTfst,
          * but, in order to be careful, we prefer not to convert it */
         u_strcpy(tmp,"<E>");
      } else {
         Hanguls_to_Jamos(t->content,tmp,infos->korean,0);
      }
   }
   infos->jamo_tfst_tags[i]=u_strdup(tmp);
   if (infos->jamo_tfst_tags[i]==NULL) {
      fatal_alloc_error("compute_jamo_tfst_tags");
   }
}
}


/**
 * Explores in parallel the tfst and the fst2.
 */
void explore_tfst(int* visits,Tfst* tfst,int current_state_in_tfst,
		          int current_state_in_fst2,int graph_depth,
		          struct tfst_match* match_element_list,
                struct tfst_match_list* *LIST,
                struct locate_tfst_infos* infos,
                /* This is used for Korean only when a fst2 tag contains a token that
                 * can match several boxes in the tfst. 'pos_kr_in_fst2_tag' represents 
                 * the current position in the current fst2 tag, or -1 if unused. 
                 * 'current_kr_fst2_transition' is the current fst2 transition being explored if
                 * 'pos_kr_in_fst2_tag' is not -1; null otherwise. */
                int pos_pending_in_fst2_tag,
                int pos_pending_in_tfst_tag,
                Transition* current_pending_fst2_transition,
                Transition* current_pending_tfst_transition,
                struct list_context* ctx /* information about the current context, if any */,
                int tilde_negation_operator) {
//error("visits for current state=%d  tfst state=%d  fst2 state=%d\n",visits[current_state_in_tfst],current_state_in_tfst,current_state_in_fst2);
if (visits[current_state_in_tfst]>MAX_VISITS_PER_TFST_STATE) {
   /* If there are too much recursive calls */
   return;
}
visits[current_state_in_tfst]++;
if (current_pending_fst2_transition!=NULL && current_pending_tfst_transition!=NULL) {
   fatal_error("Internal error in explore_tfst: cannot have two non NULL pending transitions\n");
}

/* CASE 1: if we have not finished to explore a fst2 tag in the grammar */
if (current_pending_fst2_transition!=NULL) {
	if (infos->match_word_boundaries) {
		return;
	}
   struct tfst_match* list=NULL;
   Transition* text_transition=tfst->automaton->states[current_state_in_tfst]->outgoing_transitions;
   /* For a given tag in the grammar, we test all the transitions in the
    * text automaton, and we note all the states we can reach */
   while (text_transition!=NULL) {
      int pos_pending_fst2=pos_pending_in_fst2_tag;
      int pos_pending_tfst=pos_pending_in_tfst_tag;
      int result=match_between_text_and_grammar_tags(tfst,(TfstTag*)(tfst->tags->tab[text_transition->tag_number]),
                                              infos->fst2->tags[current_pending_fst2_transition->tag_number],
                                              text_transition->tag_number,
                                              current_pending_fst2_transition->tag_number,
                                              infos,&pos_pending_fst2,&pos_pending_tfst,tilde_negation_operator);
      if (result==OK_MATCH_STATUS) {
         /* Case of a match with something in the text automaton (i.e. <V>) */
         list=insert_in_tfst_matches(list,current_state_in_tfst,text_transition->state_number,
               current_pending_fst2_transition,pos_pending_fst2,text_transition->tag_number,0);
      }
      else if (result==TEXT_INDEPENDENT_MATCH) {
         /* Case of a match independent of the text automaton (i.e. <E>) */
         /* I (S.P.) suspect that we should not arrive here, since the current
          * case #1 is supposed to occur when a fst2 tag was not fully consumed */
         list=insert_in_tfst_matches(list,current_state_in_tfst,current_state_in_tfst,
               current_pending_fst2_transition,-1,NO_TEXT_TOKEN_WAS_MATCHED,0);
      }
      else if (result==PARTIAL_MATCH_STATUS) {
         /* If we have consumed all the fst2 tag but not all the tfst one, we go on */
         explore_tfst(visits,tfst,current_state_in_tfst,current_pending_fst2_transition->state_number,
                                                graph_depth,list,LIST,infos,-1,pos_pending_tfst,NULL,current_pending_tfst_transition,ctx,tilde_negation_operator);
      }
      text_transition=text_transition->next;
   }
   struct tfst_match* tmp;
   /* Then, we continue the exploration from the reached states. This
    * procedure avoids exploring several times a same state when
    * it can be reached through various tags in the text automaton.
    * For instance, if we have "le" in the grammar and {le,.DET} and
    * {le,.PRO} in the text automaton that point to the same state XXX,
    * we will explore XXX just once. */
   while (list!=NULL) {
      tmp=list->next;
      list->next=match_element_list;
      /* match_element_list is pointed by one more element */
      if (match_element_list!=NULL) {
       (match_element_list->pointed_by)++;
      }
      Transition* tmp_trans=(list->pos_kr!=-1)?list->fst2_transition:NULL;
      int dest_state=(list->pos_kr!=-1)?-1:current_pending_fst2_transition->state_number;
      explore_tfst(visits,tfst,list->dest_state_text,dest_state,
                        graph_depth,list,LIST,infos,list->pos_kr,-1,tmp_trans,NULL,ctx,tilde_negation_operator);
      if (list->pointed_by==0) {
         /* If list is not blocked by being part of a match for the calling
          * graph, we can free it */
         list->next=NULL;
         if (match_element_list!=NULL) {(match_element_list->pointed_by)--;}
         free_tfst_match(list);
      }
      list=tmp;
   }
   return;
} /* END OF CASE 1 */


/* CASE 2: if we have not finished to explore a tfst tag */
if (current_pending_tfst_transition!=NULL) {
	if (infos->match_word_boundaries) {
		return;
	}
   Fst2State current_state_in_grammar=infos->fst2->states[current_state_in_fst2];
   Transition* grammar_transition=current_state_in_grammar->transitions;
   struct tfst_match* list=NULL;
   while (grammar_transition!=NULL) {
      int e=grammar_transition->tag_number;
      if (e<0) {
         /* We forbid subgraph calls when a partial tfst tag match is running */
         return;
      }
      int pos_pending_fst2=pos_pending_in_fst2_tag;
      int pos_pending_tfst=pos_pending_in_tfst_tag;
      int result=match_between_text_and_grammar_tags(tfst,(TfstTag*)(tfst->tags->tab[current_pending_tfst_transition->tag_number]),
                                                       infos->fst2->tags[grammar_transition->tag_number],
                                                       current_pending_tfst_transition->tag_number,
                                                       grammar_transition->tag_number,
                                                       infos,&pos_pending_fst2,&pos_pending_tfst,tilde_negation_operator);
      if (result==OK_MATCH_STATUS) {
         /* Case of a match with something in the text automaton (i.e. <V>) */
         list=insert_in_tfst_matches(list,current_state_in_tfst,current_pending_tfst_transition->state_number,
               grammar_transition,pos_pending_fst2,current_pending_tfst_transition->tag_number,1);
      }
      else if (result==TEXT_INDEPENDENT_MATCH || result==PARTIAL_MATCH_STATUS) {
         /* If we have a match independent of the text automaton (i.e. <E>) 
          * or that does not consume all the tfst tag, we go on */
         struct tfst_match* foo=insert_in_tfst_matches(NULL,current_state_in_tfst,current_state_in_tfst,
                                      grammar_transition,pos_pending_tfst,NO_TEXT_TOKEN_WAS_MATCHED,1);
         foo->next=match_element_list;
         /* match_element_list is pointed by one more element */
         if (match_element_list!=NULL) {
            (match_element_list->pointed_by)++;
         }
         explore_tfst(visits,tfst,current_state_in_tfst,grammar_transition->state_number,
                                       graph_depth,foo,LIST,infos,-1,pos_pending_tfst,NULL,current_pending_tfst_transition,ctx,tilde_negation_operator);
         if (foo->pointed_by==0) {
            /* If list is not blocked by being part of a match for the calling
             * graph, we can free it */
            foo->next=NULL;
            if (match_element_list!=NULL) {(match_element_list->pointed_by)--;}
            free_tfst_match(foo);
         } 
      }
      grammar_transition=grammar_transition->next;
   }
   struct tfst_match* tmp;
   /* Then, we continue the exploration from the reached states. This
    * procedure avoids exploring several times a same state when
    * it can be reached through various tags in the text automaton.
    * For instance, if we have "le" in the grammar and {le,.DET} and
    * {le,.PRO} in the text automaton that point to the same state XXX,
    * we will explore XXX just once. */
   while (list!=NULL) {
      tmp=list->next;
      list->next=match_element_list;
      /* match_element_list is pointed by one more element */
      if (match_element_list!=NULL) {
         (match_element_list->pointed_by)++;
      }
      Transition* tmp_trans=(list->pos_kr!=-1)?list->fst2_transition:NULL;
      int dest_state=(list->pos_kr!=-1)?-1:list->fst2_transition->state_number;
      explore_tfst(visits,tfst,list->dest_state_text,dest_state,
                        graph_depth,list,LIST,infos,list->pos_kr,-1,tmp_trans,NULL,ctx,tilde_negation_operator);
      if (list->pointed_by==0) {
         /* If list is not blocked by being part of a match for the calling
          * graph, we can free it */
         list->next=NULL;
         if (match_element_list!=NULL) {(match_element_list->pointed_by)--;}
         free_tfst_match(list);
      }
      list=tmp;
   }
   return;
} /* END OF CASE 2 */


/* CASE 3: we are in the normal state exploration case */
Fst2State current_state_in_grammar=infos->fst2->states[current_state_in_fst2];
if (is_final_state(current_state_in_grammar)) {
   /* If the current state is final */
   if (ctx!=NULL) {
      /* If we have reached the final state of a graph while
       * looking for a context, it's an error because every
       * opened context must be closed before the end of the graph. */
      fatal_error("ERROR: unclosed context\n");
   }   
   if (graph_depth==0) {
      /* If we are in the main graph, we add a match to the main match list */
      if (match_element_list!=NULL) {
    	  add_tfst_match(infos,match_element_list);
      }
   } else {
      /* If we are in a subgraph, we add a match to the current match list */
	   if (match_element_list!=NULL) {
		  (*LIST)=add_match_in_list((*LIST),match_element_list);
	   }
   }
}

/* We test every transition that goes out the current state in the grammar */
Transition* grammar_transition=current_state_in_grammar->transitions;
while (grammar_transition!=NULL) {
   int e=grammar_transition->tag_number;
   if (e<0) {
      /* Case of a subgraph */
      struct tfst_match_list* list_for_subgraph=NULL;
      struct tfst_match_list* tmp;

      explore_tfst(visits,tfst,current_state_in_tfst,infos->fst2->initial_states[-e],
              graph_depth+1,match_element_list,&list_for_subgraph,infos,-1,-1,NULL,NULL,
              NULL,tilde_negation_operator); /* ctx is set to NULL because the end of a context must occur in the
                      * same graph than its beginning */
      while (list_for_subgraph!=NULL) {
         tmp=list_for_subgraph->next;
         /* Before exploring an element that points on a subgraph match,
          * we decrease its 'pointed_by' variable that was previously increased
          * in the 'add_match_in_list' function */
         if (list_for_subgraph->match!=NULL) {
        	 (list_for_subgraph->match->pointed_by)--;
        	 explore_tfst(visits,tfst,list_for_subgraph->match->dest_state_text,
                           grammar_transition->state_number,
                           graph_depth,list_for_subgraph->match,LIST,infos,-1,-1,NULL,NULL,ctx,tilde_negation_operator);
        	 /* Finally, we remove, if necessary, the list of match element
        	  * that was used for storing the subgraph match. This cleaning
        	  * will only free elements that are not involved in others
        	  * matches, that is to say element with pointed_by=0 */
        	 clean_tfst_match_list(list_for_subgraph->match,match_element_list);
         }
         free(list_for_subgraph);
         list_for_subgraph=tmp;
      }
   }
   else if (infos->fst2->tags[e]->type==BEGIN_POSITIVE_CONTEXT_TAG) {
      /* If we have a positive context tag $[ */
      struct opt_contexts* context=infos->contexts[current_state_in_fst2];
      if (context==NULL) {
         fatal_error("Unexpected NULL contexts in explore_tfst\n");
      }
      Transition* t;
      for (int n_ctxt=0;n_ctxt<context->size_positive;n_ctxt=n_ctxt+2) {
         t=context->positive_mark[n_ctxt];
         /* We look for a positive context from the current position */
         struct list_context* c=new_list_context(0,ctx);
         explore_tfst(visits,tfst,current_state_in_tfst,t->state_number,
                                    graph_depth,NULL,LIST,infos,-1,-1,NULL,NULL,c,tilde_negation_operator);
         /* Note that there is no matches to free since matches cannot be built within a context */
         if (c->n) {
            /* If the context has matched, then we can explore all the paths
             * that starts from the context end */
            Transition* states=context->positive_mark[n_ctxt+1];
            struct tfst_match* debug_match_element=match_element_list;
            if (infos->debug && c->output!=NULL) {
            	debug_match_element=new_debug_tfst_match(c->output,match_element_list);
            }
            while (states!=NULL) {
               explore_tfst(visits,tfst,current_state_in_tfst,states->state_number,
                                          graph_depth,debug_match_element,LIST,infos,-1,-1,NULL,NULL,ctx,tilde_negation_operator);
               states=states->next;
            }
            if (debug_match_element!=match_element_list) {
            	free_tfst_match(debug_match_element);
            }
         }
         free_list_context(c);
      }
      /* End of $[ case */
   }
   else if (infos->fst2->tags[e]->type==BEGIN_NEGATIVE_CONTEXT_TAG) {
      /* If we have a positive context tag $![ */
      struct opt_contexts* context=infos->contexts[current_state_in_fst2];
      if (context==NULL) {
         fatal_error("Unexpected NULL contexts in explore_tfst\n");
      }
      Transition* t;
      for (int n_ctxt=0;n_ctxt<context->size_negative;n_ctxt=n_ctxt+2) {
         t=context->negative_mark[n_ctxt];
         /* We look for a negative context from the current position */
         struct list_context* c=new_list_context(0,ctx);
         explore_tfst(visits,tfst,current_state_in_tfst,t->state_number,
                                    graph_depth,NULL,LIST,infos,-1,-1,NULL,NULL,c,tilde_negation_operator);
         /* Note that there is no matches to free since matches cannot be built within a context */
         if (!c->n) {
            /* If the context has not matched, then we can explore all the paths
             * that starts from the context end */
            Transition* states=context->negative_mark[n_ctxt+1];
            while (states!=NULL) {
               explore_tfst(visits,tfst,current_state_in_tfst,states->state_number,
                                          graph_depth,match_element_list,LIST,infos,-1,-1,NULL,NULL,ctx,tilde_negation_operator);
               states=states->next;
            }
         }
         free_list_context(c);
      }
      /* End of $![ case */
   }
   else if (infos->fst2->tags[e]->type==END_CONTEXT_TAG) {
      /* If we have a $] tag */
      if (ctx==NULL) {
         /* If there was no current opened context, it's an error */
         error("ERROR: unexpected closing context mark\n");
         return;
      }
      /* Otherwise, we just indicate that we have found a context closing mark,
       * and we return */
      ctx->n=1;
      if (infos->debug && ctx->output==NULL) {
    	  Ustring* output=get_debug_output_in_context(match_element_list,infos);
    	  set_list_context_output(ctx,output->str);
    	  free_Ustring(output);
      }
      return;
      /* End of $] case */
   } else {
      /* Normal case (not a subgraph call) */
      struct tfst_match* list=NULL;
      Transition* text_transition=tfst->automaton->states[current_state_in_tfst]->outgoing_transitions;
	  if (text_transition==NULL) {
    	  /* If there is no transition, it should mean that we are on the final state, but
    	   * even here, some special transitions may match */
    	  if (!u_strcmp(infos->fst2->tags[e]->input,"<E>")) {
    		  list=insert_in_tfst_matches(list,current_state_in_tfst,current_state_in_tfst,
    		              grammar_transition,-1,NO_TEXT_TOKEN_WAS_MATCHED,1);
    	  }
    	  else if (!u_strcmp(infos->fst2->tags[e]->input,"{$}")) {
    		  /* {$} may match only if we are in the final state of the last sentence */
    		  if (is_final_state(tfst->automaton->states[current_state_in_tfst])
    				  && tfst->current_sentence==tfst->N) {
    			  list=insert_in_tfst_matches(list,current_state_in_tfst,current_state_in_tfst,
    			      		              grammar_transition,-1,NO_TEXT_TOKEN_WAS_MATCHED,1);
    		  }
    	  }
      }
      /* {^} can be tested without considering text transitions */
	  if (!u_strcmp(infos->fst2->tags[e]->input,"{^}")) {
		  /* {^} may match only if we are in the initial state of the first sentence */
		  if (is_initial_state(tfst->automaton->states[current_state_in_tfst])
				  && tfst->current_sentence==1) {
			  list=insert_in_tfst_matches(list,current_state_in_tfst,current_state_in_tfst,
			      		              grammar_transition,-1,NO_TEXT_TOKEN_WAS_MATCHED,1);
		  }
	  }
      /* For a given tag in the grammar, we test all the transitions in the
       * text automaton, and we note all the states we can reach */
      while (text_transition!=NULL) {
         int pos_pending_fst2=-1;
         int pos_pending_tfst=-1;
         
         int result=match_between_text_and_grammar_tags(tfst,(TfstTag*)(tfst->tags->tab[text_transition->tag_number]),
                                                 infos->fst2->tags[grammar_transition->tag_number],
                                                 text_transition->tag_number,
                                                 grammar_transition->tag_number,
                                                 infos,&pos_pending_fst2,&pos_pending_tfst,tilde_negation_operator);
         if (result==OK_MATCH_STATUS) {
            /* Case of a match with something in the text automaton (i.e. <V>) */
            list=insert_in_tfst_matches(list,current_state_in_tfst,text_transition->state_number,
                 grammar_transition,pos_pending_fst2,text_transition->tag_number,1);
         }
         else if (result==TEXT_INDEPENDENT_MATCH) {
            /* Case of a match independent of the text automaton (i.e. <E>) */
            list=insert_in_tfst_matches(list,current_state_in_tfst,current_state_in_tfst,
                 grammar_transition,-1,NO_TEXT_TOKEN_WAS_MATCHED,1);
         } else if (result==PARTIAL_MATCH_STATUS) {
            /* If the fst2 tag was entirely consumed, but not the tfst one,
             * we must go on. At the opposite of a partial fst2 tag, we don't 
             * add a normal partial match to 'list', because this must only be done when
             * a tfst tag has matched, and this is not the case here. We add a
             * text independent match, just in order to use the output associated to
             * the fst2 tag, if any */
            struct tfst_match* foo=insert_in_tfst_matches(NULL,current_state_in_tfst,current_state_in_tfst,
                             grammar_transition,pos_pending_tfst,NO_TEXT_TOKEN_WAS_MATCHED,1);
            foo->next=match_element_list;
            /* match_element_list is pointed by one more element */
            if (match_element_list!=NULL) {
               (match_element_list->pointed_by)++;
            }
            explore_tfst(visits,tfst,current_state_in_tfst,grammar_transition->state_number,
                              graph_depth,foo,LIST,infos,-1,pos_pending_tfst,NULL,text_transition,ctx,tilde_negation_operator);
            if (foo->pointed_by==0) {
               /* If list is not blocked by being part of a match for the calling
                * graph, we can free it */
               foo->next=NULL;
               if (match_element_list!=NULL) {(match_element_list->pointed_by)--;}
               free_tfst_match(foo);
            }
         }
         text_transition=text_transition->next;
      }
      struct tfst_match* tmp;
      /* Then, we continue the exploration from the reached states. This
       * procedure avoids exploring several times a same state when
       * it can be reached through various tags in the text automaton.
       * For instance, if we have "le" in the grammar and {le,.DET} and
       * {le,.PRO} in the text automaton that point to the same state XXX,
       * we will explore XXX just once. */
      while (list!=NULL) {
         tmp=list->next;
         list->next=match_element_list;
         /* match_element_list is pointed by one more element */
         if (match_element_list!=NULL) {
        	   (match_element_list->pointed_by)++;
         }
         Transition* tmp_trans=(list->pos_kr!=-1)?list->fst2_transition:NULL;
         int dest_state_in_fst2=(list->pos_kr!=-1)?-1:grammar_transition->state_number;
         explore_tfst(visits,tfst,list->dest_state_text,dest_state_in_fst2,
                           graph_depth,list,LIST,infos,list->pos_kr,-1,tmp_trans,NULL,ctx,tilde_negation_operator);
         if (list->pointed_by==0) {
            /* If list is not blocked by being part of a match for the calling
             * graph, we can free it */
            list->next=NULL;
            if (match_element_list!=NULL) {(match_element_list->pointed_by)--;}
            free_tfst_match(list);
         }
         list=tmp;
      }
   }
   grammar_transition=grammar_transition->next;
}
}


/**
 * This function tests if a text tag can be matched by a grammar tag, but only
 * if the result is not already known.
 */
int match_between_text_and_grammar_tags(Tfst* tfst,TfstTag* text_tag,Fst2Tag grammar_tag,
                                        int tfst_tag_index,int fst2_tag_index,
                                        struct locate_tfst_infos* infos,
                                        int *pos_pending_fst2_tag,int *pos_pending_tfst_tag,
                                        int tilde_negation_operator) {
if (grammar_tag->type==BEGIN_POSITIVE_CONTEXT_TAG
   || grammar_tag->type==BEGIN_NEGATIVE_CONTEXT_TAG
   || grammar_tag->type==END_CONTEXT_TAG
   || grammar_tag->type==TEXT_START_TAG
   || grammar_tag->type==TEXT_END_TAG) {
   /* A context should not start or end within a text tag, so we fail here */
	return NO_MATCH_STATUS;
}

/* We start by looking at special fst2 tags */
if (grammar_tag->type==BEGIN_MORPHO_TAG
   || grammar_tag->type==END_MORPHO_TAG) {
   fatal_error("Tag '%S' should not be found in a grammar applied to a text automaton\n",grammar_tag->input);
}
/*******************************************************
 * We want to match something that is text independent */
if (grammar_tag->type==BEGIN_VAR_TAG
   || grammar_tag->type==END_VAR_TAG
   || grammar_tag->type==BEGIN_OUTPUT_VAR_TAG
   || grammar_tag->type==END_OUTPUT_VAR_TAG
   || grammar_tag->type==LEFT_CONTEXT_TAG
   || !u_strcmp(grammar_tag->input,"<E>")) {
   return TEXT_INDEPENDENT_MATCH;
}

/* Here we test the special case of the " " and # tags that are contextual matches, and 
 * for this reason, that cannot be cached */
if (!u_strcmp(grammar_tag->input," ")) {
   if ((*pos_pending_tfst_tag)==-1 && is_space_on_the_left_in_tfst(tfst,text_tag)) {
      //error("space has matched with %S\n",text_tag->content);
      return TEXT_INDEPENDENT_MATCH;
   }
   return NO_MATCH_STATUS;
}
if (!u_strcmp(grammar_tag->input,"#")) {
   if (*pos_pending_tfst_tag>0 || !is_space_on_the_left_in_tfst(tfst,text_tag)) {
      return TEXT_INDEPENDENT_MATCH;
   }
   return NO_MATCH_STATUS;
}
if (!u_strcmp(text_tag->content,"{STOP}")) {
   /* {STOP} can NEVER be matched */
   return NO_MATCH_STATUS;
}
/* Now, we will compare the text tag and the grammar one, but only if we
 * don't already know the result */
int result=get_cached_result(infos->cache,text_tag->content,fst2_tag_index,tfst_tag_index,
                             *pos_pending_fst2_tag,*pos_pending_tfst_tag,pos_pending_fst2_tag,pos_pending_tfst_tag);
if (result!=UNKNOWN_MATCH_STATUS) {
   return result;
}
/* Otherwise, we compute the result */
int old_pos_pending_fst2_tag=*pos_pending_fst2_tag;
int old_pos_pending_tfst_tag=*pos_pending_tfst_tag;
result=real_match_between_text_and_grammar_tags(tfst,text_tag,grammar_tag,
                                                tfst_tag_index,fst2_tag_index,
                                                infos,pos_pending_fst2_tag,pos_pending_tfst_tag,tilde_negation_operator);
/* And we save it in the cache */
set_cached_result(infos->cache,tfst_tag_index,fst2_tag_index,
      old_pos_pending_fst2_tag,old_pos_pending_tfst_tag,result,*pos_pending_fst2_tag,*pos_pending_tfst_tag);
return result;
}


/**
 * This function tests if a text tag can be matched by a grammar tag.
 * 
 * WARNING: as we use a cache that is supposed to be context independent, no
 *          context dependent operation must be done here. Contextual
 *          operations like # or " " must be handled in 
 *          'match_between_text_and_grammar_tags'
 */
int real_match_between_text_and_grammar_tags(Tfst* tfst,TfstTag* text_tag,Fst2Tag grammar_tag,
                                        int tfst_tag_index,int fst2_tag_index,
                                        struct locate_tfst_infos* infos,
                                        int *pos_pending_fst2_tag,int *pos_pending_tfst_tag,
                                        int tilde_negation_operator) {
DISCARD_UNUSED_PARAMETER(tfst)
int ret_value;
if (/*infos->korean &&*/ *pos_pending_fst2_tag!=-1 && *pos_pending_tfst_tag!=-1) {
   fatal_error("Internal error in match_between_text_and_grammar_tags: cannot have partial match on both\n"
               "text tag and grammar tag\n");
}
//error("ici tag=%S fst2 #%d\n",text_tag->content,fst2_tag_index);
if (/*infos->korean &&*/ (*pos_pending_fst2_tag!=-1 || (grammar_tag->input[0]!='{' && grammar_tag->input[0]!='<'))) {
   /* If we have an untagged token in the fst2 */
   if (*pos_pending_fst2_tag==-1) {
      /* If we were not in token exploration mode, we turn into this mode now */
      *pos_pending_fst2_tag=0;
   }
   unichar* jamo_tfst;
   unichar* jamo_fst2;
   struct dela_entry* my_entry=NULL;
   if (infos->korean) {
	   jamo_tfst=infos->jamo_tfst_tags[tfst_tag_index];
	   jamo_fst2=infos->jamo_fst2_tags[fst2_tag_index];
   } else {
	   if (text_tag->content[0]=='{' && text_tag->content[1]!='\0') {
	         /* text={toto,tutu.XXX} */
		  my_entry=tokenize_tag_token(text_tag->content,1);
	   	  if (my_entry==NULL) {
	   		  fatal_error("NULL text_entry error in match_between_text_and_grammar_tags\n");
	   	  }
	   	  jamo_tfst=my_entry->inflected;
	   } else {
		   jamo_tfst=text_tag->content;
	   }
	   jamo_fst2=grammar_tag->input;
   }
   int k=(*pos_pending_fst2_tag);
   int j=(*pos_pending_tfst_tag!=-1)?(*pos_pending_tfst_tag):0;
   while (jamo_fst2[k]!='\0' && jamo_tfst[j]!='\0') {
      /* We ignore syllable bounds in both tfst and fst2 tags */
      if (jamo_fst2[k]==KR_SYLLABLE_BOUND) {
         k++;
         continue;
      }
      if (jamo_tfst[j]==KR_SYLLABLE_BOUND) {
         j++;
         continue;
      }
      if ((grammar_tag->control & RESPECT_CASE_TAG_BIT_MASK && jamo_fst2[k]!=jamo_tfst[j])
    		  || !is_equal_or_uppercase(jamo_fst2[k],jamo_tfst[j],infos->alphabet)) {
         /* If a character doesn't match */
         //error("match failed between tfst=%S and fst2=%S\n",jamo_tfst,jamo_fst2);
    	 if (my_entry!=NULL) free_dela_entry(my_entry);
         return NO_MATCH_STATUS;
      }
      k++;
      j++;
   }
   if (jamo_fst2[k]=='\0' && jamo_tfst[j]=='\0') {
      /* If we are at both ends of strings, it's a full match */
      (*pos_pending_fst2_tag)=-1;
      //error("XX full match between tfst=%S and fst2=%S\n",jamo_tfst,jamo_fst2);
      if (my_entry!=NULL) free_dela_entry(my_entry);
      return OK_MATCH_STATUS;
   }
   if (jamo_fst2[k]=='\0') {
      /* If we are at the end of the fst2 tag but not at the end of the tfst tag, 
       * it's a partial match */
      (*pos_pending_fst2_tag)=-1;
      (*pos_pending_tfst_tag)=j;
      //error("ZZ partial match between tfst=%S and fst2=%S\n",jamo_tfst,jamo_fst2);
      if (my_entry!=NULL) free_dela_entry(my_entry);
      return PARTIAL_MATCH_STATUS;
   }
   /* If we have consumed all the tfst tag, but not all the fst2 one, it's a partial match */
   (*pos_pending_fst2_tag)=k;
   (*pos_pending_tfst_tag)=-1;
   //error("YY partial match #2 between tfst=%S and fst2=%S\n",jamo_tfst,jamo_fst2);
   if (my_entry!=NULL) free_dela_entry(my_entry);
   return OK_MATCH_STATUS;
}

struct dela_entry* grammar_entry=NULL;
struct dela_entry* text_entry=NULL;
/**************************************************
 * We want to match a token like "le" */

int pos_in_tfst_input = (*pos_pending_tfst_tag)!=-1 ? (*pos_pending_tfst_tag) : 0;
if (pos_in_tfst_input!=0 && text_tag->content[0]=='{') {
	   /* pos_in_tfst_input contains a value that is relative to the whole tag like {toto,tutu.XXX},
	    * so we have to substract 1 to start on the inflected form
	    */
	   pos_in_tfst_input--;
}



if (is_letter(grammar_tag->input[0],infos->alphabet)) {
   if (is_letter(text_tag->content[0],infos->alphabet)) {
      /* text= "toto" */
      if (grammar_tag->control & RESPECT_CASE_TAG_BIT_MASK) {
         /* If we must respect case */
         if (!u_strcmp(grammar_tag->input,text_tag->content+pos_in_tfst_input)) {return OK_MATCH_STATUS;}
         else {return NO_MATCH_STATUS;}
      } else {
         /* If case does not matter */
         if (is_equal_or_uppercase(grammar_tag->input,text_tag->content+pos_in_tfst_input,infos->alphabet)) {return OK_MATCH_STATUS;}
         else {return NO_MATCH_STATUS;}
      }
   } else if (text_tag->content[0]=='{' && text_tag->content[1]!='\0') {
      /* text={toto,tutu.XXX} */
	  text_entry=tokenize_tag_token(text_tag->content,1);
	  if (text_entry==NULL) {
		  fatal_error("NULL text_entry error in match_between_text_and_grammar_tags\n");
	  }
	  if (grammar_tag->control & RESPECT_CASE_TAG_BIT_MASK) {
         /* If we must respect case */
		 if (!u_strcmp(grammar_tag->input,text_entry->inflected+pos_in_tfst_input)) {
			 goto ok_match;
		 } else {
			 goto no_match;
		 }
      } else {
         /* If case does not matter */
    	 if (is_equal_or_uppercase(grammar_tag->input,text_entry->inflected+pos_in_tfst_input,infos->alphabet)) {
    		 goto ok_match;
    	 } else {
    		 goto no_match;
    	 }
      }
   }
   return NO_MATCH_STATUS;
}

/**************************************************
 * We want to match a tag like "{tutu,toto.XXX}" */
if (grammar_tag->input[0]=='{' && u_strcmp(grammar_tag->input,"{S}")) {
   if (text_tag->content[0]!='{') {
      /* If the text tag is not of the form "{tutu,toto.XXX}" */
      return NO_MATCH_STATUS;
   }
   grammar_entry=tokenize_tag_token(grammar_tag->input,1);
   text_entry=tokenize_tag_token(text_tag->content,1);
   if (!is_equal_or_uppercase(grammar_entry->inflected,text_entry->inflected+pos_in_tfst_input,infos->alphabet)) {
      /* We allow case variations on the inflected form :
       * if there is "{tutu,toto.XXX}" in the grammar, we want it
       * to match "{Tutu,toto.XXX}" in the text automaton */
	  goto no_match;
   }
   if (u_strcmp(grammar_entry->lemma,text_entry->lemma)) {
      /* If lemmas are different,we don't match */
	  goto no_match;
   }
   if (!same_codes(grammar_entry,text_entry)) {
      /* If grammatical, semantical and inflectional informations
       * are different we don't match*/
	  goto no_match;
   }
   goto ok_match;
}

/**************************************************
 * We want to match something like "<....>". The
 * <E> case has already been handled in text independent matchings */
if (grammar_tag->input[0]=='<' && grammar_tag->input[1]!='\0') {
	/* We tokenize the text tag, if we have one */
	if (text_tag->content[0]=='{' && text_tag->content[1]!='\0') {
	   text_entry=tokenize_tag_token(text_tag->content,1);
	}
   if (!u_strcmp(grammar_tag->input,"<MOT>") || !u_strcmp(grammar_tag->input,"<WORD>")) {
      /* <MOT> matches a sequence of letters or a tag like {tutu,toto.XXX}, even
       * if 'tutu' is not made of characters,
       * BUT, it matches only if we are not already inside a tfst tag in order
       * to avoid that a grammar like "pr <MOT>" could match tags like
       * {préciser,.V:W}
       * <WORD> can also be used instead of <MOT> */
      if ((is_letter(text_tag->content[pos_in_tfst_input],infos->alphabet) || text_entry!=NULL)
    		  && !((*pos_pending_tfst_tag)>0)) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<!MOT>") || !u_strcmp(grammar_tag->input,"<!WORD>")) {
      /* <!MOT> matches the opposite of <MOT> 
       <!WORD> matches the opposite of <WORD>*/
      if (!is_letter(text_tag->content[pos_in_tfst_input],infos->alphabet)
          && text_entry==NULL) {
    	  goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<MIN>") || !u_strcmp(grammar_tag->input,"<LOWER>")) {
      /* <MIN> or <LOWER> matches a sequence of letters or a tag like {tutu,toto.XXX}
       * only made of lower case letters */
      if (is_sequence_of_lowercase_letters(text_tag->content+pos_in_tfst_input,infos->alphabet) ||
          (text_entry!=NULL && is_sequence_of_lowercase_letters(text_entry->inflected+pos_in_tfst_input,infos->alphabet))) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<!MIN>") || !u_strcmp(grammar_tag->input,"<!LOWER>")) {
      /* <!MIN> or <!LOWER> matches a sequence of letters or a tag like {tutu,toto.XXX}
       * that is not only made of lower case letters */
      if ((is_letter(text_tag->content[pos_in_tfst_input],infos->alphabet) && !is_sequence_of_lowercase_letters(text_tag->content+pos_in_tfst_input,infos->alphabet))
          || (text_entry!=NULL && !is_sequence_of_lowercase_letters(text_entry->inflected+pos_in_tfst_input,infos->alphabet))) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<MAJ>") || !u_strcmp(grammar_tag->input,"<UPPER>")) {
      /* <MAJ> or <UPPER> matches a sequence of letters or a tag like {TUTU,toto.XXX}
       * only made of upper case letters */
      if (is_sequence_of_uppercase_letters(text_tag->content+pos_in_tfst_input,infos->alphabet) ||
          (text_entry!=NULL && is_sequence_of_uppercase_letters(text_entry->inflected+pos_in_tfst_input,infos->alphabet))) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<!MAJ>") || !u_strcmp(grammar_tag->input,"<!UPPER>")) {
      /* <!MAJ> or <!UPPER> matches a sequence of letters or a tag like {tutu,toto.XXX}
       * that is not only made of upper case letters */
      if ((is_letter(text_tag->content[pos_in_tfst_input],infos->alphabet) && !is_sequence_of_uppercase_letters(text_tag->content+pos_in_tfst_input,infos->alphabet))
          || (text_entry!=NULL && !is_sequence_of_uppercase_letters(text_entry->inflected+pos_in_tfst_input,infos->alphabet))) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<PRE>") || !u_strcmp(grammar_tag->input,"<FIRST>")) {
      /* <PRE> or <FIRST> matches a sequence of letters or a tag like {TUTU,toto.XXX}
       * that begins by an upper case letter */
      if (is_upper(text_tag->content[pos_in_tfst_input],infos->alphabet) ||
          (text_entry!=NULL && is_upper(text_entry->inflected[pos_in_tfst_input],infos->alphabet))) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<!PRE>") || !u_strcmp(grammar_tag->input,"<!FIRST>")) {
      /* <!PRE> or <!FIRST> matches a sequence of letters or a tag like {tutu,toto.XXX}
       * that does not begin by an upper case letter */
      if ((is_letter(text_tag->content[pos_in_tfst_input],infos->alphabet) && !is_upper(text_tag->content[pos_in_tfst_input],infos->alphabet)) ||
          (text_entry!=NULL && is_letter(text_entry->inflected[pos_in_tfst_input],infos->alphabet) && !is_upper(text_entry->inflected[pos_in_tfst_input],infos->alphabet))) {
         /* We test is_letter+!is_upper instead of is_lower in order to handle
          * cases where there is no difference between lower and upper case letters
          * (for instance Thai) */
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<TOKEN>")) {
      /* <TOKEN> matches anything but the {STOP} tag, and the {STOP} tag case
       * has already been dealt with */
      goto ok_match;
   }
   if (!u_strcmp(grammar_tag->input,"<DIC>")) {
      /* <DIC> matches any tag like {tutu,toto.XXX} */
      if (text_entry!=NULL) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<TDIC>")) {
      /* <TDIC> matches any tag like {tutu,toto.XXX} */
      if (text_entry!=NULL) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<!DIC>")) {
      /* <!DIC> matches any sequence of letters that is not a tag like {tutu,toto.XXX} */
      if (is_letter(text_tag->content[0],infos->alphabet)) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<SDIC>")) {
      /* <SDIC> matches any tag like {tutu,toto.XXX} where tutu is simple word
       *
       * NOTE: according to the formal definition of simple words, something like
       *       {3,.NUMBER} is not considered as a simple word since 3 is not
       *       a letter. It will be considered as a compound word */
      if (text_entry!=NULL && is_sequence_of_letters(text_entry->inflected,infos->alphabet)) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<CDIC>")) {
      /* <CDIC> matches any tag like {tutu,toto.XXX} where tutu is a compound word
       *
       * NOTE: see <SDIC> note */
      if (text_entry!=NULL && !is_sequence_of_letters(text_entry->inflected,infos->alphabet)) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<NB>")) {
      /* <NB> matches any tag like 3 or {37,toto.XXX} */
      if (u_are_digits(text_tag->content) ||
          (text_entry!=NULL && u_are_digits(text_entry->inflected))) {
         goto ok_match;
      }
      goto no_match;
   }
   /**************************************************************
    * If we arrive here, we are in the case of a pattern.
    * We will handle several cases, but only if the text contains
    * a tag like {tutu,toto.XXX} */
   if (text_entry==NULL) {
      goto no_match;
   }
   /* First, we tokenize the grammar pattern tag */
   int negation;
   struct pattern* pattern=tokenize_grammar_tag(grammar_tag->input,&negation,tilde_negation_operator);
   int ok=is_entry_compatible_with_pattern(text_entry,pattern);
   free_pattern(pattern);
   if ((ok && !negation) || (!ok && negation)) {
	   goto ok_match;
   }
   goto no_match;
}

/**************************************************
 * Last case: we want to match a non alphabetic token like "!" */
if (!u_strcmp(grammar_tag->input,text_tag->content)) {
   return OK_MATCH_STATUS;
}
return NO_MATCH_STATUS;

/* We arrive here when we have used dela_entry variables */

ok_match:
/* We test the morphological filter, if any */
if (!morphological_filter_is_ok((text_entry!=NULL)?text_entry->inflected:text_tag->content,grammar_tag,infos)) {
	goto no_match;
}
ret_value=OK_MATCH_STATUS;
goto clean;

no_match:
ret_value=NO_MATCH_STATUS;
/* This goto is useless, but there because if one day someone inserts something here...
 * Note that our friend compiler certainly removes it */
goto clean;

clean:
free_dela_entry(grammar_entry);
free_dela_entry(text_entry);
return ret_value;
}


/**
 * This function takes a tag of the form <.......> and tokenizes it.
 */
struct pattern* tokenize_grammar_tag(unichar* tag,int *negation,int tilde_negation_operator) {
(*negation)=0;
if (tag==NULL) {
   fatal_error("NULL pattern error in tokenize_grammar_tag\n");
}
if (tag[0]!='<') {
   fatal_error("ERROR: a pattern does not start with < in tokenize_grammar_tag\n");
}
int l=u_strlen(tag);
if (tag[l-1]!='>') {
   fatal_error("ERROR: a pattern does not end with > in tokenize_grammar_tag\n");
}
if (tag[1]=='!') {(*negation)=1;}
else {(*negation)=0;}
tag[l-1]='\0';
struct pattern* pattern=build_pattern(&(tag[1+(*negation)]),NULL,tilde_negation_operator);
tag[l-1]='>';
return pattern;
}


/**
 * Returns 1 if there is a space on the immediate left of the given tag.
 */
int is_space_on_the_left_in_tfst(Tfst* tfst,TfstTag* tag) {
if (tag->m.start_pos_in_char==0) {
	/* The tag starts exactly at the beginning of a token, so we just
	 * have to look if the previous token was ending with a space. By convention,
	 * we say that the token #0 has no space on its left */
	if (tag->m.start_pos_in_token==0) {
		return 0;
	}
	int size_of_previous=tfst->token_sizes->tab[tag->m.start_pos_in_token-1];
	return tfst->token_content[tag->m.start_pos_in_token-1][size_of_previous-1]==' ';
} else {
	/* The tag is in the middle of a token */
	return tfst->token_content[tag->m.start_pos_in_token][tag->m.start_pos_in_char-1]==' ';
}
}


/**
 * Tests if the text tag is compatible with the grammar tag morphological filter, if any.
 * Returns 1 in case of success (no filter is a case of success); 0 otherwise.
 */
int morphological_filter_is_ok(const unichar* content,Fst2Tag grammar_tag,const struct locate_tfst_infos* infos) {
if (grammar_tag->filter_number==-1) {
	return 1;
}
#ifdef REGEX_FACADE_ENGINE
return string_match_filter(infos->filters,content,grammar_tag->filter_number);
#else
return 0;
#endif
}

} // namespace unitex

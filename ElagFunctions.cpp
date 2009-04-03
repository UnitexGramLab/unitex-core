/*
 * Unitex
 *
 * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/* elag-functions.cpp */
/* Date         : juin 98 */
/* Auteur(s)    : MAYER Laurent et al. Olivier Blanc */
/* Objet        :  fonction principale de levee */
/*                d'ambiguite */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "Fst2Automaton.h"
#include "ElagFunctions.h"
#include "ElagFstFilesIO.h"
#include "AutConcat.h"
#include "AutDeterminization.h"
#include "AutMinimization.h"
#include "AutIntersection.h"
#include "File.h"
#include "Symbol.h"
#include "Ustring.h"


static void add_sentence_delimiters(Tfst* tfst);
static void remove_sentence_delimiters(Tfst* tfst);
vector_ptr* convert_elag_symbols_to_tfst_tags(Elag_Tfst_file_in*);

/**
 * This function loads a .tfst text automaton, disambiguates it according to the given rules,
 * and saves the result in another text automaton.
 */
void remove_ambiguities(char* input_tfst,vector_ptr* gramms,char* output) {
   static unichar _unloadable[] = { 'U', 'N', 'L', 'O', 'A', 'D', 'A', 'B', 'L', 'E', 0 };
   static unichar _rejected[] = { 'R', 'E', 'J', 'E', 'C', 'T', 'E', 'D', 0 };
   symbol_t* unloadable = new_symbol_UNKNOWN(LANGUAGE, language_add_form(LANGUAGE,_unloadable),-1);
   symbol_t* rejected = new_symbol_UNKNOWN(LANGUAGE, language_add_form(LANGUAGE,_rejected),-1);
   
   Elag_Tfst_file_in* input=load_tfst_file(input_tfst,LANGUAGE);
   if (input==NULL) {
      fatal_error("Unable to load text automaton'%s'\n",input_tfst);
   }
   error("%d sentence(s) in %s\n", input->tfst->N,input_tfst);
   
   FILE* output_tfst=u_fopen(output,U_WRITE);
   if (output_tfst==NULL) {
      fatal_error("Cannot open %s\n",output);
   }
   /* We save the number of sentence graphs */
   u_fprintf(output_tfst,"%010d\n",input->tfst->N);
   char tind[FILENAME_MAX];
   remove_extension(output,tind);
   strcat(tind,".tind");
   FILE* output_tind=fopen(tind,"wb");
   if (output_tind==NULL) {
      fatal_error("Cannot open %s\n",tind);
   }
   
   time_t start_time = time(0);
   u_printf("\nProcessing ...\n");
   int n_rejected_sentences = 0;
   int nb_unloadable = 0;
   double before, after;
   double total_before = 0.0, total_after = 0.0;
   double length_before = 0., length_after = 0.; // average text length in words
   Tfst* tfst=input->tfst;
         
   for (int current_sentence=1;current_sentence<=input->tfst->N;current_sentence++) {
      load_tfst_sentence_automaton(input,current_sentence);
      elag_determinize(tfst->automaton);
      elag_minimize(tfst->automaton);
      
      int is_rejected=0;
      if (current_sentence % 100 == 0) {
         u_printf("Sentence %d/%d...\r",current_sentence,input->tfst->N);
      }
      if (tfst->automaton->number_of_states<2) {
         /* If the sentence is empty, we replace the sentence automaton
          * by a 2-states automaton with one transition saying "UNLOADABLE" */
         error("Sentence %d is empty\n",current_sentence+1);
         free_SingleGraph(tfst->automaton);
         tfst->automaton=new_SingleGraph(2,PTR_TAGS);
         SingleGraphState initial_state=add_state(tfst->automaton);
         set_initial_state(initial_state);
         SingleGraphState final_state=add_state(tfst->automaton);
         set_final_state(final_state);
         add_outgoing_transition(initial_state,unloadable,1);
         nb_unloadable++;
      } else {
         int min,max;
         before=evaluate_ambiguity(tfst->automaton,&min,&max);
         total_before += before;
         length_before = length_before + ((double) (min + max) / (double) 2);
         add_sentence_delimiters(tfst);
         if (tfst->automaton->number_of_states<2) {
            error("Sentence %d is empty\n",current_sentence+1) ;
         } else {
            for (int j=0;j<gramms->nbelems;j++) {
               Fst2Automaton* grammar=(Fst2Automaton*)(gramms->tab[j]);
               SingleGraph temp=elag_intersection(tfst->automaton,grammar->automaton,TEXT_GRAMMAR);
               trim(temp);
               free_SingleGraph(tfst->automaton);
               tfst->automaton=temp;
               if (tfst->automaton->number_of_states<2) {
                  /* If the sentence has been rejected by the grammar */
                  error("Sentence %d rejected\n\n",current_sentence+1);
                  j=gramms->nbelems; /* We don't go on intersecting with other grammars */
                  n_rejected_sentences++;
                  free_SingleGraph(tfst->automaton);
                  tfst->automaton=new_SingleGraph(2,PTR_TAGS);
                  SingleGraphState initial_state=add_state(tfst->automaton);
                  set_initial_state(initial_state);
                  SingleGraphState final_state=add_state(tfst->automaton);
                  set_final_state(final_state);
                  add_outgoing_transition(initial_state,rejected,1);
                  is_rejected=1;
               }
            }
         }
         if (!is_rejected) {
            elag_determinize(tfst->automaton);
            trim(tfst->automaton);
            elag_minimize(tfst->automaton);
            remove_sentence_delimiters(tfst);
            after = evaluate_ambiguity(tfst->automaton,&min,&max);
            total_after += after;
            length_after = length_after + ((double) (min + max) / (double) 2);
         }
      }
      vector_ptr* new_tags=convert_elag_symbols_to_tfst_tags(input);
      save_current_sentence(input->tfst,output_tfst,output_tind,(unichar**)new_tags->tab,new_tags->nbelems);
      free_vector_ptr(new_tags,free);
   }
   u_printf("\n");
   int N=input->tfst->N;
   tfst_file_close_in(input);
   u_fclose(output_tfst);
   fclose(output_tind);
   time_t end_time = time(0);
   u_printf("\n*** Done. Result in '%s'\n",output);
   u_printf("\nElapsed time: %.0f s.\n", difftime(end_time,start_time));
   u_printf("Text. Before: %.1f, after: %.1f units per sentence. ", total_before / N, total_after / N);
   if (total_before > 0.0) {
      if (total_after / total_before > 0.01) {
         u_printf("Residual: %.0f%%.\n", 100.0 * total_after / total_before);
      } else {
         u_printf("Residual: %.1f%%.\n", 100.0 * total_after / total_before);
      }
   }
   u_printf("\n****************\n\n");
   double logITo = total_before; // logITo = log(nb of interpretation in the original text)
   double logITe = total_after; // logITe = log(nb of interpretation in the text after Elag)
   double ambrateo = exp(logITo/length_before);
   double ambratee = exp(logITe/length_after);
   u_printf("Before grammar application:\n");
   u_printf(
         "log(|Int(Torig)|) = %.1f (%.0f interpretations, %.1f interp. per sentence)\n",
         logITo, exp(logITo), exp(logITo/(double) N));
   u_printf(
         "Corpus length = %.1f (%.1f lexems per sentence)\naverage ambiguity rate: %.3f\n",
         length_before, length_before / (double) N, ambrateo);
   u_printf("\nAfter grammar application:\n");
   u_printf(
         "log(|Int(Telag)|) = %.1f (%.0f interpretations, %.1f interp. per sentence)\n",
         logITe, exp(logITe), exp(logITe/(double) N));
   u_printf("Corpus length = %.1f (%.1f lexems per sentence)\n"
            "Average ambiguity rate: %.3f\n", length_after, length_before / (double) N,
                                                 ambratee);
   u_printf("\nResidual: %.2f %%.\n", (ambratee / ambrateo) * 100.);
   u_printf("(Other residual: %.5f %% of residual ambig.)\n", exp(logITe
         - logITo) * (double) 100.);
   u_printf(
         "\n%d sentences, %d not successfully loaded and %d rejected by elag grammars.\n\n",
         N, nb_unloadable, n_rejected_sentences);
   free_symbol(unloadable);
   free_symbol(rejected);
}



/**
 * This function loads a .tfst text automaton, and saves the result in another text automaton.
 * The desired side effect is to explode tags. For instance, a transition tagged like:
 * 
 *    {jeunes,jeune.A:mp:fp}
 * 
 * will be replaced by two transitions tagged like:
 * 
 *    {jeunes,jeune.A:mp}
 *    {jeunes,jeune.A:fp}
 */
void explode_tfst(char* input_tfst,char* output) {
   static unichar _unloadable[] = { 'U', 'N', 'L', 'O', 'A', 'D', 'A', 'B', 'L', 'E', 0 };
   static unichar _rejected[] = { 'R', 'E', 'J', 'E', 'C', 'T', 'E', 'D', 0 };
   symbol_t* unloadable = new_symbol_UNKNOWN(LANGUAGE, language_add_form(LANGUAGE,_unloadable),-1);
   symbol_t* rejected = new_symbol_UNKNOWN(LANGUAGE, language_add_form(LANGUAGE,_rejected),-1);
   
   Elag_Tfst_file_in* input=load_tfst_file(input_tfst,LANGUAGE);
   if (input==NULL) {
      fatal_error("Unable to load text automaton'%s'\n",input_tfst);
   }
   FILE* output_tfst=u_fopen(output,U_WRITE);
   if (output_tfst==NULL) {
      fatal_error("Cannot open %s\n",output);
   }
   /* We save the number of sentence graphs */
   u_fprintf(output_tfst,"%010d\n",input->tfst->N);
   char tind[FILENAME_MAX];
   remove_extension(output,tind);
   strcat(tind,".tind");
   FILE* output_tind=fopen(tind,"wb");
   if (output_tind==NULL) {
      fatal_error("Cannot open %s\n",tind);
   }
   u_printf("\nProcessing ...\n");
   Tfst* tfst=input->tfst;
         
   for (int current_sentence=1;current_sentence<=input->tfst->N;current_sentence++) {
      load_tfst_sentence_automaton(input,current_sentence);
      elag_determinize(tfst->automaton);
      elag_minimize(tfst->automaton);
      if (current_sentence % 100 == 0) {
         u_printf("Sentence %d/%d...\r",current_sentence,input->tfst->N);
      }
      vector_ptr* new_tags=convert_elag_symbols_to_tfst_tags(input);
      save_current_sentence(input->tfst,output_tfst,output_tind,(unichar**)new_tags->tab,new_tags->nbelems);
      free_vector_ptr(new_tags,free);
   }
   tfst_file_close_in(input);
   u_fclose(output_tfst);
   fclose(output_tind);
   free_symbol(unloadable);
   free_symbol(rejected);
}



/**
 * Loads all the ELAG grammars contained in the given .elg file.
 */
vector_ptr* load_elag_grammars(char* filename) {
FILE* f=fopen(filename,"r");
if (f==NULL) {
   error("Cannot open file %s\n",filename);
   return NULL;
}
vector_ptr* grammars=new_vector_ptr(16);
char buf[FILENAME_MAX];
while (fgets(buf,FILENAME_MAX,f) != NULL) {
   if (*buf != '<') {
      continue;
   }
   char* p=strchr(buf,'>');
   if (p==NULL) {
      error("In %s: at line '%s': delimitor '>' not found\n",filename,buf);
      free_vector_ptr(grammars,(release_f)free_Fst2Automaton);
      return NULL;
   }
   (*p)='\0';
   u_printf("\nLoadding %s...\n",buf+1);
   Fst2Automaton* A=load_elag_grammar_automaton(buf+1,LANGUAGE);
   if (A==NULL) {
      error("Unable to load '%s' automaton\n",buf+1);
      free_vector_ptr(grammars,(release_f)free_Fst2Automaton);
      return NULL;   
   }
   vector_ptr_add(grammars,A);
}
u_printf("%d grammar(s) loaded\n",grammars->nbelems);
return grammars;
}



/**
 * Adds {S} at the beginning and end of the sentence automaton.
 */
static void add_sentence_delimiters(Tfst* tfst) {
static unichar S[] = { '{', 'S', '}', 0 };
int idx=language_add_form(LANGUAGE,S);
symbol_t* delimiter=new_symbol_PUNC(LANGUAGE,idx,-1);
int pseudo_initial_state_index=tfst->automaton->number_of_states;
SingleGraphState pseudo_initial_state=add_state(tfst->automaton);
int new_final_state_index=tfst->automaton->number_of_states;
SingleGraphState new_final_state=add_state(tfst->automaton);
set_final_state(new_final_state);
if (!is_initial_state(tfst->automaton->states[0])) {
   fatal_error("add_sentence_delimiter: state #0 is not initial\n");
}
pseudo_initial_state->outgoing_transitions=tfst->automaton->states[0]->outgoing_transitions;
tfst->automaton->states[0]->outgoing_transitions=NULL;
add_outgoing_transition(tfst->automaton->states[0],delimiter,pseudo_initial_state_index);
if (is_final_state(tfst->automaton->states[0])) {
   set_final_state(pseudo_initial_state);
   unset_initial_state(tfst->automaton->states[0]);
}
for (int q=1;q<tfst->automaton->number_of_states-2;q++) {
   if (is_final_state(tfst->automaton->states[q])) {
      add_outgoing_transition(tfst->automaton->states[q],delimiter,new_final_state_index);
      unset_initial_state(tfst->automaton->states[q]);
   }
}
free_symbol(delimiter);
}


/**
 * Removes a transition to state #n from the given list, if any.
 * If a transition to state #n is removed, then *flag is set to 1.  
 */
Transition* remove_transition_to_state(int n,Transition* list,int *flag) {
if (list==NULL) return NULL;
if (list->state_number==n) {
   (*flag)=1;
   Transition* tmp=list->next;
   list->next=NULL;
   free_Transition(list);
   return tmp;
}
list->next=remove_transition_to_state(n,list->next,flag);
return list;
}


/**
 * Removes the {S} delimiters previously added by add_sentence_delimiters.
 */
static void remove_sentence_delimiters(Tfst* tfst) {
topological_sort(tfst->automaton);
if (tfst->automaton->number_of_states<4 || get_initial_state(tfst->automaton)!=0
    || tfst->automaton->states[0]->outgoing_transitions==NULL 
    || tfst->automaton->states[0]->outgoing_transitions->next!=NULL
    || !is_final_state(tfst->automaton->states[tfst->automaton->number_of_states-1])
    || tfst->automaton->states[tfst->automaton->number_of_states-1]->outgoing_transitions!=NULL) {
   /* If the automaton does not start and ends with a {S} transition,
    * it's an error */
   fatal_error("remove_sentence_delimiters: bad automaton\n");
}
if (u_strcmp(language_get_form(tfst->automaton->states[0]->outgoing_transitions->label->lemma),"{S}")) {
   fatal_error("remove_sentence_delimiters: no sentence delimiter found\n");
}
#warning we could do the same at a lower cost by shifting the 1->N-2 states to 0->N-3
unset_initial_state(tfst->automaton->states[0]);
free_Transition_list(tfst->automaton->states[0]->outgoing_transitions);
tfst->automaton->states[0]->outgoing_transitions=NULL;
set_initial_state(tfst->automaton->states[1]);
int final_state_index=tfst->automaton->number_of_states-1;
for (int q=1;q<final_state_index;q++) {
   SingleGraphState s=tfst->automaton->states[q];
   int flag=0;
   s->outgoing_transitions=remove_transition_to_state(final_state_index,s->outgoing_transitions,&flag);
   if (flag) {
      set_final_state(s);
   }
}
unset_final_state(tfst->automaton->states[final_state_index]);
trim(tfst->automaton);
topological_sort(tfst->automaton);
}


/**
 * Stores in ustr a string representing the given symbol tag.
 * foo_tag is used to avoid recreating to many temp TfstTag.
 * The same for foo_content
 */
void symbol_to_tfst_tag(symbol_t* symbol,TfstTag* original_tag,
                        TfstTag* foo_tag,Ustring* foo_content,unichar* out) {
if (symbol->type==EPSILON) {
   u_strcpy(out,"@<E>\n.\n");
   return;
}
symbol_to_text_label(symbol,foo_content);
foo_tag->content=foo_content->str;
foo_tag->start_pos_token=original_tag->start_pos_token;
foo_tag->start_pos_char=original_tag->start_pos_char;
foo_tag->end_pos_token=original_tag->end_pos_token;
foo_tag->end_pos_char=original_tag->end_pos_char;
TfstTag_to_string(foo_tag,out);
foo_tag->content=NULL;
}


/**
 * Inserts if necessary the given tag in the given vector. Returns
 * the tag's index.
 */
int insert_tag(vector_ptr* tags,unichar* tag) {
for (int i=0;i<tags->nbelems;i++) {
   if (!u_strcmp((unichar*)tags->tab[i],tag)) return i;
}
return vector_ptr_add(tags,u_strdup(tag));
}


/**
 * We have worked with symbol_t* and now, we want to replace them with 
 * tfst tag strings like "@STD\n@{fait,faire.V:P3s:Kms}\n@2-2\n.\n"
 * We replace symbol_t* by integers that are indexes in the vector we return. 
 */
vector_ptr* convert_elag_symbols_to_tfst_tags(Elag_Tfst_file_in* input) {
/* We change the tag type */
input->tfst->automaton->tag_type=INT_TAGS;
vector_ptr* tags=new_vector_ptr(16);
SingleGraph automaton=input->tfst->automaton;
unichar tmp[4096];
TfstTag* foo_tag=new_TfstTag(T_STD);
Ustring* foo_content=new_Ustring(256);
/* First, we insert the <E> tag that must always be the #0 one */
vector_ptr_add(tags,u_strdup("@<E>\n.\n"));
for (int i=0;i<automaton->number_of_states;i++) {
   Transition* t=automaton->states[i]->outgoing_transitions;
   while (t!=NULL) {
      symbol_t* symbol=t->label;
      symbol_to_text_label(symbol,foo_content);
      if (symbol->tfsttag_index==-1) {
         fatal_error("Internal error in convert_elag_symbols_to_tfst_tags: unexpected -1 tag index\n");
      }
      TfstTag* original_tag=(TfstTag*)input->tfst->tags->tab[symbol->tfsttag_index];
      symbol_to_tfst_tag(symbol,original_tag,foo_tag,foo_content,tmp);
      t->tag_number=insert_tag(tags,tmp);
      t=t->next;
   }
}
free_Ustring(foo_content);
free_TfstTag(foo_tag);
return tags;
}



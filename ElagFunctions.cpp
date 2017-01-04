/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
/* Objet        : fonction principale de levee */
/*                d'ambiguite */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "Fst2Automaton.h"
#include "HashTable.h"
#include "ElagFunctions.h"
#include "ElagFstFilesIO.h"
#include "AutConcat.h"
#include "AutDeterminization.h"
#include "AutMinimization.h"
#include "AutIntersection.h"
#include "File.h"
#include "Symbol.h"
#include "Ustring.h"
#include "TfstStats.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

static void add_sentence_delimiters(Tfst* tfst,language_t*);
static void remove_sentence_delimiters(Tfst* tfst,language_t*);
vector_ptr* convert_elag_symbols_to_tfst_tags(Elag_Tfst_file_in*);


/**
 * This function loads a .tfst text automaton, disambiguates it according to the given rules,
 * and saves the result in another text automaton.
 */
void remove_ambiguities(const char* input_tfst,vector_ptr* gramms,const char* output, const VersatileEncodingConfig* vec,language_t* language,int save_statistics) {
   Elag_Tfst_file_in* input=load_tfst_file(vec,input_tfst,language);
   if (input==NULL) {
      fatal_error("Unable to load text automaton'%s'\n",input_tfst);
   }
   error("%d sentence(s) in %s\n", input->tfst->N,input_tfst);

   U_FILE* output_tfst=u_fopen(vec,output,U_WRITE);
   if (output_tfst==NULL) {
      fatal_error("Cannot open %s\n",output);
   }
   /* We save the number of sentence graphs */
   u_fprintf(output_tfst,"%010d\n",input->tfst->N);
   char tind[FILENAME_MAX];
   remove_extension(output,tind);
   strcat(tind,".tind");
   U_FILE* output_tind=u_fopen(BINARY,tind,U_WRITE);
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

   /* We use this hash table to rebuild files tfst_tags_by_freq/alph.txt */
   hash_table* form_frequencies=new_hash_table((HASH_FUNCTION)hash_unichar,(EQUAL_FUNCTION)u_equal,
           (FREE_FUNCTION)free,NULL,(KEYCOPY_FUNCTION)keycopy);

   for (int current_sentence=1;current_sentence<=input->tfst->N;current_sentence++) {
      load_tfst_sentence_automaton(input,current_sentence);
      elag_determinize(language,tfst->automaton,free_symbol);
      elag_minimize(tfst->automaton);
      int is_rejected=0;
      if (current_sentence % 100 == 0) {
         u_printf("Sentence %d/%d...\r",current_sentence,input->tfst->N);
      }
      u_printf("Sentence %d\n",current_sentence);
      if (tfst->automaton->number_of_states<2) {
         /* If the sentence is empty, we replace the sentence automaton
          * by a 1-state automaton with no transition. */
         error("Sentence %d is empty\n",current_sentence);
         free_SingleGraph(tfst->automaton,free_symbol);
         tfst->automaton=new_SingleGraph(1,PTR_TAGS);
         SingleGraphState initial_state=add_state(tfst->automaton);
         set_initial_state(initial_state);
         nb_unloadable++;
      } else {
         int min,max;
         before=evaluate_ambiguity(tfst->automaton,&min,&max);
         total_before += before;
         length_before = length_before + ((double) (min + max) / (double) 2);
         add_sentence_delimiters(tfst,language);
         if (tfst->automaton->number_of_states<2) {
            error("Sentence %d is empty\n",current_sentence) ;
         } else {
            for (int j=0;j<gramms->nbelems;j++) {
               Fst2Automaton* grammar=(Fst2Automaton*)(gramms->tab[j]);
               SingleGraph temp=elag_intersection(language,tfst->automaton,grammar->automaton,TEXT_GRAMMAR);
               trim(temp,free_symbol);
               free_SingleGraph(tfst->automaton,free_symbol);
               tfst->automaton=temp;
               if (tfst->automaton->number_of_states<2) {
                  /* If the sentence has been rejected by the grammar */
                  error("Sentence %d rejected\n\n",current_sentence);
                  j=gramms->nbelems; /* We don't go on intersecting with other grammars */
                  n_rejected_sentences++;
                  free_SingleGraph(tfst->automaton,free_symbol);
                  tfst->automaton=new_SingleGraph(1,PTR_TAGS);
                  SingleGraphState initial_state=add_state(tfst->automaton);
                  set_initial_state(initial_state);
                  is_rejected=1;
                  break;
               }
            }
         }
         if (!is_rejected) {
            elag_determinize(language,tfst->automaton,free_symbol);
            trim(tfst->automaton,free_symbol);
            elag_minimize(tfst->automaton);
            remove_sentence_delimiters(tfst,language);
            after = evaluate_ambiguity(tfst->automaton,&min,&max);
            total_after += after;
            length_after = length_after + ((double) (min + max) / (double) 2);
         }
      }
      vector_ptr* new_tags=convert_elag_symbols_to_tfst_tags(input);
      save_current_sentence(input->tfst,output_tfst,output_tind,(unichar**)new_tags->tab,new_tags->nbelems,form_frequencies);
      free_vector_ptr(new_tags,free);
      free_SingleGraph(tfst->automaton,NULL);
      tfst->automaton=NULL;
   }
   u_printf("\n");
   int N=input->tfst->N;
   tfst_file_close_in(input);
   u_fclose(output_tfst);
   u_fclose(output_tind);
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

   /* Finally, we save statistics */
   if (save_statistics) {
       char tfst_tags_by_freq[FILENAME_MAX];
       char tfst_tags_by_alph[FILENAME_MAX];
       get_path(input_tfst, tfst_tags_by_freq);
       strcat(tfst_tags_by_freq, "tfst_tags_by_freq.new.txt");
       get_path(input_tfst, tfst_tags_by_alph);
       strcat(tfst_tags_by_alph, "tfst_tags_by_alph.new.txt");
       U_FILE* f_tfst_tags_by_freq = u_fopen(vec, tfst_tags_by_freq, U_WRITE);
       if (f_tfst_tags_by_freq == NULL) {
           error("Cannot open %s\n", tfst_tags_by_freq);
       }
       U_FILE* f_tfst_tags_by_alph = u_fopen(vec, tfst_tags_by_alph, U_WRITE);
       if (f_tfst_tags_by_alph == NULL) {
           error("Cannot open %s\n", tfst_tags_by_alph);
       }
       sort_and_save_tfst_stats(form_frequencies, f_tfst_tags_by_freq, f_tfst_tags_by_alph);
       u_fclose(f_tfst_tags_by_freq);
       u_fclose(f_tfst_tags_by_alph);
   }
   free_hash_table(form_frequencies);
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
 *
 * The 'elag_fst' parameter is used to know whether the form frequencies files must be named
 * tfst_tags_by_freq/alph.txt or tfst_tags_by_freq/alph.new.txt
 */
void explode_tfst(const char* input_tfst,const char* output, const VersatileEncodingConfig* vec,language_t* language,
        hash_table* form_frequencies) {
   static const unichar _unloadable[] = { 'U', 'N', 'L', 'O', 'A', 'D', 'A', 'B', 'L', 'E', 0 };
   static const unichar _rejected[] = { 'R', 'E', 'J', 'E', 'C', 'T', 'E', 'D', 0 };
   symbol_t* unloadable = new_symbol_UNKNOWN(language, language_add_form(language,_unloadable),-1);
   symbol_t* rejected = new_symbol_UNKNOWN(language, language_add_form(language,_rejected),-1);

   Elag_Tfst_file_in* input=load_tfst_file(vec,input_tfst,language);
   if (input==NULL) {
      fatal_error("Unable to load text automaton'%s'\n",input_tfst);
   }
   U_FILE* output_tfst=u_fopen(vec,output,U_WRITE);
   if (output_tfst==NULL) {
      fatal_error("Cannot open %s\n",output);
   }
   /* We save the number of sentence graphs */
   u_fprintf(output_tfst,"%010d\n",input->tfst->N);
   char tind[FILENAME_MAX];
   remove_extension(output,tind);
   strcat(tind,".tind");
   U_FILE* output_tind=u_fopen(BINARY,tind,U_WRITE);
   if (output_tind==NULL) {
      fatal_error("Cannot open %s\n",tind);
   }
   u_printf("\nProcessing ...\n");
   Tfst* tfst=input->tfst;

   for (int current_sentence=1;current_sentence<=input->tfst->N;current_sentence++) {
      load_tfst_sentence_automaton(input,current_sentence);
      elag_determinize(language,tfst->automaton,free_symbol);
      elag_minimize(tfst->automaton);
      if (current_sentence % 100 == 0) {
         u_printf("Sentence %d/%d...\r",current_sentence,input->tfst->N);
      }
      vector_ptr* new_tags=convert_elag_symbols_to_tfst_tags(input);
      save_current_sentence(input->tfst,output_tfst,output_tind,(unichar**)new_tags->tab,new_tags->nbelems,
                                 form_frequencies);
      free_vector_ptr(new_tags,free);
   }
   tfst_file_close_in(input);
   u_fclose(output_tfst);
   u_fclose(output_tind);
   free_symbol(unloadable);
   free_symbol(rejected);
}



/**
 * Loads all the ELAG grammars contained in the given .elg file.
 */
vector_ptr* load_elag_grammars(const VersatileEncodingConfig* vec,const char* filename,language_t* language,const char* directory) {
U_FILE* f=u_fopen(ASCII,filename,U_READ);
if (f==NULL) {
   error("Cannot open file %s\n",filename);
   return NULL;
}
vector_ptr* grammars=new_vector_ptr(16);
char buf[FILENAME_MAX];
char buf2[FILENAME_MAX];
while (af_fgets(buf,FILENAME_MAX,f->f) != NULL) {
   if (*buf != '<') {
      continue;
   }
   char* p=strchr(buf,'>');
   if (p==NULL) {
      error("In %s: at line '%s': delimiter '>' not found\n",filename,buf);
      free_vector_ptr(grammars,(release_f)free_Fst2Automaton_excluding_symbols);
      u_fclose(f);
      return NULL;
   }
   (*p)='\0';
   u_printf("\nLoading %s...\n",buf+1);

   if (is_absolute_path(buf+1)) {
      strcpy(buf2,buf+1);
   } else {
      sprintf(buf2,"%s%s",directory,buf+1);
   }
   Fst2Automaton* A=load_elag_grammar_automaton(vec,buf2,language);
   if (A==NULL) {
      error("Unable to load '%s' automaton\n",buf2);
      free_vector_ptr(grammars,(release_f)free_Fst2Automaton_excluding_symbols);
      u_fclose(f);
      return NULL;
   }
   vector_ptr_add(grammars,A);
}
u_printf("%d grammar(s) loaded\n",grammars->nbelems);
u_fclose(f);
return grammars;
}



/**
 * Adds {S} at the beginning and end of the sentence automaton.
 */
static void add_sentence_delimiters(Tfst* tfst,language_t* language) {
static const unichar S[] = { '{', 'S', '}', 0 };
int idx=language_add_form(language,S);
symbol_t* delimiter=new_symbol_PUNC(language,idx,-1);
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
      unset_final_state(tfst->automaton->states[q]);
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
   free_Transition(list,free_symbol);
   return tmp;
}
list->next=remove_transition_to_state(n,list->next,flag);
return list;
}


/**
 * Removes the {S} delimiters previously added by add_sentence_delimiters.
 */
static void remove_sentence_delimiters(Tfst* tfst,language_t* language) {
topological_sort(tfst->automaton,free_symbol);
if (tfst->automaton->number_of_states<4 || get_initial_state(tfst->automaton)!=0
    || tfst->automaton->states[0]->outgoing_transitions==NULL
    || tfst->automaton->states[0]->outgoing_transitions->next!=NULL
    || !is_final_state(tfst->automaton->states[tfst->automaton->number_of_states-1])
    || tfst->automaton->states[tfst->automaton->number_of_states-1]->outgoing_transitions!=NULL) {
   /* If the automaton does not start and ends with a {S} transition,
    * it's an error */
   fatal_error("remove_sentence_delimiters: bad automaton\n");
}
if (u_strcmp(language_get_form(language,tfst->automaton->states[0]->outgoing_transitions->label->lemma),"{S}")) {
   fatal_error("remove_sentence_delimiters: no sentence delimiter found\n");
}


#ifdef REMINDER_WARNING
#ifdef __GNUC__
#warning we could do the same at a lower cost by shifting the 1->N-2 states to 0->N-3
#elif ((defined(__VISUALC__)) || defined(_MSC_VER))
#pragma message("warning : we could do the same at a lower cost by shifting the 1->N-2 states to 0->N-3")
#endif
#endif


unset_initial_state(tfst->automaton->states[0]);
free_Transition_list(tfst->automaton->states[0]->outgoing_transitions,free_symbol);
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
trim(tfst->automaton,free_symbol);
topological_sort(tfst->automaton,free_symbol);
}


/**
 * Stores in ustr a string representing the given symbol tag.
 * foo_tag is used to avoid recreating to many temp TfstTag.
 * The same for foo_content
 */
void symbol_to_tfst_tag(symbol_t* symbol,TfstTag* original_tag,
                        TfstTag* foo_tag,Ustring* foo_content,unichar* out) {
if (symbol->type==S_EPSILON) {
   u_strcpy(out,"@<E>\n.\n");
   return;
}
symbol_to_text_label(symbol,foo_content);
foo_tag->content=foo_content->str;
foo_tag->m=original_tag->m;
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
         fatal_error("Internal error in convert_elag_symbols_to_tfst_tags: unexpected -1 tag index for this tag:\n%S\n",foo_content->str);
      }
      TfstTag* original_tag=(TfstTag*)input->tfst->tags->tab[symbol->tfsttag_index];
      symbol_to_tfst_tag(symbol,original_tag,foo_tag,foo_content,tmp);
      t->tag_number=insert_tag(tags,tmp);
      t=t->next;
      free_symbol(symbol);
   }
}
free_Ustring(foo_content);
free_TfstTag(foo_tag);
return tags;
}

} // namespace unitex


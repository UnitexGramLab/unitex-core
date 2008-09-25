/*
 * Unitex
 *
 * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "utils.h"
#include "autalmot.h"
#include "list_aut.h"
#include "elag-functions.h"
#include "ElagFstFilesIO.h"
#include "AutConcat.h"
#include "AutDeterminization.h"
#include "AutMinimization.h"
#include "AutIntersection.h"

double eval_sentence(Fst2Automaton * A, int * min = NULL, int * max = NULL);
static void add_sentence_delimiters(Fst2Automaton * A);
static void remove_sentence_delimiters(Fst2Automaton * A);

//void leve_ambiguite(char * nom_fic_phrases, list_aut * gramm, char * nomSortie) {

void remove_ambiguities(char * fstname, list_aut * gramms, char * outname) {
   static unichar _unloadable[] = { 'U', 'N', 'L', 'O', 'A', 'D', 'A', 'B', 'L', 'E', 0 };
   static unichar _rejected[] = { 'R', 'E', 'J', 'E', 'C', 'T', 'E', 'D', 0 };
   symbol_t * unloadable = new_symbol_UNKNOWN(LANGUAGE, language_add_form(LANGUAGE,_unloadable));
   symbol_t * rejected = new_symbol_UNKNOWN(LANGUAGE, language_add_form(LANGUAGE,_rejected));
   Elag_fst_file_in * txtin = load_fst_file(fstname, FST_TEXT, LANGUAGE);
   if (txtin == NULL) {
      fatal_error("unable to load text '%s'\n", fstname);
   }
   error("%d sentence(s) in %s\n", txtin->nb_automata, fstname);
   Elag_fst_file_out * fstout = fst_file_out_open(outname, FST_TEXT);
   if (fstout == NULL) {
      fatal_error("unable to open '%s' for writing\n", outname);
   }
   time_t start_time = time(0);
   u_printf("\nProcessing ...\n");
   int current_sentence = 0;
   int n_rejected_sentences = 0;
   int nb_unloadable = 0;
   Fst2Automaton* A;
   double before, after;
   double total_before = 0.0, total_after = 0.0;
   double length_before = 0., length_after = 0.; // average text length in words

   while ((A = load_automaton(txtin)) != NULL) {
      elag_determinize(A->automaton);
      elag_minimize(A->automaton);
      
      int is_rejected=0;
      if (current_sentence % 100 == 0) {
         u_printf("Sentence %d/%d...\r",current_sentence+1,txtin->nb_automata);
      }
      if (A->automaton->number_of_states<2) {
         /* If the sentence is empty, we replace the sentence automaton
          * by a 2-states automaton with one transition saying "UNLOADABLE" */
         error("Sentence %d is empty\n",current_sentence+1);
         free_SingleGraph(A->automaton);
         A->automaton=new_SingleGraph(2,PTR_TAGS);
         SingleGraphState initial_state=add_state(A->automaton);
         set_initial_state(initial_state);
         SingleGraphState final_state=add_state(A->automaton);
         set_final_state(final_state);
         add_outgoing_transition(initial_state,unloadable,1);
         nb_unloadable++;
      } else {
         int min,max;
         before=evaluate_ambiguity(A->automaton,&min,&max);
         total_before += before;
         length_before = length_before + ((double) (min + max) / (double) 2);
         add_sentence_delimiters(A);
         if (A->automaton->number_of_states<2) {
            error("Sentence %d is empty\n",current_sentence+1) ;
         } else {
            for (int j=0;j<gramms->nbelems;j++) {
               Fst2Automaton* grammar=(Fst2Automaton*)(gramms->tab[j]);
               SingleGraph temp=elag_intersection(A->automaton,grammar->automaton);
               trim(temp);
               free_SingleGraph(A->automaton);
               A->automaton=temp;
               if (A->automaton->number_of_states<2) {
                  /* If the sentence has been rejected by the grammar */
                  error("Sentence %d rejected\n\n",current_sentence+1);
                  j=gramms->nbelems; /* We don't go on intersecting with other grammars */
                  n_rejected_sentences++;
                  free_SingleGraph(A->automaton);
                  A->automaton=new_SingleGraph(2,PTR_TAGS);
                  SingleGraphState initial_state=add_state(A->automaton);
                  set_initial_state(initial_state);
                  SingleGraphState final_state=add_state(A->automaton);
                  set_final_state(final_state);
                  add_outgoing_transition(initial_state,rejected,1);
                  is_rejected=1;
               }
            }
         }
         if (!is_rejected) {
            elag_determinize(A->automaton);
            trim(A->automaton);
            elag_minimize(A->automaton);
            remove_sentence_delimiters(A);
            after = evaluate_ambiguity(A->automaton,&min,&max);
            total_after += after;
            length_after = length_after + ((double) (min + max) / (double) 2);
         }
      }
      fst_file_write(fstout, A);
      free_Fst2Automaton(A);
      current_sentence++;
   }
   u_printf("\n");
   fst_file_close_in(txtin);
   fst_file_close_out(fstout);
   time_t fin = time(0);
   u_printf("\n*** Done. Result in '%s'\n", outname);
   u_printf("\nElapsed time: %.0f s.\n", difftime(fin, start_time));
   u_printf("Text. Before: %.1f, after: %.1f units per sentence. ", total_before / current_sentence, total_after / current_sentence);
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
         logITo, exp(logITo), exp(logITo/(double) current_sentence));
   u_printf(
         "Corpus length = %.1f (%.1f lexems per sentence)\naverage ambiguity rate: %.3f\n",
         length_before, length_before / (double) current_sentence, ambrateo);
   u_printf("\nAfter grammar application:\n");
   u_printf(
         "log(|Int(Telag)|) = %.1f (%.0f interpretations, %.1f interp. per sentence)\n",
         logITe, exp(logITe), exp(logITe/(double) current_sentence));
   u_printf("Corpus length = %.1f (%.1f lexems per sentence)\n"
            "Average ambiguity rate: %.3f\n", length_after, length_before / (double) current_sentence,
                                                 ambratee);
   u_printf("\nResidual: %.2f %%.\n", (ambratee / ambrateo) * 100.);
   u_printf("(Other residual: %.5f %% of residual ambig.)\n", exp(logITe
         - logITo) * (double) 100.);
   u_printf(
         "\n%d sentences, %d not successfully loaded and %d rejected by elag grammars.\n\n",
         current_sentence, nb_unloadable, n_rejected_sentences);
   free_symbol(unloadable);
   free_symbol(rejected);
}




/* Charge les grammaires deja compilees. */

list_aut * chargeGramm(char * nomFichGramm) {

   char fname[strlen(nomFichGramm) + 5];

   strcpy(fname, nomFichGramm);

   if (strcmp(fname + strlen(fname) - 4, ".rul") != 0) {
      strcat(fname, ".rul");
   }

   FILE * fGramm = fopen(fname, "r");

   if (!fGramm) {
      fatal_error("opening file %s\n", fname);
   }

   list_aut * gramms = list_aut_new();

   char buf[FILENAME_MAX];

   while (fgets(buf, FILENAME_MAX, fGramm) != NULL) {

      if (*buf != '<') {
         continue;
      }

      char * p = strchr(buf, '>');

      if (p == NULL) {
         fatal_error("in %s: at line '%s': delimitor '>' not found\n",
               nomFichGramm, buf);
      }

      *p = 0;

      error("\nReading %s...\n", buf + 1);

      Fst2Automaton * A = load_elag_grammar_automaton(buf + 1, LANGUAGE);

      if (A == NULL) {
         fatal_error("unable to load '%s' automaton\n", buf + 1);
      }

      list_aut_add(gramms, A);
   }

   error("%d gramm(s) loaded\n", gramms->nbelems);

   return gramms;
}

list_aut * chargeUneGramm(char * name) {
   fatal_error("charhgeUneGramm: not implemented\n");
   return NULL;
}


/**
 * Adds {S} at the beginning and end of the sentence automaton.
 */
static void add_sentence_delimiters(Fst2Automaton* A) {
static unichar S[] = { '{', 'S', '}', 0 };
int idx=language_add_form(LANGUAGE,S);
symbol_t* delimiter=new_symbol_PUNC(LANGUAGE,idx);
int pseudo_initial_state_index=A->automaton->number_of_states;
SingleGraphState pseudo_initial_state=add_state(A->automaton);
int new_final_state_index=A->automaton->number_of_states;
SingleGraphState new_final_state=add_state(A->automaton);
set_final_state(new_final_state);
if (!is_initial_state(A->automaton->states[0])) {
   fatal_error("add_sentence_delimiter: state #0 is not initial\n");
}
pseudo_initial_state->outgoing_transitions=A->automaton->states[0]->outgoing_transitions;
A->automaton->states[0]->outgoing_transitions=NULL;
add_outgoing_transition(A->automaton->states[0],delimiter,pseudo_initial_state_index);
if (is_final_state(A->automaton->states[0])) {
   set_final_state(pseudo_initial_state);
   unset_initial_state(A->automaton->states[0]);
}
for (int q=1;q<A->automaton->number_of_states-2;q++) {
   if (is_final_state(A->automaton->states[q])) {
      add_outgoing_transition(A->automaton->states[q],delimiter,new_final_state_index);
      unset_initial_state(A->automaton->states[q]);
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
static void remove_sentence_delimiters(Fst2Automaton* A) {
topological_sort(A->automaton);
if (A->automaton->number_of_states<4 || get_initial_state(A->automaton)!=0
    || A->automaton->states[0]->outgoing_transitions==NULL 
    || A->automaton->states[0]->outgoing_transitions->next!=NULL
    || !is_final_state(A->automaton->states[A->automaton->number_of_states-1])
    || A->automaton->states[A->automaton->number_of_states-1]->outgoing_transitions!=NULL) {
   /* If the automaton does not start and ends with a {S} transition,
    * it's an error */
   fatal_error("remove_sentence_delimiters: bad automaton\n");
}
if (u_strcmp(language_get_form(A->automaton->states[0]->outgoing_transitions->label->lemma),"{S}")) {
   fatal_error("remove_sentence_delimiters: no sentence delimiter found\n");
}
#warning we could do the same at a lower cost by shifting the 1->N-2 states to 0->N-3
unset_initial_state(A->automaton->states[0]);
free_Transition_list(A->automaton->states[0]->outgoing_transitions);
set_initial_state(A->automaton->states[1]);
int final_state_index=A->automaton->number_of_states-1;
for (int q=1;q<final_state_index;q++) {
   SingleGraphState s=A->automaton->states[q];
   int flag=0;
   s->outgoing_transitions=remove_transition_to_state(final_state_index,s->outgoing_transitions,&flag);
   if (flag) {
      set_final_state(s);
   }
}
unset_final_state(A->automaton->states[final_state_index]);
trim(A->automaton);
topological_sort(A->automaton);
}


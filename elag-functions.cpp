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
#include "fst_file.h"


double eval_sentence(autalmot_t * A, int * min = NULL, int * max = NULL);
static void add_limphrase(autalmot_t * A);
static int suppress_limphrase(autalmot_t * A);


//void leve_ambiguite(char * nom_fic_phrases, list_aut * gramm, char * nomSortie) {

void leve_ambiguite(char * fstname, list_aut * gramms, char * outname) {

  static unichar _unloadable[] = { 'U', 'N', 'L', 'O', 'A', 'D', 'A', 'B', 'L', 'E', 0 };
  static unichar _rejected[]   = { 'R', 'E', 'J', 'E', 'C', 'T', 'E', 'D', 0 };

  symbol_t * unloadable = symbol_unknow_new(LANG, language_add_form(LANG, _unloadable));
  symbol_t * rejected   = symbol_unknow_new(LANG, language_add_form(LANG, _rejected));
  
  u_printf("\n* leve ambiguite(%s): %d grammar%s.\n", fstname, gramms->nbelems,
         gramms->nbelems > 1 ?  "s" : "");

  fst_file_in_t * txtin = fst_file_in_open(fstname, FST_TEXT);

  if (txtin == NULL) { fatal_error("unable to load text '%s'\n", fstname); }


  error("%d sentence(s) in %s\n", txtin->nbelems, fstname);


  fst_file_out_t * fstout = fst_file_out_open(outname, FST_TEXT);

  if (fstout == NULL) { fatal_error("unable to open '%s' for writing\n", outname); }


  time_t debut = time(0);

  u_printf("\nprocessing ...\n");

  int no = 0;       // numero de la phrase courante
  int nbPhrRej = 0; // nombre de phrases rejetées
  int nb_unloadable = 0; // nombre de phrases qui n'ont pas pu etre chargees

  autalmot_t * A;

  double avant, apres;
  double cumulavant = 0.0, cumulapres = 0.0;
  double lgavant = 0., lgapres = 0.; // longueur moyenne du texte (en mots)

  while ((A = fst_file_autalmot_load_next(txtin)) != NULL) {

    int isrej = 0;

    autalmot_t * orig = autalmot_dup(A);

    if (no % 100 == 0) { u_printf("sentence %d/%d ...\r", no + 1, txtin->nbelems); }

    autalmot_determinize(A);
    autalmot_emonde(A);
    autalmot_minimize(A);

    if (A->nbstates < 2) {

      error("sentence %d is empty.\n", no + 1);

      autalmot_delete(A);
      A = autalmot_new();
      autalmot_add_state(A);
      autalmot_add_state(A, AUT_TERMINAL);
 
      /*
        int idx = language_add_form(LANG, unloadable);
        symbol_t * s = symbol_unknow_new(LANG, idx);
      */
      
      autalmot_add_trans(A, 0, unloadable, 1);
      nb_unloadable++;

    } else {

      int min, max;

      avant       = eval_sentence(A, &min, &max);
      cumulavant += avant;
      lgavant = lgavant + ((double) (min + max) / (double) 2);

      add_limphrase(A);
      
      if (A->nbstates < 2) {

        error("sentence %d is void?????.\n", no + 1) ;

      } else {

        for (int j = 0; j < gramms->nbelems; j++) {

          autalmot_t * temp = interAutAtome(A, (autalmot_t *) gramms->tab[j]);
          
          //debug("avant emonde\n");
          
          autalmot_emonde(temp);
          
          //debug("apres emonde\n");
  
          autalmot_delete(A);
          A = temp;

          //debug("un peu plus loin ...\n");
 
          if (A->nbstates < 2) { // sentence rejected by grammar

            error("sentence %d rejected.\n\n", no + 1) ;
            j = gramms->nbelems;  /* on arrete pour cette phrase */
            nbPhrRej++;

	    autalmot_delete(A);
	    A = autalmot_new();
	    autalmot_add_state(A);
	    autalmot_add_state(A, AUT_TERMINAL);

            /*
	    int idx = language_add_form(LANG, rejected);
	    symbol_t * s = symbol_unknow_new(LANG, idx);
            */
	    autalmot_add_trans(A, 0, rejected, 1);
            autalmot_concat(A, orig);
            isrej = 1;
          }
        }
      }


      if (! isrej) {

        autalmot_determinize(A);
        autalmot_emonde(A);
        autalmot_minimize(A);
      
        if (suppress_limphrase(A) == -1) {
          error("an error occured while trying to remove sentence limits in sentence %d.\n", 
                no + 1);
        }

        apres = eval_sentence(A, &min, &max);
        cumulapres += apres;
        lgapres = lgapres + ((double) (min + max) / (double) 2);
      }
    }

    fst_file_write(fstout, A);

    autalmot_delete(A);
    autalmot_delete(orig);

    no++;
  }

  u_printf("\n");

  fst_file_close(txtin);
  fst_file_close(fstout);


  time_t fin = time(0);

  u_printf("\n*** done. result in '%s'\n", outname);
  u_printf("\nElapsed time: %.0f s.\n",difftime(fin, debut));
  u_printf("Text. Before: %.1f, after: %.1f units per sentence. ", cumulavant / no, cumulapres / no);
  if (cumulavant > 0.0) {
    
    if (cumulapres / cumulavant > 0.01) {
       u_printf("Residual: %.0f%%.\n",100.0 * cumulapres / cumulavant);
    } else {
      u_printf("Residual: %.1f%%.\n",100.0 * cumulapres / cumulavant);
    }
  }


  /* nouveau decompte */

  u_printf("\n****************\n\n");

  double logITo = cumulavant; // logITo pour log(nbre d'Interpretation du Texte d'origine)
  double logITe = cumulapres; // logITe pour log(nbre d'Interpretation du Texte élagué)
  double ambrateo = exp(logITo/lgavant);
  double ambratee = exp(logITe/lgapres);

  u_printf("before grammar application:\n");
  u_printf("log(|Int(Torig)|) = %.1f (%.0f interpretations, %.1f interp. per sentence)\n",
         logITo, exp(logITo), exp(logITo/(double) no));
  u_printf("corpus length = %.1f (%.1f lexems per sentence)\naverage ambiguity rate: %.3f\n",
         lgavant, lgavant / (double) no, ambrateo);
  u_printf("\nafter grammar application:\n");
  u_printf("log(|Int(Telag)|) = %.1f (%.0f interpretations, %.1f interp. per sentence)\n",
         logITe, exp(logITe), exp(logITe/(double) no));
  u_printf("corpus length = %.1f (%.1f lexems per sentence)\n"
         "average ambiguity rate: %.3f\n", lgapres, lgavant / (double) no, ambratee);
  u_printf("\nResidual: %.2f %%.\n", (ambratee / ambrateo) * 100.);
  u_printf("(Other residual: %.5f %% of residual ambig.)\n", exp(logITe - logITo) * (double) 100.);
  u_printf("\n%d sentences, %d not successfully loaded and %d rejected by elag grammars.\n\n",
         no, nb_unloadable, nbPhrRej);
  symbol_delete(unloadable);
  symbol_delete(rejected);
}





/* Charge les grammaires deja compilees. */

list_aut * chargeGramm(char * nomFichGramm) {

  char fname[strlen(nomFichGramm) + 5];

  strcpy(fname, nomFichGramm);

  if (strcmp(fname + strlen(fname) - 4, ".rul") != 0) { strcat(fname, ".rul"); } 

  FILE * fGramm = fopen(fname, "r");

  if (! fGramm) { fatal_error("opening file %s\n", fname); }

  list_aut * gramms = list_aut_new();

  char buf[FILENAME_MAX];

  while (fgets(buf, FILENAME_MAX, fGramm) != NULL) {

    if (*buf != '<') { continue; }

    char * p = strchr(buf, '>');

    if (p == NULL) { fatal_error("in %s: at line '%s': delimitor '>' not found\n", nomFichGramm, buf);  }

    *p = 0;

    error("\nReading %s...\n", buf + 1);

    autalmot_t * A = load_grammar_automaton(buf + 1);

    if (A == NULL) { fatal_error("unable to load '%s' automaton\n", buf + 1); }

    list_aut_add(gramms, A);
  }

  error("%d gramm(s) loaded\n", gramms->nbelems);

  return gramms;
}






list_aut * chargeUneGramm(char * name) { fatal_error("charhgeUneGramm: not implemented\n"); return NULL; }






/* Ajoute les limites de phrase.
 * A utiliser avec la version Windows d'INTEX. Preconditions :
 * d a au moins un etat ; la taille est egale au nombre d'etats.
 * Postcondition : la taille est egale au nombre d'etats.
 */
 
static void add_limphrase(autalmot_t * A) {

  static unichar S[] = { '{', 'S', '}', 0 };

  int idx = language_add_form(LANG, S);

  symbol_t * LIM = symbol_PUNC_new(LANG, idx);

  int initBis   = autalmot_add_state(A);
  int nouvFinal = autalmot_add_state(A, AUT_TERMINAL);

  A->states[initBis].trans  = A->states[A->initials[0]].trans;
  A->states[A->initials[0]].trans = NULL;

  A->states[initBis].flags = A->states[A->initials[0]].flags & ~(AUT_INITIAL);
  A->states[A->initials[0]].flags = AUT_INITIAL;

  autalmot_add_trans(A, A->initials[0], LIM, initBis);


  for (int q = 1; q < A->nbstates - 2; q++) {
    if (autalmot_is_final(A, q)) {
      autalmot_add_trans(A, q, LIM, nouvFinal);
      autalmot_unset_terminal(A, q);
    }
  }

  symbol_delete(LIM);
}










/* enleve les '{S}' rajoutes aux limites des phrases lors de leur chargement
 *
 * l'automate doit avoir un unique etat initial a la premiere position
 * et un unique etat final ... a la derniere position. 
 */


int suppress_limphrase(autalmot_t * A) {


  if (A->initials[0] != 0 || ! autalmot_is_final(A, A->nbstates - 1)
      || (A->states[A->nbstates - 2].trans->to != A->nbstates - 1)
      || (A->states[A->nbstates - 2].trans->next != NULL)) {

    autalmot_tri_topo(A);
  }

  if (A->nbstates < 4 || A->nbinitials != 1 || A->initials[0] != 0 || A->states[0].trans == NULL
      || A->states[0].trans->next != NULL || ! autalmot_is_final(A, A->nbstates - 1)) {

    error("suppress_limphrase: bad automaton\n");

    error("nbetats=%d, nbinitiaux=%d, initial[0]=%d, etats[0]=%d, etats[0]->suivant=%d, type[last]=%d\n",
          A->nbstates, A->nbinitials, A->initials[0], A->states[0].trans,
          A->states[0].trans ? A->states[0].trans->next : (void *) -1,  A->states[A->nbstates - 1].flags);
    return -1;
  }

  if (u_strcmp(language_get_form(A->states[0].trans->label->canonic), "{S}") != 0) {
    error("suppress_limphrase: no sentence limit found\n");
    return -1;
  }


  int newinit = A->states[0].trans->to - 1;

  int qfinal  = A->nbstates - 1;
  int nbstate = A->nbstates - 2;

  state_t * newtab = (state_t *) xmalloc(nbstate * sizeof(state_t));


  for (int q = 0; q < nbstate; q++) {

    newtab[q].trans = NULL;
    newtab[q].flags = A->states[q + 1].flags;
    newtab[q].defto = -1;

    transition_t * tmp;

    for (transition_t * t = A->states[q + 1].trans; t; t = tmp) {

      tmp = t->next;

      if (t->to == qfinal) { /* on indique l'etat comme final et on supprime la transition */

        newtab[q].flags |= AUT_FINAL;
        transition_delete(t);

      } else {                /* on garde la transition */

        t->to = t->to - 1;
        t->next = newtab[q].trans;
        newtab[q].trans = t;
      }
    }

  }

  newtab[newinit].flags |= AUT_INITIAL;
  A->initials[0]     = newinit;

  free(A->states); A->states = newtab;
  A->nbstates = A->size = nbstate;

  return 0;
}




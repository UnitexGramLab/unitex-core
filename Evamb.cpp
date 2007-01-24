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

#ifdef __GNUC__  // gcc (i.e. UNIX)

#include <unistd.h>

#elif  defined(__VISUALC__)

//  #ifdef __VISUALC__  // visual studio

#include <DIRECT.H>

#else    // Borldand

#include <dir.h>
#endif


#include <math.h>
#include "Copyright.h"
#include "utils.h"
#include "autalmot_old.h"
#include "decompte.h"
#include "fst2autalmot.h"
#include "implose.h"
#include "Fst2.h"
#include "String_hash.h"
#include "IOBuffer.h"
#include "Error.h"

#define MAX(a,b) (((a) < (b)) ? (b) : (a))

static void denombrer(tAutAlMot * A, int q, int * tab) {

  if (tab[q] > -1) { return; }

  tab[q] = 0;
  for (tTransitions * t = A->etats[q]; t; t = t->suivant) {
    denombrer(A, t->but, tab);
    tab[q] = tab[q] + tab[t->but];
  }

  if (final(A, q)) { tab[q] = tab[q] + 1; }
}



/**
 * explosion combinatoire
 * le nombre de sequences sur certaines phrases est trop grand pour rentrer dans un entier 
 *
 * deprected
 */

/*static int denombrer(tAutAlMot * A) {
  int res = 0;

  int tab[A->nbEtats];

  for (int i = 0; i < A->nbEtats; i++) { tab[i] = -1; }

  for (int i = 0; i < A->nbEtatsInitiaux; i++) {
    denombrer(A, A->initial[i], tab);
    res = res + tab[A->initial[i]];
  }

  return res;
}*/




static long denombrer(tAutAlMot *  A, int q1, int q2) {

 long res = 0;

  for (tTransitions * t = A->etats[q1]; t; t = t->suivant) {
    if (t->but == q2) {
      res++;
    } else {
      res = res + denombrer(A, t->but, q2);
    }
  }

  return res;
}



double eval_sentence(tAutAlMot * A) {

  tri_topologique(A);


  /* recherche du plus grand saut */

  int maxdirect = 0;

  int q;
  for (q = 0; q < A->nbEtats; q++) {
    for (tTransitions * t = A->etats[q]; t; t = t->suivant) {
      maxdirect = MAX(maxdirect, t->but - q);
    }
  }


  /* construction de la matrice directe */

  int direct[A->nbEtats][maxdirect + 1];

  for (q = 0; q < A->nbEtats; q++) {

    for (int i = 0; i < maxdirect + 1; i++) { direct[q][i] = 0; }

    for (tTransitions * t = A->etats[q]; t; t = t->suivant) { direct[q][t->but - q] = 1; }
  }

  /* noeuds factorisants */

  bool factorisants[A->nbEtats];

  for (q = 0; q < A->nbEtats; q++) { factorisants[q] = true; }

  for (int i = 0; i < A->nbEtats; i++) {
    for (int j = 2; j <= maxdirect; j++) {
      if (direct[i][j]) {
	for (int k = i + 1; k < i + j; k++) { factorisants[k] = false; }
      }
    }
  }

  
  /* denombrement */

  double res = 0;

  int q1, q2;

  q2 = 0;
  while (q2 < A->nbEtats - 1) {
    q1 = q2;
    q2++;
    while (! factorisants[q2]) { q2++; }
    int dn = denombrer(A, q1, q2);
    res = res + log((double) dn);
  }
  
  return res;
}



/*
 * retourne le nombre de sequences entre 2 etats (q1 et q2) de A
 * mets dans min la long de la sequence la plus courte ...
 */


long evamb(tAutAlMot * A, int q1, int q2, int * min, int * max) {

  if (q1 == q2) { *min = 0; *max = 0; return 1; }
  
  long res = 0;
  *min = 1000000;
  *max = 0;

  for (tTransitions * t = A->etats[q1]; t; t = t->suivant) {
   
    int lmin, lmax;
    res = res + evamb(A, t->but, q2, & lmin, & lmax);

    if ((lmin + 1) < *min) { *min = lmin + 1; }
    if ((lmax + 1) > *max) { *max = lmax + 1; }
  }

  return res;
}



/*
 * calcule le logarithme du nombre de chemins dans l'automate,
 * mets dans min la longueur du chemin le plus court et dans
 * max celle du chemin le plus long
 */


double evamb(tAutAlMot * A, int * min, int * max) {

  tri_topologique(A);


  /* recherche du plus grand saut */

  int maxdirect = 0;

  int q;
  for (q = 0; q < A->nbEtats; q++) {
    for (tTransitions * t = A->etats[q]; t; t = t->suivant) {
      maxdirect = MAX(maxdirect, t->but - q);
    }
  }


  /* construction de la matrice directe */

  int direct[A->nbEtats][maxdirect + 1];

  for (q = 0; q < A->nbEtats; q++) {

    for (int i = 0; i < maxdirect + 1; i++) { direct[q][i] = 0; }

    for (tTransitions * t = A->etats[q]; t; t = t->suivant) { direct[q][t->but - q] = 1; }
  }

  /* noeuds factorisants */

  bool factorisants[A->nbEtats];

  for (q = 0; q < A->nbEtats; q++) { factorisants[q] = true; }

  for (int i = 0; i < A->nbEtats; i++) {
    for (int j = 2; j <= maxdirect; j++) {
      if (direct[i][j]) {
	for (int k = i + 1; k < i + j; k++) { factorisants[k] = false; }
      }
    }
  }

  
  /* denombrement */

  *min = 0, *max = 0;
  double res = 0;

  int q1, q2;

  q2 = 0;
  while (q2 < A->nbEtats - 1) {
    q1 = q2;
    q2++;
    while (! factorisants[q2]) { q2++; }
    //    int dn = denombrer(A, q1, q2);
    int lmin, lmax;
    long dn = evamb(A, q1, q2, &lmin, &lmax);
    *max = *max + lmax; *min = *min + lmin;
    res = res + log((double) dn);
  }
  
  return res;
}






void usage() {
  printf("%s", COPYRIGHT);
  printf("usage: Evamb [ -imp | -exp ] [-o] <fstname> [ -n <sentenceno> ]\n"
         "\n"
         "where :\n"
         " <fstname>     :   text automaton FST2 file\n"
         " <sentenceno>  :   sentence number\n"
         " -imp          :   implose automaton first\n"
         " -exp          :   explose automaton first\n"
         "\n"
         "Give average lexical ambiguity rate of the whole text automaton, or the sentence specified by <sentenceno>.\n"
         "If '-imp' is set, the computation is performed on the implosed form of the automaton, i.e. not considering\n"
         "flexional ambiguities.\n"
         "Reciprocally, if '-exp' is set, the factorized labels are considering as several times ambiguous.\n"
         "The text automaton is not modified.\n");
}



int main(int argc, char ** argv) {
setBufferMode();

  argv++, argc--;

  if (argc == 0) { usage(); return 0; }


  char * autoname = NULL;
  bool implosion = false, explosion = false;
  int no = -1;


  while (argc) {

    if (**argv != '-') { // fst filename
      autoname = *argv;
    
    } else if (strcmp(*argv, "-h") == 0) {
    
        usage();
        return 0;
    
    } else if (strcmp(*argv, "-o") == 0) {

      argv++, argc--;
      if (argc < 1) {
         usage();
         fatal_error("bad args\n");
      }
      autoname = *argv;
      
    } else if (strcmp(*argv, "-exp") == 0) {

      explosion = true;

    } else if (strcmp(*argv, "-imp") == 0) {

      implosion = true;
    
    } else if (strcmp(*argv, "-n") == 0) {
   
      argv++, argc--;
      if (argc < 1) { usage(); fatal_error("bad args\n"); }

      no = atoi(*argv);
    }

    argv++, argc--;
  }


  if (autoname == NULL) { fatal_error("no fst specified\n"); }

  debug("loading '%s' ...\n", autoname);
  list_aut_old * txtauto = load_text_automaton(autoname, explosion);

  if (txtauto == NULL) { fatal_error("unable to load '%s' fst2\n", *argv); }

  if (no > txtauto->nb_aut) { fatal_error("only %d sentence(s) in '%s'\n", txtauto->nb_aut, autoname); }


  if (no < 0) { // eval all sentences

    double cumullognp = 0., cumullmoy = 0.;
    int badauto  = 0;

    double maxlognp  = 0., minlognp  = 1000000.;
    double maxlogamb = 0., minlogamb = 1000000.;
    int maxnpno = -1, minnpno = -1, maxambno = -1, minambno = -1;

    for (no = 0; no < txtauto->nb_aut; no++) {

      if (txtauto->les_aut[no]->nbEtats < 1) {

	badauto++;
	error("%d: empty automaton\n", no + 1);

      } else {
      
        if (implosion) { implose(txtauto->les_aut[no]); }
      
        double lognp;       // log du nombre de chemins dans l'automate
        int lmin, lmax;     // longueur minimum/maximum de la phrase
        double lmoy;        // approx. de la longueur moyenne de la phrase (lmin + lmax) / 2
        double logamb;  // log du taux d'ambiguité lexicale :  ambrate = exp(log(np)/lmoy)

        lognp = evamb(txtauto->les_aut[no], & lmin, & lmax);
        lmoy  = (double) (lmin + lmax) / (double) 2;

        logamb = lognp / lmoy;

	if (maxlognp < lognp)   { maxlognp = lognp; maxnpno = no + 1; }
	if (minlognp > lognp)   { minlognp = lognp; minnpno = no + 1; }
	if (maxlogamb < logamb) { maxlogamb = logamb; maxambno = no + 1; }
	if (minlogamb > logamb) { minlogamb = logamb; minambno = no + 1; }
        
        error("%d: lognp=%.2f, lmoy=%.1f, amb. rate=%.2f, (%d intr.)\n", no + 1, (double) lognp, (double) lmoy,
               (double) exp(logamb), (int) exp(lognp));

        cumullognp = cumullognp + lognp; cumullmoy = cumullmoy + lmoy;
      }
    }

    // cumullognp = log du nbre d'interpretations du texte
    // cumullmoy = longueur moyenne du texte (en mots)

    if (badauto >= txtauto->nb_aut) {
      error("no automaton ?\n");
    } else {
    
      printf("\n%s: average of %.2f (old) units per sentence\n", autoname, cumullognp / (txtauto->nb_aut - badauto));
      printf("\n%s: lognp = %.2f, lmoy = %.1f, amb. rate = %.3f\n\n", autoname, cumullognp, cumullmoy, exp(cumullognp/cumullmoy));

      printf("min lognp: %.2f (sentence %d)\n", minlognp, minnpno);
      printf("max lognp: %.2f (sentence %d)\n", maxlognp, maxnpno);
      printf("min amb. rate: %.2f (sentence %d)\n", exp(minlogamb), minambno);
      printf("max amb. rate: %.2f (sentence %d)\n", exp(maxlogamb), maxambno);
    }

  } else { // eval one sentence

    int min, max;
    double lognp = evamb(txtauto->les_aut[no - 1], & min, & max);
    double lmoy = (double) (min + max) / (double) 2;
    
    printf("%s: sentence %d: lognp=%.2f, lmoy=%.1f, amb. rate=%.3f, (%d intr.)\n", autoname, no, (double) lognp, (double) lmoy,
           (double) exp(lognp / lmoy), (int) exp(lognp));
  }

  return 0;
}


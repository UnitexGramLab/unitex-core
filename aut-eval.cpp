 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <math.h>
#include "autalmot.h"
#include "utils.h"

#define MAX(a, b) ((a < b) ? b : a)




static void denombrer(Fst2Automaton *  A, int q1, int q2, int * mintab, int * maxtab, int * nptab) {

  //  debug("denombrer(%d,%d)\n", q1, q2);

  if (q1 == q2) { //debug("!!!!!!!!!!!!!!!!!!!!\n");
    mintab[q1] = 0; maxtab[q1] = 0; nptab[q1] = 1;
    //*min = 0; *max = 0; return 1; 
  }
  
  if (nptab[q1] != -1) {
    return;
  }
  
  nptab[q1]  = 0;
  mintab[q1] = 1000000;
  maxtab[q1] = 0;

  for (transition_t * t = A->states[q1].trans; t; t = t->next) {
   
    denombrer(A, t->to, q2, mintab, maxtab, nptab);
    nptab[q1] = nptab[q1] + nptab[t->to];

    if ((mintab[t->to] + 1) < mintab[q1]) { mintab[q1] = mintab[t->to] + 1; }
    if ((maxtab[t->to] + 1) > maxtab[q1]) { maxtab[q1] = maxtab[t->to] + 1; }
  }
}

double eval_sentence(Fst2Automaton * A, int * min, int * max) {

  int _dumbmax, _dumbmin;

  if (max == NULL) { max = &_dumbmax; }
  if (min == NULL) { min = &_dumbmin; }

//  debug("aut_eval\n");

  autalmot_tri_topo(A);

  //debug("apres tri_topo\n");

  /* recherche du plus grand saut */

  int maxdirect = 0;

  int q;
  for (q = 0; q < A->nbstates; q++) {
    for (transition_t * t = A->states[q].trans; t; t = t->next) {
      maxdirect = MAX(maxdirect, t->to - q);
    }
    maxdirect = MAX(maxdirect, A->states[q].defto);
  }

  /* construction de la matrice directe */

  //int ** direct = (int **) alloca(A->nbstates * sizeof(int *));
  int direct[A->nbstates][maxdirect+1];

  for (q = 0; q < A->nbstates; q++) {

    //direct[q] = (int *) alloca((maxdirect + 1) * sizeof(int));

    for (int i = 0; i < maxdirect + 1; i++) { direct[q][i] = 0; }

    for (transition_t * t = A->states[q].trans; t; t = t->next) { direct[q][t->to - q] = 1; }
    if (A->states[q].defto != -1) { direct[q][A->states[q].defto - q] = 1; }
  }

//  debug("here\n");

  /* noeuds factorisants */

  bool factorisants[A->nbstates];

  for (q = 0; q < A->nbstates; q++) { factorisants[q] = true; }

  for (int i = 0; i < A->nbstates; i++) {
    for (int j = 2; j <= maxdirect; j++) {
      if (direct[i][j]) {
	for (int k = i +1; k < i + j; k++) { factorisants[k] = false; }
      }
    }
  }


//  debug("apres fact\n");

  /* denombrement */

  double res = 0;
  *max = 0, *min = 0;

  int mintab[A->nbstates];
  int maxtab[A->nbstates];
  int nptab[A->nbstates];

  for (int i = 0; i < A->nbstates; i++) {
    mintab[i] = maxtab[i] = nptab[i] = -1;
  }

  int q1, q2;

  q2 = 0;
  while (q2 < A->nbstates - 1) {
//    debug("q2=%d\n", q2);
 
    q1 = q2;
    q2++;
    while (! factorisants[q2]) { q2++; }
    //    int lmin, lmax;
    nptab[q1] = -1;
    denombrer(A, q1, q2, mintab, maxtab, nptab);
    *max = *max + maxtab[q1]; *min = *min + mintab[q1];
    res = res + log((double) nptab[q1]);
  }
 
  return res;
}




static void denombrer2(Fst2Automaton * A, int q, int * tab) {

  if (tab[q] > -1) { return; }

  tab[q] = 0;
  for (transition_t * t = A->states[q].trans; t; t = t->next) {
    denombrer2(A, t->to, tab);
    tab[q] = tab[q] + tab[t->to];
  }
}


double eval_sentence2(Fst2Automaton * A) {

  int res = 0;
  int tab[A->nbstates];

  for (int i = 0; i < A->nbstates; i++) { tab[i] = -1; }

  for (int i = 0; i < A->nbinitials; i++) {
    denombrer2(A, A->initials[i], tab);
    res = res + tab[A->initials[i]];
  }

  return res;
}

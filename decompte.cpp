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

#include <stdio.h>
#include <stdlib.h>

#include "autalmot_old.h"
#include "decompte.h"

#include "utils.h"

#define TAILLE_MAX_SYMBOL  80


/* Preconditions : autTrie est vide.
 * Ne modifie pas le premier automate d'entree A.
 * Place dans autTrie un automate equivalent dans lequel les etats sont
 * numerotes suivant un tri topologique. Postconditions :
 * il n'y a pas de partage de memoire entre A et autTrie ;
 * la taille est egale au nombre d'etats dans autTrie.
 * remplissage du tableau du degre entrant
 */

tAutAlMot * tri_topologique(tAutAlMot * A, int * dicoInverse) {

  int * dico ;
  etat i, nouveau, ancien ;
  tTransitions * T;

  int * entrants = (int *) xmalloc(A->nbEtats * sizeof(int)) ;

  for (i = 0; i < A->nbEtats; i ++) { entrants[i] = 0; }

  for (i = 0; i < A->nbEtats; i ++) {
    for (T = A->etats[i]; T; T = T->suivant) { entrants[T->but]++; }
  }

  /* tri topologique */

  dico = (int *) xcalloc(A->nbEtats, sizeof(int));

  for (nouveau = 0 ; nouveau < A->nbEtats ; nouveau++) {

    ancien = 0;

    while (entrants[ancien] != 0) { ancien++; }

    dico[ancien]         = nouveau;
    dicoInverse[nouveau] = ancien;
    entrants[ancien]     = -1;

    for (T = A->etats[ancien]; T != NULL; T = T->suivant) { entrants[T->but]--; }
  }


  /* creation du nouvel automate */

  tAutAlMot * autTrie = initAutAlMot(A->nbEtats);


  if (A->nbEtatsInitiaux != 1 || A->initial[0]) { fatal_error("tri_topologique: bad automaton"); }

  for (ancien = 0; ancien < autTrie->nbEtats; ancien++) {

    autTrie->type[dico[ancien]] = A->type[ancien];

    autTrie->etats[dico[ancien]] = NULL;

    for (T = A->etats[ancien]; T; T = T->suivant) {
      nouvTrans(autTrie, dico[ancien], T->etiq, dico[T->but]);
    }
  }
  
  free(dico);
  free(entrants);

  return autTrie;
}




void tri_topologique(tAutAlMot * A) {

  int * dicoInverse = (int *) xmalloc(A->nbEtats * sizeof(int));

  tAutAlMot * tri = tri_topologique(A, dicoInverse);

  tTransitions ** tmptrans = A->etats;
  A->etats   = tri->etats;
  tri->etats = tmptrans;

  char * tmpsorte = A->type;
  A->type         = tri->type;
  tri->type       = tmpsorte;

  free(A->entrantesEtats);
  A->entrantesEtats = NULL;

  etat tmptaille = A->taille;
  A->taille      = tri->taille;
  tri->taille    = tmptaille;

  etat * tmpinit = A->initial;
  A->initial     = tri->initial;
  tri->initial   = tmpinit;


  free(dicoInverse);
  libereAutAlMot(tri);
}

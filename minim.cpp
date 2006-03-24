 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "general.h"
#include "autalmot.h"
#include "minim.h"

#include "utils.h"

static tTransCol * * triTrans(tAutAlMot * aut, tAlphMotPartage * alph) ;
static void insereTri(tTransCol * nouveau, tTransCol * * liste, etat e) ;
static int * initCouleur(tAutAlMot * aut, int * nbCouleurs) ;
static void colore(tTransCol * * sorties, int * couleur, etat nbEtats) ;
static int trouveNuance(etat e, tTransCol * * sorties, int * couleur, int * nuance, int * nbNuances) ;
static int compTrans(tTransCol * t1, tTransCol * t2);
static etat * choisirEtats(int * couleur, int nbCouleurs, etat nbEtats);
static tAutAlMot * remplirMin(tAutAlMot * aut, etat * repr, int * couleur, int nbCouleurs);

void afficheSorties(tTransCol * sorties);



void alphmot_dump(tAlphMotPartage * alph) {

  fprintf(stderr, "%d symbols:\n", alph->nbSymboles);
  for (unsigned int i = 0; i < alph->nbSymboles; i++) { Affiche_Symbole(alph->symb[i]); fprintf(stderr, "\n"); }
}



/* Detruit aut et renvoie son automate deterministe complet minimal
 * (avec au moins un etat). Precondition : l'automate d'entree aut
 * est deterministe et complet ; il a au moins un etat.
 */


tAutAlMot * minimise(tAutAlMot * aut) {

  tAlphMotPartage * alph;
  int nbCouleurs, nbNuances;
  tTransCol ** sorties;
  int * couleur, * nuance;  /* couleur[e] <= e */
  etat e, * repr;
  tAutAlMot * min;
  BOOL parDefaut;

  //  debug("Minimisation... (%d etats)\n", aut->nbEtats);

  alph = listeSymboles(aut, & parDefaut);


  //  debug("alphabet:\n"); alphmot_dump(alph);

  sorties = triTrans(aut, alph);
  libereAlphabet(alph);

  couleur = (int *) xcalloc(aut->nbEtats, sizeof(int));
  nuance  = initCouleur(aut, & nbNuances);


  do {

    //    debug("%d couleurs :\n", nbNuances);
    //    for (e = 0; e < aut->nbEtats; e++) { fprintf(stderr, "%d -> %d\n", e, nuance[e]); }

    for (e = 0; e < aut->nbEtats; e++) { couleur[e] = nuance[e]; }

    nbCouleurs = nbNuances;
    nbNuances = 0;
    colore(sorties, couleur, aut->nbEtats);

    for (e = 0 ; e < aut->nbEtats ; e++) { nuance[e] = trouveNuance(e, sorties, couleur, nuance, & nbNuances); }

  } while (nbCouleurs != nbNuances);


  repr = choisirEtats(couleur, nbCouleurs, aut->nbEtats);

  free(sorties);
  free(nuance);

  min = remplirMin(aut, repr, couleur, nbCouleurs);

  free(couleur);
  free(repr);

  //  debug("Minimisation OK (%d etats)\n", min->nbEtats);

  return min;
}



/*
Es imperdonable no ense~nar la musica espa~nola a tus compa~neros de despacho !!!
Ven a visitar a Teresa.
Que pasa, ya no compila ?????
*/


/* Construit les listes de transitions sortantes, triees et avec
 * les numeros de symboles.
 */

static tTransCol ** triTrans(tAutAlMot * aut, tAlphMotPartage * alph) {

  tTransCol ** sorties, * temp ;
  etat e ;
  tTransitions * t ;

  sorties = (tTransCol **) xcalloc(aut->nbEtats, sizeof(tTransCol *)) ;

  for (e = 0; e < aut->nbEtats; e++) {

    sorties[e] = NULL ;

    for (t = aut->etats[e]; t; t = t->suivant) {

      temp = (tTransCol *) xcalloc(1, sizeof(tTransCol)) ;

      temp->etiquette  = (t->etiq ? numeroDansAlph(t->etiq, alph) : alph->nbSymboles - 1) ;
      temp->but        = t->but;
      temp->couleurBut = 0;
      temp->suivant    = NULL;
      insereTri(temp, sorties, e) ;
    }

    /*
      if (aut -> initial[0]) {
      printf("[triTrans2] etat %d, l etat %d est initial\n", e + 1,
      aut -> initial[0] + 1) ;
      sauvegAutAlMot(stdout, aut, 1, FALSE) ;
      }
    */

    /*afficheSorties(sorties[e]) ;*/
  }

  return sorties ;
}



/* insere un nouvel element dans une liste triee.
 * Le nouvel element est distinct de tous les autres.
 */

static void insereTri(tTransCol * nouveau, tTransCol * * liste, etat e) {

  tTransCol * t, * prec = NULL ;

  /*printf("insereTri %d\n", nouveau -> etiquette) ;*/

  for (t = liste[e]; t; t = t->suivant) {
    if (nouveau -> etiquette < t -> etiquette) {
      nouveau -> suivant = t ;
      if (prec)
	prec -> suivant = nouveau ;
      else liste[e] = nouveau ;
      return ;
    } else { prec = t ; }
  }
  nouveau->suivant = NULL ;

  if (prec) {
    prec->suivant = nouveau ;
  } else {
    liste[e] = nouveau ;
  }
}




/* initialise le tableau de couleurs. La couleur de l etat 0 est 0.
 */

static int * initCouleur(tAutAlMot * aut, int * nbCouleurs) {

  int * couleur ;
  etat e ;
  BOOL bicolore = FALSE ;
  couleur = (int *) xcalloc(aut->nbEtats, sizeof(int)) ;

  if (final(aut, 0)) {
    for (e = 0; e < aut->nbEtats ; e ++) {
      couleur[e] = (final(aut, e) ? 0 : (bicolore = TRUE, 1)); /* 0 si terminal, 1 sinon*/
    }
  } else {
    for (e = 0; e < aut->nbEtats; e++) {
      couleur[e] = (final(aut, e) ? (bicolore = TRUE, 1) : 0);   /* 1 si terminal, 0 sinon */
    }
  }

  *nbCouleurs = (bicolore ? 2 : 1) ;
  return couleur;
}



/* Colore les listes de transitions en donnant
 * les bonnes couleurs aux etats buts de "sorties".
 */

static void colore(tTransCol ** sorties, int * couleur, etat nbEtats) {

  etat e ;
  tTransCol * t;

  for (e = 0; e < nbEtats; e++) {
    for (t = sorties[e]; t; t = t->suivant) {
      t->couleurBut = couleur[t->but] ;
    }
  }
}




/* Renvoie la nuance de l'etat e. Met a jour nbNuances s'il s'agit
 * d'une nouvelle nuance. Les comparaisons se font avec les autres 
 * etats de la meme couleur c dont les nuances sont deja connues.
 */

static int trouveNuance(etat e, tTransCol ** sorties, int * couleur, int * nuance, int * nbNuances) {

  etat f ;

  for (f = couleur[e]; f < e; f++) {
    if (couleur[f] == couleur[e]) {
      if (compTrans(sorties[e], sorties[f]) == 0) { return nuance[f]; }
    }
  }

  /* nouvelle nuance */

  (*nbNuances)++;
  return (*nbNuances) - 1;
}


void afficheSorties(tTransCol * sorties) {

  for (; sorties; sorties = sorties->suivant) { printf(" %d>%d", sorties->etiquette, sorties->couleurBut) ; }

  printf("\n") ;
}


/* Compare t1 et t2. Renvoie 0 si elles sont identiques
 * et un autre entier si elles sont differentes.
 */

static int compTrans(tTransCol * t1, tTransCol * t2) {

  while (t1 && t2) {

    if ((t1->etiquette != t2->etiquette) || (t1->couleurBut != t2->couleurBut)) { return 1; }

    t1 = t1->suivant;
    t2 = t2->suivant;
  }
  
  //  if (t1 == t2) { debug("trans equivs!!!\n"); }

  return t1 != t2; /* (== NULL) */
}


/* Pour chaque couleur, choisir parmi les etats un representant
 * canonique. Le numero du representant est >= au numero de la couleur
 */

static etat * choisirEtats(int * couleur, int nbCouleurs, etat nbEtats) {

  int c ;
  etat e, * repr ;
  BOOL trouve ;

  repr = (etat *) xcalloc(nbEtats, sizeof(etat)) ;


  for (c = 0; c < nbCouleurs; c++) {

    for (trouve = FALSE, e = c; ! trouve && e < nbEtats; e ++) {

      if (couleur[e] == c) {
	repr[c] = e;
	trouve = TRUE;
      }
    }

    if (trouve == FALSE) { die("choisirEtats: color %d not found!\n", c); }

  }

  return repr ;
}



/* Renvoie un automate deduit de aut en remplacant les etats par leurs
 * couleurs.
 */

static tAutAlMot * remplirMin(tAutAlMot * aut, etat * repr, int * couleur, int nbCouleurs) {

  tAutAlMot * min ;
  tTransitions * t1 ;
  int c ;

  min = initAutAlMot(nbCouleurs) ;

  for (c = 0; c < nbCouleurs; c ++) {

    min->etats[c] = aut->etats[repr[c]] ;
    aut->etats[repr[c]] = NULL ;

    for (t1 = min->etats[c] ; t1 ; t1 = t1 -> suivant) { t1->but = couleur[t1->but]; }

    min->type[c] = aut->type[repr[c]] ;
  }

  min->initial[0] = couleur[aut->initial[0]];
  min->name = aut->name;

  aut->name = NULL;
  libereAutAlMot(aut);

  return min;
}

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

/* Nom : minim.c
 * Fonction : minimisation d'automates
 *              dans le projet de levee d'ambiguites d'une phrase.
 * Auteur : Eric Laporte
 */

/*
Es imperdonable no ense~nar la musica espa~nola a tus compa~neros de despacho !!!
Ven a visitar a Teresa.
Que pasa, ya no compila ?????
*/


#include <stdio.h>
#include <stdlib.h>

#include "autalmot.h"
#include "SymbolAlphabet.h"
#include "utils.h"


struct tTransCol {
  int etiquette;
  int but;
  int couleurBut;
  struct tTransCol * suivant;
};

typedef struct tTransCol tTransCol;


tTransCol * transCol_new(int etiq, int but, int color, tTransCol * next = NULL) {

  tTransCol * res = (tTransCol *) xmalloc(sizeof(tTransCol));

  res->etiquette  = etiq ;
  res->but        = but;
  res->couleurBut = color;
  res->suivant    = next;

  return res;
}


void transCols_delete(tTransCol * trans) {
  while (trans) { tTransCol * next = trans->suivant; free(trans); trans = next; }
}


/* insere un nouvel element dans une liste triee.
 * Le nouvel element est distinct de tous les autres.
 */

static void insereTri(tTransCol * nouveau, tTransCol ** liste, int e) {

  tTransCol * t, * prec = NULL;

  for (t = liste[e]; t; t = t->suivant) {
    if (nouveau->etiquette < t->etiquette) {
      nouveau->suivant = t;
      if (prec)
	prec->suivant = nouveau;
      else liste[e] = nouveau;
      return;
    } else { prec = t ; }
  }

  nouveau->suivant = NULL ;

  if (prec) {
    prec->suivant = nouveau ;
  } else {
    liste[e] = nouveau ;
  }
}


/* Construit les listes de transitions sortantes, triees et avec
 * les numeros de symboles.
 */

static tTransCol ** triTrans(Fst2Automaton * aut, SymbolAlphabet * alph) {

  tTransCol ** sorties = (tTransCol **) xmalloc(aut->nbstates * sizeof(tTransCol *));

  for (int e = 0; e < aut->nbstates; e++) {

    sorties[e] = NULL;

    for (transition_t * t = aut->states[e].trans; t; t = t->next) {

      tTransCol * temp = transCol_new(alphabet_lookup(alph, t->label), t->to, 0);
      insereTri(temp, sorties, e);
    }

    if (aut->states[e].defto != -1) { // transition par defaut
      tTransCol * temp = transCol_new(alphabet_lookup(alph, SYMBOL_DEF), aut->states[e].defto, 0);
      insereTri(temp, sorties, e);
    }
  }

  return sorties;
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
  
  return t1 != t2; /* (== NULL) */
}



/* initialise le tableau de couleurs: partitionne les etats finaux des etats non finaux
 * La couleur de l etat 0 est 0.
 */

static int * initCouleur(Fst2Automaton * aut, int * nbCouleurs) {

  int * couleur = (int *) xcalloc(aut->nbstates, sizeof(int)) ;

  bool bicolore = false;

  if (autalmot_is_final(aut, 0)) {
    for (int e = 0; e < aut->nbstates ; e ++) {
      couleur[e] = autalmot_is_final(aut, e) ? 0 : (bicolore = true, 1); /* 0 si terminal, 1 sinon*/
    }
  } else {
    for (int e = 0; e < aut->nbstates; e++) {
      couleur[e] = autalmot_is_final(aut, e) ? (bicolore = true, 1) : 0;   /* 1 si terminal, 0 sinon */
    }
  }

  *nbCouleurs = (bicolore ? 2 : 1);
  return couleur;
}



/* Colore les listes de transitions en donnant
 * les bonnes couleurs aux etats buts de "sorties".
 */

static void colore(tTransCol ** sorties, int * couleur, int nbstates) {
  for (int e = 0; e < nbstates; e++) {
    for (tTransCol * t = sorties[e]; t; t = t->suivant) {  t->couleurBut = couleur[t->but]; }
  }
}




/* Renvoie la nuance de l'etat e. Met a jour nbNuances s'il s'agit
 * d'une nouvelle nuance. Les comparaisons se font avec les autres 
 * etats de la meme couleur c dont les nuances sont deja connues.
 */

static int trouveNuance(int e, tTransCol ** sorties, int * couleur, int * nuance, int * nbNuances) {

  for (int f = couleur[e]; f < e; f++) {
    if (couleur[f] == couleur[e]) {
      if (compTrans(sorties[e], sorties[f]) == 0) { return nuance[f]; }
    }
  }

  /* nouvelle nuance */

  (*nbNuances)++;
  return (*nbNuances) - 1;
}




/* Pour chaque couleur, choisir parmi les etats un representant
 * canonique. Le numero du representant est >= au numero de la couleur
 */

static int * choisirEtats(int * couleur, int nbCouleurs, int nbstates) {

  int * repr = (int *) xmalloc(nbCouleurs * sizeof(int)) ;

  for (int c = 0; c < nbCouleurs; c++) {

    bool trouve = false;

    for (int e = c; ! trouve && e < nbstates; e++) {

      if (couleur[e] == c) {
	repr[c] = e;
	trouve = true;
      }
    }

    if (! trouve) { fatal_error("choisirEtats: color %d not found!\n", c); }
  }

  return repr;
}


/**
 * Removes all transitions whose destination state is n.
 */
Transition* clean_transitions(Transition* trans, int n) {
while (trans!=NULL && trans->state_number==n) {
   Transition* next=trans->next;
   free_Transition(trans,(void (*)(void*))free_symbol);
   trans=next;
}
for (Transition* t=trans;t!=NULL;t=t->next) {
   while (t->next!=NULL && t->next->state_number==n) {
      Transition* next=t->next->next;
      free_Transition(t->next,(void (*)(void*))free_symbol);
      t->next=next;
   }
}
return trans;
}


/**
 * This function removes transitions that take the same way than the
 * default ones.
 */
void compact_default_transitions(SingleGraph g) {
for (int q=0;q<g->number_of_states;q++) {
   if (g->states[q]->default_state!=-1) {
      g->states[q]->outgoing_transitions=clean_transitions(g->states[q]->outgoing_transitions,g->states[q]->default_state);
   }
}
}



/* minimise aut
 * aut doit etre deterministe
 */

/**
 * This function minimizes the given automaton. Note
 * that it must be deterministic.
 */
void elag_minimize(Fst2Automaton* A,int level) {
SingleGraph automaton=A->automaton;
struct list_int* initials=get_initial_states(automaton);
if (initials==NULL) {
   fatal_error("Cannot minimize an automaton with no initial state\n");
}
if (initials->next!=NULL) {
   fatal_error("Non-deterministic automaton in elag_minimize\n");
}
free_list_int(initials);
if (level>0) {
   /* If necessary, we remove transitions that are included in the
    * default ones */
   compact_default_transitions(automaton);
}
SymbolAlphabet* alph=build_symbol_alphabet(automaton);

  tTransCol ** sorties = triTrans(A, alph);

  alphabet_delete(alph);


  /* couleur[e] <= e */

  int nbCouleurs, nbNuances;

  int * couleur = (int *) xcalloc(A->nbstates, sizeof(int));
  int * nuance  = initCouleur(A, & nbNuances);


  do {

    //    debug("nbcolor=%d, nbnuances=%d\n", nbCouleurs, nbNuances);

    int e;

    for (e = 0; e < A->nbstates; e++) {
      couleur[e] = nuance[e];
      //      debug("couleur[%d] = %d\n", e, couleur[e]);
    }

    nbCouleurs = nbNuances;
    nbNuances = 0;
    colore(sorties, couleur, A->nbstates);

    for (e = 0 ; e < A->nbstates ; e++) { nuance[e] = trouveNuance(e, sorties, couleur, nuance, & nbNuances); }

  } while (nbCouleurs != nbNuances);

  //  debug("out: nbcolor=%d, nbnuances=%d\n", nbCouleurs, nbNuances);

  int * repr = choisirEtats(couleur, nbCouleurs, A->nbstates);

  for (int i = 0; i < A->nbstates; i++) { transCols_delete(sorties[i]); }
  free(sorties);
  free(nuance);

  /* on remplit le resultat  */

  state_t * nouvo = (state_t *) xmalloc(nbCouleurs * sizeof(state_t));

  for (int c = 0; c < nbCouleurs; c++) {

    nouvo[c].trans = A->states[repr[c]].trans;
    A->states[repr[c]].trans = NULL;

    for (transition_t * t1 = nouvo[c].trans; t1; t1 = t1->next) { t1->to = couleur[t1->to]; }

    nouvo[c].flags = A->states[repr[c]].flags;
    nouvo[c].defto = A->states[repr[c]].defto;
  }

  /* free old states */
  for (int i = 0; i < A->nbstates; i++) {
    transitions_delete(A->states[i].trans);
  }
  free(A->states);

  /* replace it */

  A->states = nouvo;
  A->size = A->nbstates = nbCouleurs;

  A->initials[0] = couleur[A->initials[0]];

  free(couleur);
  free(repr);


  //  debug("out of mimimize:\n"); autalmot_dump(aut);
}


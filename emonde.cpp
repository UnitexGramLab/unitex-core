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

/*************************************************************************
 *                                                                       *
 *                                  emonde.cpp                     v0.01 *
 *                          Emondation d'automates                       *
 *                                                                       *
 *                      (c) 1997 S. SAUPIQUE - MagiX                     *
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "general.h"
#include "autalmot.h"
#include "emonde.h"

#include "utils.h"

static int MarqueEtatSortant(int *tab, int e, tAutAlMot * a) ;
static int MarqueEtatEntrant(int *tab, int e, tAutAlMot * a) ;
/* static int LibereEtiquette(tSymbole * a) ; */

void verifVide(tAutAlMot * a) {
	/* Precondition : a est deterministe complet. */
	/* Si l'automate a possede un etat unique et non final, */
	/* cet etat est supprime. Sinon, l'automate reste intact. */
  if (a->nbEtats == 1 && ! final(a, 0)) {
    videAutomate(a);
    a->nbEtats = 0; 
    a->taille = 0; 
    a->nbEtatsInitiaux = 0;
  }
}




/* Emonde l automate a. L automate d entree est modifie.
 * Precondition : l automate a au moins un etat.
 * Si l'automate de depart etait deja emonde, il est rendu avec
 * le tableau entrantesEtats rempli ; sinon, ce tableau est libere.
 * Si le booleen puits vaut TRUE, l automate a est deterministe
 * complet : on conserve un etat puits, a reste complet, et le sens des
 * transitions par defaut n est pas modifie.
 * Si puits vaut FALSE, on emonde completement a, et le sens des
 * transitions par defaut, s il en reste, est modifie.
 * Si l'automate ne reconnait aucun mot, on renvoie l automate sans etats.
 * Si l automate de depart est acyclique et puits vaut FALSE,
 * l automate resultat est acyclique aussi.
 */

int EmondeAutomate(tAutAlMot * a, BOOL puits) {

  etat nb_etats;
  tTransitions * pt = NULL; 
  tTransitions * to_del, * prochain ;
  tTransitions * prec = NULL ;

  int * go = 0;    /* go[e] == 1 ssi e est accessible */
  int * back = 0;  /* back[e] == 1 ssi e est co-accessible */
  int * inutile;   /* tableau des etats inutiles */
  int * utile;     /* tableau des etats utiles */
  int * no = 0;    /* no[e] est le nouveau numero de l etat e */
  etat nbInutile = 0, nbUtile = 0;
  etat i, j, k;
  tTransitions ** new_out = NULL;
  char * new_sorte = NULL;

  /*printf("EmondeAutomate %s\n", (puits ? "puits" : "sans puits")) ;*/

  if (! a) { die("emondeAutomate: a == NULL\n"); }

  go    = (int *) xcalloc(a->nbEtats, sizeof(int));
  utile = (int *) xcalloc(a->nbEtats, sizeof(int));

  inutile = (int *) xcalloc(a->nbEtats, sizeof(int));
  back    = (int *) xcalloc(a->nbEtats, sizeof(int));
  no      = (int *) xmalloc(a->nbEtats * sizeof(int));

  /* On engendre les etats entrants */

  if(! MAJEtatsEntrants(a)) {
    error("Erreur interne [EmondeAutomate].\n") ;
    return 0 ;
  }

  /* Initialisation du tableau des numÝros */

  for (i = 0; i < a->nbEtats; i++) { no[i] = i; }

  /* On fait le parcours recursif avant */

  for (i = 0; i < a->nbEtats; i++) { if (initial(a, i)) { MarqueEtatSortant(go, i, a); }}

  /* On fait le parcours recursif arriere */

  for (i = 0; i < a->nbEtats; i++) { if (final(a, i)) { MarqueEtatEntrant(back, i, a); } }

  /*printf("Initialisations terminees.\n") ;*/

  /* Maitenant on garde chaque etat present
     a la fois dans les parcours avant et arriere */

  for (i = 0; i < a->nbEtats; i++) {

    if (! (go[i] && back[i])) {    /* si l'etat n'est pas marque dans les 2 */

      inutile[nbInutile++] = i;    /*  parcours, : il est inutile. */

      //      debug("%d/%d is inutile\n", i, a->nbEtats);

    } else { utile[nbUtile++] = i; }
  }

  //  debug("%d inutiles\n", nbInutile);

  if (nbUtile + nbInutile != a -> nbEtats) { erreurInt("EmondeAutomate"); }

  if (! nbUtile) {

    videAutomate(a);
    a->nbEtats = 0; 
    a->taille = 0; 
    a->nbEtatsInitiaux = 0;

  } else if (nbInutile) {

    /*printf("On libere les transitions sortant des etats a retirer.\n") ;*/

    for (i = 0; i < nbInutile; i++) {

      for (pt = a->etats[inutile[i]]; pt; pt = prochain) {

	prochain = pt->suivant;

	if (pt->etiq) {
	  free(pt->etiq->canonique);
	  free(pt->etiq);
	}

	free(pt);
      }

      /* On indique que l'etat va etre detruit */
      no[inutile[i]] = -1;
      for (j = inutile[i] + 1; j < a->nbEtats; j++)
         if(no[j] > 0)
            no[j]--;
    } /* MAJ chainage */


   for (i = 0; i < nbUtile; i++) {

     /* On supprime ou on met a jour les transitions sortant de utile[i] */

     /*printf("etat %d\n", utile[i] + 1) ;*/

     prec = NULL;
     pt = a->etats[utile[i]];

     while (pt) {

       /*Affiche_Symbole(pt -> etiq) ;
         printf(" %d\n", pt -> but + 1) ;*/

       if (no[pt->but] < 0) { /* transition vers etat inutile */

	 if (puits) {

	   pt->but = nbUtile;
	   prec = pt;
	   pt = pt->suivant;

	 } else {

	   to_del = pt; /* On retablit le chainage */
	   if (prec)
	     prec->suivant = pt->suivant;
	   else   /* c'est la premiere transition de cet etat */
	     a->etats[utile[i]] = pt->suivant;
	   pt = pt->suivant;

	   if (to_del->etiq) {
	     free(to_del->etiq->canonique);
	     free(to_del->etiq);
	   }

	   free(to_del);
	 }

       } else {

	 pt->but = (etat) no[pt->but];
	 prec = pt;
	 pt = pt->suivant;
       }
     }
   }

   nb_etats = (puits ? nbUtile + 1 : nbUtile) ;
   new_out = (tTransitions **) xmalloc(nb_etats * sizeof(tTransitions *));

   /* On recopie les pointeurs des etats a garder */

   for (j = 0, k = 0; j < a->nbEtats; j++)
     if (no[j] >= 0)
       new_out[k++] = a->etats[j];

   if (puits) {
     new_out[nbUtile] = (tTransitions *) xmalloc(sizeof(tTransitions));
     new_out[nbUtile]->etiq = NULL ;
     new_out[nbUtile]->but = nbUtile ;
     new_out[nbUtile]->suivant = NULL ;
   }

   free(a->etats);
   a->etats = new_out;

   new_sorte = (char *) xcalloc(nb_etats, sizeof(char)) ; 

   for (j=0, k=0; j < a->nbEtats; j++)
     if (no[j] >= 0)
       new_sorte[k++] = a->type[j];

   if (puits)
     new_sorte[nbUtile] = 0;

   free(a->type);
   a->type = new_sorte;


   if (a->entrantesEtats) {
     libereEntrantesEtats(a) ;
   } else erreurInt("EmondeAutomate") ;

   a->taille = a->nbEtats = nb_etats;

  } else if (a->entrantesEtats) { libereEntrantesEtats(a); }

  /* On libere les differents tableaux utilises */

  free(go);
  free(back);
  free(inutile);
  free(utile);
  free(no);

  if (a->entrantesEtats) { error("EmondeAutomate laisse entrantesEtats.\n"); }

  return 1;
} 

/*************************************************************************/

static int MarqueEtatSortant(int * tab, int e, tAutAlMot * a) {

  tTransitions * pt;

  if ((!tab) || (!a)) { return 0; }

  if(! tab[e]) {/* on n est pas encore passe par l'etat */

    /* on indique que l'on passe par l'etat */

    tab[e] = 1;

    /* On va parcourir recursivement les transitions sortantes */

    pt = a->etats[e];
    while (pt) {
      MarqueEtatSortant(tab, pt->but, a);  
      pt = pt->suivant;
    }
  }

  return 1;                                                           
}


/* Precondition : le tableau entrantesEtats est deja rempli.
 */

static int MarqueEtatEntrant(int *tab, int e, tAutAlMot * a) {

  tTransitions * pt = 0;

  if ((!tab) || (!a)) { return 0; }

  if (! tab[e]) { /* on n est pas encore passe par l'etat */

    /* on indique que l'on passe par l'etat */

    tab[e]=1; 

    /* On va parcourir recursivement les transitions entrantes */

    pt = a->entrantesEtats[e];

    while(pt) {
      MarqueEtatEntrant(tab, pt->but, a);  
      pt = pt->suivant;
    }
  }

  return 1;
}



/* Remplit le tableau entrantesEtats.
 * Precondition : l automate a au moins un etat.
 */

int MAJEtatsEntrants(tAutAlMot * a) {

  etat i ;
  tTransitions * pt, * pt2 ; 

   /* si les transitions entrantes ont deja ete remplies, on les libere avant la MAJ */

  if (a->entrantesEtats) { libereEntrantesEtats(a); }

  a->entrantesEtats = (tTransitions **) xcalloc(a->nbEtats, sizeof(tTransitions *)) ;

   /* initialisation des pointeurs */

  for (i = 0; i < a->nbEtats; i++) { a->entrantesEtats[i] = NULL; }


  /* Pour chaque transition sortante on cree une transition entrante */

  for (i = 0; i < a->nbEtats ; i++) {

    for (pt = a->etats[i]; pt; pt = pt->suivant) {

      /* Cree la transition entrante correspondant a la transition sortante */

      pt2 = (tTransitions *) xcalloc(1, sizeof(tTransitions)) ;
      pt2->but = i;

      /* Les transitions partagent l'etiquette */

      pt2->etiq = pt->etiq;
      pt2->suivant = a->entrantesEtats[pt->but];
      a->entrantesEtats[pt->but] = pt2;
    }
  }

  return 1;
}


/*static int LibereEtiquette(tSymbole * a) {
   if(!a)
      return 0;
free(a -> canonique) ;
   free(a);
a = NULL ;
return 1 ; } */

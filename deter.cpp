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

/* Nom : deter.cpp
 * determinisation d'un automate dans le projet de levee d'ambiguites.
 * 18 juillet 1998
 */

#include <stdio.h>
#include <stdlib.h>

#include "inter.h"
#include "general.h"
#include "autalmot.h"
#include "deter.h"
#include "variable.h"

#include "utils.h"

static etat lgFEtat;	   /* indice de longueur de la table des futurs etats */
static etat indFEtat;	   /* indice courant dans la tables des futurs etats */
static etat tailleFE ;     /* taille de futursEtats a multiplier par facteur */
static int facteur = 4 ;   /* en cas de reallocation */
//static int * nbTrans;

static noeud * * creerEnsemble() ;
static noeud * * agrandirEnsemble(noeud **liste);
static noeud * image(tAutAlMot * autom, noeud *depart, tSymbole * s) ;
static noeud * imageTransAbr(noeud * depart, tSymbole * s) ;
static tBiAlph * initBiAlph(noeud * source, tAutAlMot * au, alphabet * alpha,
   noeud * * butsDef, BOOL * univGlobal) ;
static noeud * etatsInitiaux(tAutAlMot * autom);
static noeud * * ajouter(noeud **FE, noeud *Etatbut);
static void ajouterTransition(tAutAlMot * autom, /*noeud ** Fe,*/ int indice,
   tSymbole * symbole);
static alphabet * lisibles(tBiAlph * biAlph) ;
//static alphabet * developpe(alphabet * d1, alphabet * d2, tBiAlph * biAlph, BOOL * modif);
static noeud * trier(noeud *liste);
void affListe(noeud * liste, FILE * f = stderr);
//static int Liste_Vide(noeud *l) ;
static int Element(noeud *b, noeud **a);
static int egal(noeud *a, noeud *b);
static void RemplirSorte(tAutAlMot * entree, tAutAlMot * sortie, noeud **h);
static noeud * unir(noeud * etatBut, noeud * butsDef) ;
//static void subst(tSymbole *s, tSymbole *i, alphabet * tab, tBiAlph * biAlph);





void bialph_developp(tBiAlph * bialph);


/*********************************************************************/
/* Fonction de determinisation d'un automate. */
/* Prend en entree un automate et renvoie */
/* un automate deterministe equivalent.                 */
/* Preconditions : autEntree n'est pas nul et n'a pas de transitions abregees. */
/*********************************************************************/
tAutAlMot * determinisation(tAutAlMot * autEntree) {
   tAutAlMot * autSortie ;      	 /* automate de sortie */
   noeud * * futursEtats = NULL ; /* tableau des futurs etats en attente */
   /* un futur etat est une liste chainee d etats */
   noeud * etatCourant,	 /* pointeur sur l'etat en cours de traitement */
      * etatBut,
       /* pointeur sur l'etat image de l'etat courant par une transition */
      * EI ;
    	 /* pointeur sur la liste des etats initiaux de l'ancien automate */
   alphabet * alpha,
      /* liste des transitions possibles de l'automate d'entree */
      * s ;		 /* pointeur sur l'alphabet */
   int indBut ;  /* indice de l'etat but dans la table des futurs etats */
   etat i ;
/* printf("\nDeterminisation...\n") ; */
if (! autEntree)
   erreurInt("determinisation") ;
fprintf(fErr, "\n%d etats\n", autEntree -> nbEtats) ;
/* On alloue de l'espace pour le nouvel automate */
autSortie = (tAutAlMot *) calloc(1, sizeof(tAutAlMot));
if (autSortie == NULL)
   erreurMem("determinisation") ;
autSortie -> nbEtats = 0 ;
autSortie -> taille = autEntree -> nbEtats + 1 ;
autSortie -> etats = (tTransitions * *)
   calloc(autSortie -> taille, sizeof(tTransitions *)) ;
if (! autSortie -> etats)
   erreurMem("determinisation") ;
autSortie -> entrantesEtats = 0 ;
   /* On cree la liste des symboles de l'automate */
alpha = CreerAlphabet(autEntree, NULL);
   /* On cree les listes d'etats necessaires a la determinisation */
futursEtats = creerEnsemble() ;
EI = etatsInitiaux(autEntree) ;
/* On trie les etats pour en faciliter l'exploitation */
/* dans les autres fonctions */
EI = trier(EI) ;
/*printf("Creation 1 : ") ;
affListe(EI) ; */
marqueEtatInitial(autSortie) ;
futursEtats = ajouter(futursEtats, EI) ;
/*printf("Initialisations terminees\n") ;*/
/* On cherche les images de tous les ensembles d'etats */
/* engendres par la determinisation */
for (indFEtat = 0 ; indFEtat < lgFEtat ; indFEtat ++) {
   etatCourant = futursEtats[indFEtat] ;
   if (! etatCourant)
      printf("Erreur interne [determinisation], %d vide\n", indFEtat + 1) ;
   if (autSortie -> taille <= indFEtat) {
      autSortie -> taille *= facteur ;
      autSortie -> etats = (tTransitions * *)
         realloc(autSortie -> etats,
            autSortie -> taille * sizeof(tTransitions *)) ;
      if (! autSortie -> etats)
         erreurMem("determinisation") ; }
   autSortie -> etats[indFEtat] = NULL ;
   autSortie -> nbEtats ++ ;
   for (s = alpha ; s ; s = s -> suiv) {
         /* Pour chaque etiquette, on cherche l'image */
         /* de l ensemble d etats */
         etatBut = image(autEntree, etatCourant, s -> etiquette) ;
         etatBut = trier(etatBut) ;
         if (etatBut) {
            indBut = Element(etatBut, futursEtats) ;
            if (indBut == - 1) {
               /*printf("Creation %d : ", lgFEtat + 1) ;
               affListe(etatBut) ;*/
               futursEtats =  ajouter(futursEtats, etatBut) ;
               indBut = lgFEtat - 1 ; }
            ajouterTransition(autSortie, /*futursEtats,*/
               indBut, copieSymbole(s -> etiquette)) ; } } }
/* La determinisation est faite. On remplit l'automate de sortie */
RemplirSorte(autEntree, autSortie, futursEtats) ;
autSortie -> nbEtats = lgFEtat ;
   /* On libere l'espace utilise par tous les tableaux. */
/* On suppose que l'utilisateur de la fonction s'occupe */
/* de liberer l'espace utilise par l'automate d'entree */
while (alpha) {
      s = alpha ;
      alpha = alpha -> suiv ; /* alpha -> etiquette partage avec l automate */
      free(s) ; }
/*printf("Alphabet libere\n") ;*/
for (i = 0 ; i < tailleFE ; i ++) {
   etatCourant = futursEtats[i] ;
   while (etatCourant) {
      etatBut = etatCourant -> suiv ;
      free(etatCourant) ;
      etatCourant = etatBut ; } }
libereAutAlMot(autEntree) ;
fprintf(fErr, "\nDeterminisation OK, %d etats\n", autSortie -> nbEtats) ;
return autSortie ; }




/* Determinise un automate. Prend en entree un automate et renvoie
 * un automate deterministe complet equivalent.
 * autEntree peut avoir des transitions abregees.
 * Detruit l automate donne en entree.
 * Postconditions : l etat initial de l automate deterministe est 0 ;
 * le tableau entrantesEtats n'est pas rempli.
 */

tAutAlMot * deterCompl(tAutAlMot * autEntree) {

  tAutAlMot * autSortie ;      	 /* automate de sortie */
  noeud ** futursEtats = NULL,   /* tableau des futurs etats en attente, un futur etat est une liste chainee d etats */
    * etatCourant,	         /* l'etat en cours de traitement */
    * nTemp,
    * etatBut,   /* l'etat image de l'etat courant par une transition */
    * butsDef,   /* l'etat image par les transitions par defaut */
    * EI ;       /* etats initiaux de l'ancien automate */

  alphabet * alpha, /* liste des etiquettes de l'automate d'entree */
    * alphLoc,     /* alphabet des sorties d un futur etat */
    * s;

  int indBut ;  /* indice de l'etat but dans la table des futurs etats */
  etat i ;
  BOOL transUniv, univ ;
  tBiAlph * biAlph ;
  tTransBiAlph * t, * temp ;


  if (! autEntree) { die("deterCompl: Cet automate est NULL.\n"); }


  /* On alloue de l'espace pour le nouvel automate */

  autSortie = (tAutAlMot *) xcalloc(1, sizeof(tAutAlMot));

  autSortie->nbEtats = 0 ;
  autSortie->taille  = autEntree->nbEtats + 1 ;
  autSortie->etats   = (tTransitions **) xcalloc(autSortie->taille, sizeof(tTransitions *));

  autSortie->entrantesEtats = 0;
  alpha = CreerAlphabet(autEntree, NULL);
  futursEtats = creerEnsemble();

  EI = etatsInitiaux(autEntree);


  /* On trie les etats pour en faciliter l'exploitation */
  /* dans les autres fonctions */

  EI = trier(EI) ;

  //  debug("Creation 1 : "); affListe(EI); fprintf(stderr, "\n");

  marqueEtatInitial(autSortie);
  futursEtats = ajouter(futursEtats, EI);


  /* On cherche les images de tous les ensembles d'etats */
  /* engendres par la determinisation */

  for (indFEtat = 0 ; indFEtat < lgFEtat ; indFEtat++) {

    etatCourant = futursEtats[indFEtat] ;
    //    affListe(etatCourant); fprintf(stderr, "\n");

    if (autSortie->taille <= indFEtat) {
      autSortie->taille *= facteur;
      autSortie->etats   = (tTransitions **) xrealloc(autSortie->etats, autSortie->taille * sizeof(tTransitions *));
    }

    autSortie->etats[indFEtat] = NULL;
    autSortie->nbEtats++;


    if (! etatCourant) { /* etat poubelle */

      ajouterTransition(autSortie, indFEtat, NULL) ;

    } else {

      biAlph = initBiAlph(etatCourant, autEntree, alpha, & butsDef, & univ);

      if (! univ) { /* il n y a pas eu d etiquette universelle, */

	/* on fait la transition avec but par defaut */

	indBut = Element(butsDef, futursEtats) ;

	if (indBut == - 1) {

	  futursEtats = ajouter(futursEtats, butsDef);
	  indBut = lgFEtat - 1;

	} else {

	  while (butsDef) {
	    nTemp = butsDef->suiv;
	    free(butsDef);
	    butsDef = nTemp;
	  }
	}
	ajouterTransition(autSortie, indBut, NULL) ;
      }


      bialph_developp(biAlph);
      // debug("biAlph=\n"); affBiAlph(biAlph);

      alphLoc = lisibles(biAlph) ;

      for (s = alphLoc; s; s = s->suiv) {

	//	Affiche_Symbole(s->etiquette);

	etatBut = imageTransAbr(etatCourant, s->etiquette) ;
	etatBut = trier(etatBut) ;

	if (etatBut == NULL) { /* l etat but est l etat poubelle */
	  die("Erreur interne [deterCompl]\n");
	}

	if ((transUniv = (s->etiquette->sorteSymbole == UNIVERSEL))) {
	  if (butsDef) {
	    etatBut = trier(unir(etatBut, butsDef)) ;
	  }
	}
	indBut = Element(etatBut, futursEtats) ;

	if (indBut == - 1) {

	  futursEtats = ajouter(futursEtats, etatBut) ;
	  indBut = lgFEtat - 1 ;

	} else {

	  while (etatBut) {
	    nTemp = etatBut->suiv;
	    free(etatBut);
	    etatBut = nTemp;
	  }
	}
	ajouterTransition(autSortie, indBut, (transUniv ? NULL : copieSymbole(s->etiquette))) ;
      }

      libereBiAlph(biAlph) ;

      while (alphLoc) {
	s = alphLoc;  /* s->etiquette etait partage avec biAlph */
	alphLoc = alphLoc->suiv;
	free(s);
      }
    }
  }

  /* La determinisation est faite. On remplit l'automate de sortie */

  RemplirSorte(autEntree, autSortie, futursEtats) ;
  autSortie->nbEtats = lgFEtat ;


  while (alpha) {
    s = alpha ;
    alpha = alpha->suiv;
    /*free(s -> etiquette) ;*/ /* le contenu de s -> etiquette est partage avec autEntree, on ne le libere pas */
    free(s) ;
  }

  for (i = 0; i < tailleFE; i++) {

    etatCourant = futursEtats[i] ;

    while (etatCourant) {

      etatBut = etatCourant->suiv ;

      for (t = etatCourant->trans; t; t = temp) {
	temp = t->suiv; /* t -> etiq etait partage avec biAlph, il est deja libere */
	free(t);
      }
      free(etatCourant) ;
      etatCourant = etatBut ;
    }
  }

  autSortie->name = autEntree->name;

  autEntree->name = NULL;
  libereAutAlMot(autEntree);

  return autSortie;
}



/***********************************************************************
 * On cree un ensemble d'etats, pour determiniser l'automate.          *
 * L'ensemble est cree vide. C'est les premiers elements d'une liste   *
 * FIFO que l'on cree. Si on arrive a la fin du tableau,               *
 * on utilisera realloc pour reallouer d autres elements du tableau.   *
 ***********************************************************************/

static noeud * * creerEnsemble() {

  noeud * * liste ;
  tailleFE = facteur ;
  lgFEtat = 0 ;

  liste = (noeud **) xcalloc(tailleFE, sizeof(noeud *)) ;

  return liste ;
}


/*********************************************************
 * On agrandit l'ensemble d un certain nombre d elements *
 *********************************************************/

static noeud ** agrandirEnsemble(noeud ** liste) {

  etat i ;

  liste = (noeud **) xrealloc(liste, facteur * tailleFE * sizeof(noeud *)) ;

  for (i = tailleFE; i < facteur * tailleFE; i++)
    liste[i] = NULL ;

  tailleFE *= facteur ; 

  return liste ;
}


static noeud * unir(noeud * ens1, noeud * ens2) {
/* concatene ens2 a la fin de ens1 */
/* ens1 et ens2 sont non vides */
/* modifie et renvoie ens1 ; ens2 est partage avec une partie de ens1 */
   noeud * n ;
if (! ens1)
   printf("Erreur interne [unir]\n") ;
for (n = ens1 ; n -> suiv ; n = n -> suiv) ;
n -> suiv = ens2 ;
return ens1 ; }

/***************************************************************************/
/* Recherche l'image d'une liste d'etats par une transition donnee */
/* Renvoie NULL ou une liste triee d'elements images              */
/* Suppose qu il n y a pas de transitions abregees dans autom */
/**************************************************************************/
static noeud * image(tAutAlMot * autom, noeud * depart, tSymbole * s) {
   noeud * i,    /* pointeur sur la liste des etats buts */
      * temp ;
   tTransitions * t ;
/* pour chaque etat, on cherche l'image par la transition donnee */
for (i = NULL ; depart ; depart = depart -> suiv)
    /* on cherche les transitions partant de chaque etat de la liste depart */
   for (t = autom -> etats[depart -> nom] ; t ; t = t -> suivant)
      if (! compSymb(t -> etiq, s)) {
         temp = i ;
         i = (noeud *) calloc(1, sizeof(noeud)) ;
	 if (! i)
     	    erreurMem("image") ;
         i -> suiv = temp ;
         i -> nom = t -> but ; }
return i ; }




/* Cree le bi-alphabet des symboles lisibles depuis source.
 * Construit l image de source par les transitions par defaut (butsDef).
 * univGlobal dit s il y a un etat dans source qui a une transition
 * etiquetee par le symbole universel.
 */

static tBiAlph * initBiAlph(noeud * source, tAutAlMot * au, alphabet * alpha, noeud ** butsDef, BOOL * univGlobal) {

  tBiAlph * biAlph = NULL, * alTemp ;
  tTransBiAlph * trTemp ;
  tTransitions * t ;
  BOOL univLocal; /* dit si l etat a une transition etiquetee par le symbole universel : elle prime sur les trans. avec but par def. */
  noeud * temp ;


  //  debug("initBiAlph... "); affListe(source);

  *butsDef    = NULL ;
  *univGlobal = FALSE ;

  for (; source ; source = source->suiv) {

    source->trans = NULL;
    //    fprintf(stderr, "[%d]\n", source->nom);
    univLocal = FALSE;

    for (t = au->etats[source->nom]; t; t = t->suivant) {

      trTemp = (tTransBiAlph *) xcalloc(1, sizeof(tTransBiAlph));

      if (t->etiq) {

	if (! univLocal && t->etiq->sorteSymbole == UNIVERSEL)
	  *univGlobal = univLocal = TRUE;

	if (! (trTemp->etiq = posBiAlph(t->etiq, biAlph))) {

	  alTemp = biAlph ;   /* nouveau symbole */
	  biAlph = (tBiAlph *) xcalloc(1, sizeof(tBiAlph)) ;
	  biAlph->orig  = t->etiq ; /* partage avec l automate */
	  biAlph->devel = (alphabet *) xcalloc(1, sizeof(alphabet)) ;
	  biAlph->devel->etiquette = copieSymbole(t->etiq) ;
	  biAlph->devel->suiv      = NULL ;
	  biAlph->suiv = alTemp ;
	  trTemp->etiq = biAlph ;
	}

      } else {

	trTemp->etiq = NULL ; /* transition avec but par defaut */
	//	debug("%d def %d\n", source->nom, t->but);
      }

      trTemp->but   = t->but ;
      trTemp->suiv  = source->trans ;
      source->trans = trTemp ;
    }


    if (! univLocal) {/* on regarde les trans. avec but par def. */

      for (t = au->etats[source->nom]; t; t = t->suivant) {

	if (! t->etiq) { /* transition avec but par defaut */
	  temp = (noeud *) xcalloc(1, sizeof(noeud)) ;
	  temp->nom  = t->but ;
	  temp->suiv = *butsDef ;
	  *butsDef   = temp ;
	}
      }
    }
  }

  *butsDef = trier(*butsDef) ;

  return biAlph ;
}




/******************************************************/
/* Teste si un symbole s appartient a une liste de symboles */
/* Si c est le cas, renvoie la position de s dans alpha */
/* Sinon, renvoie NULL */
/******************************************************/
tSymbole * posAlph(tSymbole * s, alphabet * a) {
for ( ; a ; a = a -> suiv)
   if (! compSymb(a -> etiquette, s))
      return a -> etiquette ;
return NULL ; }


/* Renvoie la position de s dans biAlph
 * renvoie NULL si s ne figure pas dans biAlph
 */

tBiAlph * posBiAlph(tSymbole * s, tBiAlph * biAlph) {

  for (; biAlph; biAlph = biAlph->suiv) {
    if (compSymb(biAlph->orig, s) == 0) { return biAlph; }
  }

  return NULL;
}



void alphabet_developp(alphabet * d1, alphabet * d2) {

  extern BOOL developpeInter(alphabet * d1, alphabet * d2);

  developpeInter(d1, d2);
}


void bialph_developp(tBiAlph * bialph) {

  tBiAlph  * c1, * c2;
  alphabet * d1, * d2;

  for (c1 = bialph; c1; c1 = c1->suiv) {

    for (d1 = c1->devel; d1; d1 = d1->suiv) {

      for (d2 = d1->suiv; d2; d2 = d2->suiv) { alphabet_developp(d1, d2); }

      for (c2 = c1->suiv; c2; c2 = c2->suiv) {

	for (d2 = c2->devel; d2; d2 = d2->suiv) { alphabet_developp(d1, d2); }
      }
    }
  }
}



/*
static BOOL remplBiAlph(tBiAlph * biAlph) {

  tBiAlph * c1, * c2 ;
  alphabet * d1, * d2 ;
  BOOL modif = FALSE ;

  debug("remplBiAlph...\n");

  for (c1 = biAlph ; c1 ; c1 = c1->suiv) {

    if (! c1->devel) die("Erreur interne [remplBiAlph] 1.\n") ;

    for (d1 = c1->devel; d1; d1 = d1->suiv) {

      for (d2 = d1->suiv; d2; d2 = developpe(d1, d2, biAlph, & modif));

      for (c2 = c1->suiv; c2; c2 = c2->suiv) {

	if (! c2->devel) die("Erreur interne [remplBiAlph] 2.\n") ;

	for (d2 = c2->devel; d2; d2 = developpe(d1, d2, biAlph, & modif));
      }
    }
  }

  return modif ;
}
*/


static alphabet * lisibles(tBiAlph * biAlph) {

  alphabet * d, * res, * temp ;

  for (res = NULL ; biAlph ; biAlph = biAlph -> suiv)
    for (d = biAlph -> devel ; d ; d = d -> suiv)
      if (! posAlph(d -> etiquette, res)) {
	/* Affiche_Symbole(d -> etiquette) ;*/
	temp = res ;
	res = (alphabet *) calloc(1, sizeof(alphabet)) ;
	if (! res)
	  erreurMem("lisibles") ;
	res -> etiquette = d -> etiquette ;  /* partage */
	res -> suiv = temp ; }
  /* printf("lisibles OK\n") ;*/
  return res ;
}

void libereBiAlph(tBiAlph * a) {

  tBiAlph * alTemp ;
  alphabet * a1, * a2 ;

  while (a) {

    alTemp = a->suiv ; /* a -> orig est partage avec l automate */

    for (a1 = a->devel ; a1 ; a1 = a2) {
      a2 = a1->suiv;
      free(a1->etiquette->canonique);
      a1->etiquette->canonique = 0;
      free(a1->etiquette);
      a1->etiquette = 0;
      free(a1);
    }

    free(a);
    a = alTemp;
  }
}


#if 0

/* compare d1 et d2 et les developpe si necessaire.
 * renvoie la prochaine valeur de d2 a essayer.
 */

static alphabet * developpe(alphabet * d1, alphabet * d2, tBiAlph * biAlph, BOOL * modif) {

  alphabet * prochain, * tab;
  tSymbole * i;

  if (compSymb(d1->etiquette, d2->etiquette) && (d1->etiquette->sorteSymbole != ATOME || d2->etiquette->sorteSymbole != ATOME)
      && d1->etiquette->sorteSymbole != UNIVERSEL && d2->etiquette->sorteSymbole != UNIVERSEL
      && (i = inter(d1->etiquette, d2->etiquette))) {

    /* si l une des 2 est universelle, on la laisse, */
    /* on la remplacera par une transition avec but par defaut */

    *modif = TRUE ;
    prochain = d2->suiv ;

    if (compSymb(d1->etiquette, i)) { 

      if (! d1->etiquette->sorteSymbole) erreurInt("developpe") ;

      tab = sauf(d1->etiquette, i) ;

      /* d1 est a remplacer par i et tab */
      subst(copieSymbole(d1->etiquette), i, tab, biAlph);
    }

    if (compSymb(d2->etiquette, i)) {

      if (! d2->etiquette->sorteSymbole) erreurInt("developpe") ;

      tab = sauf(d2->etiquette, i) ;

      /* d2 est a remplacer par i et tab */
      subst(copieSymbole(d2->etiquette), i, tab, biAlph) ;
    }

   free(i->canonique) ;
   free(i) ;

   return prochain;

  } else  { /* d1 et d2 sont egaux ou disjoints ou l un des 2 est universel */

    return d2->suiv;
  }
} 



/* Remplace, dans biAlph, chaque occurrence de s par i et tab.
 * Reutilise tab mais pas i. Libere s.
 */

static void subst(tSymbole * s, tSymbole * i, alphabet * tab, tBiAlph * biAlph) {

  alphabet * c1, * temp ;

  tBiAlph * dbg = biAlph;

  debug("subst:"); Affiche_Symbole(s); fprintf(stderr, " = "); 
  Affiche_Symbole(i); fprintf(stderr, " union "); alphabet_dump(tab);
  fprintf(stderr, "\n");

  debug("avant:\n"); affBiAlph(biAlph);

  for (; biAlph ; biAlph = biAlph->suiv) {

    for (c1 = biAlph->devel; c1; c1 = c1->suiv) {

      if (compSymb(c1->etiquette, s) == 0) {
	temp = c1->suiv ;
	free(c1->etiquette->canonique) ;
	free(c1->etiquette) ;
	c1->etiquette = copieSymbole(i) ;
	c1->suiv = tab ;

	while (c1->suiv) { c1 = c1->suiv; }
	c1->suiv = temp;
      }
    }
  }

  free(s->canonique);
  free(s) ;

  debug("apres:\n"); affBiAlph(dbg);
}


#endif


/*******************************************************************
 * Recherche l'image d'une liste d'etats par une transition donnee *
 * Renvoie NULL ou une liste triee d'elements images               *
 * Il peut y avoir des transitions abregees dans autom             *
 *******************************************************************/

static noeud * imageTransAbr(noeud * depart, tSymbole * s) {
   noeud * i,    /* pointeur sur la liste des etats buts */
      * temp ;
   tTransBiAlph * t ;
   alphabet * d ;
   BOOL trouve ;
/*printf("imageTransAbr...") ;
Affiche_Symbole(s) ;*/
/* pour chaque etat, on construit l'image par s */
for (i = NULL ; depart ; depart = depart -> suiv) {
   trouve = FALSE ;
    /* transitions partant de cet etat dans l ancien automate : */
   for (t = depart -> trans ; t ; t = t -> suiv)
      if (t -> etiq)
         for (d = t -> etiq -> devel ; d ; d = d -> suiv)
            if (! compSymb(d -> etiquette, s) ||
                  d -> etiquette -> sorteSymbole == UNIVERSEL) {
               /*printf("%d sym %d\n", depart -> nom + 1, t -> but + 1) ;*/
               temp = i ;
               i = (noeud *) calloc(1, sizeof(noeud)) ;
	       if (! i)
     	          erreurMem("imageTransAbr") ;
               i -> suiv = temp ;
               i -> trans = NULL ;
               i -> nom = t -> but ;
               trouve = TRUE ; }
   if (! trouve)
      for (t = depart -> trans ; t ; t = t -> suiv)
         if (! t -> etiq) {
            /*printf("%d def %d\n", depart -> nom + 1, t -> but + 1) ;*/
            temp = i ;
            i = (noeud *) calloc(1, sizeof(noeud)) ;
            if (! i)
     	       erreurMem("imageTransAbr") ;
            i -> suiv = temp ;
            i -> trans = NULL ;
            i -> nom = t -> but ; } }
/*printf("imageTransAbr OK\n") ;*/
return i ; }


/* Renvoie la liste des etats initiaux de l'automate d'entree
 */

static noeud * etatsInitiaux(tAutAlMot * autom) {

  noeud * courant, * nouveau, * premier ;
  unsigned int compte ;

  if (! autom->nbEtatsInitiaux)
    die("Erreur [etatsInitiaux], pas d etats initiaux.\n") ;

  premier = (noeud *) xcalloc(1, sizeof(noeud)) ;

  premier->nom   = autom->initial[0];
  premier->suiv  = NULL;
  premier->trans = NULL;
  courant        = premier;

  for (compte = 1; compte < autom->nbEtatsInitiaux; compte++) {
    nouveau = (noeud *) xcalloc(1, sizeof(noeud)) ;
    courant->suiv = nouveau ;
    courant       = courant->suiv ;
    courant->nom   = autom->initial[compte] ;
    courant->trans = NULL ;
    courant->suiv  = NULL ;
  }

  return premier ;
}



/********************************************************/
/* Ajoute Etatbut dans le tableau FE qui a tailleFE places */
/* dont lgFEtat sont deja occupees  */
/********************************************************/
static noeud * * ajouter(noeud * * FE, noeud * Etatbut) {
if (! FE)
   printf("Erreur interne [ajouter]\n") ;
if (lgFEtat == tailleFE)
   FE = agrandirEnsemble(FE) ;
lgFEtat ++ ;
FE[lgFEtat - 1] = Etatbut ;
return FE ; }


/***********************************************************************/
/* Ajoute une transition symbole de l'etat indFEtat vers l'etat indice */
/***********************************************************************/

static void ajouterTransition(tAutAlMot * autom, /*noeud * * Fe,*/  int indice, tSymbole * symbole) { /* Le parametre Fe semble inutilise */

  tTransitions * temp ;

  /*printf("Transition de %d par %d vers %d\n", indFEtat + 1, symbole, indice + 1) ;*/

  /*
  debug("ajouter transition:");
  fprintf(stderr, "(%d, ", indFEtat); Affiche_Symbole(symbole); fprintf(stderr, ", %d)\n", indice);
  */

  temp = (tTransitions *) xcalloc(1, sizeof(struct strTransitions));
  temp->etiq = symbole;
  temp->but  = indice;
  temp->suivant = autom->etats[indFEtat];
  autom->etats[indFEtat] = temp;
}


/*********************************************************************/
/* Cree la liste des symboles utilises dans l'automate de depart */
/* S il y a des transitions par defaut, NULL n est pas considere */
/* comme un symbole de l alphabet */
/* Il y a partage de memoire entre alpha et autom */
/*********************************************************************/
alphabet * CreerAlphabet(tAutAlMot * autom, alphabet * alpha) {
   alphabet * nveau ;
   tTransitions * t;
   etat i = 0;
/*   int compte = 0 ;*/
for (i = 0 ; i < autom -> nbEtats ; i++)
   for (t = autom -> etats[i] ; t ; t = t -> suivant)
      if (t -> etiq && ! posAlph(t -> etiq, alpha)) {
         nveau = (alphabet *) calloc (1, sizeof(alphabet));
   	 if (nveau == NULL)
	    erreurMem("CreerAlphabet") ;
	 nveau -> etiquette = t -> etiq ;   /* partage */
	 nveau -> suiv = alpha ;
	 alpha = nveau ;
         /*compte ++ ;*/ }
/*printf("%d symboles.\n", compte) ;*/
return alpha ; }

/*************************************************************/
/* Trie une liste de noeuds dans l'ordre croissant des etats */
/*************************************************************/
static noeud * trier(noeud * liste) {
   noeud * crt, * p, * temp ;
   etat nomTemp ;
/*affListe(liste) ;*/
for (crt = liste ; crt ; crt = crt -> suiv)
   while ((crt -> suiv) && (crt -> suiv -> nom < crt -> nom))
      if (crt -> suiv -> nom < liste -> nom) { /* insertion en debut */
         /*printf("%d < %d, insertion en tete\n",
            crt -> suiv -> nom + 1, crt -> nom + 1) ;*/
	 nomTemp = liste -> nom ;   /* 1er nom */
         liste -> nom = crt -> suiv -> nom ;  /* nouveau nom */
         crt -> suiv -> nom = nomTemp ;
         if (liste != crt) {
            temp = liste -> suiv ;  /* contient le 2eme */
            liste -> suiv = crt -> suiv ;   /* contient le 1er */
	    crt -> suiv = liste -> suiv -> suiv ;
	    liste -> suiv -> suiv  = temp ; }
         /*affListe(liste) ;*/ }
      else { /* autres cas d'insertion */
         /*printf("%d < %d, insertion apres tete\n",
            crt -> suiv -> nom + 1, crt -> nom + 1) ;*/
	 for (p = liste ; p->suiv -> nom < crt->suiv -> nom ; p = p->suiv) ;
         /* p -> nom < crt -> suiv -> nom == p -> suiv -> nom */
         /* on va inserer le nouveau apres p */
         temp = p -> suiv ;  /* contient le successeur de p */
         p -> suiv = crt -> suiv ;   /* contient le nouveau */
	 crt -> suiv = p -> suiv -> suiv ;
	 p -> suiv -> suiv  = temp ;
         /*affListe(liste) ;*/ }
/*affListe(liste) ;*/
for (crt = liste ; crt ; crt = crt -> suiv)
   while (crt -> suiv && crt -> nom == crt -> suiv -> nom) {
      temp = crt -> suiv ;
      crt -> suiv = temp -> suiv ;
      free(temp) ; }
/*affListe(liste) ;*/
return liste ; }


void affListe(noeud * liste, FILE * f) {

  noeud * courant ;

  fprintf(f, "[ ") ;

  for (courant = liste ; courant ; courant = courant->suiv) { fprintf(f, "%d, ", courant->nom); }

  fprintf(f, "]") ;
}


/*******************************************************/
/* Fonction permettant de savoir si une liste est vide */
/*******************************************************/
#if 0
static int Liste_Vide(noeud *l)
{
	int OK=1;
	if (l==NULL)
		OK=0;
	return OK;
}
#endif

/*******************************************************************/
/* Fonction qui permet de mettre a jour le tableau sorte du nouvel */ 
/* automate en cas de presence d'un etat terminal dans EI          */
/*******************************************************************./
static void Rendre_Terminal(noeud *EI, tAutAlMot * nouveau)
{
	int ret;
	ret=Liste_Vide(EI);
	if (ret==0)
	{
		printf("La liste est vide!!!\n");
	}
	else
	{
		while(EI->suiv!=NULL)
		{
                        nouveau->sorte[EI->nom]='3';
		}
        }
}
*/


/********************************************************************/
/** Fonction qui determine si une liste de noeuds appartient a un  **/
/** tableau de liste                                               **/
/********************************************************************/
static int Element(noeud *b, noeud **a)
{
etat i = 0;
while( i<lgFEtat )
	{
	if( egal(a[i], b))
		return i;
	else
		i++;
	}
return(-1);
}


static int egal(noeud *a, noeud *b)
{
int ok=1;
noeud *a1, *b1;
b1 = b;
a1 = a;
while(ok)
	{
	if((a1 == NULL) && (b1 == NULL))
		return 1;
	if (((a1 == NULL) &&(b1 != NULL)) || ((a1 != NULL) && (b1 == NULL)))
		ok = 0;
	else
		{
		if(a1->nom == b1->nom)
			{
			a1=a1->suiv;
			b1=b1->suiv;
			}
		else
			ok=0;
		}
	}
return 0;
}



/* Fonction permettant de mettre a jour le tableau sorte du nouvel automate
 * Precondition : sortie -> nbEtats > 0.
 */

static void RemplirSorte(tAutAlMot * entree, tAutAlMot * sortie, noeud ** h) {

   etat i ;
   BOOL trouve ;
   noeud * n ;

   sortie->type = (char *) xcalloc(sortie->nbEtats, sizeof(char)) ;

   sortie->type[0] = AUT_INITIAL ;

   /* si etat initial et terminal */

   for (trouve = FALSE, n = h[0] ; n && ! trouve ; n = n->suiv)
     if (final(entree, n->nom)) { //entree->sorte[n->nom] % 2) {
       sortie->type[0] = AUT_INITIAL | AUT_FINAL; //INITIAL_TERMINAL ;
       trouve = TRUE;
     }

   for(i = 1 ; i < sortie -> nbEtats ; i ++) {
     sortie->type[i] = 0; //NON_INITIAL_NON_TERMINAL ;
     for (trouve = FALSE, n = h[i] ; n && ! trouve ; n = n -> suiv)
       if (final(entree, n->nom)) { //entree -> sorte[n -> nom] % 2) {
         sortie->type[i] = AUT_FINAL;
         trouve = TRUE ;
       }
   }
}


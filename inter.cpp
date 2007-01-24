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

/*************************************************************************
 *                                  inter.cpp                      v0.02 *
 *                         Intersection d'automates                      *
 *                                                                       *
 *                       (c) 1997 S. SAUPIQUE - MagiX                    *
 *************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include "general.h"
#include "autalmot.h"
#include "inter.h"
#include "deter.h"
#include "variable.h"

#include "utils.h"

#ifndef max
#define max(a, b) ((a)>(b))?(a):(b)
#endif

#ifndef min
#define min(a, b) ((a)<(b))?(a):(b)
#endif

static void interEtat(tAutAlMot * ret, int * * numero, tAutAlMot * a, int i, tAutAlMot * b, int j, BOOL * * deja, alphabet * alpha) ;
static void interEtatAtome(tAutAlMot * ret, int * * numero, tAutAlMot * a, int i, tAutAlMot * b, int j, BOOL * * deja) ;
static tTransBiAlph * initBiAlphInter(tAutAlMot * aut, etat e, alphabet * alpha, tBiAlph * * biAlph) ;
static BOOL compBiAlph(tBiAlph * biAlph1, tBiAlph * biAlph2) ;
BOOL developpeInter(alphabet * d1, alphabet * d2) ;
static void remplace(alphabet * d, tSymbole * i, alphabet * tab) ;
static void creerTrans(tAutAlMot * a, etat i, tAutAlMot * b, etat j, tSymbole * s, etat k, etat l, tAutAlMot * ret, int * * numero) ;
static void creerEtat(tAutAlMot * r, int * * numero, tAutAlMot * a, etat i, tAutAlMot * b, etat j) ;

/*static int MemeEtiquette(tSymbole * a, tSymbole * b) ;*/
/*static int TestSorte(int a, int b) ;*/
                                                                

void alphabet_dump(alphabet * a, FILE * f) {

  fputs("[ ", f);

  while (a) {
    Affiche_Symbole(a->etiquette, f);
    fputs(", ", f);
    a = a->suiv;
  }

  fputs(" ]", f);
}




void affBiAlph(tBiAlph * b, FILE * f) {

  for (; b ; b = b->suiv) {
    Affiche_Symbole(b->orig, f);
    alphabet_dump(b->devel, f);
    putc('\n', f);
 }
}





/* Ne modifie pas les automates d entree a et b 
 * Precondition : Les automates a et b sont deterministes et complets.
 * Postconditions : l automate intersection est deterministe et complet ;
 * s'il a des etats, son etat initial est 0 ;
 * le tableau entrantesEtats n'est pas rempli.
 */

tAutAlMot * interAut(tAutAlMot * a, tAutAlMot * b) {

  BOOL ** deja   = NULL;
  int  ** numero = NULL;
  etat i, j;
  tAutAlMot * ret = NULL;
  alphabet * alpha, * tmp;


  /*
    static int dbgi = 0;
    char buf[128];

    dbgi++;
  */

  /*
  debug("****************************************************\n");
  debug("interAut [%d]\n", dbgi);

  debug("a:\n");
  autalmot_dump_plain(a);
  debug("b:\n");
  autalmot_dump_plain(b);
  */


  /*
  sprintf(buf, "inter%d-a.dot", dbgi);
  autalmot_dump_dot_fname(a, buf);
  sprintf(buf, "inter%d-b.dot", dbgi);
  autalmot_dump_dot_fname(b, buf);
  */

  // debug("anb=%d, b->nb=%d\n", a->nbEtats, b->nbEtats);


  if (! a->nbEtats || ! b->nbEtats) {

    ret = (tAutAlMot *) xcalloc(1, sizeof(tAutAlMot)) ;
    ret->nbEtats = 0;
    ret->taille  = 0;
    ret->etats   = 0;
    ret->type    = 0;
    ret->entrantesEtats = 0;
    ret->initial = 0;
    ret->nbEtatsInitiaux = 0;

    return ret ;
  }

  numero = (int **)  xmalloc(a->nbEtats * sizeof(int *));
  deja   = (BOOL **) xmalloc(a->nbEtats * sizeof(int *));

  for (i = 0; i < a->nbEtats; i++) {

    deja[i]   = (int *) xcalloc(b->nbEtats, sizeof(int));
    numero[i] = (int *) xmalloc(b->nbEtats * sizeof(int));

    for (j = 0; j < b->nbEtats; j++) {
      numero[i][j] = -1;  /* l etat a(i), b(j) n'existe pas */
    }
  }

  /* On cree le nouvel automate */

  ret = initAutAlMot(a->nbEtats * b->nbEtats);

  ret->nbEtats = 0;
  alpha = CreerAlphabet(a, NULL);
  alpha = CreerAlphabet(b, alpha);

  creerEtat(ret, numero, a, i = a->initial[0], b, j = b->initial[0]);

  interEtat(ret, numero, a, i, b, j, deja, alpha);


  ret->taille = ret->nbEtats;
  ret->etats  = (tTransitions **) xrealloc(ret->etats, ret->nbEtats * sizeof(tTransitions *)) ;
  ret->type   = (char *) xrealloc(ret->type, ret->nbEtats * sizeof(char)) ;


  /* On libere deja et numero */

  for (i = 0; i < a->nbEtats; i++) {
    free(deja[i]);
    deja[i]=NULL;
    free(numero[i]);
    numero[i]=NULL;
  }

  free(deja);
  deja=NULL;
  free(numero);
  numero=NULL;

  while (alpha) {
    tmp = alpha->suiv;
    free(alpha);  /* alpha -> etiquette est partage avec a ou b */
    alpha = tmp;
  }

  /*
  sprintf(buf, "inter%d-out.dot", dbgi);
  autalmot_dump_dot_fname(ret, buf);

  debug("out of interAut\nres=\n");
  autalmot_dump_plain(ret);
  debug("****************************************************\n");
  */

  return ret ;
}



/* Precondition : l etat a(i), b(j) a ete cree et a recu un numero.
 */

static void interEtat(tAutAlMot * ret, int ** numero, tAutAlMot * a, int i, tAutAlMot * b, int j, BOOL ** deja, alphabet * alpha) {

  tBiAlph * biAlphA, * biAlphB;
  tTransBiAlph * trA, * trA0, * trB, * trB0, * univA, * univB, * parDefautA, * parDefautB;
  alphabet * da, * db;
  BOOL trouve;

  //  debug("interEtat(%d,%d)\n", i, j);

  if (deja[i][j]) {  /* Si on a deja traite l'etat a(i), b(j), on sort  */
    return;
  }


  deja[i][j] = TRUE ;

  trA0 = initBiAlphInter(a, i, alpha, & biAlphA);
  trB0 = initBiAlphInter(b, j, alpha, & biAlphB);

  /*
    debug("alpha a[%d]:\n", i);
    affBiAlph(biAlphA);

    debug("alpha b[%d]:\n", j);
    affBiAlph(biAlphB);
  */


  if (! compBiAlph(biAlphA, biAlphB)) {
    error("\na=\n"); affBiAlph(biAlphA);
    error("\nb=\n"); affBiAlph(biAlphB);
    die("\ninterEtat: error with compBiAlph(a,b) [%d, %d]\n", i, j);
    return;
  }

  /*
    debug("after compBiAlph:\n");
    debug("alpha a[%d]:\n", i);
    affBiAlph(biAlphA);
    debug("alpha b[%d]:\n", j);
    affBiAlph(biAlphB);
  */


  parDefautA = parDefautB = univA = univB = NULL ;


  for (trB = trB0; trB && (! parDefautB || ! univB); trB = trB->suiv) {

    if (trB->etiq) {

      for (db = trB->etiq->devel ; db && ! univB ; db = db->suiv) {

	if (db->etiquette->sorteSymbole == UNIVERSEL) { univB = trB; }
      }

    } else { parDefautB = trB ; }
  }


  for (trA = trA0; trA; trA = trA->suiv) {

    if (trA->etiq) {

      for (da = trA -> etiq -> devel ; da ; da = da -> suiv) {

	trouve = FALSE ;

	if (! univA && da -> etiquette -> sorteSymbole == UNIVERSEL) { univA = trA; }

	for (trB = trB0 ; trB ; trB = trB -> suiv) {

	  if (trB -> etiq) {

	    for(db = trB -> etiq -> devel ; db ; db = db -> suiv) {

	      if(! compSymb(da -> etiquette, db -> etiquette)) {
		creerTrans(a, i, b, j, da->etiquette, trA->but, trB->but, ret, numero) ;
		trouve = TRUE ;
		interEtat(ret, numero, a, trA->but, b, trB->but, deja, alpha) ;
	      }
	    }
	  }
	}

	if (! trouve) { /* on utilise la trans. par defaut ou univ. */

	  /*
	    Affiche_Symbole(da -> etiquette) ; printf(" non trouvÝ\n") ;
	  */

	  if (! univB && ! parDefautB)
	    error("Erreur interne [interEtat] 2\n");

	  creerTrans(a, i, b, j, da -> etiquette, trA -> but, (univB ? univB : parDefautB) -> but, ret, numero) ;
	  interEtat(ret, numero, a, trA -> but, b, (univB ? univB : parDefautB) -> but, deja, alpha) ;
	}
      }
    } else { parDefautA = trA ; }
  }


  for (trB = trB0 ; trB ; trB = trB -> suiv) {

    if (trB -> etiq) {

      for (db = trB -> etiq -> devel ; db ; db = db -> suiv) {

	trouve = FALSE ;

	for (trA = trA0 ; trA && ! trouve ; trA = trA -> suiv) {

	  if (trA -> etiq) {

	    for(da = trA -> etiq -> devel ; da && ! trouve ; da = da -> suiv) {
	      if(! compSymb(da -> etiquette, db -> etiquette))
		trouve = TRUE ;
	    }
	  }
	}

	if (! trouve) { /* on utilise la trans. par defaut ou univ. */

	  if (! univA && ! parDefautA)
	    error("Erreur interne [interEtat] 3\n") ;

	  creerTrans(a, i, b, j, db -> etiquette, (univA ? univA : parDefautA) -> but, trB -> but, ret, numero) ;
	  interEtat(ret, numero, a, (univA ? univA : parDefautA) -> but, b, trB -> but, deja, alpha) ;
	}
      }
    }
  }

  if ((univA || parDefautA) && (univB || parDefautB)) {
    creerTrans(a, i, b, j, NULL, (univA ? univA : parDefautA) -> but, (univB ? univB : parDefautB) -> but, ret, numero) ;
    interEtat(ret, numero, a, (univA ? univA : parDefautA) -> but, b, (univB ? univB : parDefautB) -> but, deja, alpha) ;
  }

  libereBiAlph(biAlphA) ;
  libereBiAlph(biAlphB) ;

  for (trA = trA0; trA; trA = trB) {
    trB = trA->suiv;
    free(trA);
  }

  for (trB = trB0; trB; trB = trA) {
    trA = trB->suiv;
    free(trB);
  }
}      


/* Cree le bi-alphabet des symboles lisibles depuis l etat e
 * Renvoie la liste des transitions sortant de e
 * le champ orig est partage avec alpha et avec les 2 automates
 * Le champ devel est initialise avec une copie du champ orig
 */

static tTransBiAlph * initBiAlphInter(tAutAlMot * aut, etat e, alphabet * alpha, tBiAlph ** biAlph) {

  tBiAlph * alTemp;
  tTransBiAlph * trTemp, * tr = NULL;
  tTransitions * t;

  *biAlph = NULL;

  for (t = aut->etats[e]; t; t = t->suivant) {

    trTemp = (tTransBiAlph *) xmalloc(sizeof(tTransBiAlph));

    if (t->etiq) {

      if (! (trTemp->etiq = posBiAlph(t->etiq, *biAlph))) {

	alTemp = *biAlph;
	*biAlph = (tBiAlph *) xcalloc(1, sizeof(tBiAlph));
	(*biAlph)->orig  = posAlph(t->etiq, alpha);  /* partage */
	(*biAlph)->devel = (alphabet *) xcalloc(1, sizeof(alphabet));
	(*biAlph)->devel->etiquette = symbole_dup(t->etiq);
	(*biAlph)->devel->suiv = NULL;
	(*biAlph)->suiv = alTemp;
	trTemp->etiq = *biAlph;

      } else { die("Erreur interne [initBiAlphInter]\n"); }

    } else {   /* transition avec but par defaut */

      trTemp->etiq = NULL;
    }

    trTemp->but  = t->but ;
    trTemp->suiv = tr ;
    tr = trTemp ;
  }

  return tr ;
}



static BOOL compBiAlph(tBiAlph * biAlph1, tBiAlph * biAlph2) {

  tBiAlph  * c1, * c2;
  alphabet * d1, * d2;


  for (c1 = biAlph1; c1; c1 = c1->suiv) {

    if (! c1->devel) { error("Erreur interne [compBiAlph] 1.\n"); }

    for (d1 = c1->devel; d1; d1 = d1->suiv) {

      for (c2 = biAlph2; c2; c2 = c2->suiv) {

	if (! c2->devel) { error("Erreur interne [compBiAlph] 2.\n"); }

	for (d2 = c2->devel ; d2 ; d2 = d2->suiv) {
	  if (! developpeInter(d1, d2)) {
	    error("Erreur interne [compBiAlph] 3 :\n");
	    Affiche_Symbole(c1->orig);
	    Affiche_Symbole(d1->etiquette);
	    Affiche_Symbole(c2->orig);
	    Affiche_Symbole(d2->etiquette);
	    die("");
	  }
	}
      }
    }
  }

  return TRUE ;
}



/* compare d1 et d2 et les developpe si necessaire.
 * quand on developpe un symbole, on le remplace par une
 * suite de symboles 2 a 2 disjoints dont l union constitue
 * le symbole d origine.
 */

BOOL developpeInter(alphabet * d1, alphabet * d2) {

  tSymbole * i;
  alphabet * tab;

  if (compSymb(d1->etiquette, d2->etiquette) && (d1->etiquette->sorteSymbole != ATOME || d2->etiquette->sorteSymbole != ATOME)
      && d1->etiquette->sorteSymbole != UNIVERSEL && d2->etiquette->sorteSymbole != UNIVERSEL
      && (i = inter(d1->etiquette, d2->etiquette))) {

    /* si l une des 2 est universelle, on la laisse ; s il le faut, */
    /* on la remplacera par une transition avec but par defaut      */

    if (compSymb(d1->etiquette, i)) { /* d1 non inclus dans d2 */

      if (! d1->etiquette->sorteSymbole) { erreurInt("developpeInter"); }

      tab = sauf(d1->etiquette, i);

      if (tab == NULL) {
	error("error with: "); Affiche_Symbole(d1->etiquette); error(" sauf "); Affiche_Symbole(i);
   error("\n");
	fatal_error("developpeInter: error with sauf\n"); 
      }

      remplace(d1, i, tab) ;   /* d1 est a remplacer par i et tab */
    }

    if (compSymb(d2->etiquette, i)) { /* d2 non inclus dans d1 */

      // debug("d2 in d1\n");

      if (! d2->etiquette->sorteSymbole) { erreurInt("developpeInter"); }

      tab = sauf(d2->etiquette, i) ;

      if (tab == NULL) {
	error("error with: "); Affiche_Symbole(d2->etiquette); error(" sauf "); Affiche_Symbole(i);
   error("\n");
	fatal_error("developpeInter: error with sauf\n"); 
      }

      remplace(d2, i, tab) ; /* d2 est a remplacer par i et tab */
    }

    symbole_delete(i);
  }
  return TRUE ;
}



/* Remplace d par i et tab. Reutilise tab mais pas i. */

static void remplace(alphabet * d, tSymbole * i, alphabet * tab) {

  alphabet * tabend;

  free(d->etiquette->canonique) ;
  free(d->etiquette) ;

  d->etiquette = copieSymbole(i) ;

  /* on insert tab dans l'alphabet */

  for (tabend = tab; tabend->suiv; tabend = tabend->suiv);
  tabend->suiv = d->suiv;
  d->suiv      = tab ;
}




/* Precondition : l etat a(i), b(j) a ete cree et marque comme traite.
 * Postcondition : l'etat a(k), b(l) a ete cree.
 */

static void creerTrans(tAutAlMot * a, etat i, tAutAlMot * b, etat j, tSymbole * s, etat k, etat l, tAutAlMot * ret, int * * numero) {

  tTransitions * pt;

  if ((numero[k][l]) < 0) {
    creerEtat(ret, numero, a, k, b, l);
  }

  /* par prudence, il faudrait verifier que la transition n'existe pas */
  /*
    for (pt = ret -> etats[numero[i][j]] ; pt ; pt = pt -> suivant)
    if (! compSymb(pt -> etiq, s)) {
    int ea, eb ;
    BOOL trouve = FALSE ;
    printf("Erreur interne [creerTrans] : transition deja presente\n") ;
    printf("Etat source : %d = (%d, %d)\n",
    numero[i][j] + 1, i + 1, j + 1) ;
    Affiche_Symbole(s) ;
    for (ea = 0 ; ! trouve && ea < a -> nbEtats ; ea ++)
    for (eb = 0 ; ! trouve && eb < b -> nbEtats ; eb ++)
    if (numero[ea][eb] == pt -> but)
    trouve = TRUE ;
    if (! trouve)
    printf("\nEtat but : %d, inexistant\n", pt -> but + 1) ;
    else
    printf("\nEtat but : %d = (%d, %d)\n", pt -> but + 1, ea+1, eb+1) ;
    printf("nouvel etat but : %d = (%d, %d)\n",
    numero[k][l] + 1, k + 1, l + 1) ;
    return ; }
  */

  pt = (tTransitions *) xmalloc(sizeof(struct strTransitions));

  pt->suivant = ret->etats[numero[i][j]];
  ret->etats[numero[i][j]] = pt;
  pt->but = numero[k][l];
  pt->etiq = (s ? copieSymbole(s) : NULL);
}



static void creerEtat(tAutAlMot * r, int ** numero, tAutAlMot * a, etat i, tAutAlMot * b, etat j) {

  etat e = numero[i][j] = r->nbEtats++;

  r->etats[e] = NULL;
  r->type[e]  = 0;

  /* Il est initial ssi i et j sont initiaux */

  if (initial(a, i) && initial(b, j)) { r->type[e] |= AUT_INITIAL; }

  /* il est final ssi i et j sont finaux */

  if (final(a, i) && final(b, j)) { r->type[e] |= AUT_FINAL; }
}



/* Ne modifie pas les automates d entree a et b.
 * Preconditions : l'automate d entree a est deterministe ;
 * dans l'automate a toutes les etiquettes sont des atomes ;
 * l'automate d entree b est deterministe et complet ;
 * a et b ont au moins un etat chacun. 
 * Postconditions : l automate intersection est deterministe ;
 * son etat initial est 0 ; le tableau entrantesEtats n'est pas rempli.
 * Si a est acyclique, l automate intersection est acyclique.
 */

tAutAlMot * interAutAtome(tAutAlMot * a, tAutAlMot * b) {

   BOOL ** deja;
   int ** numero;
   etat i, j;
   tAutAlMot * ret;

   /*
     static int dbgi = 0;
     char buf[128];


   dbgi++;

   sprintf(buf, "interatome%d-a.dot", dbgi);
   autalmot_dump_dot_fname(a, buf);

   sprintf(buf, "interatome%d-b.dot", dbgi);
   autalmot_dump_dot_fname(b, buf);
   */

   numero = (int **) xcalloc(a->nbEtats, sizeof(int *)) ;
   deja   = (int **) xcalloc(a->nbEtats, sizeof(int *)) ;

   for (i = 0; i < a->nbEtats; i++) {

     deja[i]   = (int *) xcalloc(b->nbEtats, sizeof(int)) ;
     numero[i] = (int *) xcalloc(b->nbEtats, sizeof(int)) ;

     for (j = 0; j < b->nbEtats; j ++) {
       numero[i][j] = -1;  /* l etat a(i), b(j) n'existe pas */
     }
   }

   ret = initAutAlMot(a->nbEtats * b->nbEtats);
   ret->nbEtats = 0;

   creerEtat(ret, numero, a, a->initial[0], b, b->initial[0]);
   interEtatAtome(ret, numero, a, a->initial[0], b, b->initial[0], deja);

   ret->taille = ret->nbEtats;
   ret->etats  = (tTransitions **) xrealloc(ret->etats, ret->nbEtats * sizeof(tTransitions *));

   ret->type = (char *) xrealloc(ret->type, ret->nbEtats * sizeof(char));


   /* On libere deja et numero */

   for (i = 0; i < a->nbEtats; i++) {
     free(deja[i]);
     free(numero[i]);
   }
   free(deja);
   free(numero);

   //   debug("out of interAutAtome\n");

   /*
   sprintf(buf, "interatome%d-out.dot", dbgi);
   autalmot_dump_dot_fname(ret, buf);
   */

   return ret ;
}



/* Precondition : l etat a(i), b(j) a ete cree et a recu un numero.
 */

static void interEtatAtome(tAutAlMot * ret, int ** numero, tAutAlMot * a, int i, tAutAlMot * b, int j, BOOL ** deja) {


  tTransitions * trA, * trB, * trB0;
  BOOL trouve;


  /* Si on a deja traite l'intersection entre l'etat i de l'aut a et l'etat j de l'aut b, on sort */

  if (deja[i][j]) { return; }

  deja[i][j] = TRUE ;

  // debug("intersection %d = (%d %d)...\n", numero[i][j], i, j);

  trB0 = b->etats[j];


  /* cherche la transition par defaut */

  for (trB = b->etats[j]; trB && trB->etiq; trB = trB->suivant);

  tTransitions * parDefautB = trB;


  for (trA = a->etats[i]; trA; trA = trA->suivant) {

    if (trA->etiq == NULL) { die("interEtatAtome: <def> label in autamaton on atoms\n") ; }

    trouve = FALSE ;

    for (trB = trB0; trB; trB = trB->suivant) {

      if (trB->etiq) {

	if (appartientBis(trA->etiq, trB->etiq)) {
	  if (trouve) {
	     Affiche_Symbole(trA->etiq); error(" IN "); Affiche_Symbole(trB->etiq);
	     error("\nnondeterminist automaton\n"); }

	  creerTrans(a, i, b, j, trA->etiq, trA->but, trB->but, ret, numero) ;
	  trouve = TRUE ;
	  interEtatAtome(ret, numero, a, trA->but, b, trB->but, deja) ;

	}
      }
    }

    if (! trouve) { /* on utilise la trans. par defaut ou univ. */

      if (parDefautB == NULL) {

	error("Erreur interne [interEtatAtome] 1\n");
	Affiche_Symbole(trA->etiq);
	error("not found from state %d\n", j);
	exit(1);
      }

      creerTrans(a, i, b, j, trA->etiq, trA->but, parDefautB->but, ret, numero);
      interEtatAtome(ret, numero, a, trA->but, b, parDefautB->but, deja);
    }
  }
}

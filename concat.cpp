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

/* concat.cpp */
/* concatenation ; union  */

#include <stdio.h>
#include <stdlib.h>
#include "general.h"
#include "autalmot.h"
#include "deter.h"
#include "concat.h"
#include "variable.h"
#include "compgr.h"
#include "utils.h"
 
static void renommerButs(tAutAlMot * aut, etat num) ;
static tMultiTrans * versDef(tAutAlMot * a1, etat e1, tAutAlMot * a2,
   etat e2) ;
static void ajout(tAutAlMot * a, etat e, tMultiTrans * t) ;
static void libereMultiTrans(tMultiTrans * t1) ;
//static void copieUchar(unsigned char * nouvTab, unsigned char *tabOrig) ;
//static void affMultiTrans(tMultiTrans * t) ;
static alphabet * soustr(tSymbole * s, tTransitions * t) ;
static alphabet * soustrSymb(alphabet * a, tSymbole * s) ;


/* Les deux automates d entree sont perdus (detruits ou reutilises dans
 * l automate resultat). Pour les utiliser apres l appel de la fonction,
 *	IL FAUT Les avoir COPIEs AVANT L'APPEL.
 * Precondition : aut1 et aut2 sont deterministes et complets.
 * Postconditions : l automate resultat est complet
 * et il a au plus un etat initial.
 */


tAutAlMot * concatenation(tAutAlMot * aut1, tAutAlMot * aut2) { 

  tTransitions ** t ; 
  etat e, i2, oldNbEtats ;
  tMultiTrans * versDef1, * versDef2 ;

  if (! aut1 -> nbEtats) {
    libereAutAlMot(aut2);
    return aut1;
  }

  if (! aut2 -> nbEtats) {
    libereAutAlMot(aut1);
    return aut2;
  }

  i2 = aut2 -> initial[0] ;
  renommerButs(aut2, aut1 -> nbEtats) ;
  oldNbEtats = aut1 -> nbEtats ; 
  aut1->nbEtats += aut2 -> nbEtats ;
  aut1->taille = aut1 -> nbEtats ;
  aut1->etats = (tTransitions * *) realloc(aut1 -> etats, sizeof(tTransitions *) * aut1 -> nbEtats);

  if (! aut1 -> etats)
    erreurMem("concatenation") ;

  if (aut1 -> entrantesEtats)
    printf("Fuite de mémoire.\n") ;

  aut1 -> entrantesEtats = 0 ;

  /*printf(" ajoute les transitions de aut2 dans aut1 \n") ; */

  for (e = 0 ; e < aut2 -> nbEtats ; e ++) 
    aut1 -> etats[e + oldNbEtats] = aut2 -> etats[e] ; 
  for (e = 0 ; e < oldNbEtats ; e ++)
    if (final(aut1, e)) {
      /*printf("Etat terminal %d...\n", e + 1) ;*/
      versDef1 = versDef(aut2, i2, aut1, e) ;
      /* transitions explicites sortant de aut2(i2) mais pas de aut1(e) : */
      /* on fait des transitions explicites vers le defaut de aut1(e) */
      /*affMultiTrans(versDef1) ;*/
      versDef2 = versDef(aut1, e, aut2, i2) ;
      /* transitions explicites sortant de aut1(e) mais pas de aut2(i2) : */
      /* on fait des transitions explicites vers le defaut de aut2(i2) */
      for (t = aut1 -> etats + e ; * t ; t = & (* t) -> suivant) ;
      /*printf(" recopie des transitions de l'etat initial de aut2\n") ;*/
      * t = copie_file_transition(aut2 -> etats[i2]) ;
      /*printf("Copie faite\n") ;*/
      ajout(aut1, e, versDef1) ; /* on les attribue a aut1(e) */
      ajout(aut1, e, versDef2) ; /* on les attribue a aut1(e) */
      /*printf("Ajouts faits\n") ;*/
      /*affMultiTrans(versDef1) ;*/
      libereMultiTrans(versDef1) ;
      libereMultiTrans(versDef2) ;
      /*printf("Liberation OK\n") ;*/ }
/*printf(" initialites et finalites des etats : \n") ; */
 aut1 -> type = (char *) xrealloc(aut1 -> type, sizeof(char) * aut1 -> nbEtats) ; 

for (e = oldNbEtats ; e < aut1 -> nbEtats ; e ++) 
   aut1 -> type[e] = aut2 -> type[e - oldNbEtats] ; 

/* l etat initial du 2eme automate n est plus initial : */ 
 aut1->type[oldNbEtats] &= ~(AUT_INITIAL);

/* les etats finaux de aut1 prennent la finalite de */
/* l etat initial de aut2 : */
 for (e = 0; e < oldNbEtats ; e ++)
   if (final(aut1, e)) {
     if (initial(aut1,e)) {
       aut1->type[e] = aut1->type[oldNbEtats] | AUT_INITIAL;
     } else {
       aut1->type[e] = aut1->type[oldNbEtats];
     }
   }

 /*
   if (aut1 -> sorte[e] % 4 == TERMINAL)
      aut1 -> sorte [e] = aut1 -> sorte[oldNbEtats] ; 
   else if (aut1 -> sorte[e] % 4 == INITIAL_TERMINAL)
      aut1 -> sorte[e] = aut1 -> sorte[oldNbEtats] - 2 ;
 */

free(aut2 -> etats) ; /* Les transitions sortantes sont partagees avec aut1 */
free(aut2 -> type) ;
free(aut2 -> initial) ;
if (aut2 -> entrantesEtats)
   libereEntrantesEtats(aut2) ;
free(aut2) ;
 return aut1 ;
}
 
static void renommerButs(tAutAlMot * aut, etat num) { 
   etat i ; 
   tTransitions * t ;
/*printf("Translation de %d...\n", num) ;*/
for (i = 0 ; i < aut -> nbEtats ; i ++)
   for (t = aut -> etats[i] ; t ; t = t -> suivant) {
      /*printf("%d ", i + num + 1) ;
      Affiche_Symbole(t -> etiq) ;*/
      t -> but += num ;
      /*printf(" %d\n", t -> but + 1) ;*/ }
/*printf("\nTranslation OK\n") ;*/ }

static tMultiTrans * versDef(tAutAlMot * a1, etat e1, tAutAlMot * a2,
   etat e2) {
/* pour chaque transition explicite sortant de a1(e1) mais pas de a2(e2), */
/* on produit une transition explicite vers le defaut de a2(e2) */
   tMultiTrans * mT = NULL, * mTemp ;
   tTransitions * t ;
   etat defaut ;  /* sortie par defaut de e2 dans a2 */
   BOOL trouve = FALSE ;
/*printf("versDef %d a partir de %d\n", e2 + 1, e1 + 1) ;*/
for (t = a2 -> etats[e2] ; t && ! trouve ; t = t -> suivant)
   if (! t -> etiq) {
      defaut = t -> but ;
      trouve = TRUE ; }
if (! trouve) {   /* l etat e2 de a2 n a pas de trans. avec but par defaut */
   /*printf("versDef : pas de defaut\n") ;*/
   return mT ; }
for (t = a1 -> etats[e1] ; t ; t = t -> suivant)
   if (t -> etiq) {   /* nouveau symbole */
      mTemp = (tMultiTrans *) calloc(1, sizeof(tMultiTrans)) ;
	  if (! mTemp)
		 erreurMem("versDef") ;
      mTemp -> suivant = mT ;
      mT = mTemp ;
      mT -> but = defaut ;
      mT -> etiq = soustr(t -> etiq, a2 -> etats[e2]) ; }
/*printf("vers %d a partir de %d : OK\n", defaut + 1, e1 + 1) ;*/
return mT ; }

#if 0
static void affMultiTrans(tMultiTrans * t) {
  alphabet * a ;
  for ( ; t ; t = t -> suivant) {
    printf("vers %d :", t -> but + 1) ;
    for (a = t -> etiq ; a ; a = a -> suiv)
      Affiche_Symbole(a -> etiquette) ;
    printf("\n") ; }
}
#endif


static alphabet * soustr(tSymbole * s, tTransitions * t) {
/* Rend une liste chainee de symboles dont l union disjointe */
/* constitue s sauf les symboles de t */
   alphabet * a = (alphabet *) calloc(1, sizeof(alphabet)) ;
if (! a)
   erreurMem("soustr") ;
a -> etiquette = copieSymbole(s) ;
a -> suiv = NULL ;
for( ; t && a ; t = t -> suivant)
   if (t -> etiq)
	a = soustrSymb(a, t -> etiq) ;
 return a ;
}

static alphabet * soustrSymb(alphabet * a, tSymbole * s) {
/* Rend une liste chainee de symboles obtenue en soustrayant */
/* s de chacun des symboles de a. Detruit a. Ne modifie pas s. */
   alphabet * alph = 0, /* pointe sur le premier symbole du resultat */
      * b = 0,     /* b pointe sur le dernier symbole pret */
      * temp ;
   alphabet * res ;   /* resultat partiel */
   BOOL panique = FALSE ;
   int compte = 0 ;
for (temp = a ; temp ; temp = temp -> suiv) {
   compte ++ ;
   if (4 > temp -> etiquette -> sorteSymbole ||
       temp -> etiquette -> sorteSymbole > 8) {
      fprintf(fErr, "\nInternal error [soustrSymb].\n") ;
      panique = TRUE ; } }
if (panique) {
	fclose(fErr) ;
   printf("soustrSymb ") ;
   Affiche_Symbole(s) ;
   printf("\n") ;
   for (temp = a ; temp ; temp = temp -> suiv) {
      Affiche_Symbole(temp -> etiquette) ;
      printf("\n") ; }
   exit(1) ; }
while (a)
   if (a -> etiquette -> sorteSymbole == ATOME)
      if (s -> sorteSymbole == ATOME)
         if (compSymb(a -> etiquette, s)) { /* 2 atomes distincts */
            if (! alph)
               alph = a ;
            else
               b -> suiv = a ;
            b = a ;
            a = a -> suiv ; }
         else { /* on supprime cette etiquette */
            free(a -> etiquette -> canonique) ;
            free(a -> etiquette) ;
            temp = a -> suiv ;
            free(a) ;
            a = temp ; }
      else if (appartient(a -> etiquette, s)) { /* on supprime l etiquette */
         free(a -> etiquette -> canonique) ;
         free(a -> etiquette) ;
         temp = a -> suiv ;
         free(a) ;
         a = temp ; }
      else { /* 2 etiquettes disjointes */
         if (! alph)
            alph = a ;
         else
            b -> suiv = a ;
         b = a ;
         a = a -> suiv ; }
   else {
      if (4 > a -> etiquette -> sorteSymbole ||
           a -> etiquette -> sorteSymbole > 8) {
         printf("Erreur interne [soustrSymb].\n") ;
         Affiche_Symbole(a -> etiquette) ;
         Affiche_Symbole(s) ;
         printf("...\n") ;
         exit(1) ; }
      res = sauf(a -> etiquette, s) ;
      /*printf("...%d\n", nbSymboles) ;*/
      if (res) {   /* on reutilise res*/
         if (! alph)
               alph = res ;
         else b -> suiv = res ;
         while (res -> suiv)
			 res = res -> suiv ;
         b = res ; }
	   /* on libere a */
      free(a -> etiquette -> canonique) ;
      free(a -> etiquette) ;
      temp = a -> suiv ;
      free(a) ;
      a = temp ; }
if (alph)
   b -> suiv = NULL ;
return alph ; }

static void ajout(tAutAlMot * aut, etat e, tMultiTrans * t) {
/* Ajoute comme sorties de aut(e) les transitions de t */
   alphabet * a ;
   tTransitions * temp ;
for ( ; t ; t = t -> suivant)
   for (a = t -> etiq ; a ; a = a -> suiv) {
      /*printf("%d ", e + 1) ;
      Affiche_Symbole(a -> etiquette) ;
      printf(" %d\n", t -> but + 1) ;*/
      temp = (tTransitions *) calloc(1, sizeof(tTransitions)) ;
      if (! temp)
         erreurMem("ajout") ;
      temp -> etiq = copieSymbole(a -> etiquette) ;
      temp -> but = t -> but ;
      temp -> suivant = aut -> etats[e] ;
      aut -> etats[e] = temp ; } }

static void libereMultiTrans(tMultiTrans * t1) {
   alphabet * a1, * a2 ;
   tMultiTrans * t2 ;
for ( ; t1 ; t1 = t2) {
   /*printf("but %d\n", t1 -> but + 1) ;*/
   for (a1 = t1 -> etiq ; a1 ; a1 = a2) {
      /*Affiche_Symbole(a1 -> etiquette) ;
      printf("\n") ;*/
      if (a1 -> etiquette -> canonique) {
         /*printf("%s...\n", a1 -> etiquette -> canonique) ;*/
         free(a1 -> etiquette -> canonique) ; }
      /*else printf("forme canonique null\n") ;*/
      /*printf("Liberation symbole...\n") ;*/
      free(a1 -> etiquette) ;
      a2 = a1 -> suiv ;
      /*printf("Liberation alphabet...\n") ;*/
      free(a1) ; }
   t2 = t1 -> suivant ;
   /*printf("Liberation tMultiTrans...\n") ;*/
   free(t1) ; } }

tAutAlMot * AjoutBoucle(tAutAlMot * aut1, int pos) {
/* Concatenation avec A* a gauche (pos == etatInitial) */
/* ou a droite (pos == etatsFinaux). */
/* Modifie et renvoie l'automate donne en entree. */
/* Precondition : aut1 est deterministe. */
/* Lorsque pos == etatsFinaux, on espere qu aucune transition ne sort */
/* d un etat final, mais on n a pas de garantie */
   etat i ;
   tTransitions * nouvTr, * s, * t ;
   tSymbole * nouvetiq ;
if (aut1 == NULL)
   printf("Automate nul.\n") ;
if (aut1 -> etats == NULL)
   printf("Automate sans etats [AjoutBoucle %s].\n",
      (pos == etatInitial ? "etatInitial" : "etatsFinaux")) ;
if (aut1 -> nbEtats == 0)
   printf("Nombre d etats nul [AjoutBoucle %s].\n",
      (pos == etatInitial ? "etatInitial" : "etatsFinaux")) ;
if (pos == etatInitial) { 
   nouvTr = (tTransitions *) calloc(1,sizeof(tTransitions)) ;
   if (! nouvTr)
      erreurMem("AjoutBoucle") ;
   nouvTr -> but = aut1 -> initial[0] ;
   nouvetiq = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
   if (! nouvetiq)
      erreurMem("AjoutBoucle") ;
   nouvetiq -> sorteSymbole = UNIVERSEL ;
   nouvetiq -> canonique = (unichar *) calloc(1, sizeof(char)) ;
   if (! nouvetiq -> canonique)
      erreurMem("AjoutBoucle") ;
   nouvetiq -> canonique[0] = 0 ;
   nouvetiq -> flechie[0] = 0 ;
   nouvetiq -> gramm[0] = 0 ;
   nouvTr -> etiq = nouvetiq ;
   nouvTr -> suivant = aut1 -> etats[aut1 -> initial[0]] ;
   aut1 -> etats[aut1 -> initial[0]] = nouvTr ; }
else if (pos == etatsFinaux)
   for (i = 0 ; i < aut1 -> nbEtats ; i ++) {
      if (final(aut1,i)) {  
         for (t = aut1 -> etats[i] ; t ; t = s) {
            s = t -> suivant ;          /* Destruction de la liste des transitions */
            libereEtiq(t) ;		/* sortantes pour chaque etat */
            free(t) ;
            t = 0 ; }
         nouvTr = (tTransitions *) calloc(1, sizeof(tTransitions)) ;
         if (! nouvTr)
            erreurMem("AjoutBoucle") ;
         nouvTr -> but = i ;
         nouvetiq = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
         if (! nouvetiq)
            erreurMem("AjoutBoucle") ;
         nouvetiq -> sorteSymbole = UNIVERSEL ;
         nouvetiq -> canonique = (unichar *) calloc(1, sizeof(char)) ;
         if (! nouvetiq -> canonique)
            erreurMem("AjoutBoucle") ;
         nouvetiq -> canonique[0] = 0 ;
         nouvetiq -> flechie[0] = 0 ;
         nouvetiq -> gramm[0] = 0 ;
         nouvTr -> etiq = nouvetiq ;
         nouvTr -> suivant = 0 ;
         aut1 -> etats[i] = nouvTr ; } }
else printf("Erreur interne [AjoutBoucle].\n") ;
return aut1 ; }

tAutAlMot * unionAut(tAutAlMot * aut1, tAutAlMot * aut2) {
/* Les deux automates donnes en entree sont perdus (detruits ou reutilises */
/* dans l automate resultat). */
/* Preconditions : aut1 et aut2 sont deterministes et complets, */
/* et le tableau entrantesEtats n'est pas rempli. */
/* Postcondition : l automate resultat est complet */
/* et il a au plus un etat initial. */
   etat i, aut1nbEtats, i1, i2 ;
   tTransitions * * t ;
   tMultiTrans * versDef1, * versDef2 ;
if (! aut1 -> nbEtats) {
   libereAutAlMot(aut1) ;
   return aut2 ; }
if (! aut2 -> nbEtats) {
   libereAutAlMot(aut2) ;
   return aut1 ; }
aut1nbEtats = aut1 -> nbEtats ;
aut1 -> nbEtats += (aut2 -> nbEtats + 1) ;
aut1 -> taille = aut1 -> nbEtats ;
renommerButs(aut2, aut1nbEtats + 1) ;
aut1 -> etats = (tTransitions * *)
   realloc(aut1 -> etats, sizeof(tTransitions *) * aut1 -> nbEtats) ;
if (! aut1 -> etats)
   erreurMem("unionAut") ;
for(i = aut1nbEtats + 1 ; i < aut1 -> nbEtats ; i ++)
    aut1 -> etats[i] = aut2 -> etats[i - aut1nbEtats - 1] ;
if (aut1 -> entrantesEtats)
   fprintf(fErr, "\nFuite de mémoire [unionAut].\n") ;
aut1 -> entrantesEtats = 0 ;
/***** le nouvel etat initial *****/
i1 = aut1 -> initial[0] ;
i2 = aut2 -> initial[0] ;
aut1 -> initial[0] = aut1nbEtats ;
/* Transitions sortant du nouvel etat initial : */
versDef1 = versDef(aut2, i2, aut1, i1) ;
/* transitions explicites sortant de aut2(i2) mais pas de aut1(i1) : */
/* on fait des transitions explicites vers le defaut de aut1(i1) */
/*affMultiTrans(versDef1) ;*/
versDef2 = versDef(aut1, i1, aut2, i2) ;
/* transitions explicites sortant de aut1(i1) mais pas de aut2(i2) : */
/* on fait des transitions explicites vers le defaut de aut2(i2) */
/*affMultiTrans(versDef2) ;*/
aut1 -> etats[aut1nbEtats] = copie_file_transition(aut1 -> etats[i1]) ;
for (t = aut1 -> etats + aut1nbEtats ; * t ; t = & (* t) -> suivant) ;
* t = copie_file_transition(aut2 -> etats[i2]) ;
/*printf("Copie faite\n") ;*/
ajout(aut1, aut1nbEtats, versDef1) ;
/*printf("Ajouts faits\n") ;*/
libereMultiTrans(versDef1) ;
/*printf("Liberation OK\n") ;*/
ajout(aut1, aut1nbEtats, versDef2) ;
libereMultiTrans(versDef2) ;
/** remplit le tableau sorte avec les initialites et les terminalites  ***/
aut1 -> type = (char *) xrealloc(aut1 -> type, sizeof(char) * aut1 -> nbEtats) ; 

/*** sorte de l'etat initial de l'automate ***/

 aut1->type[aut1nbEtats] = AUT_INITIAL ;

 if (final(aut1,i1) || final(aut2,i2))
   aut1->type[aut1nbEtats] |= AUT_TERMINAL ;

 for(i = aut1nbEtats + 1 ; i < aut1 -> nbEtats ; i ++)
   aut1->type[i] = aut2->type[i - aut1nbEtats - 1] ;

 /*** change l initialite des etats initiaux des deux automates ***/
 aut1->type[i1] &= ~(AUT_INITIAL);
 aut1->type[aut1nbEtats + 1 + i2] &= ~(AUT_INITIAL);

 free(aut2 -> etats) ; /* Les transitions sortantes sont partagees avec aut1 */
 free(aut2 -> type) ;
 free(aut2 -> initial) ;
 if (aut2 -> entrantesEtats)
   libereEntrantesEtats(aut2) ;
 free(aut2) ;
 return aut1 ;
}



static void _aut_concat(tAutAlMot * res, tAutAlMot * a, etat e, int * corresp) {
  
  debug("_aut_concat(e=%d)\n", e);

  if (a->type[e] & AUT_FINAL) { res->type[corresp[e]] |= AUT_FINAL; }

  for (tTransitions * t = a->etats[e]; t; t = t->suivant) {
    
    if (corresp[t->but] == NEXISTEPAS) {

      debug("_aut_concat: %d NEXISTEPAS (-> %d)\n", t->but, res->nbEtats);

      corresp[t->but] = res->nbEtats++;
      res->type[corresp[t->but]] = 0;
      res->etats[corresp[t->but]] = NULL;
      nouvTrans(res, corresp[e], t->etiq, corresp[t->but]);

      _aut_concat(res, a, t->but, corresp);

    } else { nouvTrans(res, corresp[e], t->etiq, corresp[t->but]); }
  }
}


void autalmot_concat(tAutAlMot * res, tAutAlMot * a) {

  if (a->nbEtatsInitiaux != 1) { die("autalmot_concat: bad automaton (%d initial states)\n", a->nbEtatsInitiaux); }

  autalmot_resize(res, res->nbEtats + a->nbEtats);

  int * corresp = (int *) xmalloc(a->nbEtats * sizeof(int));

  for (int i = 0; i < (int) a->nbEtats; i++) { corresp[i] = NEXISTEPAS; }

  etat oldnb = res->nbEtats;

  for (etat e = 0; e < oldnb; e++) {

    if (res->type[e] & AUT_FINAL) {

      debug("concat: %d is terminal\n", e);

      res->type[e] &= ~(AUT_FINAL);
      corresp[a->initial[0]] = e;

      _aut_concat(res, a, a->initial[0], corresp);
    }
  }

  debug("after concat:\n");
  autalmot_dump_plain(res);
}


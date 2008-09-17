 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/* autalmot.c */
/* Date 	: juin 98 */
/* Auteur(s) 	: MAYER Laurent et al */
/* automates sur un alphabet d etiquettes lexicales */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "autalmot_old.h"
#include "String_hash.h"
#include "Unicode.h"
#include "utils.h"
#include "Error.h"


void chargeSymbole(tSymbole * symb, unichar * lex, char * nomFich) ;
static char * copie_tab_sorte(char *tabOrig,int nbEtats) ;
static int * copie_tab_initial(int *tabOrig, int nbElements) ;
static tTransitions * * copie_tab_etats(tTransitions * *tabOrig,int nbEtats) ;


//int dobreak = 0;

tSymbole * tSymbole_new() {
  tSymbole * res = (tSymbole *) xmalloc(sizeof(tSymbole));
  res->sorteSymbole = UNIVERSEL;
  res->flex = new_Ustring();
  res->canonique = NULL;
  return res;
}

void symbole_delete(tSymbole * symb) {
  if (! symb) { return; }
  free_Ustring(symb->flex);
  free(symb->canonique); 
  free(symb);
}


void symbole_copy(tSymbole * dest, tSymbole * src) {
  dest->sorteSymbole = src->sorteSymbole;
  u_strcpy(dest->flex, src->flex);
  //u_strcpy(dest->flechie, src->flechie);
  dest->canonique = u_strdup(src->canonique);
  u_strcpy(dest->gramm, src->gramm);
}


tSymbole * symbole_dup(tSymbole * src) {

//  debug("symbole_dup: (%d:%S,%S.%S)\n", src->sorteSymbole, src->flex->str, src->canonique, src->gramm);

  if (src == NULL) { return NULL; }

  //tSymbole * dest = (tSymbole *) xmalloc(sizeof(tSymbole));
  tSymbole * dest = tSymbole_new();

  dest->sorteSymbole = src->sorteSymbole;
  //u_strcpy(dest->flechie, src->flechie);
  u_strcpy(dest->flex, src->flex);
  u_strcpy(dest->gramm, src->gramm);
  dest->canonique = u_strdup(src->canonique);

  return dest;
}




void initAlphabet(tAlphMot * alphabet, int nb) {  
  alphabet->symb       = (tSymbole *) xmalloc(nb * sizeof(tSymbole));
  alphabet->size    = nb;
  alphabet->nbSymboles = 0;
}



tAlphMot * alphabet_new(int size) {
  tAlphMot * alphabet  = (tAlphMot *) xmalloc(sizeof(tAlphMot));
  alphabet->symb       = (tSymbole *) xmalloc(size * sizeof(tSymbole));
  alphabet->size    = size;
  alphabet->nbSymboles = 0;
  return alphabet;
}


void free_SymbolAlphabet(tAlphMot * alpha) {
  free(alpha->symb);
  free(alpha);
}

void alphabet_clear(tAlphMot * alpha) {
  alpha->nbSymboles = 0;
}


tSymbole * add_symbol(tAlphMot * alphabet, tSymbole * symb) {

  while (alphabet->nbSymboles >= alphabet->size) {
    alphabet->size = alphabet->size * 2;
    alphabet->symb    = (tSymbole *) xrealloc(alphabet->symb, alphabet->size * sizeof(tSymbole));
  }

  tSymbole * addr = alphabet->symb + alphabet->nbSymboles;
  addr->flex = new_Ustring();
  symbole_copy(addr, symb);

  alphabet->nbSymboles++;

  return addr;
}





void transition_delete(tTransitions * t) {
  symbole_delete(t->etiq);
  free(t);
}



/* Postconditions : la taille est egale au nombre d'etats ;
 * l'unique etat initial est 0, sauf s'il n'y a pas d'etats.
 */

tAutAlMot * initAutAlMot(int nbEtats) {

  tAutAlMot * aut = (tAutAlMot *) xcalloc(1, sizeof(tAutAlMot));

  aut->name           = NULL;
  aut->nbEtats        = nbEtats; 
  aut->taille         = nbEtats; 
  aut->entrantesEtats = NULL;

  if (nbEtats == 0) {

    error("automaton with no states.\n") ;
    aut->nbEtatsInitiaux = 0 ;

  } else {

    aut->etats = (tTransitions **) xcalloc(nbEtats, sizeof(tTransitions *)) ;
    aut->type  = (char *) xcalloc(nbEtats, sizeof(char)); 

    marqueEtatInitial(aut);
  }

  return aut ;
}


/* Marque 0 comme etat initial de l automate.
 */

void marqueEtatInitial(tAutAlMot * aut) {
  aut->nbEtatsInitiaux = 1;
  aut->initial = (int *) xcalloc(1, sizeof(int));
  aut->initial[0] = 0;
  //  aut->type[0] |= AUT_INITIAL; 
}


/* Initialise un automate deja alloue. L'unique etat initial est 0. */

void initAutAlMotAlloue(tAutAlMot * aut, int nbEtats) {

  aut->name           = NULL;
  aut->nbEtats        = nbEtats ; 
  aut->taille         = nbEtats ; 
  aut->entrantesEtats = NULL ;

  if (nbEtats == 0) {
    error("Attention, automate sans etats.\n") ;
    aut->nbEtatsInitiaux = 0 ;
  } else {
    aut->etats = (tTransitions **) xcalloc(nbEtats, sizeof(tTransitions *)) ;
    aut->type  = (char *) xcalloc(nbEtats, sizeof(char)); 
    marqueEtatInitial(aut) ;
  }
}




/* Cree une nouvelle transition de source vers but etiquetee s. 
 * Il n'y a pas de partage de memoire entre s et l'etiquette de la transition.
 */

void nouvTrans(tAutAlMot * a, int source, tSymbole * s, int but) {

  tTransitions * t = (tTransitions *) xmalloc(sizeof(tTransitions)) ;

  //  t->etiq    = s ? copieSymbole(s) : NULL;
  t->etiq = symbole_dup(s);
  t->but     = but;
  t->suivant = a->etats[source];
  a->etats[source] = t;
}




void libereAlphabet(tAlphMotPartage * alphabet) {
  free(alphabet -> symb) ;
  free(alphabet) ;
  alphabet = 0 ;
}




/* Copie un automate Õ l©identique.
 * * entree : pointeur sur structure tAutAlMot
 * * sortie : nouveau pointeur sur structure tAutAlMot
 * Les transitions entrantes ne sont  pas copiees.
 */

tAutAlMot * copieAutomate(tAutAlMot * autOrig) {

  tAutAlMot * nouvAut = (tAutAlMot *) xmalloc(sizeof(tAutAlMot));

  unichar buf[] = { 'b', 'l', 'a', 0 };
  nouvAut->name = u_strdup(buf);
  nouvAut->taille = nouvAut->nbEtats = autOrig->nbEtats; 
  nouvAut->etats  = copie_tab_etats(autOrig->etats, autOrig->nbEtats);
  nouvAut->entrantesEtats = NULL;
  nouvAut->type = copie_tab_sorte(autOrig->type, autOrig->nbEtats); 
  nouvAut->nbEtatsInitiaux = autOrig->nbEtatsInitiaux;
  nouvAut->initial = copie_tab_initial(autOrig->initial, nouvAut->nbEtatsInitiaux);
  return nouvAut;
}



/* entree : tableau de pointeur sur structure tTransition de taille nbEtats
 * sortie : nouveau tableau de pointeur sur structure tTransition
 */ 

static tTransitions ** copie_tab_etats(tTransitions ** tabOrig, int nbEtats) { 

  int i; 
  tTransitions ** tabNouv; 

  tabNouv = (tTransitions **) xcalloc(1, sizeof(tTransitions *) * nbEtats); 

  for(i = 0; i < nbEtats; i++) { tabNouv[i]=copie_file_transition(tabOrig[i]); }

  return tabNouv; 
} 




/*** entree : pointeur sur structure tTransitions 
 *** sortie : une copie
 */ 

tTransitions * copie_file_transition(tTransitions * transOrig) {

   if (transOrig == NULL) { return NULL; }

   tTransitions * NtT = (tTransitions *) xcalloc(1, sizeof(tTransitions));

   /*Affiche_Symbole(transOrig -> etiq) ;
     printf(" %d\n", transOrig -> but + 1) ;*/
   NtT->but  = transOrig->but;
   NtT->etiq = (transOrig->etiq ? copieSymbole(transOrig->etiq) : NULL); 

   if (transOrig->suivant == NULL) 
     NtT->suivant = NULL; 
   else 
     NtT->suivant = copie_file_transition(transOrig->suivant) ; 

   return NtT;
} 



/*** entree : tableau de sorte des etats de taille nbEtats ***/ 
/*** sortie : nouveau tableau de sorte des etats de taille nbEtats   ***/ 

static char * copie_tab_sorte(char *tabOrig,int nbEtats) { 

  int i; 
  char *nouvTab; 

  nouvTab = (char*) xcalloc(1, sizeof(char)*nbEtats);

  for(i=0;i<nbEtats;i++) { 
    nouvTab[i]=tabOrig[i]; 
  } 
  return(nouvTab); 
} 



/*** entree : tableau des  etats initiaux ***/ 
/*** sortie : nouveau tableau des etats initiaux    ***/ 

static int * copie_tab_initial(int *tabOrig, int nbElements) {
  int i ; 
  int * nouvTab ; 

  if (nbElements != 1)
    u_printf("Automate avec %d etats initiaux.\n", nbElements) ;

  nouvTab = (int *) xcalloc(1,  sizeof(int) * nbElements) ; 

  for(i = 0 ; i < nbElements ; i ++) 
    nouvTab[i] = tabOrig[i] ; 

  return nouvTab ;
} 




/* Renvoie 0 si les symboles sont identiques 
 * et un autre entier s'ils sont distincts.
 */

int compSymb(tSymbole * s1, tSymbole * s2) {

  if (s1 == s2) { return 0 ; }

  if (! s1 || ! s2) { return 1 ; }

  if (s1->sorteSymbole == s2->sorteSymbole) {

    switch (s1->sorteSymbole) {

    case ATOME:
    case NEGATIF:
    case INCOMPLET:
      if (! u_strcmp(s1->canonique, s2->canonique) && ! u_strcmp(s1->gramm, s2->gramm)) {
	return 0 ;
      } else { return 1 ; }

    case CODE_POUET:
      return u_strcmp(s1->gramm, s2->gramm);

    case UNIVERSEL:
      return 0;

    default :
      error("Erreur interne [compSymb], code %d\n", s1->sorteSymbole);
    }
  }

  return 1;
}



tSymbole * copieSymbole(tSymbole * Source) {


  if (! Source) { error("Erreur interne [copieSymbole] : symbole nul.\n"); return NULL; }


  //tSymbole * s = (tSymbole *) xcalloc(1, sizeof(tSymbole));
  tSymbole * s = tSymbole_new();

  switch (s->sorteSymbole = Source->sorteSymbole) {

  case ATOME:
  case INDETERMINE:
    u_strcat(s->flex, Source->flex);
    /*
    if (1 + u_strlen(Source->flechie) >= maxMot) { fatal_error("Erreur interne [copieSymbole], f.f. trop longue\n"); }
    u_strcpy(s->flechie, Source->flechie);
    */

  /* et on continue */

  case INCOMPLET:
  case NEGATIF:
    s->canonique = u_strdup(Source->canonique);
    if (1 + u_strlen(Source->gramm) >= maxGramm){ fatal_error("Erreur interne [copieSymbole], c.g. trop long\n"); }
    u_strcpy(s->gramm, Source->gramm) ;
    break;

  case CODE_POUET:
    if (1 + u_strlen(Source->gramm) >= maxGramm) { fatal_error("Erreur interne [copieSymbole], c.g. trop long\n"); }
    u_strcpy(s->gramm, Source->gramm);
    s->canonique    = (unichar *) xmalloc(sizeof(unichar));
    s->canonique[0] = 0;
    //s->flechie[0]   = 0;
    break;


  case UNIVERSEL:
    s->canonique = (unichar *) xmalloc(sizeof(unichar));
    s->canonique[0] = 0;
    //s->flechie[0]   = 0;
    s->gramm[0]     = 0;
    break;

  default :
    fatal_error("Erreur interne [copieSymbole], unknow sorteSymbole: %d.\n", Source->sorteSymbole);
  }

  return s;
}





/* Libere le contenu de l'automate sans liberer l'automate. */
/* Precondition : l automate a au moins un etat. */

void videAutomate(tAutAlMot * Aut) {

  int i ;
  tTransitions * t, * s ;       /* Transition courante et suivante */

  for (i = 0 ; i < Aut -> nbEtats ; i ++) {
    for (t = Aut->etats[i]; t; t = s) { /* Destruction des transitions sortantes pour chaque etat */
      s = t->suivant;
      //libereEtiq(t);
      symbole_delete(t->etiq);
      free(t);
    }
  }

  free(Aut->etats);
  Aut->etats = 0;

  free(Aut->type);
  Aut->type = 0;

  free(Aut->initial);
  Aut->initial = 0;
 
  if (Aut->entrantesEtats) libereEntrantesEtats(Aut);

  Aut->nbEtats = 0;
}



/* PrÝcondition : a -> entrantesEtats != 0. */

void libereEntrantesEtats(tAutAlMot * a) {

  int i ;
  tTransitions * pt, * temp ;

  for (i = 0; i < a->nbEtats; i ++) {
    for (pt = a->entrantesEtats[i] ; pt ; pt = temp) {
      temp = pt->suivant ;
      free(pt) ;
      pt = 0 ;
    }   /* pt -> etiq est partage avec Aut -> etats[i] */
  }

  free(a->entrantesEtats) ;
  a->entrantesEtats = 0 ; /* free() ne le fait pas */
}   



void libereAutAlMot(tAutAlMot * Aut) {

  if (Aut->nbEtats) { videAutomate(Aut); }

  free(Aut->name);
  free(Aut);
}


void list_aut_old_delete(list_aut_old * lst) {

  for (int i = 0; i < lst->nb_aut; ++i) {
    libereAutAlMot(lst->les_aut[i]);
  }
  free(lst->les_aut);
  free(lst);
}


/*** entree : automate deterministe complet  ***/ 
/*** sortie : automate deterministe complet du complementaire    ***/ 
/*** modifie et reutilise l automate d entree          ***/ 

void complementation(tAutAlMot * entAut) {

  int i ; 
  for (i = 0 ; i < entAut -> nbEtats ; i ++)
    if (final(entAut, i)) /* etat terminal */
      rendreNonFinal(entAut, i);
  else
    rendreFinal(entAut, i);
}





void output_fst2_labels(string_hash* hash,FILE* f) {
for (int i=0;i<hash->size;i++) {
   u_fprintf(f,"%%%S\n",hash->value[i]);
}
u_fprintf(f,"f\n");
}



void autalmot_resize(tAutAlMot * a, int size) {

  if (size < (int) a->nbEtats) { fatal_error("autalmot_resize: size(=%d) < nbEtats(=%d)\n", size, a->nbEtats); }

  a->etats = (tTransitions **) xrealloc(a->etats, size * sizeof(tTransitions *));
  a->type  = (char *) xrealloc(a->type, size * sizeof(char));

  a->taille = size;
}



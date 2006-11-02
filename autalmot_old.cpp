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
#include "unicode.h"
#include "utils.h"
#include "Error.h"


void chargeSymbole(tSymbole * symb, unichar * lex, char * nomFich) ;
static char * copie_tab_sorte(char *tabOrig,etat nbEtats) ;
static etat * copie_tab_initial(etat *tabOrig, etat nbElements) ;
static tTransitions * * copie_tab_etats(tTransitions * *tabOrig,etat nbEtats) ;


int dobreak = 0;


void symbole_delete(tSymbole * symb) {
  if (! symb) { return; }
  free(symb->canonique); free(symb);
}


void symbole_copy(tSymbole * dest, tSymbole * src) {
  dest->sorteSymbole = src->sorteSymbole;
  u_strcpy(dest->flechie, src->flechie);
  dest->canonique = u_strdup(src->canonique);
  u_strcpy(dest->gramm, src->gramm);
}


tSymbole * symbole_dup(tSymbole * src) {

  if (src == NULL) { return NULL; }

  tSymbole * dest = (tSymbole *) xmalloc(sizeof(tSymbole));

  dest->sorteSymbole = src->sorteSymbole;
  u_strcpy(dest->flechie, src->flechie);
  u_strcpy(dest->gramm, src->gramm);
  dest->canonique = u_strdup(src->canonique);

  return dest;
}




void initAlphabet(tAlphMot * alphabet, int nb) {  
  alphabet->symb       = (tSymbole *) xmalloc(nb * sizeof(tSymbole));
  alphabet->tabsize    = nb;
  alphabet->nbSymboles = 0;
}



tAlphMot * alphabet_new(int size) {
  tAlphMot * alphabet  = (tAlphMot *) xmalloc(sizeof(tAlphMot));
  alphabet->symb       = (tSymbole *) xmalloc(size * sizeof(tSymbole));
  alphabet->tabsize    = size;
  alphabet->nbSymboles = 0;
  return alphabet;
}


void alphabet_delete(tAlphMot * alpha) {
  free(alpha->symb);
  free(alpha);
}

void alphabet_clear(tAlphMot * alpha) {
  alpha->nbSymboles = 0;
}


tSymbole * alphabet_add(tAlphMot * alphabet, tSymbole * symb) {

  while (alphabet->nbSymboles >= alphabet->tabsize) {
    alphabet->tabsize = alphabet->tabsize * 2;
    alphabet->symb    = (tSymbole *) xrealloc(alphabet->symb, alphabet->tabsize * sizeof(tSymbole));
  }

  tSymbole * addr = alphabet->symb + alphabet->nbSymboles;
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

tAutAlMot * initAutAlMot(etat nbEtats) {

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
  aut->initial = (etat *) xcalloc(1, sizeof(etat));
  aut->initial[0] = 0;
  //  aut->type[0] |= AUT_INITIAL; 
}


/* Initialise un automate deja alloue. L'unique etat initial est 0. */

void initAutAlMotAlloue(tAutAlMot * aut, etat nbEtats) {

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



#if 0

/*
 * Renvoie l'automate lu dans fich qui est deja ouvert en lecture.
 * Les symboles de l'alphabet sont reperes par des numeros et la 
 * correspondance est fournie dans alphabetLu qui est deja alloue. 
 * Attention : libereAutAlMot ne peut donc pas liberer l'automate charge. 
 * Postcondition : la taille est egale au nombre d'etats.
 */


tAutAlMot * chargeAutAlMot(FILE * fich, char * nomFich, tAlphMot * alphabetLu) {

  unichar tampon[tailleTampon], * p;
  int numSymb;
  etat etatSource, etatBut ;      
  tAutAlMot * aut ;

  while(u_fgets(tampon, tailleTampon, fich)) {

    p = tampon;

    if (*p != '#' && *p != '\n') {

      if (! u_is_digit(*p)) {
	die("%s: bad autalmot format\n", nomFich);
      }

      alphabetLu->nbSymboles = u_parse_int(p, &p);

      while (!u_is_digit(*p)) {

	if (*p == 0) { die("%s: bad autalmot format\n", nomFich); }	  
	p++;
      }

      aut = initAutAlMot(u_parse_int(p, NULL)) ;

      if (alphabetLu->nbSymboles == 0) {  /* on passe l'alphabet */

	alphabetLu->symb = 0 ;
	u_fgets(tampon, tailleTampon, fich) ;

      } else { chargeAlphabet(fich, nomFich, alphabetLu) ; }

      /*printf("Alphabet charge OK\n");*/

      /* Lignes decrivant les etats */

      for (etatSource = 0 ; etatSource < aut->nbEtats ; etatSource ++) {

	u_fgets(tampon, tailleTampon, fich);
	p = tampon;

	if (tampon[0] == 'f') {
	  die("too few states in file %s.\n", nomFich) ;
	}

	/* printf("Etat %d\n",etatSource+1);*/

	aut->type[etatSource] = (etatSource == 0) ? AUT_INITIAL : 0;

	if (*p == 't') { aut->type[etatSource] |= AUT_FINAL; }

	/*
	if (*p == 't') {
	  aut->sorte[etatSource] = (etatSource == 0 ? INITIAL_TERMINAL : TERMINAL) ;
	} else {
	  aut->sorte[etatSource] = (etatSource == 0 ? INITIAL : NON_INITIAL_NON_TERMINAL) ;
	}
	*/

	aut->etats[etatSource] = NULL;

	p++;

	while (*p == ' ') { p++; }

	numSymb = u_parse_int(p, &p);

	while (numSymb != -1) {

	  while (*p == ' ') { p++; }
	  etatBut = u_parse_int(p, &p);

	  nouvTransNum(aut, etatSource, numSymb, etatBut) ;

	  while (*p == ' ') { p++; }
	  numSymb = u_parse_int(p, &p);
	}

      }


      u_fgets(tampon, tailleTampon, fich) ;
      if (*tampon != 'f') {
	die("too many states in file %s : %S.\n", nomFich, tampon) ;
      }

      /*printf("aut fini OK\n") ;*/
      return aut ;
    }
  }

  return 0 ;
}





/* Preconditions : le fichier fich est deja ouvert en lecture */
/* et alphabet est deja alloue ; alphabet -> nbSymboles > 0. */
/* Charge les symboles dans l'alphabet. */
/* Les etiquettes particulieres (ponctuations, mot vide, transitions avec */
/* but par defaut) sont traitees avec sorteSymbole = SPECIAL et la */
/* chaine est copiee dans canonique. */

void chargeAlphabet(FILE * fich, char * nomFich, tAlphMot * alphabet) {

   unichar tampon[tailleTampon], * delimGauche, * delimDroit ;
   unsigned int numSymb;

   alphabet->symb = (tSymbole *) xcalloc(alphabet->nbSymboles, sizeof(tSymbole)) ;

   u_fgets(tampon, tailleTampon, fich) ; 

   if (*tampon != '%') {
     die("[chargeAlphabet] in file %s : %S.\n", nomFich, tampon) ;
   }

   delimGauche = delimDroit = tampon;
   numSymb = 0;

   while (numSymb < alphabet->nbSymboles) {

     while (*(++delimDroit) != '%') { /* look for next '%' */

       if (*delimDroit == '\\') { delimDroit++; }

       if (*delimDroit == 0) { /* no '%' found */
	 die("[chargeAlphabet] in file %s : %S.\n", nomFich, delimGauche + 1);
       }
     }


     if (delimDroit == delimGauche + 1) {   /* deux % de suite */
       die("[chargeAlphabet] in file %s : %%%%, %S.\n", nomFich, delimGauche + 1) ;
     }


     *delimDroit = 0 ;

     chargeSymbole(alphabet->symb + numSymb, delimGauche + 1, nomFich) ;
     numSymb++;

     delimGauche = delimDroit;
   }
}


#endif


void chargeSymbole(tSymbole * symb, unichar * lex, char * nomFich) {

  unichar * delim ;
  unsigned int i, j ;


  static unichar specials[] = { ' ', '\'', '©', '.', ',', '-', ':', ';', '#', '!', '=', '"', '?', '(', ')', '/', '%',
				'>', '$', '&', '*', '+', '@', '[', ']', '|', '_', '\\', 0 };

  /* " '©.,-:;#!=\"?()/%>$&*+@[]|_\\" */

  debug("chargeSymbole: lex=%S\n", lex);

  if (u_strchr(specials, *lex) || u_is_digit(*lex)) {

    symb->sorteSymbole = SPECIAL;
    symb->flechie[0]   = 0;
    symb->gramm[0]     = 0;
    symb->canonique    = (unichar *) malloc((u_strlen(lex) + 1) * sizeof(unichar));

    u_strcpy(symb->canonique, lex) ;
    return ;
  }

  i = 0;

  if (*lex == '<') {

    if (! u_strcmp_char(lex, "<E>") || ! u_strcmp_char(lex, "<def>") || (lex[1] == 0)) {

      symb->sorteSymbole = SPECIAL ;
      symb->flechie[0]   = 0 ;
      symb->gramm[0]     = 0 ;
      symb->canonique    = (unichar *) xcalloc(u_strlen(lex) + 1, sizeof(unichar)) ;

      u_strcpy(symb->canonique, lex);
  
      return;

    } else { fatal_error("[chargeSymbole] in file %s : illegal symbol %S\n",  nomFich, lex) ; }
  }


  if (*lex == '{')
    i = 1 ;  /* On ne conserve pas les { } ; on continue ci-dessous */

  symb->sorteSymbole = INDETERMINE;
  delim = u_strchr(lex + i, ',') ;

  if (! delim) {  /* Pas de virgule */

    if (u_strchr(lex + i, '.')) {

      symb->flechie[0] = 0 ;

    } else { /* Ni point, ni virgule */

      if (lex[i] == '\'' || lex[i] == (unsigned char) '©' || lex[i] == '-' || lex[i] == '#' || lex[i] == '"') {


	if (lex[i + 1] && ((lex[i + 1] != '}') || (lex[i + 2] != 0))) {
	  fatal_error("[chargeSymbole] in file %s : illegal symbol %S\n", nomFich, lex) ;
	}

	symb->sorteSymbole = SPECIAL ;
	symb->flechie[0]   = 0 ;
	u_strcpy_char(symb->gramm, "PNC") ;

	symb->canonique = (unichar *) xmalloc(2 * sizeof(unichar)) ;
	symb->canonique[0] = lex[i] ;
	symb->canonique[1] = 0 ;
	return ;

      } else if (i == 1) { /* Code entre accolades, sans point */

	j = 0 ;
	symb->flechie[0] = 0 ;

	while (lex[i] && lex[i] != '}' && j < maxGramm - 1) { symb->gramm[j++] = lex[i++]; }
	symb->gramm[j] = 0 ;

	symb->canonique = (unichar *) xmalloc(sizeof(unichar));
	symb->canonique[0] = 0;
	return;
      }


      /* mot inconnu dans un texte (pas d'accolade, pas de point, pas de virgule) */

      j = 0;

      while (lex[i]) {

	if (j >= maxMot)   { fatal_error("inflected form too long in file %s : %S.\n", nomFich, lex); }
	if (lex[i] == '}') { fatal_error("illegal form in file %s : %S.\n", nomFich, lex); }

	symb->flechie[j++] = lex[i++]; 
      }

      symb->flechie[j] = 0 ;

      symb->canonique = (unichar *) xmalloc(sizeof(unichar)) ;
      symb->canonique[0] = 0;

      symb->gramm[0] = '?';
      symb->gramm[1] = 0;
      return ;
    }

  } else { /* on a une virgule */

    j = 0 ;

    while (lex + i != delim) {

      symb->flechie[j++] = lex[i++];

      if (j >= maxMot) {
	fatal_error("inflected form too long in file %s : %S.\n", nomFich, lex) ;
      }
    }

    symb->flechie[j] = 0 ;
    i ++ ;
  }

  delim = (unichar *) u_strchr(lex + i, '.') ;

  if (! delim) { /* Pas de point apres la virgule eventuelle (pas de forme canonique?) */

    symb->canonique = (unichar *) xmalloc(sizeof(unichar)) ;
    symb->canonique[0] = 0 ;

  } else {

    /* forme canonique */

    for (j = i ; lex + j != delim ; j++);
    symb->canonique = (unichar *) xmalloc((j + 1 - i) * sizeof(unichar)) ;

    j = 0 ;
    while (lex + i != delim)
      symb->canonique[j++] = lex[i++] ;

    symb->canonique[j] = 0 ;
    i ++ ;
  }


  /* traits gramma ... */

  j = 0;
  while (lex[i] && lex[i] != '}' && j < maxGramm - 1) { symb->gramm[j++] = lex[i++]; }

  symb->gramm[j] = 0;
}




/* Cree une nouvelle transition de source vers but etiquetee s. 
 * Il n'y a pas de partage de memoire entre s et l'etiquette de la transition.
 */

void nouvTrans(tAutAlMot * a, etat source, tSymbole * s, etat but) {

  tTransitions * t = (tTransitions *) xmalloc(sizeof(tTransitions)) ;

  //  t->etiq    = s ? copieSymbole(s) : NULL;
  t->etiq = symbole_dup(s);
  t->but     = but;
  t->suivant = a->etats[source];
  a->etats[source] = t;
}



#if 0

/* Ajoute une transition, mais l'etiquette est reperee par un entier. */

static void nouvTransNum(tAutAlMot * a, etat source, int s, etat but) {

  tTransitions * t = (tTransitions *) xcalloc(1, sizeof(tTransitions)) ;

  t->etiq = (tSymbole *) s;
  t->but  = but;
  t->suivant = a->etats[source];
  a->etats[source] = t;
}


/* Verifie la coherence des indications d initialite */

BOOL verifInit(tAutAlMot * a) {

   etat e, i ;
   BOOL trouve, incoherent = FALSE ;
   unsigned int compte = 0 ;

   if (a->nbEtatsInitiaux != 1) {
     printf("[verifInit] %d etats initiaux :\n", a->nbEtatsInitiaux);
     for (i = 0; i < a->nbEtatsInitiaux; i ++)
       printf("%d ", a->initial[i] + 1);
     printf("\n");
   }

   for (i = 0; i < a->nbEtatsInitiaux; i++) {
     if (! initial(a, a->initial[i])) {
       printf("Etat %d non initial, sorte %d, indexe comme initial\n",  a->initial[i] + 1, a->type[a->initial[i]]);
       incoherent = TRUE;
     }
   }

   for (e = 0 ; e < a->nbEtats ; e++) {

     if (initial(a, e)) {

       compte++;
       trouve = FALSE ;

       for (i = 0 ; i < a->nbEtatsInitiaux && ! trouve ; i ++)
         if (a -> initial[i] == e)
	   trouve = TRUE ;

       if (! trouve) {
         printf("Etat %d initial, sorte %d, non indexe comme tel\n", e + 1, a->type[e]) ;
         incoherent = TRUE ;
       }
     }
   }

   if (compte != a -> nbEtatsInitiaux) {
     printf("%d etats initiaux au lieu de %d indexes.\n", compte, a -> nbEtatsInitiaux) ;
     incoherent = TRUE ;
   }

   if (! initial(a, 0))
     printf("Etat 1 non initial, sorte %d\n", a->type[0]) ;

   return incoherent ;
}



/*#define Write(text) printf("\n%s",text); fflush(stdout)*/

static BOOL dansDico(tSymbole * Etiq, tAlphMotPartage * alphabet) {
/* determine si une etiquette est deja dans l'alphabet */
   unsigned int n ;
for (n = 0 ; n < alphabet -> nbSymboles ; n ++)
  if (! compSymb(Etiq, alphabet -> symb[n]))
     return TRUE ;
return FALSE ; }

int numeroDansAlph(tSymbole * Etiq, tAlphMotPartage * alphabet) {
/* Etant donne un symbole, renvoie son numero dans l'alphabet. */
/* Renvoie - 1 si le symbole n'y figure pas. */
   unsigned int n ;
for (n = 0 ; n < alphabet -> nbSymboles ; n ++)
  if (! compSymb(Etiq, alphabet -> symb[n]))
     return n ;
return - 1 ; }



/* Renvoie l'alphabet de l'automate. Chaque symbole
 * apparait une seule fois dans la liste. S'il y a des transitions avec but
 * par defaut, on met le booleen parDefaut a vrai puis on ne tient pas
 * compte des transitions avec but par defaut dans l'alphabet.
 */

tAlphMotPartage * listeSymboles(tAutAlMot * Aut, BOOL * parDefaut) {

   etat Netat ;            /* numero de l'etat */
   unsigned int tailleAllouee = minTailleAllouee ;
   tTransitions * Trs ;
   tAlphMotPartage * alphabet ;

   *parDefaut = FALSE ;

   alphabet = (tAlphMotPartage *) xcalloc(1, sizeof(tAlphMotPartage)) ;

   alphabet->nbSymboles = 0 ;
   alphabet->symb = (tSymbole **) xcalloc(tailleAllouee, sizeof(tSymbole *)) ;


   if (! Aut->etats) { fatal_error("listeSymbole: Pas d'etats\n"); }


   for (Netat = 0; Netat < Aut->nbEtats; Netat++) {

     for (Trs = Aut->etats[Netat]; Trs; Trs = Trs->suivant) {

       if (Trs->etiq) {
         if (! dansDico(Trs->etiq, alphabet)) {

	   if (alphabet->nbSymboles >= tailleAllouee) {
	     tailleAllouee *= 2 ;
	     alphabet->symb = (tSymbole **) xrealloc(alphabet->symb, sizeof(tSymbole *) * tailleAllouee) ;
	   }

	   alphabet->symb[alphabet->nbSymboles++] = Trs->etiq;
	 }
       } else { *parDefaut = TRUE ; }
     }
   }

   return alphabet ;
}

#endif

void libereAlphabet(tAlphMotPartage * alphabet) {
  free(alphabet -> symb) ;
  free(alphabet) ;
  alphabet = 0 ;
}


#if 0

/* Sauvegarde un automate dans f. Preconditions : il y a au moins un etat ; 
 * le premier etat est l'unique etat initial. 
 * S'il y a des transitions par defaut, on compte un symbole de plus 
 * dans l'alphabet. Les symboles sont ecrits entre accolades si chevrons 
 * vaut vrai.
 */

void sauvegAutAlMot(FILE * f, tAutAlMot * Aut, char * titre, int numAut, BOOL chevrons) {

   etat Netat ; unsigned int n ;
   tAlphMotPartage * alphabet ;
   tTransitions * Trs;
   BOOL parDefaut ;

   debug("sauvegAutAlMot [%d]\n", numAut);

   if (! Aut)
     fatal_error("sauvegAutAlMot: aut is void");

   alphabet = listeSymboles(Aut, & parDefaut) ;

   if (! alphabet) { fatal_error("sauvegAutAlMot: no alphabet\n"); }

   debug("symboles: nb=%d\n", alphabet->nbSymboles);
   for (unsigned int i = 0; i < alphabet->nbSymboles; i++) { debug("%d: {%S,%S.%S}\n", i, alphabet->symb[i]->flechie,
								   alphabet->symb[i]->canonique, alphabet->symb[i]->gramm); }

   u_fprintf(f, "# %s #%d", titre, numAut) ;
   u_fprintf(f, "\n%d %d", (parDefaut ? alphabet->nbSymboles + 1 : alphabet->nbSymboles), Aut->nbEtats) ;
   u_fprintf(f,"\n%%") ;

   for (n = 0 ; n < alphabet->nbSymboles ; n ++) {
     debug("n=%d\n", n);
     sauvegSymbole(f, alphabet->symb[n], chevrons);
   }
   if (parDefaut)
     u_fprintf(f, "<def>%%");

   verifInit(Aut);
   if (Aut->nbEtatsInitiaux != 1)
     fprintf(fErr, "\nWarning: %d initial states\n", Aut->nbEtatsInitiaux) ;

   for (Netat = 0 ; Netat < Aut->nbEtats ; Netat++) {

     /* indication des etats terminaux */

     u_fprintf(f,"\n%c", (final(Aut, Netat)) ? 't' : ':') ;

     for (Trs = Aut->etats[Netat] ; Trs ; Trs = Trs->suivant) {

       if (Trs->etiq) {
	 n = numeroDansAlph(Trs->etiq, alphabet) ;
	 if (n < 0)
	   fatal_error("in sauvegAutAlMot: n < 0") ;
	 u_fprintf (f," %d %d", n, Trs->but + 1) ;

       } else { u_fprintf(f," %d %d", alphabet->nbSymboles, Trs->but + 1) ; }
     }

     fprintf(f, " -1") ;             /* Fin de l'etat */
   }

   fprintf(f, "\nf\n") ;               /* Fin de l'automate */
   libereAlphabet(alphabet) ;
   /* printf("sauvegAutAlMot OK\n") ; */
}
#endif



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

static tTransitions ** copie_tab_etats(tTransitions ** tabOrig, etat nbEtats) { 

  etat i; 
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

static char * copie_tab_sorte(char *tabOrig,etat nbEtats) { 

  etat i; 
  char *nouvTab; 

  nouvTab = (char*) xcalloc(1, sizeof(char)*nbEtats);

  for(i=0;i<nbEtats;i++) { 
    nouvTab[i]=tabOrig[i]; 
  } 
  return(nouvTab); 
} 



/*** entree : tableau des  etats initiaux ***/ 
/*** sortie : nouveau tableau des etats initiaux    ***/ 

static etat * copie_tab_initial(etat *tabOrig, etat nbElements) {
  etat i ; 
  etat * nouvTab ; 

  if (nbElements != 1)
    printf("Automate avec %d etats initiaux.\n", nbElements) ;

  nouvTab = (etat *) xcalloc(1,  sizeof(etat) * nbElements) ; 

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

    case CODE:
      return u_strcmp(s1->gramm, s2->gramm);

    case UNIVERSEL:
      return 0;

    default :
      printf("Erreur interne [compSymb], code %d\n", s1->sorteSymbole);
    }
  }

  return 1;
}



tSymbole * copieSymbole(tSymbole * Source) {


  if (! Source) { error("Erreur interne [copieSymbole] : symbole nul.\n"); return NULL; }


  tSymbole * s = (tSymbole *) xcalloc(1, sizeof(tSymbole));


  switch (s->sorteSymbole = Source->sorteSymbole) {

  case ATOME:
  case INDETERMINE:

    if (1 + u_strlen(Source->flechie) >= maxMot) { fatal_error("Erreur interne [copieSymbole], f.f. trop longue\n"); }
    u_strcpy(s->flechie, Source->flechie);

  /* et on continue */

  case INCOMPLET:
  case NEGATIF:
    s->canonique = u_strdup(Source->canonique);
    if (1 + u_strlen(Source->gramm) >= maxGramm){ fatal_error("Erreur interne [copieSymbole], c.g. trop long\n"); }
    u_strcpy(s->gramm, Source->gramm) ;
    break;

  case CODE:
    if (1 + u_strlen(Source->gramm) >= maxGramm) { fatal_error("Erreur interne [copieSymbole], c.g. trop long\n"); }
    u_strcpy(s->gramm, Source->gramm);
    s->canonique    = (unichar *) xmalloc(sizeof(unichar));
    s->canonique[0] = 0;
    s->flechie[0]   = 0;
    break;


  case UNIVERSEL:
    s->canonique = (unichar *) xmalloc(sizeof(unichar));
    s->canonique[0] = 0;
    s->flechie[0]   = 0;
    s->gramm[0]     = 0;
    break;

  default :
    fatal_error("Erreur interne [copieSymbole], unknow sorteSymbole: %d.\n", Source->sorteSymbole);
  }

  return s;
}



/* Renseigne les champs de s qui est deja alloue.
 * On suppose que s -> canonique n'a pas ete alloue et on l'alloue.
 */

void remplitSymbole(tSymbole * s, tSymbole * source) {

  if (! s) {
    fatal_error(27,"\n[remplitSymbole] Pas de place\n") ;
  }

  if (! source)
    printf("Erreur interne [remplitSymbole].\n") ;

  switch (s -> sorteSymbole = source -> sorteSymbole) {

  case ATOME:
  case INDETERMINE: {
    if (1 + u_strlen(source->flechie) >= maxMot) { printf("Erreur interne [remplitSymbole]\n") ; }
    u_strcpy(s->flechie, source->flechie) ;
  }

  /* et on continue */

  case NEGATIF : {
    s -> canonique = (unichar *) xcalloc(1 + u_strlen(source -> canonique), sizeof(unichar)) ;
  
    u_strcpy(s->canonique, source->canonique) ;

    if (1 + u_strlen(source->gramm) >= maxGramm) { printf("Erreur interne [remplitSymbole]\n") ; }
    u_strcpy(s->gramm, source->gramm) ;
  }
  break ;


  case CODE : case INCOMPLET : {

    if (1 + u_strlen(source->gramm) >= maxGramm) printf("Erreur interne [remplitSymbole]\n") ;

    u_strcpy(s->gramm, source->gramm) ;
    s->canonique = (unichar *) xcalloc(1, sizeof(unichar)) ;

    s->canonique[0] = 0 ;
    s->flechie[0]   = 0 ;
  }
  break ;


  case UNIVERSEL : {
    s->canonique = (unichar *) xcalloc(1, sizeof(unichar)) ;

    s->canonique[0] = 0;
    s->flechie[0]   = 0;
    s->gramm[0]     = 0;
  }
  break ;

  default :
    printf("Erreur interne [remplitSymbole], sorte %d.\n", source -> sorteSymbole) ;
  }
}



#if 0
void sauvegSymbole(FILE * f, tSymbole * s, BOOL chevrons) {

  if (s)
    debug("sauvegSymbole: {%S,%S.%S}, sorte=%d\n", s->flechie, s->canonique, s->gramm, s->sorteSymbole);

  if (s) {

    if (chevrons) { u_fprintf(f, "{") ; }

    switch(s->sorteSymbole) {

    case ATOME:
      u_fprintf(f, "%S,%S.%S", s->flechie, s->canonique, s->gramm) ;
      break ;

    case NEGATIF:
      u_fprintf(f, "%S.%S", s->canonique, s->gramm) ;
      break ;

    case CODE:
    case INCOMPLET:
      u_fprintf(f, ".%S", s->gramm) ;
      break ;

    case UNIVERSEL:
      u_fprintf(f, ".") ;
      break ;

    default :
      die("in sauvegSymbole; internal error") ;
    }

    if (chevrons) { u_fprintf(f, "}"); }

    u_fprintf(f, "%%");

  } else { u_fprintf(f, "<def>"); }
}


#endif


void Affiche_Symbole(tSymbole * s, FILE * f) {

  if (! s) { fprintf(f, "<def>"); return; }

  //  fprintf(f, "[%d]", s->sorteSymbole);

  switch (s->sorteSymbole) {

  case UNIVERSEL:
    fprintf(f, "UNIV");
    break;

  case ATOME :
    fprintf(f, "ATOM");
    break;

  case NEGATIF :
    fprintf(f, "NEG");
    break ;

  case CODE:
    fprintf(f, "CODE");
    break;

  case INCOMPLET:
    fprintf(f, "INC");
    break ;

  default :
    fatal_error("Affiche_Symbole: unknown code %d (<%S,%S.%S>)\n", s->sorteSymbole, s->flechie, s->canonique, s->gramm);
  }

  i_fprintf(f, "<%S,%S.%S>", s->flechie, s->canonique, s->gramm);

  fflush(f);
}




/* Libere le contenu de l'automate sans liberer l'automate. */
/* Precondition : l automate a au moins un etat. */

void videAutomate(tAutAlMot * Aut) {

  etat i ;
  tTransitions * t, * s ;       /* Transition courante et suivante */

  for (i = 0 ; i < Aut -> nbEtats ; i ++) {
    for (t = Aut->etats[i]; t; t = s) { /* Destruction des transitions sortantes pour chaque etat */
      s = t->suivant;
      libereEtiq(t);
      free(t);
      t = 0;
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

  etat i ;
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




/*** entree : automate deterministe complet  ***/ 
/*** sortie : automate deterministe complet du complementaire    ***/ 
/*** modifie et reutilise l automate d entree          ***/ 

void complementation(tAutAlMot * entAut) {

  etat i ; 
  for (i = 0 ; i < entAut -> nbEtats ; i ++)
    if (final(entAut, i)) /* etat terminal */
      rendreNonFinal(entAut, i);
  else
    rendreFinal(entAut, i);
}


#if 0
/* affiche le nombre d etats de l automate, et d autres informations
 * si le nombre d etats est inferieur ou egal a maxEtats.
 */

void diagnostic(tAutAlMot * aut, char * nom, etat max) {

  etat e ;
  int i ;
  tTransitions * t ;

  printf("%s : %d etats", nom, aut -> nbEtats) ;
  if (aut -> nbEtats > max) {
    printf(".\n") ;
    return ;
  }
  printf(" :") ;

  for (e = 0 ; e < aut -> nbEtats ; e ++) {
    printf(" %d", e + 1) ;
    if (initial(aut, e))
      printf("i") ;
    if (final(aut, e))
      printf("f") ;
    for (i = 0, t = aut -> etats[e] ; t ; t = t -> suivant) {
      i ++ ;
      if (t -> but >= aut -> nbEtats)
	printf("\nBut extÝrieur (%d vers %d)\n", e + 1, t -> but + 1) ; }
    printf(",%d", i) ;
  }
  printf("\n") ;
  sauvegAutAlMot(stdout, aut, "automate", 0, FALSE) ;
}

#endif




void autalmot_dump_plain(tAutAlMot * A, FILE * f) {

  i_fprintf(f, "[%S] -- nbstates = %d\n", A->name, A->nbEtats);

  for (etat q = 0; q< A->nbEtats; q++) {

    fprintf(f, "%d [%c%c]:, ", q, initial(A, q) ? 'I' : ' ', final(A, q) ? 'F' : ' ');

    for (tTransitions * t = A->etats[q]; t; t = t->suivant) {

      fprintf(f, "("); Affiche_Symbole(t->etiq); fprintf(f, ", %d) ", t->but);
    }

    fprintf(f, "\n");
  }
}



void autalmot_dump_dot(tAutAlMot * A, FILE * f) {

  if (A == NULL) { return; }

  char buf[1024];

  fprintf(f,
	  "# AutalMot output\n\n"
	  "digraph G {\n"
	  "  graph [ center = true, orientation = landscape, rankdir = LR ];\n"
	  "  node  [ shape  = circle ];\n\n");

  for (etat q = 0; q < A->nbEtats; q++) {

    fprintf(f, "\n  %d [ label=\"%d\" ", q, q);
    if (final(A, q)) { fprintf(f, "shape=\"doublecircle\" "); }
    fprintf(f, "];\n");

    for (tTransitions * t = A->etats[q]; t; t = t->suivant) {

      if (t->etiq) {

	switch (t->etiq->sorteSymbole) {

	case ATOME:
	  i_sprintf(buf, "<%S,%S.%S>", t->etiq->flechie, t->etiq->canonique, t->etiq->gramm);
	  break;

	case SPECIAL:
	  i_sprintf(buf, "SPE(%S)", t->etiq->flechie);
	  break;

	case UNIVERSEL:
	  i_sprintf(buf, "UNIV(%S,%S.%S)", t->etiq->flechie, t->etiq->canonique, t->etiq->gramm);
	  break;

	case CODE:
	case NEGATIF:
	case INCOMPLET:
	  if (*t->etiq->canonique) {
	    i_sprintf(buf, "<%S.%S>", t->etiq->canonique, t->etiq->gramm);
	  } else {
	    i_sprintf(buf, "<%S>", t->etiq->gramm);
	  }
	  break;

	case INDETERMINE:
	  error("dot_output: symbol code %d not implemented\n", t->etiq->sorteSymbole);
	  sprintf(buf, "<NOTIMPLEMENTED-%d>", t->etiq->sorteSymbole);
	}
      } else {
	sprintf(buf, "<def>");
      }
      i_fprintf(f, "  %d -> %d [ label=\"%s\" ];\n", q, t->but, buf);
    }
  }
  fprintf(f, "}\n");
}



void autalmot_dump_dot_fname(tAutAlMot * A, char * fname) {

  FILE * f;

  if ((f = fopen(fname, "w")) == NULL) {
    error("unable to open %s\n", fname);
    return;
  }

  autalmot_dump_dot(A, f);

  fclose(f);
}



void output_fst2_labels(string_hash * hash, FILE * f) {
  for (int i = 0; i < hash->N; i++) { u_fprintf(f, "%%%S\n", hash->tab[i]); }
  u_fprints_char("f\n", f);
}



void autalmot_dump_fst2(tAutAlMot * A, FILE * f) {

  unichar buf[128];

  string_hash * hash = new_string_hash();


  u_fprints_char("0000000001\n", f);

  u_fprintf(f, "-1 %s\n", A->name);

  for (etat i = 0; i < A->nbEtats; i++) {
  
    if (A->type[i] & AUT_TERMINAL) {
      u_fputc('t', f); u_fputc(' ', f);
    } else {
      u_fputc(':', f); u_fputc(' ', f);
    }


    for (tTransitions * t = A->etats[i]; t; t = t->suivant) {

      if (t->etiq == NULL) {

	u_strcpy_char(buf, "<def>");

      } else {

	switch (t->etiq->sorteSymbole) {

	case ATOME:

	  // debug("ATOME:"); Affiche_Symbole(t->etiq);

	  if (*t->etiq->flechie) {

	    u_sprintf(buf, "{%S,%S.%S}", t->etiq->flechie, t->etiq->canonique, t->etiq->gramm);

	  } else if (u_strcmp_char(t->etiq->gramm, "PNC") == 0) {

	    u_strcpy(buf, t->etiq->canonique);

	  } else {

	    u_sprintf(buf, "<%S.%S>", t->etiq->canonique, t->etiq->gramm);
	  }
	  break;

	case SPECIAL:
	  u_strcpy(buf, t->etiq->flechie);
	  break;

	case INCOMPLET:
	case CODE:
	case NEGATIF:
	  if (*t->etiq->canonique) {
	    u_sprintf(buf, "<%S.%S>", t->etiq->canonique, t->etiq->gramm);
	  } else {
	    u_sprintf(buf, "<%S>", t->etiq->gramm);
	  }
	  break;

	case UNIVERSEL:
	    u_sprintf(buf, "<.>");
	    break;

	case INDETERMINE:
	  Affiche_Symbole(t->etiq);
	  fatal_error("ouptut_fst: symbol code INDETERMINE\n");
	  u_sprintf(buf, "<INDETERMINE:%S,%S.%S>", t->etiq->flechie, t->etiq->canonique, t->etiq->gramm);
	}
      }
      u_fprintf(f, "%d %d ", get_hash_number(buf, hash), t->but);
    }

    u_fputc('\n', f);
  }

  u_fprints_char("f \n", f);

  output_fst2_labels(hash, f);

  free_string_hash(hash);
}


void autalmot_dump_fst2_fname(tAutAlMot * A, char * fname) {

  FILE * f;

  if ((f = u_fopen(fname, U_WRITE)) == NULL) {
    error("unable to open %s\n", fname);
    return;
  }

  autalmot_dump_fst2(A, f);

  fclose(f);
}


void autalmot_resize(tAutAlMot * a, int size) {

  if (size < (int) a->nbEtats) { fatal_error("autalmot_resize: size(=%d) < nbEtats(=%d)\n", size, a->nbEtats); }

  a->etats = (tTransitions **) xrealloc(a->etats, size * sizeof(tTransitions *));
  a->type  = (char *) xrealloc(a->type, size * sizeof(char));

  a->taille = size;
}



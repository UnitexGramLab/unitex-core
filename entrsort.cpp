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

/* entrsort.cpp */
/* Chargement et sauvegarde des phrases. */
/* Chargement des grammaires, compilees ou non. */
/* Auteurs : RAVET , POINTAUX et al.         */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "general.h"
#include "autalmot.h"
#include "list_aut.h"
#include "entrsort.h"
#include "deter.h"
#include "variable.h"
#include "decompte.h"
#include "emonde.h"
#include "unicode.h"
#include "utils.h"
#include "minim.h"
#include "AutomateFst2.h"
#include "fst2autalmot.h"


//static void analyseEtiqPhrase(tAlphMot * alphabet, char * nomFic) ;
//static int analyseEtiqGramm(tAlphMot * alphabet, char * nomFic) ;
//static void verifMotVide(tAutAlMot * aut, int motVide, char * nomFic) ;
//static void developpEtiq(tAutAlMot * aut, tAlphMot * alphabet, char * nomFich) ;
//static tSymbole * combine(tSymbole * ancien, unichar * debut, unichar * fin, char * nomFich) ;


static void ajouterLimPhrase(tAutAlMot * d);
int classe(tSymbole * s, char * nomFich, unichar * symbole) ;
static BOOL Vcomplet(unichar * gramm);

int compte_flex(unichar * gramm) ;

/*static BOOL estMot(tSymbole *) ;*/





void dup_etiq(tAutAlMot * A, tAlphMot * alphabet) {

  etat q;
  tTransitions * t;

  for (q = 0; q < A->nbEtats; q++) {
    for (t = A->etats[q]; t; t = t->suivant) {
      t->etiq = copieSymbole(t->etiq); //alphabet->symb[num]);
    }
  }
}



tAutAlMot * chargeAutGramm(char * nomFich) {

  debug("chargeAutGramm : %s\n", nomFich);

  tAutAlMot * gram = load_grammar_automaton(nomFich);

  if (! gram) { die("unable to load grammar %s\n", nomFich); }

  debug("end of chargeAutGramm\n");

  return gram;
}




tChargeurPhrases * constrChargeurPhrases(char * nomFich) {

  tChargeurPhrases * ch;

  ch = (tChargeurPhrases *) xmalloc(sizeof(tChargeurPhrases));

  ch->nomFich = nomFich;

  ch->cumul = 0.0 ;
  ch->nbPhrases = ch->nbPhrasesVides = 0 ;

  return ch;
}


void destrChargeurPhrases(tChargeurPhrases * chPhrases) {
  free(chPhrases);
}



/* Charge les phrases. Calcule leur taille logarithmique.
 * Renvoie le nombre de phrases chargées.
 */

list_aut * chargePhrases(tChargeurPhrases * chPhrases) {

  list_aut * automates = load_text_automaton(chPhrases->nomFich);

  for (int i = 0; i < automates->nb_aut; i++) {

    tAutAlMot * aut = automates->les_aut[i];

    // debug("i=%d\n", i);

    if (aut->nbEtats < 2) {

      automates->tailleLog[i] = 0.0;
      chPhrases->nbPhrasesVides++;

    } else {

      aut = deterCompl(aut);
      aut = automates->les_aut[i] = minimise(aut);

      EmondeAutomate(aut, FALSE);

      ajouterLimPhrase(aut);

      /* tri topo */

      autalmot_tri_topologique(aut);

      /* evaluation */

      tEvaluation * evaluation = evaluation_new(aut);

      automates->tailleLog[i]  = eval_phrase(evaluation);

      // fprintf(stderr, "taillelog=%f\n", automates->tailleLog[i]);

      chPhrases->cumul += automates->tailleLog[i];

      // fprintf(stderr, "cumul=%f\n", chPhrases->cumul);

      evaluation_delete(evaluation);

      /*
	evaluation = evaluation_new(aut);
	double log2 = evalPhrase(evaluation);
	debug("eval: %d == %d\n", (int) automates->tailleLog[i], (int) log2);
	evaluation_delete(evaluation);
      */

    }
  }

  (chPhrases->nbPhrases) += (automates->nb_aut);

  //  fprintf(stderr, "cumul=%f\n", chPhrases->cumul);

  return automates;
}




#if 0

/* Renseigne les champs de la structure tSymbole en tenant compte
 * des conventions des fichiers de sortie d'INTEX.
 * Toutes les etiquettes sont classees ATOME.
 * A utiliser avec la version Windows d'INTEX.
 */

static void analyseEtiqPhrase(tAlphMot * alphabet, char * nomFich) {

  unsigned int numSymb ;
  int i, j ;
  unichar * gramm, * delim ; 

  for (numSymb = 0 ; numSymb < alphabet->nbSymboles ; numSymb++) {

    switch (alphabet->symb[numSymb].sorteSymbole) {

    case SPECIAL :

      alphabet->symb[numSymb].sorteSymbole = ATOME;

      if (alphabet->symb[numSymb].canonique[0] == (unichar) '©') {
	alphabet->symb[numSymb].canonique[0] = '\'';
      }

      if (alphabet->symb[numSymb].canonique[0] == '\\') {

	if (alphabet->symb[numSymb].canonique[2]) {
	  die("label with illegal string (%S) in file %s\n", alphabet->symb[numSymb].canonique, nomFich) ;
	}
	u_strcpy(alphabet->symb[numSymb].flechie, alphabet->symb[numSymb].canonique) ;
	u_strcpy_char(alphabet->symb[numSymb].gramm, "PNC") ;

      } else if (strchr("-'\"%.,:;#!=?()/>$&*+@[]|_", alphabet->symb[numSymb].canonique[0])) {

	if (alphabet->symb[numSymb].canonique[1]) {
	  die("label with illegal string (%S) in file %s\n", alphabet->symb[numSymb].canonique, nomFich) ;
	}

	u_strcpy(alphabet->symb[numSymb].flechie, alphabet->symb[numSymb].canonique) ;
	u_strcpy_char(alphabet->symb[numSymb].gramm, "PNC") ;

      } else if (u_is_digit(alphabet->symb[numSymb].canonique[0])) {

	if (alphabet->symb[numSymb].canonique[1]) {
	  die("label with illegal string (%S) in file %s\n", alphabet -> symb[numSymb].canonique, nomFich) ;
	}
	u_strcpy(alphabet->symb[numSymb].flechie, alphabet->symb[numSymb].canonique) ;
	u_strcpy_char(alphabet -> symb[numSymb].gramm, "CHFA") ; /* chiffres arabes */

      } else {
	die("label with illegal string (%S) in file %s\n", alphabet->symb[numSymb].canonique, nomFich) ;
      }
      break ;


    case INDETERMINE : {

      alphabet->symb[numSymb].sorteSymbole = ATOME;

      if (alphabet->symb[numSymb].canonique[0] == 0) {

	if (alphabet->symb[numSymb].flechie[0] == 0) {

	  die("no forms in file %s\n", nomFich) ;

	} else {

	  alphabet->symb[numSymb].canonique = (unichar *) xrealloc(alphabet->symb[numSymb].canonique,
								   1 + u_strlen(alphabet->symb[numSymb].flechie)) ;

	  u_strcpy(alphabet->symb[numSymb].canonique, alphabet->symb[numSymb].flechie) ;
	}
      }

      gramm = alphabet->symb[numSymb].gramm ;

      delim = u_strchr(gramm, '+') ;

      if (delim) {	/* exemples : .ADV+PDETC% .N+NA:ms/-++un% .DET+Dnum:mp:fp%  */

	j = delim - gramm ;    /* delim == gramm + j ; gramm[j] == '+' */
	delim = u_strchr(delim, ':') ;

	if (! delim) {

	  gramm[j] = 0 ;

	} else {        /* exemples : .N+NA:ms/-++un% .DET+Dnum:mp:fp%  */

	  i = delim - gramm ;   /* gramm + i == delim ; gramm[i] == ':' */
	  delim = u_strchr(delim, '/') ;
	  if (! delim) {
	    u_strcpy(gramm + j, gramm + i) ;
	  } else {                       /* exemple : .N+NA:ms/-++un%  */
	    while(gramm + i != delim)
	      gramm[j ++] = gramm[i ++] ;    
	    gramm[j] = '\0' ;
	  }
	}

      } else {
	delim = u_strchr(gramm, ';') ;
	if (delim)     /* exemple : l,cr.DET;CR=50%  */
	  while (delim < gramm + maxGramm)
	    *delim++ = 0 ;
      }
    }
    break;

    default : erreurInt("analyseEtiqPhrase") ;
    }
  }
}





/* Renseigne les champs de la structure tSymbole en tenant compte
 * des conventions d'ecriture des grammaires de levee d'ambiguite.
 * Les etiquettes sont classees ATOME, INDETERMINE  ou SPECIAL.
 */

static int analyseEtiqGramm(tAlphMot * alphabet, char * nomFich) {

  unsigned int numSymb ;
  unichar * gramm, * delim ;
  int motVide = - 1 ;

  for (numSymb = 0 ; numSymb < alphabet -> nbSymboles ; numSymb ++) {

    switch (alphabet->symb[numSymb].sorteSymbole) {

    case SPECIAL:

      switch (alphabet->symb[numSymb].canonique[0]) {

      case 169 :
      case '©' : alphabet->symb[numSymb].canonique[0] = '\'' ;

	/* et on continue ci-dessous */

      case '#' : case '-' : case '\'' : case '"' : {
	alphabet->symb[numSymb].sorteSymbole = ATOME ;
	if (u_strlen(alphabet->symb[numSymb].canonique) != 1) {
	  die("label with illegal string (%S) in file %s\n", alphabet -> symb[numSymb].canonique, nomFich) ;
	}
	u_strcpy_char(alphabet->symb[numSymb].gramm, "PNC") ; 
      }
      break ;

      case '!':
      case '=': {

	alphabet->symb[numSymb].sorteSymbole = ATOME ;

	if (u_strlen(alphabet->symb[numSymb].canonique) != 1) {
	  die("label with illegal string (%S) in file %s\n", alphabet -> symb[numSymb].canonique, nomFich) ;
	}
	u_strcpy(alphabet->symb[numSymb].flechie, alphabet->symb[numSymb].canonique) ;
	u_strcpy(alphabet->symb[numSymb].gramm,   alphabet->symb[numSymb].canonique) ;
      }
      break ;

      case '<':
	if (! u_strcmp_char(alphabet->symb[numSymb].canonique, "<E>")) {
	  motVide = numSymb ;
	} else if (! u_strcmp_char(alphabet -> symb[numSymb].canonique, "<def>")) {
	  /* seuls cas ou on laisse sorteSymbole == SPECIAL */
	  alphabet -> symb[numSymb].canonique[0] = 0 ;
	} else { erreurInt("analyseEtiqGramm") ; }
	break ;

      case '?': /* Represente les mots inconnus */
	alphabet->symb[numSymb].sorteSymbole = ATOME ;
	if (u_strlen(alphabet -> symb[numSymb].canonique) != 1) {
	  die("label with illegal string (%S) in file %s\n", alphabet->symb[numSymb].canonique, nomFich) ;
	}
	alphabet->symb[numSymb].canonique[0] = 0 ;
	u_strcpy_char(alphabet->symb[numSymb].gramm, "?") ;
	break ;

      default:
	die("illegal symbol in file %s : %c (%d), %S\n", nomFich, alphabet->symb[numSymb].canonique[0],
	    alphabet->symb[numSymb].canonique[0], alphabet->symb[numSymb].canonique);
      }
      break ;

    case INDETERMINE: {
      gramm = alphabet->symb[numSymb].gramm;
      delim = u_strchr(gramm, '+');
      if (delim) {
	while (delim < gramm + maxGramm) { *delim++ = 0; }
      }
      if (alphabet->symb[numSymb].canonique[0] == '!')
	ordonne(& alphabet->symb[numSymb]) ;
    }
    break ;

    default:
      erreurInt("analyseEtiqGramm") ;
    }
  }

  return motVide;
}


static void verifMotVide(tAutAlMot * aut, int motVide, char * nomFic) {

  etat source;
  tTransitions * t;

  for (source = 0 ; source < aut->nbEtats ; source ++) {
    for (t = aut->etats[source] ; t ; t = t->suivant) {
      if ((int) t->etiq == motVide) { die("empty word used in file %s.\n", nomFic); }
    }
  }
}





/* Developpe les etiquettes lexicales qui representent plusieurs formes
 * flechies a la fois. En entree, les etiquettes des transitions sont 
 * reperees par les numeros des symboles dans l'alphabet. En sortie, 
 * les etiquettes sont des pointeurs sur tSymbole. L automate d entree
 * est modifie.
 */

static void developpEtiq(tAutAlMot * aut, tAlphMot * alphabet, char * nomFich) {

  etat source ;
  tTransitions * t, * temp, * nouveau ;
  int nb_flex, i, hyp, k, numSymb ;
  unichar partieCommune[maxGramm] ;
  unichar tpg[maxFormesFl][maxGramm] ;

  /* Tableau de parties d etiquettes grammaticales */

  for (source = 0 ; source < aut -> nbEtats ; source ++) {

    for (t = aut->etats[source]; t; t = t->suivant) {

      numSymb = (int) t -> etiq ;

      nb_flex = compte_flex(alphabet->symb[numSymb].gramm) ;

      if (nb_flex >= maxFormesFl) {
	die("too many hypotheses in file %s : %S.\n", nomFich, alphabet -> symb[numSymb].gramm) ;
      }

      if (nb_flex < 2) {

	switch (alphabet -> symb[numSymb].sorteSymbole) {

	case SPECIAL : {
	  if (alphabet->symb[numSymb].flechie[0] || alphabet->symb[numSymb].canonique[0]
	      || alphabet->symb[numSymb].gramm[0]) 
	    erreurInt("developpEtiq") ;
	  t->etiq = NULL ;  /* Trans. avec but par defaut. */
	}
	break ;

	case ATOME :
	  t->etiq = copieSymbole(& alphabet->symb[numSymb]) ; 
	  break ;

	case INDETERMINE : {
	  t->etiq = copieSymbole(& alphabet->symb[numSymb]) ; 
	  t->etiq->sorteSymbole = classe(t->etiq, nomFich, alphabet->symb[numSymb].gramm) ;
	}
	break ;

	default :
	  erreurInt("developpEtiq") ;
	}

      } else {    /* Transition a remplacer par plusieurs transitions */

	for (i = 0 ; alphabet->symb[numSymb].gramm[i] != ':' ; i ++)
	  partieCommune[i] = alphabet->symb[numSymb].gramm[i] ;

	partieCommune[i] = '\0' ;

	for(hyp = 0 ; hyp < nb_flex ; hyp ++) {
	  k = 0 ;
	  i ++ ;   /* On saute ':' */
	  while(alphabet -> symb[numSymb].gramm[i] && alphabet -> symb[numSymb].gramm[i] != ':')
	    tpg[hyp][k ++] = alphabet -> symb[numSymb].gramm[i ++] ;
	  tpg[hyp][k] = '\0' ;
	}

	t -> etiq = combine(& alphabet -> symb[numSymb], partieCommune, tpg[0], nomFich) ;
	temp = t -> suivant ;

	for(hyp = 1 ; hyp < nb_flex ; hyp ++) {
	  nouveau = (tTransitions *) calloc(1, sizeof(tTransitions)) ;
	  if (! nouveau)
	    erreurMem("developpEtiq") ;
	  nouveau -> but = t -> but ;
	  t -> suivant = nouveau ;
	  t = nouveau ;
	  t -> etiq = combine(& alphabet -> symb[numSymb], partieCommune, tpg[hyp], nomFich) ;
	}
	t -> suivant = temp ;
      }
    }
  }
}


static tSymbole * combine(tSymbole * ancien, unichar * debut, unichar * fin, char * nomFich) {

  tSymbole * nouveau = copieSymbole(ancien) ;
  unichar codeSimple[maxGramm] ;

  u_sprintf(codeSimple, "%S:%S", debut, fin) ;

  if (1 + u_strlen(codeSimple) >= maxGramm) {
    printf("Erreur interne [combine].\n") ;
    exit(1) ;
  }

  u_strcpy(nouveau->gramm, codeSimple) ;

  if (nouveau -> sorteSymbole == INDETERMINE)
    nouveau -> sorteSymbole = classe(nouveau, nomFich, ancien -> gramm) ;

  return nouveau ;
}


#endif

int classe(tSymbole * s, char * nomFich, unichar * symbole) {

  if (s->gramm[0]) {

    if (complet(s->gramm)) {

      switch (s->canonique[0]) {

      case 0 :
	return CODE ;

      case '!' :
	return NEGATIF ;

      default :
	return ATOME ;
      }

    } else {

      if (s->canonique[0]) {
	die("[classe] in file %s : canonical form %s with incomplete code %S in %S\n", nomFich, s->canonique, s->gramm, symbole) ;
      }
      return INCOMPLET ;
    }
  } else {
    if (s->canonique[0]) {
      i_fprintf(fErr, "Error: in file %s : canonical form %S without code in %S\n", nomFich, s->canonique, symbole);
    }
    return UNIVERSEL ;
  }
}



/* Determine si un code grammatical non vide est complet ou non. */

BOOL complet(unichar * gramm) {

  switch (gramm[0]) {

  case 'C':
  case 'D':
  case 'E':
  case 'I':
  case 'P':
  case 'X':
  case '?':
    return TRUE ;

  case 'A':
    if (gramm[1] == 'D') { return TRUE; }   /* c'est un adverbe */

    /* sinon c'est un adjectif et on continue */

  case 'N':
    return (u_strlen(gramm) >= 4);

  case 'V':
    return Vcomplet(gramm);

  default:
    die("[complet]: code %S illegal in %S\n", gramm);
    return FALSE ;
  }
}



/* Determine si un code grammatical de verbe est complet ou non. */

static BOOL Vcomplet(unichar * gramm) {

  int i = 0;

  while (gramm[i] !=  ':') {
    //    if (gramm[i] == 0) { die("[VComplet] illegal code \"%S\" (missing ':')\n", gramm); }
    if (gramm[i] == 0) { return FALSE; }
    i++;
  }

  i++;

  switch (gramm[i]) {

  case 'P' :
  case 'K' :
  case 'C' :
  case 'F' :
  case 'I' :
  case 'S' :
  case 'T' :
  case 'Y' :
  case 'J' :
    return (u_strlen(gramm + i) == 3);

  case 'G' : 
  case 'W' :
    return TRUE ;

  case '1' : case '2' : case '3' :case 'm' : case 'f' : case 's' : case 'p' :
    return FALSE ;

  default :
    die("[Vcomplet]: code %S illegal\n", gramm);
    return FALSE ;
  }
}


#if 0
static BOOL Vcomplet_OLD(unichar * gramm, char * nomFich, unichar * symbole) {

  switch (gramm[1]) {

  case 0 :
    return FALSE ;

  case ':' :
    break ; 

  default :
    die("[Vcomplet] in file %s : code %S illegal in %S\n", nomFich, gramm, symbole) ;
    return FALSE ;
  }

  switch (gramm[2]) {

  case 'P' :
  case 'K' :
  case 'C' :
  case 'F' :
  case 'I' :
  case 'S' :
  case 'T' :
  case 'Y' :
  case 'J' :
    return (u_strlen(gramm) == 5) ;

  case 'G' : 
  case 'W' :
    return TRUE ;

  case '1' : case '2' : case '3' :case 'm' : case 'f' : case 's' : case 'p' :
    return FALSE ;

  default :
    die("[Vcomplet] infile %s : code %S illegal in %S\n", nomFich, gramm, symbole) ;
    return FALSE ;
  }
}
#endif



int compte_flex(unichar * gramm) {

  int nb = 0;

  while (*gramm) {
    if (*gramm == ':') { nb++; }
    gramm++;
  }

  return nb;
}


/* Renvoie TRUE si le symbole s est un mot */
/* et FALSE si c'est une ponctuation. */

#define estMot(s) (u_strcmp_char(s -> gramm, "PNC") != 0)



/* Ajoute les limites de phrase.
 * A utiliser avec la version Windows d'INTEX. Preconditions :
 * d a au moins un etat ; la taille est egale au nombre d'etats.
 * Postcondition : la taille est egale au nombre d'etats.
 */

static void ajouterLimPhrase(tAutAlMot * d) {

  static unichar S[] = { '{', 'S', '}', 0 };

  etat ancien, initBis, nouvFinal;

  tSymbole lim;

  lim.sorteSymbole = ATOME;

  lim.flechie[0] = 0;
  lim.canonique  = S;
  u_strcpy_char(lim.gramm, "PNC");


  if (d->taille != d->nbEtats) { erreurInt("ajouterLimPhrase"); }

  d->nbEtats += 2;
  d->etats    = (tTransitions **) xrealloc(d->etats, d->nbEtats * sizeof(tTransitions *));

  d->taille = d->nbEtats ;
  d->type   = (char *) xrealloc(d->type, d->nbEtats * sizeof(char)) ; 

  initBis   = d->nbEtats - 2;
  nouvFinal = d->nbEtats - 1;

  d->etats[initBis] = d->etats[0];
  d->etats[0]       = 0;

  nouvTrans(d, 0, & lim, initBis);

  d->type[initBis] = d->type[0] & ~(AUT_INITIAL);
  d->type[0]       = AUT_INITIAL;

  d->type[nouvFinal]  = AUT_FINAL;
  d->etats[nouvFinal] = 0;

  for (ancien = 1 ; ancien < d->nbEtats - 2 ; ancien++) {
    if (final(d, ancien)) {
      nouvTrans(d, ancien, & lim, nouvFinal);
      d->type[ancien] = 0; //NON_INITIAL_NON_TERMINAL;
    }
  }
}


/* enleve les '#' rajoutes aux limites des phrases lors de leur chargement
 *
 * l'automate doit avoir un unique etat initial a la premiere position
 * et un unique etat final ... a la derniere position. 
 */


int suppress_limphrase(tAutAlMot * A) {


  if (A->initial[0] != 0 || ! final(A, A->nbEtats - 1)) {
    autalmot_tri_topologique(A);
  }

  if (A->nbEtats < 4 || A->nbEtatsInitiaux != 1 || A->initial[0] != 0 || A->etats[0] == NULL || A->etats[0]->suivant != NULL
      || ! final(A, A->nbEtats - 1)) {
    error("suppress_limphrase: bad automaton\n");
    debug("nbetats=%d, nbinitiaux=%d, initial[0]=%d, etats[0]=%d, etats[0]->suivant=%d, type[last]=%d\n",
	  A->nbEtats, A->nbEtatsInitiaux, A->initial[0], A->etats[0], A->etats[0] ? A->etats[0]->suivant : (void *) -1,
	  A->type[A->nbEtats - 1]);
    return -1;
  }

  if (u_strcmp_char(A->etats[0]->etiq->canonique, "{S}") != 0) {
    error("suppress_limphrase: no sentence limit found: ");
    Affiche_Symbole(A->etats[0]->etiq);
    fprintf(stderr, "\n");
    return -1;
  }


  etat newinit  = A->etats[0]->but - 1;

  etat qfinal  = A->nbEtats -1;
  etat nbstate = A->nbEtats - 2;

  tTransitions ** newtab = (tTransitions **) xmalloc(nbstate * sizeof(tTransitions *));
  char * newtype         = (char *) xmalloc(nbstate * sizeof(char));

  for (etat q = 0; q < nbstate; q++) {

    newtab[q]  = NULL;
    newtype[q] = A->type[q + 1];

    tTransitions * tmp;

    for (tTransitions * t = A->etats[q + 1]; t; t = tmp) {

      tmp = t->suivant;

      if (t->but == qfinal) { /* on indique l'etat comme final et on supprime la transition */

	newtype[q] |= AUT_FINAL;

	transition_delete(t);

      } else {                /* on garde la transition */

	t->but = t->but - 1;
	t->suivant = newtab[q];
	newtab[q] = t;

      }
    }
  }

  
  newtype[newinit] |= AUT_INITIAL;
  A->initial[0] = newinit;

  free(A->etats); A->etats = newtab;
  free(A->type);  A->type  = newtype;
  A->nbEtats = A->taille = nbstate;

  return 0;
}



/* Sauvegarde dans un fichier les resultats pour un texte : automates
 * et valeurs numeriques.
 */

void sauvegTexte(FILE * f, list_aut * Laut, int premier) {

   int Naut;   /* Numero de l'automate courant dans la liste */

   /*int n ;*/
   /*tEvaluation * evaluation ;*/

   if (! Laut)
     erreurInt("sauvegTexte") ;

   for (Naut = 0 ; Naut < Laut -> nb_aut ; Naut ++) {
     if (! Laut->les_aut[Naut]->nbEtats) {
       fprintf(f, "# Sentence #%1d\n0 1\n%%\nt -1\nf\n", Naut + premier);
     } else {
       sauvegAutAlMot(f, Laut->les_aut[Naut], "Sentence", Naut + premier, TRUE);
     }
   }

   /*
     evaluation = (tEvaluation *) calloc(1, sizeof(tEvaluation)) ;  
     if (! evaluation)
     erreurMem("sauvegTexte") ;
     evaluation -> aut = initAutAlMot(Laut -> les_aut[Naut] -> nbEtats) ;
     prepareEval(Laut -> les_aut[Naut], evaluation) ;
     visu(f, evaluation) ;
     fprintf(fErr, "Lexical ambiguity before: %.1f, after: %.1f.\n",
     Laut -> tailleLog[Naut], evaluation -> tailleLog) ;
     if (Laut -> tailleLog[Naut] > 0.0 && evaluation -> tailleLog > 0.0)
     fprintf(fErr, "Residual lexical ambiguity: %.1f %%.\n",
     (evaluation -> tailleLog) * 100.0/Laut -> tailleLog[Naut]) ;
     libereAutAlMot(evaluation -> aut) ;
     free(evaluation -> factorisants) ;
     for (n = 0 ; n <= evaluation -> tailleDirect ; n ++)
     free(evaluation -> direct[n]) ;
     free(evaluation -> direct) ;
     free(evaluation -> dicoInverse) ;
     free(evaluation) ;
    */

   /*fprintf(f,"## Fin du fichier\n");*/
}


/* Sauvegarde dans un fichier les resultats pour une seule phrase : */
/* automate et valeurs numeriques. */

void sauvegPhrase(char * nomFichier, tAutAlMot * aut, int Naut, float tailleInit) {

  FILE * f ;
  tEvaluation * evaluation ;
  int n ;

  if (! (f = fopen(nomFichier, "w"))) {
    fprintf(fErr, "Error opening %s for backup.\n", nomFichier) ;
    return ;
  }

  fprintf(f, "## Temporary backup\n") ;

  if (! aut -> nbEtats) {

    fprintf(f, "# Sentence #%1d\n0 1\n%%\nt -1\nf\n", Naut) ;
    fprintf(f, "\n###%f###\n", tailleInit) ;

  } else {

    sauvegAutAlMot(f, aut, "Sentence", Naut, FALSE) ;
    fprintf(f, "\n###%f###\n", tailleInit) ;

    evaluation = (tEvaluation *) xcalloc(1, sizeof(tEvaluation)) ;  

    evaluation -> aut = initAutAlMot(aut -> nbEtats) ;
    prepareEval(aut, evaluation) ;
    visu(f, evaluation) ;
    fprintf(f, "## Lexical ambiguity before: %.1f, after: %.1f.\n", tailleInit, evaluation -> tailleLog) ;

    if (tailleInit > 0.0 && evaluation -> tailleLog > 0.0)
      fprintf(f, "## Residual: %.1f %%.\n", 100.0 * (evaluation -> tailleLog)/tailleInit) ;

    libereAutAlMot(evaluation -> aut) ;
    free(evaluation -> factorisants) ;

    for (n = 0 ; n <= evaluation -> tailleDirect ; n ++)
      free(evaluation -> direct[n]) ;

    free(evaluation->direct);
    free(evaluation->dicoInverse);
    free(evaluation);
  }

  fprintf(f,"## End of file\n");
  fclose(f) ;
}

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

/* autalmot.h	Automates finis sur les mots */

#ifndef __autalmot_h
#define __autalmot_h

#include "String_hash.h"
#include "Unicode.h"
#include "ustring.h"

#define TRUE	1
#define FALSE 	0

#define ATOME        4           /* Symbole de l'alphabet */
#define UNIVERSEL    5           /* Represente tous les symboles */
#define CODE         6           /* Code sans forme canonique */
#define NEGATIF      7           /* Code avec formes canoniques negatives */
#define INCOMPLET    8           /* Etiquette incomplete */
#define INDETERMINE  9
#define SPECIAL     10



/*
#define INITIAL                  0
#define INITIAL_TERMINAL         1
#define NON_INITIAL_NON_TERMINAL 2
#define TERMINAL                 3
*/


#define AUT_INITIAL    1
#define AUT_FINAL      2
#define AUT_TERMINAL   AUT_FINAL
#define AUT_MARK       4


#define initial(aut, e) ((aut)->type[e] & AUT_INITIAL)
#define final(aut, e)   ((aut)->type[e] & AUT_FINAL)

#define rendreInitial(aut, e)    ((aut)->type[e] |=   AUT_INITIAL)
#define rendreNonInitial(aut, e) ((aut)->type[e] &= ~(AUT_INITIAL))
#define rendreFinal(aut, e)      ((aut)->type[e] |=   AUT_FINAL)
#define rendreNonFinal(aut, e)   ((aut)->type[e] &= ~(AUT_FINAL))


/*
#define initial(aut, e) ((aut)->sorte[e] % 4 < 2)
#define final(aut, e)   ((aut)->sorte[e] % 2)
#define rendreInitial(aut, e)    { if (! initial(aut, e)) (aut)->sorte[e] -= 2 ; }
#define rendreNonInitial(aut, e) { if (initial(aut, e))   (aut)->sorte[e] += 2 ; }
#define rendreFinal(aut, e)      { if (! final(aut, e))   (aut)->sorte[e]++ ; }
#define rendreNonFinal(aut, e)   { if (final(aut, e))     (aut)->sorte[e]-- ; }
*/


//#define libereEtiq(t) {
//if ((t)->etiq) { A
//   free((t)->etiq -> canonique) ; (t)->etiq->canonique = 0 ; 
//   free((t)->etiq) ; (t)->etiq = 0 ; } }

/*
void libereEtiq(tTransitions * t) {
if (t -> etiq) {
   free(t -> etiq -> canonique) ;
   free(t -> etiq) ;
   t -> etiq = 0 ; } }
*/

#define maxMot             4096     /* pour anne */
#define maxGramm           256
#define tailleTampon     32768     /* tampon de lecture d'une ligne */ 
#define minTailleAllouee     8


typedef int BOOL ;


/* Symbole de l'alphabet d'etiquettes lexicales */

typedef struct symb {
  int sorteSymbole;
  unichar   flechie[maxMot];   /* Forme flechie du mot */
  Ustring * flex;
  unichar * canonique;         /* Forme canonique du mot */
  unichar   gramm[maxGramm];   /* Code grammatical */
} tSymbole;


tSymbole * tSymbole_new();

/* Liste de transitions */

typedef struct strTransitions {
  int but ;             /* Etat but de la transition (etat source s'il s'agit d'une liste de transitions entrantes) */
  tSymbole * etiq ;      /* Etiquette de la transition */
  struct strTransitions * suivant ;
} tTransitions ;



/* Tableau des etiquettes lexicales */

typedef struct {
  tSymbole * symb ;
  unsigned int nbSymboles ;
  unsigned int size;
} tAlphMot ;



/* Tableau des etiquettes lexicales */

typedef struct {
   unsigned int nbSymboles ;
   tSymbole ** symb ;
} tAlphMotPartage;


typedef struct {

  unichar * name;

  /* Nombre d'etats effectivement crees
   * En memoire, les etats sont numerotes de 0 a nbEtats - 1 ;
   * dans les fichiers de dechargement, de 1 a nbEtats.
   */

  int nbEtats;


  /* Donne pour chaque etat la liste des transitions sortantes */

  tTransitions ** etats;


  /* Donne pour chaque etat la liste des transitions entrantes.
   * A remplir seulement en cas de besoin. Si entrantesEtats est rempli,
   * sa taille est egale a nbEtats ; sinon, entrantesEtats = PN.
   */

  tTransitions ** entrantesEtats ;


  /* Dit si chaque etat est initial et s'il est terminal
   * La valeur de sorte[numeroEtat] % 4 dit si l'etat est initial
   * et s'il est terminal. D'autres informations peuvent etre codees
   * dans sorte[numeroEtat] par l'utilisateur.
   */

  char * type;


  /* Taille du tableau etats et du tableau sorte */

  int taille;


  /* Tableau des etats initiaux. initial[0],
   * initial[1], etc. contiennent les numeros des etats initiaux.
  */

  int * initial;


  /* taille de initial */

  int nbEtatsInitiaux;

} tAutAlMot ;


#if 0
typedef struct {

  int nbEtats;

  tTransitions ** etats;

  tTransitions ** entrantesEtats ;

  char * type;

  int taille;

  int * initial;

  unsigned int nbEtatsInitiaux;

} tAutAlMot ;
#endif


typedef struct list_aut_old {
  int nb_aut;             /* Nombre d'automates */
  tAutAlMot ** les_aut;   /* Tableau des adresses des automates */
} list_aut_old;

void list_aut_old_delete(list_aut_old * list);

tAutAlMot * initAutAlMot(int nbEtats);
void marqueEtatInitial(tAutAlMot * aut);
void initAutAlMotAlloue(tAutAlMot * aut, int nbEtats);
tAutAlMot * chargeAutAlMot(FILE * fich, char * nomFich, tAlphMot * alphabetLu);
void chargeAlphabet(FILE * fich, char * nomFich, tAlphMot * alphabet);
BOOL verifInit(tAutAlMot * a);
void nouvTrans(tAutAlMot * a, int source, tSymbole * s, int but);
tAlphMotPartage * listeSymboles(tAutAlMot * Aut, BOOL * parDefaut);
void libereAlphabet(tAlphMotPartage * alphabet);
int numeroDansAlph(tSymbole * Etiq, tAlphMotPartage * alphabet) ;
tAutAlMot * copieAutomate(tAutAlMot * autOrig);
tTransitions * copie_file_transition(tTransitions * transOrig);
int compSymb(tSymbole * s1, tSymbole * s2) ;
tSymbole * copieSymbole(tSymbole * Source) ;
void remplitSymbole(tSymbole * destination, tSymbole * source) ;
void sauvegSymbole(FILE * f, tSymbole * s, BOOL chevrons) ;
void videAutomate(tAutAlMot * Aut) ;
void libereAutAlMot(tAutAlMot * Aut) ;
void libereEntrantesEtats(tAutAlMot * a) ;
void complementation(tAutAlMot * entAut) ;


void chargeSymbole(tSymbole * symb, unichar * lex, char * nomFich);
void chargeTxtSymbole(tSymbole * symb, unichar * lex, char * nomFich);


void symbole_delete(tSymbole * symb);
void symbole_copy(tSymbole * dest, tSymbole * src);
tSymbole * symbole_dup(tSymbole * src);

void transition_delete(tTransitions * t);

tAlphMot * alphabet_new(int size = 64);
void free_SymbolAlphabet(tAlphMot * alpha);
void alphabet_clear(tAlphMot * alpha);
tSymbole * add_symbol(tAlphMot * alphabet, tSymbole * symb);

void output_fst2_labels(string_hash * hash, FILE * f);

void autalmot_resize(tAutAlMot * a, int size);

#endif

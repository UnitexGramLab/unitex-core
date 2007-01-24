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

#ifndef _AUTALMOT_H_
#define _AUTALMOT_H_

#include "unicode.h"
#include "symbol.h"

//#define TRUE	1
//#define FALSE 	0


#define AUT_INITIAL    1
#define AUT_FINAL      2
#define AUT_TERMINAL   AUT_FINAL


//#define initial(A, q)  (A->states[q].flags & AUT_INITIAL)
//#define final(A, q)  (A->states[q].flags & AUT_FINAL)


typedef int etat;


/* Liste de transitions */

typedef struct transition_t {
  int to;                /* Etat but de la transition (etat source s'il s'agit d'une liste de transitions entrantes) */
  symbol_t * label;      /* Etiquette de la transition */
  struct transition_t * next;
} transition_t;

transition_t * transition_new(int to, symbol_t * label, transition_t * next = NULL);
void transition_delete(transition_t * trans);
void transitions_delete(transition_t * trans);

void transition_dump(transition_t * t, FILE * f = stderr);
void transitions_dump(transition_t * t, FILE * f = stderr);

void transitions_concat(transition_t ** t1, transition_t * t2);

transition_t * transition_dup(const transition_t * trans);
transition_t * transitions_dup(const transition_t * trans);



typedef struct state_t {
  int flags;               /* final, initial */
  transition_t * trans;    /* transitions en partance de cet état */
  int defto;               /* etat destination de la transition par défaut depuis cet état, -1 si il n'y en a pas */
} state_t;



typedef struct autalmot_t {

  unichar * name;

  int nbstates;

  state_t * states;   /* tableau des etats */
  int size;           /* taille du tableau */


  /* Tableau des etats initiaux. initial[0],
   * initial[1], etc. contiennent les numeros des etats initiaux.
   */

  int * initials;
  int   nbinitials;


  /* Donne pour chaque etat la liste des transitions entrantes.
   * A remplir seulement en cas de besoin. Si entrantesEtats est rempli,
   * sa taille est egale a nbEtats ; sinon, entrantesEtats = PN.
   */

  //  transition_t ** trans_in;

} autalmot_t;



autalmot_t * autalmot_new(unichar * name = NULL, int size = 8);
void autalmot_delete(autalmot_t * A);

void autalmot_empty(autalmot_t * A);
autalmot_t * autalmot_dup(const autalmot_t * A);

void autalmot_resize(autalmot_t * A, int size);
static inline void autalmot_resize(autalmot_t * A) { autalmot_resize(A, A->nbstates); }


inline void autalmot_set_name(autalmot_t * A, unichar * name) { free(A->name); A->name = u_strdup(name); }
inline void autalmot_set_name(autalmot_t * A, char * name) { free(A->name); A->name = u_strdup_char(name); }

void autalmot_set_initial(autalmot_t * A, int state);
static inline void autalmot_set_final(autalmot_t * A, int state);

void autalmot_unset_initial(autalmot_t * A, int q);


int autalmot_add_state(autalmot_t * A, int flags = 0);
void autalmot_add_trans(autalmot_t * A, int from, symbol_t * label, int to);


autalmot_t * load_grammar_automaton(char * name, language_t * lang = LANG);



void autalmot_dump(const autalmot_t * A, FILE * f = stderr);

void autalmot_dump_dot(const autalmot_t * A, FILE * f = stderr);
void autalmot_dump_dot_fname(const autalmot_t * A, char * fname);

// type == TEXT | GRAM | LOCATE

void autalmot_output_fst2(const autalmot_t * A, char * name, int type);




void autalmot_determinize(autalmot_t * A);

autalmot_t * autalmot_intersection(const autalmot_t * A, const autalmot_t * B);

autalmot_t * interAutAtome(const autalmot_t * A, const autalmot_t * B);

void autalmot_tri_topo(autalmot_t * A);

/* TODO */

autalmot_t * autalmot_union(autalmot_t * A, autalmot_t * B);
void autalmot_concat(autalmot_t * A, autalmot_t * B);
void autalmot_minimize(autalmot_t * A, int level = 0);
void autalmot_complementation(autalmot_t * A);
void autalmot_emonde(autalmot_t * A);


// inline implementations

static inline void autalmot_set_final(autalmot_t * A, int q)    { A->states[q].flags |= AUT_FINAL; }
static inline void autalmot_set_terminal(autalmot_t * A, int q) { A->states[q].flags |= AUT_FINAL; }

static inline void autalmot_unset_final(autalmot_t * A, int q)    { A->states[q].flags &= ~(AUT_FINAL); }
static inline void autalmot_unset_terminal(autalmot_t * A, int q) { A->states[q].flags &= ~(AUT_FINAL); }

inline int autalmot_is_final(autalmot_t * A, int q)    { return A->states[q].flags & AUT_TERMINAL; }
inline int autalmot_is_terminal(autalmot_t * A, int q) { return A->states[q].flags & AUT_TERMINAL; }
inline int autalmot_is_initial(autalmot_t * A, int q)  { return A->states[q].flags & AUT_INITIAL;  }

#endif

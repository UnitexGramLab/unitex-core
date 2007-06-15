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

#include "Unicode.h"
#include "symbol.h"
#include "SingleGraph.h"
#include "String_hash.h"


//#define TRUE	1
//#define FALSE 	0


#define AUT_INITIAL    1
#define AUT_FINAL      2
#define AUT_TERMINAL   AUT_FINAL


//#define initial(A, q)  (A->states[q].flags & AUT_INITIAL)
//#define final(A, q)  (A->states[q].flags & AUT_FINAL)



/* Liste de transitions */

typedef struct transition_t {
  int to;                /* Etat but de la transition (etat source s'il s'agit d'une liste de transitions entrantes) */
  symbol_t * label;      /* Etiquette de la transition */
  struct transition_t * next;
} transition_t;

transition_t * transition_new(int to, symbol_t * label, transition_t * next = NULL);
void transition_delete(transition_t * trans);
void transitions_delete(transition_t * trans);

void transitions_concat(transition_t ** t1, transition_t * t2);

transition_t * transition_dup(const transition_t * trans);
transition_t * transitions_dup(const transition_t * trans);



typedef struct state_t {
  int flags;               /* final, initial */
  transition_t * trans;    /* transitions en partance de cet état */
  int defto;               /* etat destination de la transition par défaut depuis cet état, -1 si il n'y en a pas */
} state_t;


/**
 * This structure defines an automaton of .fst2, ready to be used
 * for by ELAG programs.
 */
typedef struct {
   /* The name of the automaton. For a text .fst2, it will
    * be the original sentence. For a grammar, it will be the
    * name of the subgraph. */
   unichar* name;

   /* The automaton itself */
   SingleGraph automaton;
   
   /* The symbols that tag the transitions of the automaton */
   struct string_hash_ptr* symbols;



  int nbstates;
  state_t * states;   /* tableau des etats */
  int size;           /* taille du tableau */
  /* Tableau des etats initiaux. initial[0],
   * initial[1], etc. contiennent les numeros des etats initiaux.
   */
  int * initials;
  int   nbinitials;
} Fst2Automaton;



Fst2Automaton* new_Fst2Automaton(unichar* name=NULL,int size=8);
void free_Fst2Automaton(Fst2Automaton*);

void autalmot_empty(Fst2Automaton * A);
Fst2Automaton * autalmot_dup(const Fst2Automaton * A);

void autalmot_resize(Fst2Automaton * A, int size);
static inline void autalmot_resize(Fst2Automaton * A) { autalmot_resize(A, A->nbstates); }


inline void autalmot_set_name(Fst2Automaton * A, unichar * name) { free(A->name); A->name = u_strdup(name); }
inline void autalmot_set_name(Fst2Automaton * A, char * name) { free(A->name); A->name = u_strdup(name); }

void autalmot_set_initial(Fst2Automaton * A, int state);
static inline void autalmot_set_final(Fst2Automaton * A, int state);

void autalmot_unset_initial(Fst2Automaton * A, int q);


int autalmot_add_state(Fst2Automaton * A, int flags = 0);
void add_transition(Fst2Automaton * A, int from, symbol_t * label, int to);
void add_transition(SingleGraph,struct string_hash_ptr*,int,symbol_t*,int);

Fst2Automaton * load_elag_grammar_automaton(char * name, language_t * lang = LANGUAGE);

// type == TEXT | GRAM | LOCATE

void save_automaton(const Fst2Automaton * A, char * name, int type);


Fst2Automaton * autalmot_intersection(const Fst2Automaton * A, const Fst2Automaton * B);

Fst2Automaton * interAutAtome(const Fst2Automaton * A, const Fst2Automaton * B);

void autalmot_tri_topo(Fst2Automaton * A);

/* TODO */

Fst2Automaton * autalmot_union(Fst2Automaton * A, Fst2Automaton * B);
void elag_minimize(Fst2Automaton * A, int level = 0);
void autalmot_complementation(Fst2Automaton * A);
void elag_trim(Fst2Automaton * A);


// inline implementations

static inline void autalmot_set_final(Fst2Automaton * A, int q)    { A->states[q].flags |= AUT_FINAL; }
static inline void autalmot_set_terminal(Fst2Automaton * A, int q) { A->states[q].flags |= AUT_FINAL; }

static inline void autalmot_unset_final(Fst2Automaton * A, int q)    { A->states[q].flags &= ~(AUT_FINAL); }
static inline void autalmot_unset_terminal(Fst2Automaton * A, int q) { A->states[q].flags &= ~(AUT_FINAL); }

inline int autalmot_is_final(Fst2Automaton * A, int q)    { return A->states[q].flags & AUT_TERMINAL; }
inline int autalmot_is_terminal(Fst2Automaton * A, int q) { return A->states[q].flags & AUT_TERMINAL; }
inline int autalmot_is_initial(Fst2Automaton * A, int q)  { return A->states[q].flags & AUT_INITIAL;  }

#endif

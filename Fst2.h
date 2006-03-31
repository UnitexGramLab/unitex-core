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

//---------------------------------------------------------------------------
#ifndef Fst2H
#define Fst2H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unicode.h"


/* Maximum number of tags in a .fst2 */
#define MAX_FST2_TAGS 100000

/* Maximum number of states in a .fst2 */
#define MAX_FST2_STATES 500000

#define FST2_FINAL_STATE_BIT_MASK 1
#define FST2_INITIAL_STATE_BIT_MASK 2



extern int etiquette_courante;

//----------TYPES------------------------------------------
struct fst2Tag {
  int numero;
  unsigned char controle;
  unichar *contenu;
  unichar *transduction;
  unichar *flechi;
  unichar *canonique;
  unichar *infos_gramm;
  struct liste_nombres* numeros;
  int nombre_mots;
  int pattern_compose;

  /* $CD$ begin */
  unichar* contentGF;
  int entryMasterGF;
  /* $CD$ end   */

};

typedef struct fst2Tag* Fst2Tag;


struct etat_fst {
  unsigned char controle;        // etat final ou pas
  // 1: est terminal
  // 2: est initial
  // 4: bit de marquage
  struct transition_fst *trans;     // transitions partant de cet etat
};

typedef struct etat_fst* Etat_fst;


struct transition_fst {
  int etiquette;                // etiquette de la transition : un entier
  int arr;                      // etat d'arrivee de la transition
  struct transition_fst *suivant;   // transition suivante
};

typedef struct transition_fst *liste_transition;


struct variable_list {
  unichar* name;
  int start;
  int end;
  struct variable_list* suivant;
};


struct automate_fst2 {
    Etat_fst* etat;
    Fst2Tag* etiquette;
    int nombre_graphes;
    int nombre_etats;
    int nombre_etiquettes;
    int* debut_graphe_fst2;
    unichar** nom_graphe;
    int* nombre_etats_par_grf;
    struct variable_list* variables;
};

typedef struct automate_fst2 Automate_fst2;


//----------PROTOTYPES-------------------------------------------
void charger_graphe_fst2(FILE*,Etat_fst[],Fst2Tag[],int*,int*,int*,int**,
                         unichar***,int,int**);

liste_transition nouvelle_transition_mat();
Automate_fst2* load_fst2(char*,int);
void free_fst2(Automate_fst2*);
struct variable_list* get_variable(unichar*,struct variable_list*);
Automate_fst2* load_one_sentence_of_fst2(char*,int,FILE*);
int is_final_state(Etat_fst);
void unprotect_characters_in_fst2_tags(Automate_fst2*);
void free_transition(struct transition_fst*);
#endif

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
#ifndef Optimized_fst2H
#define Optimized_fst2H
//---------------------------------------------------------------------------

#include "unicode.h"
#include "Fst2.h"
#include "TransductionVariables.h"


struct liste_arrivees {
  int arr;
  int etiquette_origine;
  struct liste_arrivees* suivant;
};

struct appel_a_sous_graphe {
  int numero_de_graphe;
  struct liste_arrivees* liste_arr;
  struct appel_a_sous_graphe* suivant;
};

struct appel_a_meta {
  int numero_de_meta;
  int negation;
  int numero_de_variable;
  struct liste_arrivees* liste_arr;
  struct appel_a_meta* suivant;
};

struct appel_a_pattern {
  int numero_de_pattern;
  int negation;
  struct liste_arrivees* liste_arr;
  struct appel_a_pattern* suivant;
};

struct token {
  int numero_de_token;
  struct liste_arrivees* liste_arr;
};

struct liste_de_tokens {
  struct token* tok;
  struct liste_de_tokens* suivant;
};

struct etat_opt {
  unsigned char controle;
  struct appel_a_sous_graphe* liste_sous_graphes;
  //  struct appel_a_sous_graphe* liste_inter_graphes; // act. unused
  struct appel_a_meta* liste_metas;
  struct appel_a_pattern* liste_patterns;
  struct appel_a_pattern* liste_patterns_composes;
  struct liste_de_tokens* liste_tokens;
  int* tableau_de_tokens;
  struct liste_arrivees** tableau_liste_arr;
  int nombre_de_tokens;
};

typedef struct etat_opt* Etat_opt;



extern struct etat_opt* graphe_opt[500000];



void optimize_fst2(Fst2*);

#endif

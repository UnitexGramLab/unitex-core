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
#ifndef Sentence_to_grfH
#define Sentence_to_grfH
//---------------------------------------------------------------------------

#include "unicode.h"
#include "Fst2.h"
#include "Liste_nombres.h"

#define WIDTH_OF_A_CHAR 10
#define NBRE_BITS_DE_DECALAGE 19

struct grf_state {
   unichar* content;
   int pos_X;
   int rang;
   struct liste_nombres* l;
};


struct grf_state* new_grf_state(char*,int,int);
struct grf_state* new_grf_state(unichar*,int,int);
void free_grf_state(struct grf_state*);
void add_transition_to_grf_state(struct grf_state*,int);
void remove_duplicates_grf_states(struct grf_state**,int*);
int are_equivalent_grf_states(struct grf_state*,struct grf_state*);
void save_grf_states(FILE*,struct grf_state**,int,int,char* font);

void sentence_to_grf(Automate_fst2*,int,char*,FILE*);
void write_grf_header(int,int,int,char*,FILE*);
int calculer_rang(Automate_fst2*,int,int*,int);
void explorer_rang_etat(int,int,Automate_fst2*,int*,char*,int*);
int numeroter_etiquettes_sur_octets_forts(Automate_fst2*,int,int);
int get_etiquette_reelle(int);
int get_numero_de_la_transition(int);
void convertir_transitions_en_etats(Automate_fst2*,int,int,int*,FILE*,int,int,int,int*,char*);
int width_of_tag(Etiquette);
int calculer_largeur_max_pour_chaque_rang(Automate_fst2*,int,int*,int,int*);

#endif

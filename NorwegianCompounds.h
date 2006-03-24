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
#ifndef NorwegianCompoundsH
#define NorwegianCompoundsH
//---------------------------------------------------------------------------
#include "unicode.h"
#include "Alphabet.h"
#include "DELA.h"
#include "String_hash.h"
#include "Compounds.h"


#define N_SIA 0
#define N_SIE 1
#define N_SIG 2
#define A_SIO 3
#define A_SIE 4
#define V_W 5
#define ADV 6
#define INVALID_LEFT_COMPONENT 7



extern Alphabet* norwegian_alphabet;

struct word_decomposition {
   int n_parts;
   unichar decomposition[2000];
   unichar dela_line[2000];
   int is_a_valid_right_N;
   int is_a_valid_right_A;
};

struct word_decomposition_list {
   struct word_decomposition* element;
   struct word_decomposition_list* suivant;
};


void analyse_norwegian_compounds(Alphabet*,unsigned char*,struct INF_codes*,FILE*,FILE*,FILE*,FILE*,struct string_hash*);
void check_valid_right_component(char*,struct INF_codes*);
void check_valid_left_component(char*,struct INF_codes*);
char check_valid_left_component_for_an_INF_line(struct token_list*);
char check_valid_left_component_for_one_INF_code(unichar*);
char check_valid_right_component_for_an_INF_line(struct token_list*);
char check_valid_right_component_for_one_INF_code(unichar*);
void analyse_norwegian_word_list(unsigned char*,struct INF_codes*,FILE*,FILE*,FILE*,FILE*,struct string_hash*);
int analyse_norwegian_word(unichar*,struct string_hash*);
void explorer_etat_decomposition(int,unichar*,int,int,unichar*,unichar*,unichar*,int,int*);
void explorer_etat(int,unichar*,int,int,unichar*,unichar*,unichar*,int,int*,int);
void get_first_sia_code(int,unichar*);
void ecrire_ligne_dico_sortie(unichar*,int);
char check_Nsia(unichar*);
char check_Asio(unichar*);
char check_VW(unichar*);
char check_ADV(unichar*);
int get_valid_left_component_type_for_one_INF_code(unichar*);

struct word_decomposition* new_word_decomposition();
void free_word_decomposition(struct word_decomposition*);
struct word_decomposition_list* new_word_decomposition_list();
void free_word_decomposition_list(struct word_decomposition_list*);
void explore_state(int,unichar*,int,unichar*,int,unichar*,unichar*,
                   struct word_decomposition_list**,int,struct string_hash*);

#endif

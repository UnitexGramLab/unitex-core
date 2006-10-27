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
#ifndef Normalization_transducerH
#define Normalization_transducerH
//---------------------------------------------------------------------------

#include "unicode.h"
#include "Alphabet.h"
#include "Fst2.h"
#include "Text_tokens.h"
#include "String_hash.h"
#include "List_int.h"
#include "String_list.h"

#define EMPTY_TOKEN -4

struct noeud_arbre_normalization {
   struct string_list* liste_arrivee;
   struct trans_arbre_normalization* trans;
};

struct trans_arbre_normalization {
   int token;
   unichar* s;
   struct noeud_arbre_normalization* arr;
   struct trans_arbre_normalization* suivant;
};


struct temp_list {
   unichar* output;
   struct noeud_arbre_normalization* arr;
   struct temp_list* suivant;
};




struct noeud_arbre_normalization* load_normalization_transducer(char*,Alphabet*,struct text_tokens*);
struct noeud_arbre_normalization* new_noeud_arbre_normalization();
struct trans_arbre_normalization* new_trans_arbre_normalization(int);
void free_noeud_arbre_normalization(struct noeud_arbre_normalization*);
void free_trans_arbre_normalization(struct trans_arbre_normalization*);
struct string_list* tokenize_normalization_output(unichar*,Alphabet*);

struct trans_arbre_normalization* new_trans_arbre_normalization_string(unichar*);
struct noeud_arbre_normalization* load_normalization_transducer_string(char*);


#endif

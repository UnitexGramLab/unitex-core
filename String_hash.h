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
#ifndef String_hashH
#define String_hashH
//---------------------------------------------------------------------------
#include "unicode.h"
#include "Alphabet.h"


struct arbre_hash_trans {
  unichar c;
  struct arbre_hash* arr;
  struct arbre_hash_trans* suivant;
};


struct arbre_hash {
  int final;
  struct arbre_hash_trans* trans;
};


struct string_hash {
  int N;
  struct arbre_hash* racine;
  unichar** tab;
};


struct string_hash* new_string_hash();
struct string_hash* new_string_hash_N(int);
int get_hash_number(unichar*,struct string_hash*);
int get_hash_number_without_insert(unichar*,struct string_hash*);
void free_string_hash(struct string_hash*);
void free_string_hash_without_insert(struct string_hash*);
void sauver_lignes_hash(FILE*,struct string_hash*);
int get_token_number(unichar*,struct string_hash*);
struct string_hash* load_word_list(char*);

int is_in_string_hash_modulo_case(unichar*,struct string_hash*,Alphabet*);

#endif

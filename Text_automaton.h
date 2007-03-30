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

#ifndef Text_automatonH
#define Text_automatonH


#include "Unicode.h"
#include "Text_tokens.h"
#include "Alphabet.h"
#include "DELA_tree.h"
#include "String_hash.h"
#include "NormalizationFst2.h"

#define MAX_TOKENS_IN_SENTENCE 4 //2000


struct trans_text_automaton {
  unichar* chaine;
  int indice_noeud_arrivee;
  struct trans_text_automaton* suivant;
};


struct noeud_text_automaton {
  int numero;
  char final;
  char controle;
  struct trans_text_automaton* trans;
};


struct info {
  struct text_tokens* tok;
  int* buffer;
  Alphabet* alph;
  int SPACE;
  int length_max;
};


int count_non_space_tokens(int*,int,int);
void build_sentence_automaton(int*,int,struct text_tokens*,
                               struct DELA_tree*,struct string_hash*,
                               Alphabet*,FILE*,int,int,
                               struct normalization_tree*);


#endif

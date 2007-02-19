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

//---------------------------------------------------------------------------
#ifndef PortugueseNormalizationH
#define PortugueseNormalizationH
//---------------------------------------------------------------------------

#include "unicode.h"
#include "Alphabet.h"
#include "Matches.h"
#include "List_ustring.h"
#include "Sentence_to_grf.h"
#include "Normalization_transducer.h"
#include "DELA.h"

void build_portuguese_normalization_grammar(Alphabet*,struct match_list*,unsigned char*,
                                            struct INF_codes*,unsigned char*,struct INF_codes*,
                                            char*,struct noeud_arbre_normalization*, struct noeud_arbre_normalization* nasal_norm_tree);
int replace_match_output_by_normalization_line(struct match_list*,Alphabet*,unsigned char*,
                                                struct INF_codes*,unsigned char*,struct INF_codes*,
                                                struct noeud_arbre_normalization*);
int tokenize_portuguese_match(unichar*,unichar*,unichar*,unichar*,unichar*);
int get_radical_lemma(unichar*,struct list_ustring**,Alphabet*,unsigned char*,struct INF_codes*);
int get_inf_number_for_token(int,unichar*,int,unichar*,Alphabet*,unsigned char*);
int compatible_portuguese_inflectional_codes(struct dela_entry*,int,unichar**);
void save_portuguese_normalization_grammar(int,struct match_list*,char*);
int explore_portuguese_normalization_tree(unichar*,unichar*,struct list_ustring*,struct noeud_arbre_normalization*,
                                          Alphabet*);
struct list_ustring* tokenize_portuguese_pronoun(unichar*);

#endif

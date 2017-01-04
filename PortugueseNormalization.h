/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef PortugueseNormalizationH
#define PortugueseNormalizationH


#include "Unicode.h"
#include "Alphabet.h"
#include "LocateMatches.h"
#include "List_ustring.h"
#include "Sentence2Grf.h"
#include "NormalizationFst2.h"
#include "DELA.h"
#include "CompressedDic.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

void build_portuguese_normalization_grammar(const Alphabet*,struct match_list*,
                                            Dictionary*,Dictionary*,
                                            const char*, const VersatileEncodingConfig*,
                                            struct normalization_tree*, struct normalization_tree* nasal_norm_tree);
int replace_match_output_by_normalization_line(struct match_list*,const Alphabet*,
                                                Dictionary*,Dictionary*,
                                                struct normalization_tree*);
int tokenize_portuguese_match(const unichar*,unichar*,unichar*,unichar*,unichar*);
int get_radical_lemma(unichar*,struct list_ustring**,const Alphabet*,Dictionary*);
int get_inf_number_for_token(int,const unichar*,int,unichar*,const Alphabet*,Dictionary*,Ustring*);
int compatible_portuguese_inflectional_codes(struct dela_entry*,int,unichar**);
void save_portuguese_normalization_grammar(int,struct match_list*,const char*, const VersatileEncodingConfig*);
int explore_portuguese_normalization_tree(unichar*,const unichar*,struct list_ustring*,struct normalization_tree*,
                                          const Alphabet*);
struct list_ustring* tokenize_portuguese_pronoun(const unichar*);

} // namespace unitex

#endif

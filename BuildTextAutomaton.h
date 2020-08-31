/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef TextAutomatonH
#define TextAutomatonH


#include "Unicode.h"
#include "Text_tokens.h"
#include "Alphabet.h"
#include "DELA_tree.h"
#include "String_hash.h"
#include "NormalizationFst2.h"
#include "LocateMatches.h"
#include "LanguageDefinition.h"
#include "Korean.h"
#include "HashTable.h"
#include "SingleGraph.h"
#include "Vector.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure is used to compute the list of tags to be inserted in the text automaton,
 * on the base of information taken from either the normalization fst2 or the tags.ind file.
 */
struct output_info {
    /* The output to a appear in the .tfst */
    unichar* output;
    /* The content of the tag. If the tag is of the form {xx,yyy.zzz},
     * it means xx; otherwise it is the same than 'output'. */
    unichar* content;
    /* Bounds of the sequence in the sentence, given in the form (X,Y) (W,Z) where
     * X is the start position in tokens, Y is the position in char in this token,
     * W is the end position in tokens, Z is the position in char in this token. */
    int start_pos;
    int end_pos;
    int start_pos_char;
    int end_pos_char;
    int start_pos_letter;
    int end_pos_letter;
};

#define MAX_TOKENS_IN_SENTENCE 2000

/**
 * This is an internal structure only used to give a set of parameters to some functions.
 */
struct info {
    const struct text_tokens* tok;
    const int* buffer;
    const Alphabet* alph;
    int SPACE;
    int TOKEN;
    int length_max;
};

void build_sentence_automaton(const int*,int,const struct text_tokens*,
                              const struct DELA_tree*,
                              const Alphabet*,U_FILE*,U_FILE*,int,int,
                              struct normalization_tree*,
                              struct match_list**,int,int,
                              language_t*,Korean* korean,
                              struct hash_table* form_frequencies);
void keep_best_paths(SingleGraph graph,struct string_hash* tmp_tags) ;
int count_non_space_tokens(const int* buffer,int length,int SPACE);
vector_ptr* tokenize_normalization_output(unichar* s, const Alphabet* alph);
void free_output_info(struct output_info* x);
void explore_dictionary_tree(int pos ,const unichar* token, unichar* inflected,
                            int ,const struct string_hash_tree_node* ,
                            const struct DELA_tree* ,struct info* ,
                            SingleGraphState ,
                            int, int, int*, int, int, struct string_hash* ,
                            Ustring* ,language_t* ,unichar* tb);

void add_path_to_sentence_automaton(int start_pos, int end_pos,
                            int start_state_index, const Alphabet* alph,
                            SingleGraph graph,struct string_hash* tmp_tags,
                            unichar* s, int destination_state_index,
                            Ustring* foo, struct info* INFO, Korean* korean);
void explore_normalization_tree(int first_pos_in_buffer,
                            int current_pos_in_buffer, int token,
                            struct info* INFO,SingleGraph graph,
                            struct string_hash* tmp_tags,
                            struct normalization_tree* norm_tree_node,
                            int first_state_index,int shift, Ustring* foo,
                            int increment, language_t* language,
                            Korean* korean);
} // namespace unitex

#endif

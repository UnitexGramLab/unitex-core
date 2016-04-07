/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/*
 * author : Anthony Sigogne
 */

#ifndef TaggingProcessH
#define TaggingProcessH

#include <stdio.h>
#include "Copyright.h"
#include "UnitexGetOpt.h"
#include "Tagger.h"
#include "TaggingProcess.h"
#include "Error.h"
#include "Tfst.h"
#include "File.h"
#include "DELA.h"
#include "Transitions.h"
#include "Unicode.h"
#include "ElagFunctions.h"
#include "Match.h"
#include "HashTable.h"
#include "LoadInf.h"
#include "CompressedDic.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure corresponds to a entry in the viterbi
 * matrix (and a transition tag in the automata).
 * It contains a dela_entry (informations about
 * the tag of the transition automata), a tag_code (i.e. tag
 * associated to the entry, for example ".N+Propre:mp"),
 * a predecessor (the best predecessor transition in the automata
 * that maximizes partial probability), tag_number ( the number of the
 * transition in the automata), state_number (the number of the state
 * where the transition gets away) and a partial_prob (partial probability
 * of the best predecessor).
 */
struct matrix_entry {
	struct dela_entry* tag;
	unichar* tag_code;
	int predecessor;
	int tag_number;
	int state_number;
	float partial_prob;
};

void compute_tag_code(struct dela_entry*,unichar*,int);
int create_matrix_entry(const unichar*,struct matrix_entry**,int,int,int);
struct matrix_entry** allocate_matrix(int);
struct matrix_entry** initialize_viterbi_matrix(SingleGraph,int);
void free_matrix_entry(struct matrix_entry*);
void free_viterbi_matrix(struct matrix_entry**,int);

unichar* get_pos_unknown(const unichar*);
int search_matrix_predecessor(struct matrix_entry**,unichar*,int,int,int);
void get_INF_code(unsigned char*,const unichar*,int,int,int,const Alphabet*,int*);
long int get_inf_value(const struct INF_codes*,int);
long int get_sequence_integer(const unichar*,Dictionary*,const Alphabet*);

unichar* create_bigram_sequence(const unichar*,const unichar*,int);
unichar* create_bigram_sequence(const char*,const unichar*,int);
unichar* create_trigram_sequence(const unichar*,const unichar*,const unichar*);
unichar* u_strnsuffix(const unichar*,int);

double compute_emit_probability(Dictionary*,const Alphabet*,const unichar*,const unichar*);
double compute_transition_probability(Dictionary*,const Alphabet*,const unichar*,const unichar*,const unichar*);
double compute_partial_probability(Dictionary*,const Alphabet*,struct matrix_entry*,struct matrix_entry*,struct matrix_entry*);
int* get_state_sequence(struct matrix_entry**,int);
int is_compound_word(const unichar*);
unichar* compound_to_simple(const unichar*);
vector_ptr* do_backtracking(struct matrix_entry**,int,SingleGraph,vector_ptr*,int);
void compute_best_probability(Dictionary*,const Alphabet*,struct matrix_entry**,int,int,int);

vector_ptr* do_viterbi(Dictionary*,const Alphabet*,Tfst*,int);
int get_form_type(Dictionary*,const Alphabet*);
void do_tagging(Tfst*,Tfst*,Dictionary*,const Alphabet*,int,struct hash_table*);

} // namespace unitex

#endif


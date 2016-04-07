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

#ifndef TrainingProcessH
#define TrainingProcessH

#include <stdio.h>
#include "Copyright.h"
#include "UnitexGetOpt.h"
#include "TaggingProcess.h"
#include "Error.h"
#include "File.h"
#include "DELA.h"
#include "Unicode.h"
#include "String_hash.h"
#include "SortTxt.h"
#include "Compress.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define MAX_CONTEXT 3
#define RAW_FORMS 0
#define INFLECTED_FORMS 1
#define MAX_TAGGED_CORPUS_LINE 20000

struct corpus_entry{
	unichar* word;
	unichar* pos_code;
	unichar* overall_codes;
};

void create_disclaimer(const VersatileEncodingConfig* vec,const char* file);
void free_corpus_entry(corpus_entry*);
void push_corpus_entry(corpus_entry*,corpus_entry**);
struct corpus_entry* new_corpus_entry(const unichar*);
int check_corpus_entry(const unichar*);
struct corpus_entry* create_corpus_entry(const unichar*);
void free_context_matrix(struct corpus_entry**);
void initialize_context_matrix(struct corpus_entry**);
struct corpus_entry** new_context_matrix();
void add_key_table(const unichar*,struct string_hash_ptr*);
void add_key_table(const char*,struct string_hash_ptr*);
unichar* compute_contextual_entries(struct corpus_entry**,int,int);
void add_statistics(struct corpus_entry**,struct string_hash_ptr*,struct string_hash_ptr*);
void write_keys_values(struct string_hash_ptr*,struct string_hash_tree_node*,const unichar*,U_FILE*);
void do_training(U_FILE*,U_FILE*,U_FILE*);

} // namespace unitex

#endif


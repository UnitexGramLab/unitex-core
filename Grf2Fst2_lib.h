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

#ifndef Grf2Fst2_libH
#define Grf2Fst2_libH


#include "Unicode.h"
#include "Alphabet.h"
#include "Fst2.h"
#include "LocateConstants.h"
#include "String_hash.h"
#include "SingleGraph.h"
#include "Vector.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure defines all the information that will be useful for the
 * graph compilation process.
 */
struct compilation_info {
   unichar main_graph_path[FILENAME_MAX];
   char repository[FILENAME_MAX];
   struct string_hash_ptr* named_repositories;
   struct string_hash* tags;
   struct string_hash* graph_names;
   int nombre_graphes_comp;
   int nombre_etiquettes_comp;
   TokenizationPolicy tokenization_policy;
   Alphabet* alphabet;
   U_FILE* fst2;
   char no_empty_graph_warning;

   /**
    * This counter is used to identify each context start mark $[ or $![ with
    * a unique number in order to make them different. This is used to
    * avoid merging distinct context start marks during the determinization.
    * For instance, let us consider the two following paths from the initial state:
    *
    * ------> $![ ---> <N:s> ---> $] ---> <N:p>
    *    |--> $![ ---> <N:p> ---> $] ---> <N:s>
    *
    * The first path recoqnizes a noun at plural that cannot be at singular, and
    * the second recoqnizes a noun at singular that cannot be at plural. However,
    * if the determinization merges the two $![ marks, we will have:
    *
    * ------> $![ ---> <N:s> ---> $] ---> <N:p>
    *              |-> <N:p> ---> $] ---> <N:s>
    *
    * With such a negative context, we will fail in both singular and plural
    * cases, and the behavior of the grammar will not be the expected one.
    */
   int CONTEXT_COUNTER;

   VersatileEncodingConfig vec;
   int verbose_name_grf;
   int debug;

   /* This array indicates the renumbering to apply to each graph call. If
    * no .fst2 was loaded, we should always have t[i]==i, but it may be different
    * if a precompiled .fst2 contains more than 1 graph
    */
   vector_int* renumber;
   /* This array indicates whether a graph was a real .grf or a part of a
    * precompiled .fst2, because in this last case, graph calls shouldn't be
    * renumbered */
   vector_int* part_of_precompiled_fst2;
   int current_saved_graph;
   char check_outputs;
   char strict_tokenization;
   /* output associated with subgraphs*/
   struct string_hash* graph_output;
};



struct compilation_info* new_compilation_info();
void free_compilation_info(struct compilation_info*);

int compile_grf(char*,struct compilation_info*,int);
void write_tags(U_FILE*,struct string_hash*);
void write_number_of_graphs(const VersatileEncodingConfig*,const char*,int,int);
void write_graph(U_FILE*,SingleGraph,int,unichar*);
void renumber_graph_calls(Fst2*,vector_int*,vector_int*);

} // namespace unitex

#endif


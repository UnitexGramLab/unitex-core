 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


/**
 * This structure defines all the information that will be useful for the
 * graph compilation process.
 */
struct compilation_info {
   unichar main_graph_path[FILENAME_MAX];
   char repository[FILENAME_MAX];
   struct string_hash* tags;
   struct string_hash* graph_names;
   int nombre_graphes_comp;
   int nombre_etiquettes_comp;
   TokenizationPolicy tokenization_policy;
   Alphabet* alphabet;
   FILE* fst2;
   char no_empty_graph_warning;
};



struct compilation_info* new_compilation_info();
void free_compilation_info(struct compilation_info*);

int compile_grf(char*,struct compilation_info*);
void write_tags(FILE*,struct string_hash*);
void write_number_of_graphs(char*,int);
void write_graph(FILE*,SingleGraph,int,unichar*);



#endif


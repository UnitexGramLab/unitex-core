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
#ifndef FlattenFst2H
#define FlattenFst2H
//---------------------------------------------------------------------------

#include "unicode.h"
#include "Fst2.h"
#include "Liste_nombres.h"
#include "Grf2Fst2_lib.h"

#define FLATTEN_ERROR 0
#define EQUIVALENT_FST 1
#define APPROXIMATIVE_FST 2
#define EQUIVALENT_RTN 3



struct flattened_main_graph_info {
   Etat_comp* states;
   int size;
   int current_pos;
};


int flatten_fst2(Fst2*,int,char*,int);
void compute_dependences(Fst2*);
void compute_dependences_for_subgraph(Fst2*,int,struct liste_nombres**);
void print_dependences(Fst2*);
void check_for_graphs_to_keep(Fst2*,int);
int renumber_graphs_to_keep(Fst2*);
struct flattened_main_graph_info* new_flattened_main_graph_info();
void free_flattened_main_graph_info(struct flattened_main_graph_info*);
int flatten_graph(Fst2*,int,int,int,
                  struct flattened_main_graph_info*,
                  int,int,int*,int*);
void remove_epsilon_transitions_in_flattened_graph(struct flattened_main_graph_info*);
void save_graphs_to_keep(Fst2*,FILE*);
void save_graph_to_be_kept(int,Fst2*,FILE*);
void copy_tags_into_file(Fst2*,FILE*);

#endif

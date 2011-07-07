/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/* Created by Agata Savary (agata.savary@univ-tours.fr)
 */


/********************************************************************************/
//// Inflection graphs for multi-word units //////////
/********************************************************************************/

#ifndef MU_graphH
#define MU_graphH

#include "Unicode.h"
#include "MF_LangMorpho.h"
#include "MF_MU_morpho.h"
#include "MF_Global.h"

#define MAX_GRAPH_NODE 2000  //Maximum length of the contents of a graph node

/////////////////////////////////////////////////
// Possible types for a value description
// e.g. Gen=fem is a constant description, 
//      Gen=$g1 is a description containing a unification variable
//      Gen==$g1 is a description containing an inheritance variable
typedef enum {cnst, unif_var, inherit_var} MU_graph_value_T;

/////////////////////////////////////////////////
//Structure for a single category-value pair in a graph's node
typedef struct {
  l_category_T* cat;       //e.g. Gen
  MU_graph_value_T type;   //type of the description: constant, unification variable, or inheritance variable
  union {
    int value;             //category's value (e.g. fem): index of val in the domain of 'cat'
    unichar* unif_var;     //e.g. g1
    unichar* inherit_var;  //e.g. g1
  } val;  
} MU_graph_category_T;

/////////////////////////////////////////////////
// Set of category-value pairs in a node, e.g. <Gen==$g1, Nb=$n1, Case=gen>
typedef struct {
  int no_cats;                         //number of category-value couples
  MU_graph_category_T cats[MAX_CATS];  //collection of category-value couples
} MU_graph_morpho_T;

/////////////////////////////////////////////////
// Possible types for a graphical unit reference in a node
// e.g. "of" is a reference to a constant unit, 
//      $2 is a reference to a variable (here representing the 2nd constituent of the MWU's lemma)
typedef enum {cst,var} MU_graph_u_T;

/////////////////////////////////////////////////
// Reference to a graphical unit of a MWU. It may be either a constant (e.g. "of")
// or a variable (e.g. $1) referring to a unit in the lemma of the MWU.
typedef struct {
  MU_graph_u_T type; //cst or var
  union {
    unichar* seq;   //e.g. "of"
    int num;        //number of the constituent referred to, e.g. 1 if $1
  } u;
} MU_graph_unit_T;

/////////////////////////////////////////////////
// Input of a node, e.g. $1<Gen==$g1, Nb=$n1, Case=gen>
typedef struct {
  MU_graph_unit_T unit;       //e.g. $1
  MU_graph_morpho_T* morpho;   //e.g. <Gen==$g1, Nb=$n1, Case=gen>
} MU_graph_in_T;

/////////////////////////////////////////////////
// Output of a node, e.g. <Gen=$g1, Nb=$n1, Case=gen>
typedef MU_graph_morpho_T MU_graph_out_T;

/////////////////////////////////////////////////
// Label of a node, e.g. $1<Gen==$g1, Nb=$n1, Case=gen>/<Gen=$g1, Nb=sing, Case=$c2>
typedef struct {
  MU_graph_in_T* in;     //e.g. $1<Gen==$g1, Nb=$n1, Case=gen>; equal to NULL in case of "<E>"
  MU_graph_out_T* out;   //e.g. <Gen=$g1, Nb=sing, Case=$c2>; equal to NULL in case of "<E>"
} MU_graph_label_T;

/////////////////////////////////////////////////

/////////////////////////////////////////////////
// Initializes the structure for inflection graphs
// Returns 1 on success, 1 otherwise.
int MU_graph_init_graphs(MultiFlex_ctx* p_multiFlex_ctx);

/////////////////////////////////////////////////
// Liberates the memory allocated for the structure for inflection graphs
void MU_graph_free_graphs(MultiFlex_ctx* p_multiFlex_ctx);

/////////////////////////////////////////////////
// Frees the memory allocated for all inflection tranducers.
void MU_graph_free_all_graphs(MultiFlex_ctx* p_multiFlex_ctx);

/////////////////////////////////////////////////
// Explores the inflection transducer of the MU-lemma 'MU_lemma' 
// in order to generate all its inflected forms. The generated forms are put to 'forms'
// (initially, 'forms' does not have its space allocated).
//Returns 0 on success, 1 otherwise. 
int MU_graph_explore_graph(MultiFlex_ctx* p_multiFlex_ctx,struct l_morpho_t* pL_MORPHO,
		const VersatileEncodingConfig* vec,MU_lemma_T* MU_lemma, MU_forms_T* forms,const char* pkgdir);

/////////////////////////////////////////////////
// Creates a MU_graph label from two strings.    
// We suppose that MU_label already has its memory allocated.
// Returns 0 on success, 1 otherwise.
int MU_graph_scan_label(MultiFlex_ctx* p_multiFlex_ctx,struct l_morpho_t* pL_MORPHO,unichar* label_in, unichar* label_out, MU_graph_label_T* MU_label);

/////////////////////////////////////////////////
// Prints a MU_graph label.    
void MU_graph_print_label(MU_graph_label_T* MU_label);

/////////////////////////////////////////////////
// Liberates the memory allocated for a MU_graph label.    
void MU_graph_free_label(MU_graph_label_T* MU_label);


#endif

/*
  * Unitex 
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  *
  */

/* Created by Agata Savary (savary@univ-mlv.fr)
 * Last modification on June 23 2005
 */
//---------------------------------------------------------------------------

////////////////////////////////////////////////////////////
// Implementation of unification variables

#ifndef UnifH
#define UnifH

#include <stdio.h>
#include "MF_LangMorpho.h"

///// CONSTANTS
//Maximum number of unification variables
#define MAX_UNIF_VARS 50


/////////////////////////////////////////////
// Representation of a unification variable
// e.g. Gen = $g1
typedef struct {
  l_category_T* cat;   //e.g. Gen
  unichar* id;         //variable's id, e.g. g1
  int val;             //variable's instantiation (e.g. fem): index of val in the domain of cat
} unif_v_T;

////////////////////////////////////////////
// Set of instantiations
typedef struct {
  int no_vars;     //number of unification variables
  unif_v_T vars[MAX_UNIF_VARS];
} unif_vars_T;

////////////////////////////////////////////
// Initializes the set of instantiations. 
int unif_init_vars();

////////////////////////////////////////////
// Prints the set of instantiations. 
int unif_print_vars();

//////////////////////////////////////////////////////////////
// Liberates the space allocated for the set of instantiations. 
int unif_free_vars();

//////////////////////////////////////////////////////////////////////////////////
// Instantiates the unification variable "var" to category "cat" and value "val". 
int unif_instantiate(unichar* var, l_category_T* cat, unichar* val);

//////////////////////////////////////////////////////////////////////////////////
// If variable "var" already instantiated, returns -1. Otherwise,
// instantiates the unification variable "var" to category "cat" and value whole index in the domain of "cat" is "val". 
// Returns 1 or -1 in case of error, 0 otherwise.
int unif_instantiate_index(unichar* var, l_category_T* cat, int val);

//////////////////////////////////////////////////////////////////////////////////
// Desinstantiates the unification variable "var". 
int unif_desinstantiate(unichar* var);

//////////////////////////////////////////////////////////////////////////////////
// Returns 1 if the unification variable "var" is instantiated, 0 otherwise.
int unif_instantiated(unichar* var);

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its value, 
// otherwise returns NULL.
unichar* unif_get_val(unichar* var);

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its index 
// in the domain of its category otherwise returns -1.
int unif_get_val_index(unichar* var);

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its index 
// in the domain of its category otherwise returns -1.
int unif_get_val_index(unichar* var);

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its category, 
// otherwisz returns NULL..
l_category_T* unif_get_cat(unichar* var);

#endif

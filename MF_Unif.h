/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/* Created by Agata Savary (savary@univ-mlv.fr)
 */


////////////////////////////////////////////////////////////
// Implementation of unification variables

#ifndef UnifH
#define UnifH

#include <stdio.h>
#include "MF_Global.h"
#include "MF_Unif.h"
#include "MF_LangMorpho.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

////////////////////////////////////////////
// Initializes the set of instantiations. 
int unif_init_vars(unif_vars_T* UNIF_VARS);

////////////////////////////////////////////
// Prints the set of instantiations. 
int unif_print_vars(unif_vars_T* UNIF_VARS);

//////////////////////////////////////////////////////////////
// Liberates the space allocated for the set of instantiations. 
int unif_free_vars(unif_vars_T* UNIF_VARS);

//////////////////////////////////////////////////////////////////////////////////
// Instantiates the unification variable "var" to category "cat" and value "val". 
int unif_instantiate(unif_vars_T* UNIF_VARS,unichar* var, l_category_T* cat, unichar* val);

//////////////////////////////////////////////////////////////////////////////////
// If variable "var" already instantiated, returns -1. Otherwise,
// instantiates the unification variable "var" to category "cat" and value whole index in the domain of "cat" is "val". 
// Returns 1 or -1 in case of error, 0 otherwise.
int unif_instantiate_index(unif_vars_T* UNIF_VARS,unichar* var, l_category_T* cat, int val);

//////////////////////////////////////////////////////////////////////////////////
// Desinstantiates the unification variable "var". 
int unif_desinstantiate(unif_vars_T* UNIF_VARS,unichar* var);

//////////////////////////////////////////////////////////////////////////////////
// Returns 1 if the unification variable "var" is instantiated, 0 otherwise.
int unif_instantiated(unif_vars_T* UNIF_VARS,unichar* var);

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its value, 
// otherwise returns NULL.
unichar* unif_get_val(unif_vars_T* UNIF_VARS,unichar* var);

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its index 
// in the domain of its category otherwise returns -1.
int unif_get_val_index(unif_vars_T* UNIF_VARS,unichar* var);

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its index 
// in the domain of its category otherwise returns -1.
int unif_get_val_index(unif_vars_T* UNIF_VARS,unichar* var);

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its category, 
// otherwisz returns NULL..
l_category_T* unif_get_cat(unif_vars_T* UNIF_VARS,unichar* var);

} // namespace unitex

#endif

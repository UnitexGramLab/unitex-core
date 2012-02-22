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

/* Created by Agata Savary (savary@univ-tours.fr)
 */


////////////////////////////////////////////////////////////
// Implementation of unification variables
////////////////////////////////////////////////////////////

#include <stdio.h>
#include "MF_Unif.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

///////////////////////////////////////////////////////////////////////////////////////
//Global structure describing the morphological categories of a language
//extern l_cats_T L_CATS;
///////////////////////////////////////////////////////////////////////////////////////
//Global structure describing the morphological system of a language
//extern l_classes_T L_CLASSES;
//////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////
// Set of all existing instantiations
///unif_vars_T UNIF_VARS;

////////////////////////////////////////////
// Initializes the set of instantiations. 
int unif_init_vars(MultiFlex_ctx* p_multiFlex_ctx) {
  (p_multiFlex_ctx->UNIF_VARS).no_vars = 0;
  return 0;
}

////////////////////////////////////////////
// Prints the set of instantiations. 
int unif_print_vars(MultiFlex_ctx* p_multiFlex_ctx) {
int v;
int i;
u_printf("INSTANTIATION VARIABLES:\n");
for (v=0;v<(p_multiFlex_ctx->UNIF_VARS).no_vars;v++) {
    u_printf("%S:%S=",(p_multiFlex_ctx->UNIF_VARS).vars[v].id,(p_multiFlex_ctx->UNIF_VARS).vars[v].cat->name);
    i=(p_multiFlex_ctx->UNIF_VARS).vars[v].val;
    u_printf("%S\n",(p_multiFlex_ctx->UNIF_VARS).vars[v].cat->values[i]);
}
return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// If variable "var" already instantiated, returns -1. Otherwise,
// instantiates the unification variable "var" to category "cat" and value "val". 
// Returns 1 or -1 in case of error, 0 otherwise.
int unif_instantiate(MultiFlex_ctx* p_multiFlex_ctx,unichar* var, l_category_T* cat, unichar* val) {
  int i;
  int v;
  //Check if not yet instantiated.
  if (unif_instantiated(p_multiFlex_ctx,var))
      return -1;
 
  i = (p_multiFlex_ctx->UNIF_VARS).no_vars;

  //Category
  (p_multiFlex_ctx->UNIF_VARS).vars[i].cat = cat;

  //value
  v = is_valid_val(cat,val);
  if (v == -1) {
    error("Instantiation impossible: %S is an invalid value in category %S.\n",val,cat->name);
    return 1;    
  }
  (p_multiFlex_ctx->UNIF_VARS).vars[i].val = v;

  //id
  (p_multiFlex_ctx->UNIF_VARS).vars[i].id = u_strdup(var);

  (p_multiFlex_ctx->UNIF_VARS).no_vars++;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// If variable "var" already instantiated, returns -1. Otherwise,
// instantiates the unification variable "var" to category "cat" and value whole index in the domain of "cat" is "val". 
// Returns 1 or -1 in case of error, 0 otherwise.
int unif_instantiate_index(MultiFlex_ctx* p_multiFlex_ctx,unichar* var, l_category_T* cat, int val) {
  //Check if not yet instantiated.
  if (unif_instantiated(p_multiFlex_ctx,var))
      return -1;
  
  //Check if the value is in the domain of "cat"
  if (cat->no_values <= val)
    return -1;
 
  int i = (p_multiFlex_ctx->UNIF_VARS).no_vars;
  //Category
  (p_multiFlex_ctx->UNIF_VARS).vars[i].cat = cat;
  //Value
  (p_multiFlex_ctx->UNIF_VARS).vars[i].val = val;
  //Variable's id
  (p_multiFlex_ctx->UNIF_VARS).vars[i].id = u_strdup(var);

  (p_multiFlex_ctx->UNIF_VARS).no_vars++;
  return 0;

}

//////////////////////////////////////////////////////////////////////////////////
// Desinstantiates the unification variable "var". 
int unif_desinstantiate(MultiFlex_ctx* p_multiFlex_ctx,unichar* var) {
  int v, w, found;

  found = 0;
  for (v=0; v<(p_multiFlex_ctx->UNIF_VARS).no_vars; v++)
    if (!u_strcmp(var,(p_multiFlex_ctx->UNIF_VARS).vars[v].id)) {
        found = 1;
        break;
    }
  // if found v points to the variable following the one we want to eliminate
  if (found) {
    free((p_multiFlex_ctx->UNIF_VARS).vars[v].id);
    for (w=v+1; w<(p_multiFlex_ctx->UNIF_VARS).no_vars;w++)
      (p_multiFlex_ctx->UNIF_VARS).vars[w-1] = (p_multiFlex_ctx->UNIF_VARS).vars[w];
    (p_multiFlex_ctx->UNIF_VARS).no_vars--;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// Returns 1 if the unification variable "var" is instantiated, 0 otherwise.
int unif_instantiated(MultiFlex_ctx* p_multiFlex_ctx,unichar* var) {
  int v;
  for (v=0; v<(p_multiFlex_ctx->UNIF_VARS).no_vars; v++)
    if (!u_strcmp(var,(p_multiFlex_ctx->UNIF_VARS).vars[v].id))
      return 1;
  return 0;
  }

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its index 
// in the domain of its category otherwise returns -1.
int unif_get_val_index(MultiFlex_ctx* p_multiFlex_ctx,unichar* var) {
  int v;
  for (v=0; v<(p_multiFlex_ctx->UNIF_VARS).no_vars; v++)
    if (!u_strcmp(var,(p_multiFlex_ctx->UNIF_VARS).vars[v].id)) 
      return (p_multiFlex_ctx->UNIF_VARS).vars[v].val;
  return -1;
  }

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its value, 
// otherwise returns NULL.
unichar* unif_get_val(MultiFlex_ctx* p_multiFlex_ctx,unichar* var) {
  int v,i;
  for (v=0; v<(p_multiFlex_ctx->UNIF_VARS).no_vars; v++)
    if (!u_strcmp(var,(p_multiFlex_ctx->UNIF_VARS).vars[v].id)) { 
      i = (p_multiFlex_ctx->UNIF_VARS).vars[v].val;
      return (p_multiFlex_ctx->UNIF_VARS).vars[v].cat->values[i];
  }
  return NULL;
  }

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its category, 
// otherwise returns NULL.
l_category_T* unif_get_cat(MultiFlex_ctx* p_multiFlex_ctx,unichar* var) {
  int v;
  for (v=0; v<(p_multiFlex_ctx->UNIF_VARS).no_vars; v++)
    if (!u_strcmp(var,(p_multiFlex_ctx->UNIF_VARS).vars[v].id)) 
      return (p_multiFlex_ctx->UNIF_VARS).vars[v].cat;
  return NULL;
  }

//////////////////////////////////////////////////////////////
// Liberates the space allocated for the set of instantiations. 
int unif_free_vars(MultiFlex_ctx* p_multiFlex_ctx) {
  int v;
  for (v=0; v<(p_multiFlex_ctx->UNIF_VARS).no_vars; v++)
    free((p_multiFlex_ctx->UNIF_VARS).vars[v].id);
  return 0;
}

} // namespace unitex

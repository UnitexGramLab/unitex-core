/*
  * Unitex 
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

/* Created by Agata Savary (savary@univ-tours.fr)
 * Last modification on June 23 2005
 */
//---------------------------------------------------------------------------

////////////////////////////////////////////////////////////
// Implementation of unification variables
////////////////////////////////////////////////////////////

#include <stdio.h>
#include "MF_Unif.h"
#include "Error.h"

///////////////////////////////////////////////////////////////////////////////////////
//Global structure describing the morphological categories of a language
extern l_cats_T L_CATS;
///////////////////////////////////////////////////////////////////////////////////////
//Global structure describing the morphological system of a language
extern l_classes_T L_CLASSES;
//////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////
// Set of all existing instantiations
unif_vars_T UNIF_VARS;

////////////////////////////////////////////
// Initializes the set of instantiations. 
int unif_init_vars() {
  UNIF_VARS.no_vars = 0;
  return 0;
}

////////////////////////////////////////////
// Prints the set of instantiations. 
int unif_print_vars() {
  int v;
  unichar tmp[100];
  int i;

  u_strcpy_char(tmp, "INSTANTIATION VARIABLES:\n");
  u_prints(tmp);
  for (v=0; v<UNIF_VARS.no_vars; v++) {
    u_prints(UNIF_VARS.vars[v].id);
    u_strcpy_char(tmp, ":");
    u_prints(tmp);
    u_prints(UNIF_VARS.vars[v].cat->name);
    u_strcpy_char(tmp, "=");
    u_prints(tmp);
    i = UNIF_VARS.vars[v].val;
    u_prints(UNIF_VARS.vars[v].cat->values[i]);
    u_strcpy_char(tmp, "\n");
    u_prints(tmp);
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// If variable "var" already instantiated, returns -1. Otherwise,
// instantiates the unification variable "var" to category "cat" and value "val". 
// Returns 1 or -1 in case of error, 0 otherwise.
int unif_instantiate(unichar* var, l_category_T* cat, unichar* val) {
  int i;
  int v;
  //Check if not yet instantiated.
  if (unif_instantiated(var))
      return -1;
 
  i = UNIF_VARS.no_vars;

  //Category
  UNIF_VARS.vars[i].cat = cat;

  //value
  v = is_valid_val(cat,val);
  if (v == -1) {
    error("Instantiation impossible: ");
    error(val);
    error(" is an invalid value in category ");
    error(cat->name);
    error(".\n");
    return 1;    
  }
  UNIF_VARS.vars[i].val = v;

  //id
  if (!(UNIF_VARS.vars[i].id = (unichar*) malloc((u_strlen(var)+1)*sizeof(unichar)))) {
     fatal_error("Not enough memory in function unif_instantiate\n");
  }
  u_strcpy(UNIF_VARS.vars[i].id,var);

  UNIF_VARS.no_vars++;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// If variable "var" already instantiated, returns -1. Otherwise,
// instantiates the unification variable "var" to category "cat" and value whole index in the domain of "cat" is "val". 
// Returns 1 or -1 in case of error, 0 otherwise.
int unif_instantiate_index(unichar* var, l_category_T* cat, int val) {
  //Check if not yet instantiated.
  if (unif_instantiated(var))
      return -1;
  
  //Check if the value is in the domain of "cat"
  if (cat->no_values <= val)
    return -1;
 
  int i = UNIF_VARS.no_vars;
  //Category
  UNIF_VARS.vars[i].cat = cat;
  //Value
  UNIF_VARS.vars[i].val = val;
  //Variable's id
  if (!(UNIF_VARS.vars[i].id = (unichar*) malloc((u_strlen(var)+1)*sizeof(unichar)))) {
     fatal_error("Not enough memory in function unif_instantiate_index\n");
  }
  u_strcpy(UNIF_VARS.vars[i].id,var);

  UNIF_VARS.no_vars++;
  return 0;

}

//////////////////////////////////////////////////////////////////////////////////
// Desinstantiates the unification variable "var". 
int unif_desinstantiate(unichar* var) {
  int v, w, found;

  found = 0;
  for (v=0; v<UNIF_VARS.no_vars && (!found); v++)
    if (!u_strcmp(var,UNIF_VARS.vars[v].id))
      found = 1;
  if (found) { //v points to the variable following the one we want to eliminate
    for (w=v; w<UNIF_VARS.no_vars;w++)
      UNIF_VARS.vars[w-1] = UNIF_VARS.vars[w];
    UNIF_VARS.no_vars--;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// Returns 1 if the unification variable "var" is instantiated, 0 otherwise.
int unif_instantiated(unichar* var) {
  int v;
  for (v=0; v<UNIF_VARS.no_vars; v++)
    if (!u_strcmp(var,UNIF_VARS.vars[v].id))
      return 1;
  return 0;
  }

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its index 
// in the domain of its category otherwise returns -1.
int unif_get_val_index(unichar* var) {
  int v;
  for (v=0; v<UNIF_VARS.no_vars; v++)
    if (!u_strcmp(var,UNIF_VARS.vars[v].id)) 
      return UNIF_VARS.vars[v].val;
  return -1;
  }

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its value, 
// otherwise returns NULL.
unichar* unif_get_val(unichar* var) {
  int v,i;
  for (v=0; v<UNIF_VARS.no_vars; v++)
    if (!u_strcmp(var,UNIF_VARS.vars[v].id)) { 
      i = UNIF_VARS.vars[v].val;
      return UNIF_VARS.vars[v].cat->values[i];
  }
  return NULL;
  }

//////////////////////////////////////////////////////////////////////////////////
// If the unification variable "var" is instantiated returns its category, 
// otherwise returns NULL.
l_category_T* unif_get_cat(unichar* var) {
  int v;
  for (v=0; v<UNIF_VARS.no_vars; v++)
    if (!u_strcmp(var,UNIF_VARS.vars[v].id)) 
      return UNIF_VARS.vars[v].cat;
  return NULL;
  }

//////////////////////////////////////////////////////////////
// Liberates the space allocated for the set of instantiations. 
int unif_free_vars() {
  int v;
  for (v=0; v<UNIF_VARS.no_vars; v++)
    free(UNIF_VARS.vars[v].id);
  return 0;
}

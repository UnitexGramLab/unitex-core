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

/* Created by Agata Savary (savary@univ-mlv.fr)
 */


////////////////////////////////////////////////////////////
// Implementation of unification variables

#ifndef UnifBaseH
#define UnifBaseH

#include <stdio.h>
#include "MF_LangMorphoBase.h"

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

#endif

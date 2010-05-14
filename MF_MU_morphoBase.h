/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
//// Morphology and inflection of a multi-word unit ///
/********************************************************************************/

#ifndef MU_morphoBaseH
#define MU_morphoBaseH

#include "Unicode.h"
#include "MF_FormMorpho.h"
#include "MF_SU_morphoBase.h"

/////////////////////////////////////////////////
//Maximum number of constituents in a multi-word unit
#define MAX_UNITS 20

/////////////////////////////////////////////////
//Structure for the morphology of a multi-word (inflected) form
typedef struct {
  unichar* form;         //e.g. "pommes de terre"
  f_morpho_T* features;   //e.g. {Gen=fem, Nb=pl}
} MU_f_T;

/////////////////////////////////////////////////
// Set of inflected forms of a MWU
typedef struct  {
  int no_forms;   //number of inflected forms
  MU_f_T *forms;  //table of inflected forms
} MU_forms_T;

/////////////////////////////////////////////////
//Structure for the lemma of a MWU
typedef struct {
  int no_units;
  SU_id_T* units[MAX_UNITS];	//e.g. pointer to "vive" in the paadigm of "vif"
  l_class_T *cl;	        //e_.g. adj
  char *paradigm;		//e.g. N41
} MU_lemma_T;

/////////////////////////////////////////////////
// Structure for the unique identification of an inflected form of a MWU.
// See comment on unique identification of single graphical units (MF_SU_morpho.h file)
typedef struct {
  unichar* form;         // the textual form, e.g. "pommes de terre"
  MU_lemma_T *MU_lemma; //lemma and its info
  f_morpho_T* feat;   //the form's morphology, e.g. {Gen=fem; Nb=pl; Case=I}
  //  int form_nr;   	//ordinal number of the form in the list of all inflected forms of the lemma
} MU_id_T;

#endif

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

/* Created by Agata Savary (agata.savary@univ-tours.fr)
 */

#ifndef SU_morphoBaseH
#define SU_morphoBaseH

#include "Unicode.h"
#include "Alphabet.h"
#include "MF_FormMorpho.h"
#include "Korean.h"


/////////////////////////////////////////////////
//Structure for the morphology of a single (inflected) form
//of a single graphical unit
//e.g. for French
//          form = "vives"
//          features = {Gen=fem, Nb=pl}
typedef struct {
   unichar* form;         //e.g. "vives", or "-"
   unichar* local_semantic_code;
   union {
      f_morpho_T* features;   //e.g. {Gen=fem, Nb=pl}, or {}
      unichar* raw_features; // used for simple words
   };
} SU_f_T;

/////////////////////////////////////////////////
// Set of inflected forms
typedef struct  {
  int no_forms;   //number of inflected forms
  SU_f_T *forms;  //table of inflected forms
} SU_forms_T;

/////////////////////////////////////////////////
//Structure for the lemma of a single word
typedef struct {
  unichar* unit;		//e.g. "vif"
  l_class_T *cl;	        //e.g. adj
  char *paradigm;		//e.g. A41
} SU_lemma_T;

/////////////////////////////////////////////////
// Possible types of a unit
// - a word
// - a separator
//typedef enum {word,sep} SU_unit_T;

/////////////////////////////////////////////////
// Structure for the unique identification of an inflected form
// We suppose that each inflected form may be uniquely identified on the basis of 4 elements (in case of a word):
// the form, its lemma, its paradigm, its features. In case of a separator or any other uninflected form,
// the unit itself (e.g. ",") is enough to uniquely identify itself.
// Given these elements we may access the form indentifier and conversely. See * below.
// One possibility is to have a deterministic linear order of all forms (variants included) so that
// the same form always gets the same identifier.
typedef struct {
  unichar* form;          //the textual form
  //  SU_unit_T type;        //word or sep
  SU_lemma_T *lemma;     //lemma and its info; empty for a separator
  f_morpho_T* feat;    //the form's morphology, e.g. {Gen=fem; Nb=sing; Case=I}
  //  int form_nr;   	 //identifier of the form in the list of all inflected forms of the lemma; irrelevant for a separator
} SU_id_T;

#endif

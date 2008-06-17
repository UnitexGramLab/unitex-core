/*
  * Unitex 
  *
  * Copyright (C) 2001-2008 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

/* Created by Agata Savary (agata.savary@univ-tours.fr)
 * Last modification on July 11 2005
 */
//---------------------------------------------------------------------------

/********************************************************************************/
/********************************************************************************/

#ifndef SU_morphoH
#define SU_morphoH

#include "Unicode.h"
#include "Alphabet.h"
#include "MF_FormMorpho.h"

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

////////////////////////////////////////////
// For a given single word, generates all the inflected forms corresponding to the given inflection features.
// SU_id:  single word form's identifier
// feat: morphology of the desired forms, e.g. {Gen=fem, Case=Inst}, or {} (if separator)
// forms: return parameter; set of the inflected forms corresponding to the given inflection features
//        e.g. (3,{[reka,{Gen=fem,Nb=sing,Case=Instr}],[rekami,{Gen=fem,Nb=pl,Case=Instr}],[rekoma,{Gen=fem,Nb=pl,Case=Instr}]})
//        or   (1,{["-",{}]})
// Returns 0 on success, 1 otherwise.   
int SU_inflect(SU_id_T* SU_id,f_morpho_T* feat, SU_forms_T* forms,int);

/* This prototype has been added in order to deal with simple words */
int SU_inflect(unichar* lemma,char* inflection_code,unichar **filters,SU_forms_T* forms,int);

////////////////////////////////////////////
// Liberates the memory allocated for a set of forms
int SU_delete_inflection(SU_forms_T* forms);

////////////////////////////////////////////
// Returns in 'feat' a copy of the inflection features of the given form.
// Initially 'feat' has its space allocated but is empty.
// Returns 1 on error, 0 otherwise.
int SU_cpy_features(f_morpho_T* feat,SU_id_T* SU_id);

////////////////////////////////////////////
// Liberates the memory allocated for a form's morphology.
void SU_delete_features(f_morpho_T* f);

////////////////////////////////////////////
// Returns the word form's identifier on the basis of the form, its lemma, its inflection paradigm, and its inflection features.
// SU_form : form and its inflection features, e.g. [rekoma,{Gen=fem,Nb=pl,Case=Instr}], or ["-",{}]
// SU_lemma : lemma and its inflection paradigm, e.g. [reka,noun,N56,{"Conc"},""body"], or void (if separator)
// Returns the pointer to the forms identifier on success 
// (e.g. ->("reka",word,[reka,noun,N56,{"Conc"},"body"],{Gen=fem; Nb=sing; Case=I})), NULL otherwise.
// The identifier is allocated in this function. The liberation must be done by the calling function.
SU_id_T*  SU_get_id(SU_f_T* SU_form, SU_lemma_T* SU_lemma);

////////////////////////////////////////////
// Gets next unit from the input line 'line' and puts it to 'unit'.
// 'max' is the maximum length of the copied sequence
// 'alph' is the current alphabet
// This is the essential function defining the segmentation
// of a text into units.
// If "eliminate_bcksl" is set to 1 each protecting backslash is omitted in the
// copied sequence.                                                            
// Returns the length of the scanned sequence.
// Returns -2 on memory allocation problem.
int SU_get_unit(unichar* unit,unichar* line, int max, Alphabet* alph, int eliminate_bcksl);

////////////////////////////////////////////
// Liberates the memory allocated for a form's id.
int SU_delete_id(SU_id_T* id);

////////////////////////////////////////////
// Prints a form and its inflection features.
int SU_print_f(SU_f_T* f);

////////////////////////////////////////////
// Prints a set of forms and their inflection features.
int SU_print_forms(SU_forms_T* f);

////////////////////////////////////////////
// Prints a lemma and its info.
int SU_print_lemma(SU_lemma_T* l);

////////////////////////////////////////////
// Initialise the sample lemma structure for tests.
int SU_init_lemma(SU_lemma_T* l, char* word, char* cl, char* para);

////////////////////////////////////////////
// Delete sample lemma stucture for tests.
int SU_delete_lemma(SU_lemma_T* l);

////////////////////////////////////////////
// COMMENT CONCERNING THE UNIQUE IDENTIFICATION OF SINGLE WORD FORMS
//
// In order to uniquely identify a word form four elements are necessary:
//	- the form  (e.g. "rekami")
//	- its lemma (e.g. "reka")
//	- its inflection paradigm (e.g. N56)
//	- its inflection features (e.g. {Gen=fem,Nb=pl,Case=Inst}
// Note that omitting one of these elements may introduce ambiguities from the morphological point of view.
// Examples :
//	1) If the form itself is missing, an ambiguity may exist between variants corresponding to the same inflection features
//	   e.g. given only the lemma "reka", the paradigm N56, and the features {Gen=fem,Nb=pl,Case=Inst}), we may not distinguish
// 	   between "rekami" and "rekoma"
//	2) If the lemma is missing we may not lemmatize the form (unless the paradigm allows to do that on the basis of the 3 elements).
//         A certain form may be identical for two different lemmas.  
//	3) If the inflection paradigm is missing we may not produce other inflected forms.
//	4) If the inflection features are missing there may be an ambiguity in case of homographs e.g. "rece" may be both
//	   {Gen=fem,Nb=pl,Case=Nom} and {Gen=fem,Nb=pl,Case=Acc}
//
// A UNIQUE identification of a form in the set of all inflected forms
#endif

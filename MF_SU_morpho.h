/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef SU_morphoH
#define SU_morphoH

#include "Unicode.h"
#include "Alphabet.h"
#include "MF_FormMorpho.h"
#include "Korean.h"
#include "MF_SU_morphoBase.h"
#include "MF_Global.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

////////////////////////////////////////////
// For a given single word, generates all the inflected forms corresponding to the given inflection features.
// SU_id:  single word form's identifier
// feat: morphology of the desired forms, e.g. {Gen=fem, Case=Inst}, or {} (if separator)
// forms: return parameter; set of the inflected forms corresponding to the given inflection features
//        e.g. (3,{[reka,{Gen=fem,Nb=sing,Case=Instr}],[rekami,{Gen=fem,Nb=pl,Case=Instr}],[rekoma,{Gen=fem,Nb=pl,Case=Instr}]})
//        or   (1,{["-",{}]})
// Returns 0 on success, 1 otherwise.
int SU_inflect(MultiFlex_ctx* p_multiFlex_ctx,
        SU_id_T* SU_id,f_morpho_T* feat,
        SU_forms_T* forms);

/* This prototype has been added in order to deal with simple words */
int SU_inflect(MultiFlex_ctx* p_multiFlex_ctx,
        unichar* lemma,
        char* inflection_code,SU_forms_T* forms);

////////////////////////////////////////////
// Liberates the memory allocated for a set of forms
void SU_delete_inflection(SU_forms_T* forms);

////////////////////////////////////////////
// Returns in 'feat' a copy of the inflection features of the given form.
// Initially 'feat' has its space allocated but is empty.
// Returns 1 on error, 0 otherwise.
//int SU_cpy_features(f_morpho_T* feat,SU_id_T* SU_id);

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
// Initialize the single-unit 'forms' with null values
// We suppose that 'forms' has its space allocated
void SU_init_forms(SU_forms_T* forms);

////////////////////////////////////////////
// Initialize the set of inflected forms "forms" with
// the unique form "form"
// E.g. if form = "rekami" then forms becomes (1,{("rekami",NULL)}
void SU_init_invariable_form(SU_forms_T* forms,const unichar* form);

/////////////////////////////////////////////////////////////////////
// Same as SU_init_invariable_form but the second parameter is a char*
void SU_init_invariable_form_char(SU_forms_T* forms,const char* form);

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
int SU_init_lemma(struct l_morpho_t* pL_MORPHO,SU_lemma_T* l, char* word, char* cl, char* para);

////////////////////////////////////////////
// Delete sample lemma stucture for tests.
int SU_delete_lemma(SU_lemma_T* l);

////////////////////////////////////////////
// COMMENT CONCERNING THE UNIQUE IDENTIFICATION OF SINGLE WORD FORMS
//
// In order to uniquely identify a word form four elements are necessary:
//  - the form  (e.g. "rekami")
//  - its lemma (e.g. "reka")
//  - its inflection paradigm (e.g. N56)
//  - its inflection features (e.g. {Gen=fem,Nb=pl,Case=Inst}
// Note that omitting one of these elements may introduce ambiguities from the morphological point of view.
// Examples :
//  1) If the form itself is missing, an ambiguity may exist between variants corresponding to the same inflection features
//     e.g. given only the lemma "reka", the paradigm N56, and the features {Gen=fem,Nb=pl,Case=Inst}), we may not distinguish
//     between "rekami" and "rekoma"
//  2) If the lemma is missing we may not lemmatize the form (unless the paradigm allows to do that on the basis of the 3 elements).
//         A certain form may be identical for two different lemmas.
//  3) If the inflection paradigm is missing we may not produce other inflected forms.
//  4) If the inflection features are missing there may be an ambiguity in case of homographs e.g. "rece" may be both
//     {Gen=fem,Nb=pl,Case=Nom} and {Gen=fem,Nb=pl,Case=Acc}
//
// A UNIQUE identification of a form in the set of all inflected forms

} // namespace unitex

#endif

/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef MU_morphoH
#define MU_morphoH

#include "MF_MU_morphoBase.h"
#include "MF_Global.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

////////////////////////////////////////////
// For a given multi-word unit, generates all the inflected forms,
// e.g. {["mémoire vive",{Gen=fem,Nb=sing}],["mémoires vives",{Gen=fem,Nb=pl}]}
// Returns 0 on success, 1 otherwise.   
int MU_inflect(MultiFlex_ctx* p_multiFlex_ctx,
		MU_lemma_T* lemma, MU_forms_T* forms);

////////////////////////////////////////////
// Liberates the memory allocated for a set of forms
void MU_delete_inflection(MU_forms_T* forms);

/*
////////////////////////////////////////////
// Returns the word form's identifier on the basis of the form, its lemma, its inflection paradigm, and its inflection features.
// MU_form : form and its inflection features, e.g. ["mémoires vives",{Gen=fem,Nb=pl]
// MU_lemma : lemma and its inflection paradigm, 
//                e.g. [[[word,"mémoires",->[mémoire],2],[sep," "],[word,"vives",->[vif],4]],noun,NC76,{"Conc"},"computing"]
// Returns the pointer to the forms identifier on success (e.g. ->([mémoire vive,noun,NC76,{"Conc"},"computing"],2)), NULL otherwise.
MU_id_T*  MU_get_id(MU_f_T* MU_form, MU_lemma_T* MU_lemma);
*/

////////////////////////////////////////////
// Liberates the memory allocated for a form's id.
int MU_free_id(MU_id_T* id);

////////////////////////////////////////////
// Initialize the multi-unit 'forms' with null values
// We suppose that 'forms' has its space allocated
void MU_init_forms(MU_forms_T* forms);

////////////////////////////////////////////
// Add an empty form with empty features 'feat' to the initially empty set of forms 'forms'
void MU_add_empty_form(MU_forms_T* forms);

////////////////////////////////////////////
// Concatenante each simple form in 'SU_forms' in front of
// each multi-unit form in 'MU_forms'. Put the resulting
// forms into 'forms'
// E.g. while generating the instrucmental of "rece pelne roboty", if we have :
// SU_forms = {("rekami",{Case=Inst, Nb=pl, Gen=fem}), ("rekoma",{Case=Inst, Nb=pl, Gen=fem})}
// MU_forms = {("pelnymi roboty",{Case=Inst, Nb=pl, Gen=fem})}
// then we obtain {("rekami pelnymi roboty",{Case=Inst, Nb=pl, Gen=fem}), ("rekoma pelnymi roboty",{Case=Inst, Nb=pl, Gen=fem})}
// Initially, 'forms' has its space allocated but is empty.
void MU_concat_forms(SU_forms_T* SU_forms, MU_forms_T* MU_forms, MU_forms_T* forms);

////////////////////////////////////////////
// Add forms appearing in 'new_forms' to 'forms' so that
// no form appears twice in the result.
// Exist in case of errors.
void MU_merge_forms(MU_forms_T* forms, MU_forms_T* new_forms);

////////////////////////////////////////////
// Prints a form and its inflection features.
int MU_print_f(MU_f_T* f);

////////////////////////////////////////////
// Prints a set of forms and their inflection features.
int MU_print_forms(MU_forms_T* f);

////////////////////////////////////////////
// Prints a lemma and its info.
int MU_print_lemma(MU_lemma_T* l);

////////////////////////////////////////////
// Delete sample lemma stucture for tests.
void MU_delete_lemma(MU_lemma_T* l);

} // namespace unitex

#endif

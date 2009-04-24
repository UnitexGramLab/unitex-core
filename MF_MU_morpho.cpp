/*
 * Unitex
 *
 * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
 */

/********************************************************************************/
//// Morphology and inflection of a multi-word unit ///
/********************************************************************************/

#include "MF_MU_morpho.h"
#include "MF_Unif.h"
#include "MF_MU_graph.h"
#include "Error.h"

////////////////////////////////////////////
// For a given multi-word unit, generates all the inflected forms,
// e.g. {["m�moire vive",{Gen=fem,Nb=sing}],["m�moires vives",{Gen=fem,Nb=pl}]}
// Initially, 'forms' has its space allocated but is empty.
// Returns 0 on success, 1 otherwise.
int MU_inflect(MU_lemma_T* lemma, MU_forms_T* forms) {
	int err;

	//Explore the inflection tranducer and produce the inflected forms
	err = MU_graph_explore_graph(lemma, forms);
	if (err)
		return 1;
	else
		return 0;
}

////////////////////////////////////////////
// Liberates the memory allocated for a set of forms
void MU_delete_inflection(MU_forms_T* forms) {
	int f;
	if (!forms)
		return;
	for (f = 0; f < forms->no_forms; f++) {
		if (forms->forms[f].form)
			free(forms->forms[f].form);
		if (forms->forms[f].features)
			f_delete_morpho(forms->forms[f].features);
	}
	free(forms->forms);
}

/*
 ////////////////////////////////////////////
 // Returns the unit's form's identifier on the basis of the form, its lemma, its inflection paradigm, and its inflection features.
 // MU_form : form and its inflection features, e.g. ["m�moires vives",{Gen=fem,Nb=pl]
 // MU_lemma : lemma and its inflection paradigm,
 //                e.g. [[[word,"m�moires",->[m�moire],2],[sep," "],[word,"vives",->[vif],4]],noun,NC76,{"Conc"},"computing"]
 // Returns the pointer to the forms identifier on success (e.g. ->([m�moire vive,noun,NC76,{"Conc"},"computing"],2)), NULL otherwise.
 MU_id_T*  MU_get_id(MU_f_T* MU_form, MU_lemma_T* MU_lemma) {
 return NULL;
 }
 */

////////////////////////////////////////////
// Liberates the memory allocated for a form's id.
void MU_delete_id(MU_id_T* id) {
	if (id) {
		free(id->form);
		MU_delete_lemma(id->MU_lemma);
		f_delete_morpho(id->feat);
	}
	free(id);
}

////////////////////////////////////////////
// Compare two multi-unit forms
// Return 0 if they are identical, 1 otherwise.
int MU_form_cmp(MU_f_T f1, MU_f_T f2) {
	if (u_strcmp(f1.form, f2.form))
		return 1;
	return f_morpho_cmp(f1.features, f2.features);
}

////////////////////////////////////////////
// Initialize the multi-unit 'forms' with null values
// We suppose that 'forms' has its space allocated
void MU_init_forms(MU_forms_T* forms) {
	forms->no_forms = 0;
	forms->forms = NULL;
}

////////////////////////////////////////////
// Add an empty form with empty features 'feat' to the initially empty set of forms 'forms'
void MU_add_empty_form(MU_forms_T* forms) {
	//Allocate space for a couple (epsilon,empty_set)
	forms->forms = (MU_f_T*) malloc(sizeof(MU_f_T));
	if (!forms->forms) {
		fatal_alloc_error("MU_add_empty_form");
	}
	MU_f_T* f;
	f = &(forms->forms[0]);
	f->form = (unichar*) malloc(sizeof(unichar));
	if (f->form == NULL) {
		fatal_alloc_error("MU_add_empty_form");
	}
	f->form[0] = (unichar) '\0';
	f->features = (f_morpho_T*) malloc(sizeof(f_morpho_T));
	if (f->features == NULL) {
		fatal_alloc_error("MU_add_empty_form");
	}
	f->features->no_cats = 0;

	forms->no_forms++;
}

////////////////////////////////////////////
// Concatenante each simple form in 'SU_forms' in front of
// each multi-unit form in 'MU_forms'. Add the resulting
// forms into 'forms'
// E.g. while generating the instrumental of "rece pelne roboty", if we have :
// SU_forms = {("rekami",{Case=Inst, Nb=pl, Gen=fem}), ("rekoma",{Case=Inst, Nb=pl, Gen=fem})}
// MU_forms = {("pelnymi roboty",{Case=Inst, Nb=pl, Gen=fem})}
// forms = {("rak pelnych roboty",{Case=Acc, Nb=pl, Gen=fem})}
// then we obtain {("rak pelnych roboty",{Case=Acc, Nb=pl, Gen=fem}),
//                 ("rekami pelnymi roboty",{Case=Inst, Nb=pl, Gen=fem}),
//                 ("rekoma pelnymi roboty",{Case=Inst, Nb=pl, Gen=fem})}
// Initially, 'forms' has its space allocated, it may be empty or non empty.
// If it is non empty, the existing forms must not be lost
void MU_concat_forms(SU_forms_T* SU_forms, MU_forms_T* MU_forms,
		MU_forms_T* forms) {
	int sf; //Index of a simple form in SU_forms
	int mf; //Index of a multi-word form in MU_form
	int f; //Index of a concatenated form

	if (MU_forms->no_forms && SU_forms->no_forms) { //Check if there is anything to concatenante
		forms->forms = (MU_f_T*) realloc(forms->forms, (forms->no_forms
				+ MU_forms->no_forms * SU_forms->no_forms) * sizeof(MU_f_T));
		if (!forms->forms) {
			fatal_alloc_error("MU_concat_forms");
		}
		f = forms->no_forms;
		for (sf = 0; sf < SU_forms->no_forms; sf++)
			for (mf = 0; mf < MU_forms->no_forms; mf++) {
				forms->forms[f].form = (unichar*) malloc((u_strlen(
						MU_forms->forms[mf].form) + u_strlen(
						SU_forms->forms[sf].form) + 1) * sizeof(unichar));
				if (!forms->forms[f].form) {
					fatal_alloc_error("MU_concat_forms");
				}
				//Concatenate the forms
				u_strcpy(forms->forms[f].form, SU_forms->forms[sf].form);
				u_strcat(forms->forms[f].form, MU_forms->forms[mf].form);
				//Copy the features
				forms->forms[f].features = (f_morpho_T*) malloc(
						sizeof(f_morpho_T));
				if (!forms->forms[f].features) {
					fatal_alloc_error("MU_concat_forms");
				}
				forms->forms[f].features->no_cats
						= MU_forms->forms[mf].features->no_cats;
				for (int c = 0; c < MU_forms->forms[mf].features->no_cats; c++)
					forms->forms[f].features->cats[c]
							= MU_forms->forms[mf].features->cats[c];
				f++;
			}
		forms->no_forms = f;
	} else
		//If there is nothing to concatenate
		forms = NULL;
}

////////////////////////////////////////////
// Add forms appearing in 'new_forms' to 'forms' so that
// no form appears twice in the result. The forms allocated in
// 'new_forms' are copied by pointer assignment. They shouldn't be
// liberated while 'new_forms' are liberated.
// Exit in case of errors.
void MU_merge_forms(MU_forms_T* forms, MU_forms_T* new_forms) {
	int nb_mf; //Number of forms to be added to 'forms'
	int mf; //index of a sigle MU form in 'forms'
	int nmf; //index of a sigle MU form in 'new_forms
	int found; //Boolean showing if a search form has been found

	//Check how many forms are to be added to 'forms'
	nb_mf = 0;
	for (nmf = 0; nmf < new_forms->no_forms; nmf++) {
		mf = 0;
		found = 0;
		while (mf < forms->no_forms && !found) {
			if (!MU_form_cmp(forms->forms[mf], new_forms->forms[nmf])) //If forms are identical
				found = 1;
			mf++;
		}
		if (!found)
			nb_mf++;
	}

	//Add the new forms
	if (nb_mf) { //If any forms are to be added
		//Reallocate the memory for new forms
		forms->forms = (MU_f_T*) realloc(forms->forms, sizeof(MU_f_T)
				* (forms->no_forms + nb_mf));
		if (!forms->forms) {
			fatal_alloc_error("MU_merge_forms");
		}
		//Treat each new form
		for (nmf = 0; nmf < new_forms->no_forms; nmf++) {
			//Check if the new form exists already in 'forms'
			mf = 0;
			found = 0;
			while (mf < forms->no_forms && !found) {
				if (!MU_form_cmp(forms->forms[mf], new_forms->forms[nmf])) //If forms are identical
					found = 1;
				mf++;
			}
			//If the new form does not exist in 'forms', add it
			if (!found) {
				forms->forms[forms->no_forms] = new_forms->forms[nmf];
				//If a for has been added to a different list, it shouldn't be accessible from the old one (otherwise memory liberation problems).
				new_forms->forms[nmf].form = NULL;
				new_forms->forms[nmf].features = NULL;
				forms->no_forms++;
			}
			//Otherwise free the space allocated for the form
			else {
				free(new_forms->forms[nmf].form);
				f_delete_morpho(new_forms->forms[nmf].features);
			}
		}
	}
}

////////////////////////////////////////////
// Prints a form and its inflection features.
int MU_print_f(MU_f_T* f) {
	u_printf("%S : ", f->form);
	f_print_morpho(f->features);
	return 0;
}

////////////////////////////////////////////
// Prints a set of forms and their inflection features.
int MU_print_forms(MU_forms_T* F) {
	int f;
	for (f = 0; f < F->no_forms; f++)
		MU_print_f(&(F->forms[f]));
	return 0;
}

////////////////////////////////////////////
// Prints a lemma and its info.
int MU_print_lemma(MU_lemma_T* l) {
	u_printf("-----------------\n");
	u_printf("MULTI-WORD LEMMA:\n");
	u_printf("Units:\n");
	int u;
	for (u = 0; u < l->no_units; u++) {
		u_printf("%S", l->units[u]->form);
		if (l->units[u]->lemma) {
			u_printf(":");
			SU_print_lemma(l->units[u]->lemma);
		} else {
			u_printf("\n");
		}
	}
	u_printf("Class: %S\n", l->cl->name);
	u_printf("Paradigm: %s\n", l->paradigm);
	u_printf("-----------------\n");
	return 0;
}

////////////////////////////////////////////
// Delete sample lemma stucture for tests.
void MU_delete_lemma(MU_lemma_T* l) {
	int u;
	if (!l)
		return;
	//Delete the single units
	for (u = 0; u < l->no_units; u++)
		SU_delete_id(l->units[u]);
	//Don't delete the class because it is a global variable usefull all through the treatment
	//Delete the paradigm
	free(l->paradigm);
	//Delete the lemma
	free(l);
}


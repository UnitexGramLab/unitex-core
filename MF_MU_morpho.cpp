/*
  * Unitex 
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
//// Morphology and inflection of a multi-word unit ///
/********************************************************************************/

#include "MF_MU_morpho.h"
#include "MF_Unif.h"
#include "MF_MU_graph.h"

////////////////////////////////////////////
// For a given multi-word unit, generates all the inflected forms,
// e.g. {["mémoire vive",{Gen=fem,Nb=sing}],["mémoires vives",{Gen=fem,Nb=pl}]}
// Initially, 'forms' does not have its space allocated.
// Returns 0 on success, 1 otherwise.   
int MU_inflect(MU_lemma_T* lemma, MU_forms_T* forms) {
  int err;

  //Explore the inflection tranducer and produce the inflected forms
  err = MU_graph_explore_graph(lemma,forms);
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
  for (f=0; f<forms->no_forms; f++) {
    free(forms->forms[f].form);
    f_delete_morpho(forms->forms[f].features);
  }
  free(forms);
}

/*
////////////////////////////////////////////
// Returns the unit's form's identifier on the basis of the form, its lemma, its inflection paradigm, and its inflection features.
// MU_form : form and its inflection features, e.g. ["mémoires vives",{Gen=fem,Nb=pl]
// MU_lemma : lemma and its inflection paradigm, 
//                e.g. [[[word,"mémoires",->[mémoire],2],[sep," "],[word,"vives",->[vif],4]],noun,NC76,{"Conc"},"computing"]
// Returns the pointer to the forms identifier on success (e.g. ->([mémoire vive,noun,NC76,{"Conc"},"computing"],2)), NULL otherwise.
MU_id_T*  MU_get_id(MU_f_T* MU_form, MU_lemma_T* MU_lemma) {
  return NULL;
}
*/

////////////////////////////////////////////
// Liberates the memory allocated for a form's id.
void MU_delete_id(MU_id_T* id){
  if (id) {
    free(id->form);
    MU_delete_lemma(id->MU_lemma);
    f_delete_morpho(id->feat);
  }
  free(id);
}

////////////////////////////////////////////
// Prints a form and its inflection features.
int MU_print_f(MU_f_T* f) {
u_printf("%S : ",f->form);
f_print_morpho(f->features);
return 0;
}

////////////////////////////////////////////
// Prints a set of forms and their inflection features.
int MU_print_forms(MU_forms_T* F) {
  int f;
  for (f=0; f<F->no_forms; f++)
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
  for (u=0; u<l->no_units; u++) {
    u_printf("%S",l->units[u]->form);      
    if (l->units[u]->lemma) {
      u_printf(":");
      SU_print_lemma(l->units[u]->lemma);
    }
    else {
      u_printf("\n");
    }
  }
  u_printf("Class: %S\n",l->cl->name);
  u_printf("Paradigm: %s\n",l->paradigm);
  u_printf("-----------------\n");
  return 0;
}

////////////////////////////////////////////
// Delete sample lemma stucture for tests.
void MU_delete_lemma(MU_lemma_T* l){
  int u;
  if (!l)
    return;
  //Delete the single units
  for (u=0; u<l->no_units; u++)
    SU_delete_id(l->units[u]);
  //Don't delete the class because it is a global variable usefull all through the treatment 
  //Delete the paradigm
  free(l->paradigm);
  //Delete the lemma
  free(l);
}


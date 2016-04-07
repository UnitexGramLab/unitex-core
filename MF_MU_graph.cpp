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

#include "MF_MU_graph.h"
#include "Fst2.h"
#include "MF_Util.h"
#include "MF_SU_morpho.h"
#include "MF_Unif.h"
#include "Error.h"
#include "Transitions.h"
#include "MF_Global.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

Fst2State MU_graph_get_initial(MultiFlex_ctx* p_multiFlex_ctx,char* graph_name);
int MU_graph_explore_state(MultiFlex_ctx* p_multiFlex_ctx, Fst2State q,MU_forms_T* forms);
int MU_graph_explore_label(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_label_T* l, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_scan_label(MultiFlex_ctx* p_multiFlex_ctx, unichar* label_in,
		unichar* label_out, MU_graph_label_T* MU_label);
int MU_graph_explore_label_in_var(MultiFlex_ctx* p_multiFlex_ctx, SU_id_T* u,
		MU_graph_morpho_T* l_in_morpho, MU_graph_morpho_T* l_out_morpho,
		Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_in_var_rec(MultiFlex_ctx* p_multiFlex_ctx, SU_id_T* u,
		MU_graph_morpho_T* l_in_morpho, int i_morpho, f_morpho_T* feat,
		MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_get_unit_forms(MultiFlex_ctx* p_multiFlex_ctx, SU_id_T* u,
		f_morpho_T* feat, SU_forms_T* SU_forms);
int MU_graph_explore_label_in_morph_const(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_category_T* c, SU_id_T* u, MU_graph_morpho_T* l_in_morpho,
		int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho,
		Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_in_morph_inher(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_category_T* c, SU_id_T* u, MU_graph_morpho_T* l_in_morpho,
		int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho,
		Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_in_morph_unif(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_category_T* c, SU_id_T* u, MU_graph_morpho_T* l_in_morpho,
		int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho,
		Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_out(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_out_rec(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_morpho_T* l_out_morpho, int i_morpho, f_morpho_T* feat,
		Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_out_morph_const(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_category_T* c, MU_graph_morpho_T* l_out_morpho, int i_morpho,
		f_morpho_T* feat, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_out_morph_unif(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_category_T* c, MU_graph_morpho_T* l_out_morpho, int i_morpho,
		f_morpho_T* feat, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_scan_label(MultiFlex_ctx* p_multiFlex_ctx,
		unichar* label_in, unichar* label_out,
		MU_graph_label_T* MU_label);
int MU_graph_scan_label_in(MultiFlex_ctx* p_multiFlex_ctx,
		unichar* label,
		MU_graph_in_T* MU_label_in);
int MU_graph_scan_label_out(MultiFlex_ctx* p_multiFlex_ctx,
		unichar* label,
		MU_graph_out_T* MU_label_out);
int MU_graph_scan_graph_morpho(MultiFlex_ctx* p_multiFlex_ctx,
		unichar* label,
		MU_graph_morpho_T* MU_graph_morpho);
void MU_graph_print_label(MU_graph_label_T* MU_label);
void MU_graph_print_morpho(MU_graph_morpho_T* MU_morpho);
void MU_graph_free_label(MU_graph_label_T* MU_label);
void MU_graph_free_morpho(MU_graph_morpho_T* MU_morpho);

/////////////////////////////////////////////////
// Explores the inflection transducer of the MU-lemma 'p_multiFlex_ctx->MU_lemma'
// in order to generate all its inflected forms. The generated forms are put to 'forms'
// Initially, 'forms' has its space allocated but is empty.
// Returns 0 on success, 1 otherwise.
int MU_graph_explore_graph(MultiFlex_ctx* p_multiFlex_ctx,
		MU_lemma_T* MU_l, MU_forms_T* forms) {
	int res;

	//Initialize the current multi-word unit
	p_multiFlex_ctx->MU_lemma = MU_l;

	//Initialize the structure for graph unification variables
	unif_init_vars(&(p_multiFlex_ctx->UNIF_VARS));

	//Get the initial state of the inflection tranducer
	Fst2State initial;
	initial = MU_graph_get_initial(p_multiFlex_ctx,p_multiFlex_ctx->MU_lemma->paradigm);

	if (!initial)
		return 1;

	//Explore the inflection transducer starting from its initial state
	res = MU_graph_explore_state(p_multiFlex_ctx,initial,forms);

	unif_free_vars(&(p_multiFlex_ctx->UNIF_VARS));
	return res;
}

/////////////////////////////////////////////////
// In the graph not yet loaded loads it otherwise searches for it in the structure.
// On success returns the graph's initial state, otherwise returns NULL.
Fst2State MU_graph_get_initial(MultiFlex_ctx* p_multiFlex_ctx,char* graph_name) {
	//Get the index of the tranducer in the transducer table
	p_multiFlex_ctx->T = get_transducer(p_multiFlex_ctx, graph_name);
	if (p_multiFlex_ctx->fst2[p_multiFlex_ctx->T] == NULL) {
		// if the automaton has not been loaded
		return NULL;
	}
	//Get the initial state
	Fst2* a = p_multiFlex_ctx->fst2[p_multiFlex_ctx->T];
	Fst2State q = a->states[0];
	return q;
}

////////////////////////////////////////////
// Given the current instantiation of unification variables and of
// inheritance variables, starting from state q, explore the current state q
// of an inflection transducer for MWUs. Generate suffixes of the inflected
// forms and their features, and put them to 'forms'.
// In case of error put an empty list into 'forms' (which is not identical to ((epsilon,empty_set))).
// Initially, 'forms' has its space allocated but is empty.
// Return a number !=0 in case of errors, 0 otherwise.
int MU_graph_explore_state(MultiFlex_ctx* p_multiFlex_ctx,Fst2State q, MU_forms_T* forms) {
	int err;
	MU_graph_label_T* lab;
	Fst2State q_bis;

	//If we are in a final state, then the empty word is recognized, we add it to 'forms' and we continue to explore
	if (q->control & 1)
		MU_add_empty_form(forms);

	//Explore each outgoing transition
	Transition* t;
	MU_forms_T forms_bis; //Suffixes obtained by the exploration of one outgoing transition

	t = q->transitions;
	while (t) {
		lab = (MU_graph_label_T*) malloc(sizeof(MU_graph_label_T));
		if (!lab) {
			fatal_alloc_error("MU_graph_explore_state");
		}
		q_bis
				= p_multiFlex_ctx->fst2[p_multiFlex_ctx->T]->states[t->state_number]; //get the arrival state
		Fst2Tag e =
				p_multiFlex_ctx->fst2[p_multiFlex_ctx->T]->tags[t->tag_number]; //get the transition's label
		err = MU_graph_scan_label(p_multiFlex_ctx, e->input,
				e->output, lab); //transform the label into a MU_graph_label
		if (err) {
			MU_graph_free_label(lab);
			free(lab);
			return err;
		}
		//Initialize the set of inflected forms
		MU_init_forms(&forms_bis);
		err = MU_graph_explore_label(p_multiFlex_ctx, lab, q_bis, &forms_bis);
		if (err) {
			MU_graph_free_label(lab);
			free(lab);
			MU_delete_inflection(&forms_bis);
			return err;
		}

		//Add each new form to the one generated previously, if it does not exist already
		MU_merge_forms(forms, &forms_bis);

		MU_delete_inflection(&forms_bis);

		//Free the current label
		MU_graph_free_label(lab);
		free(lab);

		//Go to the next transition
		t = t->next;
	}
	return 0;
}

////////////////////////////////////////////
// Given the current instantiation of unification variables and of
// inheritance variables, explore the current transition label 'l',
// and the subautomaton starting from its arrival state 'q_bis'.
// Generate suffixes of the inflected
// forms and their features, and put them to 'forms'.
// In case of error put an empty list into 'forms'.
// Initially, '*forms' has its space allocated but is empty.
// Return a number !=0 in case of errors, 0 otherwise.
int MU_graph_explore_label(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_label_T* l, Fst2State q_bis, MU_forms_T* forms) {
	int err;
	//If the current unit is a reference to a lemma's unit (e.g. <$2>)
	if (l->in && l->in->unit.type != cst) {
		SU_id_T* u; //Referenced lemma's unit
		int u_no; //Number of the referenced unit
		u_no = l->in->unit.u.num - 1;
		u = p_multiFlex_ctx->MU_lemma->units[u_no]; //Get the referenced lemma's unit
		//explore the current unit according to its morphology, then the label's output and the rest of the automaton
		err = MU_graph_explore_label_in_var(p_multiFlex_ctx, u,
				l->in->morpho, l->out, q_bis, forms);
		if (err)
			return err;
	}

	else { //If the current unit is fixed (empty or a fixed word, e.g. "of")

		//////////////////////////////////
		//Get the forms of the current unit
		SU_forms_T SU_forms;
		SU_init_forms(&SU_forms);

		///If the current unit's input is empty (it is an epsilon input <E>)
		if (!l->in)
			//The current form is empty
			SU_init_invariable_form_char(&SU_forms, "");

		//If the current unit is a constant (e.g. "of")
		else
			//Get the form appearing in the node's input
			SU_init_invariable_form(&SU_forms, l->in->unit.u.seq);

		///////////////////////////////////
		//Get the suffixes of the MWU forms
		MU_forms_T suffix_forms;
		MU_init_forms(&suffix_forms);
		err = MU_graph_explore_label_out(p_multiFlex_ctx,
				l->out, q_bis, &suffix_forms); //Explore the rest of the automaton
		if (err) {
			SU_delete_inflection(&SU_forms);
			MU_delete_inflection(&suffix_forms);
			return err;
		}

		/////////////////////////////////////////////////////////////////////////////////////////
		//Concatenate each inflected form of the current unit in front of each multi-unit form
		//resulting from the exploration of the rest of the automaton
		MU_concat_forms(&SU_forms, &suffix_forms, forms);

		//Delete the intermediate simple et compound forms
		SU_delete_inflection(&SU_forms);
		MU_delete_inflection(&suffix_forms);
	}
	return 0;
}

/*For testing
 ////////////////////////////////////////////
 int MU_graph_explore_label(MU_graph_label_T* l,Etat_fst q_bis, MU_forms_T* forms) {
 MU_graph_print_label(l);
 MU_graph_explore_state(q_bis,forms);
 return 0;
 }
 *///End of testing

////////////////////////////////////////////
// Given the current instantiation of unification variables and of
// inheritance variables, recursively explore the input of current transition label,
// knowing that it is a reference to the MU lemma's unit 'u' and its
// desired morphology is 'l_in_morpho'.
// Then explore the label's output 'l_out_morpho'.
// Then explore the subautomaton starting from its arrival state 'q_bis'.
// Generate suffixes of the inflected
// forms and their features, and put them to 'forms'.
// Initially, '*forms' has its space allocated but is empty.
// In case of error put an empty list into 'forms'.
// Return a number !=0 in case of errors, 0 otherwise.
int MU_graph_explore_label_in_var(MultiFlex_ctx* p_multiFlex_ctx,SU_id_T* u,
		MU_graph_morpho_T* l_in_morpho, MU_graph_morpho_T* l_out_morpho,
		Fst2State q_bis, MU_forms_T* forms) {
	int err;
	f_morpho_T feat; //A set of the already treated morphological features contained in the label's input
	feat.no_cats = 0;
	err = MU_graph_explore_label_in_var_rec(p_multiFlex_ctx, u,
			l_in_morpho, 0, &feat, l_out_morpho, q_bis, forms);
	return err;
}

////////////////////////////////////////////
// Recursive exploration of the current transition label's input
// then output, then of the rest of the automaton.
// u: lemma's unit concerned
// l_in_morpho: morphology contained in the label's input, e.g. Nb=pl; Case=$c1; Gen==$g
// i_morpho: index of the first label's input morhological feature not yet treated
// feat: accumulated desired features of the current unit (deduced from this unit's features
// in the MU lemma, and from the label's input).
// l_out_morpho: morphology contained in the label's output
// q_bis: arrival state of the label's transition
// forms: suffixes of the inflected forms and their features
// Initially, '*forms' has its space allocated but is empty.
// In case of error put an empty list into 'forms'.
// Return a number !=0 in case of errors, 0 otherwise.
int MU_graph_explore_label_in_var_rec(MultiFlex_ctx* p_multiFlex_ctx, SU_id_T* u,
		MU_graph_morpho_T* l_in_morpho, int i_morpho, f_morpho_T* feat,
		MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms) {
	int err = 0;

	/////////////////////////////////////////////////////////////////
	//If the whole label's input not yet treated continue treating it
	if (l_in_morpho && i_morpho != l_in_morpho->no_cats) {
		MU_graph_category_T* c; // a single graph category-value pair, e.g. Case==$n1
		c = &(l_in_morpho->cats[i_morpho]);
		switch (c->type) {
		case cnst: //e.g. Nb=pl
			err = MU_graph_explore_label_in_morph_const(p_multiFlex_ctx,
					c, u, l_in_morpho, i_morpho + 1, feat,
					l_out_morpho, q_bis, forms);
			break;
		case inherit_var: //e.g. Gen==$g
			err = MU_graph_explore_label_in_morph_inher(p_multiFlex_ctx,
					c, u, l_in_morpho, i_morpho + 1, feat,
					l_out_morpho, q_bis, forms);
			break;
		case unif_var: //e.g. Case=$c1
			err = MU_graph_explore_label_in_morph_unif(p_multiFlex_ctx,
					c, u, l_in_morpho, i_morpho + 1, feat,
					l_out_morpho, q_bis, forms);
			break;
		}
		if (err)
			return err;
	}

	////////////////////////////////////////////////////////////////////////////////
	//If the whole label's input treated get the inflected forms of the current unit
	else {

		//Get the inflected forms of the current unit
		SU_forms_T SU_forms;
		SU_init_forms(&SU_forms);

		//If no morphology in the input label, take the unit as it appears in the MWU's lemma
		if (!l_in_morpho)
			//Get the inflected form from the current unit (this form is not to be modified since there is no morphology in the input label)
			SU_init_invariable_form(&SU_forms, u->form);

		//If all morphological category-value equations in the input label treated
		else {
			//Inflect the unit concerned according to desired features
			err = MU_graph_get_unit_forms(p_multiFlex_ctx, u,feat, &SU_forms);
			if (err) {
				SU_delete_inflection(&SU_forms);
				return err;
			}
		}

		///////////////////////////////////
		//Get the suffixes of the MWU forms
		MU_forms_T suffix_forms;
		MU_init_forms(&suffix_forms);
		err = MU_graph_explore_label_out(p_multiFlex_ctx,
				l_out_morpho, q_bis, &suffix_forms); //Explore the rest of the automaton
		if (err) {
			SU_delete_inflection(&SU_forms);
			MU_delete_inflection(&suffix_forms);
			return err;
		}

		/////////////////////////////////////////////////////////////////////////////////////////
		//Concatenante each inflected form of the current unit in front of each multi-unit form
		//resulting from the exploration of the rest of the automaton
		MU_concat_forms(&SU_forms, &suffix_forms, forms);

		//Delete the intermediate simple et compound forms
		SU_delete_inflection(&SU_forms);
		MU_delete_inflection(&suffix_forms);
	}
	return 0;
}

/////////////////////////////////////////////////
// Generate the desired inflected forms (defined by 'feat')
// of the current single unit 'u'.
// Return the generated forms in 'SU_forms'.
// In case of error put an empty list into 'SU_forms'.
// Initially, 'SU_forms' has its space allocated but is empty.
// Return a number !=0 in case of errors, 0 otherwise.
int MU_graph_get_unit_forms(MultiFlex_ctx* p_multiFlex_ctx, SU_id_T* u,
		f_morpho_T* feat, SU_forms_T* SU_forms) {
	int err;
	f_morpho_T old_feat; //Features that the current unit has in the lemma of the MWU

	//Get the features that the unit has in the lemma of the MWU, e.g. <Nb=sing; Gen=fem> for "vif" in "memoire vive"
	//SU_cpy_features(&old_feat,u);
	err = f_copy_morpho(&old_feat, u->feat);
	if (err)
		return err;

	//Change the features to adapt them to the desired features, e.g. if 'feat'=<Nb=pl> then old_feat<-<Nb=pl; Gen=fem>
	err = f_change_morpho(p_multiFlex_ctx->pL_MORPHO, &old_feat, feat);
	if (err)
		return err;

	//Generate the desired inflected forms of the single unit
	err = SU_inflect(p_multiFlex_ctx, u, &old_feat, SU_forms);
	return err;
}

////////////////////////////////////////////
// Recursive exploration of the current transition label's input
// then output, then of the rest of the automaton.
// c: single category-value pair from the current label, it has a CONSTANT value e.g. Nb=pl
// u: lemma's unit concerned, e.g. vive
// l_in_morpho: morphology contained in the label's input, e.g. Nb=pl; Case=$c1; Gen==$g
// i_morpho: index of the first label's input morhological feature not yet treated
// feat: accumulated desired features of the current unit (deduced from this unit's features
// in the MU lemma, and from the label's input).
// l_out_morpho: morphology contained in the label's output
// q_bis: arrival state of the label's transition
// forms: suffixes of the inflected forms and their features
// Initially, '*forms' has its space allocated but is empty.
// In case of error put an empty list into 'forms'.
// Return a number !=0 in case of errors, 0 otherwise.
int MU_graph_explore_label_in_morph_const(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_category_T* c, SU_id_T* u, MU_graph_morpho_T* l_in_morpho,
		int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho,
		Fst2State q_bis, MU_forms_T* forms) {
	int err;

	//Add the current label's category-value pair to the features of the single unit to be generated
	err = f_add_morpho(feat, c->cat, c->val.value);
	if (err == -1) {
		MU_delete_inflection(forms);
		return 1;
	}

	//Explore recursively the rest of the label
	return MU_graph_explore_label_in_var_rec(p_multiFlex_ctx,
			u, l_in_morpho, i_morpho, feat, l_out_morpho, q_bis, forms);
}

////////////////////////////////////////////
// Recursive exploration of the current transition label's input
// then output, then of the rest of the automaton.
// c: single category-value pair from the current label, it is an INHERITANCE variable e.g. Gen==$gl
// u: lemma's unit concerned, e.g. vive
// l_in_morpho: morphology contained in the label's input, e.g. Nb=pl; Case=$c1; Gen==$g
// i_morpho: index of the first label's input morhological feature not yet treated
// feat: accumulated desired features of the current unit (deduced from this unit's features
// in the MU lemma, and from the label's input), e.g. <Nb=pl; Case=Nom>
// l_out_morpho: morphology contained in the label's output
// q_bis: arrival state of the label's transition
// forms: suffixes of the inflected forms and their features
// Initially, '*forms' has its space allocated but is empty.
// In case of error put an empty list into 'forms'.
// Return a number !=0 in case of errors, 0 otherwise.
int MU_graph_explore_label_in_morph_inher(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_category_T* c, SU_id_T* u, MU_graph_morpho_T* l_in_morpho,
		int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho,
		Fst2State q_bis, MU_forms_T* forms) {
	int err;
	int new_instant = 0; //Controls if an instantiation occured in the current instance of the function

	//Get the features that the unit has in the lemma of the MWU, e.g. <Nb=sing; Gen=fem; Case=Nom>
	f_morpho_T old_feat; //Features that the current unit has in the lemma of the MWU
	//err = SU_cpy_features(&old_feat,u);    //e.g. <Nb=sing; Case=$c1; Gen==$g>
	err = f_copy_morpho(&old_feat, u->feat);
	if (err)
		return err;

	//Get the value of the current category (determined by 'c') of the unit as it appears in the lemma of the MWU
	int val; //Value of the current category (index that this value has in the domain os the category)
	val = f_get_value(p_multiFlex_ctx->pL_MORPHO, &old_feat, c->cat); //e.g. val: fem

	if (val == -1)
		return -1;

	//Instantiate the variable if necessary
	unichar* var;
	var = c->val.inherit_var; //get the identifier of the variable, e.g. g1
	if (unif_instantiated(&(p_multiFlex_ctx->UNIF_VARS), var)) {
		//If the same variable already instantiated to a DIFFERENT value then cut off the exploration path
		//The 'forms' remain empty list as they were (which is not equivalent to a list containing (epsilon,empty_set)
		if ((unif_get_cat(&(p_multiFlex_ctx->UNIF_VARS), var) != c->cat)
				|| (unif_get_val_index(&(p_multiFlex_ctx->UNIF_VARS), var) != val))
			return 0;
		//If variable already instantiated to the same value, no further instantiation needed
	} else { //Variable not yet instantiated
		err = unif_instantiate_index(&(p_multiFlex_ctx->UNIF_VARS), var, c->cat, val);
		if (err)
			return err;
		new_instant = 1;
	}

	//Add the the instantiated category-value pair to the features of the single unit to be generated
	err = f_add_morpho(feat, c->cat, val);
	if (err)
		return err;

	//Explore recursively the rest of the label
	err = MU_graph_explore_label_in_var_rec(p_multiFlex_ctx, u,
			l_in_morpho, i_morpho, feat, l_out_morpho, q_bis, forms);
	if (err)
		return err;

	//Desinstantiate the variable only if it has been instantiated by the current category-value pair 'c'
	if (new_instant)
		unif_desinstantiate(&(p_multiFlex_ctx->UNIF_VARS), var);

	err = f_del_one_morpho(feat, c->cat);
	return err;
}

////////////////////////////////////////////
// Recursive exploration of the current transition label's input
// then output, then of the rest of the automaton.
// c: single category-value pair from the current label, it is a UNIFICATION variable e.g. Case=$c2
// u: lemma's unit concerned, e.g. vive
// l_in_morpho: morphology contained in the label's input, e.g. Nb=pl; Case=$c1; Gen==$g
// i_morpho: index of the first label's input morhological feature not yet treated
// feat: accumulated desired features of the current unit (deduced from this unit's features
// in the MU lemma, and from the label's input), e.g. <Nb=pl; Case=Nom>
// l_out_morpho: morphology contained in the label's output
// q_bis: arrival state of the label's transition
// forms: suffixes of the inflected forms and their features
// Initially, '*forms' has its space allocated but is empty.
// In case of error put an empty list into 'forms'.
// Return a number !=0 in case of errors, 0 otherwise.
int MU_graph_explore_label_in_morph_unif(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_category_T* c, SU_id_T* u, MU_graph_morpho_T* l_in_morpho,
		int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho,
		Fst2State q_bis, MU_forms_T* forms) {
	int err; //Result of a function called

	unichar* var; //Unification variable's identifier
	var = c->val.unif_var;

	//Get the features that the unit has in the lemma of the MWU, e.g. <Nb=sing; Gen=fem; Case=Nom>
	//  f_morpho_T old_feat;  //Features that the current unit has in the lemma of the MWU
	//  err = SU_cpy_features(&old_feat,u);    //e.g. <Nb=sing; Case=$c1; Gen==$g>
	//  if (err)
	//    return err;

	if (unif_instantiated(&(p_multiFlex_ctx->UNIF_VARS), var)) {
		//If the same variable already instantiated for a DIFFERENT category then cut off the exploration path
		//The 'forms' remain empty list as they were (which is not equivalent to a list containing (epsilon,empty_set)
		if (unif_get_cat(&(p_multiFlex_ctx->UNIF_VARS), var) != c->cat)
			return 0;

		//If the same variable already instantiated for the same category, only this instantiation is taken into account
		//Add the instantiated category-value pair to the features of the single unit to be generated
		err = f_add_morpho(feat, c->cat, unif_get_val_index(&(p_multiFlex_ctx->UNIF_VARS),
				var));
		if (err == -1) {
			MU_delete_inflection(forms);
			return 1;
		}

		//Return MU_graph_explore_label_in_var_rec(u,l_in_morpho,i_morpho,feat,l_out_morpho,q_bis,forms);
		err = MU_graph_explore_label_in_var_rec(p_multiFlex_ctx,
				u, l_in_morpho, i_morpho, feat, l_out_morpho, q_bis,forms);
		if (err == -1) {
			MU_delete_inflection(forms);
			return 1;
		}
		err = f_del_one_morpho(feat, c->cat);
		return err;
	}

	else {//If the variable not yet instantiated
		int val; //Index of different values in the domain of the current category
		MU_forms_T suffix_forms; //Suffixes generated in one run of the for-loop
		for (val = 0; val < c->cat->no_values; val++) {

			//Instantiated to the current value
			unif_instantiate_index(&(p_multiFlex_ctx->UNIF_VARS), var, c->cat, val);

			//Add the the instantiated category-value pair to the features of the single unit to be generated
			err = f_add_morpho(feat, c->cat, val);
			if (err) {
				MU_delete_inflection(&suffix_forms);
				return err;
			}

			//Explore the rest of the label and the rest of the automaton
			MU_init_forms(&suffix_forms);
			err = MU_graph_explore_label_in_var_rec(p_multiFlex_ctx, u, l_in_morpho,
					i_morpho, feat, l_out_morpho, q_bis, &suffix_forms);
			if (err) {
				//Delete the intermediate simple et compound forms
				MU_delete_inflection(&suffix_forms);
				return err;
			}

			//Add each suffix obtained to the list of previously obtained suffixes
			MU_merge_forms(forms, &suffix_forms);

			//Delete the current category-value pair
			//feat->no_cats--;
			err = f_del_one_morpho(feat, c->cat);
			if (err) {
				//Delete the intermediate simple et compound forms
				MU_delete_inflection(&suffix_forms);
				return err;
			}

			//Delete the current instantiation
			unif_desinstantiate(&(p_multiFlex_ctx->UNIF_VARS), var);

			//Delete the intermediate simple et compound forms
			MU_delete_inflection(&suffix_forms);
		}
	}
	return 0;
}

////////////////////////////////////////////
// Given the current instantiation of unification variables and of
// inheritance variables, explore the current transition label's output 'l_out_morpho',
// and the subautomaton starting from its arrival state 'q_bis'.
// Generate suffixes of the inflected
// forms and their features, and put them to 'forms'.
// In case of error put an empty list into 'forms'.
// Initially, '*forms' has its space allocated but is empty.
// Return a number !=0 in case of errors, 0 otherwise.
int MU_graph_explore_label_out(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_morpho_T* l_out_morpho,
		Fst2State q_bis, MU_forms_T* forms) {
	int err;
	f_morpho_T feat; //A set of the already treated morphological features contained in the label's input
	f_init_morpho(&feat);
	//feat.no_cats = 0;
	err = MU_graph_explore_label_out_rec(p_multiFlex_ctx,
			l_out_morpho, 0, &feat, q_bis, forms);
	return (err);
}

////////////////////////////////////////////
// Recursive exploration of the current transition label's output,
// then of the rest of the automaton.
// l_out_morpho: morphology contained in the label's output, e.g. Nb=pl; Case=$c1; Gen=$g
// i_morpho: index of the first label's output morhological feature not yet treated
// feat: accumulated output features of the MWU
// q_bis: arrival state of the label's transition
// forms: suffixes of the inflected forms and their features
// Initially, '*forms' has its space allocated but is empty.
// In case of error put an empty list into 'forms'.
// Return a number !=0 in case of errors, 0 otherwise.
int MU_graph_explore_label_out_rec(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_morpho_T* l_out_morpho,
		int i_morpho, f_morpho_T* feat, Fst2State q_bis, MU_forms_T* forms) {
	int err = 0;

	//If no morphology in the output label, then go to the next state
	if (!l_out_morpho) {
		//Explore the arrival state
		err = MU_graph_explore_state(p_multiFlex_ctx,q_bis, forms);
		if (err)
			return err;
	} else //Label's input not empty

	//If the whole output morphology has been treated
	if (i_morpho == l_out_morpho->no_cats) {

		//Explore the arrival state
		err = MU_graph_explore_state(p_multiFlex_ctx, q_bis, forms);
		if (err)
			return err;

		//Add the current features to the features of the suffixes
		int f; //Index of the current suffix
		int c; //Index of the current category-value pair in 'feat'
		for (f = 0; f < forms->no_forms; f++)
			for (c = 0; c < feat->no_cats; c++) {
				//If the ouput morphology in a graph is ambiguous, the final MWU's morphology is undefined
				err = f_add_morpho(forms->forms[f].features, feat->cats[c].cat,
						feat->cats[c].val);
				if (err == -1) {
					MU_delete_inflection(forms);
					return 1;
				}
			}
	}

	//If the whole label's output not yet treated
	else {
		MU_graph_category_T* c; // a single graph category-value pair, e.g. Case==$n1
		c = &(l_out_morpho->cats[i_morpho]);
		switch (c->type) {
		case cnst: //e.g. Nb=pl
			err = MU_graph_explore_label_out_morph_const(p_multiFlex_ctx,
					c, l_out_morpho,i_morpho + 1, feat, q_bis, forms);
			break;
		case unif_var: //e.g. Case=$c1
			err = MU_graph_explore_label_out_morph_unif(p_multiFlex_ctx,
					c, l_out_morpho,i_morpho + 1, feat, q_bis, forms);
			break;
		default:
			;
		}
		if (err)
			return err;
	}
	return 0;
}

////////////////////////////////////////////
// Recursive exploration of the current transition label's output,
// then of the rest of the automaton.
// c: single category-value pair from the current label, it has a CONSTANT value e.g. Nb=pl
// l_out_morpho: morphology contained in the label's output
// i_morpho: index of the first label's output morhological feature not yet treated
// feat: accumulated output features of the MWU
// q_bis: arrival state of the label's transition
// forms: suffixes of the inflected forms and their features
// Initially, '*forms' has its space allocated but is empty.
// In case of error put an empty list into 'forms'.
// Return a number !=0 in case of errors, 0 otherwise.
int MU_graph_explore_label_out_morph_const(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_category_T* c,
		MU_graph_morpho_T* l_out_morpho, int i_morpho, f_morpho_T* feat,
		Fst2State q_bis, MU_forms_T* forms) {
	int err;
	//Add the current label's category-value pair to the features of the single unit to be generated
	err = f_add_morpho(feat, c->cat, c->val.value);
	if (err == -1) {
		MU_delete_inflection(forms);
		return 1;
	}
	//Explore recursively the rest of the label
	return MU_graph_explore_label_out_rec(p_multiFlex_ctx,
			l_out_morpho, i_morpho, feat, q_bis, forms);
}

////////////////////////////////////////////
// Recursive exploration of the current transition label's output,
// then of the rest of the automaton.
// c: single category-value pair from the current label, it is a UNIFICATION variable e.g. Case=$c2
// l_out_morpho: morphology contained in the label's output, e.g. Nb=pl; Case=$c1; Gen=$g
// i_morpho: index of the first label's output morhological feature not yet treated
// feat: accumulated output features of the MWU
// q_bis: arrival state of the label's transition
// forms: suffixes of the inflected forms and their features
// Initially, '*forms' has its space allocated but is empty.
// In case of error put an empty list into 'forms'.
// Return a number !=0 in case of errors, 0 otherwise.
int MU_graph_explore_label_out_morph_unif(MultiFlex_ctx* p_multiFlex_ctx,
		MU_graph_category_T* c,
		MU_graph_morpho_T* l_out_morpho, int i_morpho, f_morpho_T* feat,
		Fst2State q_bis, MU_forms_T* forms) {
	int err; //Result of a function called

	unichar* var; //Unification variable's identifier
	var = c->val.unif_var;

	if (unif_instantiated(&(p_multiFlex_ctx->UNIF_VARS), var)) {
		//If the same variable already instantiated for a DIFFERENT category then cut off the exploration path
		//The 'forms' remain empty list as they were (which is not equivalent to a list containing (epsilon,empty_set)
		if (unif_get_cat(&(p_multiFlex_ctx->UNIF_VARS), var) != c->cat)
			return 0;

		//If the same variable already instantiated for the same category, only this instantiation is taken into account
		//Add the the instantiated category-value pair to the features of the single unit to be generated
		err = f_add_morpho(feat, c->cat, unif_get_val_index(&(p_multiFlex_ctx->UNIF_VARS),
				var));
		if (err == -1) {
			MU_delete_inflection(forms);
			return 1;
		}
		err = MU_graph_explore_label_out_rec(p_multiFlex_ctx,
				l_out_morpho, i_morpho, feat, q_bis, forms);

		//Delete the current category-value pair
		f_del_one_morpho(feat, c->cat);
		//feat->no_cats--;

		//Pass on the error
		return err;

	}

	else {//If the variable not yet instantiated
		int val; //Index of different values in the domain of the current category
		MU_forms_T suffix_forms; //Suffixes generated in one run of the for-loop
		for (val = 0; val < c->cat->no_values; val++) {

			//Instantiated to the current value
			unif_instantiate_index(&(p_multiFlex_ctx->UNIF_VARS), var, c->cat, val);

			//Add the the instantiated category-value pair to the features of the single unit to be generated
			err = f_add_morpho(feat, c->cat, val); //e.g. 'feat' devient <Nb=pl; Case=Nom; Gen=fem>
			if (err == -1) {
				MU_delete_inflection(forms);
				return 1;
			}

			//Explore the rest of the label and the rest of the automaton
			MU_init_forms(&suffix_forms);
			err = MU_graph_explore_label_out_rec(p_multiFlex_ctx,
					l_out_morpho, i_morpho, feat, q_bis, &suffix_forms);
			if (err) {
				MU_delete_inflection(&suffix_forms);
				return err;
			}
			//If any suffix found, add it to the list of those generated previously
			MU_merge_forms(forms, &suffix_forms);

			//Delete the current category-value pair
			f_del_one_morpho(feat, c->cat);

			//Delete the current instantiation
			unif_desinstantiate(&(p_multiFlex_ctx->UNIF_VARS), var);
		}
	}
	return 0;
}

/////////////////////////////////////////////////
// Creates a MU_graph label from two strings.
// We suppose that MU_label already has its memory allocated.
// Returns 0 on success, 1 otherwise.
int MU_graph_scan_label(MultiFlex_ctx* p_multiFlex_ctx,
		unichar* label_in, unichar* label_out,
		MU_graph_label_T* MU_label) {
	int err1, err2;
	err1 = err2 = 0;

	//Input label
	/*
	 printf("label_in = ");  //debug
	 u_prints(label_in);//debug
	 printf("\n");//debug
	 */
	if (!u_strcmp(label_in, "<E>")) //Epsilon case
		MU_label->in = NULL;
	else {
		MU_label->in = (MU_graph_in_T*) malloc(sizeof(MU_graph_in_T));
		if (MU_label->in == NULL) {
			fatal_alloc_error("MU_graph_scan_label");
		}
		err1 = MU_graph_scan_label_in(p_multiFlex_ctx, label_in,
				MU_label->in);
	}
	//Output label
	/*
	 printf("label_out = ");  //debug
	 u_prints(label_out);//debug
	 printf("\n");//debug
	 */
	if (label_out == NULL || (!u_strcmp(label_out, "<E>")) || (!u_strlen(
			label_out))) //Case of epsilon or void output
		MU_label->out = NULL;
	else {
		MU_label->out = (MU_graph_out_T*) malloc(sizeof(MU_graph_out_T));
		if (MU_label->out == NULL) {
			fatal_alloc_error("MU_graph_scan_label");
		}
		err2 = MU_graph_scan_label_out(p_multiFlex_ctx,label_out,
				MU_label->out);
	}
	return (err1 || err2);
}

/////////////////////////////////////////////////
// Creates a MU_graph label's input from a string (e.g. "of" or "<$1[Gen==$g;Nb=$n;Case=Inst]>"
// We suppose that MU_label_in already has its memory allocated.
// Returns 0 on success, 1 otherwise.
int MU_graph_scan_label_in(MultiFlex_ctx* p_multiFlex_ctx,
		unichar* label,
		MU_graph_in_T* MU_label_in) {
	int l; //length of a scanned sequence
	unichar* pos; //current position in label
	unichar tmp[MAX_GRAPH_NODE];
	int err = 0;

	pos = label;

	//Omit void characters
	pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE - 1, " \t");

	//Constant unit, e.g. "of"
	if (*pos != (unichar) '<') {
		l = u_scan_until_char(tmp, pos, MAX_GRAPH_NODE - 1, "", 1);
		MU_label_in->unit.type = cst;
		MU_label_in->unit.u.seq = u_strdup(tmp);
		MU_label_in->morpho = NULL;
		pos = pos + l;
	} else { //Variable unit, e.g. <$2:Gen==$g>
		pos++; //Omit the '<'
		MU_label_in->unit.type = var;
		if (*pos != (unichar) '$') {
			error("In graph %s label format incorrect in %S",
					p_multiFlex_ctx->MU_lemma->paradigm, label);
			error(" (at position %d): ", (int) (pos - label));
			error(" a '$' missing after '<'.\n");
			return 1;
		}
		pos++; //Omit the '$'
		l = u_scan_while_char(tmp, pos, MAX_GRAPH_NODE - 1, "0123456789");
		if (!l) {
			error("In graph %s label format incorrect in %S",
					p_multiFlex_ctx->MU_lemma->paradigm, label);
			error(" (at position %d): ", (int) (pos - label));
			error(" unit number missing after \'$\'.\n");
			return 1;
		}
		char tmp_char[MAX_GRAPH_NODE];
		u_to_char(tmp_char, tmp);
		MU_label_in->unit.u.num = atoi(tmp_char);
		pos = pos + l;

		//Omit void characters
		pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE - 1, " \t");

		//A ':' or a '>' must follow
		if ((*pos != (unichar) ':') && (*pos != (unichar) '>')) {
			error("In graph %s label format incorrect in ",
					p_multiFlex_ctx->MU_lemma->paradigm);
			error("Graph label format incorrect in %S", label);
			error(" (at position %d): ", (int) (pos - label));
			error("':' or '>' missing.\n");
			return 1;
		}

		//Check if morphology follows
		if (*pos == (unichar) ':') {
			pos++;
			MU_label_in->morpho = (MU_graph_morpho_T*) malloc(
					sizeof(MU_graph_morpho_T));
			if (MU_label_in->morpho == NULL) {
				fatal_alloc_error("MU_graph_scan_label_in");
			}
			unichar tmp1[MAX_GRAPH_NODE];
			l = u_scan_until_char(tmp1, pos, MAX_GRAPH_NODE - 1, ">", 1);
			err = MU_graph_scan_graph_morpho(p_multiFlex_ctx,tmp1,
					MU_label_in->morpho);
			pos = pos + l;
		} else
			MU_label_in->morpho = NULL;
		//Closing '>'
		if (*pos != (unichar) '>') {
			error("In graph %s label format incorrect in %S",
					p_multiFlex_ctx->MU_lemma->paradigm, label);
			error(" (at position %d): ", (int) (pos - label));
			error("'>' missing.\n");
			return 1;
		}
		pos++;
	}

	//Omit void characters
	pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE - 1, " \t");

	if (*pos != 0) {
		error("In graph %s label format incorrect in %S",
				p_multiFlex_ctx->MU_lemma->paradigm, label);
		error(" (at position %d): ", (int) (pos - label));
		error(" end of label expected.\n");
		return 1;
	}
	return err;
}

/////////////////////////////////////////////////
// Creates a MU_graph label's output structure from a string.
// We suppose that MU_label_out already has its memory allocated.
// Returns 0 on success, 1 otherwise.
int MU_graph_scan_label_out(MultiFlex_ctx* p_multiFlex_ctx,
		unichar* label,
		MU_graph_out_T* MU_label_out) {
	int cv; //number of the current category-value pair
	int err;
	int l; //length of a scanned sequence
	unichar* pos; //current position in label

	pos = label;

	//Opening '<'
	if (*pos != (unichar) '<') {
		error("In graph %s label format incorrect in %S",
				p_multiFlex_ctx->MU_lemma->paradigm, label);
		error(" (at position %d): ", (int) (pos - label));
		error(" '<'  expected.\n");
		return 1;
	}
	pos++;
	unichar tmp[MAX_GRAPH_NODE];
	l = u_scan_until_char(tmp, pos, MAX_GRAPH_NODE - 1, ">", 1);
	err = MU_graph_scan_graph_morpho(p_multiFlex_ctx,tmp,
			MU_label_out);
	pos = pos + l;
	if (!err)
		for (cv = 0; cv < MU_label_out->no_cats; cv++)
			if (MU_label_out->cats[cv].type == inherit_var) {
				error("In graph %s label format incorrect in %S",
						p_multiFlex_ctx->MU_lemma->paradigm, label);
				error(
						": an output label may not contain a double assignment \'==\'.\n");
				return 1;
			}
	//Closing '>'
	if (*pos != (unichar) '>') {
		error("In graph %s label format incorrect in %S",
				p_multiFlex_ctx->MU_lemma->paradigm, label);
		error(" (at position %d): ", (int) (pos - label));
		error(" '>'  expected.\n");
		return 1;
	}
	return 0;
}

/////////////////////////////////////////////////
// Creates a MU_graph morpho structure from a string.
// We suppose that MU_graph_morpho already has its memory allocated.
// Returns 0 on success, 1 otherwise.
int MU_graph_scan_graph_morpho(MultiFlex_ctx* p_multiFlex_ctx,
		unichar* label,
		MU_graph_morpho_T* MU_graph_morpho) {
	int l; //length of a scanned sequence
	unichar* pos; //current position in label
	unichar tmp[MAX_GRAPH_NODE];
	int cv; //number of the current category-value pair
	l_category_T* cat;
	int val; //Index of a value in the domain of a category.
	int dbl_eq; //Checks if a double equal sign has appeared

	pos = label;

	//Omit void characters
	pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE - 1, " \t");

	//Category-value pairs
	int done = 0;
	cv = 0;
	while (!done) {
		//Omit void characters
		pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE - 1, " \t");

		//Category, e.g. Nb
		l = u_scan_until_char(tmp, pos, MAX_GRAPH_NODE - 1, " \t=", 1);
		cat = is_valid_cat(p_multiFlex_ctx->pL_MORPHO, tmp);
		if (!cat) {
			error("In graph %s label format incorrect in %S",
					p_multiFlex_ctx->MU_lemma->paradigm, label);
			error(" (at position %d): %S", (int) (pos - label), tmp);
			error(" is not a valid category\n");
			return 1;
		}
		MU_graph_morpho->cats[cv].cat = cat;
		pos = pos + l;

		//Omit void characters
		pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE - 1, " \t");

		//The '=' character
		if (*pos != (unichar) '=') {
			error("In graph %s label format incorrect in %S",
					p_multiFlex_ctx->MU_lemma->paradigm, label);
			error(" (at position %d): ", (int) (pos - label));
			error("\'=\' missing.\n");
			return 1;
		}
		pos++;

		//The double '==' if any
		if (*pos == (unichar) '=') {
			dbl_eq = 1;
			pos++;
		} else
			dbl_eq = 0;

		//Omit void characters
		pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE - 1, " \t");

		//Value e.g. gen, $n1
		//Variable
		if (*pos == (unichar) '$') {
			if (dbl_eq)
				MU_graph_morpho->cats[cv].type = inherit_var;
			else
				MU_graph_morpho->cats[cv].type = unif_var;
			pos++; //omit the '$'
			l = u_scan_until_char(tmp, pos, MAX_GRAPH_NODE - 1, " \t;>", 1);
			if (!l) {
				error("In graph %s label format incorrect in %S",
						p_multiFlex_ctx->MU_lemma->paradigm, label);
				error(" (at position %d): ", (int) (pos - label));
				error("a variable missing after \'$\'.\n");
				return 1;
			}
			MU_graph_morpho->cats[cv].val.unif_var = u_strdup(tmp);
		} else { //constant value, e.g. fem
			if (dbl_eq) {
				error("In graph %s label format incorrect in %S",
						p_multiFlex_ctx->MU_lemma->paradigm, label);
				error(" (at position %d): ", (int) (pos - label));
				error("a variable missing after \'==\'.\n");
				return 1;
			}
			MU_graph_morpho->cats[cv].type = cnst;
			l = u_scan_until_char(tmp, pos, MAX_GRAPH_NODE - 1, " \t;>", 1);
			if (!l) {
				error("In graph %s label format incorrect in %S",
						p_multiFlex_ctx->MU_lemma->paradigm, label);
				error(" (at position %d): %S", (int) (pos - label), tmp);
				error("a value missing after \'=\'.\n");
				return 1;
			}
			val = is_valid_val(cat, tmp);
			if (val == -1) {
				error("In graph %s label format incorrect in %S",
						p_multiFlex_ctx->MU_lemma->paradigm, label);
				error(" (at position %d): %S", (int) (pos - label), tmp);
				error(" is not a valid value in the domain of %S\n", cat->name);
				return 1;
			}
			MU_graph_morpho->cats[cv].val.value = val;
		}
		pos = pos + l; //go to the end of the current value

		//New category-value pair read in
		cv++;

		//Omit void characters
		pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE - 1, " \t");

		//See if end of category-value pairs
		if ((*pos == (unichar) '>') || (*pos == (unichar) '\0'))
			done = 1;
		else {
			if (*pos != (unichar) ';') {
				error("In graph %s label format incorrect in %S",
						p_multiFlex_ctx->MU_lemma->paradigm, label);
				error(" (at position %d): %S", (int) (pos - label), tmp);
				error(" ';' missing\n");
				return 1;
			}
			pos++; //Omit the ';'
		}
	}
	MU_graph_morpho->no_cats = cv;

	//Omit void characters
	pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE - 1, " \t");

	return 0;
}

/////////////////////////////////////////////////
// Prints a MU_graph label.
void MU_graph_print_label(MU_graph_label_T* MU_label) {
	//Label's input
	if (!MU_label->in) { //Epsilon case
		u_printf("<E>");
	} else {
		if (MU_label->in->unit.type == cst)
			u_printf("%S", MU_label->in->unit.u.seq);
		else {
			u_printf("<$%d", MU_label->in->unit.u.num);
			if (MU_label->in->morpho) {
				//Opening ':'
				u_printf(":");
				MU_graph_print_morpho(MU_label->in->morpho);
			}
			u_printf(">");
		}
	}
	//Separating '/'
	u_printf("/");
	//Label's output
	if (!MU_label->out) { //Epsilon case
		u_printf("<E>");
	} else {
		u_printf("<");
		MU_graph_print_morpho(MU_label->out);
		u_printf(">");
	}
	//Newline
	u_printf("\n");
}

/////////////////////////////////////////////////
// Prints a MU_graph morpho.
void MU_graph_print_morpho(MU_graph_morpho_T* MU_morpho) {
	int cv; //number of the current category-value pair
	//Category-value features
	for (cv = 0; cv < MU_morpho->no_cats; cv++) {
		//Category
		u_printf("%S", MU_morpho->cats[cv].cat->name);
		//Equality
		u_printf("=");
		if (MU_morpho->cats[cv].type == inherit_var) {
			u_printf("="); //Double '=='
		}
		//Value
		if (MU_morpho->cats[cv].type == inherit_var) { //inherit_var
			u_printf("$%S", MU_morpho->cats[cv].val.inherit_var);
		} else if (MU_morpho->cats[cv].type == unif_var) { //unif_var
			u_printf("$%S", MU_morpho->cats[cv].val.unif_var);
		} else { //constant
			int val = MU_morpho->cats[cv].val.value;
			u_printf("%S", MU_morpho->cats[cv].cat->values[val]);
		}
		//Semi-colon
		if (cv < MU_morpho->no_cats - 1) {
			u_printf(";");
		}
	}
}
/////////////////////////////////////////////////
// Frees the memory allocated for a MU_graph label.
void MU_graph_free_label(MU_graph_label_T* MU_label) {
	//Free the label's input
	if (MU_label->in) {
		if (MU_label->in->unit.type == cst)
			free(MU_label->in->unit.u.seq);
		if (MU_label->in->morpho) {
			MU_graph_free_morpho(MU_label->in->morpho);
			free(MU_label->in->morpho);
		}
		free(MU_label->in);
	}
	//Free the label's output
	if (MU_label->out) {
		MU_graph_free_morpho(MU_label->out);
		free(MU_label->out);
	}
}

/////////////////////////////////////////////////
// Liberates the memory allocated for a MU_graph morpho.
void MU_graph_free_morpho(MU_graph_morpho_T* MU_morpho) {
	int cv; //number of the current category-value pair
	for (cv = 0; cv < MU_morpho->no_cats; cv++)
		if (MU_morpho->cats[cv].type == unif_var)
			free(MU_morpho->cats[cv].val.unif_var);
		else if (MU_morpho->cats[cv].type == inherit_var)
			free(MU_morpho->cats[cv].val.inherit_var);
}

} // namespace unitex

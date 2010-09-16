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

#include <string.h>
#include "MF_LangMorpho.h"
#include "MF_FormMorpho.h"
#include "MF_SU_morpho.h"
#include "MF_InflectTransd.h"
#include "MF_DicoMorpho.h"
#include "MF_Util.h"
#include "Error.h"
#include "List_ustring.h"
#include "StringParsing.h"
#include "Transitions.h"
#include "MF_Operators_Util.h"
#include "MF_Global.h"
#include "Arabic.h"


#define MAX_CHARS_IN_STACK 4096

//////////////////////////////
//Description of all classes in the current language
//extern l_classes_T L_CLASSES;

//////////////////////////////
// Table of inflection tranducers
//extern Fst2* fst2[N_FST2];

/**
 * This structure represents a list of inflection information. It is used
 * to get up information from a subgraph exploration.
 */
struct inflect_infos {
	unichar* inflected;
	unichar* local_semantic_code;
	unichar* output;
	struct inflect_infos* next;
};

//////////////////////////////
int SU_inflect(MultiFlex_ctx* p_multiFlex_ctx,struct l_morpho_t* pL_MORPHO,SU_id_T* SU_id, f_morpho_T* desired_features, SU_forms_T* forms,
		         Korean* korean,const char* pkgdir);
int SU_explore_state(MultiFlex_ctx* p_multiFlex_ctx,struct l_morpho_t* pL_MORPHO,unichar* flechi, unichar* canonique, unichar* sortie,
		Fst2* a, int etat_courant, f_morpho_T* desired_features,
		SU_forms_T* forms, int, int, unichar* var_name, unsigned int, unichar **,
		unichar *,Korean* korean);
int SU_explore_state_recursion(MultiFlex_ctx* p_multiFlex_ctx,struct l_morpho_t* pL_MORPHO,unichar* flechi, unichar* canonique,
		unichar* sortie, Fst2* a, int etat_courant, struct inflect_infos** L,
		f_morpho_T* desired_features, SU_forms_T* forms, int, int, unichar* var_name,
		unsigned int, unichar **, unichar *,Korean* korean);
int SU_explore_tag(MultiFlex_ctx* p_multiFlex_ctx,struct l_morpho_t* pL_MORPHO,Transition* T, unichar* flechi, unichar* canonique,
		unichar* sortie, Fst2* a, struct inflect_infos** LISTE,
		f_morpho_T* desired_features, SU_forms_T* forms, int, int, unichar* var_name,
		unsigned int, unichar **, unichar*,Korean* korean);
void shift_stack(unichar* stack, int pos, int shift);
void shift_stack(unichar* stack, int pos);
void shift_stack_left(unichar* stack, int pos);
void shift_stack_left2(unichar* stack,int shift);
int SU_convert_features(struct l_morpho_t* pL_MORPHO,f_morpho_T*** feat, unichar* feat_str);
struct list_ustring* SU_split_raw_features(unichar*);
int SU_feature_agreement(struct l_morpho_t* pL_MORPHO,f_morpho_T* feat, f_morpho_T* desired_features);
void SU_delete_inflection(SU_forms_T* forms);
int SU_cpy_features(f_morpho_T* feat, SU_id_T* SU_id);
void SU_delete_features(f_morpho_T* f);
SU_id_T* SU_get_id(SU_f_T* SU_form, SU_lemma_T* SU_lemma);
int SU_delete_id(SU_id_T* id);
void SU_init_forms(SU_forms_T** forms);
int SU_print_f(SU_f_T* f);
int SU_print_forms(SU_forms_T* F);
int SU_print_lemma(SU_lemma_T* l);
int SU_init_lemma(struct l_morpho_t* pL_MORPHO,SU_lemma_T* l, char* word, char* cl, char* para);
int SU_delete_lemma(SU_lemma_T* l);

//unichar Variables_op[22][100];
//int save_pos;

////////////////////////////////////////////
// For a given single unit, generates all the inflected forms corresponding to
// the given inflection features. For instance, this is used when we want to get
// the plural of "vive" knowing that its lemma is "vif" and that it's an adjective with
// the inflection code YYY and the inflection features "fs".
//
// SU_id:  single unit's identifier
// desired_features: morphology of the desired forms, e.g. {Gen=fem, Case=Inst}, or {} (if separator)
//                   NULL means that all inflected forms must be produced
// forms: return parameter; set of the inflected forms corresponding to the given inflection features
//        e.g. (3,{[reka,{Gen=fem,Nb=sing,Case=Instr}],[rekami,{Gen=fem,Nb=pl,Case=Instr}],[rekoma,{Gen=fem,Nb=pl,Case=Instr}]})
//        or   (1,{["-",{}]})
//        this structure is supposed to be allocated
//
// semitic: if not null, it means that we are inflecting a semitic word. In that case,
//          the stack is not initialized with the lemma but with the empty string. The lemma
//          is supposed to represent a consonantic skeleton like "ktb". The inflection fst2's paths
//          contains references to the consonants of this skeleton in the form of number from 1 to n
//          For instance, if we have the path "li1a2u3na", we will have the inflected form
//          "likatubna"
//
// Returns 0 on success, 1 otherwise.
int SU_inflect(MultiFlex_ctx* p_multiFlex_ctx,struct l_morpho_t* pL_MORPHO,Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input,
               SU_id_T* SU_id, f_morpho_T* desired_features, SU_forms_T* forms,
               int semitic,Korean* korean,const char* pkgdir) {
	int err;
	unichar inflected[MAX_CHARS_IN_STACK];
	unichar inflection_codes[MAX_CHARS_IN_STACK];
	unichar local_sem_code[MAX_CHARS_IN_STACK];
	inflection_codes[0] = '\0';
	int T = get_transducer(p_multiFlex_ctx,SU_id->lemma->paradigm,encoding_output,bom_output,
			mask_encoding_compatibility_input,pkgdir);
	if (p_multiFlex_ctx->fst2[p_multiFlex_ctx->T] == NULL) {
		// if the automaton has not been loaded
		return 1;
	}
	u_strcpy(inflected, semitic ? U_EMPTY : SU_id->lemma->unit);
	local_sem_code[0] = '\0';
    unichar var_name[100];
	err = SU_explore_state(p_multiFlex_ctx,pL_MORPHO,inflected, SU_id->lemma->unit, inflection_codes,
			p_multiFlex_ctx->fst2[T], 0, desired_features, forms, semitic, 0, var_name,
			0, NULL, local_sem_code,korean);
	return err;
}

/**
 * This function inflects a simple word. 'lemma' is the lemma as found in
 * the DELAS, 'inflection_code' is the name of the inflection transducer without
 * extension (ex: N43). 'forms' is a structure (supposed to be allocated) that
 * will receive all the produced inflected forms with their inflectional features.
 * The output DELAF lines will have to be built from 'forms'.
 */
int SU_inflect(MultiFlex_ctx* p_multiFlex_ctx,struct l_morpho_t* pL_MORPHO,Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input,
               unichar* lemma, char* inflection_code, unichar** filters,
               SU_forms_T* forms, int semitic,Korean* korean,const char* pkgdir) {
	int err;
	unichar inflected[MAX_CHARS_IN_STACK];
	unichar inflection_codes[MAX_CHARS_IN_STACK];
	unichar local_semantic_code[MAX_CHARS_IN_STACK];

	inflection_codes[0] = '\0';
	int T = get_transducer(p_multiFlex_ctx,inflection_code,encoding_output,bom_output,
			mask_encoding_compatibility_input,pkgdir);
	if (p_multiFlex_ctx->fst2[T] == NULL) {
		// if the automaton has not been loaded
		return 1;
	}
	u_strcpy(inflected, semitic ? U_EMPTY : lemma);
	local_semantic_code[0] = '\0';
    unichar var_name[100];
	err = SU_explore_state(p_multiFlex_ctx,pL_MORPHO,inflected, lemma, inflection_codes, p_multiFlex_ctx->fst2[T], 0,
			NULL, forms, semitic, 0, var_name, 0, filters,
			local_semantic_code,korean);
	return err;
}

Transition* explore_trans(Transition** T, Transition** debut, Fst2* a) {
	Transition empty, *ptr, *defaut;
	empty.next = *T;
	ptr = &empty;
	defaut = NULL;

	while (ptr != NULL && ptr->next != NULL) {
		if (ptr->next->tag_number >= 0) {
			Fst2Tag e = a->tags[ptr->next->tag_number];// A VERIFIER

			if (!u_strcmp(e->input, "<!>")) {
				defaut = ptr->next;
				ptr->next = ptr->next->next;
				defaut->next = empty.next;
				empty.next = defaut;
				//u_fprintf(stderr,"PASS0  def==%x\n",defaut);
				*T = empty.next;
				*debut = empty.next->next;
				return defaut;
			}
		}
		ptr = ptr->next;
	}
	*T = empty.next;
	*debut = *T;
	//u_fprintf(stderr,"PASS1\n");
	return defaut;
}

void aff_trans(Transition* T, Fst2* a) {
	Transition *t;
	t = T;

	while (t != NULL) {
		if (t->tag_number >= 0) {
			Fst2Tag e = a->tags[t->tag_number];
			error("AFF %S,", e->input);
		}
		t = t->next;
	}

	error("\n");
}

////////////////////////////////////////////
// Explores the transducer a starting from state 'etat_courant'.
// Conserves only the forms that agree with the 'desired_features'.
// desired_features: morphology of the desired forms, e.g. {Gen=fem, Case=Inst}, or {} (if separator)
//                   if 'desired_features' is NULL, it means that we want to generate all the
//                   inflected forms of a simple word. In that case, we consider raw inflection
//                   features like "fp" instead of structured ones like {Gen=fem, Nb=pl}
// forms: return parameter; set of the inflected forms corresponding to the given inflection features
//        e.g. (3,{[reka,{Gen=fem,Nb=sing,Case=Instr}],[rekami,{Gen=fem,Nb=pl,Case=Instr}],[rekoma,{Gen=fem,Nb=pl,Case=Instr}]})
//        or   (1,{["-",{}]})
// Returns 0 on success, 1 otherwise.
int SU_explore_state(MultiFlex_ctx* p_multiFlex_ctx,struct l_morpho_t* pL_MORPHO,unichar* flechi,
		unichar* canonique, unichar* sortie,
		Fst2* a, int etat_courant, f_morpho_T* desired_features,
		SU_forms_T* forms, int semitic, int flag_var, unichar* var_name,
		unsigned int var_in_use, unichar **filters,
		unichar *local_semantic_codes,Korean* korean) {
	int err;
	Fst2State e = a->states[etat_courant];
	if (e->control & 1) { //If final state
		if (desired_features != NULL) {
			/* If we want to select only some inflected forms */
			f_morpho_T** feat; //Table of sets of inflection features; necessary in case of factorisation of entries, e.g. :ms:fs
			err = SU_convert_features(pL_MORPHO,&feat, sortie);
			if (err) {
				return (err);
			}
			int f; //Index of the current morphological features in the current node
			f = 0;
			while (feat[f]) {
				//If the form's morphology agrees with the desired features
				if (SU_feature_agreement(pL_MORPHO,feat[f], desired_features)) {
					//Put the form into 'forms'
					forms->forms = (SU_f_T*) realloc(forms->forms,
							(forms->no_forms + 1) * sizeof(SU_f_T));
					if (!forms->forms) {
						fatal_alloc_error("SU_explore_state");
					}
					forms->forms[forms->no_forms].form = u_strdup(flechi);
					forms->forms[forms->no_forms].local_semantic_code = u_strdup(local_semantic_codes);
					forms->forms[forms->no_forms].features = feat[f];
					forms->no_forms++;
				} else { // If undesired form delete 'feat'
					f_delete_morpho(feat[f]);
				}
				f++;
			}
			free(feat);
		} else {
			/* If we want all the inflected forms */
			if (filters != NULL && filters[/*1*/0] != NULL) {
				filtrer(sortie, filters);
			}
			if (sortie[0] == '\0') {
				/* If we have an empty output, for instance in the case of an ADV grammar */
				//Put the form into 'forms'
				forms->forms = (SU_f_T*) realloc(forms->forms,
						(forms->no_forms + 1) * sizeof(SU_f_T));
				if (!forms->forms) {
					fatal_alloc_error("SU_explore_state");
				}
				forms->forms[forms->no_forms].form = u_strdup(flechi);
				forms->forms[forms->no_forms].local_semantic_code
						= u_strdup(local_semantic_codes);
				forms->forms[forms->no_forms].raw_features=u_strdup("");
				forms->no_forms++;
			} else {
				struct list_ustring* features = SU_split_raw_features(sortie);
				while (features != NULL) {
					//Put the form into 'forms'
					forms->forms = (SU_f_T*) realloc(forms->forms,
							(forms->no_forms + 1) * sizeof(SU_f_T));
					if (!forms->forms) {
						fatal_alloc_error("SU_explore_state");
					}
					forms->forms[forms->no_forms].form = u_strdup(flechi);

					forms->forms[forms->no_forms].local_semantic_code
							= u_strdup(local_semantic_codes);

					forms->forms[forms->no_forms].raw_features
							= features->string;
					forms->no_forms++;
					struct list_ustring* tmp = features->next;
					/* WARNING: here we must not call free_list_ustring, since the associated
					 * string would also be freed */
					free(features);
					features = tmp;
				}
			}
		}
	}
	int retour_all_tags = 0;
	int retour_tag;
	Transition* t = e->transitions, *default_trans = NULL;

	default_trans = explore_trans(&(e->transitions), &t, a);

	while (t != NULL) {//u_fprintf(stderr,"Explore 1\n");
		retour_tag = SU_explore_tag(p_multiFlex_ctx,pL_MORPHO,t, flechi, canonique, sortie, a, NULL,
				desired_features, forms, semitic, flag_var, var_name, var_in_use,
				filters, local_semantic_codes,korean);
		retour_all_tags += retour_tag;
		t = t->next;
	}
	if (default_trans != NULL) {//u_fprintf(stderr,"PASS02\n");retour_tag = SU_explore_tag(default_trans,flechi,canonique,sortie,a,NULL,desired_features,forms,semitic,flag_var,var_name, var_in_use,filters);
		//default_trans->next=e2->transitions->next;e2->transitions=default_trans;e=e2;
	}

	//if ((retour_all_tags+retour_tag)) return 0; else return 1;
	//Modif Agata
	// return 1;
	return 0;
}

/**
 * Allocates, initializes and returns a new inflect_infos structue.
 */
struct inflect_infos* new_inflect_infos() {
	struct inflect_infos* i = (struct inflect_infos*) malloc(
			sizeof(struct inflect_infos));
	if (i == NULL) {
		fatal_alloc_error("new_inflect_infos");
	}
	i->inflected = NULL;
	i->output = NULL;
	i->local_semantic_code = NULL;
	i->next = NULL;
	return i;
}

/**
 * Frees the given single list cell.
 */
void free_inflect_infos(struct inflect_infos* i) {
	if (i == NULL)
		return;
	if (i->inflected != NULL)
		free(i->inflected);
	if (i->output != NULL)
		free(i->output);
	if (i->local_semantic_code != NULL)
		free(i->local_semantic_code);
	free(i);
}

////////////////////////////////////////////
// Explore the sub-transducer a
// Conserve only the forms that agree with the 'desired_features'.
// desired_features: morphology of the desired forms, e.g. {Gen=fem, Case=Inst}, or {} (if separator)
// forms: return parameter; set of the inflected forms corresponding to the given inflection features
//        e.g. (3,{[reka,{Gen=fem,Nb=sing,Case=Instr}],[rekami,{Gen=fem,Nb=pl,Case=Instr}],[rekoma,{Gen=fem,Nb=pl,Case=Instr}]})
//        or   (1,{["-",{}]})
// Returns 0 on success, 1 otherwise.
int SU_explore_state_recursion(MultiFlex_ctx* p_multiFlex_ctx,struct l_morpho_t* pL_MORPHO,unichar* inflected, unichar* lemma,
		unichar* output, Fst2* a, int current_state, struct inflect_infos** L,
		f_morpho_T* desired_features, SU_forms_T* forms, int semitic,
		int flag_var, unichar* var_name, unsigned int var_in_use, unichar **filters,
		unichar *local_semantic_codes,Korean* korean) {
	Fst2State e = a->states[current_state];
	if (e->control & 1) {
		// if we are in a final state, we save the computed things
		struct inflect_infos* res = new_inflect_infos();
		res->inflected = u_strdup(inflected);

		res->local_semantic_code = u_strdup(local_semantic_codes);

		if (filters != NULL && filters[1] != NULL)
			filtrer(output, filters);
		res->output = u_strdup(output);
		res->next = (*L);
		(*L) = res;
	}
	//Transition* t=e->transitions,*default_trans;
	// default_trans = explore_trans(&t,a);
	Transition* t = e->transitions, *default_trans = NULL;

	default_trans = explore_trans(&(e->transitions), &t, a);
	//default_trans=NULL;
	//Transition* t=e->transitions;
	//static Transition *default_trans=explore_trans(&t,a);
	int retour_all_tags = 0;
	int retour_tag;
	while (t != NULL) {//u_fprintf(stderr,"Explore 2\n");local_
		retour_tag = SU_explore_tag(p_multiFlex_ctx,pL_MORPHO,t, inflected, lemma, output, a, L,
				desired_features, forms, semitic, flag_var, var_name, var_in_use,
				filters, local_semantic_codes,korean);
		retour_all_tags += retour_tag;
		t = t->next;
	}
	if (default_trans != NULL) { //u_fprintf(stderr,"PASS2\n");
		SU_explore_tag(p_multiFlex_ctx,pL_MORPHO,default_trans, inflected, lemma, output, a, L,
				desired_features, forms, semitic, flag_var, var_name, var_in_use,
				filters, local_semantic_codes,korean);
	}

	//if ((retour_all_tags+retour_tag)) return 0; else return 1;
	return 1;
}

struct SU_explore_tag_buffers
{
	unichar out[MAX_CHARS_IN_STACK];
	unichar stack[MAX_CHARS_IN_STACK];
	unichar tag[MAX_CHARS_IN_STACK];
} ;
////////////////////////////////////////////
// Explores the tag of the transition T
//
// desired_features: morphology of the desired forms, e.g. {Gen=fem, Case=Inst}, or {} (if separator)
// forms: return parameter; set of the inflected forms corresponding to the given inflection features
//        e.g. (3,{[reka,{Gen=fem,Nb=sing,Case=Instr}],[rekami,{Gen=fem,Nb=pl,Case=Instr}],[rekoma,{Gen=fem,Nb=pl,Case=Instr}]})
//        or   (1,{["-",{}]})
// Returns 0 on success, 1 otherwise.
int SU_explore_tag(MultiFlex_ctx* p_multiFlex_ctx,struct l_morpho_t* pL_MORPHO,Transition* T, unichar* inflected, unichar* lemma,
		unichar* output, Fst2* a, struct inflect_infos** LIST,
		f_morpho_T* desired_features, SU_forms_T* forms, int semitic,
		int flag_var, unichar* var_name, unsigned int var_in_use, unichar **filters,
		unichar *local_semantic_codes,Korean* korean) {
int old_local_semantic_code_length=u_strlen(local_semantic_codes);
	if (T->tag_number < 0) {
		/* If we are in the case of a call to a sub-graph */
		struct inflect_infos* L = NULL;
		struct inflect_infos* temp;
		int retour_state = 0;
		int retour_all_states = 1;
		SU_explore_state_recursion(p_multiFlex_ctx,pL_MORPHO,inflected, lemma, output, a,
				a->initial_states[-(T->tag_number)], &L, desired_features,
				forms, semitic, flag_var, var_name, var_in_use, filters,
				local_semantic_codes,korean);
		while (L != NULL) {
			if (LIST == NULL) {//u_fprintf(stderr,"Explore state 1\n");
				retour_state = SU_explore_state(p_multiFlex_ctx,pL_MORPHO,L->inflected, lemma, L->output,
						a, T->state_number, desired_features, forms,
						semitic, flag_var, var_name, var_in_use, filters,
						local_semantic_codes,korean);
				retour_all_states += (retour_state + 1);
			} else {//u_fprintf(stderr,"Explore state recursion 1\n");
				retour_state = SU_explore_state_recursion(p_multiFlex_ctx,pL_MORPHO,L->inflected, lemma,
						L->output, a, T->state_number, LIST, desired_features,
						forms, semitic, flag_var, var_name, var_in_use, filters,
						local_semantic_codes,korean);
				retour_all_states += (1 - retour_state);
			}
			temp = L;
			L = L->next;
			free_inflect_infos(temp);
		}
		//  return retour_all_states;
		return 0;
	}
	Fst2Tag t = a->tags[T->tag_number];
	int pos = u_strlen(inflected);
    /*
	unichar out[MAX_CHARS_IN_STACK];
	unichar stack[MAX_CHARS_IN_STACK];
	unichar tag[MAX_CHARS_IN_STACK];
    */
    struct SU_explore_tag_buffers* p_SU_buf = (struct SU_explore_tag_buffers*)malloc(sizeof(struct SU_explore_tag_buffers));
    if (p_SU_buf == NULL) {
        fatal_alloc_error("SU_explore_tag");
    }


	int i, ln, ind, retour;
	//static unichar var_name[100];
	retour = 1;

	u_strcpy(p_SU_buf->out, output);
	int pos_out = u_strlen(p_SU_buf->out);
	u_strcpy(p_SU_buf->stack, inflected);
	u_strcpy(p_SU_buf->tag, t->input);
	if (u_strcmp(p_SU_buf->tag, "<E>")) {
		/* If the tag is not <E>, we process it */
	   unichar foo;
	   int val;

	   if (u_starts_with(p_SU_buf->tag,"<R=")) {
		   /* Replacement of the first letter, useful for Malagasy */
		   if (p_SU_buf->tag[4]!='>' || p_SU_buf->tag[5]!='\0') {
			   fatal_error("Invalid <R=?> tag\n");
		   }
		   p_SU_buf->stack[0]=p_SU_buf->tag[3];
	   } else if (u_starts_with(p_SU_buf->tag,"<I=")) {
		   /* Insertion of an initial letter, useful for Malagasy */
		   if (p_SU_buf->tag[4]!='>' || p_SU_buf->tag[5]!='\0') {
			   fatal_error("Invalid <I=?> tag\n");
		   }
		   shift_stack(p_SU_buf->stack,1);
		   pos++;
		   p_SU_buf->stack[0]=p_SU_buf->tag[3];
	   } else if (1==u_sscanf(p_SU_buf->tag,"<X=%d>%C",&val,&foo)) {
		   /* Removal of the first val letters */
		   shift_stack_left2(p_SU_buf->stack,val);
		   pos=pos-val;
	   } else if (semitic && 1==u_sscanf(p_SU_buf->tag,"<%d>%C",&val,&foo)) {
         /* If we are in semitic mode, we must handle tags like <12> like references
          * to letters in the lemma. We must deal this way with values >9, because
          * the graph compiler would split "12" in "1" and "2" */
	      val--;
	      if (val<0 || val>=((int)u_strlen(lemma))) {
	         error("Invalid reference in %S.fst2 to consonant #%d for skeleton \"%S\"\n",
                  a->graph_names[1], val+1, lemma);
            free(p_SU_buf);
            return 0;
         }
         p_SU_buf->stack[pos++] = lemma[val];
	   }
	   /* Otherwise, we deal with the tag in the normal way */
	   else for (int pos_tag = 0; p_SU_buf->tag[pos_tag] != '\0';) {
		   if (t->control & RESPECT_CASE_TAG_BIT_MASK
				   ||
				   (semitic && (is_solar(p_SU_buf->tag[pos_tag]) || is_lunar(p_SU_buf->tag[pos_tag]))
				   )
				) {
			   /* If the transition was a "..." one, we don't try to interpret its content.
			    * This is useful when one needs to produce a symbol that is an inflection
			    * operator */
			   p_SU_buf->stack[pos++]=p_SU_buf->tag[pos_tag++];
		   } else switch (p_SU_buf->tag[pos_tag]) {
			case '<':
				retour = flex_op_with_var(p_multiFlex_ctx->Variables_op, p_SU_buf->stack, p_SU_buf->tag, &pos,
						&pos_tag, &var_in_use);

				break;
			case '$':
			case (unichar) POUND: {
				var_name[0] = p_SU_buf->tag[pos_tag];
				var_name[1] = '\0';
				p_multiFlex_ctx->save_pos = pos;
				ind = get_indice_var_op(var_name);
				if (get_flag_var(ind, var_in_use)) {
					ln = u_strlen(p_multiFlex_ctx->Variables_op[ind]);
					for (i = 0; i < ln; i++, pos++)
						p_SU_buf->stack[pos] = p_multiFlex_ctx->Variables_op[ind][i];
				}
				//if (VERBOSE) fprintf(stderr,"COPIE VAR \n");
				flag_var = 1;
				pos_tag++;
			}
				;
				break;
				/* Left move operator */
			case 'L': {
				if (pos != 0) {
					/* If the stack is not empty, we decrease the stack pointer */
					pos--;
				}
				pos_tag++;
				break;
			}

				/* Right move operator */
			case 'R': {
				pos++;
				pos_tag++;
				break;
			}

				/* Unaccent operator */
			case 'U': {
				p_SU_buf->stack[pos] = u_deaccentuate(p_SU_buf->stack[pos]);
				pos++;
				pos_tag++;
				break;
			}

				/* Lowercase operator */
			case 'W': {
				p_SU_buf->stack[0] = u_tolower(p_SU_buf->stack[0]);
				pos_tag++;
				break;
			}

				/* Uppercase operator */
			case 'P': {
				p_SU_buf->stack[0] = u_toupper(p_SU_buf->stack[0]);
				pos_tag++;
				break;
			}

			/* Korean Jamo left operator */
			case 'J': {
				pos_tag++;
				if (korean==NULL) {
					fatal_error("NULL jamo error: cannot apply operator J if no jamo table is defined\n");
				}
				if (pos==0) {
					fatal_error("Cannot apply operator J to empty stack\n");
				}
				if (!u_is_Hangul(p_SU_buf->stack[pos-1]) && !u_is_Hangul_Jamo(p_SU_buf->stack[pos-1])) {
					fatal_error("Cannot apply J operator to a non Hangul or Jamo character '%C' (%04X)\n",p_SU_buf->stack[pos-1],p_SU_buf->stack[pos-1]);
				}
				if (u_is_Hangul(p_SU_buf->stack[pos-1])) {
					/* If we have a Hangul syllable, we first turn it into a Jamo
					 * character sequence */
					unichar tmp[10];
					unichar src[2];
					src[0]=p_SU_buf->stack[pos-1];
					src[1]='\0';
					Hanguls_to_Jamos(src,tmp,korean,0);
					int len=u_strlen(tmp);
					/* Now, we copy the jamo sequence
					 * in place of the hangul syllable. We use l-1 because we take into
					 * account the hangul syllable */
					pos--;
					for (int il=0;il<len;il++) {
						p_SU_buf->stack[pos++]=tmp[il];
					}
				}
				if (u_is_Hangul_Jamo_consonant(p_SU_buf->stack[pos-1])) {
					while (u_is_Hangul_Jamo_consonant(p_SU_buf->stack[pos-1])) {
						pos--;
					}
					p_SU_buf->stack[pos]='\0';
				}
				else if (u_is_Hangul_Jamo_medial_vowel(p_SU_buf->stack[pos-1])){
					while (u_is_Hangul_Jamo_medial_vowel(p_SU_buf->stack[pos-1])) {
						pos--;
					}
					p_SU_buf->stack[pos]='\0';
				} else {
					fatal_error("Operator J: unexpected character '%C' (%04X)\n",p_SU_buf->stack[pos-1],p_SU_buf->stack[pos-1]);
				}
				break;
			}

			/* Korean syllable delimiter operator */
			case '.': {
				if (pos>0 && u_is_Hangul_Jamo(p_SU_buf->stack[pos-1])) {
					/* If the last char is a jamo, then we want to recombine all previous jamo
					 * with the first syllable found on the left */
                   int z=pos-1;
                   while (z>0 && (u_is_Hangul_Jamo(p_SU_buf->stack[z]) || p_SU_buf->stack[z]==KR_SYLLABLE_BOUND)) {
                	   z--;
                   }
                   if (z<0 || !u_is_Hangul(p_SU_buf->stack[z])) {
                	   fatal_error("Operator . unexpected if no hangul before jamos\n");
                   }
                   unichar hangul[2];
                   hangul[0]=p_SU_buf->stack[z];
                   hangul[1]='\0';
                   unichar tmp[32];
                   Hanguls_to_Jamos(hangul,tmp,korean,0);
                   int len2=u_strlen(tmp);
                   int ip;
                   for (ip=z+1;ip<pos;ip++) {
                	  if (p_SU_buf->stack[ip]!=KR_SYLLABLE_BOUND) {
                		  /* The syllable bound must be ignored when we have to recombine
                		   * jamos with an hangul */
                		  tmp[len2++]=p_SU_buf->stack[ip];
                	  }
                   }
                   tmp[len2]='\0';
                   unichar tmp2[32];
                   convert_jamo_to_hangul(tmp,tmp2,korean);
                   u_strcpy(p_SU_buf->stack+z,tmp2);
                   pos=z+u_strlen(tmp2);
				}
				p_SU_buf->stack[pos++] = KR_SYLLABLE_BOUND;
				pos_tag++;
				break;
			}

				/* Right copy operator */
			case 'C': {
				shift_stack(p_SU_buf->stack, pos);
				pos++;
				pos_tag++;
				break;
			}

				/* Left copy operator */
			case 'D': {
				shift_stack_left(p_SU_buf->stack, pos);
				pos--;
				pos_tag++;
				break;
			}

				/* If we have a digit between 1 and 9, we consider it as a reference to a root
				 * consonant, but only if we are in semitic mode */

			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (flag_var) {
					var_name[1] = p_SU_buf->tag[pos_tag];
					var_name[2] = '\0';
					flag_var = 0;
					pos = p_multiFlex_ctx->save_pos;
					ind = get_indice_var_op(var_name);
					if (get_flag_var(ind, var_in_use)) {
						ln = u_strlen(p_multiFlex_ctx->Variables_op[ind]);
						for (i = 0; i < ln; i++, pos++)
							p_SU_buf->stack[pos] = p_multiFlex_ctx->Variables_op[ind][i];
					}
					pos_tag++;
				} else if (semitic) {
				   int pos_letter=p_SU_buf->tag[pos_tag++]-'0';
					int ip = pos_letter-1; /* Numbering from 0, always... */
					if (ip >= ((int)u_strlen(lemma))) {
						error(
								"Invalid reference in %S.fst2 to consonant #%C for skeleton \"%S\"\n",
								a->graph_names[1], p_SU_buf->tag[pos_tag - 1], lemma);
                        free(p_SU_buf);
						return 0;
					}
					p_SU_buf->stack[pos++] = lemma[ip];
				}
				break;

				/* Default push operator */
			default: {
				unichar tmp[32];
				single_HGJ_to_Jamos(p_SU_buf->tag[pos_tag],tmp,korean);
				int len3=u_strlen(tmp);
				u_strncpy(p_SU_buf->stack+pos,tmp,len3);
				pos=pos+len3;
				//old version before Korean: stack[pos++] = tag[pos_tag];
				pos_tag++;
				break;
			}
			}
		}
	}
	p_SU_buf->out[pos_out] = '\0';
	/* We process the output, if any and not NULL */
	if (t->output != NULL && u_strcmp(t->output, "<E>")) {
		/* If we are in normal mode, we just append the tag's output to the global one */
		if (t->output[0] == '+') {
			//error("SEMANTIC:%S\n",local_semantic_codes);
			//error("SEM:%S\n",t->output);
			int sem = 0;
			int x=old_local_semantic_code_length;
			while (t->output[sem] != ':' && t->output[sem] != '\0') {
				local_semantic_codes[x++] = t->output[sem];
				sem++;
			}
			local_semantic_codes[x] = '\0';
			u_strcat(p_SU_buf->out, t->output+sem);
		} else {
		    u_strcat(p_SU_buf->out, t->output);
		}
	}
	/* Then, we go the next state */
	p_SU_buf->stack[pos] = '\0';

	int retour_state = 1;
	if (retour) {
		if (LIST == NULL) {//u_fprintf(stderr,"Explore state 2\n");
			retour_state = SU_explore_state(p_multiFlex_ctx,pL_MORPHO,p_SU_buf->stack, lemma, p_SU_buf->out, a,
					T->state_number, desired_features, forms,
					semitic, flag_var, var_name, var_in_use, filters,
					local_semantic_codes,korean);
		} else {//u_fprintf(stderr,"Explore state recursion 2\n");
			retour_state = SU_explore_state_recursion(p_multiFlex_ctx,pL_MORPHO,p_SU_buf->stack, lemma, p_SU_buf->out, a,
					T->state_number, LIST, desired_features, forms,
					semitic, flag_var, var_name, var_in_use, filters,
					local_semantic_codes,korean);
		}
	}
	local_semantic_codes[old_local_semantic_code_length]='\0';
	//return (retour * (1-retour_state));
    free(p_SU_buf);
	return retour;
}

////////////////////////////////////////////
// Shifts right all the stack from the position pos
// 'shift' is the length of the move in chars
void shift_stack(unichar* stack, int pos,int shift) {
	if (pos == 0) {
		// this case should never happen
		return;
	}
	for (int i = (MAX_CHARS_IN_STACK - shift); i >= pos; i--) {
		stack[i] = stack[i - shift];
	}
}


////////////////////////////////////////////
// Shifts right all the stack from the position pos
void shift_stack(unichar* stack, int pos) {
shift_stack(stack,pos,1);
}


//
// Shifts all the stack to the left from the position pos
//
void shift_stack_left(unichar* stack, int pos) {
	if (pos == 0) {
		// this case should never happen
		return;
	}
	for (int i = pos - 1; i < MAX_CHARS_IN_STACK-1; i++) {
		stack[i] = stack[i + 1];
	}
}

//
// Shifts all the stack to the left from the position pos
//
void shift_stack_left2(unichar* stack,int shift) {
	if (shift == 0) {
		// this case should never happen
		return;
	}
	for (int i=0;i<MAX_CHARS_IN_STACK-shift;i++) {
		stack[i]=stack[i+shift];
	}
}


////////////////////////////////////////////
// Convert a textual representation of features 'feat_str', e.g. :Ipf:Ipm
// into a set of feature structures, e.g. <<Case=I; Nb=p; Gen=f>,<Case=I; Nb=p; Gen=m>>
// Initially 'feat' does not have its space allocated
// Returns 1 on error, 0 otherwise.
int SU_convert_features(struct l_morpho_t* pL_MORPHO,f_morpho_T*** feat, unichar* feat_str) {
	unichar tmp[MAX_CATS]; //buffer for a single set of features, e.g. Ipf
	f_morpho_T* f; //current feature structure
	int l; //length of a scanned sequence
	int no_f; //number of feature sets scanned

	(*feat) = (f_morpho_T**) malloc(sizeof(f_morpho_T*));
	if ((*feat) == NULL) {
		fatal_alloc_error("SU_convert_features");
	}
	no_f = 0;

	while (*feat_str) {
		//Delete the leading ':' if any
		if (feat_str[0] == (unichar) ':')
			feat_str++;
		//Get next feature structures, e.g. Ipf, out of a sequence, e.g. :Ipf:Ipm
		l = u_scan_until_char(tmp, feat_str, MAX_CATS - 1, ":", 1);
		feat_str = feat_str + l;
		//Convert the set of features, e.g. Ipm, into a feature structure, e.g. <Case=I; Nb=p; Gen=m>
		f = d_get_feat_str(pL_MORPHO,tmp);
		if (!f) {
			for (int i = 0; i < no_f; i++)
				//delete the previously constracted feature structures
				free((*feat)[i]);
			free(*feat);
			return 1;
		}
		no_f++;
		(*feat) = (f_morpho_T**) realloc((*feat), (no_f + 1)
				* sizeof(f_morpho_T*));
		if (!(*feat)) {
			fatal_alloc_error("SU_convert_features");
		}
		(*feat)[no_f - 1] = f;
	}
	(*feat)[no_f] = NULL;
	return 0;
}

////////////////////////////////////////////
// Splits a textual representation of features 'feat_str', e.g. :Ipf:Ipm
// into a list of strings "Ipf", "Ipm"
// Returns the list; NULL if an error occurs.
struct list_ustring* SU_split_raw_features(unichar* features) {
	struct list_ustring* result = NULL;
	if (features == NULL || features[0] == '\0') {
		return NULL;
	}
	int pos = 0;
	if (features[0] == ':') {
		pos++;
	}
	unichar feature[1024];
	while (P_OK == parse_string(features, &pos, feature, P_COLON)) {
		result = new_list_ustring(feature, result);
		if (features[pos] == ':') {
			pos++;
		}
	}
	return result;
}

////////////////////////////////////////////
// Returns 1 if the set of morphological features 'feat1'
// agrees with the set 'feat2', i.e. both contains exactly the same
// values, except for empty values
// Otherwise returns 0.
// If an empty feature is present in on set then the
// corresponding category in the other set must also be empty.
// e.g. if feat1=<Gen=m;Nb=pl> and feat2=<Gen=m;Nb=pl; Gr=<E>>
// then both agree
// But if feat1=<Gen=m;Nb=pl; Gr=D> and feat2=<Gen=m;Nb=pl>
// then they do not agree
//
// In order to facilitate the processing of simple words, we say
// that 'feat2'=NULL means that all the inflected forms are
// to keep.
int SU_feature_agreement(struct l_morpho_t* pL_MORPHO,f_morpho_T* feat1, f_morpho_T* feat2) {
	if (feat2 == NULL)
		return 1;

	if (feat1 == NULL)
		return 0;

	int f1; //Index of the current feature in 'feat1'
	int f2; //Index of the current feature in 'feat2'
	int found; //Has the current feature's category been found in 'feat2'

	//Each category-value pair of the 'feat2' has to be present in 'feat1'
	//except empty features (<E>)
	for (f2 = 0; f2 < feat2->no_cats; f2++) {
		found = 0;
		f1 = 0;
		while ((!found) && (f1 < feat1->no_cats)) {
			if (feat2->cats[f2].cat == feat1->cats[f1].cat) {
				found = 1;
				//If the same category then the value has to be the same
				if (feat2->cats[f2].val != feat1->cats[f1].val)
					return 0;
			}
			f1++;
		}
		//If a desired category has a non empty value, and it does not appear in 'feat'
		//then both feature sets do not agree
		if (!found && !is_empty_val(pL_MORPHO,feat2->cats[f2].cat, feat2->cats[f2].val))
			return 0;
	}

	//Each category-value pair of the 'feat' has to be present in 'feat2'
	//except empty features (<E>)
	for (f1 = 0; f1 < feat1->no_cats; f1++) {
		found = 0;
		f2 = 0;
		while ((!found) && (f2 < feat2->no_cats)) {
			if (feat1->cats[f1].cat == feat2->cats[f2].cat) {
				found = 1;
				//If the same category then the value has to be the same
				if (feat1->cats[f1].val != feat2->cats[f2].val)
					return 0;
			}
			f2++;
		}
		//If a desired category has a non empty value, and it does not appear in 'feat'
		//then both feature sets do not agree
		if (!found && !is_empty_val(pL_MORPHO,feat1->cats[f1].cat, feat1->cats[f1].val))
			return 0;
	}
	return 1;
}

////////////////////////////////////////////
// Liberates the memory allocated for a set of forms
void SU_delete_inflection(SU_forms_T* forms) {
	int f;
	if (!forms)
		return;
	for (f = 0; f < forms->no_forms; f++) {
		free(forms->forms[f].form);
		free(forms->forms[f].features);
        if (forms->forms[f].local_semantic_code != NULL) {
		   free(forms->forms[f].local_semantic_code);
        }
	}
    free(forms->forms);
}

////////////////////////////////////////////
// Returns in 'feat' a copy of the inflection features of the given form.
// Initially 'feat' has its space allocated but is empty.
// Returns 1 on error, 0 otherwise.
int SU_cpy_features(f_morpho_T* feat, SU_id_T* SU_id) {
	int c;

	if (!SU_id || !SU_id->feat)
		return 1;

	for (c = 0; c < SU_id->feat->no_cats; c++)
		feat->cats[c] = SU_id->feat->cats[c];

	feat->no_cats = SU_id->feat->no_cats;
	return 0;
}

////////////////////////////////////////////
// Liberates the memory allocated for a form's morphology.
void SU_delete_features(f_morpho_T* f) {
	free(f);
}

////////////////////////////////////////////
// Returns the word form's identifier on the basis of the form, its lemma, its inflection paradigm, and its inflection features.
// SU_form : form and its inflection features, e.g. [rekoma,{Gen=fem,Nb=pl,Case=Instr}], or ["-",{}]
// SU_lemma : lemma and its inflection paradigm, e.g. [reka,noun,N56,{"Conc"},""body"], or void (if separator)
// Returns the pointer to the forms identifier on success
// (e.g. ->("reka",word,[reka,noun,N56,{"Conc"},"body"],{Gen=fem; Nb=sing; Case=I})), NULL otherwise.
// The identifier is allocated in this function. The liberation must be done by the calling function.
SU_id_T* SU_get_id(unichar* form, f_morpho_T* feat, SU_lemma_T* SU_lemma) {
	SU_id_T* SU_id;
	SU_id = (SU_id_T*) malloc(sizeof(SU_id_T));
	if (SU_id == NULL) {
		fatal_alloc_error("SU_get_id");
	}

	//Form
	SU_id->form = u_strdup(form);

	if (SU_lemma) { //if not a separator

		//Lemma
		SU_id->lemma = (SU_lemma_T*) malloc(sizeof(SU_lemma_T));
		if (!SU_id->lemma) {
			fatal_alloc_error("SU_get_id");
		}
		//Lemma form
		SU_id->lemma->unit = u_strdup(SU_lemma->unit);
		//Class
		SU_id->lemma->cl = SU_lemma->cl;
		//Paradigm
		SU_id->lemma->paradigm = strdup(SU_lemma->paradigm);
		if (SU_id->lemma->paradigm == NULL) {
			fatal_alloc_error("SU_get_id");
		}
		//Features
		f_morpho_T* fea;
		fea = (f_morpho_T*) malloc(sizeof(f_morpho_T));
		if (!fea) {
			fatal_alloc_error("SU_get_id");
		}
		f_init_morpho(fea);
		int f; //index of the current category-value pair in 'feat'
		for (f = 0; f < feat->no_cats; f++) {
			f_add_morpho(fea, feat->cats[f].cat, feat->cats[f].val);
		}

	}
	return SU_id;
}

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
int SU_get_unit(unichar* unit, unichar* line, int max, Alphabet* alph,
		int eliminate_bcksl) {
	int l = 0; //length of the scanned sequence
	int u = 0; //index of the next element in 'unit'
	int end = 0;
	int no_elim_bcksl = 0; //number of eliminated backslashes
	int bcksl_precedes = 0; //1 if the preceding character was a '\', 0 otherwise
	if (!line)
		return 0;

	//Separator
	if (line[0] == (unichar) '\\' && line[1] && !is_letter(line[1], alph))
		if (eliminate_bcksl) {
			unit[0] = line[1];
			unit[1] = '\0';
			return 2;
		} else {
			unit[0] = line[0];
			unit[1] = line[1];
			unit[2] = '\0';
			return 2;
		}
	else if (line[0] != (unichar) '\\' && line[0] && !is_letter(line[0], alph)) {
		unit[0] = line[0];
		unit[1] = '\0';
		return 1;
	}

	//Word
	else {
		l = 0;
		u = 0;
		end = 0;
		no_elim_bcksl = 0;
		bcksl_precedes = 0;
		while (!end && u < max && line[l] != (unichar) '\0')
			if (line[l] == (unichar) '\\') //current character is a '\'
				if (!bcksl_precedes) { //not preceded by '\'
					bcksl_precedes = 1;
					l++;
				} else //preceded by '\'
				if (!is_letter(line[l], alph)) //'\' is no letter
					end = 1;
				else { //\ '\' is a letter
					if (!eliminate_bcksl)
						unit[u++] = (unichar) '\\';
					else
						no_elim_bcksl++;
					unit[u++] = line[l++];
					bcksl_precedes = 0;
				}
			else {//Current character is not a '\'
				if (!is_letter(line[l], alph)) //'\' is no letter
					end = 1;
				else { //\ '\' is a letter
					if (bcksl_precedes) {
						if (!eliminate_bcksl)
							unit[u++] = (unichar) '\\';
						else
							no_elim_bcksl++;
					}
					unit[u++] = line[l++];
					bcksl_precedes = 0;
				}
			}
	}
	unit[u] = (unichar) '\0';
	return u + no_elim_bcksl;
}

////////////////////////////////////////////
// Liberates the memory allocated for a form's id.
int SU_delete_id(SU_id_T* id) {
	free(id->form);
	if (id->lemma)
		SU_delete_lemma(id->lemma);
    if (id->feat)
        f_delete_morpho(id->feat);
	free(id);
	return 0;
}

////////////////////////////////////////////
// Initialize the single-unit 'forms' with null values
// We suppose that 'forms' has its space allocated
void SU_init_forms(SU_forms_T* forms) {
	forms->no_forms = 0;
	forms->forms = NULL;
}

////////////////////////////////////////////
// Initialize the set of inflected forms "forms" with
// the unique form "form"
// E.g. if form = "rekami" then forms becomes (1,{("rekami",NULL)}
void SU_init_invariable_form(SU_forms_T* forms, const unichar* form) {
	forms->no_forms = 1;
	forms->forms = (SU_f_T*) malloc(sizeof(SU_f_T));
	if (!forms->forms) {
		fatal_alloc_error("SU_init_invariable_form");
	}
	forms->forms[0].form = u_strdup(form);
	forms->forms[0].local_semantic_code=NULL;
	forms->forms[0].features = NULL;
}

/////////////////////////////////////////////////////////////////////
// Same as SU_init_invariable_form but the second parameter is a char*
void SU_init_invariable_form_char(SU_forms_T* forms, const char* form) {
	forms->no_forms = 1;
	forms->forms = (SU_f_T*) malloc(sizeof(SU_f_T));
	if (!forms->forms) {
		fatal_alloc_error("SU_init_invariable_form_char");
	}
	forms->forms[0].form = u_strdup(form);
	forms->forms[0].local_semantic_code=NULL;
	forms->forms[0].features = NULL;
}

////////////////////////////////////////////
// Prints a form and its inflection features.
int SU_print_f(SU_f_T* f) {
	u_printf("%S : ", f->form);
	f_print_morpho(f->features);
	return 0;
}

////////////////////////////////////////////
// Prints a set of forms and their inflection features.
int SU_print_forms(SU_forms_T* F) {
	int f;
	for (f = 0; f < F->no_forms; f++)
		SU_print_f(&(F->forms[f]));
	return 0;
}

////////////////////////////////////////////
// Prints a lemma and its info.
int SU_print_lemma(SU_lemma_T* l) {
	u_printf("%S:", l->unit); //lemma
	u_printf("%S:", l->cl->name); //class
	u_printf("%s", l->paradigm); //inflection paradigm
	return 0;
}

////////////////////////////////////////////
// For testing purposes
// Initialise a sample lemma structure for tests.
int SU_init_lemma(struct l_morpho_t* pL_MORPHO,SU_lemma_T* l, char* word, char* cl, char* para) {
	//lemma
	l->unit = u_strdup(word);
	//class
	if (!strcmp(cl, "noun"))
		l->cl = &(pL_MORPHO->L_CLASSES.classes[0]);
	else if (!strcmp(cl, "adj"))
		l->cl = &(pL_MORPHO->L_CLASSES.classes[1]);
	else if (!strcmp(cl, "adv"))
		l->cl = &(pL_MORPHO->L_CLASSES.classes[2]);
	else
		l->cl = NULL;
	//paradigm
	l->paradigm = strdup(para);
	if (l->paradigm == NULL) {
		fatal_alloc_error("SU_init_lemma");
	}
	return 0;
}

////////////////////////////////////////////
// For testing purposes
// Delete sample lemma stucture for tests.
int SU_delete_lemma(SU_lemma_T* l) {
	free(l->unit);
	free(l->paradigm);
	free(l);
	return 0;
}

////////////////////////////////////////////
// *COMMENT CONCERNING THE UNIQUE IDENTIFICATION OF SINGLE WORD FORMS
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
//	2) If the lemma is missing we may not lemmatize the form (unless the paradigm allows to do that on the basis of the 3 elements)
//	3) If the inflection paradigm is missing we may not produce other inflected forms.
//	4) If the inflection deatures are missing the may be an ambiguity in case of homographs e.g. "rece" may be both
//	   {Gen=fem,Nb=pl,Case=Nom} and {Gen=fem,Nb=pl,Case=Acc}

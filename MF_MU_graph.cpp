/*
  * Unitex 
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//#include <stdlib.h>

#include "MF_MU_graph.h"
#include "Fst2.h"
#include "MF_Util.h"
#include "MF_SU_morpho.h"
#include "MF_Unif.h"
#include "MF_InflectTransd.h"
#include "Error.h"
////////////////////////////////////////////
// GLOBAL VARIABLES

//////////////////////////////
// Table of inflection tranducers
extern Fst2* fst2[N_FST2];

//////////////////////////////
// Index of the current transducer in the transducer table
int T;

////////////////////////////////////////////
// Current muli-word unit to be inflected
MU_lemma_T* MU_lemma;

int MU_graph_explore_graph(MU_lemma_T* MU_lemma, MU_forms_T* forms);
int MU_graph_init_graphs();
void MU_graph_free_graphs();
Fst2State MU_graph_get_initial(char* graph_name);
int MU_graph_explore_state(Fst2State q,MU_forms_T* forms);
int MU_graph_explore_label(MU_graph_label_T* l,Fst2State q_bis, MU_forms_T* forms);
int MU_graph_scan_label(unichar* label_in, unichar* label_out, MU_graph_label_T* MU_label);
int MU_graph_explore_label_in_var(SU_id_T* u,MU_graph_morpho_T* l_in_morpho, MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_in_var_rec(SU_id_T* u,MU_graph_morpho_T* l_in_morpho, int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_get_unit_forms(SU_id_T* u,f_morpho_T* feat,SU_forms_T* SU_forms);
int MU_graph_explore_label_in_morph_const(MU_graph_category_T* c, SU_id_T* u, MU_graph_morpho_T* l_in_morpho, int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_in_morph_inher(MU_graph_category_T* c, SU_id_T* u, MU_graph_morpho_T* l_in_morpho, int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_in_morph_unif(MU_graph_category_T* c, SU_id_T* u, MU_graph_morpho_T* l_in_morpho, int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_out(MU_graph_morpho_T* l_out_morpho,Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_out_rec(MU_graph_morpho_T* l_out_morpho,int i_morpho,f_morpho_T* feat, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_out_morph_const(MU_graph_category_T* c, MU_graph_morpho_T* l_out_morpho, int i_morpho, f_morpho_T* feat, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_explore_label_out_morph_unif(MU_graph_category_T* c, MU_graph_morpho_T* l_out_morpho, int i_morpho, f_morpho_T* feat, Fst2State q_bis, MU_forms_T* forms);
int MU_graph_scan_label(unichar* label_in, unichar* label_out, MU_graph_label_T* MU_label);
int MU_graph_scan_label_in(unichar* label, MU_graph_in_T* MU_label_in);
int MU_graph_scan_label_out(unichar* label, MU_graph_out_T* MU_label_out);
int MU_graph_scan_graph_morpho(unichar* label, MU_graph_morpho_T* MU_graph_morpho);
void MU_graph_print_label(MU_graph_label_T* MU_label);
void MU_graph_print_morpho(MU_graph_morpho_T* MU_morpho);
void MU_graph_free_label(MU_graph_label_T* MU_label);
void MU_graph_free_morpho(MU_graph_morpho_T* MU_morpho);

/////////////////////////////////////////////////
// Explores the inflection transducer of the MU-lemma 'MU_lemma' 
// in order to generate all its inflected forms. The generated forms are put to 'forms'
// Initially, '*forms' has its space allocated but is empty.
// Returns 0 on success, 1 otherwise. 
int MU_graph_explore_graph(MU_lemma_T* MU_l, MU_forms_T* forms) {
  int res;

 //Initialize the current muli-word unit
 MU_lemma = MU_l;

 //Initialize the structure for graph unification variables
  unif_init_vars();
  
  //Initialize the set of inflected forms
  forms->no_forms = 0;
  forms->forms = NULL;

  //Get the initial state of the inflection tranducer
  Fst2State initial;
  initial = MU_graph_get_initial(MU_lemma->paradigm);
  if (!initial)
    return 1;

  //Explore the inflection transducer starting from its initial state
  res=MU_graph_explore_state(initial,forms);

  unif_free_vars();
  return res;
}

/////////////////////////////////////////////////
// Initializes the structure for inflection graphs
// Returns 1 on success, 1 otherwise.
int MU_graph_init_graphs() {
 if (init_transducer_tree())
   return 1;
 return 0;
}

/////////////////////////////////////////////////
// Liberates the memory allocated for the structure for inflection graphs' names
void MU_graph_free_graphs() {
  free_transducer_tree();
}

/////////////////////////////////////////////////
// In the graph not yet loaded loads it otherwise searches for it in the structure.
// On success returns the graph's initial state, otherwise returns NULL.
Fst2State MU_graph_get_initial(char* graph_name) {
  //Get the index of the tranducer in the transducer table
  T=get_transducer(graph_name);  
  if (fst2[T]==NULL) {
    // if the automaton has not been loaded
    return NULL;
  }
  //Get the initial state
  Fst2* a = fst2[T];
  Fst2State q=a->states[0];
  return q;
}

////////////////////////////////////////////
// Given the current instantiation of unification variables and of
// inheritance variables, starting from state q, explore the current state q
// of an inflection transducer for MWUs. Generate suffixes of the inflected
// forms and their features, and put them to 'forms'.
// In case of error put an empty list into 'forms' (which is not identical to ((epsilon,empty_set))). 
// Initially, '*forms' has its space allocated but is empty.
// Return a number !=0 in case of errors, 0 otherwise. 
int MU_graph_explore_state(Fst2State q, MU_forms_T* forms) {
  int err;
  MU_graph_label_T* l;
  Fst2State q_bis;

  //If we are in a final state, then the empty word is recognized, we add it to 'forms' and we continue to explore
  if (q->control & 1) {
    //Allocate space for a couple (epsilon,empty_set)
    forms->forms = (MU_f_T*)malloc(sizeof(MU_f_T));
    if (!forms->forms) {
      fatal_error("Not enough memory in function MU_graph_explore_state\n");
    }
    MU_f_T* f;
    f = &(forms->forms[0]);
    f->form = (unichar*) malloc(sizeof(unichar));
    if (f->form==NULL) {
       fatal_error("Not enough memory in function MU_graph_explore_state\n");
    }
    f->form[0] = (unichar) '\0';
    f->features = (f_morpho_T*) malloc(sizeof(f_morpho_T));
    if (f->features==NULL) {
       fatal_error("Not enough memory in function MU_graph_explore_state\n");
    }
    f->features->no_cats = 0;
    forms->no_forms++;
  }
  
  //Explore each outgoing transition
  Fst2Transition t;
  MU_forms_T forms_bis;
  int mf;  //index of a sigle MU form
  
  t = q->transitions;
  while (t) {
    l = (MU_graph_label_T*) malloc(sizeof(MU_graph_label_T));
    if (!l) {
       fatal_error("Not enough memory in function MU_graph_explore_state\n");
    }
    q_bis = fst2[T]->states[t->state_number];  //get the arrival state
    Fst2Tag e = fst2[T]->tags[t->tag_number];  //get the transition's label
    err = MU_graph_scan_label(e->input,e->output,l);  //transform the label into a MU_graph_label
    if (err)
      return err;
    //Initialize the set of inflected forms
    forms_bis.no_forms = 0;
    forms_bis.forms = NULL;
    err = MU_graph_explore_label(l,q_bis,&forms_bis);
    if (err)
      return err;
    if (forms_bis.no_forms) {  //If any suffix found, add it to the list of those generated previously
      forms->forms = (MU_f_T*)realloc(forms->forms,sizeof(MU_f_T)*(forms->no_forms+forms_bis.no_forms));
      if (! forms->forms) {
	      fatal_error("Not enough memory in function MU_graph_explore_state\n");
      }
      for (mf=0; mf<forms_bis.no_forms; mf++) 
	forms->forms[forms->no_forms+mf] = forms_bis.forms[mf];
      forms->no_forms += forms_bis.no_forms;
      //Free the current label
      MU_graph_free_label(l);
    }
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
int MU_graph_explore_label(MU_graph_label_T* l,Fst2State q_bis, MU_forms_T* forms) { 
  int err;

  //If label's input empty then explore the output
  if (! l->in) { //Epsilon case
    err = MU_graph_explore_label_out(l->out,q_bis,forms);
    if (err)
      return err;
  }
  //Explore the label's input (and recursively also the output)
  else  
    if (l->in->unit.type == cst) { //if the current unit is a constant (e.g. "of")
      err = MU_graph_explore_label_out(l->out,q_bis,forms);   //explore the output and the rest of the automaton
      if (err)
	return err;
      int f;  //a single form suffix

      //Concatenate the current constituent in front of each of the form suffixes obtained
      unichar* tmp;
      for (f=0; f<forms->no_forms; f++) {
	tmp = (unichar*) malloc((u_strlen(forms->forms[f].form)+u_strlen(l->in->unit.u.seq)+1) * sizeof(unichar));
	if (!tmp) {
	   fatal_error("Not enough memory in function MU_graph_explore_label\n");
	}
	u_strcpy(tmp,l->in->unit.u.seq);
	u_strcat(tmp,forms->forms[f].form);
	free(forms->forms[f].form);
	forms->forms[f].form = tmp;
      }
    }
  
    else {  //if the current unit is a reference to a lemma's unit (e.g. <$2>)
      SU_id_T* u;  //Referenced lemma's unit
      int u_no;    //Number of the referenced unit
      u_no = l->in->unit.u.num-1;
      u = MU_lemma->units[u_no];  //Get the referenced lemma's unit
      //explore the current unit according to its morphology, then the label's output and the rest of the automaton
      err = MU_graph_explore_label_in_var(u,l->in->morpho,l->out,q_bis,forms);  
      if (err)
	return err;
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
*/ //End of testing

////////////////////////////////////////////
// Given the current instantiation of unification variables and of
// inheritance variables, recursively explore the input of current transition label's input, 
// knowing that it is a reference to the MU lemma's unit 'u' and its
// desired morphology is 'l_in_morpho'.
// Then explore the label's output 'l_out_morpho'.
// Then explore the subautomaton starting from its arrival state 'q_bis'.
// Generate suffixes of the inflected
// forms and their features, and put them to 'forms'.
// Initially, '*forms' has its space allocated but is empty.
// In case of error put an empty list into 'forms'. 
// Return a number !=0 in case of errors, 0 otherwise. 
int MU_graph_explore_label_in_var(SU_id_T* u,MU_graph_morpho_T* l_in_morpho, MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms) {
  int err;
  f_morpho_T feat;   //A set of the already treated morphological features contained in the label's input
  feat.no_cats = 0;
  err = MU_graph_explore_label_in_var_rec(u, l_in_morpho, 0, &feat, l_out_morpho, q_bis, forms);
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
int MU_graph_explore_label_in_var_rec(SU_id_T* u, MU_graph_morpho_T* l_in_morpho, int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms) {
  int err;

  //If no morphology in the input label, take the unit as it appears in the MWU's lemma
  if (!l_in_morpho) {
    //Explore the rest of the automaton
    err = MU_graph_explore_label_out(l_out_morpho,q_bis,forms);  
    if (err)
      return err;
    //Attach the current unit to each suffix
    int mf;  //Index of a single suffix generated by the rest of the automaton
    unichar* tmp;
    for (mf=0; mf<forms->no_forms;mf++) {
      tmp = (unichar*) malloc((u_strlen(u->form)+u_strlen(forms->forms[mf].form)+1) * sizeof(unichar));
      if (!tmp) {
         fatal_error("Not enough memory in function MU_graph_explore_label_in_var_rec\n");
      }
      u_strcpy(tmp,u->form);
      u_strcat(tmp,forms->forms[mf].form);
      free(forms->forms[mf].form);
      forms->forms[mf].form = tmp;
    }
  }

  //If the whole label's input treated
  else
    if (i_morpho == l_in_morpho->no_cats) {
      //Inflected forms of the unit concerned
      SU_forms_T* SU_forms;  
      SU_forms = (SU_forms_T*) malloc(sizeof(SU_forms_T));
      if (!SU_forms) {
	      fatal_error("Not enough memory in function MU_graph_explore_label_in_var_rec\n");
      }
      SU_forms->no_forms = 0;
      SU_forms->forms = NULL;
      //Suffixes of the MWU forms
      MU_forms_T* forms_bis;  
      forms_bis = (MU_forms_T*) malloc(sizeof(MU_forms_T));
      if (!forms_bis) {
	      fatal_error("Not enough memory in function MU_graph_explore_label_in_var_rec\n");
      }
      forms_bis->no_forms = 0;
      forms_bis->forms = NULL;
      
      err = MU_graph_explore_label_out(l_out_morpho,q_bis,forms_bis);  //Explore the rest of the automaton
      if (err)
	return err;
      //Inflect the current unit only if any suffix has been found
      if (forms_bis->no_forms) {
	err = MU_graph_get_unit_forms(u,feat,SU_forms);  //Inflect the unit concerned according to desired features
	if (err)
	  return err;
	int sf;  //Index of a single form of the current unit
	int mf_bis;  //Index of a single suffix generated by the rest of the automaton
	int mf;  //Index of a suffix concatenated with the current unit
	//Concatenate each inflected form of the current unit in front of each suffix
	mf = 0;
	if (forms_bis->no_forms && SU_forms->no_forms) {  //Check if there is anything to concatenante
	  forms->forms = (MU_f_T*) malloc((forms_bis->no_forms*SU_forms->no_forms) * sizeof(MU_f_T));
	  if (!forms->forms) {
	     fatal_error("Not enough memory in function MU_graph_explore_label_in_var_rec\n");
	  }
	}
	for (sf=0; sf<SU_forms->no_forms; sf++)
	  for (mf_bis=0; mf_bis<forms_bis->no_forms;mf_bis++) {
	    forms->forms[mf].form = 
	      (unichar*) malloc((u_strlen(forms_bis->forms[mf_bis].form)+u_strlen(SU_forms->forms[sf].form)+1) * sizeof(unichar));
	    if (!forms->forms[mf].form) {
	       fatal_error("Not enough memory in function MU_graph_explore_label_in_var_rec\n");
	    }
	    //Concatenate the forms
	    u_strcpy(forms->forms[mf].form, SU_forms->forms[sf].form);
	    u_strcat(forms->forms[mf].form, forms_bis->forms[mf_bis].form);
	    //Copy the features
	    forms->forms[mf].features = (f_morpho_T*) malloc(sizeof(f_morpho_T));
	    if (! forms->forms[mf].features) {
	       fatal_error("Not enough memory in function MU_graph_explore_label_in_var_rec\n");
	    }
	    forms->forms[mf].features->no_cats = forms_bis->forms[mf_bis].features->no_cats;
	    for (int c=0; c<forms_bis->forms[mf_bis].features->no_cats; c++)
	      forms->forms[mf].features->cats[c] = forms_bis->forms[mf_bis].features->cats[c];
	    mf++;
	  }
	forms->no_forms = mf;
	SU_delete_inflection(SU_forms);
      }
      MU_delete_inflection(forms_bis);
    }
  
  //If the whole label's input not yet treated
    else {
      MU_graph_category_T* c;  // a single graph category-value pair, e.g. Case==$n1
      c = &(l_in_morpho->cats[i_morpho]);
      switch (c->type) {
      case cnst : //e.g. Nb=pl
	err = MU_graph_explore_label_in_morph_const(c,u,l_in_morpho,i_morpho+1,feat,l_out_morpho,q_bis,forms); 
	break;
      case inherit_var: //e.g. Gen==$g
	err = MU_graph_explore_label_in_morph_inher(c,u,l_in_morpho,i_morpho+1,feat,l_out_morpho,q_bis,forms); 
	break;
      case unif_var:  //e.g. Case=$c1
	err = MU_graph_explore_label_in_morph_unif(c,u,l_in_morpho,i_morpho+1,feat,l_out_morpho,q_bis,forms); 
	break;
      }
      if (err)
	return err;
    }														   
  return 0;
}

/////////////////////////////////////////////////
// Generate the desired inflected forms (defined by 'feat')
// of the current single unit 'u'.
// Return the generated forms in 'forms'.
// In case of error put an empty list into 'forms'. 
// Initially, '*forms' has its space allocated but is empty.
// Return a number !=0 in case of errors, 0 otherwise. 
int MU_graph_get_unit_forms(SU_id_T* u,f_morpho_T* feat,SU_forms_T* SU_forms) {
  int err;
  f_morpho_T old_feat;  //Features that the current unit has in the lemma of the MWU

  //Get the features that the unit has in the lemma of the MWU, e.g. <Nb=sing; Gen=fem> for "vif" in "memoire vive"
  SU_cpy_features(&old_feat,u);

  //Change the features to adapt them to the desired features, e.g. if 'feat'=<Nb=pl> then old_feat<-<Nb=pl; Gen=fem>
  err = f_change_morpho(&old_feat,feat); 
  if (err)
    return 1;

  //Generate the desired inflected forms of the single unit
  err = SU_inflect(u,&old_feat, SU_forms);
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
int MU_graph_explore_label_in_morph_const(MU_graph_category_T* c, SU_id_T* u, MU_graph_morpho_T* l_in_morpho, int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms) {
  int err;
  
  //Add the current label's category-value pair to the features of the single unit to be generated
  err = f_add_morpho(feat,c->cat,c->val.value);
  if (err == -1) {
    error(" in graph %s.\n",MU_lemma->paradigm);
    MU_delete_inflection(forms);
    return 1;
  }

  //Explore recursively the rest of the label
  return MU_graph_explore_label_in_var_rec(u,l_in_morpho,i_morpho,feat,l_out_morpho,q_bis,forms);
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
int MU_graph_explore_label_in_morph_inher(MU_graph_category_T* c, SU_id_T* u, MU_graph_morpho_T* l_in_morpho, int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms) {
  int err;
  int new_instant;  //Controls if an instantiation occured in the current instance of the function

  //Get the features that the unit has in the lemma of the MWU, e.g. <Nb=sing; Gen=fem; Case=Nom>
  f_morpho_T old_feat;  //Features that the current unit has in the lemma of the MWU
  err = SU_cpy_features(&old_feat,u);    //e.g. <Nb=sing; Case=$c1; Gen==$g>
  if (err)
    return err;

  //Get the value of the current category (detuermined by 'c') of the unit as it appears in the lemma of the MWU
  int val;  //Value of the current category (index that this value has in the domain os the category)
  val = f_get_value(&old_feat,c->cat);   //e.g. val: fem
  if (val == -1)
    return -1;

  //Instantiate the variable if necessary
  unichar* var;
  var = c->val.inherit_var;  //get the identifier of the variable, e.g. g1
  if (unif_instantiated(var)) {
    //If the same variable already instantiated to a DIFFERENT value then cut off the exploration path 
    //The 'forms' remain empty list as they were (which is not equivalent to a list containing (epsilon,empty_set)
    if ( (unif_get_cat(var)!=c->cat) || (unif_get_val_index(var)!=val))
      return 0; 
    //If variable already instantiated to the same value, no further instantiation needed
  }
  else {  //Variable not yet instantiated
    err = unif_instantiate_index(var,c->cat,val);
    if (err)
      return err;
    new_instant = 1;
  }

  //Add the the instantiated category-value pair to the features of the single unit to be generated
  feat->cats[feat->no_cats].cat = c->cat;   //e.g. 'feat' devient <Nb=pl; Case=Nom; Gen=fem>
  feat->cats[feat->no_cats].val = val;
  feat->no_cats++;

  //Explore recursively the rest of the label
  err = MU_graph_explore_label_in_var_rec(u,l_in_morpho,i_morpho,feat,l_out_morpho,q_bis,forms);

  //Desinstantiate the variable only if it has been instantiated by the current category-value pair 'c'
  if (new_instant)
    unif_desinstantiate(var);
  
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
int MU_graph_explore_label_in_morph_unif(MU_graph_category_T* c, SU_id_T* u, MU_graph_morpho_T* l_in_morpho, int i_morpho, f_morpho_T* feat, MU_graph_morpho_T* l_out_morpho, Fst2State q_bis, MU_forms_T* forms) {
  int err;  //Result of a function called 
  
  unichar* var;  //Unification variable's identifier
  var = c->val.unif_var;

  //Get the features that the unit has in the lemma of the MWU, e.g. <Nb=sing; Gen=fem; Case=Nom>
  //  f_morpho_T old_feat;  //Features that the current unit has in the lemma of the MWU
  //  err = SU_cpy_features(&old_feat,u);    //e.g. <Nb=sing; Case=$c1; Gen==$g>
  //  if (err)
  //    return err;

  if (unif_instantiated(var)) {
    //If the same variable already instantiated for a DIFFERENT category then cut off the exploration path 
    //The 'forms' remain empty list as they were (which is not equivalent to a list containing (epsilon,empty_set)
    if (unif_get_cat(var)!=c->cat)
      return 0; 
   
    //If the same variable already instantiated for the same category, only this instantiation is taken into account
    //Add the the instantiated category-value pair to the features of the single unit to be generated
    err = f_add_morpho(feat,c->cat,unif_get_val_index(var));
    if (err == -1) {
      error(" in graph %s.\n",MU_lemma->paradigm);
      MU_delete_inflection(forms);
      return 1;
    }    
    return MU_graph_explore_label_in_var_rec(u,l_in_morpho,i_morpho,feat,l_out_morpho,q_bis,forms);
  }

  else {//If the variable not yet instantiated
    int val;  //Index of different values in the domain of the current category
    MU_forms_T forms_bis; //Suffixes generated in one run of the for-loop
    int mf;  //index of a sigle MU form
    for (val=0; val<c->cat->no_values; val++) {
      
      //Instantiated to the current value
      unif_instantiate_index(var,c->cat,val);

      //Add the the instantiated category-value pair to the features of the single unit to be generated
      feat->cats[feat->no_cats].cat = c->cat;   //e.g. 'feat' devient <Nb=pl; Case=Nom; Gen=fem>
      feat->cats[feat->no_cats].val = val;
      feat->no_cats++;

      //Explore the rest of the label and the rest of the automaton 
      forms_bis.no_forms = 0;
      forms_bis.forms = NULL;
      err = MU_graph_explore_label_in_var_rec(u,l_in_morpho,i_morpho,feat,l_out_morpho,q_bis,&forms_bis);
      if (err)
	return err;
      if (forms_bis.no_forms) {  //If any suffix found, add it to the list of those generated previously
	forms->forms = (MU_f_T*)realloc(forms->forms,sizeof(MU_f_T)*(forms->no_forms+forms_bis.no_forms));
	if (! forms->forms) {
	   fatal_error("Not enough memory in function MU_graph_explore_label_in_morph_unif\n");
	}
	for (mf=0; mf<forms_bis.no_forms; mf++) 
	  forms->forms[forms->no_forms+mf] = forms_bis.forms[mf];
	forms->no_forms += forms_bis.no_forms;
      }

      //Delete the current category-value pair
      feat->no_cats--;
      
      //Delete the current instantiation
      unif_desinstantiate(var);
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
int MU_graph_explore_label_out(MU_graph_morpho_T* l_out_morpho,Fst2State q_bis, MU_forms_T* forms) { 
  int err;
  f_morpho_T feat;   //A set of the amready treated morphological features contained in the label's input
  feat.no_cats = 0;
  err = MU_graph_explore_label_out_rec(l_out_morpho,0,&feat,q_bis,forms);
  return(err);
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
int MU_graph_explore_label_out_rec(MU_graph_morpho_T* l_out_morpho,int i_morpho,f_morpho_T* feat, Fst2State q_bis, MU_forms_T* forms) { 
  int err;

  //If no morphology in the output label, then go to the next state
  if (!l_out_morpho) {
    //Explore the arrival state
    err = MU_graph_explore_state(q_bis,forms);  
    if (err)
      return err;
  }
  else  //Label's input not empty

    //If the whole output morphology has been treated
    if (i_morpho == l_out_morpho->no_cats) {

      //Explore the arrival state
      err = MU_graph_explore_state(q_bis,forms);  
      if (err)
	return err;
      
      //Add the current features to the features of the suffixes
      int f;  //Index of the current suffix
      int c;  //Index of the current category-value pair in 'feat' 
      for (f=0; f<forms->no_forms; f++) 
	for (c=0; c<feat->no_cats; c++) { 
	  //If the ouput morphology in a graph is ambiguous, the final MWU's morphology is undefined
	  err = f_add_morpho(forms->forms[f].features, feat->cats[c].cat,feat->cats[c].val);
	  if (err == -1) {
	    error(" in graph %s.\n",MU_lemma->paradigm);
	    MU_delete_inflection(forms);
	    return 1;
	  }
	}
    }
  
  //If the whole label's output not yet treated
    else {
      MU_graph_category_T* c;  // a single graph category-value pair, e.g. Case==$n1
      c = &(l_out_morpho->cats[i_morpho]);
      switch (c->type) {
      case cnst : //e.g. Nb=pl
	err = MU_graph_explore_label_out_morph_const(c,l_out_morpho,i_morpho+1,feat,q_bis,forms); 
	break;
      case unif_var:  //e.g. Case=$c1
	err = MU_graph_explore_label_out_morph_unif(c,l_out_morpho,i_morpho+1,feat,q_bis,forms); 
	break;
   default:;
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
int MU_graph_explore_label_out_morph_const(MU_graph_category_T* c, MU_graph_morpho_T* l_out_morpho, int i_morpho, f_morpho_T* feat, Fst2State q_bis, MU_forms_T* forms) {
  int err;
  //Add the current label's category-value pair to the features of the single unit to be generated
  err = f_add_morpho(feat,c->cat,c->val.value);
  if (err == -1) {
    error(" in graph %s.\n",MU_lemma->paradigm);
    MU_delete_inflection(forms);
    return 1;
  }
  //Explore recursively the rest of the label
  return MU_graph_explore_label_out_rec(l_out_morpho,i_morpho,feat,q_bis,forms);
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
int MU_graph_explore_label_out_morph_unif(MU_graph_category_T* c, MU_graph_morpho_T* l_out_morpho, int i_morpho, f_morpho_T* feat, Fst2State q_bis, MU_forms_T* forms) {
  int err;  //Result of a function called 
  
  unichar* var;  //Unification variable's identifier
  var = c->val.unif_var;

  if (unif_instantiated(var)) {
    //If the same variable already instantiated for a DIFFERENT category then cut off the exploration path 
    //The 'forms' remain empty list as they were (which is not equivalent to a list containing (epsilon,empty_set)
    if (unif_get_cat(var)!=c->cat)
      return 0; 
   
    //If the same variable already instantiated for the same category, only this instantiation is taken into account
    //Add the the instantiated category-value pair to the features of the single unit to be generated
    err = f_add_morpho(feat,c->cat,unif_get_val_index(var));
    if (err == -1) {
      error(" in graph %s.\n",MU_lemma->paradigm);
      MU_delete_inflection(forms);
      return 1;
    }
    return MU_graph_explore_label_out_rec(l_out_morpho,i_morpho,feat,q_bis,forms);
  }

  else {//If the variable not yet instantiated
    int val;  //Index of different values in the domain of the current category
    MU_forms_T forms_bis; //Suffixes generated in one run of the for-loop
    int mf;  //index of a sigle MU form
    for (val=0; val<c->cat->no_values; val++) {
      
      //Instantiated to the current value
      unif_instantiate_index(var,c->cat,val);

      //Add the the instantiated category-value pair to the features of the single unit to be generated
      err = f_add_morpho(feat,c->cat,val);   //e.g. 'feat' devient <Nb=pl; Case=Nom; Gen=fem>
      if (err == -1) {
         error(" in graph %s.\n",MU_lemma->paradigm);
	      MU_delete_inflection(forms);
         return 1;
      }

      //Explore the rest of the label and the rest of the automaton 
      forms_bis.no_forms = 0;
      forms_bis.forms = NULL;
      err = MU_graph_explore_label_out_rec(l_out_morpho,i_morpho,feat,q_bis,&forms_bis);
      if (err)
	return err;
      if (forms_bis.no_forms) {  //If any suffix found, add it to the list of those generated previously
	forms->forms = (MU_f_T*)realloc(forms->forms,sizeof(MU_f_T)*(forms->no_forms+forms_bis.no_forms));
	if (! forms->forms) {
	  fatal_error("Not enough memory in function MU_graph_explore_label_out_morph_unif\n");
	}
	for (mf=0; mf<forms_bis.no_forms; mf++) 
	  forms->forms[forms->no_forms+mf] = forms_bis.forms[mf];
	forms->no_forms += forms_bis.no_forms;
      }

      //Delete the current category-value pair
      feat->no_cats--;
      
      //Delete the current instantiation
      unif_desinstantiate(var);
    }
  }
  return 0;
}

/////////////////////////////////////////////////
// Creates a MU_graph label from two strings.    
// We suppose that MU_label already has its memory allocated.
// Returns 0 on success, 1 otherwise.
int MU_graph_scan_label(unichar* label_in, unichar* label_out, MU_graph_label_T* MU_label) {
  int err1, err2;
  err1 = err2 =0;

  //Input label
  /*
  printf("label_in = ");  //debug
  u_prints(label_in);//debug
  printf("\n");//debug
  */
  if (!u_strcmp_char(label_in,"<E>"))  //Epsilon case
    MU_label->in = NULL;
  else {
    MU_label->in = (MU_graph_in_T*)malloc(sizeof(MU_graph_in_T));
    err1 = MU_graph_scan_label_in(label_in, MU_label->in);
  }

  //Output label
  /*
  printf("label_out = ");  //debug
  u_prints(label_out);//debug
  printf("\n");//debug
  */
  if ((!u_strcmp_char(label_out,"<E>")) || (!u_strlen(label_out)) ) //Case of epsilon or void output
    MU_label->out = NULL;
  else {
    MU_label->out = (MU_graph_out_T*)malloc(sizeof(MU_graph_out_T));
    err2 = MU_graph_scan_label_out(label_out, MU_label->out);
  }
  
  return (err1 or err2);
}


/////////////////////////////////////////////////
// Creates a MU_graph label's input from a string (e.g. "of" or "<$1[Gen==$g;Nb=$n;Case=Inst]>"  
// We suppose that MU_label_in already has its memory allocated.
// Returns 0 on success, 1 otherwise.
int MU_graph_scan_label_in(unichar* label, MU_graph_in_T* MU_label_in) {
  int l; //length of a scanned sequence
  unichar* pos; //current position in label
  unichar tmp[MAX_GRAPH_NODE];
  int err=0;
  
  pos = label;

  //Omit void characters
  pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE-1," \t");

  //Constant unit, e.g. "of"
  if (*pos != (unichar)'<') {
    l = u_scan_until_char(tmp,pos,MAX_GRAPH_NODE-1,"",1);
    MU_label_in->unit.type = cst;
    MU_label_in->unit.u.seq = (unichar*) malloc((u_strlen(tmp)+1) * sizeof(unichar));
    u_strcpy(MU_label_in->unit.u.seq,tmp);    
    MU_label_in->morpho = NULL;
    pos = pos + l;
  }
  else { //Variable unit, e.g. <$2:Gen==$g>
    pos++;  //Omit the '<'
    MU_label_in->unit.type = var;
    if (*pos != (unichar)'$') {
      error("In graph %s label format incorrect in ",MU_lemma->paradigm);
      error(label);
      error(" (at position %d): ",(int)(pos-label));
      error(" a '$' missing after '<'.\n");
      return 1;      
    }
    pos++;  //Omit the '$'
    l = u_scan_while_char(tmp, pos, MAX_GRAPH_NODE-1,"0123456789");
    if (!l) {
      error("In graph %s label format incorrect in ",MU_lemma->paradigm);
      error(label);
      error(" (at position %d): ",(int)(pos-label));
      error(" unit number missing after \'$\'.\n");
      return 1;
    }
    char tmp_char[MAX_GRAPH_NODE];
    u_to_char(tmp_char,tmp);
    MU_label_in->unit.u.num = atoi(tmp_char);
    pos = pos + l;

    //Omit void characters
    pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE-1," \t");
    
    //A ':' or a '>' must follow
    if ((*pos != (unichar) ':') && (*pos != (unichar) '>')) {
      error("In graph %s label format incorrect in ",MU_lemma->paradigm);
      error("Graph label format incorrect in ");
      error(label);
      error(" (at position %d): ",(int)(pos-label));
      error("':' or '>' missing.\n");
      return 1;
    }

    //Check if morphology follows
    if (*pos == (unichar) ':') {
      pos++;
      MU_label_in->morpho = (MU_graph_morpho_T*) malloc(sizeof(MU_graph_morpho_T));
      unichar tmp1[MAX_GRAPH_NODE];
      l = u_scan_until_char(tmp1,pos,MAX_GRAPH_NODE-1,">",1);
      err = MU_graph_scan_graph_morpho(tmp1, MU_label_in->morpho);
      pos = pos + l;
    }
    else
      MU_label_in->morpho = NULL;
    //Closing '>'
    if (*pos !=  (unichar)'>') {
      error("In graph %s label format incorrect in ",MU_lemma->paradigm);
      error(label);
      error(" (at position %d): ",(int)(pos-label));
      error("'>' missing.\n");
      return 1;
    }
    pos++;
  }

  //Omit void characters
  pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE-1," \t");

  if (*pos != 0) {
    error("In graph %s label format incorrect in ",MU_lemma->paradigm);
    error(label);
    error(" (at position %d): ",(int)(pos-label));
    error(" end of label expected.\n");
    return 1;      
  }
  return err;
}

/////////////////////////////////////////////////
// Creates a MU_graph label's output structure from a string.    
// We suppose that MU_label_out already has its memory allocated.
// Returns 0 on success, 1 otherwise.
int MU_graph_scan_label_out(unichar* label, MU_graph_out_T* MU_label_out) {
  int cv;  //number of the current category-value pair
  int err;
  int l; //length of a scanned sequence
  unichar* pos; //current position in label

  pos = label;

  //Opening '<'
  if (*pos != (unichar) '<') {
    error("In graph %s label format incorrect in ",MU_lemma->paradigm);
    error(label);
    error(" (at position %d): ",(int)(pos-label));
    error(" '<'  expected.\n");
    return 1;      
  }
  pos++;
  unichar tmp[MAX_GRAPH_NODE];
  l = u_scan_until_char(tmp,pos,MAX_GRAPH_NODE-1,">",1);
  err = MU_graph_scan_graph_morpho(tmp, MU_label_out);
  pos = pos + l;
  if (!err)
    for (cv=0; cv<MU_label_out->no_cats; cv++)
      if (MU_label_out->cats[cv].type == inherit_var) {
	error("In graph %s label format incorrect in ",MU_lemma->paradigm);
	error(label);
	error(": an output label may not contain a double assignment \'==\'.\n");
	return 1;
      }
  //Closing '>'
  if (*pos != (unichar) '>') {
    error("In graph %s label format incorrect in ",MU_lemma->paradigm);
    error(label);
    error(" (at position %d): ",(int)(pos-label));
    error(" '>'  expected.\n");
    return 1;      
  }
  return 0;
}

/////////////////////////////////////////////////
// Creates a MU_graph morpho structure from a string.    
// We suppose that MU_graph_morpho already has its memory allocated.
// Returns 0 on success, 1 otherwise.
int MU_graph_scan_graph_morpho(unichar* label, MU_graph_morpho_T* MU_graph_morpho) {
  int l; //length of a scanned sequence
  unichar* pos; //current position in label
  unichar tmp[MAX_GRAPH_NODE];
  int cv;  //number of the current category-value pair
  l_category_T* cat;
  int val;  //Index of a value in the domain of a category.
  int dbl_eq; //Checks if a double equal sign has appeared
  
  pos = label;

  //Omit void characters
  pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE-1," \t");

  //Category-value pairs
  int done = 0;
  cv = 0;
  while (!done) {
    //Omit void characters
    pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE-1," \t");
    
    //Category, e.g. Nb
    l = u_scan_until_char(tmp,pos,MAX_GRAPH_NODE-1," \t=",1);
    cat = is_valid_cat(tmp);
    if (!cat) { 
      error("In graph %s label format incorrect in ",MU_lemma->paradigm);
      error(label);
      error(" (at position %d): ",(int)(pos-label));
      error(tmp);
      error(" is not a valid category\n");
      return 1;
    }
    MU_graph_morpho->cats[cv].cat = cat;
    pos = pos+l;
    
    //Omit void characters
    pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE-1," \t");
    
    //The '=' character
    if (*pos != (unichar) '=') {
      error("In graph %s label format incorrect in ",MU_lemma->paradigm);
      error(label);
      error(" (at position %d): ",(int)(pos-label));
      error("\'=\' missing.\n");
      return 1;
    }
    pos++;
    
    //The double '==' if any
    if (*pos == (unichar) '=') {
      dbl_eq = 1;
      pos++;
    }
    else 
      dbl_eq = 0;

    //Omit void characters
    pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE-1," \t"); 
    
    //Value e.g. gen, $n1
    //Variable
    if (*pos == (unichar) '$') {
      if (dbl_eq)
	MU_graph_morpho->cats[cv].type = inherit_var;
      else
	MU_graph_morpho->cats[cv].type = unif_var;
      pos++;  //omit the '$'
      l = u_scan_until_char(tmp,pos,MAX_GRAPH_NODE-1," \t;>",1);
      if (!l) {
	error("In graph %s label format incorrect in ",MU_lemma->paradigm);
	error(label);
	error(" (at position %d): ",(int)(pos-label));
	error("a variable missing after \'$\'.\n");
	return 1;      
      }
      MU_graph_morpho->cats[cv].val.unif_var = (unichar*) malloc((u_strlen(tmp)+1) * sizeof(unichar));
      u_strcpy(MU_graph_morpho->cats[cv].val.unif_var,tmp);
    }
    else {  //constant value, e.g. fem
      if (dbl_eq) {
	error("In graph %s label format incorrect in ",MU_lemma->paradigm);
	error(label);
	error(" (at position %d): ",(int)(pos-label));
	error("a variable missing after \'==\'.\n");
	return 1;      
      }
      MU_graph_morpho->cats[cv].type = cnst;
      l = u_scan_until_char(tmp,pos,MAX_GRAPH_NODE-1," \t;>",1);
      if (!l) {
	error("In graph %s label format incorrect in ",MU_lemma->paradigm);
	error(label);
	error(" (at position %d): ",(int)(pos-label));
	error(tmp);
	error("a value missing after \'=\'.\n");
	return 1;      
      }
      val = is_valid_val(cat,tmp);
      if (val == -1) {
	error("In graph %s label format incorrect in ",MU_lemma->paradigm);
	error(label);
	error(" (at position %d): ",(int)(pos-label));
	error(tmp);
	error(" is not a valid value in the domain of ");
	error(cat->name);
	error("\n");
	return 1;            
      }
      MU_graph_morpho->cats[cv].val.value = val;
    }
    pos = pos + l;  //go to the end of the current value
    
    //New category-value pair read in
    cv++;

    //Omit void characters
    pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE-1," \t"); 
    
    //See if end of category-value pairs
    if ((*pos == (unichar)'>') || (*pos == (unichar)'\0'))
      done = 1;
    else { 
      if (*pos != (unichar)';') {
	error("In graph %s label format incorrect in ",MU_lemma->paradigm);
	error(label);
	error(" (at position %d): ",(int)(pos-label));
	error(tmp);
	error(" ';' missing\n");
	return 1;      
      }
      pos++; //Omit the ';'
      }
  }
  MU_graph_morpho->no_cats = cv;
  
  //Omit void characters
  pos = pos + u_scan_while_char(tmp, pos, MAX_GRAPH_NODE-1," \t"); 
  
  return 0;
}

/////////////////////////////////////////////////
// Prints a MU_graph label.    
void MU_graph_print_label(MU_graph_label_T* MU_label) {
  unichar tmp[50];

  //Label's input
  if (! MU_label->in) {  //Epsilon case
      u_strcpy_char(tmp,"<E>");
      u_prints(tmp);
  }
  else {
    if (MU_label->in->unit.type == cst)
      u_prints(MU_label->in->unit.u.seq);
    else {
      u_strcpy_char(tmp,"<$");
      u_prints(tmp);
      char tmp_char[50];
      sprintf(tmp_char,"%d",MU_label->in->unit.u.num);
      u_strcpy_char(tmp,tmp_char);
      u_prints(tmp);
      if (MU_label->in->morpho) {
	//Opening ':'
	u_strcpy_char(tmp,":");
	u_prints(tmp);
	MU_graph_print_morpho(MU_label->in->morpho);
      }
      u_strcpy_char(tmp,">");
      u_prints(tmp);
    }
  }
  
  //Separating '/'
  u_strcpy_char(tmp,"/");
  u_prints(tmp);

  //Label's output
  if (! MU_label->out) {  //Epsilon case
    u_strcpy_char(tmp,"<E>");
    u_prints(tmp);
  }
  else {
    u_strcpy_char(tmp,"<");
    u_prints(tmp);
    MU_graph_print_morpho(MU_label->out);
    u_strcpy_char(tmp,">");
    u_prints(tmp);
  }
  
  //Newline 
  u_strcpy_char(tmp,"\n");
  u_prints(tmp);
}

/////////////////////////////////////////////////
// Prints a MU_graph morpho.    
void MU_graph_print_morpho(MU_graph_morpho_T* MU_morpho) {
  int cv;  //number of the current category-value pair
  unichar tmp[3];

  //Category-value features
  for (cv=0; cv<MU_morpho->no_cats; cv++) {

    //Category
    u_prints(MU_morpho->cats[cv].cat->name);

    //Equality
    u_strcpy_char(tmp,"=");
    u_prints(tmp);
    if (MU_morpho->cats[cv].type == inherit_var) {
      u_strcpy_char(tmp,"=");   //Double '=='
      u_prints(tmp);
    }
    
    //Value
    if (MU_morpho->cats[cv].type == inherit_var) {  //inherit_var
       u_strcpy_char(tmp,"$");
       u_prints(tmp);
       u_prints(MU_morpho->cats[cv].val.inherit_var);
    }
    else
      if (MU_morpho->cats[cv].type == unif_var)  {  //unif_var
	u_strcpy_char(tmp,"$");
	u_prints(tmp);
	u_prints(MU_morpho->cats[cv].val.unif_var);
      }
      else {                                      //constant
	int val = MU_morpho->cats[cv].val.value;
	u_prints(MU_morpho->cats[cv].cat->values[val]);
      }

    //Semi-colon
    if (cv<MU_morpho->no_cats-1) {
      u_strcpy_char(tmp,";");
      u_prints(tmp);
    }
  }
}
/////////////////////////////////////////////////
// Liberates the memory allocated for a MU_graph label.    
void MU_graph_free_label(MU_graph_label_T* MU_label) {
  //Free the label's input
  if (MU_label->in) {
    if (MU_label->in->unit.type == cst)
      free(MU_label->in->unit.u.seq);
    if (MU_label->in->morpho)
      MU_graph_free_morpho(MU_label->in->morpho);
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
  int cv;  //number of the current category-value pair
  for (cv=0; cv<MU_morpho->no_cats; cv++)
    if (MU_morpho->cats[cv].type == unif_var)
      free(MU_morpho->cats[cv].val.unif_var);
    else
      if (MU_morpho->cats[cv].type == inherit_var)
	free(MU_morpho->cats[cv].val.inherit_var);
}

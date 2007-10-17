/*
  * Unitex 
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
/********************************************************************************/

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

#define MAX_CHARS_IN_STACK 4096

//////////////////////////////
//Description of all classes in the current language 
extern l_classes_T L_CLASSES; 

//////////////////////////////
// Table of inflection tranducers
extern Fst2* fst2[N_FST2];

/**
 * This structure represents a list of inflection information. It is used
 * to get up information from a subgraph exploration.
 */
struct inflect_infos {
   unichar* inflected;
   unichar* output;
   unichar* semitic;
   struct inflect_infos* next;
};

//////////////////////////////
int SU_inflect(SU_id_T* SU_id,f_morpho_T* desired_features, SU_forms_T* forms);
int SU_explore_state(unichar* flechi,unichar* canonique,unichar* sortie,
		               Fst2* a,int etat_courant, f_morpho_T* desired_features, SU_forms_T* forms,unichar*);
int SU_explore_state_recursion(unichar* flechi,unichar* canonique,unichar* sortie,
                   Fst2* a,int etat_courant,struct inflect_infos** L,
		             f_morpho_T* desired_features, SU_forms_T* forms,unichar*);
void SU_explore_tag(Transition* T,unichar* flechi,unichar* canonique,unichar* sortie,
			           Fst2* a,struct inflect_infos** LISTE,f_morpho_T* desired_features, SU_forms_T* forms,unichar*);
void shift_stack(unichar* stack,int pos);
void shift_stack_left(unichar* stack,int pos);
int SU_convert_features(f_morpho_T*** feat,unichar* feat_str);
struct list_ustring* SU_split_raw_features(unichar*);
int SU_feature_agreement(f_morpho_T* feat,f_morpho_T* desired_features);
int SU_delete_inflection(SU_forms_T* forms);
int SU_cpy_features(f_morpho_T* feat,SU_id_T* SU_id);
void SU_delete_features(f_morpho_T* f);
SU_id_T*  SU_get_id(SU_f_T* SU_form, SU_lemma_T* SU_lemma);
int SU_delete_id(SU_id_T* id);
int SU_print_f(SU_f_T* f);
int SU_print_forms(SU_forms_T* F);
int SU_print_lemma(SU_lemma_T* l);
int SU_init_lemma(SU_lemma_T* l, char* word, char* cl, char* para);
int SU_delete_lemma(SU_lemma_T* l);


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
// semitic: if not NULL, it means that we are inflecting a semitic word. In that case,
//          the stack is not initialized with the lemma but with the empty string. The lemma
//          is supposed to represent a consonantic skeleton like "ktb". The inflection fst2's paths
//          contains references to the consonants of this skeleton in the form of number from 1 to n
//          For instance, if we have the path "li1a2u3na", we will have the inflected form
//          "likatubna"
//
// Returns 0 on success, 1 otherwise.   
int SU_inflect(SU_id_T* SU_id,f_morpho_T* desired_features, SU_forms_T* forms,int semitic) {
  int err;
  unichar inflected[MAX_CHARS_IN_STACK];
  unichar inflection_codes[MAX_CHARS_IN_STACK];
  unichar semitic_[MAX_CHARS_IN_STACK];
  inflection_codes[0]='\0';
  semitic_[0]='\0';
  int T=get_transducer(SU_id->lemma->paradigm);
  if (fst2[T]==NULL) {
    // if the automaton has not been loaded
    return 1;
  }
  u_strcpy(inflected,semitic?U_EMPTY:SU_id->lemma->unit);
  err = SU_explore_state(inflected,SU_id->lemma->unit,inflection_codes,fst2[T],0,desired_features,forms,semitic?semitic_:NULL);
  return err;
}


/**
 * This function inflects a simple word. 'lemma' is the lemma as found in
 * the DELAS, 'inflection_code' is the name of the inflection transducer without
 * extension (ex: N43). 'forms' is a structure (supposed to be allocated) that 
 * will receive all the produced inflected forms with their inflectional features.
 * The output DELAF lines will have to be built from 'forms'.
 */
int SU_inflect(unichar* lemma,char* inflection_code, SU_forms_T* forms,int semitic) {
  int err;
  unichar inflected[MAX_CHARS_IN_STACK];
  unichar inflection_codes[MAX_CHARS_IN_STACK];
  unichar semitic_[MAX_CHARS_IN_STACK];
  inflection_codes[0]='\0';
  semitic_[0]='\0';
  int T=get_transducer(inflection_code);
  if (fst2[T]==NULL) {
    // if the automaton has not been loaded
    return 1;
  }
  u_strcpy(inflected,semitic?U_EMPTY:lemma);
  err=SU_explore_state(inflected,lemma,inflection_codes,fst2[T],0,NULL,forms,semitic?semitic_:NULL);
  return err;
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
int SU_explore_state(unichar* flechi,unichar* canonique,unichar* sortie,
                   Fst2* a,int etat_courant, f_morpho_T* desired_features, SU_forms_T* forms,
                   unichar* semitic) {
  int err;
  Fst2State e=a->states[etat_courant];
  if (e->control & 1) {  //If final state
    if (desired_features!=NULL) {
       /* If we want to select only some inflected forms */
       f_morpho_T** feat;  //Table of sets of inflection features; necessary in case of factorisation of entries, e.g. :ms:fs
       err=SU_convert_features(&feat,sortie);
       if (err) {
          return (err);
       }
       int f;  //Index of the current morphological features in the current node
       f=0;
       while (feat[f]) {
         //If the form's morphology agrees with the desired features
         if (SU_feature_agreement(feat[f],desired_features)) {
	         //Put the form into 'forms'
	         forms->forms=(SU_f_T*)realloc(forms->forms,(forms->no_forms+1)*sizeof(SU_f_T));
            if (!forms->forms) {
	            fatal_error("Not enough memory in function SU_explore_state\n");
	         }
            forms->forms[forms->no_forms].form=u_strdup(flechi);
            forms->forms[forms->no_forms].features = feat[f];
            forms->no_forms++;
         }
         else { // If undesired form delete 'feat'
	         f_delete_morpho(feat[f]);
	      }
         f++;
       }
       free(feat);
    } else if (semitic!=NULL) {
      /* If we are in semitic mode, we don't try to split the output into
       * several inflection codes */
         forms->forms=(SU_f_T*)realloc(forms->forms,(forms->no_forms+1)*sizeof(SU_f_T));
         if (!forms->forms) {
            fatal_error("Not enough memory in function SU_explore_state\n");
         }
         forms->forms[forms->no_forms].form=u_strdup(flechi);
         forms->forms[forms->no_forms].raw_features=u_strdup(sortie);
         forms->no_forms++;
    } else {
      /* If we want all the inflected forms */
      struct list_ustring* features=SU_split_raw_features(sortie);
      while (features!=NULL) {
         //Put the form into 'forms'
         forms->forms=(SU_f_T*)realloc(forms->forms,(forms->no_forms+1)*sizeof(SU_f_T));
         if (!forms->forms) {
            fatal_error("Not enough memory in function SU_explore_state\n");
         }
         forms->forms[forms->no_forms].form=u_strdup(flechi);
         forms->forms[forms->no_forms].raw_features=features->string;
         forms->no_forms++;
         struct list_ustring* tmp=features->next;
         /* WARNING: here we must not call free_list_ustring, since the associated
          * string would also be freed */
         free(features);
         features=tmp;
       }
    }
  }
  Transition* t=e->transitions;
  while (t!=NULL) {
    SU_explore_tag(t,flechi,canonique,sortie,a,NULL,desired_features,forms,semitic);
    t=t->next;
  }
  return 0;
}


/**
 * Allocates, initializes and returns a new inflect_infos structue.
 */
struct inflect_infos* new_inflect_infos() {
struct inflect_infos* i=(struct inflect_infos*)malloc(sizeof(struct inflect_infos));
if (i==NULL) {
   fatal_error("Not enough memory in new_inflect_infos\n");
}
i->inflected=NULL;
i->output=NULL;
i->semitic=NULL;
i->next=NULL;
return i;
}


/**
 * Frees the given single list cell.
 */
void free_inflect_infos(struct inflect_infos* i) {
if (i==NULL) return;
if (i->inflected!=NULL) free(i->inflected);
if (i->output!=NULL) free(i->output);
if (i->semitic!=NULL) free(i->semitic);
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
int SU_explore_state_recursion(unichar* inflected,unichar* lemma,unichar* output,
                   Fst2* a,int current_state,struct inflect_infos** L,
                   f_morpho_T* desired_features, SU_forms_T* forms,unichar* semitic) {
  Fst2State e=a->states[current_state];
  if (e->control & 1) {
    // if we are in a final state, we save the computed things
    struct inflect_infos* res=new_inflect_infos();
    res->inflected=u_strdup(inflected);
    res->output=u_strdup(output);
    res->semitic=u_strdup(semitic);
    res->next=(*L);
    (*L)=res;
  }
  Transition* t=e->transitions;
  while (t!=NULL) {
    SU_explore_tag(t,inflected,lemma,output,a,L,desired_features,forms,semitic);
    t=t->next;
  }
  return 0;
}

////////////////////////////////////////////
// Explores the tag of the transition T
//
// desired_features: morphology of the desired forms, e.g. {Gen=fem, Case=Inst}, or {} (if separator)
// forms: return parameter; set of the inflected forms corresponding to the given inflection features
//        e.g. (3,{[reka,{Gen=fem,Nb=sing,Case=Instr}],[rekami,{Gen=fem,Nb=pl,Case=Instr}],[rekoma,{Gen=fem,Nb=pl,Case=Instr}]})
//        or   (1,{["-",{}]})
// Returns 0 on success, 1 otherwise.   
void SU_explore_tag(Transition* T,unichar* inflected,unichar* lemma,unichar* output,
                 Fst2* a,struct inflect_infos** LIST,f_morpho_T* desired_features, SU_forms_T* forms,
                 unichar* semitic) {
if (T->tag_number < 0) {
   /* If we are in the case of a call to a sub-graph */
   struct inflect_infos* L=NULL;
   struct inflect_infos* temp;
   SU_explore_state_recursion(inflected,lemma,output,a,a->initial_states[-(T->tag_number)],&L,desired_features,forms,semitic);
   while (L!=NULL) {
      if (LIST==NULL) {
         SU_explore_state(L->inflected,lemma,L->output,a,T->state_number,desired_features,forms,L->semitic);
      }
      else {
         SU_explore_state_recursion(L->inflected,lemma,L->output,a,T->state_number,LIST,desired_features,forms,L->semitic);
      }
      temp=L;
      L=L->next;
      free_inflect_infos(temp);
   }
   return;
}
Fst2Tag t=a->tags[T->tag_number];
int pos=u_strlen(inflected);
unichar out[MAX_CHARS_IN_STACK];
unichar stack[MAX_CHARS_IN_STACK];
unichar tag[MAX_CHARS_IN_STACK];
unichar semitic2[MAX_CHARS_IN_STACK];
u_strcpy(out,output);
if (semitic!=NULL) u_strcpy(semitic2,semitic);
int pos_out=u_strlen(out);
u_strcpy(stack,inflected);
u_strcpy(tag,t->input);
if (u_strcmp(tag,"<E>")) {
   /* If the tag is not <E>, we process it */
   for (int pos_tag=0;tag[pos_tag]!='\0';) {
      switch (tag[pos_tag]) {
         /* Left move operator */
         case 'L':   {
            if (pos!=0) {
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
         
         /* Right copy operator */
         case 'C': {
            shift_stack(stack,pos);
            pos=pos++;
            pos_tag++;
            break;
         }
         
         /* Left copy operator */
         case 'D': {
            shift_stack_left(stack,pos);
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
         case '9': if (semitic!=NULL) {
            int i=tag[pos_tag++]-'1';
            if (i>=u_strlen(lemma)) {
               error("Reference in %S.fst2 to consonant #%C for skeleton \"%S\"\n",a->graph_names[1],tag[pos_tag-1],lemma);
               return;
            }
            stack[pos++]=lemma[i];
            out[pos_out++]=lemma[i];
            break;
         }
      
         /* Default push operator */
         default: {
            stack[pos++]=tag[pos_tag];
            if (semitic!=NULL) {
               out[pos_out++]=tag[pos_tag];
            }
            pos_tag++;
            break;
         }
      }
   }
}
out[pos_out]='\0';
/* We process the output, if any and not NULL */
if (t->output!=NULL && u_strcmp(t->output,"<E>")) {
   if (semitic!=NULL) {
      /* If we are in semitic mode, we append the output to the current one, stored in 'semitic' */
      u_strcat(semitic2,t->output);
      /* Then, we test if the new output contains an unprotected '}' or if it is "{" */
      if (!u_strcmp(semitic2,"{") || u_strchr(semitic2,'}',1)!=NULL) {
         /* In that case, we must append the current output to the global one */
         u_strcat(out,semitic2);
         semitic2[0]='\0';
      }
   } else {
      /* If we are in normal mode, we just append the tag's output to the global one */
      u_strcat(out,t->output);
   }
}
/* Then, we go the next state */
stack[pos]='\0';
if (LIST==NULL) {
   SU_explore_state(stack,lemma,out,a,T->state_number,desired_features,forms,semitic?semitic2:NULL);
} else {
   SU_explore_state_recursion(stack,lemma,out,a,T->state_number,LIST,desired_features,forms,semitic?semitic2:NULL);
}
}

////////////////////////////////////////////
// Shifts right all the stack from the position pos
void shift_stack(unichar* stack,int pos) {
  if (pos==0) {
    // this case should never happen
    return;
  }
  for (int i=(MAX_CHARS_IN_STACK-1);i>=pos;i--) {
    stack[i]=stack[i-1];
  }
}


//
// Shifts all the stack to the left from the position pos
//
void shift_stack_left(unichar* stack,int pos) {
if (pos==0) {
   // this case should never happen
   return;
}
for (int i=pos-1;i<MAX_CHARS_IN_STACK;i++) {
   stack[i]=stack[i+1];
}
}



////////////////////////////////////////////
// Convert a textual representation of features 'feat_str', e.g. :Ipf:Ipm
// into a set of feature structures, e.g. <<Case=I; Nb=p; Gen=f>,<Case=I; Nb=p; Gen=m>>
// Initially 'feat' does not have its space allocated
// Returns 1 on error, 0 otherwise.
int SU_convert_features(f_morpho_T*** feat,unichar* feat_str) {
  unichar tmp[MAX_CATS];  //buffer for a single set of features, e.g. Ipf
  f_morpho_T* f;  //current feature structure
  int l;   //length of a scanned sequence
  int no_f;  //number of feature sets scanned

  (*feat) = (f_morpho_T**) malloc(sizeof(f_morpho_T*));
  no_f = 0;

  while (*feat_str) {
    //Delete the leading ':' if any
    if (feat_str[0] == (unichar) ':')
      feat_str++;
    //Get next feature structures, e.g. Ipf, out of a sequence, e.g. :Ipf:Ipm
    l = u_scan_until_char(tmp,feat_str,MAX_CATS-1,":",1);
    feat_str = feat_str + l;
    //Convert the set of features, e.g. Ipm, into a feature structure, e.g. <Case=I; Nb=p; Gen=m>
    f = d_get_feat_str(tmp);
    if (!f) {
      for (int i=0; i<no_f; i++)  //delete the previously constracted feature structures
	free((*feat)[i]);
      free(*feat);
      return 1;
    }
    no_f++;
    (*feat) = (f_morpho_T**) realloc((*feat), (no_f+1) * sizeof(f_morpho_T*));
    if (!(*feat)) {
      fatal_error("Not enough memory in function SU_convert_features\n");
    }
    (*feat)[no_f-1] = f;
  }
  (*feat)[no_f] = NULL;
  return 0;
}


////////////////////////////////////////////
// Splits a textual representation of features 'feat_str', e.g. :Ipf:Ipm
// into a list of strings "Ipf", "Ipm"
// Returns the list; NULL if an error occurs.
struct list_ustring* SU_split_raw_features(unichar* features) {
struct list_ustring* result=NULL;
if (features==NULL || features[0]=='\0') {
   return NULL;
}
int pos=0;
if (features[0]==':') {
   pos++;
}
unichar feature[1024];
while (P_OK==parse_string(features,&pos,feature,P_COLON)) {
   result=new_list_ustring(feature,result);
   if (features[pos]==':') {
      pos++;
   }
}
return result;
}


////////////////////////////////////////////
// Returns 1 if the set of morphological features 'feat'
// agrees with the set 'desired_features'.
// Otherwise returns 0.
//
// In order to facilitate the processing of simple words, we say
// that 'desired_features'=NULL means that all the inflected forms are
// to keep.
int SU_feature_agreement(f_morpho_T* feat,f_morpho_T* desired_features) {
if (desired_features==NULL) {
   return 1;
}
   
  int f; //Index of the current feature in 'feat'
  int df; //Index of the current feature in 'desired_feat'
  int found; //Has the current feature's category been found in 'desired_features'

  //Each category-value pair of the 'desired_features' has to be present in 'feat'
  for (df=0; df<desired_features->no_cats; df++) {
    found = 0;
    f = 0;
    while ((!found) && (f<feat->no_cats)) {
      if (desired_features->cats[df].cat == feat->cats[f].cat) {
	found = 1;
	//If the same category then the value has to be the same
	if (desired_features->cats[df].val != feat->cats[f].val) {
	return 0;
	}
      }
      f++;
    }
    if (!found) {
      return 0;
    }
  }
  return 1;
}


////////////////////////////////////////////
// Liberates the memory allocated for a set of forms
int SU_delete_inflection(SU_forms_T* forms) {
  int f;
  for (f=0; f<forms->no_forms; f++) {
    free(forms->forms[f].form);
    free(forms->forms[f].features);
  }
  return 0;
  free(forms);
}

////////////////////////////////////////////
// Returns in 'feat' a copy of the inflection features of the given form.
// Initially 'feat' has its space allocated but is empty.
// Returns 1 on error, 0 otherwise.
int SU_cpy_features(f_morpho_T* feat,SU_id_T* SU_id) {
  int c;

  if (!SU_id || !SU_id->feat)
    return 1;

  for (c=0; c<SU_id->feat->no_cats; c++)
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
SU_id_T*  SU_get_id(unichar* form, f_morpho_T* feat, SU_lemma_T* SU_lemma) {
  SU_id_T* SU_id;
  SU_id = (SU_id_T*) malloc(sizeof(SU_id_T));

  //Form
  SU_id->form=u_strdup(form);

  if (SU_lemma) { //if not a separator

    //Lemma
    SU_id->lemma = (SU_lemma_T*) malloc(sizeof(SU_lemma_T));
    if (!SU_id->lemma) {
       fatal_error("Not enough memory in function SU_get_id\n");
    }
    //Lemma form
    SU_id->lemma->unit=u_strdup(SU_lemma->unit);
    //Class
    SU_id->lemma->cl = SU_lemma->cl;
    //Paradigm
    SU_id->lemma->paradigm=strdup(SU_lemma->paradigm);
    //Features
    f_morpho_T* fea;
    fea = (f_morpho_T*) malloc(sizeof(f_morpho_T));
    if (!fea) {
       fatal_error("Not enough memory in function SU_get_id\n");
    }
    f_init_morpho(fea);
    int f;  //index of the current category-value pair in 'feat'
    for (f=0; f<feat->no_cats; f++) {
      f_add_morpho(fea,feat->cats[f].cat,feat->cats[f].val);
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
int SU_get_unit(unichar* unit,unichar* line, int max, Alphabet* alph, int eliminate_bcksl) {
  int l=0;  //length of the scanned sequence
  int u=0;  //index of the next element in 'unit'
  int end=0;
  int no_elim_bcksl=0;  //number of eliminated backslashes
  int bcksl_precedes=0;  //1 if the preceding character was a '\', 0 otherwise
  if (!line)
    return 0;

  //Separator
  if (line[0]==(unichar)'\\' && line[1] && !is_letter(line[1],alph))
    if (eliminate_bcksl) {
      unit[0] = line[1];
      unit[1] = '\0';
      return 2;
    }
    else {
      unit[0] = line[0];
      unit[1] = line[1]; 
      unit[2] = '\0'; 
      return 2;
    }
  else
    if(line[0]!=(unichar)'\\' && line[0] && !is_letter(line[0],alph)) {
      unit[0] = line[0];
      unit[1] = '\0';
      return 1;      
    }
  
  //Word
    else {
      l = 0;
      u = 0;
      end = 0; 
      no_elim_bcksl =0;
      bcksl_precedes = 0;
      while (!end && u<max && line[l]!=(unichar)'\0')
	if (line[l] == (unichar) '\\')   //current character is a '\'
	  if (!bcksl_precedes) {         //not preceded by '\'
	    bcksl_precedes = 1;
	    l++;
	  }
	  else                          //preceded by '\'
	    if (!is_letter(line[l],alph))  //'\' is no letter
	      end = 1;
	    else {                        //\ '\' is a letter
	      if (!eliminate_bcksl)
		unit[u++] = (unichar) '\\';
	      else
		no_elim_bcksl++;
	      unit[u++] = line[l++];
	      bcksl_precedes = 0;
	    }
	else //Current character is not a '\'
	  if (!is_letter(line[l],alph))  //'\' is no letter
	    end = 1;
	  else {                        //\ '\' is a letter
	    if (bcksl_precedes)
	      if (!eliminate_bcksl)
		unit[u++] = (unichar) '\\';
	      else
		no_elim_bcksl++;
	    unit[u++] = line[l++];
	    bcksl_precedes = 0;
	  }
    }
  unit[u] = (unichar) '\0';
  return u+no_elim_bcksl;
}

////////////////////////////////////////////
// Liberates the memory allocated for a form's id.
int SU_delete_id(SU_id_T* id) {
  free(id->form);
  if (id->lemma)
    SU_delete_lemma(id->lemma);
  free(id);
  return 0;
}
////////////////////////////////////////////
// Prints a form and its inflection features.
int SU_print_f(SU_f_T* f) {
u_printf("%S : ",f->form);
f_print_morpho(f->features);
return 0;
}

////////////////////////////////////////////
// Prints a set of forms and their inflection features.
int SU_print_forms(SU_forms_T* F) {
  int f;
  for (f=0; f<F->no_forms; f++)
    SU_print_f(&(F->forms[f]));
  return 0;
}

////////////////////////////////////////////
// Prints a lemma and its info.
int SU_print_lemma(SU_lemma_T* l) {
u_printf("%S:",l->unit);  //lemma
u_printf("%S:",l->cl->name); //class
u_printf("%s",l->paradigm);    //inflection paradigm
return 0;
}

////////////////////////////////////////////
// For testing purposes
// Initialise a sample lemma structure for tests.
int SU_init_lemma(SU_lemma_T* l, char* word, char* cl, char* para) {
  //lemma
  l->unit = (unichar*) malloc((strlen(word)+1)*sizeof(unichar));
  u_strcpy(l->unit,word);
  //class
  if (!strcmp(cl,"noun"))
    l->cl = &(L_CLASSES.classes[0]);
  else
    if (!strcmp(cl,"adj"))
      l->cl =&( L_CLASSES.classes[1]);
    else
      if (!strcmp(cl,"adv"))
	l->cl = &(L_CLASSES.classes[2]);
      else
	l->cl = NULL;
  //paradigm
  l->paradigm = (char*) malloc((strlen(para)+1)*sizeof(char));
  strcpy(l->paradigm,para);
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

/*
  * Unitex 
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#define MAX_CHARS_IN_STACK 100

//////////////////////////////
//Description of all classes in the current language 
extern l_classes_T L_CLASSES; 

//////////////////////////////
// Table of inflection tranducers
extern Fst2* fst2[N_FST2];

//////////////////////////////
struct couple_string {
   unichar flechi[MAX_CHARS_IN_STACK];
   unichar out[MAX_CHARS_IN_STACK];
   struct couple_string* suivant;
};

//////////////////////////////
int SU_inflect(SU_id_T* SU_id,f_morpho_T* desired_features, SU_forms_T* forms);
int SU_explore_state(unichar* flechi,unichar* canonique,unichar* sortie,
		     Fst2* a,int etat_courant, f_morpho_T* desired_features, SU_forms_T* forms);
int SU_explore_tag(Fst2Transition T,unichar* flechi,unichar* canonique,unichar* sortie,
		   Fst2* a,f_morpho_T* desired_features, SU_forms_T* forms);
int SU_explore_state_recursion(unichar* flechi,unichar* canonique,unichar* sortie,
                   Fst2* a,int etat_courant,struct couple_string** L,
		   f_morpho_T* desired_features, SU_forms_T* forms);
void SU_explore_tag_recursion(Fst2Transition T,unichar* flechi,unichar* canonique,unichar* sortie,
			      Fst2* a,struct couple_string** LISTE,f_morpho_T* desired_features, SU_forms_T* forms);
void shift_stack(unichar* stack,int pos);
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
// Returns 0 on success, 1 otherwise.   
int SU_inflect(SU_id_T* SU_id,f_morpho_T* desired_features, SU_forms_T* forms) {
  int err;
  unichar flechi[MAX_CHARS_IN_STACK];
  unichar sortie[MAX_CHARS_IN_STACK];
  sortie[0]='\0';
  int T=get_transducer(SU_id->lemma->paradigm);
  if (fst2[T]==NULL) {
    // if the automaton has not been loaded
    return 1;
  }
  u_strcpy(flechi,SU_id->lemma->unit);
  err = SU_explore_state(flechi,SU_id->lemma->unit,sortie,fst2[T],0,desired_features,forms);
  return err;
}


/**
 * This function inflects a simple word. 'lemma' is the lemma as found in
 * the DELAS, 'inflection_code' is the name of the inflection transducer without
 * extension (ex: N43). 'forms' is a structure (supposed to be allocated) that 
 * will receive all the produced inflected forms with their inflectional features.
 * The output DELAF lines will have to be built from 'forms'.
 */
int SU_inflect(unichar* lemma,char* inflection_code, SU_forms_T* forms) {
  int err;
  unichar inflected[MAX_CHARS_IN_STACK];
  unichar inflection_codes[MAX_CHARS_IN_STACK];
  inflection_codes[0]='\0';
  int T=get_transducer(inflection_code);
  if (fst2[T]==NULL) {
    // if the automaton has not been loaded
    return 1;
  }
  u_strcpy(inflected,lemma);
  err=SU_explore_state(inflected,lemma,inflection_codes,fst2[T],0,NULL,forms);
  return err;
}

////////////////////////////////////////////
// Explore the transducer a starting from state 'etat_courant'.
//Conserve only the forms that agree with the 'desired_features'.
// desired_features: morphology of the desired forms, e.g. {Gen=fem, Case=Inst}, or {} (if separator)
//                   if 'desired_features' is NULL, it means that we want to generate all the
//                   inflected forms of a simple word. In that case, we consider raw inflection
//                   features like "fp" instead of structured ones like {Gen=fem, Nb=pl}
// forms: return parameter; set of the inflected forms corresponding to the given inflection features
//        e.g. (3,{[reka,{Gen=fem,Nb=sing,Case=Instr}],[rekami,{Gen=fem,Nb=pl,Case=Instr}],[rekoma,{Gen=fem,Nb=pl,Case=Instr}]})
//        or   (1,{["-",{}]})
// Returns 0 on success, 1 otherwise.   
int SU_explore_state(unichar* flechi,unichar* canonique,unichar* sortie,
                   Fst2* a,int etat_courant, f_morpho_T* desired_features, SU_forms_T* forms) {
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
         else { //If undesired form delete 'feat'
	         f_delete_morpho(feat[f]);
	      }
         f++;
       }
       free(feat);
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
  Fst2Transition t=e->transitions;
  while (t!=NULL) {
    err = SU_explore_tag(t,flechi,canonique,sortie,a,desired_features,forms);
    if (err)
      return err;
    t=t->next;
  }
  return 0;
}

////////////////////////////////////////////
// Explore the tag of the transition T
// Conserve only the forms that agree with the 'desired_features'.
// desired_features: morphology of the desired forms, e.g. {Gen=fem, Case=Inst}, or {} (if separator)
// forms: return parameter; set of the inflected forms corresponding to the given inflection features
//        e.g. (3,{[reka,{Gen=fem,Nb=sing,Case=Instr}],[rekami,{Gen=fem,Nb=pl,Case=Instr}],[rekoma,{Gen=fem,Nb=pl,Case=Instr}]})
//        or   (1,{["-",{}]})
// Returns 0 on success, 1 otherwise.   
int SU_explore_tag(Fst2Transition T,unichar* flechi,unichar* canonique,unichar* sortie,
                 Fst2* a,f_morpho_T* desired_features, SU_forms_T* forms) {
  if (T->tag_number < 0) {
    // if we are in the case of a call to a sub-graph
    struct couple_string* L=NULL;
    struct couple_string* temp;
    SU_explore_state_recursion(flechi,canonique,sortie,a,a->initial_states[-(T->tag_number)],&L,desired_features,forms);
    while (L!=NULL) {
      SU_explore_state(L->flechi,canonique,L->out,a,T->state_number,desired_features,forms);
      temp=L;
      L=L->suivant;
      free(temp);
    }
    return 0;
  }
  Fst2Tag e=a->tags[T->tag_number];
  int pos=u_strlen(flechi);
  unichar out[MAX_CHARS_IN_STACK];
  unichar pile[MAX_CHARS_IN_STACK];
  unichar etiq[MAX_CHARS_IN_STACK];
  int pos_etiq;
  u_strcpy(out,sortie);
  if (e->output!=NULL && u_strcmp(e->output,"<E>")) {
    u_strcat(out,e->output);
  }
  u_strcpy(pile,flechi);
  u_strcpy(etiq,e->input);
  if (u_strcmp(etiq,"<E>")) {
    // if the tag is not <E>, we process it
    for (pos_etiq=0;etiq[pos_etiq]!='\0';) {
      switch (etiq[pos_etiq]) {
      case (unichar) 'L':   {
	if (pos!=0) {
	  // if the stack is not empty, we decrease the
	  // stack pointer
	  pos--;
	}
	pos_etiq++;
      };break;
      case (unichar) 'R':   {
	pos++;
	pos_etiq++;
      };break;
      case (unichar) 'C':   {
	shift_stack(pile,pos);
	pos=pos++;
	pos_etiq++;
      };break;
      default:    {
	pile[pos++]=etiq[pos_etiq++];
      };break;
      }
    }
  }
  // then, we go the next state
  pile[pos]=(unichar)'\0';
  SU_explore_state(pile,canonique,out,a,T->state_number,desired_features,forms);
  return 0;
}

////////////////////////////////////////////
// Explore the sub-transducer a
// Conserve only the forms that agree with the 'desired_features'.
// desired_features: morphology of the desired forms, e.g. {Gen=fem, Case=Inst}, or {} (if separator)
// forms: return parameter; set of the inflected forms corresponding to the given inflection features
//        e.g. (3,{[reka,{Gen=fem,Nb=sing,Case=Instr}],[rekami,{Gen=fem,Nb=pl,Case=Instr}],[rekoma,{Gen=fem,Nb=pl,Case=Instr}]})
//        or   (1,{["-",{}]})
// Returns 0 on success, 1 otherwise.   
int SU_explore_state_recursion(unichar* flechi,unichar* canonique,unichar* sortie,
                   Fst2* a,int etat_courant,struct couple_string** L,
                   f_morpho_T* desired_features, SU_forms_T* forms) {
  Fst2State e=a->states[etat_courant];
  if (e->control & 1) {
    // if we are in a final state, we save the computed things
    struct couple_string* res=(struct couple_string*)malloc(sizeof(struct couple_string));
    u_strcpy(res->flechi,flechi);
    u_strcpy(res->out,sortie);
    res->suivant=(*L);
    (*L)=res;
  }
  Fst2Transition t=e->transitions;
  while (t!=NULL) {
    SU_explore_tag_recursion(t,flechi,canonique,sortie,a,L,desired_features,forms);
    t=t->next;
  }
  return 0;
}

////////////////////////////////////////////
// explore the tag of the transition T
// Conserve only the forms that agree with the 'desired_features'.
// desired_features: morphology of the desired forms, e.g. {Gen=fem, Case=Inst}, or {} (if separator)
// forms: return parameter; set of the inflected forms corresponding to the given inflection features
//        e.g. (3,{[reka,{Gen=fem,Nb=sing,Case=Instr}],[rekami,{Gen=fem,Nb=pl,Case=Instr}],[rekoma,{Gen=fem,Nb=pl,Case=Instr}]})
//        or   (1,{["-",{}]})
// Returns 0 on success, 1 otherwise.   
void SU_explore_tag_recursion(Fst2Transition T,unichar* flechi,unichar* canonique,unichar* sortie,
                 Fst2* a,struct couple_string** LISTE,f_morpho_T* desired_features, SU_forms_T* forms) {
u_printf("\nTransition: ");  //debug
u_printf("%S/%S\n",a->tags[T->tag_number]->input,a->tags[T->tag_number]->output);  //debug

if (T->tag_number < 0) {
    // if we are in the case of a call to a sub-graph
    struct couple_string* L=NULL;
    struct couple_string* temp;
    SU_explore_state_recursion(flechi,canonique,sortie,a,a->initial_states[-(T->tag_number)],&L,desired_features,forms);
    while (L!=NULL) {
      SU_explore_state_recursion(L->flechi,canonique,L->out,a,T->state_number,LISTE,desired_features,forms);
      temp=L;
      L=L->suivant;
      free(temp);
    }
    return;
  }
  Fst2Tag e=a->tags[T->tag_number];
  int pos=u_strlen(flechi);
  unichar out[MAX_CHARS_IN_STACK];
  unichar pile[MAX_CHARS_IN_STACK];
  unichar etiq[MAX_CHARS_IN_STACK];
  int pos_etiq;
  u_strcpy(out,sortie);
  if (e->output!=NULL && u_strcmp(e->output,"<E>")) {
    u_strcat(out,e->output);
  }
  u_strcpy(pile,flechi);
  u_strcpy(etiq,e->input);
  if (u_strcmp(etiq,"<E>")) {
    // if the tag is not <E>, we process it
    for (pos_etiq=0;etiq[pos_etiq]!='\0';) {
      switch (etiq[pos_etiq]) {
      case 'L':   {
	if (pos!=0) {
	  // if the stack is not empty, we decrease the
	  // stack pointer
	  pos--;
	}
	pos_etiq++;
      };break;
      case 'R':   {
	pos++;
	pos_etiq++;
      };break;
      case 'C':   {
	shift_stack(pile,pos);
	pos=pos++;
	pos_etiq++;
      };break;
      default:    {
	pile[pos++]=etiq[pos_etiq++];
      };break;
      }
    }
  }
  // then, we go the next state
  pile[pos]='\0';
  SU_explore_state_recursion(pile,canonique,out,a,T->state_number,LISTE,desired_features,forms);
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

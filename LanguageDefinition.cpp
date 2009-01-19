 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "LanguageDefinition.h"
#include "Tagset.h"
#include "symbol.h"
#include "utils.h"

language_t* LANGUAGE=NULL;


/**
 * Sets the current language.
 */
void set_current_language(language_t* l) {
LANGUAGE=l;
}


/**
 * Returns the current language.
 */
language_t* get_current_language() {
return LANGUAGE;
}


/**
 * Allocates, initializes and returns a new feature_info_t.
 */
feature_info_t* new_feature_info_t(char type,int CATid,int val) {
feature_info_t* infos=(feature_info_t*)malloc(sizeof(feature_info_t));
if (infos==NULL) {
   fatal_error("Not enough memory in new_feature_info_t\n");
}
infos->type=type;
infos->CATid=CATid;
infos->val=val;
return infos;
}


/**
 * Frees all the memory associated to the given feature_info_t.
 */
void free_feature_info_t(feature_info_t* infos) {
if (infos!=NULL) free(infos);
}


/**
 * Allocates, initializes and returns a new category description.
 */
CAT_t* new_CAT_t(unichar* name) {
static unichar unspecified[]={ 'u', 'n', 's', 'p', 'e', 'c', 'i', 'f', 'i', 'e', 'd', 0 };
CAT_t* CAT=(CAT_t*)malloc(sizeof(CAT_t));
if (CAT==NULL) {
   fatal_error("Not enough memory in new_CAT_t\n");
}
CAT->name=u_strdup(name);
CAT->values=new_string_hash(16);
CAT_add_value(CAT,unspecified);
return CAT;
}


/**
 * Frees all the memory associated to the given category description.
 */
void free_CAT_t(CAT_t* CAT) {
if (CAT==NULL) return;
if (CAT->name!=NULL) free(CAT->name);
free_string_hash(CAT->values);
free(CAT);
}


/**
 * Adds the given value to the given category.
 */
int CAT_add_value(CAT_t* CAT,unichar* value) {
return get_value_index(value,CAT->values);
}


/**
 * Allocates, initializes and returns a new POS description.
 */
POS_t* new_POS_t(unichar* POS_name) {
POS_t* res=(POS_t*)malloc(sizeof(POS_t));
if (res==NULL) {
   fatal_error("Not enough memory in new_POS_t\n");
}
res->name=u_strdup(POS_name);
res->ignorable=false;
res->nb_inflect=0;
res->nb_discr=0;
res->CATs=new_string_hash_ptr();
res->values=new_string_hash_ptr();
res->codes=NULL;
res->language=NULL;
res->index=-1;
return res;
}


/**
 * Frees all the memory associated to the given POS description.
 */
void free_POS_t(POS_t* POS) {
free_symbols(POS->codes);
free_string_hash_ptr(POS->CATs,(void(*)(void*))free_CAT_t);
free_string_hash_ptr(POS->values,(void(*)(void*))free_feature_info_t);
free(POS->name);
free(POS);
}


/**
 * Adds an inflectional feature to the given POS description.
 */
int POS_add_inflectional_feature(POS_t* POS,unichar* name) {
if (POS->nb_inflect!=POS->CATs->hash->size) {
   fatal_error("POS_add_inflectional_feature: inflectional features should be added first\n");
}
if (get_value_index(name,POS->CATs,DONT_INSERT)!=-1) {
   fatal_error("POS_add_inflectional_feature: flex '%S' already exists\n", name);
}
CAT_t* CAT=new_CAT_t(name);
int index=get_value_index(name,POS->CATs,INSERT_IF_NEEDED,CAT);
POS->nb_inflect++;
POS->nb_discr++;
return index;
}


/**
 * Adds the given value for the inflectional feature #inflect_id in the
 * given POS description.
 */
int POS_add_inflectional_feature_value(POS_t* POS,int inflect_id,unichar* value) {
if (get_value_index(value,POS->values,DONT_INSERT)!=-1) {
   feature_info_t* infos=(feature_info_t*)get_value(value,POS->values);
   CAT_t* cat1=(CAT_t*)POS->CATs->value[infos->CATid];
   CAT_t* cat2=(CAT_t*)POS->CATs->value[inflect_id];
   fatal_error("POS_add_inflectional_feature_value: in POS '%S': value '%S' defined in categories '%S' and '%S'.\n",POS->name,value,cat1->name,cat2->name);
}
int index=CAT_add_value((CAT_t*)POS->CATs->value[inflect_id],value);
feature_info_t* infos=new_feature_info_t('f',inflect_id,index);
get_value_index(value,POS->values,INSERT_IF_NEEDED,infos);
return index;
}


/**
 * Adds the given category to the given POS description.
 */
int POS_add_CAT(POS_t* POS,unichar* name) {
if (get_value_index(name,POS->CATs,DONT_INSERT)!=-1) {
   fatal_error("POS new CAT: category '%S' already exists\n",name);
}
CAT_t* CAT=new_CAT_t(name);
return get_value_index(name,POS->CATs,INSERT_IF_NEEDED,CAT);
}


/**
 * Adds the given value for the category #cat_id in the
 * given POS description.
 */
int POS_add_CAT_value(POS_t* POS,int cat_id,unichar* value) {
if (get_value_index(value,POS->values,DONT_INSERT)!=-1) {
   fatal_error("add value: value '%S' already defined\n", value);
}
int index=CAT_add_value((CAT_t*)POS->CATs->value[cat_id],value);
feature_info_t* infos=new_feature_info_t('c',cat_id,index);
get_value_index(value,POS->values,INSERT_IF_NEEDED,infos);
return index;
}





/* retourne l'ensemble des codes qui balayent la partie du discours
 */

symbol_t * POS_expand(POS_t * POS) {
  
  if (POS->codes == NULL) {
    return new_symbol_POS(POS);
  }

  //  debug("POS_expand: '%S': codes==\n", POS->name); symbols_dump(POS->codes); endl();

  return dup_symbols(POS->codes);
}


/* language */


POS_t * add_POS(language_t * lang, const unichar * name);


/**
 * Allocates, initializes and returns a new language_t structure.
 */
language_t* new_language_t(unichar* language_name) {
language_t* language=(language_t*)malloc(sizeof(language_t));
if (language==NULL) {
   fatal_error("Not enough memory in new_language_t\n");
}
language->name=u_strdup(language_name);
language->POSs=new_string_hash_ptr(16);
language->forms=new_string_hash(128);
language->unknown_codes=new_string_hash_ptr(4);
/* We ensure that UNKNOWN_STR, PUNC_STR and CHFA_STR have respectively
 * the indices 0, 1 and 2 */
add_POS(language,UNKNOWN_STR);
add_POS(language,PUNC_STR);
add_POS(language,CHFA_STR);
/* We make sure that forms[0] is the empty string */
language_add_form(language,U_EMPTY);
return language;
}


/**
 * Frees all the memory associated to the given language description.
 */
void free_language_(language_t* language) {
if (language==NULL) return;
if (language->name!=NULL) free(language->name);
free_string_hash_ptr(language->POSs,(void (*)(void*))free_POS_t);
free_string_hash(language->forms);
free_string_hash_ptr(language->unknown_codes,free);
free(language);
}


/**
 * Adds the POS with the given name to the given language definition.
 * 
 * The corresponding POS_t structure is returned.
 */
POS_t* add_POS(language_t* language,const unichar* POS_name) {
if (get_value_index((unichar*)POS_name,language->POSs,DONT_INSERT)!=-1) {
   fatal_error("add_POS: POS '%S' already exists.\n", POS_name);
}
POS_t* POS=new_POS_t((unichar*)POS_name);
POS->index=get_value_index((unichar*)POS_name,language->POSs,INSERT_IF_NEEDED,POS);
POS->language=language;
return POS;
}


/**
 * This function expands a code to a symbol list. 'code' is the original code,
 * like "<gender> <number>". 'templat' is the code, being expanded. For the previous
 * example, 'templat' will be "<gender> <number>" on the first call, and then
 * "m <number>" or "f <number>" on the second, etc. 'index' is the number of the feature
 * being looked at.
 */
symbol_t* expand_code(symbol_t* code,symbol_t* templat,int index) {
/* We copy all the feature values, until we find a category like "<gender>" */
for (;index<code->POS->nb_discr && code->feature[index]!=UNSPECIFIED;index++) {
   templat->feature[index]=code->feature[index];
}
if (index>=code->POS->nb_discr) {
   /* If there is no more category to expand */
   return dup_symbol(templat);
}
symbol_t res;
res.next=NULL;
symbol_t* end=&res;
CAT_t* CAT=POS_get_CAT(code->POS,index);
for (int v=1;v<CAT->values->size;v++) {
   /* If we must expand the category #index, we take all its possible values,
    * except LOCKED and UNSPECIFIED */
   templat->feature[index]=v;
   /* Then, for a given value of the feature #index, we expand recursively
    * the rest of the code and we concatenate the result to the resulting
    * symbol list. */
   concat_symbols(end,expand_code(code,templat,index+1),&end);
}
return res.next;
}


/**
 * This function takes a symbol representing a code that may include
 * a category like "<gender>" and it returns the exhaustive list of
 * possible codes, replacing each category by all its value. For instance,
 * the symbole:
 * 
 *   <gender> <number>
 * 
 * may be replaced by:
 * 
 *    m s
 *    f s
 *    m p
 *    f p
 */
symbol_t* expand_code(symbol_t* code) {
symbol_t* templat=new_symbol_POS(code->POS);
templat->type=CODE;
symbol_t* res=expand_code(code,templat,0);
free_symbol(templat);
return res;
}


/**
 * This function takes a POS description and a list of lines representing
 * the valid codes for this POS (see example in definition of the 'POS_t' type).
 * It returns the corresponding list of symbols. 
 */
symbol_t* get_full_codes(POS_t* POS,tokens_list* code_list) {
symbol_t res;
res.next=NULL;
symbol_t* end=&res;
symbol_t* symbol=new_symbol_POS(POS);
/* We look each line */
for (tokens_list* list=code_list;list!=NULL;list=list->next) {
   /**
    * We initialize the features, saying that they are all irrelevant, as well as
    * the discriminative category code, if any.
    */
   for (int i=0;i<POS->nb_discr;i++) {
      symbol->feature[i]=LOCKED;
   }
   if (list->tokens->type==TOK_BLANK) {
      /* BLANK == all features are LOCKED */
   } else {
      /* For a given line, we examine all the information it contains */
      for (token_t* token=list->tokens;token!=NULL;token=token->next) {
         if (token->type==TOK_ANGLE) {
            /* If we find a category name like "<gender>" */
            int index;
            if ((index=get_value_index(token->str,POS->CATs,DONT_INSERT))!=-1) {
               if (index>=POS->nb_discr) {
                  fatal_error("'<%S>' isn't a discriminant category.\n",token->str);
               }
               /* We say the value is not specified for this feature, in
                * order to expand it later to all the possible values */
               symbol->feature[index]=UNSPECIFIED;
            } else {
               fatal_error("In POS '%S': unknown category: <%S>\n",POS->name,token->str);
            }
         } else {
            /* If we find a feature value like "Ppv" */
            feature_info_t* infos=(feature_info_t*)get_value(token->str,POS->values);
            if (infos==NULL) {
               fatal_error("In POS '%S': unknown value: '%S'\n",POS->name,token->str);
            }
            if (infos->CATid>=POS->nb_discr) {
               fatal_error("'%S' isn't a discriminant code.\n",token->str);
            }
            /* We set that the corresponding feature value */
            symbol->feature[infos->CATid]=infos->val;
         }
      }
   }
   concat_symbols(end,expand_code(symbol),&end);
}
free_symbol(symbol);
return res.next;
}


/**
 * Adds the given form the the language's forms and returns its
 * index.
 */
int language_add_form(language_t* language,const unichar* form) {
return get_value_index((unichar*)form,language->forms);
}


/**
 * Loads and returns the given tagset definition file.
 */
language_t* load_language_definition(FILE* f) {
/* We load the content of the tagset file */
tagset_t* tagset=load_tagset(f);
if (tagset==NULL) {
   fatal_error("Unable to load tagset file\n");
}
language_t* language=new_language_t(tagset->name);
for (pos_section_t* section=tagset->pos_sections;section!=NULL;section=section->next) {
   POS_t* POS=add_POS(language,section->name);
   tokens_list* toklist;
   if (section->ignore) {
      POS->ignorable=true;
   } else {
     POS->ignorable=false;
   }
   /* Inflection part of the POS description */
   for (toklist=section->parts[PART_FLEX];toklist!=NULL;toklist=toklist->next) {
      token_t* toks=toklist->tokens;
      int category_index=POS_add_inflectional_feature(POS,toks->str);
      /* We skip the equal sign */
      toks=toks->next->next;
      /* For the given inflectional category, we add all the possible inflectional values */
      while (toks!=NULL) {
         POS_add_inflectional_feature_value(POS,category_index,toks->str);
         toks=toks->next;
      }
   }
   /* Discriminative part of the POS description
    * 
    * If there is a discriminatory category, we put it in front of the category part,
    * so its index will always be 0 */
   if (section->parts[PART_DISCR]!=NULL) {
      POS->nb_discr=POS->nb_inflect+1;
      section->parts[PART_DISCR]->next=section->parts[PART_CAT];
      section->parts[PART_CAT]=section->parts[PART_DISCR];
      section->parts[PART_DISCR]=NULL;
   } else {
      POS->nb_discr=POS->nb_inflect;
   }
   /* Categories part of the POS description */
   for (toklist=section->parts[PART_CAT];toklist!=NULL;toklist=toklist->next) {
      token_t* toks=toklist->tokens;
      int category_index=POS_add_CAT(POS,toks->str);
      /* We skip the equal sign */
      toks=toks->next->next;
      while (toks!=NULL) {
         if (toks->type!=TOK_BLANK) {
            POS_add_CAT_value(POS,category_index,toks->str);
         } else {
            fatal_error("'_' in category '%S' values\n",toklist->tokens->str);
         }
         toks=toks->next;
      }
   }
   /* Finally, we deal with the complete code list */
   POS->codes=get_full_codes(POS,section->parts[PART_COMP]);
}
free_tagset_t(tagset);
return language;
}


/**
 * Loads and returns the given tagset definition file.
 */
language_t* load_language_definition(char* name) {
FILE* f=u_fopen(name,U_READ);
if (f==NULL) {
   fatal_error("Unable to open %s for reading.\n",name);
}
return load_language_definition(f);
}


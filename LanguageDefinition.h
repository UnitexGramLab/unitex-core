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

/**
 * This library is used to manipulate languages.
 *
 * Author: Olivier Blanc
 * Modified by S�bastien Paumier
 */

#ifndef LanguageDefinitionH
#define LanguageDefinitionH

#include "String_hash.h"
#include "Symbol.h"
#include "DELA.h"


/* Predefined POSs */
static const unichar UNKNOWN_STR[] = { '?', 0 };
static const unichar PUNC_STR[]   = { 'P', 'N', 'C', 0 };
static const unichar CHFA_STR[]   = { 'N', 'B', 0 };



/**
 * This structure defines a set of inflectional features, such as
 * "gender" that can be "m" or "f".
 */
typedef struct CAT_t {
   unichar* name;
   struct string_hash* values;
} CAT_t;


struct symbol_t;
struct language_t;


/**
 * Part of Speech definition
 */
typedef struct POS_t {
   /* The POS name like "V" or "ADV" */
   unichar* name;

   /* IGNORE symbol in text tfst, during elag grammar application */
   bool ignorable;

   /* The first nb_inflect features are inflectional features */
   int nb_inflect;

   /* The first nb_discr category features are discriminatory features */
   int nb_discr;

   /* Possible attributes for this POS. For instance, if you set the language level
    * with z1 z2 and z3, you could have the following line in the tagset definition:
    *
    *    language_level = z1 z2 z3
    *
    * Then, CATS would contain the attribute "language_level". */
   struct string_hash_ptr* CATs;

   /* This hash table is used to know for each feature value what
    * is the associated category, i.e. "f" => "gender" or
    * "z2" => "language_level".
    *
    * WARNING: a same value cannot be shared by several features. For instance,
    *          you cannot use "m" as a syntactic information if it already
    *          represents the masculine feature, even if DELAF lines like
    *
    *          arbre,.N+m+z1:ms
    *
    *          are valid. */
   struct string_hash_ptr* values;

   /* This list defines the valid combinations of features for this POS.
    * Here is the list for French verbs:
    *
    * W
    * G
    * C <pers> <nombre>
    * F <pers> <nombre>
    * I <pers> <nombre>
    * J <pers> <nombre>
    * P <pers> <nombre>
    * S <pers> <nombre>
    * T <pers> <nombre>
    * X 1 s   # euss� duss� puiss� fuss�
    * Y 1 p
    * Y 2 <nombre>
    * K <genre> <nombre>
    */
   symbol_t* codes;

   /* The language definition this POS belongs to */
   language_t* language;

   /* Index of this POS in the POS table of the tagset */
   int index;
} POS_t;


/**
 * This structure defines a grammatical or inflectional feature.
 */
typedef struct feature_info_t {
   /* 'f' => inflectional feature
    * 'c' => grammatical feature */
   char type;

   int CATid;
   char val;
} feature_info_t;


/**
 * This structure defines a language.
 */
typedef struct language_t {
   /* Language name like "English" */
   unichar* name;

   /* Hashtable for the POSs of the language */
   struct string_hash_ptr* POSs;

   /* Hashtable containing all the inflected forms and lemmas */
   struct string_hash* forms;

   /* Hashtable used to store codes that are not defined in the
    * language's legal tagset. This is useful for not printing the
    * same error messages several times. */
   struct string_hash_ptr* unknown_codes;
} language_t;



feature_info_t* new_feature_info_t(char,int,int);
void free_feature_info_t(feature_info_t*);

CAT_t* new_CAT_t(unichar*);
void free_CAT_t(CAT_t*);
int CAT_add_value(CAT_t * CAT, const unichar * value);


static inline unichar * CAT_get_valname(CAT_t * CAT, int v) {
  return (unichar *) CAT->values->value[v];
}



POS_t* new_POS_t(unichar * name);
void free_POS_t(POS_t * POS);


int POS_add_CAT(POS_t * POS, unichar * name);
int POS_add_CAT_value(POS_t * POS, int CATidx, unichar * value);



symbol_t * POS_expand(POS_t * POS);

static inline CAT_t* POS_get_CAT(POS_t* POS,int index) {
return (CAT_t *)POS->CATs->value[index];
}
static inline CAT_t* POS_get_CAT(POS_t* POS,unichar* name) {
return (CAT_t*)get_value(name,POS->CATs);
}

static inline int POS_get_CATid(POS_t* POS,unichar* name) {
return get_value_index(name,POS->CATs,DONT_INSERT);
}

/**
 * This function returns the description of the given semantic feature,
 * if any.
 */
static inline feature_info_t* POS_get_semantic_feature_infos(POS_t* POS,unichar* name) {
feature_info_t* infos=(feature_info_t*)get_value(name,POS->values);
if (infos==NULL || infos->type!='c') {
   /* If there is no corresponding feature or if it's an inflectional feature */
   return NULL;
}
return infos;
}


/**
 * This function returns the description of the given inflectional feature,
 * if any.
 */
static inline feature_info_t* POS_get_inflect_feature_infos(POS_t* POS,unichar name) {
unichar tmp[2];
tmp[0]=name;
tmp[1]='\0';
feature_info_t* infos=(feature_info_t*)get_value(tmp,POS->values);
if (infos==NULL || infos->type!='f') {
   /* If there is no corresponding feature or if it's not an inflectional feature */
   return NULL;
}
return infos;
}



language_t * new_language_t(unichar*);
void free_language_t(language_t*);


language_t* load_language_definition(U_FILE*);
language_t* load_language_definition(char*);

static inline POS_t * language_get_POS(language_t * lang, const unichar * posname) {
  return (POS_t*)get_value((unichar*)posname,lang->POSs);
}




int language_add_form(language_t * lang, const unichar * form);
static inline unichar * language_get_form(language_t * lang, int idx) {
  return (unichar *) lang->forms->value[idx];
}

struct dela_entry* filter_dela_entry(struct dela_entry*,unichar* tag,language_t*,int);

#endif

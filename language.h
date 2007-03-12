 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef _LANGUAGE_H_
#define _LANGUAGE_H_

#include "hash_str_table.h"
#include "symbol.h"

/* predefined POSs */

#define UNKNOW_idx  0
#define PUNC_idx    1
#define CHFA_idx    2

static const unichar UNKNOW_STR[] = { '?', 0 };
static const unichar PUNC_STR[]   = { 'P', 'N', 'C', 0 };
//static const unichar CHFA_STR[]   = { 'C', 'H', 'F', 'A', 0 };
static const unichar CHFA_STR[]   = { 'N', 'B', 0 };



/* categorie qui regroupe un ensemble de traits
 * ex : genre pour m et s
 *      ...
 */

typedef struct CAT_t {
  unichar * name;
  hash_str_table_t * traits;  // unichar * table (values)
} CAT_t;

struct symbol_t;
struct language_t;

/* Part of Speech definition
 */

typedef struct POS_t {

  unichar * name;

  bool ignorable; // IGNORE symbol in text fsa, during elag grammar application

  int nbflex;   // les nbflex premiers traits sont des codes de flexion 
  int nbdiscr; // les nbdiscr 1ers traits sont "discriminants" (y compris les codes flexionnels)

  hash_str_table_t * CATs;  // categories

  hash_str_table_t * values;  // table on info: pour recuperer la categorie d'un trait

  symbol_t * codes;           // etiquettes completes (UNSPECIFIED pour n'importe quelle valeur
                              // et LOCKED pour aucune valeur)

  language_t * lang;          // attached language 
  int idx;                    // index de la table des POSs du language

} POS_t;



typedef struct trait_info_t {

  char type; // flex | CAT ->  'f'|  'c' (useless now)
  int  CATid;
  char  val;

} trait_info_t;


typedef struct language_t {
  unichar * name;
  hash_str_table_t * POSs;
  hash_str_table_t * forms;        // hashtable for all canonical and flexional forms
  hash_str_table_t * unknow_codes; // for storing codes which are not defined in lang file
} language_t;

extern language_t * LANG; // current language


trait_info_t * val_info_new(char type, int CATidx, int validx);
void val_info_delete(trait_info_t * infos);



CAT_t * CAT_new(unichar * name);
void CAT_delete(CAT_t * CAT);
int CAT_add_value(CAT_t * CAT, unichar * value);

static inline unichar * CAT_get_valname(CAT_t * CAT, int v) {
  return (unichar *) CAT->traits->tab[v];
}



POS_t * POS_new(unichar * name);
void POS_delete(POS_t * POS);


int POS_new_CAT(POS_t * POS, unichar * name);
int POS_CAT_add_value(POS_t * POS, int CATidx, unichar * value);



symbol_t * POS_expand(POS_t * POS);

static inline CAT_t * POS_get_CAT(POS_t * POS, int idx) { return (CAT_t *) POS->CATs->tab[idx]; }
static inline CAT_t * POS_get_CAT(POS_t * POS, unichar * name) {
  return (CAT_t *) hash_str_table_lookup(POS->CATs, name);
}

static inline int POS_get_CATid(POS_t * POS, unichar * name) {
  return hash_str_table_idx_lookup(POS->CATs, name);
}

static inline trait_info_t * POS_get_trait_infos(POS_t * POS, unichar * val) {

  trait_info_t * infos = (trait_info_t *) hash_str_table_lookup(POS->values, val);

  if (infos == NULL || infos->type != 'c') { return NULL; }

  return infos;
}

static inline trait_info_t * POS_get_flex_infos(POS_t * POS, unichar val) {

  unichar buf[2];
  buf[0] = val; buf[1] = 0;
  trait_info_t * infos = (trait_info_t *) hash_str_table_lookup(POS->values, buf);

  if (infos == NULL || infos->type != 'f') { return NULL; }

  return infos;
}



language_t * language_new(unichar * name);
void language_delete(language_t * lang);


language_t * language_load(FILE * f);
language_t * language_load(char * fname);

static inline POS_t * language_get_POS(language_t * lang, const unichar * posname) {
  return (POS_t *) hash_str_table_lookup(lang->POSs, (unichar *) posname);
}




int language_add_form(language_t * lang, const unichar * form);
static inline unichar * language_get_form(language_t * lang, int idx) {
  return (unichar *) lang->forms->tab[idx];
}
static inline unichar * language_get_form(int idx) { return (unichar *) LANG->forms->tab[idx]; }

void set_current_language(language_t * lang);
language_t * get_current_language();


#endif

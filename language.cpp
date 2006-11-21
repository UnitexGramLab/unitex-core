 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "language.h"
#include "language_parsing.h"
#include "symbol.h"
#include "utils.h"

language_t * LANG = NULL;


void set_current_language(language_t * lang) { LANG = lang; }
language_t * get_current_language() { return LANG; }


static void u_str_dump(unichar * str, FILE * f) { i_fprintf(f, "%S", str); }


trait_info_t * val_info_new(char type, int CATid, int val) {

  trait_info_t * infos = (trait_info_t *) xmalloc(sizeof(trait_info_t));

  infos->type  = type;
  infos->CATid = CATid;
  infos->val   = val;

  return infos;
}


void val_info_delete(trait_info_t * infos) { free(infos); }


void val_info_dump(trait_info_t * i, FILE * f) {
  i_fprintf(f, "{ %c, %d, %d }", i->type, i->CATid, i->val);
}


CAT_t * CAT_new(unichar * name) {

  static unichar unspecified[] = { 'u', 'n', 's', 'p', 'e', 'c', 'i', 'f', 'i', 'e', 'd', 0 };

  CAT_t * CAT = (CAT_t *) xmalloc(sizeof(CAT_t));

  CAT->name   = u_strdup(name);

  CAT->traits = hash_str_table_new(16);

  CAT_add_value(CAT, unspecified);

  return CAT;
}



void CAT_delete(CAT_t * CAT) {
  hash_str_table_delete(CAT->traits, free);
  free(CAT->name);
  free(CAT);
}


int CAT_add_value(CAT_t * CAT, unichar * value) {
  return hash_str_table_add(CAT->traits, value, u_strdup(value));
}



void CAT_dump(CAT_t * cat, FILE * f) {
  i_fprintf(f, "CAT: %S\n", cat->name);
  hash_str_table_dump(cat->traits, (dump_f) u_str_dump, f);
}


POS_t * POS_new(unichar * name) {

  POS_t * res = (POS_t *) xmalloc(sizeof(POS_t));

  res->name      = u_strdup(name);
  res->ignorable = false;

  /*
  res->has_discr = 0;
  res->flexs     = hash_str_table_new();
  res->CATs      = hash_str_table_new();
  */

  res->nbflex    = 0;
  res->nbdiscr   = 0;

  res->CATs      = hash_str_table_new();
  res->values    = hash_str_table_new();

  res->codes  = NULL;

  res->lang  = NULL;
  res->idx   = -1;

  return res;
}


void POS_delete(POS_t * POS) {

  symbols_delete(POS->codes);

  /*
  hash_str_table_delete(POS->flexs, (release_f) CAT_delete);
  hash_str_table_delete(POS->CATs, (release_f) CAT_delete);
  */

  hash_str_table_delete(POS->CATs, (release_f) CAT_delete);
  hash_str_table_delete(POS->values, (release_f) val_info_delete);

  free(POS->name);

  free(POS);
}


int POS_new_flex(POS_t * POS, unichar * name) {

  if (POS->nbflex != POS->CATs->nbelems) { die("POS_new_flex: internal error: flexs should be added first\n"); }

  if (hash_str_table_idx_lookup(POS->CATs, name) != -1) { die("new flex: flex '%S' already exists\n", name); }

  CAT_t * CAT = CAT_new(name);

  int idx = hash_str_table_add(POS->CATs, name, CAT);

  POS->nbflex++;
  POS->nbdiscr++;

  return idx;
}



int POS_flex_add_value(POS_t * POS, int flexid, unichar * value) {

  if (hash_str_table_idx_lookup(POS->values, value) != -1) {

    trait_info_t * infos = (trait_info_t *) hash_str_table_lookup(POS->values, value);

    CAT_t * cat1 = (CAT_t *) POS->CATs->tab[infos->CATid];
    CAT_t * cat2 = (CAT_t *) POS->CATs->tab[flexid];

    die("add value: in POS '%S': value '%S' defined in categories '%S' and '%S'.\n", POS->name, value, cat1->name,
	cat2->name);
  }

  int idx = CAT_add_value((CAT_t *) POS->CATs->tab[flexid], value);

  trait_info_t * infos = val_info_new('f', flexid, idx);

  hash_str_table_add(POS->values, value, infos);

  return idx;
}


int POS_new_CAT(POS_t * POS, unichar * name) {

  if (hash_str_table_idx_lookup(POS->CATs, name) != -1) { die("POS new CAT: category '%S' already exists\n", name); }

  CAT_t * CAT = CAT_new(name);

  int idx = hash_str_table_add(POS->CATs, name, CAT);

  return idx;
}



int POS_CAT_add_value(POS_t * POS, int CATidx, unichar * value) {

  if (hash_str_table_idx_lookup(POS->values, value) != -1) { die("add value: value '%S' already defined\n", value); }

  int idx = CAT_add_value((CAT_t *) POS->CATs->tab[CATidx], value);

  trait_info_t * infos = val_info_new('c', CATidx, idx);

  hash_str_table_add(POS->values, value, infos);

  return idx;
}



void POS_dump(POS_t * POS, FILE * f) {

  i_fprintf(f, "POS:%S\n", POS->name);

  fprintf(f, "CATs:\n");
  hash_str_table_dump(POS->CATs, (dump_f) CAT_dump, f);

  fprintf(f, "value:\n");
  hash_str_table_dump(POS->values, (dump_f) val_info_dump, f);

  fprintf(f, "complet:\n");
  for (symbol_t * symb = POS->codes; symb; symb = symb->next) {
    symbol_dump(symb, f); fprintf(f, "\n");
  }
}


/* retourne l'ensemble des codes qui balayent la partie du discours
 */

symbol_t * POS_expand(POS_t * POS) {
  
  if (POS->codes == NULL) {
    return symbol_new(POS);
  }

  //  debug("POS_expand: '%S': codes==\n", POS->name); symbols_dump(POS->codes); endl();

  return symbols_dup(POS->codes);
}


/* language */


POS_t * language_new_POS(language_t * lang, const unichar * name);


language_t * language_new(unichar * name) {

  language_t * lang = (language_t *) xmalloc(sizeof(language_t));

  lang->name  = u_strdup(name);
  lang->POSs  = hash_str_table_new(16);
  lang->forms = hash_str_table_new(128);
  lang->unknow_codes = hash_str_table_new(4);

  language_new_POS(lang, UNKNOW_STR); // unknow idx == 0
  language_new_POS(lang, PUNC_STR);   // punc idx   == 1
  language_new_POS(lang, CHFA_STR);   // chfa idx   == 2


  unichar empty = 0;
  language_add_form(lang, & empty); // forms[0] = "" (empty string)

  return lang;
}


void language_delete(language_t * lang) {
  free(lang->name);
  hash_str_table_delete(lang->POSs, (release_f) POS_delete);
  hash_str_table_delete(lang->forms, (release_f) free);
  hash_str_table_delete(lang->unknow_codes);
  free(lang);
}



void language_dump(language_t * lang, FILE * f) {

  i_fprintf(f, "name=%S\n", lang->name);

  fprintf(f, "POSs=\n");
  hash_str_table_dump(lang->POSs, (dump_f) POS_dump, f);

  fprintf(f, "forms=\n");
  hash_str_table_dump(lang->forms, NULL, f);

  fprintf(f, "unknow codes=\n");
  hash_str_table_dump(lang->unknow_codes, NULL, f);
}



POS_t * language_new_POS(language_t * lang, const unichar * name) {

  if (hash_str_table_idx_lookup(lang->POSs, (unichar *) name) != -1) {
    die("when adding new POS: POS '%S' already exists.\n", name);
  }

  POS_t * POS = POS_new((unichar *) name);

  int idx = hash_str_table_add(lang->POSs, (unichar *) name, POS);

  POS->idx  = idx;
  POS->lang = lang;

  return POS;
}



static symbol_t * expand_code(symbol_t * code, symbol_t * templat, int idx) {

  for (; (idx < code->POS->nbdiscr) && (code->traits[idx] != UNSPECIFIED); idx++) {
    templat->traits[idx] = code->traits[idx];
  }

  if (idx >= code->POS->nbdiscr) { /* no more expansion */
    return symbol_dup(templat);
  }

  // expansion (code->traits[idx] == UNSPECIFIED) 

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  CAT_t * CAT = POS_get_CAT(code->POS, idx);
  
  for (int v = 1; v < CAT->traits->nbelems; v++) { // all but LOCK and UNSPEC
    templat->traits[idx] = v;
    symbols_concat(end, expand_code(code, templat, idx + 1), & end);
  }

  return res.next;
}


static inline symbol_t * expand_code(symbol_t * code) {
  symbol_t * templat = symbol_new(code->POS);
  templat->type = CODE;
  symbol_t * res = expand_code(code, templat, 0);
  symbol_delete(templat);
  return res;
}



static symbol_t * get_full_codes(POS_t * POS, tokens_list * full_part) {

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  tokens_list * toklist;

  symbol_t * symb = symbol_new(POS);

  for (toklist = full_part; toklist; toklist = toklist->next) {

    int i, idx;

    /* we lock all flex codes and only discriminant category code
     *
     * if a category is specified (with "<category>"), we expand it to
     * everything but LOCKED nor UNSPECIFIED.
     */

    for (i = 0; i < POS->nbdiscr; i++) { symb->traits[i] = LOCKED; }

    if (toklist->tokens->type == TOK_BLANK) { // BLANK == all is LOCKED

    } else {

      for (token_t * toks = toklist->tokens; toks; toks = toks->next) {

	if (toks->type == TOK_ANGLE) { // category name

	  if ((idx = hash_str_table_idx_lookup(POS->CATs, toks->str)) != -1) {

	    if (idx >= POS->nbdiscr) { die("'<%S>' isn't a discriminant category.\n", toks->str); }

	    symb->traits[idx] = UNSPECIFIED;

	  } else { die("in POS '%S': unknow category: <%S>\n", POS->name, toks->str); }


	} else { // trait value

	  trait_info_t * infos = (trait_info_t *) hash_str_table_lookup(POS->values, toks->str);

	  if (infos == NULL) { die("in POS '%S': unknow value: '%S'\n", POS->name, toks->str); }

	  if (infos->CATid >= POS->nbdiscr) { die("'%S' isn't a discriminant code.\n", toks->str); }

	  symb->traits[infos->CATid] = infos->val;
	}
      }
    }

    symbols_concat(end, expand_code(symb), &end);
  }

/*
  if (res.next == NULL && POS->CATs->nbelems) {
    warning("POS '%S' contains no full labels\n", POS->name);
  }
*/

  symbol_delete(symb);

  return res.next;
}




int language_add_form(language_t * lang, const unichar * form) {

  int idx;
  if ((idx = hash_str_table_idx_lookup(lang->forms, (unichar *) form)) == -1) {
    idx = hash_str_table_add(lang->forms, (unichar *) form, u_strdup(form));
  }

  return idx;
}


language_t * language_load(FILE * f) {

  language_tree_t * tree = language_parse(f);

  if (tree == NULL) { die("unable to load language file\n"); }

  debug("file is parsed\n");


  language_t * lang = language_new(tree->name);

  for (pos_section_t * sec = tree->pos_secs; sec; sec = sec->next) {

    POS_t * POS = language_new_POS(lang, sec->name);

    tokens_list * toklist;

    if (sec->ignore) {
      POS->ignorable = true;
    } else {
      POS->ignorable = false;
    }

    /* flexion */

    for (toklist = sec->parts[PART_FLEX]; toklist; toklist = toklist->next) {

      token_t * toks = toklist->tokens;

      int CATidx = POS_new_flex(POS, toks->str);

      toks = toks->next->next; // skip equals

      while (toks) {
	POS_flex_add_value(POS, CATidx, toks->str);
	toks = toks->next;
      }
    }

    /* discriminant category
     * we push it in front of categories list, so its index is alwais 0
     */

    if (sec->parts[PART_DISCR]) {

      POS->nbdiscr = POS->nbflex + 1;

      sec->parts[PART_DISCR]->next = sec->parts[PART_CAT];
      sec->parts[PART_CAT] = sec->parts[PART_DISCR];
      sec->parts[PART_DISCR] = NULL;

    } else { POS->nbdiscr = POS->nbflex; }


    /* categories */

    for (toklist = sec->parts[PART_CAT]; toklist; toklist = toklist->next) {

      token_t * toks = toklist->tokens;

      int CATidx = POS_new_CAT(POS, toks->str);

      toks = toks->next->next; // skip equals

      while (toks) {

	if (toks->type != TOK_BLANK) {
	  POS_CAT_add_value(POS, CATidx, toks->str);
	} else {
	  die("'_' in feature def\n");
	}

	toks = toks->next;
      }
    }


    /* full labels */

    POS->codes = get_full_codes(POS, sec->parts[PART_COMP]);
  }

//#warning "weird language_tree_delete"
  language_tree_delete(tree);

  return lang;
}



language_t * language_load(char * fname) {

  FILE * f = u_fopen(fname, U_READ);

  if (f == NULL) { die("unable to open %s for reading.\n", fname); }

  return language_load(f);
}




void CAT_output(CAT_t * cat, FILE * f) {

  i_fprintf(f, "  %S =", cat->name);
  for (int v = 1; v < cat->traits->nbelems; v++) { // skip unspecified
    i_fprintf(f, " %S", cat->traits->tab[v]);
  }
  fprintf(f, "\n");
}


void language_output(language_t * lang, FILE * f) {

  int i, j;

  i_fprintf(f, "NAME %S\n\n", lang->name);

  for (i = 0; i < lang->POSs->nbelems; i++) {

    POS_t * POS = (POS_t *) lang->POSs->tab[i];

    i_fprintf(f, "POS %S\n", POS->name);

    fprintf(f, "\nflex:\n");
    for (j = 0; j < POS->nbflex; j++) {
      CAT_output((CAT_t *) POS->CATs->tab[j], f);
    }

    fprintf(f, "\ndiscr:\n");
    for (; j < POS->nbdiscr; j++) {
      CAT_output((CAT_t *) POS->CATs->tab[j], f);
    }

    fprintf(f, "\ncat:\n");
    for (;j < POS->CATs->nbelems; j++) {
      CAT_output((CAT_t *) POS->CATs->tab[j], f);
    }

    fprintf(f, "\ncomplet:\n");
    for (symbol_t * symb = POS->codes; symb; symb = symb->next) {

      CAT_t * cat;

      for (j = 0; j < POS->nbdiscr; j++) {

	cat = (CAT_t *) POS->CATs->tab[j];

	if (symb->traits[j] != LOCKED) {

	  if (symb->traits[j] == UNSPECIFIED) {

	    i_fprintf(f, "<%S> ", cat->name);

	  } else { i_fprintf(f, "%S ", cat->traits->tab[symb->traits[j]]); }
	}
      }

      fprintf(f, "\n");
    }

    fprintf(f, ".\n\n");
  }
}


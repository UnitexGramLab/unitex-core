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

#include <assert.h>
#include <ctype.h>

#include "utils.h"
#include "symbol.h"
#include "symbol_op.h"

#define MAXBUF 1024


unichar PUNC_TAB[] = {
  '"', '\'',
  '+', '-', '*', '\\', '=',
  '.', ',', ':', ';', '!', '?',
  '(', ')', '[', ']', '<', '>', '{', '}',
  '%', '#', '@', '/', '$', '&',
  '|', '_',
//  '«', '»',
    0
};


/* symbol_new without call for symbol_type: avoid infinite loop
 */

static inline symbol_t * _symbol_new(POS_t * POS) {

  symbol_t * symb = (symbol_t *) xmalloc(sizeof(symbol_t));

  symb->type = INC;

  symb->negative = false;
  symb->form     = 0;
  symb->canonic  = 0;

  symb->POS = POS;

  if (POS->CATs->nbelems) {

    symb->traits   = (char *) xmalloc(POS->CATs->nbelems  * sizeof(char));
    symb->nbtraits = POS->CATs->nbelems;

  } else {

    symb->traits   = NULL;
    symb->nbtraits = 0;
  }

  int i;

  for (i = 0; i < POS->CATs->nbelems; i++)  { symb->traits[i] = UNSPECIFIED; }

  symb->next = NULL;

  /* symbol_type(symb); don't call symbol_type here */

  return symb;
}


symbol_t * symbol_new(POS_t * POS) {
  symbol_t * res = _symbol_new(POS);
  symbol_type(res);
  return res;
}



symbol_t * symbol_EXCLAM_new() {

  symbol_t * symb = (symbol_t *) xmalloc(sizeof(symbol_t));

  symb->type = EXCLAM;

  symb->negative = false;
  symb->form     = 0;
  symb->canonic  = 0;

  symb->POS = NULL;

  symb->traits   = NULL;
  symb->nbtraits = 0;
  symb->next     = NULL;

  return symb;
}


symbol_t * symbol_EQUAL_new() {

  symbol_t * symb = (symbol_t *) xmalloc(sizeof(symbol_t));

  symb->type    = EQUAL;

  symb->negative = false;
  symb->form    = 0;
  symb->canonic = 0;

  symb->POS = NULL;

  symb->traits   = NULL;
  symb->nbtraits = 0;
  symb->next     = NULL;

  return symb;
}

symbol_t * symbol_LEXIC_new() {

  symbol_t * symb = (symbol_t *) xmalloc(sizeof(symbol_t));

  symb->type    = LEXIC;

  symb->negative = false;
  symb->form     = 0;
  symb->canonic  = 0;

  symb->POS = NULL;

  symb->traits   = NULL;
  symb->nbtraits = 0;
  symb->next     = NULL;

  return symb;
}


symbol_t * symbol_epsilon_new() {

  symbol_t * symb = (symbol_t *) xmalloc(sizeof(symbol_t));

  symb->type    = EPSILON;

  symb->negative = false;
  symb->form    = 0;
  symb->canonic = 0;

  symb->POS = NULL;

  symb->traits   = NULL;
  symb->nbtraits = 0;
  symb->next     = NULL;

  return symb;
}


symbol_t * symbol_PUNC_new(language_t * lang, int canonic) {

  POS_t * POS = language_get_POS(lang, PUNC_STR);

  symbol_t * symb = symbol_new(POS);

  symb->type = ATOM; 

  symb->negative = false;
  symb->form    = 0;
  symb->canonic = canonic;

  return symb;
}


symbol_t * symbol_PUNC_new(int punc) { return symbol_PUNC_new(LANG, punc); }


symbol_t * symbol_CHFA_new(language_t * lang, int canonic) {

  POS_t * POS = language_get_POS(lang, CHFA_STR);

  symbol_t * symb = symbol_new(POS);

  symb->type = ATOM;

  symb->negative = false;
  symb->form    = 0;
  symb->canonic = canonic;

  return symb;
}



symbol_t * symbol_unknow_new(language_t * lang, int form) {

  POS_t * POS = language_get_POS(lang, UNKNOW_STR);

  symbol_t * symb = symbol_new(POS);

  symb->type = ATOM;

  symb->negative = false;
  symb->form    = 0;
  symb->canonic = form;

  return symb;
}



void symbol_empty(symbol_t * symb) {

  free(symb->traits);
  if (symb->negative) { free(symb->negs); }

  symb->type = -1;

  symb->negative = false;
  symb->form    = 0;
  symb->canonic = 0;

  symb->POS = NULL;
  symb->traits = NULL;
  symb->nbtraits = 0;
  symb->next = NULL;
}


void symbol_copy(symbol_t * dest, symbol_t * src) {

  int i;

  symbol_empty(dest);

  dest->type = src->type;

  if ((dest->negative = src->negative) == true) {

    dest->nbnegs = src->nbnegs;
    dest->negs   = (int *) xmalloc(dest->nbnegs * sizeof(int));
    for (i = 0; i < dest->nbnegs; i++) { dest->negs[i] = src->negs[i]; }

  } else {

    dest->form    = src->form;
    dest->canonic = src->canonic;
  }

  dest->POS = src->POS;

  dest->traits = (char *) xmalloc(src->nbtraits * sizeof(char));
  for (i = 0; i < src->nbtraits; i++) { dest->traits[i] = src->traits[i]; }
  dest->nbtraits = src->nbtraits;

  dest->next = src->next;  
}


symbol_t * symbol_dup(const symbol_t * symb) {

  if (symb == SYMBOL_DEF) { fatal_error("symbol_dup(SYMB_DEF) ??? not sure???\n"); }

  if (symb == NULL) { return NULL; }

  symbol_t * res = NULL;
  int i;

  switch (symb->type) {

  case LEXIC:
    res = symbol_LEXIC_new();
    break;

  case EPSILON:
    res = symbol_epsilon_new();
    break;

  case ATOM:
  case INC_CAN: 
  case CODE:
  case INC:
  case INC_NEG:
  case CODE_NEG:

    res = symbol_new(symb->POS);
    res->type    = symb->type;

    if (! symb->negative) {
      res->form    = symb->form;
      res->canonic = symb->canonic;
    } else {
      res->negative = true;
      res->nbnegs   = symb->nbnegs;
      res->negs = (int *) xmalloc(res->nbnegs * sizeof(int));
      for (i = 0; i < res->nbnegs; i++) { res->negs[i] = symb->negs[i]; }
    }
    for (i = 0; i < symb->nbtraits; i++) { res->traits[i] = symb->traits[i]; }
    break;

  case EXCLAM:
    res = symbol_EXCLAM_new();
    break;

  case EQUAL:
    res = symbol_EQUAL_new();
    break;

  default:
    fatal_error("symbol_dup: unknow symbol type: '%c'\n", symb->type);
  }

  return res;
}



symbol_t * symbols_dup(const symbol_t * symbs) {

  symbol_t * res = NULL;

  while (symbs) {
    symbol_t * next = res;
    res = symbol_dup(symbs);
    res->next = next;
    symbs = symbs->next;
  }
  return res;
}


void symbol_delete(symbol_t * symb) {

  if (symb == NULL || symb == SYMBOL_DEF) { return; }

  if (symb->negative) { free(symb->negs); }
  free(symb->traits);

  free(symb);
}


void symbols_delete(symbol_t * symb) {

  while (symb && symb != SYMBOL_DEF) {
    symbol_t * next = symb->next;
    symbol_delete(symb);
    symb = next;
  }
}



void symbols_concat(symbol_t * a, symbol_t * b, symbol_t ** end) {

  if (a == NULL) { fatal_error("symbol_concat: a == NULL\n"); }

  while (a->next) { a = a->next; }

  a->next = b;

  if (end) {
    while (a->next) { a = a->next; }
    *end = a;
  }
}




/* return how many codes in POS match with s
 * for each features whose value are the same in each matching code
 * set it in pcode (if non null)
 */


static int symbol_match_codes(symbol_t * s, symbol_t * pcode = NULL) {

  int count = 0;


  for (symbol_t * code = s->POS->codes; code; code = code->next) {

    bool ok = true;

    for (int i = 0; ok && i < s->POS->nbdiscr; i++) {

      switch (code->traits[i]) {

      case UNSPECIFIED:
	fatal_error("match_codes: in POS '%S': code with UNSPEC discr code\n", s->POS->name);

      default:
	if ((s->traits[i] != UNSPECIFIED) && (s->traits[i] != code->traits[i])) { ok = false; }
	break;
      }
    }

    if (ok) {

      if (pcode) {

	if (count == 0) { // copy first matching code in pcode

	  for (int i = 0; i < s->POS->nbdiscr; i++) { pcode->traits[i] = code->traits[i]; }

	} else { // set to UNSPEC features which divergent

	  for (int i = 0; i < s->POS->nbdiscr; i++) {
	    if (pcode->traits[i] != code->traits[i]) { pcode->traits[i] = UNSPECIFIED; }
	  }
	}
      }

      /*
      debug("match_codes:\n");
      symbol_dump(s); fprintf(stderr, " matches with "); symbol_dump(code); fprintf(stderr, "\n");
      */
      count++;
    }
  }

  return count;
}


/*
 * set the symbol type:
 * * if canonic starts with '!' it's NEGATIF (should probably split NEG into NEG_INC and NEG_CODE)
 * * if its traits matches with only one full label, then its a CODE or ATOM (depending if it contains a canonic form)
 *   and we lock all discriminant traits which are set to UNSPECIFIED.
 * * if it doesn't match any then it is an invalid symbol
 * * if all discr traits are fixed then it is also a code (or atom)
 * * else it is an INCOMPLET code.
 */


int symbol_type(symbol_t * symb) {

  symb->type = INC;

  if (symb->POS->codes) {

    symbol_t * mcode = _symbol_new(symb->POS);

    int count = symbol_match_codes(symb, mcode);

    if (count == 0) { // symbol doesn't match any POS code -> invalid
      symb->type = -1;
      symbol_delete(mcode);
      return symb->type;
    }

    if (count == 1) { // it's a code we set discr values to matching code's one

      symb->type = CODE;
      for (int i = 0; i < symb->POS->nbdiscr; i++) { symb->traits[i] = mcode->traits[i]; }

    } else { // we try to lock some traits wich are same value on all matching codes

      symb->type = INC;
      for (int i = 0; i < symb->POS->nbdiscr; i++) {
	if (symb->traits[i] == UNSPECIFIED) { symb->traits[i] = mcode->traits[i]; }
      }      
    }

    symbol_delete(mcode);
  }


  if (symb->type == INC) { // if all discr traits are fixed, it's a code

    int i;
    for (i = 0; i < symb->POS->nbdiscr; i++) { if (symb->traits[i] == UNSPECIFIED) { break; } }

    if (i == symb->POS->nbdiscr) { symb->type = CODE; }
  }


  /* look for canonical */

  if (symb->type == INC) {

    if (symb->negative) {
      symb->type = INC_NEG;
    } else if (symb->canonic) {
      symb->type = INC_CAN;
    }

    return symb->type;
  }

  /* type == code */

  if (symb->negative) {
    symb->type = CODE_NEG;
  } else if (symb->canonic) {
    symb->type = ATOM;
  }

  return symb->type;
}



void symbol_dump_all(const symbol_t * symb, FILE * f) {

  static const unichar locked[] = { 'l', 'o', 'c', 'k', 'e', 'd', 0 };

  int i;
  language_t * lang = symb->POS ? symb->POS->lang : LANG;

  i_fprintf(f, "<%c:", symb->type);

  if (symb->negative) {
    for (i = 0; i < symb->nbnegs; i++) {
      i_fprintf(f, "!%S", language_get_form(lang, symb->negs[i]));
    }
  } else {
    i_fprintf(f, "%S,%S", language_get_form(lang, symb->form), language_get_form(lang, symb->canonic));
  }


  if (symb->POS) {

    i_fprintf(f, ".%S", symb->POS->name);

    for (i = symb->POS->nbflex; i < symb->POS->CATs->nbelems; i++) {
      CAT_t * CAT = POS_get_CAT(symb->POS,i);
      i_fprintf(f, "+%S=%S", CAT->name, (symb->traits[i] < 0) ? locked : (unichar *) CAT->traits->tab[symb->traits[i]]);
    }

    fprintf(f, ":");

    for (i = 0; i < symb->POS->nbflex; i++) {
      CAT_t * CAT = POS_get_CAT(symb->POS,i);
      i_fprintf(f, "+%S=%S", CAT->name, (symb->traits[i] < 0) ? locked : (unichar *) CAT->traits->tab[symb->traits[i]]);
    }

  } else {
    fprintf(f, ".*");
  }

  fprintf(f, ">");
}



void symbol_to_text_label(const symbol_t * s, ustring_t * ustr) {

  if (s == NULL) { fatal_error("symbol 2 text label: symb is null\n"); }

  if (s == SYMBOL_DEF) { fatal_error("symb2txt: symb is <def>\n"); }

  if (s->type != ATOM) {
    error("symbol2txt: symbol '"); symbol_dump(s); fatal_error("' is'nt an atom.\n");
  }

  language_t * lang = s->POS->lang;

  if ((u_strcmp(s->POS->name, UNKNOW_STR) == 0) || (u_strcmp(s->POS->name, PUNC_STR) == 0) || (u_strcmp(s->POS->name, CHFA_STR) == 0)) {
    ustring_copy(ustr, language_get_form(lang, s->canonic));
    return;
  }

  ustring_printf(ustr, "{%S,%S.%S", language_get_form(lang, s->form), language_get_form(lang, s->canonic), s->POS->name);

  int i;
  CAT_t * CAT;

  for (i = s->POS->nbflex; i < s->nbtraits; i++) {
    if (s->traits[i] > 0) {
      CAT = POS_get_CAT(s->POS, i);
      ustring_concatf(ustr, "+%S", CAT_get_valname(CAT, s->traits[i]));
    }
  }


  bool colons = false;

  for (i = 0; i < s->POS->nbflex; i++) {

    CAT = POS_get_CAT(s->POS, i);

    if (s->traits[i] > 0) {
      if (colons == false) { ustring_concat(ustr, ":"); colons = true; }
      ustring_concat(ustr, CAT_get_valname(CAT, s->traits[i]));
    }
  }

  ustring_concat(ustr, "}");
}


void symbol_to_implosed_text_label(const symbol_t * s, ustring_t * ustr) {

  if (s == NULL) { fatal_error("symbol to text label: symb is null\n"); }

  if (s == SYMBOL_DEF) { fatal_error("symb2txt: symb is <def>\n"); }

  if (s->type != ATOM) {
    error("symbol2txt: symbol '"); symbol_dump(s); fatal_error("' is'nt an atom.\n");
  }

  language_t * lang = s->POS->lang;

  if ((u_strcmp(s->POS->name, UNKNOW_STR) == 0) || (u_strcmp(s->POS->name, PUNC_STR) == 0) || (u_strcmp(s->POS->name, CHFA_STR) == 0)) {
    ustring_copy(ustr, language_get_form(lang, s->canonic));
    return;
  }

  ustring_printf(ustr, "{%S,%S.%S", language_get_form(lang, s->form), language_get_form(lang, s->canonic), s->POS->name);

  int i;
  CAT_t * CAT;

  for (i = s->POS->nbflex; i < s->nbtraits; i++) {
    if (s->traits[i] > 0) {
      CAT = POS_get_CAT(s->POS, i);
      ustring_concatf(ustr, "+%S", CAT_get_valname(CAT, s->traits[i]));
    }
  }


  for (; s; s = s->next) {

    bool colons = false;

    for (i = 0; i < s->POS->nbflex; i++) {

      if (s->traits[i] > 0) {

	CAT = POS_get_CAT(s->POS, i);

	if (colons == false) { ustring_concat(ustr, ":"); colons = true; }
	ustring_concat(ustr, CAT_get_valname(CAT, s->traits[i]));
      }
    }
  }

  ustring_concat(ustr, "}");
}



void symbol_to_locate_label(const symbol_t * s, ustring_t * ustr) {

  if (s == NULL) { fatal_error("symb2locate label: symb is null\n"); }
  if (s == SYMBOL_DEF) { error("symb2locate: symb is <def>\n"); ustring_copy(ustr, "<def>"); return; }

  switch (s->type) {

  case LEXIC:
    ustring_copy(ustr, "<.>");
    return;

  case EPSILON:
    ustring_copy(ustr, "<E>");
    return;

  case EXCLAM:
    error("'<!>' in concordance fst2 ???\n");
    ustring_copy(ustr, "!");
    return;

  case EQUAL:
    error("'<=>' in concordance fst2 ???\n");
    ustring_copy(ustr, "=");
    return;
  }

  language_t * lang = s->POS->lang;

  if ((u_strcmp(s->POS->name, UNKNOW_STR) == 0)) { // unknow word
    ustring_copy(ustr, "<MOT>");
    return;
  } else if ((u_strcmp(s->POS->name, PUNC_STR) == 0) || (u_strcmp(s->POS->name, CHFA_STR) == 0)) {
    if (s->canonic) {
      //      debug("CHFA(%S) : canonic=%S\n", s->POS->name, s->canonic);
      ustring_copy(ustr, language_get_form(lang, s->canonic));
      return;
    }
  }


  if (! s->negative && s->canonic) {
    ustring_printf(ustr, "<%S.%S", language_get_form(lang, s->canonic), s->POS->name);
  } else {
    ustring_printf(ustr, "<%S", s->POS->name);
  }

  int i;
  CAT_t * CAT;

  for (i = s->POS->nbflex; i < s->nbtraits; i++) {

    if (s->traits[i] > 0) {
      CAT = POS_get_CAT(s->POS, i);
      ustring_concatf(ustr, "+%S", CAT_get_valname(CAT, s->traits[i]));
    }
  }


  bool colons = false;

  for (i = 0; i < s->POS->nbflex; i++) {

    CAT = POS_get_CAT(s->POS, i);

    if (s->traits[i] > 0) {

      if (colons == false) { ustring_concat(ustr, ":"); colons = true; }
      ustring_concat(ustr, CAT_get_valname(CAT, s->traits[i]));

    }
  }

  ustring_concat(ustr, ">");
}



void symbol_to_grammar_label(const symbol_t * s, ustring_t * ustr) {

  //  debug("symbtogrammlabel\n"); symbol_dump(s); endl();
  
  if (s == NULL) { fatal_error("symb2grammar label: symb is null\n"); }
  if (s == SYMBOL_DEF) { ustring_copy(ustr, "<def>"); return; }

  switch (s->type) {
  case LEXIC:
    ustring_copy(ustr, "<.>");
    return;
  case EPSILON:
    ustring_copy(ustr, "<E>");
    return;
  case EXCLAM:
    ustring_copy(ustr, "<!>");
    return;
  case EQUAL:
    ustring_copy(ustr, "<=>");
    return;
  }


  language_t * lang = s->POS->lang;

  if (s->negative) {

    ustring_copy(ustr, "<");
    for (int i = 0; i < s->nbnegs; i++) {
      ustring_concatf(ustr, "!%S", language_get_form(lang, s->negs[i]));
    }
    ustring_concatf(ustr, ".%S", s->POS->name);

  } else if (s->canonic) {

    ustring_printf(ustr, "<%S.%S", language_get_form(lang, s->canonic), s->POS->name);

  } else {    

    ustring_printf(ustr, "<%S", s->POS->name);
  }

  int i;
  CAT_t * CAT;

  for (i = s->POS->nbflex; i < s->nbtraits; i++) {

    if (s->traits[i] > 0) {

      CAT = POS_get_CAT(s->POS, i);
      ustring_concatf(ustr, "+%S", CAT_get_valname(CAT, s->traits[i]));

    } else if (s->traits[i] == LOCKED) {
      /* Pour specifier que l'attribut est bloque on l'ecrit !attrname 
       * ex : !discr
       */
      CAT = POS_get_CAT(s->POS, i);
      ustring_concatf(ustr, "!%S", CAT->name);
    }
  }


  bool colons = false;

  for (i = 0; i < s->POS->nbflex; i++) {

    CAT = POS_get_CAT(s->POS, i);

    if (s->traits[i] > 0) {

      if (colons == false) { ustring_concat(ustr, ":"); colons = true; }
      ustring_concat(ustr, CAT_get_valname(CAT, s->traits[i]));

    } else if (s->traits[i] == LOCKED) {

      /* pour specifier que le code de flexion est bloquer on l'ecrit @valeur
       * ex: @m signifie la meme chose que @f puisque m et f sont dans la meme categorie genres
       */

      if (colons == false) { ustring_concat(ustr, ":"); colons = true; }
      ustring_concatf(ustr, "@%S", CAT_get_valname(CAT, 1));
    }
  }

  ustring_concat(ustr, ">");
}



void symbol_to_str(const symbol_t * s, ustring_t * ustr) {

  if (s == NULL) { ustring_copy(ustr, "nil"); return; }
  if (s == SYMBOL_DEF) { ustring_copy(ustr, "<def>"); return; }

  switch (s->type) {
  case LEXIC:
    ustring_copy(ustr, "<.>");
    return;
  case EPSILON:
    ustring_copy(ustr, "<E>");
    return;
  case EXCLAM:
    ustring_copy(ustr, "<!>");
    return;
  case EQUAL:
    ustring_copy(ustr, "<=>");
    return;
  }

  language_t * lang = s->POS->lang;

  ustring_printf(ustr, "<%c:", s->type);

  if (s->negative) {

    for (int i = 0; i < s->nbnegs; i++) {
      ustring_concatf(ustr, "!%S(%d)", language_get_form(lang, s->negs[i]), s->negs[i]);
      //ustring_concatf(ustr, "!%S", language_get_form(lang, s->negs[i]));
    }

  } else {

    if (s->form)    { ustring_concatf(ustr, "%S,", language_get_form(lang, s->form)); }
    if (s->canonic) { ustring_concatf(ustr, "%S", language_get_form(lang, s->canonic)); }
  }

  ustring_concatf(ustr, ".%S", s->POS->name);

  int i;
  CAT_t * CAT;

  for (i = s->POS->nbflex; i < s->nbtraits; i++) {
    if (s->traits[i] > 0) {
      CAT = POS_get_CAT(s->POS, i);
      ustring_concatf(ustr, "+%S", CAT_get_valname(CAT, s->traits[i]));
    } else if (s->traits[i] == LOCKED) {
      CAT = POS_get_CAT(s->POS, i);
      ustring_concatf(ustr, "!%S", CAT->name);
    }
  }

  ustring_concat(ustr, ":");

  for (i = 0; i < s->POS->nbflex; i++) {

    CAT = POS_get_CAT(s->POS, i);

    if (s->traits[i] > 0) {
      ustring_concat(ustr, CAT_get_valname(CAT, s->traits[i]));
    } else if (s->traits[i] == LOCKED) {
      ustring_concatf(ustr, "!{%S}", CAT->name);
    }
  }  

  ustring_concat(ustr, ">");
}


void symbol_dump(const symbol_t * s, FILE * f) {
  ustring_t * ustr = ustring_new();
  symbol_to_str(s, ustr);
  i_fprintf(f, "%S", ustr->str);
  ustring_delete(ustr);
}


void symbols_dump(const symbol_t * s, FILE * f) {

  i_fprintf(f, "(");
  while (s) { symbol_dump(s, f); if ((s = ((s == SYMBOL_DEF) ? NULL : s->next))) { fprintf(f, ", "); } }
  i_fprintf(f, ")");
}

/* extract different label parts
 * at least pos should be non null
 */


// form,can.POS [ +traits:flex ]
// can.POS [ +traits:flex ]
// POS [ +traits:flex ]
// POS


static inline void split_dic_label(unichar * label, unichar ** form, unichar ** canonic, unichar ** pos,
				   unichar ** traits, unichar ** flex) {

  unichar * p = label;
  unichar * q;

  if ((q = u_strchr(p, ',')) != NULL) {
    *q = 0;
    *form = p;
    p = q + 1;

  } else { *form = NULL; }


  if ((q = u_strchr(p, '.')) != NULL) {
    *q = 0;
    *canonic = p;
    p = q + 1;

  } else { *canonic = NULL; }


  *pos = p;

  if ((q = u_strchr(p, '+')) != NULL) {

    *q = 0;
    *traits = q + 1;
    p = q + 1;

  } else { *traits = NULL; }


  if ((q = u_strchr(p, ':')) != NULL) {

    *q = 0;
    *flex = q + 1;

  } else { *flex = NULL; }

}




inline int check_dic_entry(const unichar * label) {

  /* {__,__.__} */

  if (*label != '{') { error("'%S': malformed DELA entry ('{' is missing)\n", label); return -1; }

  const unichar * p = label + 1;

  while (*p !=  ',') { // look for ','
    if (*p == 0) { error("'%S': malformed DELA entry (',' is missing).\n", label); return -1; }
    p++;
  }

  while (*p != '.') {
    if (*p == 0) { error("'%S': malformed DELA entry ('.' is missing).\n", label); return -1; }
    p++;
  }

  while (*p != '}') {
    if (*p == 0) { error("'%S': malformed DELA entry: '%S' ('}' is missing).\n", label); return -1; }
    p++;
  }

  p++;
  if (*p != 0) { error("'%S': malformed label: label should end with '}'.\n", label); return -1; }

  return 0;
}





/* text label can be either a full DELA entry, either a punc symbol, either an unknow word
 */

static inline int check_text_label(unichar * label) {


  if ((label == NULL)) { error("check label: no label\n"); return -1; }

  if (*label == 0) { error("error: check label: label is empty\n"); return -1; }

  if (*label == '{' && *(label + 1)) { return check_dic_entry(label); }


  /* no spaces */

  // DON'T check for spaces ! (cause errors in GREEK)
  //for (unichar * p = label; *p; p++) { if (isspace(*p)) { error("malformed label: '%S'.\n", label); return -1; } }

  return 0;
}



/* load a label of the form {form,canonic.POS[+traits]*[:flex]*}
 * buf is a writeable copy of label
 */


symbol_t * load_dic_entry(language_t * lang, const unichar * label, unichar * buf, bool warnmissing) {

  buf[u_strlen(buf) - 1] = 0; // chomp trailing '}'

  unichar * form, * canonic, * pos, * traits, * flexs;

  split_dic_label(buf + 1, & form, & canonic, & pos, & traits, & flexs);

  POS_t * POS = language_get_POS(lang, pos);

  if (POS == NULL) {
    error("'%S': unknow POS '%S'\n", label, pos);
    return NULL;
  }

  symbol_t * symb = symbol_new(POS);
  symbol_t * model;

  symb->type = ATOM;

  symb->form    = language_add_form(lang, form);
  symb->canonic = *canonic ? language_add_form(lang, canonic) : symb->form;


  /* additionnal traits ... */

  // we lock all featuress which are not explicitly set

  for (int i = 0; i < POS->CATs->nbelems; i++) { symb->traits[i] = LOCKED; }


  unichar * p = u_strtok_char(traits, "+");

  while (p) {

    trait_info_t * info = POS_get_trait_infos(POS, p);

    if (info) {

      symb->traits[info->CATid] = info->val;

    } else if (warnmissing) {

      if (hash_str_table_idx_lookup(lang->unknow_codes, p) == -1) {
	error("in symbol '%S': unknow value '%S', will not be taken into account\n", label, p);
	hash_str_table_add(lang->unknow_codes, p, NULL);
      }
    }

    p = u_strtok_char(NULL, "+");
  }


  /* flexional codes */

  p = u_strtok_char(flexs, ":");

  if (p == NULL) { // no flexionnal code

    if (POS->codes && (symbol_match_codes(symb) == 0)) {
      error("'%S': doesn't match with POS '%S' definition\n", label, POS->name);
      goto err_symb;
    }

    return symb;
  }


  model = symb;
  symb = NULL;

  while (p) {

    symbol_t * nouvo = symbol_dup(model);

    for (; *p; p++) {

      trait_info_t * infos = POS_get_flex_infos(POS, *p);

      if (infos == NULL) { error("'%S': unknow flexionnal code '%C'\n", label, *p); goto err_model; }

      nouvo->traits[infos->CATid] = infos->val;
    }


    if (POS->codes && (symbol_match_codes(nouvo) == 0)) {
      error("'%S': doesn't match with POS '%S' definition\n", label, POS->name);
      goto err_model;
    }

    nouvo->next = symb;
    symb = nouvo;

    p = u_strtok_char(NULL, ":");
  }

  symbol_delete(model);
  
  return symb;

err_model:
  symbol_delete(model);

err_symb:
  symbols_delete(symb);

  return NULL;
}



symbol_t * load_text_symbol(language_t * lang, unichar * label) {

  unichar buf[u_strlen(label) + 1];
  u_strcpy(buf, label);

  if (check_text_label(buf) == -1) {
    error("bad format in text symbol: '%S'\n", label);
    return NULL;
  }

  if (u_strcmp_char(label, "<E>") == 0) {
    return symbol_epsilon_new();
  }

  if (u_strcmp_char(label, "<def>") == 0) { fatal_error("<def> trans in text automaton!\n"); }

  if (*buf == '{' && buf[1]) {   /*  dictionnary entry ( {__,__.__} ) */
    return load_dic_entry(lang, label, buf);
  }


  /* mot inconnu dans un texte ou ponctuation */

  int idx = language_add_form(lang, buf);

  if (u_strchr(PUNC_TAB, *buf)) {          /* ponctuation */

    if (buf[1] && buf[0] != '\\') { fatal_error("bad text symbol '%S' (ponctuation too long)\n", label); }

    return symbol_PUNC_new(lang, idx);

  } else if (u_is_digit(*buf)) {     /* chiffre arabe */

    for (unichar * p = buf; *p; p ++) { if (! u_is_digit(*p)) { fatal_error("bad symbol : '%S' (mixed nums and chars)\n", label); } }

    return symbol_CHFA_new(lang, idx);
  }

  /* unknow word  */

  return symbol_unknow_new(lang, idx);
}


/* LEXIC_minus_POS: in this file because we need it in load_gramm_symbol fonction
 */

symbol_t * LEXIC_minus_POS(POS_t * POS) {

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  for (int i = 0; i < LANG->POSs->nbelems; i++) {

    POS_t * POS2 = (POS_t *) LANG->POSs->tab[i];

    if (POS2 == POS) { continue; }

    symbols_concat(end, symbol_new(POS2), & end);
  }

  return res.next;
}




static void fill_neglist(language_t * lang, symbol_t * s, unichar * canonic) {

  unichar * p;
  int nbnegs;
  for (p = canonic, nbnegs = 0; *p; p++) { if (*p == '!') { nbnegs++; } }


  s->negs = (int *) xmalloc(nbnegs * sizeof(int));
  s->nbnegs = 0;

  p = u_strtok_char(canonic, "!");

  while (p) {

    assert(s->nbnegs < nbnegs);

    int idx = language_add_form(lang, p);
    
    /* bubble sort */

    int pos = s->nbnegs; /* pos for (bubble) position (not part of speech here) */
    s->negs[pos] = idx;

    while (pos && (s->negs[pos - 1] > s->negs[pos])) {
      int tmp = s->negs[pos - 1];
      s->negs[pos - 1] = s->negs[pos];
      s->negs[pos] = tmp;
      pos--;
    }

    if (pos && (s->negs[pos - 1] == s->negs[pos])) { // bubble expulsion (doublon in neg list)

      error("bubble expulsion (%S)\n", p);

      while (pos < s->nbnegs) {
	int tmp = s->negs[pos + 1];
	s->negs[pos + 1] = s->negs[pos];
	s->negs[pos] = tmp;
	pos++;
      }
    } else { s->nbnegs++; }

    p = u_strtok_char(NULL, "!");
  }

  if (s->nbnegs != nbnegs) { error("in fill_neglist: different nbnegs?\n"); }
}


/* load a symbol which has the generic form '<canonic.POS+trait:flex>'
 * called from load_grammar_symbol only
 */

static symbol_t * load_gram_symbol(language_t * lang, const unichar * label, unichar * buf) {

  /* etiquette incomplete */

  int len = u_strlen(buf);
  if (buf[len - 1] != '>') { fatal_error("bad grammar symbol: '%S'\n", label); }

  buf[len - 1] = 0; // chomp trailing '>'

  unichar * p;
  unichar * canonic, * pos;

  if ((p = u_strchr(buf, '.')) != NULL) { //form canonic or negative form is present
    canonic = buf + 1;
    *p = 0;
    pos = p + 1;

  } else { canonic = NULL; pos = buf + 1; }


  if (*pos == '!') { // negation d'un code grammatical (ex: <!PNC>)

    POS_t * POS = language_get_POS(lang, pos + 1);

    if (POS == NULL) { fatal_error("in symbol '%S': unknow part of speech '%S'\n", label, pos + 1); }

    return LEXIC_minus_POS(POS);
  }

  
  unichar next;

  if ((p = u_strpbrk(pos, "+!:")) != NULL) { // additionnals features or flexional codes
    next = *p;
    *p   = 0;
  } else { next = 0; } // POS only


  POS_t * POS = language_get_POS(lang, pos);

  if (POS == NULL) { fatal_error("in symbol '%S': unknow part of speech '%S'\n", label, pos); }


  symbol_t * symb = symbol_new(POS);

  if (canonic && *canonic) {

    if (*canonic != '!') {

      symb->canonic = language_add_form(lang, canonic); 

    } else { // negative symbol

      symb->negative = true;
      fill_neglist(lang, symb, canonic);
    }
  }


  /* additionnal traits ... */

  unichar type = next;
  unichar * attr = p + 1;

  while ((type == '+') || (type == '!')) {

    if ((p = u_strpbrk(attr, "+!:")) != NULL) { // c'est pas termine
      next = *p;
      *p   = 0;
    } else { next = 0; }


    if (type == '+') { // attribute is set

      trait_info_t * info = POS_get_trait_infos(POS, attr);

      if (info) {

	symb->traits[info->CATid] = info->val;

      } else {

	if (hash_str_table_idx_lookup(lang->unknow_codes, attr) == -1) {
	  error("in symbol '%S': unknow attribute '%S', will not be taken into account\n", label, attr);
	  hash_str_table_add(lang->unknow_codes, attr, NULL);
	}
      }

    } else { // feature is locked (type == '!')

      int idx = POS_get_CATid(POS, attr);
      if (idx == -1) { fatal_error("in symbol '%S': unknow feature '%S'\n", label, attr); }
      if (idx < POS->nbflex) { fatal_error("in symbol '%S': '%S' is a flexionnal feature!\n", label, attr); }
      if (symb->traits[idx] != UNSPECIFIED) { fatal_error("in symbol '%S': '%S' cannot be locked and set\n", label, attr); }
      symb->traits[idx] = LOCKED;
    }

    attr = p + 1;
    type = next;
  }


  /* flexional codes */

  if (type == 0) { // no flexionnal code
    if (symbol_type(symb) == -1) { fatal_error("'%S' is not a valid\n", label); }
    return symb;
  }


  assert(type == ':');
  assert(p);

  symbol_t * model = symb;
  symb = NULL;

  while (p) { // still flex code sequence

    // debug("attr=%S\n", attr);

    if ((p = u_strchr(attr, ':')) != NULL) { // c'est pas termine
      *p = 0;
    }

    symbol_t * nouvo = symbol_dup(model);

    for (; *attr; attr++) {

      trait_info_t * infos;

      if (*attr == '@') { // flexionnal feature is locked
	attr++;
	if ((infos = POS_get_flex_infos(POS, *attr)) == NULL) {
	  fatal_error("in symbol '%S': unknow flex code '%C'\n", label, *attr);
	}
	nouvo->traits[infos->CATid] = LOCKED;
	
      } else { // flexionnal code is set

	if ((infos = POS_get_flex_infos(POS, *attr)) == NULL) {
	  fatal_error("in symbol '%S': unknow flexionnal code '%C'\n", label, *attr);
	}

	nouvo->traits[infos->CATid] = infos->val;
      }
    }

    if (symbol_type(nouvo) == -1) { fatal_error("'%S' is not a valid symbol\n", label); }

    nouvo->next = symb;
    symb = nouvo;

    attr = p + 1;
  }

  symbol_delete(model);

  return symb;
}


/* load a symbol from an Elag grammar label
 */

symbol_t * load_grammar_symbol(language_t * lang, unichar * label) {

  unichar buf[u_strlen(label) + 1];
  u_strcpy(buf, label);

  if (*buf == '{' && buf[1]) {   /*  dictionnary entry ( {__,__.__} ) */

    if (u_strcmp_char(buf, "{S}") == 0) { return symbol_PUNC_new(lang, language_add_form(lang, buf)); } // limite de phrase

    error("'%S': DELAS entry in Elag grammar????\n", label);

    if (check_dic_entry(buf) == -1) { fatal_error("bad grammar label '%S'\n", label); }

    return load_dic_entry(lang, label, buf);
  }


  /* mot inconnu dans un texte ou ponctuation */

  if (*buf == '<' && *(buf + 1)) { // etiquette

    /* EPSILON */

    if (u_strcmp_char(buf, "<E>") == 0) { return symbol_epsilon_new(); }

    /* UNIVERSEL */

    if (u_strcmp_char(buf, "<.>") == 0) { return symbol_LEXIC_new(); }


    /* special def label */

    if (u_strcmp_char(buf, "<def>") == 0) { return SYMBOL_DEF; }


    /* special EXCLAM symbol */

    if (u_strcmp_char(buf, "<!>") == 0) { return symbol_EXCLAM_new(); }


    /* special EQUAL symbol */

    if (u_strcmp_char(buf, "<=>") == 0) { return symbol_EQUAL_new(); }


    /* etiquette incomplete */

    return load_gram_symbol(lang, label, buf);
  }



  /* special EXCLAM symbol */

  if (*buf == '!' && buf[1] == 0) { return symbol_EXCLAM_new(); }


  /* special EQUAL symbol */

  if (*buf == '=' && buf[1] == 0) { return symbol_EQUAL_new(); }



  /* ponctuation */

  int idx = language_add_form(lang, buf);

  if (u_strchr(PUNC_TAB, *buf)) {

    if (*buf == '\\' && (! buf[1] || buf[2])) { fatal_error("bad PUNC symbol '%S'\n", label); }
    if (buf[1] && buf[0] != '\\') { fatal_error("bad symbol '%S' (PONC too long)\n", label); }

    return symbol_PUNC_new(lang, idx);
  }


  /* chiffre arabe */

  if (u_is_digit(*buf)) {

    for (unichar * p = buf; *p; p ++) {
      if (! u_is_digit(*p)) { fatal_error("bad symbol : '%S' (mixed nums and chars)\n", label); }
    }

    return symbol_CHFA_new(lang, idx);
  }


  /* unknow word  */

  error("label '%S': unknow word in grammar???\n", label);

  return symbol_unknow_new(lang, idx);
}



#if 0
static bool symbol_equals_traits(symbol_t * a, symbol_t * b) {

  if (b->nbtraits < a->nbtraits) {
    symbol_t * tmp = a;
    a = b;
    b = tmp;
  }

  int i;
  for (i = 0; i < a->nbtraits; i++) {
    if (a->traits[i] != b->traits[i]) { return false; }
  }

  for (; i < b->nbtraits; i++) {
    if (b->traits[i] != UNSPECIFIED) { return false; }
  }
  return true;
}


bool symbol_equals(symbol_t * a, symbol_t * b) {

  if (a == b) { return true; }

  if (a == NULL || b == NULL || a == SYMBOL_DEF || b == SYMBOL_DEF) { return false; }

  if ((a->type != b->type) || (a->POS != b->POS)) { return false; }

  if (a->canonic) {

    if ((! b->canonic) || u_strcmp(a->canonic, b->canonic)) { return false; }

  } else if (b->canonic) { return false; }

  return symbol_equals_traits(a, b);
}
#endif

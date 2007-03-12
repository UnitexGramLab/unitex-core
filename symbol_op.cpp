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

#include <assert.h>

#include "utils.h"
#include "ustring.h"
#include "symbol.h"
#include "symbol_op.h"


#define MIN(a, b) ((a <= b) ? a : b)


static inline void traits_copy(symbol_t * dest, const symbol_t * src) {
  for (int i = 0; i < src->nbtraits; i++) { dest->traits[i] = src->traits[i]; }
}

/* orders the symbol's types
 */


static char * make_TYPE_ORDER(char * TAB) {

  //  debug("make_TYPE_ORDER\n");

  for (int i = 0; i < 256; i++) { TAB[i] = -1; }

  TAB[LEXIC]     =  0;
  TAB[EPSILON]   =  1;

  TAB[ATOM]      =  2;
  TAB[CODE_NEG]  =  3;
  TAB[CODE]      =  4;

  TAB[INC_CAN]   =  5;
  TAB[INC_NEG]   =  6;
  TAB[INC]       =  7;

  TAB[EXCLAM]    = -2;
  TAB[EQUAL]     = -2;

  return TAB;
}


static char  _TYPE_ORDER_[256];
static char * TYPE_ORDER = make_TYPE_ORDER(_TYPE_ORDER_);

static inline int type_order(int type) {

  if (type < 0 || type > 255) { // out of bounds
    return -5;
  }

  return TYPE_ORDER[type];
}


/* set symbols types
 * && strip invalid symbols from the list
 */

static inline symbol_t * symbols_clean(symbol_t * symbols) {

  symbol_t * res = symbols;

  while (symbols && (symbol_type(symbols) == -1)) {
    res = symbols->next;
    symbol_delete(symbols);
    symbols = res;
  }

  for (symbol_t * s = res; s; s = s->next) {
    while (s->next && (symbol_type(s->next) == -1)) {
      symbol_t * next = s->next->next;
      symbol_delete(s->next);
      s->next = next;
    }
  }

  return res;
}



static inline bool canonic_in_neg(int canonic, const symbol_t * s) {
  for (int i = 0; i < s->nbnegs; i++) { if (canonic == s->negs[i]) { return true; } }
  return false;
}




/* intersection of 2 negation lists (avoid duplication) */


static void negs_inter_negs(symbol_t * res, const symbol_t * a, const symbol_t * b) {

  int max = a->nbnegs + b->nbnegs;

  res->nbnegs = 0;
  res->negs   = (int *) xmalloc(max * sizeof(int));

  int c1 = 0, c2 = 0;

  while ((c1 < a->nbnegs) && (c2 < b->nbnegs)) {

    if (a->negs[c1] == b->negs[c2]) {

      res->negs[res->nbnegs] = a->negs[c1];
      res->nbnegs++; c1++; c2++;

    } else if (a->negs[c1] < b->negs[c2]) {

      res->negs[res->nbnegs] = a->negs[c1];
      res->nbnegs++; c1++;

    } else {
      res->negs[res->nbnegs] = b->negs[c2];
      res->nbnegs++; c2++;
    }
  }

  while (c1 < a->nbnegs) {
    res->negs[res->nbnegs] = a->negs[c1];
    res->nbnegs++; c1++;
  }

  while (c2 < b->nbnegs) {
    res->negs[res->nbnegs] = b->negs[c2];
    res->nbnegs++; c2++;
  }
}





/* symbols comparaison */

static inline int compare_traits(const symbol_t * a, const symbol_t * b) {

  int min = MIN(a->nbtraits, b->nbtraits);

  int i;
  for (i = 0; i < min; i++) {
    if (a->traits[i] != b->traits[i]) { return b->traits[i] - a->traits[i]; }
  }

  for (; i < a->nbtraits; i++) {
    if (a->traits[i] != UNSPECIFIED) { return UNSPECIFIED - a->traits[i]; }
  }

  for (; i < b->nbtraits; i++) {
    if (b->traits[i] != UNSPECIFIED) { return b->traits[i] - UNSPECIFIED; }
  }

  return 0;
}


int symbol_compare(const symbol_t * a, const symbol_t * b) {

  int cmp;

  //  debug("compare("); symbol_dump(a); errprintf(","); symbol_dump(b); errprintf(")\n");

  if (a == b) { return 0; }

  if (a == NULL) { /* NULL == le plus petit symbole */
    error("symbol_compare: a == NULL\n");
    return -1;
  }

  if (b == NULL) {
    error("symbol_compare: b == NULL\n");
    return 1;
  }

  if (a == SYMBOL_DEF) { /* SYMBOL_DEF == deuxieme plus petit symbol */
    error("symbol_compare a == <def>\n");
    return -1;
  }

  if (b == SYMBOL_DEF) {
    error("symbol_compare b == <def>\n");
    return 1;
  }


  if (type_order(a->type) < 0) { fatal_error("symbol_compare: invalid symbol for a (type=%d)\n", a->type); }
  if (type_order(b->type) < 0) { fatal_error("symbol_compare: invalid symbol for b (type=%d)\n", b->type); }

  if (type_order(a->type) != type_order(b->type)) { return type_order(b->type) - type_order(a->type); }

  /* same type */

  /* first compare POSs */
  
  if (a->POS != b->POS) { return b->POS - a->POS; }

  /* next compare by traits */

  if ((cmp = compare_traits(a, b))) { return cmp; }


  /* end then compare strings */

  switch (a->type) {

  case ATOM:  
  case INC_CAN:
    if (a->canonic != b->canonic) { return b->canonic - a->canonic; }
    if (a->form != b->form) { return b->form - a->form; }
    break;

  case CODE_NEG:
  case INC_NEG:
    if (a->nbnegs != b->nbnegs) { return b->nbnegs - a->nbnegs; }
    for (int i = 0; i < a->nbnegs; i++) { if (a->negs[i] != b->negs[i]) { return b->negs[i] - a->negs[i]; } }
    break;
  }

  /* same symbols */

  return 0;
}



/* compute traits intersection  */

static symbol_t * inter_traits(const symbol_t * a, const symbol_t * b) {

  if (a->POS != b->POS) { return NULL; }


  /* make sure a is not smaller than b */

  if (a->nbtraits < b->nbtraits) {
    const symbol_t * tmp = a;
    a = b;
    b = tmp;
  }

  symbol_t * res = symbol_new(a->POS);

  int i;
  for (i = 0; i < b->nbtraits; i++) {

    switch (a->traits[i]) {

    case UNSPECIFIED:
      res->traits[i] = b->traits[i];
      break;

    case LOCKED:
      if (b->traits[i] > 0) { goto null; }
      res->traits[i] = LOCKED;
      break;

    default:
      if (b->traits[i] != a->traits[i] && b->traits[i] != UNSPECIFIED) { goto null; }
      res->traits[i] = a->traits[i];
      break;
    }
  }

  for (; i < a->nbtraits; i++) { res->traits[i] = a->traits[i]; }

  return res;

null:
  symbol_delete(res);
  return NULL;
}


static symbol_t * CAN_inter_CAN(const symbol_t * a, const symbol_t * b) {

  if (a->form != b->form) { return NULL; }
  if (a->canonic != b->canonic) { return NULL; }

  symbol_t * res = inter_traits(a, b);

  if (res == NULL) { return NULL; }

  res->form    = a->form;
  res->canonic = a->canonic;

  return symbols_clean(res);
}


static symbol_t * CAN_inter_NEG(const symbol_t * a, const symbol_t * b) {

  if (canonic_in_neg(a->canonic, b)) { return NULL; }

  symbol_t * res = inter_traits(a, b);

  if (res == NULL) { return NULL; }

  res->form    = a->form; 
  res->canonic = a->canonic;

  return symbols_clean(res);
}


static symbol_t * CAN_inter_CODE(const symbol_t * a, const symbol_t * b) {

  symbol_t * res = inter_traits(a, b);

  if (res == NULL) { return NULL; }

  res->form    = a->form; 
  res->canonic = a->canonic;

  return symbols_clean(res);
}


static symbol_t * NEG_inter_NEG(const symbol_t * a, const symbol_t * b) {

  symbol_t * res = inter_traits(a, b);

  if (res == NULL) { return NULL; }

  res->negative = true;
  negs_inter_negs(res, a, b);

  return symbols_clean(res);
}


static symbol_t * NEG_inter_CODE(const symbol_t * a, const symbol_t * b) {

  symbol_t * res = inter_traits(a, b);

  if (res == NULL) { return NULL; }

  res->negative = true;
  res->nbnegs = a->nbnegs;
  res->negs = (int *) xmalloc(res->nbnegs * sizeof(int));
  for (int i = 0; i < res->nbnegs; i++) { res->negs[i] = a->negs[i]; }

  return symbols_clean(res);
}



static inline symbol_t * CODE_inter_CODE(const symbol_t * a, const symbol_t * b) {
  symbol_t * res = inter_traits(a, b);
  return symbols_clean(res);
}




symbol_t * symbol_inter_symbol(const symbol_t * a, const symbol_t * b) {

  //  debug("inter("); symbol_dump(a); errprintf(", "); symbol_dump(b); errprintf(")\n");

  if (a == SYMBOL_DEF || b == SYMBOL_DEF) { fatal_error("symb inter symb: called with SYMBOL_DEF as arg\n"); }

  if (a == b) { return symbol_dup(a); }

  if (a == NULL || b == NULL) { return NULL; }


  if (type_order(a->type) < 0) { fatal_error("inter: invalid symbol type = '%c'\n", a->type); }
  if (type_order(b->type) < 0) { fatal_error("inter: invalid symbol type = '%c'\n", b->type); }


  if (a->type == EPSILON || b->type == EPSILON) { fatal_error("inter: epsilon\n"); }

  if (a->type == LEXIC) { return symbol_dup(b); }
  if (b->type == LEXIC) { return symbol_dup(a); }

  if (a->POS != b->POS) { return NULL; }

  symbol_t * res = NULL;

  switch (a->type) {

  case ATOM:
  case INC_CAN:

    switch (b->type) {

    case ATOM:
    case INC_CAN:
      res = CAN_inter_CAN(a, b);
      break;

    case CODE_NEG:
    case INC_NEG:
      res = CAN_inter_NEG(a, b);
      break;

    case CODE:
    case INC:
      res = CAN_inter_CODE(a, b);
      break;

    default:
      fatal_error("internal error in symbol_inter_symbol: invalid symbol type=%d\n", b->type);
    }
    break;


  case CODE_NEG:
  case INC_NEG:
    switch (b->type) {

    case ATOM:
    case INC_CAN:
      res = CAN_inter_NEG(b, a);
      break;

    case CODE_NEG:
    case INC_NEG:
      res = NEG_inter_NEG(a, b);
      break;

    case CODE:
    case INC:
      res = NEG_inter_CODE(a, b);
      break;

    default:
      fatal_error("internal error in symbol_inter_symbol: weird symbol type=%d\n", b->type);
    }
    break;


  case CODE:
  case INC:

    switch (b->type) {

    case ATOM:
    case INC_CAN:
      res = CAN_inter_CODE(b, a);
      break;

    case CODE_NEG:
    case INC_NEG:
      res = NEG_inter_CODE(b, a);
      break;

    case INC:
    case CODE:
      res = CODE_inter_CODE(a, b);
      break;

    default:
      fatal_error("internal error in symbol_inter_symbol: weird symbol type=%c\n", b->type);
    }
    break;

  default:
    fatal_error("internal error in symbol_inter_symbol: unexpected symbol type=%d\n", a->type);
  }

  return res;
}



// a ^ (b1 v b2 v ...) == (a ^ b1) v (a ^ b2) v ...

symbol_t * symbol_inter_symbols(const symbol_t * a, const symbol_t * B) {

  symbol_t * res = NULL;
  symbol_t * tmp;

  while (B) {

    tmp = symbol_inter_symbol(a, B);
    if (tmp) {
      tmp->next = res;
      res = tmp;
    }

    B = B->next;
  }

  return res;
}



symbol_t * symbols_inter_symbols(const symbol_t * A, const symbol_t * B) {

  symbol_t blah;

  blah.next = NULL; // blah.next == res

  symbol_t * end = & blah;

  while (A) {

    symbol_t * tmp = symbol_inter_symbols(A, B);

    symbols_concat(end, tmp, & end);

    A = A->next;
  }

  return blah.next;
}




/* appartenance */


/* true if a is in b (regarding traits) */

static bool in_traits(const symbol_t * a, const symbol_t * b) {

  if (a->POS != b->POS) { return false; }

  int min = MIN(a->nbtraits, b->nbtraits);

  int i;
  for (i = 0; i < min; i++) {
    switch (b->traits[i]) {
    case UNSPECIFIED:
      break;
    default:
      if (a->traits[i] != b->traits[i]) { return false; }
   }
  }

  for (; i < b->nbtraits; i++) { // if there is some traits left for b they should be UNSPEC
    if (b->traits[i] != UNSPECIFIED) { return false; }
  }

  return true;
}


static inline bool CAN_in_CAN(const symbol_t * a, const symbol_t * b) {
  return ((a->canonic == b->canonic) && in_traits(a, b));
}

static inline bool NEG_in_CAN(const symbol_t * /*a*/, const symbol_t * /*b*/)  { return false; }
static inline bool CODE_in_CAN(const symbol_t * /*a*/, const symbol_t * /*b*/) { return false; }


static inline bool CAN_in_NEG(const symbol_t * a, const symbol_t * b) {
  return (in_traits(a, b) && (! canonic_in_neg(a->canonic, b)));
}

static inline bool NEG_in_NEG(const symbol_t * a, const symbol_t * b) {

  if (! in_traits(a, b)) { return false; }

  for (int i = 0; i < b->nbnegs; i++) {
    if (! canonic_in_neg(b->negs[i], a)) { /* la forme est niee dans b mais pas dans a => a n'est pas dans b */
      return false;
    }
  }

  return true;
}

static inline bool CODE_in_NEG(const symbol_t * /*a*/, const symbol_t * /*b*/) { return false; }

static inline bool CAN_in_CODE(const symbol_t * a, const symbol_t * b)  { return in_traits(a, b); }
static inline bool NEG_in_CODE(const symbol_t * a, const symbol_t * b)  { return in_traits(a, b); }
static inline bool CODE_in_CODE(const symbol_t * a, const symbol_t * b) { return in_traits(a, b); }


bool symbol_in_symbol(const symbol_t * a, const symbol_t * b) {

  if (a == SYMBOL_DEF || b == SYMBOL_DEF) { fatal_error("in: called with SYMBOL_DEF as arg\n"); }

  if (a == b) { return true; }

  if (b == NULL) { return false; }

  if (a == NULL) { return true; }

  if (type_order(a->type) < 0) { fatal_error("in: invalid type in a '%c'\n", a->type); }
  if (type_order(b->type) < 0) { fatal_error("in: invalid type in b '%c'\n", b->type); }

  if (a->type == EPSILON || b->type == EPSILON) { fatal_error("in: epsilon\n"); }

  if (b->type == LEXIC) { return true; }

  if (a->type == LEXIC) { return false; }

  if (a->POS != b->POS) { return false;; }

  bool res =  false;

  switch (a->type) {

  case ATOM:
  case INC_CAN:

    switch (b->type) {

    case ATOM:
    case INC_CAN:
      res = CAN_in_CAN(a, b);
      break;

    case CODE_NEG:
    case INC_NEG:
      res = CAN_in_NEG(a, b);
      break;

    case CODE:
    case INC:
      res = CAN_in_CODE(a, b);
      break;
    }
    break;

  case CODE_NEG:
  case INC_NEG:

    switch (b->type) {

    case ATOM:
    case INC_CAN:
      res = NEG_in_CAN(a, b);
      break;

    case CODE_NEG:
    case INC_NEG:
      res = NEG_in_NEG(a, b);
      break;

    case CODE:
    case INC:
      res = NEG_in_CODE(a, b);
      break;
    }
    break;


  case INC:
  case CODE:

    switch (b->type) {

    case ATOM:
    case INC_CAN:
      res = CODE_in_CAN(a, b);
      break;

    case CODE_NEG:
    case INC_NEG:
      res = CODE_in_NEG(a, b);
      break;

    case CODE:
    case INC:
      res = CODE_in_CODE(a, b);
      break;
    }
    break;

  }
  return res;
}





/* complementation
 *
 * toutes les fonctions XXX_minus_YYY(a, b) ont pour précondition que b est inclu où egal à a
 */


/* calcule la complementation de a moins b au niveau des traits uniquement
 * b doit etre inclu ou egal à a
 */

static symbol_t * minus_traits(const symbol_t * a, const symbol_t * b) {

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;


  if (a->POS != b->POS) { fatal_error("minus_traits: ! POSs\n"); }

  symbol_t * templat = symbol_new(a->POS);

  int idx;
  for (idx = 0; idx < a->nbtraits; idx++) { templat->traits[idx] = a->traits[idx]; }

  int min = MIN(a->nbtraits, b->nbtraits);

  for (idx = 0; idx < min; idx++) {

    if (a->traits[idx] == b->traits[idx]) { continue; }
      
    if (a->traits[idx] != UNSPECIFIED) { fatal_error("minus_traits: b not in a\n"); }

    /* a->traits[idx] est UNSPEC et b->traits[idx] est fixé */

    CAT_t * CAT = POS_get_CAT(a->POS, idx);

    // debug("minus_trait: differs in '%S'\n", CAT->name);

    for (int v = -1; v < CAT->traits->nbelems; v++) { /* on ajoute pour chaque valeur fixée != b->traits[idx] */

      if (v == UNSPECIFIED || v == b->traits[idx]) { continue; }

      templat->traits[idx] = v;

      symbols_concat(end, symbol_dup(templat), & end);
    }
    
    /* on fixe la valeur a b->traits[idx] et on continue */

    templat->traits[idx] = b->traits[idx];
  }


  for (; idx < b->nbtraits; idx++) { // si il reste des traits pour b
      
    if (b->traits[idx] != UNSPECIFIED) { // a->traits[idx] == UNSPEC

      CAT_t * CAT = POS_get_CAT(a->POS, idx);

      for (int v = -1; v < CAT->traits->nbelems; v++) { /* on ajoute pour chaque valeur fixée != b->traits[idx] */

	if (v == UNSPECIFIED || v == b->traits[idx]) { continue; }

	templat->traits[idx] = v;
	symbols_concat(end, symbol_dup(templat), & end);
      }
    
      /* on fixe la valeur a b->traits[idx] et on continue */
	
      templat->traits[idx] = b->traits[idx];
    }
  }

  symbol_delete(templat);

  return res.next;
}


static inline symbol_t * CAN_minus_CAN(const symbol_t * a, const symbol_t * b) {

  if (a->canonic != b->canonic) { fatal_error("CAN minus CAN: different canonical forms\n"); }

  symbol_t * minus = minus_traits(a, b);
  for (symbol_t * s = minus; s; s = s->next) { s->canonic = a->canonic; }

  return symbols_clean(minus);
}


/* un symbole negatif ne peut jamais etre inclu dans un symbole ou la forme canonique est fixée */

static inline symbol_t * CAN_minus_NEG(const symbol_t * /*a*/, const symbol_t * /*b*/) {
  fatal_error("CAN minus NEG: should never happen\n");
  return NULL;
}


/* idem pour un symbol sans forme canonique */

static symbol_t * CAN_minus_CODE(const symbol_t * /*a*/, const symbol_t * /*b*/) {
  fatal_error("CAN minus CODE: should never happen\n");
  return NULL;
}


/* NEG minus CAN:
 * a a une forme canonique negative et b une forme canonique positive.
 * b est inclu dans a
 *
 * a minus b = (a minus code(b)) union ((code(b) minus b) inter a)
 * = (a minus code(b)) union (code(b) inter (! canonical(b)) inter canonical(a))
 *
 * (b in a) => (code(b) in code(a))
 */

static symbol_t * NEG_minus_CAN(const symbol_t * a, const symbol_t * b) {

  if (canonic_in_neg(b->canonic, a)) { fatal_error("NEG minus CAN: symbols disjoint\n"); }

  /* a minus code(b) */

  int i;

  symbol_t * minus = minus_traits(a, b);
  symbol_t * s;
  for (s = minus; s; s = s->next) {
    s->negative = true;
    s->nbnegs   = a->nbnegs;
    s->negs = (int *) xmalloc(s->nbnegs * sizeof(int));
    for (i = 0; i < s->nbnegs; i++) { s->negs[i] = a->negs[i]; }
  }

  /* (code(b) minus b) inter a */

  s = symbol_new(b->POS);
  traits_copy(s, b);

  /* insert b->canonic in neglist */

  s->negative = true;
  s->negs   = (int *) xmalloc((a->nbnegs + 1) * sizeof(int));

  s->nbnegs = 0;

  bool inserted = false;

  for (i = 0; i < a->nbnegs; i++) {

    if (! inserted && (b->canonic < a->negs[i])) {
      s->negs[s->nbnegs++] = b->canonic;
      inserted = true;
    }

    s->negs[s->nbnegs++] = a->negs[i];
  }

  if (! inserted && (s->negs[s->nbnegs - 1] < b->canonic)) { s->negs[s->nbnegs++] = b->canonic; }

  s->next = minus;

  return symbols_clean(s);
}


/* NEG minus NEG
 *
 * a minus b = (a minus code(b)) union ((code(b) minus b) inter a)
 * a minus b = (a minus code(b)) union ((code(b) inter (! canonical(b)) inter canonical(a))
 *
 */

static symbol_t * NEG_minus_NEG(const symbol_t * a, const symbol_t * b) {

  //  debug("NEG_minus_NEG\n");
  //  symbol_dump(a); errprintf(" minus "); symbol_dump(b); endl();

  /* a minus code(b) */

  int i;

  symbol_t * res = minus_traits(a, b); 
  for (symbol_t * s = res; s; s = s->next) {
    s->negative = true;
    s->nbnegs   = a->nbnegs;
    s->negs = (int *) xmalloc(s->nbnegs * sizeof(int));
    for (i = 0; i < s->nbnegs; i++) { s->negs[i] = a->negs[i]; }
  }


  for (i = 0; i < b->nbnegs; i++) {

    if (! canonic_in_neg(b->negs[i], a)) {

      /* la forme form = b->negs[i] est niée dans b mais pas dans a
       * => (canonic(form) inter code(b)) in (a minus b)
       */

      symbol_t * nouvo = symbol_new(b->POS);
      traits_copy(nouvo, b);
      nouvo->canonic = b->negs[i];
      nouvo->next = res;
      res = nouvo;
    }
  }

  //  debug(" res = "); symbols_dump(res); endl();

  return symbols_clean(res);
}


/* NEG_minus_CODE
 *
 * a minus b = (a minus code(b)) union ((code(b) minus b) inter a)
 *           = (a minus code(b))
 * (b == code(b))
 */

static symbol_t * NEG_minus_CODE(const symbol_t * a, const symbol_t * b) {

  symbol_t * res = minus_traits(a, b);

  for (symbol_t * s = res; s; s = s->next) {
    s->negative = true;
    s->nbnegs = a->nbnegs;
    s->negs = (int *) xmalloc(s->nbnegs * sizeof(int));
    for (int i = 0; i < s->nbnegs; i++) { s->negs[i]  = a->negs[i]; }
  }

  return symbols_clean(res);
}



static symbol_t * CODE_minus_CAN(const symbol_t * a, const symbol_t * b) {

  /* code(b) minus b ... */

  symbol_t * res = symbol_new(b->POS);

  traits_copy(res, b);
  res->negative = true;
  res->nbnegs  = 1;
  res->negs    = (int *) xmalloc(sizeof(int));
  res->negs[0] = b->canonic;

  /* ... union (a minus code(b)) */

  res->next = minus_traits(a, b);

  return symbols_clean(res);
}


static symbol_t * CODE_minus_NEG(const symbol_t * a, const symbol_t * b) {


  /* a minus code(b) */

  symbol_t * res = minus_traits(a, b);


  /* code(b) minus b */

  for (int i = 0; i < b->nbnegs; i++) {

    symbol_t * nouvo = symbol_new(b->POS);
    traits_copy(nouvo, b);

    nouvo->canonic = b->negs[i];

    nouvo->next = res;
    res = nouvo;
  }

  return symbols_clean(res);
}



static inline symbol_t * CODE_minus_CODE(const symbol_t * a, const symbol_t * b) { return symbols_clean(minus_traits(a, b));  }
/*
 debug("CODE_minus_CODE("); symbol_dump(a); errprintf(", "); symbol_dump(b); errprintf(")\n");

 symbol_t * res = minus_traits(a, b);

  debug("res="); symbols_dump(res); endl();

  res = symbols_clean(res);

  debug("res(clean)="); symbols_dump(res); endl();

  return res;
}
*/

static symbol_t * POS_minus_symbol(const symbol_t * a) {

  POS_t * POS = a->POS;

  if (POS->codes == NULL) {
    symbol_t * s = symbol_new(POS);
    symbol_t * res = symbol_minus_symbol(s, a);
    symbol_delete(s);
    return res;
  }

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  bool found = false;

  for (symbol_t * code = POS->codes; code; code = code->next) {

    symbol_t * inter = symbol_inter_symbol(a, code);

    if (inter) {

      found = true;
      symbols_concat(end, symbol_minus_symbol(code, inter), & end);

      symbol_delete(inter);

    } else {

      end->next = symbol_dup(code);
      end = end->next;
    }
  }

  if (! found) { error("POS minus symbol: symbol doesn't match any code\n"); }

  return res.next;
}

/*
 * moved in symbol.o, so that it doesn't depend on this file
symbol_t * LEXIC_minus_POS(POS_t * POS) {

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  for (int i = 0; i < LANG->POSs->nbelems; i++) {

    POS_t * POS2 = (POS_t *) LANG->POSs->tab[i];

    if (POS2 == POS) { continue; }

    symbols_concat(end, symbol_new(POS), & end);
  }

  return res.next;
}

*/


static symbol_t * LEXIC_minus_symbol(const symbol_t * b) {

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  if (! LANG) { fatal_error("current LANGUAGE is not set!\n"); }

  for (int i = 0; i < LANG->POSs->nbelems; i++) {

    POS_t * POS = (POS_t *) LANG->POSs->tab[i];

    if (POS == b->POS) { continue; }

    //    symbols_concat(end, POS_expand(POS), & end);
    symbols_concat(end, symbol_new(POS), & end);
  }

  symbols_concat(end, POS_minus_symbol(b)); // POS minus b

  return res.next;
}




/* retourne a moins b
 * b doit etre inclu dans a.
 */

static symbol_t * _symbol_minus_symbol(const symbol_t * a, const symbol_t * b) {

  if (a == SYMBOL_DEF || b == SYMBOL_DEF) { fatal_error("symb minus symb: called with SYMBOL_DEF as arg\n"); }

  if (a == b) { return NULL; }

  if (b == NULL) { return symbol_dup(a); }

  if (a == NULL) { fatal_error("minus: a == NULL\n"); }

  if (! symbol_in_symbol(b, a)) {
    fatal_error("minus: b not in a\n");
  }

  if (type_order(a->type) < 0) { fatal_error("minus: invalid type in a '%c'\n", a->type); }
  if (type_order(b->type) < 0) { fatal_error("minus: invalid type in b '%c'\n", b->type); }

  if (a->type == LEXIC) {

    if (b->type == LEXIC) { return NULL; }
    if (b->type == EPSILON) { fatal_error("minus: LEXIC minus EPSILON\n"); }

    return LEXIC_minus_symbol(b);
  }

  if (a->type == EPSILON) {
    error("minus: a == EPSILON\n");
    if (b->type == EPSILON) { return NULL; }
    fatal_error("minus: a == EPSILON\n");
  }


  if (b->type == LEXIC) { fatal_error("minus: b == LEXIC\n"); }

  if (b->type == EPSILON) { fatal_error("minus: b == epsilon\n"); }

  
  if (a->POS != b->POS) { fatal_error("minus: different POSs\n"); }

  symbol_t * res = NULL;

  switch (b->type) {


  case ATOM:
  case INC_CAN:

    switch (a->type) {

    case ATOM:
    case INC_CAN:
      res = CAN_minus_CAN(a, b);
      break;

    case CODE_NEG:
    case INC_NEG:
      res = NEG_minus_CAN(a, b);
      break;

    case CODE:
    case INC:
      res = CODE_minus_CAN(a, b);
      break;
    }
    break;

  case CODE_NEG:
  case INC_NEG:

    switch (a->type) {

    case ATOM:
    case INC_CAN:
      res = CAN_minus_NEG(a, b);
      break;

    case CODE_NEG:
    case INC_NEG:
      res = NEG_minus_NEG(a, b);
      break;

    case CODE:
    case INC:
      res = CODE_minus_NEG(a, b);
      break;
    }
    break;


  case INC:
  case CODE:

    switch (a->type) {

    case ATOM:
    case INC_CAN:
      res = CAN_minus_CODE(a, b);
      break;

    case CODE_NEG:
    case INC_NEG:
      res = NEG_minus_CODE(a, b);
      break;

    case CODE:
    case INC:
      res = CODE_minus_CODE(a, b);
      break;
    }
    break;

  }

  //  symbol_dump(a); errprintf(" minus "); symbol_dump(b); errprintf(" = "); symbols_dump(res); endl();

  return res;
}






/* interface avec l'exterieur
 */

symbol_t * symbol_minus_symbol(const symbol_t * a, const symbol_t * b) {

  if (! symbol_in_symbol(b, a)) {

    symbol_t * i = symbol_inter_symbol(a, b);

    if (i == NULL) { return symbol_dup(a); }

    symbol_t * res = _symbol_minus_symbol(a, i);

    symbol_delete(i);

    return res;
  }

  return _symbol_minus_symbol(a, b);
}





symbol_t * symbol_minus_symbols(const symbol_t * a, const symbol_t * B) {

  symbol_t * res = symbol_dup(a);

  while (res && B) {

    symbol_t * minus = symbol_minus_symbol(a, B);
    symbol_t * tmp   = res;

    res = symbols_inter_symbols(tmp, minus);

    symbols_delete(tmp); symbols_delete(minus);
    B = B->next;
  }

  return res;
}


symbol_t * symbols_minus_symbols(const symbol_t * A, const symbol_t * B) {

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  while (A) {
    symbol_t * minus = symbol_minus_symbols(A, B);
    symbols_concat(end, minus, & end);
    A = A->next;
  }

  return res.next;
}


symbol_t * symbols_minus_symbol(const symbol_t * A, const symbol_t * b) {

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  while (A) {
    symbol_t * minus = symbol_minus_symbol(A, b);
    symbols_concat(end, minus, & end);
    A = A->next;
  }

  return res.next;
}




symbol_t * minus_symbol(const symbol_t * b) { return LEXIC_minus_symbol(b); }


symbol_t * minus_symbols(const symbol_t * b) {

  symbol_t * LEX = symbol_LEXIC_new();
  symbol_t * res = symbol_minus_symbols(LEX, b);

  symbol_delete(LEX);
  return res;
}

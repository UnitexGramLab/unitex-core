/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Ustring.h"
#include "Symbol.h"
#include "Symbol_op.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define MIN(a, b) ((a <= b) ? a : b)


static inline void traits_copy(symbol_t * dest, const symbol_t * src) {
  for (int i = 0; i < src->nb_features; i++) { dest->feature[i] = src->feature[i]; }
}


/**
 * We set an order on symbol's types
 */
static char* make_TYPE_ORDER(char* t) {
for (int i=0;i<256;i++) {
   t[i]=-1;
}
t[S_LEXIC]=0;
t[S_EPSILON]=1;
t[S_ATOM]=2;
t[S_CODE_NEG]=3;
t[S_CODE]=4;
t[S_INC_CAN]=5;
t[S_INC_NEG]=6;
t[S_INC]=7;
t[S_EXCLAM]=-2;
t[S_EQUAL]=-2;
return t;
}


static char _TYPE_ORDER_[256];
static const char* TYPE_ORDER=make_TYPE_ORDER(_TYPE_ORDER_);

/**
 * Returns the order value corresponding to the given symbol's type.
 */
static inline int type_order(int type) {
if (type < 0 || type > 255) {
   /* If the type is out of bounds */
   return -5;
}
return TYPE_ORDER[type];
}


/**
 * This function takes a list of symbols and types
 * all of them, discarding the invalid ones, if any.
 */
symbol_t* type_and_clean_symbols(symbol_t* symbols) {
symbol_t* res=symbols;
/* We look for the first valid symbol of the list */
while (res!=NULL && (type_symbol(res)==-1)) {
   symbols=res;
   res=res->next;
   free_symbol(symbols);
}
/* Then, we type and clean the rest of the list */
for (symbol_t* s=res;s!=NULL;s=s->next) {
   while (s->next!=NULL && (type_symbol(s->next)==-1)) {
      symbol_t* next=s->next->next;
      free_symbol(s->next);
      s->next=next;
   }
}
return res;
}


static inline bool canonic_in_neg(int canonic, const symbol_t * s) {
  for (int i = 0; i < s->nbnegs; i++) { if (canonic == s->negs[i]) { return true; } }
  return false;
}



/**
 * This function takes two symbols a and b of the form <!seat!ball.N:fs>
 * and it res's forbidden lemmas with those that belong to a or b's ones.
 *
 * <!seat!ball.N:fs> inter <!chair!ball.N:fs> => <!seat!ball!chair>
 */
void negs_union_negs(symbol_t* res,const symbol_t* a,const symbol_t* b) {
int max_negs=a->nbnegs+b->nbnegs;
res->nbnegs=0;
res->negs=(int*)malloc(max_negs*sizeof(int));
if (res->negs==NULL) {
   fatal_alloc_error("negs_union_negs");
}
int c1=0;
int c2=0;
while (c1<a->nbnegs && c2<b->nbnegs) {
   if (a->negs[c1]==b->negs[c2]) {
      res->negs[res->nbnegs]=a->negs[c1];
      (res->nbnegs)++;
      c1++;
      c2++;
   } else if (a->negs[c1]<b->negs[c2]) {
      res->negs[res->nbnegs]=a->negs[c1];
      (res->nbnegs)++;
      c1++;
   } else {
      res->negs[res->nbnegs]=b->negs[c2];
      (res->nbnegs)++;
      c2++;
   }
}
while (c1<a->nbnegs) {
   /* If some forbidden lemmas remain in a */
   res->negs[res->nbnegs]=a->negs[c1];
   (res->nbnegs)++;
   c1++;
}
while (c2<b->nbnegs) {
   /* If some forbidden lemmas remain in b */
   res->negs[res->nbnegs]=b->negs[c2];
   (res->nbnegs)++;
   c2++;
}
}





/* symbols comparaison */

static inline int compare_traits(const symbol_t * a, const symbol_t * b) {

  int min = MIN(a->nb_features, b->nb_features);

  int i;
  for (i = 0; i < min; i++) {
    if (a->feature[i] != b->feature[i]) { return b->feature[i] - a->feature[i]; }
  }

  for (; i < a->nb_features; i++) {
    if (a->feature[i] != UNSPECIFIED) { return UNSPECIFIED - a->feature[i]; }
  }

  for (; i < b->nb_features; i++) {
    if (b->feature[i] != UNSPECIFIED) { return b->feature[i] - UNSPECIFIED; }
  }

  return 0;
}


/**
 * Compares the symbols a and b and returns:
 *  0 if a==b
 * <0 if a<b
 * >0 if a>b
 */
int symbol_compare(const symbol_t* a,const symbol_t* b) {
int cmp;
if (a==NULL) {
   error("NULL 'a' error in symbol_compare\n");
   return -1;
}
if (b==NULL) {
   error("NULL 'b' error in symbol_compare\n");
   return 1;
}
if (a==SYMBOL_DEF) {
   error("<def> 'a' error in symbol_compare\n");
   return -1;
}
if (b==SYMBOL_DEF) {
   error("<def> 'b' error in symbol_compare\n");
   return 1;
}
if (a==b) {
   return 0;
}
if (type_order(a->type)<0) {
   fatal_error("symbol_compare: invalid symbol for a (type=%d)\n",a->type);
}
if (type_order(b->type) < 0) { fatal_error("symbol_compare: invalid symbol for b (type=%d)\n", b->type); }
if (type_order(a->type) != type_order(b->type)) { return type_order(b->type) - type_order(a->type); }

  /* same type */

  /* first compare POSs */

  if (a->POS != b->POS) { return (int)(b->POS - a->POS); }

  /* next compare by traits */

  cmp = compare_traits(a, b);
  if (cmp) { return cmp; }


  /* end then compare strings */

  switch (a->type) {

  case S_ATOM:
  case S_INC_CAN:
    if (a->lemma != b->lemma) { return b->lemma - a->lemma; }
    if (a->form != b->form) { return b->form - a->form; }
    break;

  case S_CODE_NEG:
  case S_INC_NEG:
    if (a->nbnegs != b->nbnegs) { return b->nbnegs - a->nbnegs; }
    for (int i = 0; i < a->nbnegs; i++) { if (a->negs[i] != b->negs[i]) { return b->negs[i] - a->negs[i]; } }
    break;
  default: ; /* nothing to do: just want to avoid a warning */
  }

  /* same symbols */

  return 0;
}



/* compute traits intersection  */

/**
 * Returns a symbol containing the intersection of a and b's features.
 */
symbol_t* inter_features(const symbol_t* a,const symbol_t* b) {
if (a->POS!=b->POS) {return NULL;}
/* We make sure that a's number of features is greater than b's */
if (a->nb_features<b->nb_features) {
   const symbol_t* tmp=a;
   a=b;
   b=tmp;
}
int tag_number=a->tfsttag_index;
if (tag_number==-1) {
   tag_number=b->tfsttag_index;
} else if (b->tfsttag_index!=-1 && b->tfsttag_index!=a->tfsttag_index) {
   /* Means that we try to intersects 2 text automaton symbols.
    * This could happen if there are two parallel tags with a non empty intersection like:
    *
    * {tutu,.N:ms:fs} and {tutu,.N:ms}
    *
    * If such a case really happen, then we assume that both tags have the same offset
    * information. If not, we cannot solve the problem. */
}
symbol_t* res=new_symbol_POS(a->POS,tag_number);
int i;
for (i=0;i<b->nb_features;i++) {
   // signed char cast prevent warning when compile with android ndk
   switch ((signed char)(a->feature[i])) {
      case UNSPECIFIED: res->feature[i]=b->feature[i]; break;

      case LOCKED:
         if (b->feature[i]>0) {
            /* If a feature is set in b while locked in a, then
             * the symbols a and b are not compatible */
            goto null;
         }
         res->feature[i]=LOCKED;
         break;

      default:
         if (b->feature[i]!=a->feature[i] && b->feature[i]!=UNSPECIFIED) {
            /* If a feature is set with different values in a and b, then
             * a inter b is empty. */
            goto null;
         }
         res->feature[i]=a->feature[i];
         break;
   }
}
for (;i<a->nb_features;i++) {
   /* If a puts a constraint on a feature that is not in b, then this
    * feature must appear in the intersection */
   res->feature[i]=a->feature[i];
}
return res;

null:
free_symbol(res);
return NULL;
}


/**
 * Returns the intersection of a and b where a and b are both of the
 * form <tables,table.N:ms>
 */
symbol_t* CAN_inter_CAN(const symbol_t* a,const symbol_t* b) {
if (a->form!=b->form) {return NULL;}
if (a->lemma!=b->lemma) {return NULL;}
/* If the inflected forms and lemmas are compatible, we look
 * at the features */
symbol_t* res=inter_features(a,b);
if (res==NULL) {return NULL;}
res->form=a->form;
res->lemma=a->lemma;
/* We don't forget to type the symbol */
return type_and_clean_symbols(res);
}


/**
 * Returns the intersection of a and b where a and b are respectively
 * of the form <tables,table.N:ms> and <!seat!ball.N:fs>
 */
symbol_t* CAN_inter_NEG(const symbol_t* a,const symbol_t* b) {
if (canonic_in_neg(a->lemma,b)) {
   /* If a's lemma belongs to b's forbidden ones, we fail */
   return NULL;
}
symbol_t* res=inter_features(a,b);
if (res==NULL) {return NULL;}
res->form=a->form;
res->lemma=a->lemma;
return type_and_clean_symbols(res);
}


/**
 * Returns the intersection of a and b where a and b are respectively
 * of the form <tables,table.N:ms> and <N:fs>
 */
symbol_t* CAN_inter_CODE(const symbol_t* a,const symbol_t* b) {
symbol_t* res=inter_features(a,b);
if (res==NULL) {return NULL;}
res->form=a->form;
res->lemma=a->lemma;
return type_and_clean_symbols(res);
}


/**
 * Returns the intersection of a and b where a and b are both of the
 * form <!seat!ball.N:fs>
 *
 * <!seat!ball.N:s> inter <!chair!ball.N:m> => <!seat!ball!chair.N:ms>
 */
static symbol_t* NEG_inter_NEG(const symbol_t* a,const symbol_t* b) {
symbol_t* res=inter_features(a,b);
if (res==NULL) {return NULL;}
res->negative=true;
/* We compute in the union of a and b's forbidden lemmas */
negs_union_negs(res,a,b);
return type_and_clean_symbols(res);
}


/**
 * Returns the intersection of a and b where a and b are respectively
 * of the form <!seat!ball.N:s> and <N:f>
 */
symbol_t* NEG_inter_CODE(const symbol_t* a,const symbol_t* b) {
symbol_t* res=inter_features(a,b);
if (res==NULL) {return NULL;}
res->negative=true;
res->nbnegs=a->nbnegs;
res->negs=(int*)malloc(res->nbnegs*sizeof(int));
if (res->negs==NULL) {
   fatal_alloc_error("NEG_inter_CODE");
}
for (int i=0;i< res->nbnegs;i++) {
   res->negs[i]=a->negs[i];
}
return type_and_clean_symbols(res);
}


/**
 * Returns the intersection of a and b where a and b are both of the
 * form <N:fs>
 *
 * <V:Ks> inter <V:Kf> => <V:Kfs>
 */
symbol_t* CODE_inter_CODE(const symbol_t* a,const symbol_t* b) {
symbol_t* res=inter_features(a,b);
return type_and_clean_symbols(res);
}


/**
 * Returns a symbol representing the intersection of the two given
 * symbols, or NULL if the intersection is empty.
 */
symbol_t* symbol_inter_symbol(const symbol_t* a,const symbol_t* b) {
if (a==SYMBOL_DEF || b==SYMBOL_DEF) {
   fatal_error("symbol_inter_symbol: called with SYMBOL_DEF as arg\n");
}
if (a==b) {
   /* Full intersection */
   return dup_symbol(a);
}
if (a==NULL || b==NULL) {
   /* Empty intersection */
   return NULL;
}
if (type_order(a->type)<0) {
   fatal_error("symbol_inter_symbol: invalid symbol type = '%c'\n",a->type);
}
if (type_order(b->type)<0) {
   fatal_error("symbol_inter_symbol: invalid symbol type = '%c'\n",b->type);
}
if (a->type==S_EPSILON || b->type==S_EPSILON) {
   fatal_error("epsilon error in symbol_inter_symbol\n");
}
if (a->type==S_LEXIC) {
   /* b included in a */
   return dup_symbol(b);
}
if (b->type==S_LEXIC) {
   /* a included in b */
   return dup_symbol(a);
}
if (a->POS!=b->POS) {
   /* Symbols with distinct POS have an empty intersection */
   return NULL;
}
symbol_t* res=NULL;
switch (a->type) {
   case S_ATOM:
   case S_INC_CAN:
      switch (b->type) {
         case S_ATOM:
         case S_INC_CAN: res=CAN_inter_CAN(a,b); break;

         case S_CODE_NEG:
         case S_INC_NEG: res=CAN_inter_NEG(a,b); break;

         case S_CODE:
         case S_INC: res=CAN_inter_CODE(a,b); break;

         default: fatal_error("Internal error in symbol_inter_symbol: invalid symbol type=%d\n",b->type);
       }
       break;

   case S_CODE_NEG:
   case S_INC_NEG:
      switch (b->type) {
         case S_ATOM:
         case S_INC_CAN: res=CAN_inter_NEG(b,a); break;

         case S_CODE_NEG:
         case S_INC_NEG: res=NEG_inter_NEG(a,b); break;

         case S_CODE:
         case S_INC: res=NEG_inter_CODE(a,b); break;

         default: fatal_error("Internal error in symbol_inter_symbol: weird symbol type=%d\n",b->type);
      }
      break;

   case S_CODE:
   case S_INC:
      switch (b->type) {
         case S_ATOM:
         case S_INC_CAN: res=CAN_inter_CODE(b,a); break;

         case S_CODE_NEG:
         case S_INC_NEG: res=NEG_inter_CODE(b,a); break;

         case S_INC:
         case S_CODE: res=CODE_inter_CODE(a,b); break;

         default: fatal_error("Internal error in symbol_inter_symbol: weird symbol type=%c\n",b->type);
      }
      break;

   default: fatal_error("Internal error in symbol_inter_symbol: unexpected symbol type=%d\n",a->type);
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

    concat_symbols(end, tmp, & end);

    A = A->next;
  }

  return blah.next;
}




/* appartenance */


/* true if a is in b (regarding traits) */

static bool in_traits(const symbol_t * a, const symbol_t * b) {

  if (a->POS != b->POS) { return false; }

  int min = MIN(a->nb_features, b->nb_features);

  int i;
  for (i = 0; i < min; i++) {
    switch (b->feature[i]) {
    case UNSPECIFIED:
      break;
    default:
      if (a->feature[i] != b->feature[i]) { return false; }
   }
  }

  for (; i < b->nb_features; i++) { // if there is some traits left for b they should be UNSPEC
    if (b->feature[i] != UNSPECIFIED) { return false; }
  }

  return true;
}


static inline bool CAN_in_CAN(const symbol_t * a, const symbol_t * b) {
  return ((a->lemma == b->lemma) && in_traits(a, b));
}

static inline bool NEG_in_CAN(const symbol_t * /*a*/, const symbol_t * /*b*/)  { return false; }
static inline bool CODE_in_CAN(const symbol_t * /*a*/, const symbol_t * /*b*/) { return false; }


static inline bool CAN_in_NEG(const symbol_t * a, const symbol_t * b) {
  return (in_traits(a, b) && (! canonic_in_neg(a->lemma, b)));
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

  if (a->type == S_EPSILON || b->type == S_EPSILON) { fatal_error("in: epsilon\n"); }

  if (b->type == S_LEXIC) { return true; }

  if (a->type == S_LEXIC) { return false; }

  if (a->POS != b->POS) { return false; }

  bool res =  false;

  switch (a->type) {

  case S_ATOM:
  case S_INC_CAN:

    switch (b->type) {

    case S_ATOM:
    case S_INC_CAN:
      res = CAN_in_CAN(a, b);
      break;

    case S_CODE_NEG:
    case S_INC_NEG:
      res = CAN_in_NEG(a, b);
      break;

    case S_CODE:
    case S_INC:
      res = CAN_in_CODE(a, b);
      break;
    default: ; /* nothing to do: just want to avoid a warning */
    }
    break;

  case S_CODE_NEG:
  case S_INC_NEG:

    switch (b->type) {

    case S_ATOM:
    case S_INC_CAN:
      res = NEG_in_CAN(a, b);
      break;

    case S_CODE_NEG:
    case S_INC_NEG:
      res = NEG_in_NEG(a, b);
      break;

    case S_CODE:
    case S_INC:
      res = NEG_in_CODE(a, b);
      break;
    default: ; /* nothing to do: just want to avoid a warning */
    }
    break;


  case S_INC:
  case S_CODE:

    switch (b->type) {

    case S_ATOM:
    case S_INC_CAN:
      res = CODE_in_CAN(a, b);
      break;

    case S_CODE_NEG:
    case S_INC_NEG:
      res = CODE_in_NEG(a, b);
      break;

    case S_CODE:
    case S_INC:
      res = CODE_in_CODE(a, b);
      break;
    default: ; /* nothing to do: just want to avoid a warning */
    }
    break;

  default: ; /* nothing to do: just want to avoid a warning */
  }
  return res;
}





/* complementation
 *
 * toutes les fonctions XXX_minus_YYY(a, b) ont pour pr�condition que b est inclu o� egal � a
 */


/* calcule la complementation de a moins b au niveau des traits uniquement
 * b doit etre inclu ou egal � a
 */

static symbol_t * minus_traits(const symbol_t * a, const symbol_t * b) {

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  int tag_number=a->tfsttag_index;
  if (tag_number==-1) {
     tag_number=b->tfsttag_index;
  } else if (b->tfsttag_index!=-1) {
     /* Means that we try to intersects 2 text automaton symbols */
     fatal_error("Internal error in minus_traits\n");
  }
  if (a->POS != b->POS) { fatal_error("minus_traits: ! POSs\n"); }

  symbol_t * templat = new_symbol_POS(a->POS,tag_number);

  int idx;
  for (idx = 0; idx < a->nb_features; idx++) { templat->feature[idx] = a->feature[idx]; }

  int min = MIN(a->nb_features, b->nb_features);

  for (idx = 0; idx < min; idx++) {

    if (a->feature[idx] == b->feature[idx]) { continue; }

    if (a->feature[idx] != UNSPECIFIED) { fatal_error("minus_traits: b not in a\n"); }

    /* a->traits[idx] est UNSPEC et b->traits[idx] est fix� */

    CAT_t * CAT = POS_get_CAT(a->POS, idx);

    // debug("minus_trait: differs in '%S'\n", CAT->name);

    for (int v = -1; v < CAT->values->size; v++) { /* on ajoute pour chaque valeur fix�e != b->traits[idx] */

      if (v == UNSPECIFIED || v == b->feature[idx]) { continue; }

      templat->feature[idx] = (char)v;

      concat_symbols(end, dup_symbol(templat), & end);
    }

    /* on fixe la valeur a b->traits[idx] et on continue */

    templat->feature[idx] = b->feature[idx];
  }


  for (; idx < b->nb_features; idx++) { // si il reste des traits pour b

    if (b->feature[idx] != UNSPECIFIED) { // a->traits[idx] == UNSPEC

      CAT_t * CAT = POS_get_CAT(a->POS, idx);

      for (int v = -1; v < CAT->values->size; v++) { /* on ajoute pour chaque valeur fix�e != b->traits[idx] */

    if (v == UNSPECIFIED || v == b->feature[idx]) { continue; }

    templat->feature[idx] = (char)v;
    concat_symbols(end, dup_symbol(templat), & end);
      }

      /* on fixe la valeur a b->traits[idx] et on continue */

      templat->feature[idx] = b->feature[idx];
    }
  }

  free_symbol(templat);

  return res.next;
}


static inline symbol_t * CAN_minus_CAN(const symbol_t * a, const symbol_t * b) {

  if (a->lemma != b->lemma) { fatal_error("CAN minus CAN: different canonical forms\n"); }

  symbol_t * minus = minus_traits(a, b);
  for (symbol_t * s = minus; s; s = s->next) { s->lemma = a->lemma; }

  return type_and_clean_symbols(minus);
}


/* un symbole negatif ne peut jamais etre inclu dans un symbole ou la forme canonique est fix�e */

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

  if (canonic_in_neg(b->lemma, a)) { fatal_error("NEG minus CAN: symbols disjoint\n"); }

  /* a minus code(b) */

  int i;

  symbol_t * minus = minus_traits(a, b);
  symbol_t * s;
  for (s = minus; s; s = s->next) {
    s->negative = true;
    s->nbnegs   = a->nbnegs;
    s->negs = (int *) malloc(s->nbnegs * sizeof(int));
    if (s->negs==NULL) {
       fatal_alloc_error("NEG_minus_CAN");
    }
    for (i = 0; i < s->nbnegs; i++) { s->negs[i] = a->negs[i]; }
  }

  /* (code(b) minus b) inter a */

  int tag_number=a->tfsttag_index;
  if (tag_number==-1) {
     tag_number=b->tfsttag_index;
  } else if (b->tfsttag_index!=-1) {
     /* Means that we try to intersects 2 text automaton symbols */
     fatal_error("Internal error in NEG_minus_CAN\n");
  }

  s = new_symbol_POS(b->POS,tag_number);
  traits_copy(s, b);

  /* insert b->canonic in neglist */

  s->negative = true;
  s->negs   = (int *) malloc((a->nbnegs + 1) * sizeof(int));
  if (s->negs==NULL) {
     fatal_alloc_error("NEG_minus_CAN");
  }

  s->nbnegs = 0;

  bool inserted = false;

  for (i = 0; i < a->nbnegs; i++) {

    if (! inserted && (b->lemma < a->negs[i])) {
      s->negs[s->nbnegs++] = b->lemma;
      inserted = true;
    }

    s->negs[s->nbnegs++] = a->negs[i];
  }

  if (! inserted && (s->negs[s->nbnegs - 1] < b->lemma)) { s->negs[s->nbnegs++] = b->lemma; }

  s->next = minus;

  return type_and_clean_symbols(s);
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
    s->negs = (int *) malloc(s->nbnegs * sizeof(int));
    if (s->negs==NULL) {
       fatal_alloc_error("NEG_minus_NEG");
    }
    for (i = 0; i < s->nbnegs; i++) { s->negs[i] = a->negs[i]; }
  }
  int tag_number=a->tfsttag_index;
    if (tag_number==-1) {
       tag_number=b->tfsttag_index;
    } else if (b->tfsttag_index!=-1) {
       /* Means that we try to intersects 2 text automaton symbols */
       fatal_error("Internal error in NEG_minus_NEG\n");
    }

  for (i = 0; i < b->nbnegs; i++) {

    if (! canonic_in_neg(b->negs[i], a)) {

      /* la forme form = b->negs[i] est ni�e dans b mais pas dans a
       * => (canonic(form) inter code(b)) in (a minus b)
       */

      symbol_t * nouvo = new_symbol_POS(b->POS,tag_number);
      traits_copy(nouvo, b);
      nouvo->lemma = b->negs[i];
      nouvo->next = res;
      res = nouvo;
    }
  }

  //  debug(" res = "); symbols_dump(res); endl();

  return type_and_clean_symbols(res);
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
    s->negs = (int *) malloc(s->nbnegs * sizeof(int));
    if (s->negs==NULL) {
       fatal_alloc_error("NEG_minus_CODE");
    }
    for (int i = 0; i < s->nbnegs; i++) { s->negs[i]  = a->negs[i]; }
  }

  return type_and_clean_symbols(res);
}



static symbol_t * CODE_minus_CAN(const symbol_t * a, const symbol_t * b) {

  /* code(b) minus b ... */

   int tag_number=a->tfsttag_index;
   if (tag_number==-1) {
      tag_number=b->tfsttag_index;
   } else if (b->tfsttag_index!=-1) {
      /* Means that we try to intersects 2 text automaton symbols */
      fatal_error("Internal error in CODE_minus_CAN\n");
   }

  symbol_t * res = new_symbol_POS(b->POS,tag_number);

  traits_copy(res, b);
  res->negative = true;
  res->nbnegs  = 1;
  res->negs    = (int*)malloc(sizeof(int));
  if (res->negs==NULL) {
     fatal_alloc_error("CODE_minus_CAN");
  }
  res->negs[0] = b->lemma;

  /* ... union (a minus code(b)) */

  res->next = minus_traits(a, b);

  return type_and_clean_symbols(res);
}



static symbol_t * CODE_minus_NEG(const symbol_t * a, const symbol_t * b) {
  /* a minus code(b) */

  symbol_t * res = minus_traits(a, b);


  /* code(b) minus b */
  int tag_number=a->tfsttag_index;
  if (tag_number==-1) {
     tag_number=b->tfsttag_index;
  } else if (b->tfsttag_index!=-1) {
     /* Means that we try to intersects 2 text automaton symbols */
     fatal_error("Internal error in CODE_minus_NEG\n");
  }
  for (int i = 0; i < b->nbnegs; i++) {

    symbol_t * nouvo = new_symbol_POS(b->POS,tag_number);
    traits_copy(nouvo, b);

    nouvo->lemma = b->negs[i];

    nouvo->next = res;
    res = nouvo;
  }

  return type_and_clean_symbols(res);
}



static inline symbol_t * CODE_minus_CODE(const symbol_t * a, const symbol_t * b) {
return type_and_clean_symbols(minus_traits(a, b));
}
/*
 debug("CODE_minus_CODE("); symbol_dump(a); errprintf(", "); symbol_dump(b); errprintf(")\n");

 symbol_t * res = minus_traits(a, b);

  debug("res="); symbols_dump(res); endl();

  res = symbols_clean(res);

  debug("res(clean)="); symbols_dump(res); endl();

  return res;
}
*/

static symbol_t * POS_minus_symbol(language_t* language,const symbol_t * a) {

  POS_t * POS = a->POS;

  if (POS->codes == NULL) {
    symbol_t * s = new_symbol_POS(POS,-1);
    symbol_t * res = symbol_minus_symbol(language,s, a);
    free_symbol(s);
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
      concat_symbols(end, symbol_minus_symbol(language,code, inter), & end);

      free_symbol(inter);

    } else {

      end->next = dup_symbol(code);
      end = end->next;
    }
  }

  if (! found) { error("POS minus symbol: symbol doesn't match any code\n"); }

  return res.next;
}


static symbol_t * LEXIC_minus_symbol(language_t* language,const symbol_t * b) {

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  for (int i = 0; i < language->POSs->size; i++) {

    POS_t * POS = (POS_t *) language->POSs->value[i];

    if (POS == b->POS) { continue; }

    //    symbols_concat(end, POS_expand(POS), & end);
    concat_symbols(end, new_symbol_POS(POS,-1), & end);
  }

  concat_symbols(end, POS_minus_symbol(language,b)); // POS minus b

  return res.next;
}


/**
 * Returns a-b, assuming that b is included in a.
 */
symbol_t* _symbol_minus_symbol(language_t* language,const symbol_t* a,const symbol_t* b) {
if (a==SYMBOL_DEF || b==SYMBOL_DEF) {
   fatal_error("_symbol_minus_symbol: called with SYMBOL_DEF as arg\n");
}
if (a==b) {
   return NULL;
}
if (b==NULL) {
   return dup_symbol(a);
}
if (a==NULL) {
   fatal_error("NULL 'a' error in _symbol_minus_symbol\n");
}
if (! symbol_in_symbol(b, a)) {
   fatal_error("_symbol_minus_symbol: b not in a\n");
}
if (type_order(a->type)<0) {
   fatal_error("_symbol_minus_symbol: invalid type in 'a': '%c'\n",a->type);
}
if (type_order(b->type)<0) {
   fatal_error("_symbol_minus_symbol: invalid type in 'b': '%c'\n",b->type);
}
if (a->type==S_LEXIC) {
   if (b->type==S_LEXIC) {
      return NULL;
   }
   if (b->type==S_EPSILON) {
      fatal_error("_symbol_minus_symbol: LEXIC minus EPSILON\n");
   }
   return LEXIC_minus_symbol(language,b);
}
if (a->type==S_EPSILON) {
   error("_symbol_minus_symbol: a == EPSILON\n");
   if (b->type==S_EPSILON) {
      return NULL;
   }
   fatal_error("_symbol_minus_symbol: a == EPSILON\n");
}
if (b->type==S_LEXIC) {
   fatal_error("_symbol_minus_symbol: b == LEXIC\n");
}
if (b->type==S_EPSILON) {
   fatal_error("_symbol_minus_symbol: b == epsilon\n");
}
if (a->POS!=b->POS) {
   fatal_error("_symbol_minus_symbol: different POSs\n");
}
symbol_t* res=NULL;
switch (b->type) {
   case S_ATOM:
   case S_INC_CAN:
      switch (a->type) {
         case S_ATOM:
         case S_INC_CAN: res=CAN_minus_CAN(a,b); break;

         case S_CODE_NEG:
         case S_INC_NEG: res=NEG_minus_CAN(a,b); break;

         case S_CODE:
         case S_INC: res=CODE_minus_CAN(a,b); break;
         default: ; /* nothing to do: just want to avoid a warning */
       }
       break;

   case S_CODE_NEG:
   case S_INC_NEG:
      switch (a->type) {
         case S_ATOM:
         case S_INC_CAN: res=CAN_minus_NEG(a,b); break;

         case S_CODE_NEG:
         case S_INC_NEG: res=NEG_minus_NEG(a,b); break;

         case S_CODE:
         case S_INC: res=CODE_minus_NEG(a,b); break;
         default: ; /* nothing to do: just want to avoid a warning */
      }
      break;

   case S_INC:
   case S_CODE:
      switch (a->type) {
         case S_ATOM:
         case S_INC_CAN: res=CAN_minus_CODE(a,b); break;

         case S_CODE_NEG:
         case S_INC_NEG: res=NEG_minus_CODE(a,b); break;

         case S_CODE:
         case S_INC: res=CODE_minus_CODE(a,b); break;
         default: ; /* nothing to do: just want to avoid a warning */
      }
      break;
   default: ; /* nothing to do: just want to avoid a warning */
}
return res;
}


/**
 * Takes any 2 single symbols a and b and returns a-b.
 */
symbol_t* symbol_minus_symbol(language_t* language,const symbol_t* a,const symbol_t* b) {
if (!symbol_in_symbol(b,a)) {
   /* If b is not fully included in a, we must compute a inter b */
   symbol_t* i=symbol_inter_symbol(a,b);
   if (i==NULL) {
      /* If the intersection is empty, then the result is a */
      return dup_symbol(a);
   }
   /* Otherwise, the result is a-(a inter b) */
   symbol_t* res=_symbol_minus_symbol(language,a,i);
   free_symbol(i);
   return res;
}
/* If b is included in a */
return _symbol_minus_symbol(language,a,b);
}




/**
 * Computes and returns the set of symbols A-B, where A contains the
 * single symbol 'a'.
 */
symbol_t* symbol_minus_symbols(language_t* language,const symbol_t* a, const symbol_t* B) {
symbol_t* res=dup_symbol(a);
while (res!=NULL && B!=NULL) {
   symbol_t* minus=symbol_minus_symbol(language,a,B);
   symbol_t* tmp=res;
   res=symbols_inter_symbols(tmp,minus);
   free_symbols(tmp);
   free_symbols(minus);
   B=B->next;
}
return res;
}


symbol_t * symbols_minus_symbols(language_t* language,const symbol_t * A, const symbol_t * B) {

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  while (A) {
    symbol_t * minus = symbol_minus_symbols(language,A, B);
    concat_symbols(end, minus, & end);
    A = A->next;
  }

  return res.next;
}


/**
 * This function takes a symbol list 'list' and a symbol 's'. It returns
 * a list that contains all the elements of 'list' minus 's'. For instance,
 * If we have 'list'=<A:ms>,<A:mp>,<A:fs>,<A:fp> and 's'=<A:s>,
 * we will return the list <A:mp>,<A:fp>.
 */
symbol_t* symbols_minus_symbol(language_t* language,const symbol_t* list,const symbol_t* s) {
symbol_t res;
res.next=NULL;
symbol_t* end=&res;
while (list!=NULL) {
   symbol_t* minus=symbol_minus_symbol(language,list,s);
   concat_symbols(end,minus,&end);
   list=list->next;
}
return res.next;
}




symbol_t * minus_symbol(language_t* language,const symbol_t * b) {
    return LEXIC_minus_symbol(language,b);
}


/**
 * Computes and returns the set containing all symbols but b's ones.
 */
symbol_t* minus_symbols(language_t* language,const symbol_t* b) {
symbol_t* LEX=new_symbol(S_LEXIC,-1);
symbol_t* res=symbol_minus_symbols(language,LEX,b);
free_symbol(LEX);
return res;
}

} // namespace unitex

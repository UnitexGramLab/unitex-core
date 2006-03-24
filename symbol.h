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

#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "ustring.h"
#include "language.h"

struct language_t;
struct POS_t;

extern struct language_t * LANG;


extern unichar PUNC_TAB[];


#define LOCKED      (-1)
#define UNSPECIFIED  (0)


//#warning "TODO: change SYMBOL_DEF to symb->type."

#define SYMBOL_DEF  ((symbol_t *) -1)

/* symbol types: 
 *
 * LEXIC :     toutes les parties du discours (y compris 'PNC', 'CHFA', et '?') mais pas epsilon
 * epsilon :   ne devrait jamais apparaitre normalement ?
 * CODE :      tous les traits discriminant sont fixes, concorde avec un code de la POS (si elle en contient)
 *             pas de forme canonique
 * ATOM :      CODE + 1 forme canonique (forme flechie optionnelle)
 * CODE_NEG :  CODE + ens de negation de formes canonique
 * INC :       au moins un trait discriminant est UNSPECIFIED, concorde avec un code ...  pas de forme canoniqe
 * INC_CAN :   INC + forme canonique
 * INC_NEG :   INCOMPLET plus ensemble de negation de formes canoniques
 *
 * Les traits discriminants de tous les symbole doivent concorder avec au moins un code de la POS
 * correspondante (si elle en contient), dans le cas contraire le symbole est invalide.
 *
 * SPECIALS symbols:
 * EXCLAM | EQUAL : pour parser les grammaires de disambiguisation lors de leur compilation.
 */

enum {

  LEXIC = 'L', EPSILON = 'e',
  ATOM = 'a', CODE_NEG = 'N', CODE = 'C',
  INC_CAN = 'c', INC_NEG = 'n', INC = 'I',

  /* specials symbols */

  EXCLAM = '!', EQUAL = '='
};


#define SYMB_CODE_MASK (0xffffff00)

#define SYMB_CODE  (1 << 8)   // 0 == code ncomplet
#define SYMB_CAN   (2 << 8)
#define SYMB_NEG   (4 << 8)
#define SYMB_SPEC  (8 << 8)


typedef struct symbol_t {

  int type;

  bool negative;

  union { int form;    int nbnegs; };
  union { int canonic; int * negs; };

  POS_t * POS;

  char * traits;
  int  nbtraits;   // for dynamical add of new traits ...

  struct symbol_t * next;

} symbol_t;


/*
typedef struct symbol_t {

  int type;

  unichar * form;
  unichar * canonic;

  POS_t * POS;

  char * traits;
  int nbtraits;   // for dynamical add of new traits ...

  struct symbol_t * next;

} symbol_t;
*/

symbol_t * symbol_new(unichar * label);
symbol_t * symbol_new(POS_t * templat);

symbol_t * symbol_unknow_new(language_t * lang, int idx);

symbol_t * symbol_LEXIC_new();

symbol_t * symbol_dup(const symbol_t * symb);
symbol_t * symbols_dup(const symbol_t * symbs);

void symbol_delete(symbol_t * symb);
void symbols_delete(symbol_t * symb);

void symbol_empty(symbol_t * symb);
void symbol_copy(symbol_t * dest, symbol_t * src);

bool symbol_equals(symbol_t * a, symbol_t * b);

void symbol_dump_all(const symbol_t * symb, FILE * f = stderr);

void symbol_dump(const symbol_t * symb, FILE * f = stderr);
void symbols_dump(const symbol_t * symb, FILE * f = stderr);

void symbol_to_str(const symbol_t * s, ustring_t * ustr);
void symbol_to_grammar_label(const symbol_t * s, ustring_t * ustr);
void symbol_to_text_label(const symbol_t * s, ustring_t * ustr);
void symbol_to_implosed_text_label(const symbol_t * s, ustring_t * ustr);
void symbol_to_locate_label(const symbol_t * s, ustring_t * ustr);

symbol_t * load_text_symbol(language_t * lang, unichar * label);
symbol_t * load_grammar_symbol(language_t * lang, unichar * label);


int check_dic_entry(const unichar * label);
symbol_t * load_dic_entry(language_t * lang, const unichar * label, unichar * buf, bool warnmissing = true);

inline symbol_t * load_dic_entry(const unichar * label, unichar * buf, bool warnmissing = true) {
  return load_dic_entry(LANG, label, buf, warnmissing);
}



void symbols_concat(symbol_t * a, symbol_t * b, symbol_t ** end = NULL);

int symbol_type(symbol_t * symb);

symbol_t * symbol_epsilon_new();
symbol_t * symbol_UNIV_new();

symbol_t * symbol_PUNC_new(language_t * lang, int idx);
symbol_t * symbol_PUNC_new(int punc);

#endif

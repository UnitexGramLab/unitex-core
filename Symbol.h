/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef SymbolH
#define SymbolH

#include "Ustring.h"
#include "LanguageDefinition.h"
#include "DELA.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

struct POS_t;
struct language_t;


extern const unichar PUNC_TAB[];

/**
 * These constants are used to attribute special values to features.
 * LOCKED means that the feature value is not relevant, like tense for
 * nouns. UNSPECIFIED means that the feature can take any possible value.
 * It is used when a symbol is specified with a category like "<gender>"
 */
#define LOCKED      (-1)
#define UNSPECIFIED  (0)


//#warning "TODO: change SYMBOL_DEF to symb->type."
/* This special symbol is used to represent the default transition
 * used in the complementation algorithm. */
#define SYMBOL_DEF  ((symbol_t *) -1)


/**
 * The following enumeration defines the different kinds of symbols that
 * can appear in ELAG grammars.
 *
 * LEXIC :     every POS (including 'PNC', 'CHFA', and '?') but not epsilon
 * epsilon :   this should never appear
 * CODE :      POS category code
 *             +all discriminative categories have been set, if any
 *             +no lemma
 * ATOM :      the same as CODE, but with the lemma, and an optional inflected form
 * CODE_NEG :  the same as CODE with a set of forbidden lemmas
 * INC :       POS category code
 *             +at least one discriminative category has not been set
 *             +no lemma
 * INC_CAN :   the same as INC but with a lemma
 * INC_NEG :   the same as INC with a set of forbidden lemmas
 *
 * Note that discriminative categories must combine with the correct corresponding POS
 * category code. If not, the symbol is not valid.
 *
 * Special symbols:
 * EXCLAM and EQUAL : used to parse ELAG grammars before compiling them
 */
typedef enum {
   S_UNTYPED=-1,
   S_LEXIC='L',
   S_EPSILON='e',
   S_ATOM='a',
   S_CODE_NEG='N',
   S_INC_CAN='c',
   S_CODE='C',
   S_INC_NEG='n',
   S_INC='I',
   S_EXCLAM='!',
   S_EQUAL='='
} SymbolType;


#define SYMB_CODE_MASK (0xFFFFFF00)

#define SYMB_CODE  (1 << 8)   // 0 == code ncomplet
#define SYMB_CAN   (2 << 8)
#define SYMB_NEG   (4 << 8)
#define SYMB_SPEC  (8 << 8)


/**
 * This structure defines a list of symbols used an ELAG grammar.
 */
typedef struct symbol_t {
   /* The symbol type, see the enumeration above. We use an integer
    * here instead of a SymbolType, because it would raise many compilation
    * warnings about enumeration values not handled in switches. */
   SymbolType type;

   /* Is the lemma a negative one ? */
   bool negative;

   /* If the tag contains a negative lemma, then 'nbnegs' will be
    * the size of the 'negs' array. */
   union {
      int form;
      int nbnegs;
   };

   /* If the tag contains a lemma, then 'lemma' contains its index in
    * the language's forms; otherwise, 'negs' will contain the indices of
    * all the negative lemmas, by increasing order. */
   union {
      int lemma;
      int* negs;
   };

   /* The POS of the symbol */
   POS_t* POS;

   /* The 'feature' array is used to know if the feature #i has been set or not */
   char* feature;

   /* Size of 'feature' */
   int  nb_features;

   /* When a symbol represents a tag of a text automaton, we must remember with this
    * field to which TfstTag it corresponds. -1 means no tag. */
   int tfsttag_index;

   /* The next symbol in the list */
   struct symbol_t* next;
} symbol_t;



symbol_t* new_symbol(SymbolType,int);
symbol_t* new_symbol_POS(POS_t*,int);

symbol_t * new_symbol_UNKNOWN(language_t * lang, int idx,int);


symbol_t * dup_symbol(const symbol_t * symb);
symbol_t * dup_symbols(const symbol_t * symbs);
symbol_t * dup_symbols_identical(const symbol_t* symbs);

void free_symbol(symbol_t*);
void free_symbols(symbol_t*);

void empty_symbol(symbol_t * symb);
void copy_symbol(symbol_t * dest, symbol_t * src);

bool symbol_equals(symbol_t * a, symbol_t * b);

void symbol_dump_all(language_t* language,const symbol_t * symb);

void symbol_dump(const symbol_t * symb);
void symbols_dump(const symbol_t * symb);

void symbol_to_str(const symbol_t * s, Ustring * ustr);
void symbol_to_grammar_label(const symbol_t * s, Ustring * ustr);
void symbol_to_text_label(const symbol_t * s, Ustring * ustr);
struct dela_entry* symbol_to_dela_entry(const symbol_t*);
void symbol_to_locate_label(const symbol_t * s, Ustring * ustr);

symbol_t* load_text_symbol(language_t*,unichar*,int);
symbol_t* load_grammar_symbol(language_t*,unichar*);


int check_dic_entry(const unichar * label);
symbol_t* load_dic_entry(language_t*,unichar*,struct dela_entry*,int);

void concat_symbols(symbol_t * a, symbol_t * b, symbol_t ** end = NULL);

int type_symbol(symbol_t * symb);

symbol_t * new_symbol_PUNC(language_t * lang, int idx,int);

} // namespace unitex

#endif

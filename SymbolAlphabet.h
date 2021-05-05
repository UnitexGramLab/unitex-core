/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef SymbolAlphabetH
#define SymbolAlphabetH

#include "Symbol.h"
#include "Ustring.h"
#include "String_hash.h"
#include "SingleGraph.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This library is used to manage an alphabet made of Elag symbols.
 *
 */


/**
 * This structure is used to manage the alphabet of an Elag
 * grammar, where the alphabet elements are symbols.
 */
typedef struct {
   /* This is a string for internal use */
   Ustring* ustr;
   /* Structure containing all the symbols */
   struct string_hash_ptr* symbols;
} SymbolAlphabet;



SymbolAlphabet* new_SymbolAlphabet();
void free_SymbolAlphabet(SymbolAlphabet*);
SymbolAlphabet* build_symbol_alphabet(SingleGraph);
int add_symbol(SymbolAlphabet*,symbol_t*);
int alphabet_lookup(SymbolAlphabet*,symbol_t*);

} // namespace unitex

#endif

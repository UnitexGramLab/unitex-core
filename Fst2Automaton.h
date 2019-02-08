/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef Fst2AutomatonH
#define Fst2AutomatonH

#include "Unicode.h"
#include "Symbol.h"
#include "SingleGraph.h"
#include "String_hash.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure defines an automaton of .fst2, ready to be used
 * for by ELAG programs.
 */
typedef struct {
   /* For a grammar, it will be the
    * name of the subgraph. */
   unichar* name;

   /* The automaton itself, with the PTR_TAGS type */
   SingleGraph automaton;

} Fst2Automaton;


Fst2Automaton* new_Fst2Automaton(const unichar* name=NULL,int size=8);
void free_Fst2Automaton_including_symbols(Fst2Automaton*);
void free_Fst2Automaton_excluding_symbols(Fst2Automaton*);
void free_Fst2Automaton(Fst2Automaton*,void (*free_elag_symbol)(symbol_t*));
void add_transition(SingleGraph,struct string_hash_ptr*,int,symbol_t*,int);
void save_automaton(const Fst2Automaton * A, const char * name, const VersatileEncodingConfig*, int type);

} // namespace unitex

#endif

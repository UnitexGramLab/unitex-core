/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "SymbolAlphabet.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates, initializes and returns a new symbol alphabet.
 */
SymbolAlphabet* new_SymbolAlphabet() {
SymbolAlphabet* alph=(SymbolAlphabet*)malloc(sizeof(SymbolAlphabet));
if (alph==NULL) {
   fatal_alloc_error("new_SymbolAlphabet");
}
alph->ustr=new_Ustring();
alph->symbols=new_string_hash_ptr();
return alph;
}


/**
 * Frees the memory associated to the given symbol alphabet.
 * Note that symbols are not freed since they are shared
 * with the original grammar they come from.
 */
void free_SymbolAlphabet(SymbolAlphabet* alph) {
if (alph==NULL) return;
free_Ustring(alph->ustr);
free_string_hash_ptr(alph->symbols,NULL);
free(alph);
}


/**
 * Tests if the given symbol belongs to the given symbol alphabet.
 * Returns the index of the symbol or -1 if not found.
 */
int alphabet_lookup(SymbolAlphabet* alph,symbol_t* s) {
symbol_to_str(s,alph->ustr);
return get_value_index(alph->ustr->str,alph->symbols,DONT_INSERT);
}


/**
 * Adds the given symbol to the given symbol alphabet.
 * Returns the index of the symbol in the alphabet's table.
 * Note that if 's' refers to a symbol list, only the first
 * symbol pointed by 's' will be taken into account.
 */
int add_symbol(SymbolAlphabet* alph,symbol_t* s) {
symbol_to_str(s,alph->ustr);
return get_value_index(alph->ustr->str,alph->symbols,INSERT_IF_NEEDED,s);
}


/**
 * Builds the symbol alphabet corresponding to the given automaton.
 */
SymbolAlphabet* build_symbol_alphabet(SingleGraph A) {
SymbolAlphabet* alph=new_SymbolAlphabet();
bool transdef=false;
for (int i=0;i<A->number_of_states;i++) {
   for (Transition* t=A->states[i]->outgoing_transitions;t!=NULL;t=t->next) {
      add_symbol(alph,(symbol_t*)t->label);
   }
   if (!transdef && A->states[i]->default_state!=-1) {
      transdef=true;
      add_symbol(alph,SYMBOL_DEF);
   }
}
return alph;
}

} // namespace unitex

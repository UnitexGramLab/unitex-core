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

#include "ElagFstFilesIO.h"
#include "Fst2Automaton.h"
#include "Error.h"
#include "SingleGraph.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {


/**
 * Allocates, initializes and return a new .fst2 automaton. If size<0,
 * the automaton field is set to NULL.
 */
Fst2Automaton* new_Fst2Automaton(const unichar* name,int size) {
Fst2Automaton* aut=(Fst2Automaton*)malloc(sizeof(Fst2Automaton));
if (aut==NULL) {
   fatal_alloc_error("new_Fst2Automaton");
}
aut->name=u_strdup(name);
if (size>=0) {
   aut->automaton=new_SingleGraph(size,PTR_TAGS);
} else {
   aut->automaton=NULL;
}
return aut;
}


/**
 * Frees all the memory associated to the given automaton, except
 * the symbols.
 */
void free_Fst2Automaton(Fst2Automaton* A,void (*free_elag_symbol)(symbol_t*)) {
if (A==NULL) return;
if (A->name!=NULL) free(A->name);
free_SingleGraph(A->automaton,free_elag_symbol);
free(A);
}


void free_Fst2Automaton_excluding_symbols(Fst2Automaton* A) {
free_Fst2Automaton(A,NULL);
}

void free_Fst2Automaton_including_symbols(Fst2Automaton* A) {
free_Fst2Automaton(A,free_symbol);
}


/**
 * Adds a transition to 'automaton'.
 */
void add_transition(SingleGraph automaton,struct string_hash_ptr* symbols,int from,
                    symbol_t* label,int to) {
if (label==SYMBOL_DEF) {
   if (automaton->states[from]->default_state!=-1) {
      fatal_error("add_transition: more than one default transition\n");
   }
   automaton->states[from]->default_state=to;
   return;
}
while (label!=NULL) {
   if (label==SYMBOL_DEF) {
      fatal_error("add_transition: unexpected default transition\n");
   }
   /* We build a string representation of the symbol to avoid
    * duplicates in the value array */
   Ustring* u=new_Ustring();
   symbol_to_str(label,u);
   int n=get_value_index(u->str,symbols,INSERT_IF_NEEDED,label);
   free_Ustring(u);
   add_outgoing_transition(automaton->states[from],n,to);
   label=label->next;
}
}


/**
 * This function saves the given fst2 automaton into a file
 * with the given name. 'type' indicates the kind of automaton
 * (text fst, elag grammar, ...).
 */
void save_automaton(const Fst2Automaton* A,const char* name, const VersatileEncodingConfig* vec,int type) {
Elag_fst_file_out* fstf=fst_file_out_open(vec,name,type);
if (fstf==NULL) {
   error("Unable to open '%s'\n",name);
   return;
}
fst_file_write(fstf,A);
fst_file_close_out(fstf);
}

} // namespace unitex

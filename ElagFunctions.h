/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/* Nom 		: elag-functions.h */
/* Date 	: juin 98 */
/* Auteur(s) 	: MAYER Laurent et al */

#ifndef ElagFunctionsH
#define ElagFunctionsH


#include "Fst2Automaton.h"
#include "Vector.h"
#include "LanguageDefinition.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

void remove_ambiguities(const char* input_tfst,vector_ptr* grammars,const char* output_tfst, const VersatileEncodingConfig*,
		language_t* language);
void explode_tfst(const char* input_tfst,const char* output_tfst, const VersatileEncodingConfig*,language_t* language,struct hash_table* form_frequencies);
vector_ptr* load_elag_grammars(const VersatileEncodingConfig*,const char* filename,language_t* language,const char* directory);

} // namespace unitex

#endif

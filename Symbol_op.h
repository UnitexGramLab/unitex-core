 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef _SYMBOL_OP_H_
#define _SYMBOL_OP_H_

#include "Symbol.h"

/* operations ensemblistes sur les symboles  */

/* comparaison */

int symbol_compare(const symbol_t * a, const symbol_t * b);

/* appartenance */

bool symbol_in_symbol(const symbol_t * a, const symbol_t * b);
bool symbol_in_symbols(const symbol_t * a, const symbol_t * b);
bool symbols_in_symbols(const symbol_t * a, const symbol_t * b);

/* intersection */

symbol_t * symbol_inter_symbol(const symbol_t * a, const symbol_t * b);
symbol_t * symbol_inter_symbols(const symbol_t * a, const symbol_t * B);
symbol_t * symbols_inter_symbols(const symbol_t * A, const symbol_t * B);


/* complementation */

symbol_t * symbol_minus_symbol(language_t* language,const symbol_t* a,const symbol_t* b);
symbol_t * symbol_minus_symbols(language_t* language,const symbol_t * a, const symbol_t * B);
symbol_t * symbols_minus_symbols(language_t* language,const symbol_t * A, const symbol_t * B);
symbol_t * symbols_minus_symbol(language_t* language,const symbol_t * A, const symbol_t * B);

symbol_t * minus_symbol(language_t* language,const symbol_t * b);
symbol_t * minus_symbols(language_t* language,const symbol_t * B);


symbol_t* LEXIC_minus_POS(language_t* language,POS_t* POS);

#endif

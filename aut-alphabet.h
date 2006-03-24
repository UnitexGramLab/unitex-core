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

#ifndef _ALPHABET_H_
#define _ALPHABET_H_

#include "hash_str_table.h"
#include "symbol.h"
#include "ustring.h"
#include "autalmot.h"


typedef struct alphabet_t {
  ustring_t * ustr;
  hash_str_table_t * hash;
  // bool shared; // symbols partagés?
} alphabet_t;

alphabet_t * alphabet_new();
alphabet_t * alphabet_from_autalmot(autalmot_t * A);

void alphabet_delete(alphabet_t * alph);

int alphabet_add(alphabet_t * alph, symbol_t * s);
int alphabet_lookup(alphabet_t * alph, symbol_t * s);


void alphabet_dump(alphabet_t * alph, FILE * f = stderr);


inline int alphabet_size(alphabet_t * alph) { return alph->hash->nbelems; }

inline symbol_t * alphabet_get(alphabet_t * alph, int idx) { return (symbol_t *) alph->hash->tab[idx]; }

#endif

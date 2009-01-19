 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef PatternH
#define PatternH

#include "Unicode.h"
#include "List_ustring.h"
#include "String_hash.h"
#include "DELA.h"


/**
 * We define here the different kind of pattern that we can
 * have:
 *   TOKEN_PATTERN: hello
 *                  By convention, the token of such a pattern is stored
 *                  in the 'inflected' field.
 * 
 *   LEMMA_PATTERN: <be>
 *   CODE_PATTERN:  <N+z1:ms>
 *   LEMMA_AND_CODE_PATTERN: <be.V:K>
 *   FULL_PATTERN:  <am,be.V>
 */
enum pattern_type {
   UNDEFINED_PATTERN,
   TOKEN_PATTERN,
   LEMMA_PATTERN,
   CODE_PATTERN,
   LEMMA_AND_CODE_PATTERN,
   FULL_PATTERN
};


/**
 * This library provides functions for manipulating patterns. A pattern is defined by:
 * - a type that defines the kind of pattern it is
 * - an inflected form
 * - a lemma
 * - a list of grammatical/semantic codes
 * - a list of forbidden grammatical/semantic codes
 * - a list of inflectional codes
 * 
 * Some of these fields may be optional according to the pattern type.
 * All these list are sorted, and inflectional codes are supposed to be strings whose
 * characters are sorted. For instance, the pattern "<rouges,rouge.A+Couleur-z3:sm:sf>" will be
 * represented like this:
 *    pattern_type=FULL_PATTERN
 *    inflected="rouges"
 *    lemma="rouge"
 *    grammatical/semantic codes="Couleur", "A"
 *    forbidden grammatical/semantic codes="z3"
 *    inflectional codes="fs", "ms"
 */


/**
 * This structure defines a pattern.
 */
struct pattern {
   pattern_type type;
   unichar* inflected;
   unichar* lemma;
   struct list_ustring* grammatical_codes;
   struct list_ustring* forbidden_codes;
   struct list_ustring* inflectional_codes;
};


struct pattern* build_pattern(unichar*,struct string_hash*);
struct pattern* build_token_pattern(unichar*);
void free_pattern(struct pattern*);
int is_entry_compatible_with_pattern(struct dela_entry* entry,struct pattern* pattern);

#endif

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

#ifndef LocateConstantsH
#define LocateConstantsH

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/* Bit masks used in the control byte of each token of the text */
#define MOT_TOKEN_BIT_MASK 1
#define DIC_TOKEN_BIT_MASK 2
#define MAJ_TOKEN_BIT_MASK 4
#define MIN_TOKEN_BIT_MASK 8
#define PRE_TOKEN_BIT_MASK 16
#define CDIC_TOKEN_BIT_MASK 32
#define NOT_DIC_TOKEN_BIT_MASK 64
#define TDIC_TOKEN_BIT_MASK 128

/* This negative constant is used to indicate that we want all the matches */
#define NO_MATCH_LIMIT -1


/* Match policy for the Locate program */
typedef enum {
   LONGEST_MATCHES,
   SHORTEST_MATCHES,
   ALL_MATCHES
} MatchPolicy;


/* Here are the different output policies for the Locate program */
typedef enum {
   IGNORE_OUTPUTS,
   MERGE_OUTPUTS,
   REPLACE_OUTPUTS,
   DEBUG_OUTPUTS
} OutputPolicy;

typedef enum {
   IGNORE_AMBIGUOUS_OUTPUTS,
   ALLOW_AMBIGUOUS_OUTPUTS
} AmbiguousOutputPolicy;


/* Do we allow or not to occurrences that start with spaces ? */
typedef enum {
   START_WITH_SPACE,
   DONT_START_WITH_SPACE
} SpacePolicy;


/* Tokenization policy */
typedef enum {
   /* Word by word tokenization, where letters are defined by an alphabet */
   WORD_BY_WORD_TOKENIZATION,
   /* Character by character tokenization */
   CHAR_BY_CHAR_TOKENIZATION,
   /* Word by word tokenization, where letters are defined by the 'u_is_letter'
    * function of the Unicode library */
   DEFAULT_TOKENIZATION
} TokenizationPolicy;


/* How to behave when an output containing an invalid variable is found
 * with output policy!=IGNORE_OUTPUTS ? */
typedef enum {
   EXIT_ON_VARIABLE_ERRORS,
   IGNORE_VARIABLE_ERRORS,
   BACKTRACK_ON_VARIABLE_ERRORS
} VariableErrorPolicy;

} // namespace unitex

#endif


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

#ifndef MorphologicalFiltersH
#define MorphologicalFiltersH

/**
 * This library provides facilities to manipulate morphological filters
 * in the form of POSIX regular expressions. Such filters can be used in
 * Locate grammars (example: <<able$>> = something that ends with "able").
 * These filters are manipulated with the TRE regular expression library:
 *
 * http://laurikari.net/tre/
 *
 * The integration of these filters in Unitex has been made by
 * Claude Devis (devis@tedm.ucl.ac.be).
 */


#include "Unicode.h"
#include "RegExFacade.h"
#include "Alphabet.h"
#include "String_hash.h"
#include "Fst2.h"
#include "BitArray.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {


/**
 * This structure defines a morphological filter.
 */
typedef struct {
   /* The options strings can contain 'f' and/or 'b'. 'f' force case respect and 'b'
    * disables POSIX extended regular expressions. */
   char* options;
   /* 'content' contains the regular expression as a string */
   unichar* content;
   /* 'matcher' is a TRE object that represents an automaton that can match the same
    * things that the given regular expression. */
   regex_facade_regex_t* matcher;
} MorphoFilter;


/**
 * This is a collection of morphological filters.
 */
typedef struct {
   int size;
   MorphoFilter* filter;
} FilterSet;


/**
 * This structure is used to know for each token of a text if it verifies or
 * not morphological filters.
 */
typedef struct {
   /* Number of filters */
   int size;
   /* Each filter is associated to a bit array so that (filter #x)->bit[y]=1 means
    * that the filter number x can match the token number y.
    *
    * Note that a bit array can be NULL if the filter matches no token at all. */
   struct bit_array** matching_tokens;
} FilterMatchIndex;


FilterSet* new_FilterSet(Fst2*,Alphabet*);
void free_FilterSet(FilterSet*);

FilterMatchIndex* new_FilterMatchIndex(FilterSet*,struct string_hash*);
void free_FilterMatchIndex(FilterMatchIndex*);

int string_match_filter(const FilterSet*,const unichar*,int);
int string_match_filter(const FilterSet*,const unichar*,int,unichar_regex*,size_t);
int token_match_filter(FilterMatchIndex*,int,int);


} // namespace unitex

#endif

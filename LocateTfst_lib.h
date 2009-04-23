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

#ifndef LocateTfst_libH
#define LocateTfst_libH

#include "Unicode.h"
#include "Alphabet.h"
#include "Tfst.h"
#include "MorphologicalFilters.h"
#include "LocateTfstMatches.h"


#define OK_MATCH_STATUS 1
#define NO_MATCH_STATUS 2
#define TEXT_INDEPENDENT_MATCH 3

/**
 * This structure is used to wrap many information needed to perform the locate
 * operation on a text automaton.
 */
struct locate_tfst_infos {
	U_FILE* output;

	Alphabet* alphabet;

	int n_matches;

	Tfst* tfst;

	#ifdef TRE_WCHAR
	/* These field is used to manipulate morphological filters like:
	 *
	 * <<en$>>
	 *
	 * 'filters' is a structure used to store the filters.
	 */
	FilterSet* filters;

	struct tfst_simple_match_list* matches;
	#endif
};


int locate_tfst(char*,char*,char*,char*);


#endif


/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "LocateConstants.h"
#include "Korean.h"
#include "OptimizedTfstTagMatching.h"
#include "DicVariables.h"
#include "TransductionVariables.h"
#include "OutputTransductionVariables.h"
#include "Vector.h"

/* Following values must be !=-1, because -1 is used in
 * OptimizedTfstTagMatching to indicate that the result of a match
 * is not known yet */
#define OK_MATCH_STATUS 1
#define NO_MATCH_STATUS 2
#define TEXT_INDEPENDENT_MATCH 3
#define PARTIAL_MATCH_STATUS 4

/**
 * This structure is used to wrap many information needed to perform the locate
 * operation on a text automaton.
 */
struct locate_tfst_infos {
	U_FILE* output;

	Alphabet* alphabet;
	
	Fst2* fst2;

	int number_of_matches;

    /* Indicates the number of matches we want, or NO_MATCH_LIMIT (-1) if
     * there is no limit. */
    int search_limit;

	Tfst* tfst;

	#ifdef TRE_WCHAR
	/* These field is used to manipulate morphological filters like:
	 *
	 * <<en$>>
	 *
	 * 'filters' is a structure used to store the filters.
	 */
	FilterSet* filters;
	#endif

	MatchPolicy match_policy;
	OutputPolicy real_output_policy;
	OutputPolicy output_policy;
	AmbiguousOutputPolicy ambiguous_output_policy;
	VariableErrorPolicy variable_error_policy;
	
	Variables* input_variables;
	OutputVariables* output_variables;
	struct dic_variable* dic_variables;

	/* The total number of outputs. It may be different from the number
	 * of matches if ambiguous outputs are allowed. */
	int number_of_outputs;

	/* Position of the last printed match. It is used when ambiguous outputs
	 * are used. */
	int start_position_last_printed_match_token;
	int end_position_last_printed_match_token;
	int start_position_last_printed_match_char;
	int end_position_last_printed_match_char;
	int start_position_last_printed_match_letter;
	int end_position_last_printed_match_letter;
	
	struct tfst_simple_match_list* matches;
	
	/* Stuffs for Korean */
	Korean* korean;
	int n_jamo_fst2_tags;
	unichar** jamo_fst2_tags;
	int n_jamo_tfst_tags;
	unichar** jamo_tfst_tags;
	
	LocateTfstTagMatchingCache* cache;
	struct opt_contexts** contexts;

	int debug;
	int tagging;
};


int locate_tfst(const char*,const char*,const char*,const char*, const VersatileEncodingConfig*,MatchPolicy,OutputPolicy,AmbiguousOutputPolicy,
                VariableErrorPolicy,int,int,int,vector_ptr*,int);


#endif


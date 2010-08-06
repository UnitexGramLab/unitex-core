/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef MorphologicalLocateH
#define MorphologicalLocateH

#include "LocateMatches.h"
#include "Fst2.h"
#include "TransductionStack.h"
#include "String_hash.h"
#include "ParsingInfo.h"
#include "LocateConstants.h"
#include "CompoundWordTree.h"
#include "LocatePattern.h"
#include "MorphologicalFilters.h"


void enter_morphological_mode(/*int graph_depth, */ /* 0 means that we are in the top level graph */
            int state, /* current state in the grammar */
            int pos, /* position in the token buffer, relative to the current origin */
            //int depth, /* number of nested calls to 'locate' */
            struct parsing_info** matches, /* current match list. Irrelevant if graph_depth==0 */
            int n_matches, /* number of sequences that have matched. It may be different from
                            * the length of the 'matches' list if a given sequence can be
                            * matched in several ways. It is used to detect combinatory
                            * explosions due to bad written grammars. */
            struct list_int* ctx, /* information about the current context, if any */
            struct locate_parameters* p, /* miscellaneous parameters needed by the function */
            variable_backup_memory_reserve* backup_reserve
);

#endif



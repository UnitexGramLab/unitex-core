/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/* Created by Agata Savary (savary@univ-tours.fr)
 */


#ifndef MF_GlobalH
#define MF_GlobalH


#include "MF_InflectTransdBase.h"
#include "MF_MU_morphoBase.h"
#include "MF_UnifBase.h"

typedef struct {

// GLOBAL VARIABLES
///////////////////////////////
///////////////////////////////
// Root of the tree for inflection transducers' names
struct node* root;

///////////////////////////////
// Table of inflection tranducers
struct FST2_free_info fst2_free[N_FST2];
Fst2* fst2[N_FST2];

///////////////////////////////
// Number of inflection tranducers
int n_fst2;

///////////////////////////////
// Directory containing the inflection tranducers
char inflection_directory[FILENAME_MAX];

int T;

////////////////////////////////////////////
// Current multi-word unit to be inflected
MU_lemma_T* MU_lemma;

unichar Variables_op[22][100];
int save_pos;

unif_vars_T UNIF_VARS;
} MultiFlex_ctx;

#endif

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

#include "MF_MU_morphoBase.h"
#include "MF_UnifBase.h"
#include "AbstractFst2Load.h"
#include "MF_LangMorpho.h"
#include "FileEncoding.h"
#include "Korean.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {



//Maximum number of flexional transducers
#define N_FST2 5000

///////////////////////////////
// A node of the tree for inflection transducers' names
struct node {
  int final;  //-1 if the node is non final; otherwise gives the index
              //of the inflection transducer (whose name leads to this node)
              //in the tranducer table
  struct transition* t;
};

///////////////////////////////
// A branch of the tree for inflection transducers' names
struct transition {
    char c;
    struct node* n;
    struct transition* suivant;
};



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

struct l_morpho_t* pL_MORPHO;

VersatileEncodingConfig* vec;

Korean* korean;

int semitic;

char* pkgdir;

char* named_repositories;

} MultiFlex_ctx;


MultiFlex_ctx* new_MultiFlex_ctx(const char* inflection_dir,const char* morphologyTxt,
								VersatileEncodingConfig* vec,Korean* koran,
								const char* pkgdir,const char* named_repositories);
void free_MultiFlex_ctx(MultiFlex_ctx* ctx);

int get_transducer(MultiFlex_ctx* p_multiFlex_ctx,char* flex);

} // namespace unitex

#endif

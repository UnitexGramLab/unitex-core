/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdlib.h>
#include "MF_Global.h"
#include "Error.h"
#include "File.h"
#include "Grf2Fst2.h"
#include "MF_Global.h"
#include "MF_DicoMorpho.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

int init_transducer_tree(MultiFlex_ctx* p_multiFlex_ctx);
void free_transducer_tree(MultiFlex_ctx* p_multiFlex_ctx);
struct node* new_node();
struct transition* new_transition(char c);
void free_transition(struct transition* t);
void free_node(struct node* n);
struct transition* get_transition(char c,struct transition* t,struct node** n);
int get_node(MultiFlex_ctx* p_multiFlex_ctx,char* flex,int pos,struct node* n);



MultiFlex_ctx* new_MultiFlex_ctx(const char* inflection_dir,const char* morphologyTxt,
                                const char* equivalencesTxt,
                                VersatileEncodingConfig* vec,Korean* korean,
                                const char* pkgdir,const char* named_repositories,
                                GraphRecompilationPolicy graph_recompilation_policy) {
MultiFlex_ctx* ctx = (MultiFlex_ctx*)malloc(sizeof(MultiFlex_ctx));
if (ctx==NULL) {
   fatal_alloc_error("new_MultiFlex_ctx");
}
ctx->config_files_status=CONFIG_FILES_OK;
strcpy(ctx->inflection_directory,inflection_dir);
if (init_transducer_tree(ctx)) {
   fatal_error("init_transducer_tree error\n");
}
ctx->pL_MORPHO=init_langage_morph();
if (ctx->pL_MORPHO == NULL) {
   fatal_error("init_langage_morph error\n");
}
ctx->vec=vec;
if (morphologyTxt!=NULL && morphologyTxt[0]!='\0'
        && 0!=read_language_morpho(vec,ctx->pL_MORPHO,morphologyTxt)) {
   error("read_language_morpho error\n");
   ctx->config_files_status=CONFIG_FILES_ERROR;
}
if (ctx->config_files_status!=CONFIG_FILES_ERROR
        && equivalencesTxt!=NULL && equivalencesTxt[0]!='\0'
        && 0!=d_init_morpho_equiv(vec,ctx->pL_MORPHO,equivalencesTxt)) {
       error("d_init_morpho_equiv error\n");
       ctx->config_files_status=CONFIG_FILES_ERROR;
}
ctx->graph_recompilation_policy = graph_recompilation_policy;
ctx->semitic=0;
ctx->pkgdir=NULL;
if (pkgdir!=NULL) {
    ctx->pkgdir=strdup(pkgdir);
    if (ctx->pkgdir==NULL) {
        fatal_alloc_error("new_MultiFlex_ctx");
    }
}
ctx->named_repositories=NULL;
if (named_repositories!=NULL) {
    ctx->named_repositories=strdup(named_repositories);
    if (ctx->named_repositories==NULL) {
       fatal_alloc_error("new_MultiFlex_ctx");
    }
}
ctx->korean=korean;
if (ctx->config_files_status!=CONFIG_FILES_ERROR) {
    d_init_class_equiv(ctx->pL_MORPHO,&(ctx->D_CLASS_EQUIV));
}
ctx->n_filter_codes=0;
ctx->filter_codes=NULL;
return ctx;
}


void free_MultiFlex_ctx(MultiFlex_ctx* ctx) {
if (ctx==NULL) return;
free_language_morpho(ctx->pL_MORPHO);
free(ctx->pkgdir);
free(ctx->named_repositories);
free_transducer_tree(ctx);
free(ctx);
}

///////////////////////////////
//Initiate the tree for inflection transducers' names
//On succes return 0, 1 otherwise
int init_transducer_tree(MultiFlex_ctx* p_multiFlex_ctx) {
  p_multiFlex_ctx->n_fst2 = 0;
  p_multiFlex_ctx->root=new_node();
  if (!(p_multiFlex_ctx->root)) {
    fatal_error("Transducer tree could not be initialized in function 'init_transducer_tree'\n");
  }
  return  0;
}

///////////////////////////////
// Free the transducer tree memory
void free_transducer_tree(MultiFlex_ctx* p_multiFlex_ctx) {
free_node(p_multiFlex_ctx->root);
}

///////////////////////////////
// Try to load the transducer flex and returns its position in the
// 'fst2' array.
int get_transducer(MultiFlex_ctx* p_multiFlex_ctx,char* flex) {
return get_node(p_multiFlex_ctx,flex,0,p_multiFlex_ctx->root);
}

///////////////////////////////
// Create a new node in the tree
struct node* new_node() {
struct node* n=(struct node*)malloc(sizeof(struct node));
if (n==NULL) {
   fatal_alloc_error("new_node");
}
n->final=-1;
n->t=NULL;
return n;
}

///////////////////////////////
// Create a new branch in the tree
struct transition* new_transition(char c) {
struct transition* t=(struct transition*)malloc(sizeof(struct transition));
if (t==NULL) {
   fatal_alloc_error("new_transition");
}
t->c=c;
t->n=NULL;
t->suivant=NULL;
return t;
}

///////////////////////////////
// Free the branch
void free_transition(struct transition* t) {
struct transition* tmp;
while (t!=NULL) {
    free_node(t->n);
    tmp=t;
    t=t->suivant;
    free(tmp);
}
}

///////////////////////////////
// Free a node
void free_node(struct node* n) {
if (n==NULL) {
  error("NULL error in free_node\n");
  return;
}
free_transition(n->t);
free(n);
}

///////////////////////////////
// Looks for a transition by the char c
// Creates it if it does not exist
struct transition* get_transition(char c,struct transition* t,struct node** n) {
struct transition* tmp;
while (t!=NULL) {
    if (t->c==c) return t;
    t=t->suivant;
}
tmp=new_transition(c);
tmp->suivant=(*n)->t;
tmp->n=NULL;
(*n)->t=tmp;
return tmp;
}


/**
 * Returns 0 if
 *  -  The .grf or the .fst2 uses an installed abstract filespace; or
 *  -  The .grf file does not exists; or
 *  -  GraphRecompilationPolicy is NEVER_RECOMPILE
 * Returns 1 if
 *  - fst2 file does not exists; or
 *  - GraphRecompilationPolicy is :
 *      ALWAYS_RECOMPILE or
 *      ONLY_OUT_OF_DATE and the .grf exists and is more recent than the .fst2
 */
int must_compile_grf(char* grf,char* fst2, GraphRecompilationPolicy graph_recompilation_policy) {
  // if the fst2 or the grf uses an installed abstract filespace,
  // there is nothing to compile
  if ((is_filename_in_abstract_file_space(fst2) != 0) ||
      (is_filename_in_abstract_file_space(grf)  != 0)) {
      /* abstract ? no compare, no recompile*/
      return 0;
  }

  // if .grf does not exits, there is nothing to compile
  if (!fexists(grf)) {
     /* No .grf? We fail */
     return 0;
  }

  // if .fst2 does not exits, we should compile the .grf
  if (!fexists(fst2)) {
     /* A grf and no .fst2? Let's compile the .grf! */
     return 1;
  }


  // There are both .grf and .fst2 files, use the GraphRecompilationPolicy
  int recompilation_status = 0;
  switch(graph_recompilation_policy) {
    case ALWAYS_RECOMPILE : recompilation_status = 1; break;
    case ONLY_OUT_OF_DATE : recompilation_status = (get_file_date(grf) >= get_file_date(fst2)); break;
    //   NEVER_RECOMPILE
    default               : break;
  }

  // return 0 otherwise
  return recompilation_status;
}


///////////////////////////////
// Look for the path to 'flex', creating it if necessary
// The current node is n, and pos is the position in the flex string
int get_node(MultiFlex_ctx* p_multiFlex_ctx,char* flex,int pos,struct node* n) {
if (flex[pos]=='\0') {
    // we are at the final node for flex (a leaf)
    if (n->final!=-1) {
        // if the automaton already exists we returns its position in the transducer array (fst2)
        return n->final;
    }
    else {
        // else we load it
        if ((p_multiFlex_ctx->n_fst2)==N_FST2) {
            fatal_error("Memory error: too much inflectional transducers\n");
        }
        char s[FILENAME_MAX];
        new_file(p_multiFlex_ctx->inflection_directory,flex,s);
        strcat(s,".fst2");
        char grf[FILENAME_MAX];
        new_file(p_multiFlex_ctx->inflection_directory,flex,grf);
        strcat(grf,".grf");
        if (must_compile_grf(grf,s,p_multiFlex_ctx->graph_recompilation_policy)) {
           /* Following the GraphRecompilationPolicy, if there is no .fst2 file,
            * of a one than is older than the corresponding .grf, we try to compile it */
           pseudo_main_Grf2Fst2(p_multiFlex_ctx->vec,grf,1,NULL,1,0,p_multiFlex_ctx->pkgdir,
                   p_multiFlex_ctx->named_repositories,0);
        }
        p_multiFlex_ctx->fst2[p_multiFlex_ctx->n_fst2]=load_abstract_fst2(p_multiFlex_ctx->vec,s,1,&(p_multiFlex_ctx->fst2_free[p_multiFlex_ctx->n_fst2]));
        n->final=p_multiFlex_ctx->n_fst2;
        return (p_multiFlex_ctx->n_fst2)++;
        }
}
// if we are not at the end of the string flex
struct transition* trans=get_transition(flex[pos],n->t,&n);
if (trans->n==NULL) {
    trans->n=new_node();
}
return get_node(p_multiFlex_ctx,flex,pos+1,trans->n);
}




} // namespace unitex


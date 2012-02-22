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

#include "NormalizationFst2.h"
#include "Error.h"
#include "Transitions.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure represents a list of pairs (output,normalization tree node).
 * It is used during the exploration of the normalization fst2. It must not
 * be visible from the outside.
 */
struct norm_info {
   unichar* output;
   struct normalization_tree* node;
   struct norm_info* next;
};


void free_normalization_tree_transition(struct normalization_tree_transition*);



/**
 * Allocates, initializes and returns a new normalization tree node.
 */
struct normalization_tree* new_normalization_tree() {
struct normalization_tree* n=(struct normalization_tree*)malloc(sizeof(struct normalization_tree));
if (n==NULL) {
   fatal_alloc_error("new_normalization_tree");
}
n->outputs=NULL;
n->trans=NULL;
return n;
}


/**
 * Allocates, initializes and returns a new normalization tree transition.
 */
struct normalization_tree_transition* new_normalization_tree_transition(int token,
                                                    struct normalization_tree* dest_node,
                                                    struct normalization_tree_transition* next) {
struct normalization_tree_transition* tmp;
tmp=(struct normalization_tree_transition*)malloc(sizeof(struct normalization_tree_transition));
if (tmp==NULL) {
   fatal_alloc_error("new_normalization_tree_transition");
}
tmp->token=token;
tmp->node=dest_node;
tmp->next=next;
return tmp;
}


/**
 * Frees all the memory associated to the given normalization tree.
 * Note that it assumes that transitions are tagged with token numbers.
 */
void free_normalization_tree(struct normalization_tree* n) {
if (n==NULL) return;
free_list_ustring(n->outputs);
free_normalization_tree_transition(n->trans);
free(n);
}


/**
 * Frees all the memory associated to the given normalization tree transition.
 * Note that it assumes that transitions are tagged with token numbers.
 */
void free_normalization_tree_transition(struct normalization_tree_transition* t) {
struct normalization_tree_transition* tmp;
while (t!=NULL) {
   free_normalization_tree(t->node);
   tmp=t;
   t=t->next;
   free(tmp);
}
}


/**
 * This function looks for a transition tagged with the given token number.
 */
struct normalization_tree_transition* get_transition(int token,struct normalization_tree_transition* t) {
if (t==NULL) return NULL;
if (t->token==token) return t;
return get_transition(token,t->next);
}



//
// this function looks for a transition by a token represented by a string
//
struct normalization_tree_transition* get_trans_arbre_normalization_string(unichar* s,struct normalization_tree_transition* t) {
if (t==NULL) return NULL;
if (!u_strcmp(t->s,s)) {
   return t;
}
return get_trans_arbre_normalization_string(s,t->next);
}


/**
 * Allocates, initializes and returns a new norm_info structure.
 */
struct norm_info* new_norm_info(const unichar* output,struct normalization_tree* n) {
struct norm_info* t=(struct norm_info*)malloc(sizeof(struct norm_info));
if (t==NULL) {
   fatal_alloc_error("new_norm_info");
}
t->output=u_strdup(output);
t->node=n;
t->next=NULL;
return t;
}


/**
 * Inserts the given (output,node) couple in the given list, if not
 * already present.
 */
struct norm_info* insert_in_norm_info_list(const unichar* output,
                                           struct normalization_tree* n,
                                           struct norm_info* l) {
if (l==NULL) return new_norm_info(output,n);
if (l->node==n && !u_strcmp(l->output,output)) return l;
l->next=insert_in_norm_info_list(output,n,l->next);
return l;
}


/**
 * Frees all the memory associated to the given norm_info list.
 */
void free_norm_info(struct norm_info* t) {
if (t==NULL) return;
if (t->output!=NULL) free(t->output);
free(t);
}


/**
 * Returns 1 if the string only contains spaces; 0 otherwise.
 */
int only_spaces(unichar* s) {
int i=0;
while (s[i]!='\0') {
   if (s[i]!=' ') return 0;
   i++;
}
return 1;
}


/**
 * This function explore the normalization grammar to construct
 * the normalization tree. If the 'list' parameter is NULL, then we
 * are in the main call to the main graph; otherwise, we are within
 * a subgraph.
 */
void explore_normalization_fst2(Fst2* fst2,int current_state,
                                struct normalization_tree* node,
                                struct string_hash* tokens,const unichar* output,
                                const Alphabet* alph,struct norm_info** list) {
Fst2State state=fst2->states[current_state];
if (is_final_state(state)) {
   /* If we are in a final state, we behave differently if we are in a subgraph
    * or in the main call to the main graph. */
   if (list!=NULL) {
      (*list)=insert_in_norm_info_list(output,node,(*list));
   }
   else {
      node->outputs=sorted_insert(output,node->outputs);
   }
}
Transition* trans=state->transitions;
unichar tmp[1024];
while (trans!=NULL) {
   if (trans->tag_number<0) {
      /* Case of a subgraph call */
      struct norm_info* tmp_list=NULL;
      explore_normalization_fst2(fst2,fst2->initial_states[-(trans->tag_number)],node,
                                        tokens,output,alph,&tmp_list);
      while (tmp_list!=NULL) {
         /* We continue to explore the current graph */
         explore_normalization_fst2(fst2,trans->state_number,tmp_list->node,
                                        tokens,tmp_list->output,alph,list);
         struct norm_info* z=tmp_list;
         tmp_list=tmp_list->next;
         free_norm_info(z);
      }
   }
   else {
      /* If we have a normal transition */
      Fst2Tag tag=fst2->tags[trans->tag_number];
      u_strcpy(tmp,output);
      u_strcat(tmp," ");
      if (tag->output!=NULL && tag->output[0]!='\0' && u_strcmp(tag->output,"<E>") && !only_spaces(tag->output)) {
         /* We append the output if it exists and is not epsilon */
         u_strcat(tmp,tag->output);
      }
      if (!u_strcmp(tag->input,"<E>")) {
         /* If we have an epsilon transition, we go on in the fst2, but
          * we don't move in the normalization tree */
         explore_normalization_fst2(fst2,trans->state_number,node,tokens,tmp,alph,list);
      } else {
         /* If we have a normal transition, we explore all the tokens that match it */
         struct list_int* l=get_token_list_for_sequence(tag->input,alph,tokens);
         while (l!=NULL) {
            /* Then, we add a branch in the normalization tree for
             * each token. Note that it may introduce combinatory explosions
             * if the the fst2 matches large sequences */
            struct normalization_tree_transition* trans_norm;
            trans_norm=get_transition(l->n,node->trans);
            if (trans_norm==NULL) {
               /* If the transition does not exist in the tree, we create it */
               trans_norm=new_normalization_tree_transition(l->n,new_normalization_tree(),node->trans);
               node->trans=trans_norm;
            }
            explore_normalization_fst2(fst2,trans->state_number,trans_norm->node,
                                           tokens,tmp,alph,list);
            struct list_int* L=l;
            l=l->next;
            free(L);
         }
      }
   }
   trans=trans->next;
}
}


/**
 * This function constructs and returns a token tree from a normalization grammar.
 * Tokens are represented by integers.
 */
struct normalization_tree* load_normalization_fst2(const VersatileEncodingConfig* vec,const char* grammar,
		const Alphabet* alph,struct text_tokens* tok) {
struct FST2_free_info fst2_free;
Fst2* fst2=load_abstract_fst2(vec,grammar,0,&fst2_free);
if (fst2==NULL) {
   return NULL;
}
struct string_hash* hash=new_string_hash(DONT_USE_VALUES);
/* We create the token tree to speed up the consultation */
for (int i=0;i<tok->N;i++) {
   get_value_index(tok->token[i],hash);
}
struct normalization_tree* root=new_normalization_tree();
explore_normalization_fst2(fst2,fst2->initial_states[1],root,hash,U_EMPTY,alph,NULL);
free_abstract_Fst2(fst2,&fst2_free);
free_string_hash(hash);
return root;
}






//---------------- _string -----------------------------


struct normalization_tree_transition* new_trans_arbre_normalization_string(const unichar* s) {
struct normalization_tree_transition* tmp;
tmp=(struct normalization_tree_transition*)malloc(sizeof(struct normalization_tree_transition));
if (tmp==NULL) {
   fatal_alloc_error("new_trans_arbre_normalization_string");
}
tmp->s=u_strdup(s);
tmp->node=NULL;
tmp->next=NULL;
return tmp;
}





//
// this function explores a sub-graph, considering tokens as strings
//
void explorer_sub_automate_normalization_string(Fst2* automate,int n,
                                     struct normalization_tree* noeud_normalization,
                                     unichar* output,struct norm_info** TEMP_LIST) {
Fst2State etat;
etat=automate->states[n];
if (is_final_state(etat)) {
   // if we are in a final state
   (*TEMP_LIST)=insert_in_norm_info_list(output,noeud_normalization,(*TEMP_LIST));
}
Transition* trans;
trans=etat->transitions;
unichar tmp[1000];
while (trans!=NULL) {
   if (trans->tag_number<0) {
      // case of a sub-graph
      struct norm_info* TMP=NULL;
      explorer_sub_automate_normalization_string(automate,automate->initial_states[-(trans->tag_number)],noeud_normalization,
                                        output,&TMP);
      while (TMP!=NULL) {
         // we continue to explore the current automaton
         explorer_sub_automate_normalization_string(automate,trans->state_number,TMP->node,
                                        TMP->output,TEMP_LIST);
         struct norm_info* z=TMP;
         TMP=TMP->next;
         free_norm_info(z);
      }
   }
   else {
      // normal transition
      Fst2Tag etiq;
      etiq=automate->tags[trans->tag_number];
      u_strcpy(tmp,output);
      u_strcat(tmp," ");
      if (etiq->output!=NULL && u_strcmp(etiq->output,"")
          && u_strcmp(etiq->output,"<E>") && !only_spaces(etiq->output)) {
         // we append the output if it exists and is not epsilon
         u_strcat(tmp,etiq->output);
      }
      struct normalization_tree_transition* trans_norm;
      trans_norm=get_trans_arbre_normalization_string(etiq->input,noeud_normalization->trans);
      if (trans_norm==NULL) {
         // if the transition does not exist in the tree, we create it
         trans_norm=new_trans_arbre_normalization_string(etiq->input);
         // we also create the destination node
         trans_norm->node=new_normalization_tree();
         trans_norm->next=noeud_normalization->trans;
         noeud_normalization->trans=trans_norm;
      }
      explorer_sub_automate_normalization_string(automate,trans->state_number,trans_norm->node,
                                        tmp,TEMP_LIST);
   }
   trans=trans->next;
}
}



//
// this function explore the normalization grammar to construct the token tree
//
void explorer_automate_normalization_string(Fst2* automate,int n,
                                     struct normalization_tree* noeud_normalization,
                                     unichar* output) {
Fst2State etat;
etat=automate->states[n];
if (is_final_state(etat)) {
   // if we are in a final state
   noeud_normalization->outputs=sorted_insert(output,noeud_normalization->outputs);
}
Transition* trans;
trans=etat->transitions;
unichar tmp[1000];
while (trans!=NULL) {
   if (trans->tag_number<0) {
      // case of a sub-graph
      struct norm_info* TMP=NULL;
      explorer_sub_automate_normalization_string(automate,automate->initial_states[-(trans->tag_number)],noeud_normalization,
                                        output,&TMP);
      while (TMP!=NULL) {
         // we continue to explore the current automaton
         explorer_automate_normalization_string(automate,trans->state_number,TMP->node,TMP->output);
         struct norm_info* z=TMP;
         TMP=TMP->next;
         free_norm_info(z);
      }
   }
   else {
      // normal transition
      Fst2Tag etiq;
      etiq=automate->tags[trans->tag_number];
      u_strcpy(tmp,output);
      u_strcat(tmp," ");
      if (etiq->output!=NULL && u_strcmp(etiq->output,"")
          && u_strcmp(etiq->output,"<E>") && !only_spaces(etiq->output)) {
         // we append the output if it exists and is not epsilon
         u_strcat(tmp,etiq->output);
      }
      struct normalization_tree_transition* trans_norm;
      trans_norm=get_trans_arbre_normalization_string(etiq->input,noeud_normalization->trans);
      if (trans_norm==NULL) {
         // if the transition does not exist in the tree, we create it
         trans_norm=new_trans_arbre_normalization_string(etiq->input);
         // we also create the destination node
         trans_norm->node=new_normalization_tree();
         trans_norm->next=noeud_normalization->trans;
         noeud_normalization->trans=trans_norm;
      }
      explorer_automate_normalization_string(automate,trans->state_number,trans_norm->node,tmp);
   }
   trans=trans->next;
}
}




//
// this function constructs a token tree from a normalization grammar
// tokens are represented by strings
//
struct normalization_tree* load_normalization_transducer_string(const VersatileEncodingConfig* vec,const char* name) {
struct FST2_free_info fst2_free;
Fst2* automate=load_abstract_fst2(vec,name,0,&fst2_free);
if (automate==NULL) {
   // if the loading of the normalization transducer has failed, we return
   return NULL;
}
struct normalization_tree* root=new_normalization_tree();
unichar a[1];
a[0]='\0';
explorer_automate_normalization_string(automate,automate->initial_states[1],root,a);
free_abstract_Fst2(automate,&fst2_free);
return root;
}

} // namespace unitex


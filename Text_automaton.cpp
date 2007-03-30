 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------
#include "Text_automaton.h"
#include "StringParsing.h"
#include "List_ustring.h"
//---------------------------------------------------------------------------



struct noeud_text_automaton* new_noeud_text_automaton() {
struct noeud_text_automaton* n;
n=(struct noeud_text_automaton*)malloc(sizeof(struct noeud_text_automaton));
n->numero=-1;
n->final=0;
n->controle=0;
n->trans=NULL;
return n;
}


struct trans_text_automaton* new_trans_text_automaton(unichar* s,int indice) {
struct trans_text_automaton* t;
t=(struct trans_text_automaton*)malloc(sizeof(struct trans_text_automaton));
t->chaine=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(s)));
u_strcpy(t->chaine,s);
t->indice_noeud_arrivee=indice;
t->suivant=NULL;
return t;
}


void free_trans_text_automaton(struct trans_text_automaton* t) {
struct trans_text_automaton* tmp;
while (t!=NULL) {
  tmp=t;
  t=t->suivant;
  free(tmp->chaine);
  free(tmp);
}
}


void free_noeud_text_automaton(struct noeud_text_automaton* n) {
if (n==NULL) return;
free_trans_text_automaton(n->trans);
free(n);
}


/**
 * This function returns the number of space tokens that are in the
 * given buffer.
 */
int count_non_space_tokens(int buffer[],int length,int SPACE) {
int n=0;
for (int i=0;i<length;i++) {
   if (buffer[i]!=SPACE) {
      n++;
   }
}
return n;
}
 

struct trans_text_automaton* inserer_trans_text_automaton_(struct trans_text_automaton* trans,unichar* s,int indice) {
if (trans==NULL) {
   return new_trans_text_automaton(s,indice);
}
if (!u_strcmp(s,trans->chaine) && trans->indice_noeud_arrivee==indice) {
   return trans;
}
trans->suivant=inserer_trans_text_automaton_(trans->suivant,s,indice);
return trans;
}



struct trans_text_automaton* inserer_trans_text_automaton(struct trans_text_automaton* trans,
                                                          unichar* token,struct dela_entry* entry,
                                                          int indice) {
unichar tmp[2000];
int i;
u_strcpy(tmp,"{");
u_strcat(tmp,token);
u_strcat(tmp,",");
int l=u_strlen(tmp);
/* We protect the points, if any, in the lemma */
l=l+escape(entry->lemma,&(tmp[l]),P_DOT);
tmp[l++]='.';
/* We protect the + and :, if any, in the grammatical code */
l=l+escape(entry->semantic_codes[0],&(tmp[l]),P_PLUS_COLON);
for (i=1;i<entry->n_semantic_codes;i++) {
   tmp[l++]='+';
   l=l+escape(entry->semantic_codes[i],&(tmp[l]),P_PLUS_COLON);
}
for (i=0;i<entry->n_inflectional_codes;i++) {
   tmp[l++]=':';
   l=l+escape(entry->inflectional_codes[i],&(tmp[l]),P_COLON);
}
tmp[l++]='}';
tmp[l++]='\0';
return inserer_trans_text_automaton_(trans,tmp,indice);
}



void explore_dictionary_tree(int pos,unichar* token,unichar* global,int pos_global,
                             struct string_hash_tree_node* n,struct DELA_tree* tree,
                             struct info* INFO,struct noeud_text_automaton* noeud,int deplacement,
                             int indice_noeud_depart,int* is_not_unknown_token,int token_courant) {
if (token[pos]=='\0') {
   if (deplacement==1 && n->value_index!=-1) {
      // if we are on the first token, then it is not an unknown one
      (*is_not_unknown_token)=1;
   }
   if (n->value_index==-1) return;
   struct dela_entry_list* liste=tree->dela_entries[n->value_index];
   if (liste==NULL) return;
   global[pos_global]='\0';
   while (liste!=NULL) {
      // we create all the transitions in the text automaton
      noeud->trans=inserer_trans_text_automaton(noeud->trans,global,liste->entry,indice_noeud_depart+deplacement);
      liste=liste->next;
   }
   // we try to go on with the next token in the sentence
   if (token_courant<INFO->length_max-1) {
      // but only if we are not at the end of the sentence
      if (INFO->buffer[token_courant]!=INFO->SPACE) {
         // the nodes do not take into acccount spaces
         // so, if we have read a space, we do nothing; else, we can move one node right
         deplacement++;
      }
      token_courant++;
      explore_dictionary_tree(0,INFO->tok->token[INFO->buffer[token_courant]],global,pos_global,tree->inflected_forms->root,
                              tree,INFO,noeud,
                              deplacement,indice_noeud_depart,is_not_unknown_token,token_courant);
   }
   return;
}
struct string_hash_tree_transition* trans;
trans=n->trans;
if (token[pos]==',' || token[pos]=='.' || token[pos]=='/' || 
    token[pos]=='+' || token[pos]=='-' || token[pos]==':') {
   global[pos_global++]='\\';
}
global[pos_global]=token[pos];
while (trans!=NULL) {
  if (is_equal_or_uppercase(trans->letter,token[pos],INFO->alph)) {
     explore_dictionary_tree(pos+1,token,global,pos_global+1,trans->node,tree,INFO,noeud,deplacement,indice_noeud_depart,is_not_unknown_token,token_courant);
  }
  trans=trans->next;
}
}



void compute_best_path_to_node(int n,struct noeud_text_automaton** noeud) {
struct trans_text_automaton* trans;
int N;
N=noeud[n]->numero;
trans=noeud[n]->trans;
while (trans!=NULL) {
  if (trans->chaine[0]!='{' && u_is_letter(trans->chaine[0])) {
     // if the transition is an untagged one
     if ((N+1) < noeud[trans->indice_noeud_arrivee]->numero) {
        // if we have a better way than the existing one,
        // we keep it
        noeud[trans->indice_noeud_arrivee]->numero=N+1;
        // and we mark the destination node as modified
        noeud[trans->indice_noeud_arrivee]->controle=(char)(noeud[trans->indice_noeud_arrivee]->controle | 1);
     }
  } else {
     // if we are following a tagged transition
     if (N < noeud[trans->indice_noeud_arrivee]->numero) {
        // if we have a better way than the existing one,
        // we keep it
        noeud[trans->indice_noeud_arrivee]->numero=N;
        // and we mark the destination node as modified
        noeud[trans->indice_noeud_arrivee]->controle=(char)(noeud[trans->indice_noeud_arrivee]->controle | 1);
     }
  }
  trans=trans->suivant;
}
trans=noeud[n]->trans;
while (trans!=NULL) {
  if (noeud[trans->indice_noeud_arrivee]->controle & 1) {
     // if the node was modified, we reset its control value
     noeud[trans->indice_noeud_arrivee]->controle=(char)(noeud[trans->indice_noeud_arrivee]->controle-1);
     // and we explore it recursively
     compute_best_path_to_node(trans->indice_noeud_arrivee,noeud);
  }
  trans=trans->suivant;
}
}



struct trans_text_automaton* remove_bad_path_transitions(int N,struct trans_text_automaton* trans,struct noeud_text_automaton** noeud) {
if (trans==NULL) return NULL;
if ((trans->chaine[0]!='{' && u_is_letter(trans->chaine[0]) && noeud[trans->indice_noeud_arrivee]->numero < (N+1))
    || (trans->chaine[0]=='{' && noeud[trans->indice_noeud_arrivee]->numero < N )) {
   // if we have an untagged transition to remove
   struct trans_text_automaton* tmp=trans->suivant;
   free(trans->chaine);
   free(trans);
   return remove_bad_path_transitions(N,tmp,noeud);
}
trans->suivant=remove_bad_path_transitions(N,trans->suivant,noeud);
return trans;
}


void check_if_node_is_accessible(int n,struct noeud_text_automaton** noeud) {
if (noeud[n]->controle & 2) {
   // if we have allready marked this node, we return
   return;
}
noeud[n]->controle=(char)(noeud[n]->controle | 2);
struct trans_text_automaton* trans;
trans=noeud[n]->trans;
while (trans!=NULL) {
  check_if_node_is_accessible(trans->indice_noeud_arrivee,noeud);
  trans=trans->suivant;
}
}



int check_if_node_is_coaccessible(int n,struct noeud_text_automaton** noeud) {
if (noeud[n]->controle & 8) {
   // if we have allready visited this node, we return
   return noeud[n]->controle & 4;
}
noeud[n]->controle=(char)(noeud[n]->controle | 8);
if (noeud[n]->final==1) {
   // if we are in the final state
   noeud[n]->controle=(char)(noeud[n]->controle | 4);
   return 1;
}
int is_coaccessible=0;
struct trans_text_automaton* trans;
trans=noeud[n]->trans;
while (trans!=NULL) {
  is_coaccessible=is_coaccessible+check_if_node_is_coaccessible(trans->indice_noeud_arrivee,noeud);
  trans=trans->suivant;
}
if (is_coaccessible) {
   // if there is any possibility to reach the final node, then the
   // current node is co-accessible
   noeud[n]->controle=(char)(noeud[n]->controle | 4);
   return 1;
}
return 0;
}



void keep_best_paths(int nombre_noeuds,struct noeud_text_automaton** noeud) {
int i;
// we initialize the initial node with 0 untagged transition
noeud[0]->numero=0;
// and all other nodes with a large value
for (i=1;i<nombre_noeuds;i++) {
  noeud[i]->numero=1000000;
}
compute_best_path_to_node(0,noeud);
for (i=0;i<nombre_noeuds;i++) {
  noeud[i]->trans=remove_bad_path_transitions(noeud[i]->numero,noeud[i]->trans,noeud);
}
check_if_node_is_accessible(0,noeud);
check_if_node_is_coaccessible(0,noeud);

for (i=0;i<nombre_noeuds;i++) {
  if (!((noeud[i]->controle & 2) && (noeud[i]->controle & 4))) {
     // if a node is not both accessible and co-accessible, we mark
     // it as a removed one
     noeud[i]->final=-1;
  }
}
}


void ajouter_chemin_automate_du_texte(int indice_de_depart,Alphabet* alph,
                                      struct noeud_text_automaton** noeud,
                                      int* nombre_noeuds,unichar* s,int indice_noeud_arrivee) {
struct list_ustring* l=tokenize_normalization_output(s,alph);
if (l==NULL) {
   // if the output to be generated have no interest, we do nothing
   return;
}
struct list_ustring* tmp;
struct trans_text_automaton* TRANS;
int noeud_courant=indice_de_depart;
while (l!=NULL) {
   if (l->next==NULL) {
      // if this is the last transition to create, we make it point to
      // the destination node;
      TRANS=new_trans_text_automaton(l->string,indice_noeud_arrivee);
      TRANS->suivant=noeud[noeud_courant]->trans;
      noeud[noeud_courant]->trans=TRANS;
   }
   else {
      // if this transition is not the last, we must create a new node
      noeud[*nombre_noeuds]=new_noeud_text_automaton();
      TRANS=new_trans_text_automaton(l->string,*nombre_noeuds);
      TRANS->suivant=noeud[noeud_courant]->trans;
      noeud[noeud_courant]->trans=TRANS;
      noeud_courant=*nombre_noeuds;
      (*nombre_noeuds)++;
   }
   tmp=l;
   l=l->next;
   free_list_ustring_element(tmp);
}
}




void explore_normalization_tree(int pos_in_buffer,int token,struct info* INFO,
                                struct noeud_text_automaton** noeud,
                                int* nombre_noeuds,
                                struct normalization_tree* norm_tree_node,
                                int indice_de_depart,int deplacement) {
struct list_ustring* liste_arrivee=norm_tree_node->outputs;
while (liste_arrivee!=NULL) {
   // if there are outputs, we add paths in the text automaton
   ajouter_chemin_automate_du_texte(indice_de_depart,INFO->alph,noeud,
                                    nombre_noeuds,
                                    liste_arrivee->string,
                                    indice_de_depart+deplacement-1);

   liste_arrivee=liste_arrivee->next;
}
// then, we explore the transition from this node
struct normalization_tree_transition* trans;
trans=norm_tree_node->trans;
while (trans!=NULL) {
   if (trans->token==token) {
      // if we have a transition for the current token
      int i=0;
      if (token!=INFO->SPACE) {
         // we must ignore spaces to count the node position in the node array
         i=1;
      }
      explore_normalization_tree(pos_in_buffer+1,INFO->buffer[pos_in_buffer+1],
                                 INFO,noeud,nombre_noeuds,trans->node,
                                 indice_de_depart,deplacement+i);
      // as there can be only one matching transition, we exit the while
      trans=NULL;
   }
   else {
      trans=trans->next;
   }
}
}


/**
 * This function builds the sentence automaton that correspond to the
 * given token buffer. It saves it into the given file.
 */
void build_sentence_automaton(int* buffer,int length,struct text_tokens* tokens,
                               struct DELA_tree* DELA_tree,struct string_hash* tags,
                               Alphabet* alph,FILE* out,int sentence_number,
                               int we_must_clean,
                               struct normalization_tree* norm_tree) {
struct noeud_text_automaton* noeud[2*MAX_TOKENS_IN_SENTENCE];
int i;
/* We add +1 for the final node */
int n_nodes=1+count_non_space_tokens(buffer,length,tokens->SPACE);
for (i=0;i<n_nodes;i++) {
   noeud[i]=new_noeud_text_automaton();
}
noeud[n_nodes-1]->final=1;
struct info INFO;
INFO.tok=tokens;
INFO.buffer=buffer;
INFO.alph=alph;
INFO.SPACE=tokens->SPACE;
INFO.length_max=length;
int noeud_courant=0;
int is_not_unknown_token;
unichar global[2000];

for (i=0;i<length;i++) {
  if (buffer[i]!=tokens->SPACE && buffer[i]!=tokens->SENTENCE_MARKER) {
     // we try to produce every transition from the current token
     is_not_unknown_token=0;
     explore_dictionary_tree(0,tokens->token[buffer[i]],global,0,DELA_tree->inflected_forms->root,DELA_tree,&INFO,noeud[noeud_courant],1,noeud_courant,&is_not_unknown_token,i);
     if (norm_tree!=NULL) {
        // if there is a normalization tree, we explore it
        explore_normalization_tree(i,buffer[i],&INFO,noeud,&n_nodes,norm_tree,noeud_courant,1);
     }
     if (!is_not_unknown_token) {
        // if the token was not matched in the dictionary, we put it as an unknown trans
        noeud[noeud_courant]->trans=inserer_trans_text_automaton_(noeud[noeud_courant]->trans,tokens->token[buffer[i]],noeud_courant+1);
     }
     noeud_courant++;
  }
}
if (we_must_clean) {
   keep_best_paths(n_nodes,noeud);
}

int numero=0;
for (i=0;i<n_nodes;i++) {
   if (noeud[i]->final==0 || noeud[i]->final==1) {
      noeud[i]->numero=numero++;
   }
}
u_fprintf(out,"-%d ",sentence_number);
for (int z=0;z<length;z++) {
   u_fprintf(out,"%S",tokens->token[buffer[z]]);
}
u_fprintf(out,"\n");
for (i=0;i<n_nodes;i++) {
   if (noeud[i]->final==0 || noeud[i]->final==1) {
      if (noeud[i]->final==0) {
         u_fprintf(out,": ");
      } else {
         u_fprintf(out,"t ");
      }
      struct trans_text_automaton* trans;
      trans=noeud[i]->trans;
      while (trans!=NULL) {
         if (noeud[trans->indice_noeud_arrivee]->final==0 || noeud[trans->indice_noeud_arrivee]->final==1) {
            // we only consider the transitions that point on a node that will not be removed
            int indice=get_value_index(trans->chaine,tags);

            // the following line is used to write the tag instead of its number
            //u_strcpy(global,trans->chaine);

            u_fprintf(out,"%d %d ",indice,noeud[trans->indice_noeud_arrivee]->numero);
         }
         trans=trans->suivant;
      }
      u_fprintf(out,"\n");
   }
}
u_fprintf(out,"f \n");

for (i=0;i<n_nodes;i++) {
  free_noeud_text_automaton(noeud[i]);
}
}

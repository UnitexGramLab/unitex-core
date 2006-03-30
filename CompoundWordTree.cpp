 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "LocatePattern.h"
#include "CompoundWordTree.h"
#include "Error.h"
//---------------------------------------------------------------------------


/**
 * Allocates, initializes and returns a compound word tree node.
 */
struct DLC_tree_node* new_DLC_tree_node() {
struct DLC_tree_node* n;
n=(struct DLC_tree_node*)malloc(sizeof(struct DLC_tree_node));
if (n==NULL) fatal_error("Not enough memory in new_DLC_tree_node",1);
n->patterns=NULL;
n->number_of_patterns=0;
n->array_of_patterns=NULL;
n->transitions=NULL;
n->number_of_transitions=0;
n->destination_tokens=NULL;
n->destination_nodes=NULL;
return n;
}


/**
 * Allocates, initializes and returns a compound word tree transition.
 * The default token number is -1.
 */
struct DLC_tree_transition* new_DLC_tree_transition() {
struct DLC_tree_transition* t;
t=(struct DLC_tree_transition*)malloc(sizeof(struct DLC_tree_transition));
if (t==NULL) fatal_error("Not enough memory in new_DLC_tree_transition",1);
t->token=-1;
t->node=NULL;
t->next=NULL;
return t;
}


/**
 * This function initializes the root of the compound word tree and
 * the 'index' array that will be used to access directly to the
 * compound words that begin by a given token. 'number_of_tokens'
 * is supposed to represent the total number of distinct tokens in
 * the text to parse. 'infos' is a structure that contains the
 * root and the index. This value is modified.
 */
void init_DLC_tree(struct DLC_tree_info* DLC_tree,int number_of_tokens) {
DLC_tree->root=new_DLC_tree_node();
DLC_tree->index=(struct DLC_tree_node**)malloc(number_of_tokens*sizeof(struct DLC_tree_node*));
if (DLC_tree->index==NULL) fatal_error("Not enough memory in init_DLC_tree",1);
for (int i=0;i<number_of_tokens;i++) {
	DLC_tree->index[i]=NULL;
}
}


int decouper_chaine_en_tokens(unichar* ch,int t[],Alphabet* alph,struct string_hash* tok) {
int i,c,n_token,j,k;
struct liste_nombres* ptr;
unichar m[400];
k=0;
n_token=0;
if (CHAR_BY_CHAR==THAI) {
   // if we are processing a thai dictionary, we go char by char
   n_token=0;
   i=0;
   m[1]='\0';
   while (ch[i]!='\0') {
      m[0]=ch[i];
      j=get_token_number(m,tok);
      if (j==-1) {
         t[n_token++]=-3;
         t[n_token++]=-5;
      }
      else t[n_token++]=j;
      i++;
   }
   t[n_token]=-1;
   return 1;
}
// if we are not in THAI mode
while ((c=ch[k])!='\0') {
  if (c==' ') {
              j=ESPACE;
              if (j==-1) {
                t[n_token++]=-3;
                t[n_token++]=-5;
              }
              else t[n_token++]=j;
              while ((c=ch[++k])==' ');
            }
    else if (is_letter((unichar)c,alph)) {
              // on est dans le cas d'un mot
              i=0;
              do {
                m[i++]=(unichar)c;
                c=ch[++k];
              }
              while (is_letter((unichar)c,alph));
              m[i]='\0';

              if (n_token>0 && !ALL_CASE_VARIANTS_ARE_ALLOWED) {
                // here we compute no case variant, we only look for the exact
                // matching token
                j=get_token_number(m,tok);
                if (j==-1) {
                  t[n_token++]=-3;
                  t[n_token++]=-5;
                }
                else t[n_token++]=j;
              }
              else {
                // we compute all case variants
                t[n_token++]=-3; // debut de liste de tokens

                ptr=get_token_list_for_sequence(m,alph,tok);
                struct liste_nombres* ptr_copy = ptr; // s.n.

                while (ptr!=NULL) {
                  j=ptr->n;
                  t[n_token++]=j;
                  ptr=ptr->suivant;
                }
                free_liste_nombres(ptr_copy); // s.n.

                t[n_token++]=-5; // fin de liste de tokens
              }
            }
    else {
               m[0]=(unichar)c;
               m[1]='\0';
               k++;
               j=get_token_number(m,tok);
               if (j==-1) {
                  t[n_token++]=-3;
                  t[n_token++]=-5;
               } else t[n_token++]=j;
    }
}
t[n_token]=-1;
return 1;
}



void ajouter_code_a_etat_dlc(struct DLC_tree_node* n,int code) {
struct liste_nombres *l,*precedent;
int stop;

if (n->patterns==NULL) {
  // 1er cas liste vide
  l=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
  l->n=code;
  l->suivant=n->patterns;
  n->patterns=l;
  (n->number_of_patterns)++;
  return;
}
if (n->patterns->n==code)
  // 2eme cas: 1er element=code
  return;
if (n->patterns->n>code) {
  // 3eme cas: on doit inserer en tete de liste
  l=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
  l->n=code;
  l->suivant=n->patterns;
  n->patterns=l;
  (n->number_of_patterns)++;
  return;
}
// cas general
precedent=n->patterns;
stop=0;
while (!stop && precedent->suivant!=NULL) {
  if (precedent->suivant->n==code) return;
  else if (precedent->suivant->n<code) precedent=precedent->suivant;
  else stop=1;
}
l=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
l->n=code;
l->suivant=precedent->suivant;
precedent->suivant=l;
(n->number_of_patterns)++;
return;
}










struct DLC_tree_node* get_trans_dlc_sans_creer(struct DLC_tree_node* n,int t) {
struct DLC_tree_transition* l;
l=n->transitions;
while (l!=NULL) {
  if (l->token==t) return l->node;
  l=l->next;
}
return NULL;
}



struct DLC_tree_node* get_trans_dlc(struct DLC_tree_node* n,int t) {
struct DLC_tree_transition* l;
struct DLC_tree_transition* precedent;
int stop;
if (n->transitions==NULL) {
  // 1er cas liste vide
  l=new_DLC_tree_transition();
  l->token=t;
  l->next=n->transitions;
  l->node=new_DLC_tree_node();
  n->transitions=l;
  (n->number_of_transitions)++;
  return l->node;
}
if (n->transitions->token==t)
  // 2eme cas: 1er element=code
  return n->transitions->node;
if (n->transitions->token>t) {
  // 3eme cas: on doit inserer en tete de liste
  l=new_DLC_tree_transition();
  l->token=t;
  l->next=n->transitions;
  l->node=new_DLC_tree_node();
  n->transitions=l;
  (n->number_of_transitions)++;
  return l->node;
}
// cas general
precedent=n->transitions;
stop=0;
while (!stop && precedent->next!=NULL) {
  if (precedent->next->token==t) return precedent->next->node;
  else if (precedent->next->token<t) precedent=precedent->next;
  else stop=1;
}
l=new_DLC_tree_transition();
l->token=t;
l->node=new_DLC_tree_node();
l->next=precedent->next;
precedent->next=l;
(n->number_of_transitions)++;
return l->node;
}



void ajouter_dlc(int* token_dlc,int i,struct DLC_tree_node* n,int code,
				struct DLC_tree_info* DLC_tree) {
struct DLC_tree_node* ptr;
int pos,premier_token;
if (token_dlc[i]==-1) {
  // on est au bout de la branche
  ajouter_code_a_etat_dlc(n,code);
  return;
}
premier_token=(i==0);
if (token_dlc[i]!=-3) {
  ptr=get_trans_dlc(n,token_dlc[i]);
  if (premier_token) {
     DLC_tree->index[token_dlc[i]]=ptr;
  }
  ajouter_dlc(token_dlc,i+1,ptr,code,DLC_tree);
  return;
}
pos=i;
do pos++;
while (token_dlc[pos]!=-5);
pos++;
// pos est l'endroit auquel on cherchera le token suivant
i++;
while (token_dlc[i]!=-5)  {
  ptr=get_trans_dlc(n,token_dlc[i]);
  if (premier_token) {
     DLC_tree->index[token_dlc[i]]=ptr;
  }
  ajouter_dlc(token_dlc,pos,ptr,code,DLC_tree);
  i++;
}
}



void ajouter_a_dlc_sans_code(unichar* m,Alphabet* alph,struct string_hash* tok,
							struct DLC_tree_info* DLC_tree) {
int token_dlc_temp[MAX_TOKEN_IN_A_COMPOUND_WORD];
decouper_chaine_en_tokens(m,token_dlc_temp,alph,tok);
ajouter_dlc(token_dlc_temp,0,DLC_tree->root,-555,DLC_tree);
}



void ajouter_a_dlc_avec_code(unichar* m,int code,Alphabet* alph,struct string_hash* tok,
							struct DLC_tree_info* DLC_tree) {
int token_dlc_temp[MAX_TOKEN_IN_A_COMPOUND_WORD];
decouper_chaine_en_tokens(m,token_dlc_temp,alph,tok);
ajouter_dlc(token_dlc_temp,0,DLC_tree->root,code,DLC_tree);
}



int inserer_code_dans_etat_dlc(struct DLC_tree_node* n,int code,int code2) {
if (appartient_a_liste(code,n->patterns)) {
  ajouter_code_a_etat_dlc(n,code2);
  return 1;
}
return 0;
}



int inserer_dans_dlc(int token_dlc[],int i,struct DLC_tree_node* n,int code,int code2) {
struct DLC_tree_node* ptr;
int pos,resultat;
resultat=0;
if (token_dlc[i]==-1) {
  // on est au bout de la branche
  return inserer_code_dans_etat_dlc(n,code,code2);
}
if (token_dlc[i]!=-3) {
  ptr=get_trans_dlc_sans_creer(n,token_dlc[i]);
  if (ptr!=NULL) return inserer_dans_dlc(token_dlc,i+1,ptr,code,code2);
  else return 0;
}
pos=i;
do pos++;
while (token_dlc[pos]!=-5);
pos++;
// pos est l'endroit auquel on cherchera le token suivant
i++;
while (token_dlc[i]!=-5)  {
  ptr=get_trans_dlc_sans_creer(n,token_dlc[i]);
  if (ptr!=NULL) resultat=resultat+inserer_dans_dlc(token_dlc,pos,ptr,code,code2);
  i++;
}
return resultat;
}



int remplacer_dans_dlc(unichar* m,int code,int code2,Alphabet* alph,struct string_hash* tok,
						struct DLC_tree_info* infos) {
int token_dlc[MAX_TOKEN_IN_A_COMPOUND_WORD];
decouper_chaine_en_tokens(m,token_dlc,alph,tok);
return inserer_dans_dlc(token_dlc,0,infos->root,code,code2);
}

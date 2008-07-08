 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Text_tokens.h"
//---------------------------------------------------------------------------

int NUMBER_OF_TEXT_TOKENS=0;

struct text_tokens* new_text_tokens() {
struct text_tokens* tmp;
tmp=(struct text_tokens*)malloc(sizeof(struct text_tokens));
tmp->N=0;
tmp->SENTENCE_MARKER=-1;
tmp->token=NULL;
return tmp;
}


struct text_tokens* load_text_tokens(char* nom) {
FILE* f;
f=u_fopen(nom,U_READ);
if (f==NULL) {
   return NULL;
}
struct text_tokens* res=new_text_tokens();
u_fscanf(f,"%d\n",&(res->N));
res->token=(unichar**)malloc((res->N)*sizeof(unichar*));
unichar tmp[1000];
res->SENTENCE_MARKER=-1;
int i=0;
while (EOF!=u_fgets(tmp,f)) {
  res->token[i]=u_strdup(tmp);
  if (!u_strcmp(tmp,"{S}")) {
     res->SENTENCE_MARKER=i;
  } else if (!u_strcmp(tmp," ")) {
            (res->SPACE)=i;
         }
    else if (!u_strcmp(tmp,"{STOP}")) {
            (res->STOP_MARKER)=i;
         }
  i++;
}
u_fclose(f);
return res;
}



struct string_hash* load_text_tokens_hash(char* nom) {
FILE* f;
f=u_fopen(nom,U_READ);
if (f==NULL) {
   return NULL;
}
u_fscanf(f,"%d\n",&NUMBER_OF_TEXT_TOKENS);
NUMBER_OF_TEXT_TOKENS=NUMBER_OF_TEXT_TOKENS+100000; // the +100000 is used to prevent the addition
                                            // of tokens while locate preprocessing
struct string_hash* res;
res=new_string_hash(NUMBER_OF_TEXT_TOKENS);
unichar tmp[1000];
while (EOF!=u_fgets(tmp,f)) {
   get_value_index(tmp,res);
}
u_fclose(f);
return res;
}



struct string_hash* load_text_tokens_hash(char* nom,int *SENTENCE_MARKER,
                                          int* STOP_MARKER) {
FILE* f;
f=u_fopen(nom,U_READ);
if (f==NULL) {
   return NULL;
}
(*SENTENCE_MARKER)=-1;
u_fscanf(f,"%d\n",&NUMBER_OF_TEXT_TOKENS);
NUMBER_OF_TEXT_TOKENS=NUMBER_OF_TEXT_TOKENS+100000; // the +100000 is used to prevent the addition
                                            // of tokens while locate preprocessing
struct string_hash* res;
res=new_string_hash(NUMBER_OF_TEXT_TOKENS);
unichar tmp[1000];
int x;
while (EOF!=u_fgets(tmp,f)) {
   x=get_value_index(tmp,res);
   if (!u_strcmp(tmp,"{S}")) {
      (*SENTENCE_MARKER)=x;
   }
   else if (!u_strcmp(tmp,"{STOP}")) {
      (*STOP_MARKER)=x;
   }
}
u_fclose(f);
return res;
}




void free_text_tokens(struct text_tokens* tok) {
for (int i=0;i<tok->N;i++) {
   free(tok->token[i]);
}
free(tok->token);
free(tok);
}




void explorer_token_tree(int pos,unichar* sequence,Alphabet* alph,struct string_hash_tree_node* n,struct list_int** l) {
if (sequence[pos]=='\0') {
   // if we are at the end of the sequence
   if (n->value_index!=-1) {
      // if the sequence is a text token, we add its number to the list
      (*l)=sorted_insert(n->value_index,*l);
   }
   return;
}
struct string_hash_tree_transition* trans=n->trans;
while (trans!=NULL) {
  if (is_equal_or_uppercase(sequence[pos],trans->letter,alph)) {
     // if we can follow the transition
     explorer_token_tree(pos+1,sequence,alph,trans->node,l);
  }
  trans=trans->next;
}
}



struct list_int* get_token_list_for_sequence(unichar* sequence,Alphabet* alph,
                                                  struct string_hash* hash) {
struct list_int* l=NULL;
explorer_token_tree(0,sequence,alph,hash->root,&l);
return l;
}



int get_token_number(unichar* s,struct text_tokens* tok) {
for (int i=0;i<tok->N;i++) {
    if (!u_strcmp(tok->token[i],s)) return i;
}
return -1;
}

unichar  *get_text_token(int token_number ,struct text_tokens* tok) {
           return(tok->token[token_number]);
}


//
// return 1 if s is a digit sequence, 0 else
//
int is_a_digit_token(unichar* s) {
int i=0;
while (s[i]!='\0') {
   if (s[i]<'0' || s[i]>'9') {
      return 0;
   }
   i++;
}
return 1;
}


//
// Scans text tokens to extract semantic codes contained in tags like {le,.DET:ms}
//
void extract_semantic_codes_from_tokens(struct string_hash* tok,
                                        struct string_hash* semantic_codes) {
for (int i=0;i<tok->size;i++) {
    if (tok->value[i][0]=='{' && u_strcmp(tok->value[i],"{S}")
                            && u_strcmp(tok->value[i],"{STOP}")) {
       struct dela_entry* temp=tokenize_tag_token(tok->value[i]);
       for (int j=0;j<temp->n_semantic_codes;j++) {
          get_value_index(temp->semantic_codes[j],semantic_codes);
       }
       free_dela_entry(temp);
    }
}
}



/**
 * Scans text tokens to extract semantic codes contained in .inf 
 * of morphological mode dictionaries.
 */
void extract_semantic_codes_from_morpho_dics(struct INF_codes** array,int N,
                                        struct string_hash* semantic_codes) {
unichar line[2048];
/* Foo line for generating a dela_entry */
u_strcpy(line,"a,a");
for (int i=0;i<N;i++) {
   if (array[i]==NULL) {
      continue;
   }
   for (int j=0;j<array[i]->N;j++) {
      struct list_ustring* codes=array[i]->codes[j];
      while (codes!=NULL) {
         u_strcpy(line+3,codes->string);
         //u_printf("code=_%S_\n",codes->string);
         struct dela_entry* entry=tokenize_DELAF_line(line);
         for (int k=0;k<entry->n_semantic_codes;k++) {
            get_value_index(entry->semantic_codes[k],semantic_codes);
         }
         free_dela_entry(entry);
         codes=codes->next;
      }
   }
}
}

//---------------------------------------------------------------------------


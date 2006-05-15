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
#include <time.h>
#include "Dico_application.h"
#include "Error.h"
//---------------------------------------------------------------------------

#define DEBUG 0

struct word_struct_array* word_array;
FILE* DLF;
FILE* DLC;
FILE* ERR;
Alphabet* ALPH;
unsigned char* BIN;
struct INF_codes* INF;
FILE* TEXT;
struct text_tokens* TOK;
int buffer[BUFFER_SIZE];
int LENGTH_FOR_DICO;
int current_start_pos;
struct token_tree_node* token_tree_root;
struct tct_hash* tct_h;  
unsigned char* part_of_a_word;
unsigned char* has_been_processed;
int COMPOUND_WORDS=0;
int* n_occur=NULL;
int WORDS_HAVE_BEEN_COUNTED=0;
int ERRORS=0;
int SIMPLE_WORDS=0;
int CURRENT_BLOCK=1;



void set_part_of_a_word(int n,int priority) {
part_of_a_word[n/4]=(unsigned char)(part_of_a_word[n/4] | (priority<<(2*(n%4))));
}


int is_part_of_a_word(int n) {
return (part_of_a_word[n/4] & ((3<<(2*(n%4)))))>>(2*(n%4));
}


void set_has_been_processed(int n,int priority) {
has_been_processed[n/4]=(unsigned char)(has_been_processed[n/4] | (priority<<(2*(n%4))));
}


int token_has_been_processed(int n) {
return (has_been_processed[n/4] & ((3<<(2*(n%4)))))>>(2*(n%4));
}


struct word_struct_array* create_word_struct_array(int n) {
struct word_struct_array* res;
res=(struct word_struct_array*)malloc(sizeof(struct word_struct_array));
res->tab=(struct word_struct**)malloc(sizeof(struct word_struct*)*n);
for (int i=0;i<n;i++) {
  res->tab[i]=NULL;
}
res->N=n;
return res;
}


void liberer_offset_list(struct offset_list* l) {
struct offset_list* tmp;
while (l!=NULL) {
  free(l->contenu_provisoire);
  tmp=l;
  l=l->suivant;
  free(tmp);
}
}


void liberer_word_transition(struct word_transition* t) {
struct word_transition* tmp;
while (t!=NULL) {
  liberer_word_struct(t->arr);
  tmp=t;
  t=t->suivant;
  free(tmp);
}
}


void liberer_word_struct(struct word_struct* w) {
if (w==NULL) return;
liberer_word_transition(w->trans);
liberer_offset_list(w->list);
free(w);
}


void free_word_struct_array(struct word_struct_array* w) {
for (int i=0;i<w->N;i++) {
  liberer_word_struct(w->tab[i]);
}
free(w);
}


struct offset_list* inserer_offset_si_absent(int n,struct offset_list* l,unichar* contenu) {
if (l==NULL) {
  l=(struct offset_list*)malloc(sizeof(struct offset_list));
  l->offset=n;
  l->contenu_provisoire=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(contenu)));
  u_strcpy(l->contenu_provisoire,contenu);
  l->suivant=NULL;
  return l;
}
if (l->offset==n) return l;
l->suivant=inserer_offset_si_absent(n,l->suivant,contenu);
return l;
}


struct word_struct* new_word_struct() {
struct word_struct* res;
res=(struct word_struct*)malloc(sizeof(struct word_struct));
res->list=NULL;
res->trans=NULL;
return res;
}


struct word_transition* new_word_transition() {
struct word_transition* res;
res=(struct word_transition*)malloc(sizeof(struct word_transition));
res->arr=NULL;
res->suivant=NULL;
return res;
}


void ajouter_offset_pour_token(int token_number,int offset,unichar* contenu) {
if (word_array->tab[token_number]==NULL) {
  word_array->tab[token_number]=new_word_struct();
}
word_array->tab[token_number]->list=inserer_offset_si_absent(offset,word_array->tab[token_number]->list,contenu);
}



void explorer_bin_simples_tokens(int pos,unichar* contenu,unichar* entry,
                                 int string_pos,int token_number,int priority) {
int n_transitions;
int ref;
n_transitions=((unsigned char)BIN[pos])*256+(unsigned char)BIN[pos+1];
pos=pos+2;
if (contenu[string_pos]=='\0') {
   // if we are at the end of the string
   entry[string_pos]='\0';
   ajouter_offset_pour_token(token_number,pos-2,entry);
   if (!(n_transitions & 32768)) {
      int p=token_has_been_processed(token_number);
      if (p==0 || p==priority) {
         // we save the token only if it has the good priority
         set_part_of_a_word(token_number,priority);
         set_has_been_processed(token_number,priority);
         ref=((unsigned char)BIN[pos])*256*256+((unsigned char)BIN[pos+1])*256+(unsigned char)BIN[pos+2];
         struct word_list* tmp=INF->tab[ref];
         while (tmp!=NULL) {
            unichar res[1000];
            uncompress_entry(entry,tmp->word,res);
            u_fprints(res,DLF);
            u_fprints_char("\n",DLF);
            tmp=tmp->next;
         }
      }
   }
   return;
}
if ((n_transitions & 32768)) {
   // if we are in a normal node, we remove the control bit to
   // have the good number of transitions
   n_transitions=n_transitions-32768;
} else {
  // if we are in a final node, we must jump after the reference to the INF line number
  pos=pos+3;
}
for (int i=0;i<n_transitions;i++) {
  unichar c=(unichar)(((unsigned char)BIN[pos])*256+(unsigned char)BIN[pos+1]);
  pos=pos+2;
  int adr=((unsigned char)BIN[pos])*256*256+((unsigned char)BIN[pos+1])*256+(unsigned char)BIN[pos+2];
  pos=pos+3;
  if (is_equal_or_uppercase(c,contenu[string_pos],ALPH)) {
     // we explore the rest of the dictionary only
     // if the dico char is compatible with the token char
     entry[string_pos]=c;
     explorer_bin_simples_tokens(adr,contenu,entry,string_pos+1,token_number,priority);
  }
}
}



void traiter_simples_tokens(int priority) {
unichar entry[1000];
for (int i=0;i<TOK->N;i++) {
   explorer_bin_simples_tokens(4,TOK->token[i],entry,0,i,priority);
}
}



//
// looks for a word transition in a sorted list
//
struct word_transition* get_word_transition(struct word_transition* t,int token) {
while (t!=NULL && t->token_number<=token) {
  if (t->token_number==token) {
     return t;
  }
  t=t->suivant;
}
return NULL;
}


//
// gets a word transition from a sorted list. If neccessary, the word transition is created and
// inserted in the list. Returns the list. The result element is stored in the res parameter
//
struct word_transition* insert_word_transition(struct word_transition* list,struct word_transition** res,
                                               int token) {
if (list==NULL) {
   (*res)=new_word_transition();
   (*res)->token_number=token;
   return (*res);
}
if (list->token_number==token) {
   (*res)=list;
   return list;
}
if (list->token_number<token) {
   list->suivant=insert_word_transition(list->suivant,res,token);
   return list;
}
// if we are here, we must insert a new word transition in the list
(*res)=new_word_transition();
(*res)->token_number=token;
(*res)->suivant=list;
return (*res);
}


void explorer_bin_composes_tokens(int pos,unichar* contenu,unichar* entry,int string_pos,
                                  int entry_pos,struct word_struct *w,int deplacement,
                                  int* TOKEN_TAB,int pos_token_tab,int priority) {
int n_transitions;
int ref;
//printf("\ncontenu:\n");
//u_prints(contenu);
//printf("\n explorer_bin_composes_tokens: entry_pos: %d", entry_pos);
//u_prints(entry);


n_transitions=((unsigned char)BIN[pos])*256+(unsigned char)BIN[pos+1];
pos=pos+2;
if (contenu[string_pos]=='\0') {
   // if we are at the end of the current token
   struct word_transition* trans;
   w->trans=insert_word_transition(w->trans,&trans,buffer[current_start_pos+deplacement]);
   if (trans->arr==NULL) {
      // if node does not exist, we create it
      trans->arr=new_word_struct();
   }
   entry[entry_pos]='\0';
   TOKEN_TAB[pos_token_tab++]=trans->token_number;
   // we add this offset to the node list
   trans->arr->list=inserer_offset_si_absent(pos-2,trans->arr->list,entry);
   if (!(n_transitions & 32768)) {
      // if this node is final
       TOKEN_TAB[pos_token_tab]=-1;
	   //int w=was_allready_in_token_tree(TOKEN_TAB,0,token_tree_root,priority);
 
       int w = was_allready_in_tct_hash(TOKEN_TAB,tct_h,priority);
 
	  if (w==0 || w==priority) {
         for (int k=current_start_pos;k<=current_start_pos+deplacement;k++) {
            // if we have matched a compound, then all its part all not unknown words
            set_part_of_a_word(buffer[k],priority);
         }
         ref=((unsigned char)BIN[pos])*256*256+((unsigned char)BIN[pos+1])*256+(unsigned char)BIN[pos+2];
         unichar res[1000];
         struct word_list* tmp=INF->tab[ref];
         COMPOUND_WORDS++;
         while (tmp!=NULL) {
            uncompress_entry(entry,tmp->word,res);

            u_fprints(res,DLC);
            u_fprints_char("\n",DLC);
            tmp=tmp->next;
         }
      }
   }
   deplacement++;
   contenu=TOK->token[buffer[current_start_pos+deplacement]];
   string_pos=0;
   w=trans->arr;
}

if ((n_transitions & 32768)) {
   // if we are in a normal node, we remove the control bit to
   // have the good number of transitions
   n_transitions=n_transitions-32768;
} else {
  // if we are in a final node, we must jump after the reference to the INF line number
  pos=pos+3;
}
for (int i=0;i<n_transitions;i++) {
  unichar c=(unichar)(((unsigned char)BIN[pos])*256+(unsigned char)BIN[pos+1]);
  pos=pos+2;
  int adr=((unsigned char)BIN[pos])*256*256+((unsigned char)BIN[pos+1])*256+(unsigned char)BIN[pos+2];
  pos=pos+3;
  if (is_equal_or_uppercase(c,contenu[string_pos],ALPH)) {
     // we explore the rest of the dictionary only
     // if the dico char is compatible with the token char
     entry[entry_pos]=c;

     explorer_bin_composes_tokens(adr,contenu,entry,string_pos+1,entry_pos+1,w,deplacement,TOKEN_TAB,pos_token_tab,priority);
  }
  //save_lines_tct_hash(tct_h);
}
}



void block_change() {
int i;
printf("Block %d...              \r",++CURRENT_BLOCK);
for (i=current_start_pos;i<BUFFER_SIZE;i++) {
  // first, we copy the end of the buffer at the beginning
  buffer[i-current_start_pos]=buffer[i];
}
int N=BUFFER_SIZE-current_start_pos;
int l=fread(buffer+N,sizeof(int),current_start_pos,TEXT);
current_start_pos=0;
LENGTH_FOR_DICO=N+l;
}



void traiter_composes_tokens(int priority) {
unichar entry[1000];
int TOKEN_TAB[1000];
struct word_struct* w;
fseek(TEXT,0,SEEK_SET);
LENGTH_FOR_DICO=fread(buffer,sizeof(int),BUFFER_SIZE,TEXT);
current_start_pos=0;
printf("First block...              \r");
CURRENT_BLOCK=1;
while (current_start_pos<LENGTH_FOR_DICO) {
  if (LENGTH_FOR_DICO==BUFFER_SIZE && current_start_pos>(LENGTH_FOR_DICO-200)) {
     // if we must change of block
     block_change();
  }
  int token_number=buffer[current_start_pos];
  if (!WORDS_HAVE_BEEN_COUNTED) n_occur[token_number]++;
  w=word_array->tab[token_number];
  if (w!=NULL) {
     struct word_transition* trans;
     int fini=0;
     int pos_depart=1;
     int Z=0;

     while (!fini) {
        trans=get_word_transition(w->trans,buffer[current_start_pos+pos_depart]);
        if (trans==NULL) {
           fini=1;
        }
        else {
          w=trans->arr;
          TOKEN_TAB[Z++]=buffer[current_start_pos+pos_depart];
          TOKEN_TAB[Z]=-1;
          // we count the compound words allready found
          struct offset_list* l=w->list;
          while (l!=NULL) {
             if (!((unsigned char)BIN[l->offset] & 128)) {
                // if the node is final
				 //int W= was_allready_in_token_tree(TOKEN_TAB,0,token_tree_root,priority);

				 int W = was_allready_in_tct_hash(TOKEN_TAB,tct_h,priority);

				if (priority==W || W==0) {
                   // and if the priority is OK, then we increase the compound word counter
                   COMPOUND_WORDS++;
                }
             }
             l=l->suivant;
          }
          pos_depart++;
        }
     }
     struct offset_list* l;
     l=w->list;
     while (l!=NULL) {
        u_strcpy(entry,l->contenu_provisoire);
        explorer_bin_composes_tokens(l->offset,TOK->token[buffer[current_start_pos+pos_depart]],entry,0,u_strlen(entry),w,pos_depart,TOKEN_TAB,0,priority);
        l=l->suivant;

     }
  }
  current_start_pos++;
}
WORDS_HAVE_BEEN_COUNTED=1;
printf("\n");
}



void sauver_mots_inconnus() {
for (int i=0;i<TOK->N;i++) {
  if (is_letter(TOK->token[i][0],ALPH)) {
     // to be an unknown word, a token must be a word
     if (!is_part_of_a_word(i)) {
         ERRORS=ERRORS+n_occur[i];
         u_fprints(TOK->token[i],ERR);
         u_fprints_char("\n",ERR);
     }
     else {
         if (token_has_been_processed(i)) SIMPLE_WORDS=SIMPLE_WORDS+n_occur[i];
     }
  }
}
}



void init_dico_application(struct text_tokens* tok,FILE* dlf,FILE* dlc,FILE* err,
                           FILE* text,Alphabet* alph) {
part_of_a_word=(unsigned char*)malloc(sizeof(unsigned char)*(((tok->N)/4)+1));
has_been_processed=(unsigned char*)malloc(sizeof(unsigned char)*(((tok->N)/4)+1));
n_occur=(int*)malloc(tok->N*sizeof(int));
token_tree_root=new_token_tree_node();
tct_h =new_tct_hash(TCT_HASH_SIZE,TCT_HASH_BLOCK_SIZE);
if (part_of_a_word==NULL || has_been_processed==NULL || n_occur==NULL) {
   fatal_error("Not enough memory in init_dico_application\n");
}
for (int i=0;i<=(tok->N)/4;i++) {
   part_of_a_word[i]=0;
   has_been_processed[i]=0;
}
for (int j=0;j<tok->N;j++) {
   n_occur[j]=0;
}
DLF=dlf;
DLC=dlc;
ERR=err;
TEXT=text;
TOK=tok;
ALPH=alph;
}



void free_dico_application() {
free(part_of_a_word);
free(has_been_processed);
free_token_tree_node(token_tree_root);
free(n_occur);
}



void dico_application(unsigned char* bin,struct INF_codes* inf,
                             FILE* text,struct text_tokens* tok,Alphabet* alph,
                             FILE* dlf,FILE* dlc,FILE* err,int priority) {
INF=inf;
BIN=bin;
word_array=create_word_struct_array(tok->N);
printf("Looking for simple words...\n");
traiter_simples_tokens(priority);
printf("Looking for compound words...\n");
clock_t startTime = clock();



traiter_composes_tokens(priority);

// ... perform time-consuming task ...

clock_t endTime = clock();

double  elapsedTime = (double) (endTime - startTime);
if (DEBUG) printf("%2.8f seconds\n", elapsedTime);

free_word_struct_array(word_array);
}



// Added by Alexis for FST in Dico
void assign_text_DICO(FILE* dlf,FILE* dlc,FILE* err) {
DLF=dlf;
DLC=dlc;
ERR=err;
}

// Added by Alexis for FST in Dico
void free_text_DICO() {
DLF=NULL;
DLC=NULL;
ERR=NULL;
}





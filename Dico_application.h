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
#ifndef Dico_applicationH
#define Dico_applicationH
//---------------------------------------------------------------------------
#include "unicode.h"
#include "Alphabet.h"
#include "Text_tokens.h"
#include "DELA.h"
#include "Token_tree.h"
#include "Table_complex_token_hash.h"


struct offset_list {
  int offset;
  unichar* contenu_provisoire;
  struct offset_list* suivant;
};

struct word_struct {
  struct offset_list* list;
  struct word_transition* trans;
};


struct word_transition {
  int token_number;
  struct word_struct* arr;
  struct word_transition* suivant;
};


struct word_struct_array {
  struct word_struct** tab;
  int N;
};



#define BUFFER_SIZE 200000

extern struct word_struct_array* word_array;
extern FILE* DLF;
extern FILE* DLC;
extern FILE* ERR;
extern Alphabet* ALPH;
extern unsigned char* BIN;
extern struct INF_codes* INF;
extern FILE* TEXT;
extern struct text_tokens* TOK;
extern int buffer[BUFFER_SIZE];
extern struct token_tree_node* token_tree_root;
extern unsigned char* part_of_a_word;
extern unsigned char* has_been_processed;
extern int COMPOUND_WORDS;
extern int* n_occur;
extern int WORDS_HAVE_BEEN_COUNTED;
extern int ERRORS;
extern int SIMPLE_WORDS;
extern int CURRENT_BLOCK;

extern struct tct_hash* tct_h;				// *** Alexis to access from Dico (for the FST)

void liberer_word_struct(struct word_struct*);
void dico_application(unsigned char*,struct INF_codes*,
                             FILE*,struct text_tokens*,Alphabet*,
                             FILE*,FILE*,FILE*,int);
void init_dico_application(struct text_tokens*,FILE* dlf,FILE* dlc,FILE* err,FILE* text,Alphabet* alph);
void free_dico_application();


void sauver_mots_inconnus();

// Added by Alexis Neme: FST Functionality of Dico

//  coumputing the  coumpounds forms
void set_part_of_a_word(int n,int priority);
int is_part_of_a_word(int n) ;
void set_has_been_processed(int n,int priority) ;
int token_has_been_processed(int n) ;


// Alexis - management of the Global Variable file DLF,DLC, ERR
void assign_text_DICO (FILE* dlf,FILE* dlc,FILE* err) ;
void free_text_DICO () ;

#endif

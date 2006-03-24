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
#ifndef ConcordanceH
#define ConcordanceH
//---------------------------------------------------------------------------

#include "unicode.h"
#include "Matches.h"
#include "Text_tokens.h"


#define TEXT_ORDER 0
#define LEFT_CENTER 1
#define LEFT_RIGHT 2
#define CENTER_LEFT 3
#define CENTER_RIGHT 4
#define RIGHT_LEFT 5
#define RIGHT_CENTER 6
#define BUFFER_SIZE 1000000
#define MAX_CONTEXT_IN_UNITS 3000
#define MAX_HTML_CONTEXT 10000

//#define OCCIDENTAL 0
//#define THAI 1

#define HTML_ 0
#define TEXT_ 1
#define GLOSSANET_ 2


extern int CHAR_BY_CHAR;
extern int sort_mode;
extern int token_length[1000000];
extern int buffer[BUFFER_SIZE];
extern int BUFFER_LENGTH;
extern int N_UNITS_ALLREADY_READ;
extern int open_bracket;
extern int close_bracket;
extern int origine_courante_char;
extern int phrase_courante;


void create_concordance(FILE*,FILE*,struct text_tokens*,int,int,int,char*,char*,
                        char*,char*,char*,int,int*,char*);
void create_new_text_file(FILE*,FILE*,struct text_tokens*,char*,int,int*);
void compute_token_length(struct text_tokens*);
void write_HTML_header(FILE*,int,char*,char*);
void write_HTML_end(FILE*);
int create_raw_text_concordance(FILE*,FILE*,FILE*,struct text_tokens*,int,int,int,int,int*);
void reverse_initial_vowels_thai(unichar*);
int get_decalage(int,int*,int);

#endif

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
#ifndef LocatePatternH
#define LocatePatternH
//---------------------------------------------------------------------------
#include "unicode.h"
#include "String_hash.h"
#include "AutomateFst2.h"
#include "Text_tokens.h"
#include "Liste_nombres.h"
#include "Alphabet.h"
#include "Grammatical_codes.h"
#include "Loading_dic.h"
#include "Fst2_tags_optimization.h"
#include "DLC_optimization.h"
#include "Optimized_fst2.h"
#include "Pattern_transitions.h"
#include "Text_parsing.h"
#include "TransductionVariables.h"
#include "LocateConstants.h"



/* $CD$ begin */
#include "GF_lib.h"
/* $CD$ end   */


#define TAILLE_MOT 10000

#define WORD_BY_WORD_TOKENIZATION 0
#define CHAR_BY_CHAR_TOKENIZATION 1


extern unsigned char* index_controle;
extern unsigned char** index_code_gramm;
extern int pattern_compose_courant;
extern struct noeud_code_gramm *racine_code_gramm;
extern int ESPACE;
extern struct liste_nombres* tag_token_list;

/* $CD$ begin */
#ifdef TRE_WCHAR
extern MasterGF_T* masterGF;
extern IndexGF_T*  indexGF;
#endif
/* $CD$ end   */

int locate_pattern(char*,char*,char*,char*,char*,char*,char*,int,int,char*,int);

void numerote_tags(Automate_fst2*,struct string_hash*,int*,struct string_hash*,Alphabet*,int*,int*,int*,int);
void decouper_entre_angles(unichar*,unichar*,unichar*,unichar*,struct string_hash*,Alphabet*);
unsigned char get_controle(unichar*,Alphabet*,struct string_hash*,int);
void compute_token_controls(struct string_hash*,Alphabet*,char*,int);

#endif

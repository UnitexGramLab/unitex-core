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
#include "Fst2.h"
#include "Text_tokens.h"
#include "List_int.h"
#include "Alphabet.h"
#include "Pattern.h"
#include "Loading_dic.h"
#include "Fst2_tags_optimization.h"
#include "OptimizedFst2.h"
#include "Pattern_transitions.h"
#include "Text_parsing.h"
#include "TransductionVariables.h"
#include "LocateConstants.h"
#include "BitArray.h"
/* $CD$ begin */
#include "MorphologicalFilters.h"
/* $CD$ end   */


#define TAILLE_MOT 10000

#define WORD_BY_WORD_TOKENIZATION 0
#define CHAR_BY_CHAR_TOKENIZATION 1

/**
 * This structure is used to store all the information needed
 * during the locate operations
 */
struct locate_parameters {
   /**
    * This array is used to associate a control byte to each token.
    * These bytes will be used to know if a token can be matched by
    * <MOT>, <DIC>, <MIN>, <MAJ>, etc. All the bit masks to be used
    * are defined in LocateConstants.h with names like XXX_TOKEN_BIT_MASK
    */
   unsigned char* token_control;
   
   /**
    * This array is used to know the patterns that can match tokens. If the
    * token x is matched by no pattern, then matching_patterns[x]=NULL; Otherwise,
    * matching_patterns[x] will be a bit array so that matching_patterns[x]->array[y]
    * will be 1 if the pattern y can match the token x and 0 otherwise.
    */
   struct bit_array** matching_patterns;
   
   /* This field is used to know the current compound pattern number */
   int current_compound_pattern;
   
   /* This field designates a tree that contains all the patterns defined
    * in the grammar tags (ex: <machine.N+Conc>, <V-z2:Kms>, etc). It is
    * used to associate a unique number to each pattern */
   struct pattern_node* pattern_tree_root;
   
   /* Number of the space token in the text */
   int SPACE;

   /* Number of the sentence delimiter {S} in the text */
   int SENTENCE;

   /* Number of the stop token {STOP} in the text */
   int STOP;
   
   /* Numbers of the tokens that are tags like {soon,.ADV} */
   struct list_int* tag_token_list;
   
   #ifdef TRE_WCHAR
   /* These two fields are used to manipulate morphological filters like:
    * 
    * <<en$>>
    * 
    * 'filters' is a structure used to store the filters. 'filter_match_index' is used to
    * know if a given token is matched by a given filter.
    */
   FilterSet* filters;
   FilterMatchIndex* filter_match_index;
   #endif
   
   /* The compound word tree*/
   struct DLC_tree_info* DLC_tree;
   
   /* Array containing optimized states equivalent to the original fst2 ones */
   OptimizedFst2State* optimized_states;

   /* The original fst2 */
   Fst2* fst2;
   
   /* The tags of the original fst2, for optimization reasons */
   Fst2Tag* tags;

   /* The text tokens */
   struct string_hash* tokens;
   
   /* Current origin position in the token buffer */
   int current_origin;
};

int locate_pattern(char*,char*,char*,char*,char*,char*,char*,int,int,char*,int);

void numerote_tags(Fst2*,struct string_hash*,int*,struct string_hash*,Alphabet*,int*,int*,int*,int,struct locate_parameters*);
void decouper_entre_angles(unichar*,unichar*,unichar*,unichar*,struct string_hash*,Alphabet*);
unsigned char get_control_byte(unichar*,Alphabet*,struct string_hash*,int);
void compute_token_controls(Alphabet*,char*,int,struct locate_parameters*);

#endif

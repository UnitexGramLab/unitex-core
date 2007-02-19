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

#include "Loading_dic.h"
#include "Error.h"
#include "BitArray.h"
#include "List_pointer.h"
#include "PatternTree.h"



/**
 * Returns 1 if the given sequence can be considered as a simple word;
 * 0 otherwise.
 * 
 * NOTE: with such a definition, a single char that is not a letter is not a simple word
 */
int is_a_simple_word(unichar* sequence,Alphabet* alphabet,int tokenization_mode) {
int i;
i=0;
if (tokenization_mode==CHAR_BY_CHAR_TOKENIZATION && u_strlen(sequence)>1) {
   /* In a char by char mode, a string longer than 1 cannot be a simple word */
   return 0;
}
/* Here, we are a bit parano since '\0' is not supposed to be in the alphabet */
while (sequence[i]!='\0' && is_letter(sequence[i],alphabet)) {
   i++;
}
if (sequence[i]=='\0') {
   return 1;
}
return 0;
}


/**
 * This function loads a DLF or a DLC. It computes information about tokens
 * that will be used during the Locate operation. For instance, if we have the
 * following line:
 * 
 *   extended,.A
 * 
 * and if the .fst2 to be applied to the text contains the pattern <A> with,
 * number 456, then the function will mark the "extended" token to be matched 
 * by the pattern 456. Moreover, all case variations will be taken into account,
 * so that the "Extended" and "EXTENDED" tokens will also be updated.
 * 
 * The three parameters 'is_DIC_pattern', 'is_CDIC_pattern' and 'is_SDIC_pattern'
 * indicates if the .fst2 contains the corresponding patterns. For instance, if
 * the pattern "<SDIC>" is used in the grammar, it means that any token that is a
 * simple word must be marked as be matched by this pattern.
 */
void load_dic_for_locate(char* dic_name,Alphabet* alphabet,
                         int number_of_patterns,int is_DIC_pattern,
                         int is_CDIC_pattern,int is_SDIC_pattern,
                         struct lemma_node* root,struct locate_parameters* parameters) {
struct string_hash* tokens=parameters->tokens;
FILE* f;
unichar line[DIC_LINE_SIZE];
f=u_fopen(dic_name,U_READ);
if (f==NULL) {
   error("Cannot open dictionary %s\n",dic_name);
   return;
}
/* We parse all the lines */
while (EOF!=u_read_line(f,line)) {
   if (line[0]=='/') {
      /* NOTE: DLF and DLC files are not supposed to contain comment
       *       lines, but we test them, just in the case */
      continue;
   }
   struct dela_entry* entry=tokenize_DELAF_line(line,1);
   if (entry==NULL) {
      /* This case should never happen */
      error("Invalid dictionary line in load_dic_for_locate\n");
      continue;
   }
   /* We add the inflected form to the list of forms associated to the lemma.
    * This will be used to replace patterns like "<be>" by the actual list of
    * forms that can be matched by it, for optimization reasons */
   add_inflected_form_for_lemma(entry->inflected,entry->lemma,root);
   /* We get the list of all tokens that can be matched by the inflected form of this
    * this entry, with regards to case variations (see the "extended" example above). */
   struct list_int* ptr=get_token_list_for_sequence(entry->inflected,alphabet,tokens);
   /* We save the list pointer to free it later */
   struct list_int* ptr_copy=ptr;
   /* Here, we will deal with all simple words */
   while (ptr!=NULL) {
      int i=ptr->n;
      /* If the current token can be matched, then it can be recognized by the "<DIC>" pattern */
      parameters->token_control[i]=(unsigned char)(get_control_byte(tokens->value[i],alphabet,NULL,parameters->tokenization_policy)|DIC_TOKEN_BIT_MASK);
      if (number_of_patterns) {
         /* We look for matching patterns only if there are some */
         struct list_pointer* list=get_matching_patterns(entry,parameters->pattern_tree_root);
         if (list!=NULL) {
            /* If we have some patterns to add */
            if (parameters->matching_patterns[i]==NULL) {
               /* We allocate the pattern bit array, if needed */
               parameters->matching_patterns[i]=new_bit_array(number_of_patterns,ONE_BIT);
            }
            struct list_pointer* tmp=list;
            while (tmp!=NULL) {
               /* Then we add all the pattern numbers to the bit array */
               set_value(parameters->matching_patterns[i],((struct constraint_list*)(tmp->pointer))->pattern_number,1);
               tmp=tmp->next;
            }
            /* Finally, we free the constraint list */
            free_list_pointer(list);
         }
      }
      ptr=ptr->next;
   }
   /* Finally, we free the token list */
   free_list_int(ptr_copy);
   if (!is_a_simple_word(entry->inflected,alphabet,parameters->tokenization_policy)) {
      /* If the inflected form is a compound word */
      if (is_DIC_pattern || is_CDIC_pattern) {
         /* If the .fst2 contains "<DIC>" and/or "<CDIC>", then we
          * must note that all compound words can be matched by them */
         add_compound_word_with_no_pattern(entry->inflected,alphabet,tokens,parameters->DLC_tree,parameters->tokenization_policy,parameters->SPACE);
      }
      if (number_of_patterns) {
         /* We look for matching patterns only if there are some */
         /* We look if the compound word can be matched by some patterns */
         struct list_pointer* list=get_matching_patterns(entry,parameters->pattern_tree_root);
         struct list_pointer* tmp=list;
         while (tmp!=NULL) {
            /* If the word is matched by at least one pattern, we store it. */
            int pattern_number=((struct constraint_list*)(tmp->pointer))->pattern_number;
            add_compound_word_with_pattern(entry->inflected,pattern_number,alphabet,tokens,parameters->DLC_tree,parameters->tokenization_policy,parameters->SPACE); 
            tmp=tmp->next;
         }
         free_list_pointer(list);
      }
   }
   free_dela_entry(entry);
}
u_fclose(f);
}


/**
 * This function checks for each tag token like "{extended,extend.V:K}"
 * if it verifies some patterns. Its behaviour is very similar to the one
 * of the load_dic_for_locate function. However, as a side effect, this
 * function fills 'tag_token_list' with the list of tag token numbers.
 * This list is later used during Locate preprocessings.
 */
void check_patterns_for_tag_tokens(Alphabet* alphabet,int number_of_patterns,
									struct lemma_node* root,struct locate_parameters* parameters) {
struct string_hash* tokens=parameters->tokens;
for (int i=0;i<tokens->size;i++) {
   if (tokens->value[i][0]=='{' && u_strcmp_char(tokens->value[i],"{S}")  && u_strcmp_char(tokens->value[i],"{STOP}")) {
      /* If the token is tag like "{today,.ADV}", we add its number to the tag token list */
      parameters->tag_token_list=head_insert(i,parameters->tag_token_list);
      /* And we look for the patterns that can match it */
      struct dela_entry* entry=tokenize_tag_token(tokens->value[i]);
      if (entry==NULL) {
         /* This should never happen */
         fatal_error("Invalid tag token in function check_patterns_for_tag_tokens\n");
      }
      /* We add the inflected form to the list of forms associated to the lemma.
      * This will be used to replace patterns like "<be>" by the actual list of
      * forms that can be matched by it, for optimization reasons */
      add_inflected_form_for_lemma(tokens->value[i],entry->lemma,root);
      parameters->token_control[i]=(unsigned char)(get_control_byte(tokens->value[i],alphabet,NULL,parameters->tokenization_policy)|DIC_TOKEN_BIT_MASK);
      if (number_of_patterns) {
         /* We look for matching patterns only if there are some */
         struct list_pointer* list=get_matching_patterns(entry,parameters->pattern_tree_root);
         if (list!=NULL) {
            if (parameters->matching_patterns[i]==NULL) {
               /* We allocate the bit array if needed */
               parameters->matching_patterns[i]=new_bit_array(number_of_patterns,ONE_BIT);
            }
            struct list_pointer* tmp=list;
            while (tmp!=NULL) {
               set_value(parameters->matching_patterns[i],((struct constraint_list*)(tmp->pointer))->pattern_number,1);
               tmp=tmp->next;
            }
            free_list_pointer(list);
         }
      }
      /* At the opposite of DLC lines, a compound word tag like "{all around,.ADV}"
       * does not need to be put in the compound word tree, since the tag is already
       * characterized by its token number. */
      free_dela_entry(entry);
   }
}
}


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

#include "LocateFst2Tags.h"
#include "BitMasks.h"
#include "MetaSymbols.h"
#include "Error.h"
#include "PatternTree.h"
#include "Alphabet.h"


/**
 * This function analyses the inputs of all the tags of the given .fst2 in
 * order to determine their kind. 'tokens' contains all the text tokens.
 * After the execution of the function, 'number_of_patterns' will contain
 * the number of patterns found in the grammar, and 'is_DIC'/'is_CDIC'/'is_SDIC'
 * will contain 1 if the tag 'DIC'/'CDIC'/'SDIC' has been found. See
 * the comment below about the special case for '<!DIC>'.
 */
void process_tags(int *number_of_patterns,
                  struct string_hash* semantic_codes,
                  int *is_DIC,int *is_CDIC,
                  int *is_SDIC,struct locate_parameters* parameters) {
(*number_of_patterns)=0;
(*is_DIC)=0;
(*is_CDIC)=0;
(*is_SDIC)=0;
Fst2* fst2=parameters->fst2;
struct string_hash* tokens=parameters->tokens;
Fst2Tag* tag=fst2->tags;
/* We get the number of the SPACE token */
unichar t[2];
t[0]=' ';
t[1]='\0';
parameters->SPACE=get_value_index(t,tokens,DONT_INSERT);
/* Then, we test all the tags */
for (int i=0;i<fst2->number_of_tags;i++) {
   int length=u_strlen(tag[i]->input);
   if (tag[i]->input[0]=='$' && tag[i]->input[length-1]=='(') {
      /* If the tag is a variable start like $a(, we must add it
       * the variable list of the fst2. */
      tag[i]->type=BEGIN_VAR_TAG;
      tag[i]->variable=u_strdup(&(tag[i]->input[1]),length-2);
   }
   else if (tag[i]->input[0]=='$' && tag[i]->input[length-1]==')') {
      /* If the tag is a variable end like $a), we must add it
       * the variable list of the fst2. */
      tag[i]->type=END_VAR_TAG;
      tag[i]->variable=u_strdup(&(tag[i]->input[1]),length-2);
   }
   else if (!u_strcmp_char(tag[i]->input,"$[")) {
      /* If we have a positive context start */
      tag[i]->type=BEGIN_POSITIVE_CONTEXT_TAG;
   }
   else if (!u_strcmp_char(tag[i]->input,"$![")) {
      /* If we have a negative context start */
      tag[i]->type=BEGIN_NEGATIVE_CONTEXT_TAG;
   }
   else if (!u_strcmp_char(tag[i]->input,"$]")) {
      /* If we have a context end */
      tag[i]->type=END_CONTEXT_TAG;
   }
   else if (!u_strcmp_char(tag[i]->input,"#")) {
      /* If we have a #, we must check if it is the meta one that
       * forbids space or the "#" token */
      if (is_bit_mask_set(tag[i]->control,RESPECT_CASE_TAG_BIT_MASK)) {
         /* If the case respect bit is set to 1, then we have the "#" token */
         tag[i]->type=PATTERN_TAG;
         tag[i]->pattern=build_token_pattern(tag[i]->input);
      }
      else {
         /* If we have the meta # */
         tag[i]->type=META_TAG;
         tag[i]->meta=META_SHARP;
      }
   }
   else if (!u_strcmp_char(tag[i]->input,"<E>")) {
      /* If we have a transition tagged by the empty word epsilon */
      tag[i]->type=META_TAG;
      tag[i]->meta=META_EPSILON;
   }
   else {
      int token_number=get_value_index(tag[i]->input,tokens,DONT_INSERT);
      if (token_number!=-1) {
         /* If the input is an existing token */
         if (token_number==parameters->SPACE) {
            /* If it is a space */
            tag[i]->type=META_TAG;
            tag[i]->meta=META_SPACE;
         } else {
            /* If it is a normal token */
            tag[i]->type=PATTERN_TAG;
            tag[i]->pattern=build_token_pattern(tag[i]->input);
         }
      }
      else {
         /* This input is not an existing token. Two cases can happen:
          * 1) metas like <!MOT> or patterns like <V:K>
          * 2) a word that is not in the text tokens */
         if (tag[i]->input[0]!='<' || tag[i]->input[length-1]!='>') {
            /* If we are in case 2, it may not be an error. For instance,
             * if the tag contains "foo" and if it is a tag that allows
             * case variations, we could match "FOO" if this token is in the
             * text. */
            tag[i]->type=PATTERN_TAG;
            tag[i]->pattern=build_token_pattern(tag[i]->input);
         } else {
            /* If we have something of the form <...>, we must test first if it is
             * or not a negative tag like <!XXX> */
            int negative_tag=(tag[i]->input[1]=='!')?1:0;
            if (negative_tag) {
               set_bit_mask(&(tag[i]->control),NEGATION_TAG_BIT_MASK);
            }
            /* Then, we must test if we have or not a meta. To do that, we
             * extract the content without < > and ! if any.*/
            unichar* content=u_strdup(&(tag[i]->input[1+negative_tag]),length-2-negative_tag);
            /* And we test all the possible metas */
            if (!u_strcmp_char(content,"MOT")) {
               tag[i]->type=META_TAG;
               tag[i]->meta=META_MOT;
            }
            else if (!u_strcmp_char(content,"DIC")) {
               tag[i]->type=META_TAG;
               tag[i]->meta=META_DIC;
               if (!negative_tag) {
                  /* We mark that the DIC tag has been found, but only
                   * if it is not the negative one (<!DIC>). We do this
                   * because things matched by <DIC> will be taken from
                   * the 'dlf' and 'dlc' files, whereas things matched by <!DIC>
                   * will be taken from the 'err' file. Such a trick is necessary
                   * if we don't want 'priori' to be taken as an unknown word since
                   * it is  part of the compound word 'a priori' */
                  (*is_DIC)=1;
               }
            }
            else if (!u_strcmp_char(content,"CDIC")) {
               tag[i]->type=META_TAG;
               tag[i]->meta=META_CDIC;
               (*is_CDIC)=1;
            }
            else if (!u_strcmp_char(content,"SDIC")) {
               tag[i]->type=META_TAG;
               tag[i]->meta=META_SDIC;
               (*is_SDIC)=1;
            }
            else if (!u_strcmp_char(content,"MAJ")) {
               tag[i]->type=META_TAG;
               tag[i]->meta=META_MAJ;
            }
            else if (!u_strcmp_char(content,"MIN")) {
               tag[i]->type=META_TAG;
               tag[i]->meta=META_MIN;
            }
            else if (!u_strcmp_char(content,"PRE")) {
               tag[i]->type=META_TAG;
               tag[i]->meta=META_PRE;
            }
            else if (!u_strcmp_char(content,"NB")) {
               tag[i]->type=META_TAG;
               tag[i]->meta=META_NB;
               if (negative_tag) {
                  error("Negative mark will be ignored in <!NB>\n");
               }
            }
            else if (!u_strcmp_char(content,"TOKEN")) {
               tag[i]->type=META_TAG;
               tag[i]->meta=META_TOKEN;
            }
            else {
               /* If we arrive here, we have not a meta but a pattern like
                * <be>, <be.V>, <V:K>, ... */
               tag[i]->type=PATTERN_TAG;
               tag[i]->pattern=build_pattern(content,semantic_codes);
               if (tag[i]->pattern->type==CODE_PATTERN ||
                   tag[i]->pattern->type==LEMMA_AND_CODE_PATTERN ||
                   tag[i]->pattern->type==FULL_PATTERN) {
                  /* If the pattern we obtain contains grammatical/semantic
                   * codes, then we put it in the pattern tree and we note its number. */
                  tag[i]->pattern_number=add_pattern(number_of_patterns,tag[i]->pattern,parameters->pattern_tree_root);
                  if (tag[i]->pattern->type==CODE_PATTERN) {
                     /* If we have a code pattern, then the tag will just need to contain
                      * the pattern number, so that we can free the pattern. */
                     tag[i]->type=PATTERN_NUMBER_TAG;
                     free_pattern(tag[i]->pattern);
                     tag[i]->pattern=NULL;
                  }
               }
            }
            /* We don't forget to free the content */
            free(content);
         }
      }
   }
}
}


/**
 * This function optimizes a pattern of the form "eat".
 */
void optimize_token_pattern(int i,Fst2Tag* tag,Alphabet* alph,
               struct lemma_node* root,
               struct locate_parameters* p) {
/* Whatever happens, this pattern will be turned into a token list */
tag[i]->type=TOKEN_LIST_TAG;
unichar* opt_token=tag[i]->pattern->inflected;
/* First, we check if this token pattern can recognize some tag tokens */
struct list_int* list=p->tag_token_list;
while (list!=NULL) {
   struct dela_entry* entry=tokenize_tag_token(p->tokens->value[list->n]);
   if ((!is_bit_mask_set(tag[i]->control,RESPECT_CASE_TAG_BIT_MASK) && is_equal_or_uppercase(opt_token,entry->inflected,alph)) ||
       !u_strcmp(opt_token,entry->inflected)) {
      tag[i]->matching_tokens=sorted_insert(list->n,tag[i]->matching_tokens);
   }
   free_dela_entry(entry);
   list=list->next;
}
/* Then, we look for normal tokens */
if (is_bit_mask_set(tag[i]->control,RESPECT_CASE_TAG_BIT_MASK)) {
   /* If no case variants are allowed, then we just have to insert the number
    * of the token, but only if this token in the text ones. */
   int token_number;
   if (-1!=(token_number=get_value_index(opt_token,p->tokens,DONT_INSERT))) {
      tag[i]->matching_tokens=sorted_insert(token_number,tag[i]->matching_tokens);
   }
   return;
}
/* Here, we have to get all the case variants of the token. */
tag[i]->matching_tokens=sorted_merge(get_token_list_for_sequence(opt_token,alph,p->tokens),tag[i]->matching_tokens);
}


/**
 * This function checks if a pattern of the form "<eat>", "<eat.V>" or "<eaten,eat.V>"
 * can match the given tag token like "{today,.ADV}".
 */
void optimize_full_pattern_for_tag(unichar* tag_token,int i,Fst2Tag* tag,Alphabet* alph,
               struct lemma_node* root,
               struct locate_parameters* p) {
int token_number=get_value_index(tag_token,p->tokens);
struct dela_entry* entry=tokenize_tag_token(tag_token);
struct pattern* pattern=tag[i]->pattern;
if (pattern->type==LEMMA_PATTERN) {
   /* If the pattern has a constraint on the inflected form, we check it.
    * Note that we allow case variants so that the pattern "<today,.ADV>" will
    * match the tag "{TODAY,today.ADV}". This is a pure convention that could
    * change in the future. */
   if (!is_equal_or_uppercase(entry->lemma,pattern->lemma,alph)) {
      free_dela_entry(entry);
      return;
   }
}
if (pattern->type==LEMMA_AND_CODE_PATTERN || pattern->type==FULL_PATTERN) {
   /* If the pattern contains a constraint on grammatical/semantic/inflectional
    * codes, then it has been put in the pattern tree, and so, this pattern
    * was tried on the current tag token in the 'check_patterns_for_tag_tokens'
    * function. Then, we just have to test if the tag token matches this pattern. */
   if (p->matching_patterns==NULL ||
       0==get_value(p->matching_patterns[token_number],tag[i]->pattern_number)) {
      /* If the tag token does not match the pattern */
      free_dela_entry(entry);
      return;
   }
}
/* If the pattern matches the tag token, we add the tag token number to the list of
 * the tokens matched by the tag. */
tag[i]->matching_tokens=sorted_insert(token_number,tag[i]->matching_tokens);
/* We don't forget to free the dela entry. */
free_dela_entry(entry);
}


/**
 * Returns 1 if the given string contains only one token; 0 otherwise.
 */
int is_a_simple_token(unichar* string, Alphabet* alph,TokenizationPolicy tokenization_policy) {
if (is_a_simple_word(string,alph,tokenization_policy) || (u_strlen(string)==1)) {
   return 1;
}
return 0;
}


/**
 * This function checks if a pattern of the form "<eat>", "<eat.V>" or "<eaten,eat.V>"
 * can match the given token sequence like "today" or "black-eyed".
 */
void optimize_full_pattern_for_sequence(unichar* sequence,int i,Fst2Tag* tag,Alphabet* alph,
               struct lemma_node* root,
               struct locate_parameters* p) {
/* First, we test if the given sequence corresponds or not to a single token
 * like "today". */
if (!is_a_simple_token(sequence,alph,p->tokenization_policy)) {
   /* If we have a sequence of several tokens like "black-eyed", we handle it
    * as a compound word. To do that, we create a compound word pattern espacially
    * to match this sequence. */
   if (tag[i]->compound_pattern==NO_COMPOUND_PATTERN) {
      /* If the tag has not already a compound pattern number, we create one */
      tag[i]->compound_pattern=p->current_compound_pattern;
      p->current_compound_pattern++;
   }
   /* Then, we add the compound sequence into the compound word tree. */
   add_compound_word_with_pattern(sequence,tag[i]->compound_pattern,alph,p->tokens,
                                  p->DLC_tree,p->tokenization_policy,p->SPACE); 
   return;
}
/* If we have a single sequence like "today", we get the list of all its case variants
 * that actually are in the text. */
struct list_int* list=get_token_list_for_sequence(sequence,alph,p->tokens);
struct list_int* head=list;
int is_lemma_pattern=(tag[i]->pattern->type==LEMMA_PATTERN);
while (list!=NULL) {
   if (is_lemma_pattern || 
       ((p->matching_patterns[list->n]!=NULL) && get_value(p->matching_patterns[list->n],tag[i]->pattern_number))) {
      /* If the token can be matched by the pattern, we put it in the list of
       * the tokens that the tag can match. */
      tag[i]->matching_tokens=sorted_insert(list->n,tag[i]->matching_tokens);
   }
   list=list->next;
}
/* Finally, we free the list */
free_list_int(head);
}


/**
 * This function optimizes a pattern of the form "<eat>", "<eat.V>" or "<eaten,eat.V>".
 */
void optimize_full_pattern(int i,Fst2Tag* tag,Alphabet* alph,
               struct lemma_node* root,
               struct locate_parameters* p) {
/* Whatever happens, this pattern will be turned into a token list */
tag[i]->type=TOKEN_LIST_TAG;
/* We look at all the inflected forms that corresponds to the lemma of the pattern.
 * The association inflected forms/lemmas has been made during the loading of
 * the text dictionaries and while checking all the tag tokens of the text. */
struct list_ustring* inflected_forms=get_inflected_forms(tag[i]->pattern->lemma,root);
while (inflected_forms!=NULL) {
   if (inflected_forms->string[0]=='{' && u_strcmp_char(inflected_forms->string,"{")) {
      /* We can have a tag token like "{today,.ADV}" */
      optimize_full_pattern_for_tag(inflected_forms->string,i,tag,alph,root,p);
   } else {
      /* Or a normal token sequence like "today" */
      optimize_full_pattern_for_sequence(inflected_forms->string,i,tag,alph,root,p);
   }
   inflected_forms=inflected_forms->next;
}
}


/**
 * This function looks for all token pattern tags or pattern tags
 * that contain a lemma. Then, such patterns are replaced by the
 * exact list of tokens they can match, including case variants.
 * For instance, if we have "<be>", it is first replaced by the list
 * "be", "am", "are", "is", "was", "been", "being". This list has been
 * computed from the text dictionaries when they were loaded. Then, this
 * list is replaced by the actual forms found in the text tokens. For instance,
 * if the text contains "Be", "AM", "are", "Are", then this second list will
 * be the list of all tokens that an actually be matched by the pattern "<be>".
 * Note that a pattern can even be replaced by an empty list if it can
 * match nothing.
 */
void optimize_pattern_tags(Alphabet* alphabet,
                           struct lemma_node* root,
                           struct locate_parameters* parameters) {
int n_tags=parameters->fst2->number_of_tags;
Fst2Tag* tag=parameters->fst2->tags;
for (int i=0;i<n_tags;i++) {
   if (tag[i]->type==PATTERN_TAG) {
      /* We just look at pattern tags */
      switch (tag[i]->pattern->type) {
         case TOKEN_PATTERN: optimize_token_pattern(i,tag,alphabet,root,parameters);
                             break;
         case LEMMA_PATTERN: /* There is no difference in the handling of these
                              * kind of patterns */
         case LEMMA_AND_CODE_PATTERN: 
         case FULL_PATTERN: optimize_full_pattern(i,tag,alphabet,root,parameters);
                            break;
         default: /* Here, a code pattern would be an error since they are supposed
                   * to have been replaced by pattern numbers. An undefined pattern
                   * is also an error, of course. */
                  fatal_error("Invalid pattern type in optimize_pattern_tags\n");
                             
      }
   }
}
}

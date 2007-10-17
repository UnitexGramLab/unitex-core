 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Tokenization.h"
#include "Error.h"
#include "Unicode.h"


/**
 * This function takes a text sequence and returns the list of its tokens.
 * 'mode' defines if the tokenization must be done character by character 
 * or word by word. In that last case, a word is defined as a contiguous
 * sequence letters and the letters are defined by the given alphabet.
 */
struct list_ustring* tokenize(unichar* text,TokenizationPolicy mode,Alphabet* alphabet) {
if (mode==CHAR_BY_CHAR_TOKENIZATION) {
   return tokenize_char_by_char(text);
}
return tokenize_word_by_word(text,alphabet);
}


/**
 * This function takes a text sequence and returns the list of its tokens.
 * The tokenization is done character by character.
 */
struct list_ustring* tokenize_char_by_char(unichar* text) {
if (text==NULL) {
   fatal_error("NULL text in tokenize_char_by_char\n");
}
struct list_ustring* tokens=NULL;
unichar tmp[2];
tmp[1]='\0';
for (int l=u_strlen(text)-1;l>=0;l--) {
   tmp[0]=text[l];
   tokens=new_list_ustring(tmp,tokens);
}
return tokens;
}


/**
 * This function takes a text sequence and returns the list of its tokens.
 * The tokenization is done word by word. If the given alphabet is NULL,
 * then letters are tested with the 'u_is_letter' function.
 */
struct list_ustring* tokenize_word_by_word(unichar* text,Alphabet* alphabet) {
if (text==NULL) {
   fatal_error("NULL text in tokenize_word_by_word\n");
}
unichar tmp[4096];
int pos=0;
struct list_ustring* tokens=NULL;
struct list_ustring* end=NULL;
while (text[pos]!='\0') {
   if (is_letter2(text[pos],alphabet)) {
      /* If we have a letter, we must read the whole letter sequence */
      int j=0;
      while (j<(4096-1) && is_letter2(text[pos],alphabet)) {
         /* The loop while end if we find a non letter chararacter,
          * including '\0' */
         tmp[j++]=text[pos++];
      }
      if (j==(4096-1)) {
         fatal_error("Word too long in tokenize_word_by_word\n");
      }
      tmp[j]='\0';
   } else {
      /* If we have a non letter, it is a token by itself */
      tmp[0]=text[pos];
      tmp[1]='\0';
      pos++;
   }
   /* Then, we add the 'tmp' token to our list */
   if (tokens==NULL) {
      /* If the list was empty */
      tokens=new_list_ustring(tmp);
      end=tokens;
   } else {
      end->next=new_list_ustring(tmp);
      end=end->next;
   }
}
return tokens;
}


/**
 * Returns 1 if the given string contains only one token; 0 otherwise.
 */
int is_a_simple_token(unichar* string,TokenizationPolicy tokenization_policy,Alphabet* alph) {
if (is_a_simple_word(string,tokenization_policy,alph) || (u_strlen(string)==1)) {
   return 1;
}
return 0;
}


/**
 * Returns 1 if the given sequence can be considered as a simple word;
 * 0 otherwise.
 * 
 * NOTE: with such a definition, a single char that is not a letter is not a simple word
 */
int is_a_simple_word(unichar* sequence,TokenizationPolicy tokenization_policy,Alphabet* alphabet) {
int i;
i=0;
if (tokenization_policy==CHAR_BY_CHAR_TOKENIZATION && u_strlen(sequence)>1) {
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

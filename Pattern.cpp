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

#include "Pattern.h"
#include "Error.h"
#include "StringParsing.h"




/**
 * Allocates, initializes and returns a new pattern.
 */
struct pattern* new_pattern() {
struct pattern* p;
p=(struct pattern*)malloc(sizeof(struct pattern));
if (p==NULL) {
   fatal_error("Not enough memory in new_pattern\n");
}
p->inflected=NULL;
p->lemma=NULL;
p->grammatical_codes=NULL;
p->inflectional_codes=NULL;
p->forbidden_codes=NULL;
p->type=UNDEFINED_PATTERN;
return p;
}


/**
 * Frees all the memory associated to the given pattern.
 */
void free_pattern(struct pattern* p) {
if (p==NULL) return;
if (p->inflected!=NULL) free(p->lemma);
if (p->lemma!=NULL) free(p->lemma);
free_list_ustring(p->grammatical_codes);
free_list_ustring(p->inflectional_codes);
free_list_ustring(p->forbidden_codes);
free(p);
}



/**
 * Returns 1 if s is a code pattern (V:Kms, N+Hum, ...); 0 otherwise.
 * 'semantic_codes' is a string_hash that contains all the possible 
 * grammatical/semantic codes.
 */
int is_code_pattern(unichar* s,struct string_hash* semantic_codes) {
if ((s==NULL)||(s[0]=='\0')) {
   fatal_error("NULL or empty pattern in is_code_pattern\n");
}
int i=0;
unichar tmp[2048];
if (P_BACKSLASH_AT_END==parse_string(s,&i,tmp,P_PLUS_MINUS_COLON)) {
   fatal_error("Backslash at end of a pattern\n");
}
/* If we have found '+' '-' or ':', then we have a code pattern */
if (s[i]!='\0') {
   return 1;
}
/* Otherwise, we test if the string is a grammatical or semantic code */
if (get_value_index(s,semantic_codes,DONT_INSERT)!=-1) {
   return 1;
}
return 0;
}


/**
 * This function sorts the character that compose the given string.
 * We use here the selection sort.
 */
void sort_ustring(unichar* s) {
if (s==NULL) {
   fatal_error("NULL error in sort_ustring\n");
}
int i=0;
while (s[i]!='\0') {
   unichar min=s[i];
   int min_index=i;
   for (int j=i+1;s[j]!='\0';j++) {
      if (min>s[j]) {
         min=s[j];
         min_index=j;
      }
   }
   s[min_index]=s[i];
   s[i]=min;
   i++;
}
}


/**
 * This function takes a pattern structure and a string that represents
 * grammatical/semantical/inflectional codes constraints like
 * "V-z3:P3s:I3s". It adds the corresponding information to the
 * ones of the given patterns. For "V-z3:P3s:I3s", we would have:
 * "V" -> p->grammatical_codes
 * "z3" -> p->forbidden_codes
 * "P3s" and "I3s" -> p->inflectional_codes
 */
void build_code_pattern(struct pattern* p,unichar* codes) {
unichar tmp[2048];
int pos=0;
int minus=0;
if (codes[0]==':') {
   fatal_error("Invalid pattern with no grammatical code before inflectional codes\n");
}
/* First, we look for grammatical or forbidden codes */
do {
   switch(codes[pos]) {
      case '+': pos++; minus=0; break;
      case '-': pos++; minus=1; break;
      default: minus=0;
   }
   if (P_BACKSLASH_AT_END==parse_string(codes,&pos,tmp,P_PLUS_MINUS_COLON)) {
      fatal_error("Backslash at end of pattern\n");
   }
   if (minus) {
      /* If we have read a forbidden code */
      p->forbidden_codes=sorted_insert(tmp,p->forbidden_codes);
   } else {
      /* If we have read a grammatical/semantic code */
      p->grammatical_codes=sorted_insert(tmp,p->grammatical_codes);
   }
} while (codes[pos]!='\0' && codes[pos]!=':');
/* Then, we read for inflectional codes, if any */
if (codes[pos]=='\0') return;
do {
   pos++;
   if (P_BACKSLASH_AT_END==parse_string(codes,&pos,tmp,P_COLON)) {
      fatal_error("Backslash at end of pattern\n");
   }
   /* Remember that the characters that compose an inflectional code
    * must be sorted. For instance, "P3s" will be represented by "3Ps" */
   sort_ustring(tmp);
   p->inflectional_codes=sorted_insert(tmp,p->inflectional_codes);
} while (codes[pos]!='\0');
}


/**
 * This function takes a string representing a pattern like
 * <be.V:K>, but without the < and > characters. It splits it
 * and builds a pattern from it. Raises a fatal error in case
 * of malformed pattern.
 */
struct pattern* build_pattern(unichar* s,struct string_hash* semantic_codes) {
struct pattern* p=new_pattern();
int pos;
unichar tmp[2048];
if ((s==NULL)||(s[0]=='\0')) {
   fatal_error("The empty pattern <> has been found\n");
}
pos=0;
if (P_BACKSLASH_AT_END==parse_string(s,&pos,tmp,P_COMMA_DOT)) {
   fatal_error("Backslash at end of pattern\n");
}
/*
i=0;
int k=0;
inflected[0]='\0';
while ((s[i]!=',')&&(s[i]!='.')&&(s[i]!='\0')) {
   if (s[i]=='\\') {i++;}
   inflected[k++]=s[i++];
}
inflected[k]='\0';*/
/* If we are in the <XXX> case, we must decide if XXX is a lemma
 * or a combination of grammatical/semantic/inflectional codes */
if (s[/*i*/pos]=='\0') {
   /* We must test on s and NOT on inflected, because of patterns like
    * <A+faux\-ami>. In fact, s contains "A+faux\-ami" and inflected
    * contains "A+faux-ami". So, if we consider inflected instead of s, 
    * the minus will be taken as a negation and not as a part of the code
    * "faux-ami", and then, no difference will be made between 
    * "<A+faux\-ami>" and "<A+faux-ami>". */
   if (is_code_pattern(s,semantic_codes)) {
      /* If we are in the <V> case */
      p->type=CODE_PATTERN;
      build_code_pattern(p,s);
      return p;
   }
   else {
      /* If we are in the <be> case */
      p->type=LEMMA_PATTERN;
      p->lemma=u_strdup(/*inflected*/tmp);
      return p;
   }
}
/* If are in the <be.V> or <.V> case */
if (s[/*i*/pos]=='.') {
   if (/*inflected*/tmp[0]=='\0') {
      /* If we are in the <.V> case */
      p->type=CODE_PATTERN;
      build_code_pattern(p,&(s[/*i*/pos+1]));
      return p;
   }
   /* If we are in the <be.V> case */
   p->lemma=u_strdup(tmp);//inflected);
   p->type=LEMMA_AND_CODE_PATTERN;
   build_code_pattern(p,&(s[/*i*/pos+1]));
   return p;
}
/* If we are in the  <am,be.V> case */
if (/*inflected*/tmp[0]=='\0') {
   fatal_error("Invalid pattern has been found\n");
}
p->type=FULL_PATTERN;
p->inflected=u_strdup(tmp);//inflected);
/*i++;
j=0;
lemma[0]='\0';
while ((s[i]!='.')&&(s[i]!='\0')) {
   lemma[j++]=s[i++];
}
lemma[j]='\0';
*/
pos++;
switch(parse_string(s,&pos,tmp,P_DOT)) {
   case P_BACKSLASH_AT_END: {fatal_error("Backslash at end of pattern\n");}
   case P_EOS: {fatal_error("Missing grammatical code in pattern\n");}
}
/*if (j==0) {
   error("Invalid pattern has been found\n");
   free_pattern(p);
   return NULL;
}*/
if (s[/*i*/pos]=='\0') {
   fatal_error("Invalid pattern has been found\n");
}
p->lemma=u_strdup(tmp);//lemma);
build_code_pattern(p,&(s[/*i*/pos+1]));
return p;
}


/**
 * This function builds a token pattern from the given information.
 */
struct pattern* build_token_pattern(unichar* token) {
struct pattern* p=new_pattern();
p->type=TOKEN_PATTERN;
p->inflected=u_strdup(token);
return p;
}


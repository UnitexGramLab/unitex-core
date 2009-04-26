 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
   fatal_alloc_error("new_pattern");
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
if (p->inflected!=NULL) free(p->inflected);
if (p->lemma!=NULL) free(p->lemma);
free_list_ustring(p->grammatical_codes);
free_list_ustring(p->inflectional_codes);
free_list_ustring(p->forbidden_codes);
free(p);
}



/**
 * Tests if s is a code pattern (V:Kms, N+Hum, ...).
 * 'semantic_codes' is a string_hash that contains all the possible
 * grammatical/semantic codes. If NULL, the return value can be
 * AMBIGUOUS_PATTERN if there no indication that helps to guess if
 * we have a code or a lemma.
 */
enum pattern_type is_code_pattern(unichar* s,struct string_hash* semantic_codes) {
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
   return CODE_PATTERN;
}
/* If we have no grammatical codes, we can't decide */
if (semantic_codes==NULL) {
	return AMBIGUOUS_PATTERN;
}
/* Otherwise, we test if the string is a grammatical or semantic code */
if (get_value_index(s,semantic_codes,DONT_INSERT)!=-1) {
   return CODE_PATTERN;
}
return LEMMA_PATTERN;
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
/* If we are in the <XXX> case, we must decide if XXX is a lemma
 * or a combination of grammatical/semantic/inflectional codes */
if (s[pos]=='\0') {
   /* We must test on s and NOT on inflected, because of patterns like
    * <A+faux\-ami>. In fact, s contains "A+faux\-ami" and inflected
    * contains "A+faux-ami". So, if we consider inflected instead of s,
    * the minus will be taken as a negation and not as a part of the code
    * "faux-ami", and then, no difference will be made between
    * "<A+faux\-ami>" and "<A+faux-ami>". */
   p->type=is_code_pattern(s,semantic_codes);
   if (p->type==CODE_PATTERN) {
      /* If we are in the <V> case */
      build_code_pattern(p,s);
      return p;
   }
   else {
      /* If we are in the <XXX> where XXX is either a lemma or an unknown element
       * that can be both lemma and grammatical code */
      p->lemma=u_strdup(tmp);
      return p;
   }
}
/* If are in the <be.V> or <.V> case */
if (s[pos]=='.') {
   if (tmp[0]=='\0') {
      /* If we are in the <.V> case */
      p->type=CODE_PATTERN;
      build_code_pattern(p,&(s[pos+1]));
      return p;
   }
   /* If we are in the <be.V> case */
   p->lemma=u_strdup(tmp);
   p->type=LEMMA_AND_CODE_PATTERN;
   build_code_pattern(p,&(s[pos+1]));
   return p;
}
/* If we are in the  <am,be.V> case */
if (tmp[0]=='\0') {
   fatal_error("Invalid pattern has been found\n");
}
p->type=FULL_PATTERN;
p->inflected=u_strdup(tmp);

pos++;
switch(parse_string(s,&pos,tmp,P_DOT)) {
   case P_BACKSLASH_AT_END: {fatal_error("Backslash at end of pattern\n");}
   case P_EOS: {fatal_error("Missing grammatical code in pattern\n");}
}
if (s[pos]=='\0') {
   fatal_error("Invalid pattern has been found\n");
}
p->lemma=u_strdup(tmp);
build_code_pattern(p,&(s[pos+1]));
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


/**
 * Returns 1 if the given DELAF entry is compatible with the given code part of this pattern;
 * 0 otherwise.
 */
int is_compatible_code_pattern(struct dela_entry* entry,struct pattern* pattern) {
struct list_ustring* tmp=pattern->grammatical_codes;
while (tmp!=NULL) {
   if (!dic_entry_contain_gram_code(entry,tmp->string)) {
      /* If one code of the pattern is not present in the entry, we fail */
      return 0;
   }
   tmp=tmp->next;
}
tmp=pattern->forbidden_codes;
while (tmp!=NULL) {
   if (dic_entry_contain_gram_code(entry,tmp->string)) {
      /* If one forbidden code of the pattern is present in the entry, we fail */
      return 0;
   }
   tmp=tmp->next;
}
tmp=pattern->inflectional_codes;
while (tmp!=NULL) {
   if (!dic_entry_contain_inflectional_code(entry,tmp->string)) {
      /* If one inflectional code of the pattern is not present in the entry, we fail */
      return 0;
   }
   tmp=tmp->next;
}
return 1;
}


/**
 * Returns 1 if the given DELAF entry is compatible with the given pattern;
 * 0 otherwise.
 */
int is_entry_compatible_with_pattern(struct dela_entry* entry,struct pattern* pattern) {
switch(pattern->type) {
   case LEMMA_PATTERN: return (!u_strcmp(entry->lemma,pattern->lemma));
   case CODE_PATTERN: return is_compatible_code_pattern(entry,pattern);
   case LEMMA_AND_CODE_PATTERN: return (!u_strcmp(entry->lemma,pattern->lemma)) && is_compatible_code_pattern(entry,pattern);
   case FULL_PATTERN: return (!u_strcmp(entry->inflected,pattern->inflected)) && (!u_strcmp(entry->lemma,pattern->lemma)) && is_compatible_code_pattern(entry,pattern);
   case AMBIGUOUS_PATTERN: return !u_strcmp(entry->lemma,pattern->lemma) || dic_entry_contain_gram_code(entry,pattern->lemma);
   default: fatal_error("Unexpected case in is_entry_compatible_with_pattern\n");
}
return 0;
}


/**
 * Returns a clone of the pattern.
 */
struct pattern* clone(struct pattern* src) {
	struct pattern* dst;

	if (src == NULL)
		return NULL;

	dst=(struct pattern*)malloc(sizeof(struct pattern));
	if (dst==NULL) {
	   fatal_error("Not enough memory in new_pattern_ByCopy\n");
	}
	dst->inflected=u_strdup(src->inflected);
	dst->lemma=u_strdup(src->lemma);
	dst->grammatical_codes=clone(src->grammatical_codes);
	dst->inflectional_codes=clone(src->inflectional_codes);
	dst->forbidden_codes=clone(src->forbidden_codes);
	dst->type=src->type;
	return dst;
}



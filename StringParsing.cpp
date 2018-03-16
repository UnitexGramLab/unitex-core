/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "StringParsing.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Here we define some separator sets that will be used
 * many times.
 */
const unichar P_SPACE []= { ' ', 0 };
const unichar P_TAB []= { '\t', 0 };
const unichar P_COMMA[] = { ',', 0 };
const unichar P_DOT[] = { '.', 0 };
const unichar P_EQUAL[] = { '=', 0 };
const unichar P_PLUS[] = { '+', 0 };
const unichar P_COLON[] = { ':', 0 };
const unichar P_SLASH[] = { '/', 0 };
const unichar P_EXCLAMATION[] = { '!', 0 };
const unichar P_DOUBLE_QUOTE[] = { '"', 0 };
const unichar P_PLUS_COLON[] = { '+', ':', 0 };
const unichar P_PLUS_MINUS_COLON[] = { '+', '-', ':', 0 };
const unichar P_PLUS_TILDE_COLON[] = { '+', '~', ':', 0 };
const unichar P_PLUS_COLON_SLASH[] = { '+', ':', '/', 0 };
const unichar P_PLUS_COLON_SLASH_OPENING_BRACKET[] = { '+', ':', '/', '[', 0 };
const unichar P_PLUS_COLON_SLASH_EXCLAMATION_OPENING_BRACKET[] = { '+', ':', '/', '!', '[', 0 };
const unichar P_COLON_CLOSING_BRACKET[] = { ':', ']', 0 };
const unichar P_COLON_SLASH[] = { ':', '/', 0 };
const unichar P_CLOSING_ROUND_BRACKET[] = { '}', 0 };
const unichar P_COMMA_DOT[] = { ',', '.', 0 };
const unichar P_PLUS_COLON_SLASH_BACKSLASH[] = { '+', ':', '/' , '\\', 0 };
const unichar P_PLUS_COMMA_COLON_SLASH_BACKSLASH[] = { '+',',', ':', '/' , '\\', 0 };
const unichar P_COLON_SLASH_BACKSLASH[] = { ':', '/' , '\\', 0 };
const unichar P_COMMA_DOT_BACKSLASH_DIGITS[] = { ',', '.' , '\\', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 };
const unichar P_DOT_PLUS_SLASH_BACKSLASH[] = { '.', '+', '/', '\\', 0 };
const unichar P_DOT_COMMA_PLUS_SLASH_BACKSLASH[] = { '.', ',', '+', '/', '\\', 0 };
const unichar P_ELAG_TAG[] = { '.', '!', ':', '>', 0 };
const unichar P_DIGITS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 };
const unichar P_BACKSLASH_EQUAL[] = { '\\', '=', 0 };
const unichar P_BACKSLASH[] = { '\\', 0 };
const unichar P_COMMA_DOT_EQUAL_BACKSLASH[] = { ',', '.', '=', '\\', 0 };

const unichar STRING_EMPTY[] = { 0 };
/**
 * Parses the string 's' from the position '*ptr' until it finds '\0'
 * or a character that is in 'stop_chars'.
 * 'chars_to_keep_protected' is used to define which chars must keep their protection
 * char. If it has the value NULL, it means that all protected chars must stay protected.
 * Note that forbidden chars and stop_chars will be taken into account if and only
 * if they are not protected. The intersection between 'stop_chars' and 'forbidden_chars'
 * is supposed to be empty. If not, 'stop_chars' is considered first.
 * If an error occurs, a \0 is put at the end of the result and the function
 * returns an error code; otherwise, the substring obtained
 * is stored in 'result' and P_OK is returned.
 * If 's' is empty, the function returns P_EOS.
 * Note that '*ptr' is updated and points to the stop character (delimiter or '\0').
 * It is the responsability of the caller to increase the position if we are not
 * at the end of the input string.
 *
 * Example: parse("E\=\mc\2 is a formula",result," ","m","2") will produce the
 *          result = "E=mc\2"
 */

static inline int string_contains_unichar(const unichar* s,unichar c) {
for (;;) {
    if ((*(s)) == 0) return 0;
    if ((*(s)) == c) return 1;
    if ((*(s + 1)) == 0) return 0;
    if ((*(s + 1)) == c) return 1;
    if ((*(s + 2)) == 0) return 0;
    if ((*(s + 2)) == c) return 1;
    if ((*(s + 3)) == 0) return 0;
    if ((*(s + 3)) == c) return 1;
    s += 4;
}
 return 0;
}


int parse_string(const unichar* s,int *ptr,unichar* result,const unichar* stop_chars,
                 const unichar* forbidden_chars,const unichar* chars_to_keep_protected) {
stop_chars = (stop_chars == NULL) ? STRING_EMPTY : stop_chars;
forbidden_chars = (forbidden_chars == NULL) ? STRING_EMPTY : forbidden_chars;
int j=0;
result[0]='\0';
if (s[*ptr]=='\0') return P_EOS;
while (s[*ptr]!='\0') {
   if (s[*ptr]==PROTECTION_CHAR) {
      /* If there is a protection character (backslash) */
      if (s[(*ptr)+1]=='\0') {
         /* It must not appear at the end of the string */
         result[j]='\0';
         return P_BACKSLASH_AT_END;
      }
      if (chars_to_keep_protected==NULL || string_contains_unichar(chars_to_keep_protected,s[(*ptr)+1])) {
         /* If the character must keep its backslash */
         result[j++]=PROTECTION_CHAR;
      }
      result[j++]=s[(*ptr)+1];
      (*ptr)=(*ptr)+2;
   } else {
      /* If we have an unprotected character */
      if (string_contains_unichar(stop_chars,s[*ptr])) {
         /* If it is a stop char, we have finished */
         result[j]='\0';
         return P_OK;
      }
      if (string_contains_unichar(forbidden_chars,s[*ptr])) {
         /* If it is a forbidden char, it's an error */
         result[j]='\0';
         return P_FORBIDDEN_CHAR;
      }
      /* If it's a normal char, we copy it */
      result[j++]=s[(*ptr)++];
   }
}
/* If we arrive here, we have reached the end of the string without error */
result[j]='\0';
return P_OK;
}


/**
 * Parses the string 's' from '*ptr' until it finds '\0' or a character that is in
 * 'stop_chars'. '*ptr' is updated. All protected characters will be unprotected.
 * If an error occurs, it returns an error code; otherwise, the substring obtained
 * is stored in 'result' and P_OK is returned.
 */
int parse_string(const unichar* s,int *ptr,unichar* result,const unichar* stop_chars) {
return parse_string(s,ptr,result,stop_chars,P_EMPTY,P_EMPTY);
}


/**
 * Parses the string 's' from '*ptr' until it finds '\0' or a character that is in
 * 'stop_chars'. '*ptr' is updated. All protected characters will be unprotected.
 * If an error occurs, it returns an error code; otherwise, the substring obtained
 * is stored in 'result' and P_OK is returned.
 */
int parse_string(const unichar* s,int *ptr,unichar* result,const char* stop_chars) {
unichar* tmp=u_strdup(stop_chars);
int value=parse_string(s,ptr,result,tmp,P_EMPTY,P_EMPTY);
free(tmp);
return value;
}


/**
 * This function do the same as above, except that it starts from the
 * beginning of the string. It can be used for one-shot tokenization.
 */
int parse_string(const unichar* s,unichar* result,const unichar* stop_chars) {
int ptr=0;
return parse_string(s,&ptr,result,stop_chars,P_EMPTY,P_EMPTY);
}


/**
 * This function do the same as above, except that it starts from the
 * beginning of the string. It can be used for one-shot tokenization.
 */
int parse_string(const unichar* s,unichar* result,const char* stop_chars) {
unichar* tmp=u_strdup(stop_chars);
int ptr=0;
int value=parse_string(s,&ptr,result,tmp,P_EMPTY,P_EMPTY);
free(tmp);
return value;
}


/**
 * The Ustring version, in order to avoid buffer overflow in result.
 * Note that this version appends the string to result instead of replacing
 * its previous content.
 */
int parse_string(const unichar* s,int *ptr,Ustring* result,const unichar* stop_chars,
                 const unichar* forbidden_chars,const unichar* chars_to_keep_protected) {
stop_chars = (stop_chars == NULL) ? STRING_EMPTY : stop_chars;
forbidden_chars = (forbidden_chars == NULL) ? STRING_EMPTY : forbidden_chars;
if (s[*ptr]=='\0') return P_EOS;
while (s[*ptr]!='\0') {
   if (s[*ptr]==PROTECTION_CHAR) {
      /* If there is a protection character (backslash) */
      if (s[(*ptr)+1]=='\0') {
         /* It must not appear at the end of the string */
         return P_BACKSLASH_AT_END;
      }
      if (chars_to_keep_protected==NULL || string_contains_unichar(chars_to_keep_protected,s[(*ptr)+1])) {
         /* If the character must keep its backslash */
         u_strcat(result,PROTECTION_CHAR);
      }
      u_strcat(result,s[(*ptr)+1]);
      (*ptr)=(*ptr)+2;
   } else {
      /* If we have an unprotected character */
      if (string_contains_unichar(stop_chars,s[*ptr])) {
         /* If it is a stop char, we have finished */
         return P_OK;
      }
      if (string_contains_unichar(forbidden_chars,s[*ptr])) {
         /* If it is a forbidden char, it's an error */
         return P_FORBIDDEN_CHAR;
      }
      /* If it's a normal char, we copy it */
      u_strcat(result,s[(*ptr)++]);
   }
}
/* If we arrive here, we have reached the end of the string without error */
return P_OK;
}


/**
 * Parses the string 's' from '*ptr' until it finds '\0' or a character that is in
 * 'stop_chars'. '*ptr' is updated. All protected characters will be unprotected.
 * If an error occurs, it returns an error code; otherwise, the substring obtained
 * is stored in 'result' and P_OK is returned.
 */
int parse_string(const unichar* s,int *ptr,Ustring* result,const unichar* stop_chars) {
return parse_string(s,ptr,result,stop_chars,P_EMPTY,P_EMPTY);
}


/**
 * Parses the string 's' from '*ptr' until it finds '\0' or a character that is in
 * 'stop_chars'. '*ptr' is updated. All protected characters will be unprotected.
 * If an error occurs, it returns an error code; otherwise, the substring obtained
 * is stored in 'result' and P_OK is returned.
 */
int parse_string(const unichar* s,int *ptr,Ustring* result,const char* stop_chars) {
unichar* tmp=u_strdup(stop_chars);
int value=parse_string(s,ptr,result,tmp,P_EMPTY,P_EMPTY);
free(tmp);
return value;
}


/**
 * This function do the same as above, except that it starts from the
 * beginning of the string. It can be used for one-shot tokenization.
 */
int parse_string(const unichar* s,Ustring* result,const unichar* stop_chars) {
int ptr=0;
return parse_string(s,&ptr,result,stop_chars,P_EMPTY,P_EMPTY);
}


/**
 * This function do the same as above, except that it starts from the
 * beginning of the string. It can be used for one-shot tokenization.
 */
int parse_string(const unichar* s,Ustring* result,const char* stop_chars) {
unichar* tmp=u_strdup(stop_chars);
int ptr=0;
int value=parse_string(s,&ptr,result,tmp,P_EMPTY,P_EMPTY);
free(tmp);
return value;
}


/**
 * This function copies 's' into 'result', escaping the chars in
 * 'chars_to_escape'. Protected chars are not taken into account.
 *
 * Example: escape("E\=mc2",result,"2=") => result="E\=mc\2"
 *
 * Note that the protection char will be escaped if and only if
 * it appears in 'chars_to_escape':
 *
 * Example: escape("E\=mc2",result,"\") => result="E\\=mc2"
 *
 * The function returns the length of 'result'.
 *
 */
int escape(const unichar* s,unichar* result,const unichar* chars_to_escape) {
chars_to_escape = (chars_to_escape == NULL) ? STRING_EMPTY : chars_to_escape;
int j=0;
for (int i=0;s[i]!='\0';i++) {
   if (string_contains_unichar(chars_to_escape,s[i])) {
      result[j++]=PROTECTION_CHAR;
   } else if (s[i]==PROTECTION_CHAR) {
      i++;
      result[j++]=PROTECTION_CHAR;
   }
   result[j++]=s[i];
}
result[j]='\0';
return j;
}


/**
 * Quite the same, with a Ustring. Returns the length of the added string.
 * Note that this version appends the string to result instead of replacing
 * its previous content.
 */
int escape(const unichar* s,Ustring* result,const unichar* chars_to_escape) {
chars_to_escape = (chars_to_escape == NULL) ? STRING_EMPTY : chars_to_escape;
int n=result->len;
for (int i=0;s[i]!='\0';i++) {
   if (string_contains_unichar(chars_to_escape,s[i])) {
      u_strcat(result,PROTECTION_CHAR);
      u_strcat(result,s[i]);
   } else if (s[i]==PROTECTION_CHAR) {
      i++;
      u_strcat(result,PROTECTION_CHAR);
      u_strcat(result,s[i]);
   } else {
       u_strcat(result,s[i]);
   }
}
return result->len-n;
}


/**
 * This function performs a string copy where any protected char that
 * appears in chars_to_unprotect is unprotected.
 * The function returns the length of the resulting string.
 */
int unprotect(const unichar* s,unichar* result,const unichar* chars_to_unprotect) {
chars_to_unprotect = (chars_to_unprotect == NULL) ? STRING_EMPTY : chars_to_unprotect;
int j=0;
for (int i=0;s[i]!='\0';i++) {
    if (s[i]==PROTECTION_CHAR) {
        if (s[i+1]=='\0') {
            fatal_error("Unexpected %c at end of string in unprotect\n",PROTECTION_CHAR);
        }
        if (!string_contains_unichar(chars_to_unprotect,s[i+1])) {
            result[j++]=PROTECTION_CHAR;
        }
        result[j++]=s[i+1];
        i++;
    } else {
        result[j++]=s[i];
    }
}
result[j]='\0';
return j;
}


/**
 * This function performs a string copy where any protected char that
 * appears in chars_to_unprotect is unprotected.
 * The function returns the length of the resulting string.
 */
int unprotect(const unichar* s,Ustring* result,const unichar* chars_to_unprotect) {
chars_to_unprotect = (chars_to_unprotect == NULL) ? STRING_EMPTY : chars_to_unprotect;
int n=result->len;
for (int i=0;s[i]!='\0';i++) {
    if (s[i]==PROTECTION_CHAR) {
        if (s[i+1]=='\0') {
            fatal_error("Unexpected %c at end of string in unprotect\n",PROTECTION_CHAR);
        }
        if (!string_contains_unichar(chars_to_unprotect,s[i+1])) {
            u_strcat(result,PROTECTION_CHAR);
        }
        u_strcat(result,s[i+1]);
        i++;
    } else {
        u_strcat(result,s[i]);
    }
}
return result->len-n;
}

} // namespace unitex

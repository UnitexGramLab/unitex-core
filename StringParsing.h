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

#ifndef StringParsingH
#define StringParsingH

#include "Unicode.h"
#include "Ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Here are the error codes used for string parsing. Some of them like
 * P_UNEXPECTED_COMMENT are specially designed for errors in DELA lines.
 */
#define P_EOS -1 /* end of string */
#define P_OK 0
#define P_FORBIDDEN_CHAR 1
#define P_BACKSLASH_AT_END 2
#define P_NULL_STRING 3
#define P_UNEXPECTED_END_OF_LINE 4
#define P_UNEXPECTED_COMMENT 5
#define P_EMPTY_INFLECTED_FORM 6
#define P_EMPTY_SEMANTIC_CODE 7
#define P_EMPTY_INFLECTIONAL_CODE 8
#define P_EMPTY_LEMMA 9
#define P_EMPTY_FILTER 10
#define P_DUPLICATE_CHAR_IN_INFLECTIONAL_CODE 11
#define P_DUPLICATE_INFLECTIONAL_CODE 12
#define P_DUPLICATE_SEMANTIC_CODE 13
#define P_UNPROTECTED_DOT 14
#define P_UNPROTECTED_COMMA 15

#define PROTECTION_CHAR '\\'

/**
 * Here we define some separator sets that will be used
 * many times.
 */
#define P_EMPTY U_EMPTY
extern const unichar P_SPACE[] ;
extern const unichar P_TAB[] ;
extern const unichar P_COMMA[] ;
extern const unichar P_DOT[] ;
extern const unichar P_EQUAL[] ;
extern const unichar P_PLUS[] ;
extern const unichar P_COLON[] ;
extern const unichar P_SLASH[] ;
extern const unichar P_EXCLAMATION[] ;
extern const unichar P_DOUBLE_QUOTE[] ;
extern const unichar P_PLUS_COLON[] ;
extern const unichar P_PLUS_MINUS_COLON[] ;
extern const unichar P_PLUS_TILDE_COLON[] ;
extern const unichar P_PLUS_COLON_SLASH[] ;
extern const unichar P_PLUS_COLON_SLASH_OPENING_BRACKET[] ;
extern const unichar P_PLUS_COLON_SLASH_EXCLAMATION_OPENING_BRACKET[] ;
extern const unichar P_COLON_CLOSING_BRACKET[] ;
extern const unichar P_COLON_SLASH[] ;
extern const unichar P_CLOSING_ROUND_BRACKET[] ;
extern const unichar P_COMMA_DOT[] ;
extern const unichar P_PLUS_COLON_SLASH_BACKSLASH[] ;
extern const unichar P_PLUS_COMMA_COLON_SLASH_BACKSLASH[] ;
extern const unichar P_COLON_SLASH_BACKSLASH[] ;
extern const unichar P_COMMA_DOT_BACKSLASH_DIGITS[] ;
extern const unichar P_DOT_PLUS_SLASH_BACKSLASH[] ;
extern const unichar P_DOT_COMMA_PLUS_SLASH_BACKSLASH[] ;
extern const unichar P_ELAG_TAG[] ;
extern const unichar P_DIGITS[] ;
extern const unichar P_BACKSLASH_EQUAL[] ;
extern const unichar P_BACKSLASH[] ;
extern const unichar P_COMMA_DOT_EQUAL_BACKSLASH[] ;


int parse_string(const unichar* s,int *ptr,unichar* result,const unichar* stop_chars,
      const unichar* forbidden_chars,const unichar* chars_to_keep_protected);
int parse_string(const unichar* s,int *ptr,unichar* result,const unichar* stop_chars);
int parse_string(const unichar* s,int *ptr,unichar* result,const char* stop_chars);
int parse_string(const unichar* s,unichar* result,const unichar* stop_chars);
int parse_string(const unichar* s,unichar* result,const char* stop_chars);

int parse_string(const unichar* s,int *ptr,Ustring* result,const unichar* stop_chars,
      const unichar* forbidden_chars,const unichar* chars_to_keep_protected);
int parse_string(const unichar* s,int *ptr,Ustring* result,const unichar* stop_chars);
int parse_string(const unichar* s,int *ptr,Ustring* result,const char* stop_chars);
int parse_string(const unichar* s,Ustring* result,const unichar* stop_chars);
int parse_string(const unichar* s,Ustring* result,const char* stop_chars);

int escape(const unichar* s,unichar* result,const unichar* chars_to_escape);
int escape(const unichar* s,Ustring* result,const unichar* chars_to_escape);
int unprotect(const unichar* s,unichar* result,const unichar* chars_to_unprotect);
int unprotect(const unichar* s,Ustring* result,const unichar* chars_to_unprotect);

} // namespace unitex

#endif

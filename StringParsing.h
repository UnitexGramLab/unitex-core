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

#ifndef StringParsingH
#define StringParsingH

#include "Unicode.h"

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

#define PROTECTION_CHAR '\\'

/**
 * Here we define some separator sets that will be used
 * many times.
 */
#define P_EMPTY U_EMPTY
extern unichar* P_SPACE;
extern unichar* P_COMMA;
extern unichar* P_DOT;
extern unichar* P_EQUAL;
extern unichar* P_PLUS;
extern unichar* P_COLON;
extern unichar* P_SLASH;
extern unichar* P_EXCLAMATION;
extern unichar* P_DOUBLE_QUOTE;
extern unichar* P_PLUS_COLON;
extern unichar* P_PLUS_MINUS_COLON;
extern unichar* P_PLUS_COLON_SLASH;
extern unichar* P_PLUS_COLON_SLASH_OPENING_BRACKET;
extern unichar* P_PLUS_COLON_SLASH_EXCLAMATION_OPENING_BRACKET;
extern unichar* P_COLON_CLOSING_BRACKET;
extern unichar* P_COLON_SLASH;
extern unichar* P_CLOSING_ROUND_BRACKET;
extern unichar* P_COMMA_DOT;
extern unichar* P_PLUS_COLON_SLASH_BACKSLASH;
extern unichar* P_COLON_SLASH_BACKSLASH;
extern unichar* P_COMMA_DOT_BACKSLASH_DIGITS;
extern unichar* P_DOT_PLUS_SLASH_BACKSLASH;
extern unichar* P_ELAG_TAG;

int parse_string(unichar* s,int *ptr,unichar* result,unichar* stop_chars,
                 unichar* forbidden_chars,unichar* chars_to_keep_protected);

int parse_string(unichar* s,int *ptr,unichar* result,unichar* stop_chars);
int parse_string(unichar* s,int *ptr,unichar* result,char* stop_chars);

int parse_string(unichar* s,unichar* result,unichar* stop_chars);
int parse_string(unichar* s,unichar* result,char* stop_chars);

int escape(unichar*,unichar*,unichar*);

#endif

/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef RegExFacadeH
#define RegExFacadeH

#include "tre.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef TRE_WCHAR
#define REGEX_FACADE_ENGINE 1

#define REGEX_FACADE_REG_NOSUB REG_NOSUB
#define REGEX_FACADE_REG_EXTENDED REG_EXTENDED

// UNICHAR_REGEX_ALLOC_FACTOR is the maximal number of unichar_regex per unichar
// use 1 for unicode 16 bits, 4 for UTF8 by example
#define UNICHAR_REGEX_ALLOC_FACTOR (1)
typedef tre_char_t unichar_regex;
typedef regex_t regex_facade_regex_t;
typedef regmatch_t regex_regmatch_t ;

int regex_facade_regcomp(regex_facade_regex_t *preg, const unichar_regex *regex, int cflags);
size_t regex_facade_regerror(int errcode, const regex_facade_regex_t *preg, char *errbuf,
	 size_t errbuf_size);
void regex_facade_regfree(regex_facade_regex_t *preg);
int regex_facade_regexec(const regex_facade_regex_t *preg, const unichar_regex *string,
	 size_t nmatch, regex_regmatch_t pmatch[], int eflags);
#endif

#ifdef __cplusplus
}
#endif


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

unichar_regex* regex_facade_strcpy(unichar_regex* dest,const unichar* src);
unichar_regex* regex_facade_strncpy(unichar_regex *dest,const unichar *src,unsigned int n);
} // namespace unitex

#endif

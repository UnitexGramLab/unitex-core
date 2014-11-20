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


#include "Unicode.h"

 
#include "RegExFacade.h"

#ifdef TRE_WCHAR

int regex_facade_regcomp(regex_facade_regex_t *preg, const unichar_regex *regex, int cflags)
{
	return tre_regwcomp(preg, regex, cflags);
}

size_t regex_facade_regerror(int errcode, const regex_facade_regex_t *preg, char *errbuf,
	 size_t errbuf_size)
{
	return tre_regerror(errcode, preg, errbuf, errbuf_size);
}

void regex_facade_regfree(regex_facade_regex_t *preg)
{
	tre_regfree(preg);
}

int regex_facade_regexec(const regex_facade_regex_t *preg, const unichar_regex *string,
	 size_t nmatch, regex_regmatch_t pmatch[], int eflags)
{
	return tre_regwexec(preg, string, nmatch, pmatch, eflags);
}



#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * unichar_regex version of strcpy.
 * Copies a unichar* string into a unichar_regex* one.
 */
unichar_regex* regex_facade_strcpy(unichar_regex* dest,const unichar* src) {
unichar_regex *s = dest; // backup pointer to start of destination string
register unichar c;
do {
   c=*src++;
   *dest++=(unichar_regex)c;
} while (c!='\0');
return s;
}

/**
 * unichar_regex version of strncpy
 * Copies a unichar* string into a unichar_regex* one.
 */
unichar_regex* regex_facade_strncpy(unichar_regex *dest,const unichar *src,unsigned int n) {
register unichar c;
unichar_regex *s = dest; // backup pointer to start of destination string
do {
   c = *src++;
   *dest++ = (unichar_regex)c;
   if (--n == 0)
     return s;
} while (c != 0);
// null-padding
do
  *dest++ = 0;
while (--n > 0);
return s;
}

#define START_SIZE_DYNAMIC_BUFFER_REGEX 1024
void w_strcpy(unichar_regex** target,size_t *buffer_size,const unichar* source) {
int i=0;
unsigned int len = u_strlen(source);
if (len + 1 >= (int)(*buffer_size)) {
	if ((*buffer_size) < START_SIZE_DYNAMIC_BUFFER_REGEX) {
		*buffer_size = START_SIZE_DYNAMIC_BUFFER_REGEX;
	}
	while (len + 1 >= (int)(*buffer_size)) {
		(*buffer_size) *= 2;
	}

	*target = (unichar_regex*)(((*target) == NULL) ? malloc((*buffer_size) * sizeof(unichar_regex) * UNICHAR_REGEX_ALLOC_FACTOR)
		                      : realloc((*target), (*buffer_size) * sizeof(unichar_regex) * UNICHAR_REGEX_ALLOC_FACTOR));
	if (target==NULL) {
		fatal_alloc_error("w_strcpy");
	}
}
while (((*target)[i]=(unichar_regex)source[i])!= L'\0') i++;
}




void w_strcpy(unichar_regex* target,const unichar* source) {
int i=0;
while ((target[i]=(unichar_regex)source[i])!= L'\0') i++;
}



unichar_regex* w_strcpy_optional_buffer(unichar_regex * original_buffer, size_t original_buffer_size,
	unichar_regex**allocated_buffer, const unichar* add_string, size_t* len, Abstract_allocator prv_alloc)
{
	size_t add_string_len = u_strlen(add_string);
	size_t buffer_size_needed = add_string_len + 1;
	if (len != NULL)
		*len = add_string_len;

	if ((*allocated_buffer) == NULL)
	{
		if ((add_string_len + 1) < original_buffer_size)
		{
			w_strcpy(original_buffer, add_string);
			return original_buffer;
		}
		else
		{
			*allocated_buffer = (unichar_regex*)malloc_cb(buffer_size_needed * sizeof(unichar_regex) * UNICHAR_REGEX_ALLOC_FACTOR, prv_alloc);
			if ((*allocated_buffer) == NULL) {
				fatal_alloc_error("u_strcpy_optional_buffer");
			}

			w_strcpy((*allocated_buffer), add_string);
			return *allocated_buffer;
		}
	}
	else
	{
		free_cb(*allocated_buffer, prv_alloc);

		*allocated_buffer = (unichar_regex*)malloc_cb(buffer_size_needed * sizeof(unichar_regex) * UNICHAR_REGEX_ALLOC_FACTOR, prv_alloc);
		if ((*allocated_buffer) == NULL) {
			fatal_alloc_error("u_strcpy_optional_buffer");
		}

		w_strcpy((*allocated_buffer), add_string);
		return *allocated_buffer;
	}
}

void free_wstring_optional_buffer(unichar_regex** allocated_buffer, Abstract_allocator prv_alloc)
{
	if ((*allocated_buffer) != NULL) {
		free_cb(*allocated_buffer, prv_alloc);
		*allocated_buffer = NULL;
	}
}


} // namespace unitex

#endif

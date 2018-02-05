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


#include "Unicode.h"


#include "RegExFacade.h"


/*
UNITEX_FUNC int UNITEX_CALL RegexFacadeRegcomp(regex_facade_regex_t *preg, const unichar_regex *regex, int cflags)
{
    return regex_facade_regcomp(preg, regex, cflags);
}

UNITEX_FUNC size_t UNITEX_CALL RegexFacadeRegerror(int errcode, const regex_facade_regex_t *preg, char *errbuf,
    size_t errbuf_size)
{
    return regex_facade_regerror(errcode, preg, errbuf, errbuf_size);
}

UNITEX_FUNC void UNITEX_CALL RegexFacadeRegfree(regex_facade_regex_t *preg)
{
    regex_facade_regfree(preg);
}

UNITEX_FUNC int UNITEX_CALL RegexFacadeRegexec(const regex_facade_regex_t *preg, const unichar_regex *string,
    size_t nmatch, regex_regmatch_t pmatch[], int eflags)
{
    return regex_facade_regexec(preg, string,nmatch, pmatch, eflags);
}
*/


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
if (((int)(len + 1)) >= (int)(*buffer_size)) {
    if ((*buffer_size) < START_SIZE_DYNAMIC_BUFFER_REGEX) {
        *buffer_size = START_SIZE_DYNAMIC_BUFFER_REGEX;
    }
    while (((int)(len + 1)) >= (int)(*buffer_size)) {
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


static int test_tre_reg(const unichar* ureg, const unichar*usrch, int is_match_expecteded)
{
    regex_t matcher;


    unichar_regex* warray = (unichar_regex*)malloc(0x20 + (sizeof(unichar_regex)*((u_strlen(ureg) + 1) * UNICHAR_REGEX_ALLOC_FACTOR)));
    if (warray == NULL) {
        fatal_alloc_error("test_tre");
    }
    regex_facade_strcpy(warray, ureg);

    int ccode = regex_facade_regcomp(&matcher, warray, REG_NOSUB);
    if (ccode != 0)
    {
        error("cannot compile in regcomp\n");
        return 0;
    }

    unichar_regex* wusrch = (unichar_regex*)malloc(0x20 + (sizeof(unichar_regex)*((u_strlen(usrch) + 1)  * UNICHAR_REGEX_ALLOC_FACTOR)));
    if (wusrch == NULL) {
        fatal_alloc_error("test_tre");
    }
    regex_facade_strcpy(wusrch, usrch);


    int ret_regexec = regex_facade_regexec(&matcher, wusrch, 0, NULL, 0);
    int okay = (is_match_expecteded == (!ret_regexec));
    if (!okay) {
        error("unexpected result of %S on %S : %d - %d\n", ureg, usrch, ret_regexec, ccode);
    }

    regex_facade_regfree(&matcher);

    free(wusrch);
    free(warray);
    return okay;
}


static int internal_check_unitex()
{
    unichar ureg[0x200];
    unichar usrch[0x200];

    u_strcpy(ureg, "^Th[iI]s");
    u_strcpy(usrch, "This is a test");
    if (!test_tre_reg(ureg, usrch, 1)) {
        return 0;
    }

    u_strcpy(ureg, "^Th[iI]s");
    u_strcpy(usrch, "ThIs is a test");
    if (!test_tre_reg(ureg, usrch, 1)) {
        return 0;
    }

    u_strcpy(ureg, "^Th[iI]s");
    u_strcpy(usrch, "ThAs is a test");
    if (!test_tre_reg(ureg, usrch, 0)) {
        return 0;
    }

    const unichar regUnicode[] = { 'T', '[', 0x1ce, 'a', ']', 'u', '\0' };
    const unichar strUnicode0[] = { 'T', 0x2ce, 'u', '\0' };
    const unichar strUnicode1[] = { 'T', 0x1ce, 'u', '\0' };
    const unichar strUnicode2[] = { 'T', 0x1e1, 'u', '\0' };
    const unichar strUnicode3[] = { 'T', 'a', 'u', '\0' };
    const unichar strUnicode4[] = { 'T', 'u', '\0' };


    if ((!test_tre_reg(regUnicode, strUnicode0, 0)) ||
        (!test_tre_reg(regUnicode, strUnicode1, 1)) ||
        (!test_tre_reg(regUnicode, strUnicode2, 0)) ||
        (!test_tre_reg(regUnicode, strUnicode3, 1)) ||
        (!test_tre_reg(regUnicode, strUnicode4, 0)))
    {
        error("tre does not wotk correctly\n");
        return 0;
    }
    return 1;
}


} // namespace unitex

using namespace unitex;
int check_regex_lib_in_unitex()
{
    return internal_check_unitex();
}

UNITEX_FUNC int UNITEX_CALL CheckRegexLibInUnitex()
{
    return check_regex_lib_in_unitex();
}

#endif

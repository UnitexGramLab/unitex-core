/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS)
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */

#ifndef NO_UNITEX_LOGGER


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "Error.h"
#include "RunTools.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {

static int is_space_or_equivalent(char c)
{
    if ((c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'))
        return 1;
    return 0;
}



void do_convert_command_line_synth_to_splitted(
    const char*file_synth,
    size_t size_synth,
    char** ptr_converted,
    size_t *size_file_converted,
    char cEndArg, int firsts_line_contain_count,
    int* p_nb_args)
{
    char* dest = (char*)malloc((size_synth * 2) + 0x100);
    char begin[0x40];
    if (firsts_line_contain_count)
    {
        sprintf(begin, "%010d\n%010d\n", 0, 0);
    }
    else
    {
        begin[0] = '\0';
    }
    strcpy(dest, begin);
    char *cur_dest = dest + strlen(dest);

    const char* lpSrc = file_synth;
    const char* lpSrcLimit = file_synth + size_synth;
    int isInQuote = 0;
    int iNbArg = 0;


    while (is_space_or_equivalent(*lpSrc) != 0)
        lpSrc++;

    if (((*lpSrc) != '\0') && (lpSrc<lpSrcLimit))
        iNbArg++;

    while (((*lpSrc) != '\0') && (lpSrc<lpSrcLimit))
    {
        while (((*lpSrc) == '"') && (lpSrc<lpSrcLimit))
        {
            isInQuote = !isInQuote;
            lpSrc++;
        }

        if ((is_space_or_equivalent(*lpSrc) != 0) && (!isInQuote))
        {
            while ((is_space_or_equivalent(*lpSrc) != 0) && (lpSrc<lpSrcLimit))
                lpSrc++;
            if (((*lpSrc) == '\0') || (lpSrc == lpSrcLimit))
                break;
            *(cur_dest++) = cEndArg;
            iNbArg++;
        }

        while (((*lpSrc) == '"') && (lpSrc<lpSrcLimit))
        {
            isInQuote = !isInQuote;
            lpSrc++;
        }

        *(cur_dest++) = *(lpSrc++);
    }
    *(cur_dest) = '\0';
    *size_file_converted = (size_t)(cur_dest - dest);

    if (firsts_line_contain_count)
    {
        sprintf(begin, "%010d\n%010d\n", 0, iNbArg);
    }
    if (p_nb_args != NULL)
        *p_nb_args = iNbArg;
    memcpy(dest, begin, strlen(begin));
    *ptr_converted = dest;
}


void do_convert_command_line_synth_to_std(
    const char*file_synth,
    size_t size_synth,
    char** ptr_converted,
    size_t *size_file_converted
    )
{
    do_convert_command_line_synth_to_splitted(file_synth, size_synth, ptr_converted, size_file_converted, '\n', 1,NULL);
}


char** do_convert_command_line_synth_to_std_arg(const char*cmd_line_synth,
    size_t size_cmd_line_synth, int* param_argc)
{
    char* ptr_converted = NULL;
    size_t size_file_converted = 0;
    int iNbArgs = 0;
    do_convert_command_line_synth_to_splitted(cmd_line_synth, size_cmd_line_synth, &ptr_converted, &size_file_converted, '\0',0, &iNbArgs);

    size_t size_needed_ptr_list = sizeof(char*) * (iNbArgs + 3);

    const char* firstArg = "UnitexTool";
    size_t len_first_arg = strlen(firstArg);
    char** converted_list = (char**)malloc(size_needed_ptr_list + len_first_arg + 1 + size_file_converted + 1);
    char* command_text = ((char*)converted_list) + size_needed_ptr_list;
    strcpy(command_text, firstArg);
    memcpy(command_text + len_first_arg + 1, ptr_converted, size_file_converted + 1);
    free(ptr_converted);

    char * walk = command_text;
    for (int i = 0; i < (iNbArgs+1); i++)
    {
        *(converted_list + i) = walk;
        walk += strlen(walk) + 1;
    }
    *(converted_list + iNbArgs + 2) = NULL;
    *param_argc = iNbArgs + 1;
    return converted_list;
}

void free_std_arg_converted(char** ptr)
{
    free(ptr);
}


/**
 * This function build a string, be sure this string is always different for two differents value of uniquePr
 * and compile without warning on 32 & 64 bits
 */
void fill_unique_string_for_pointer(const void* uniquePtr, char* string)
{
    size_t value_for_pointer = (size_t)uniquePtr;
    size_t shift16 = (value_for_pointer >> 16);
    size_t shift32 = (shift16 >> 16);
    size_t low = (value_for_pointer & ((size_t)0xffffffff));
    sprintf(string, "%08lx", (unsigned long)shift32);
    sprintf(string+strlen(string), "%08lx", (unsigned long)low);
}

} // namespace logger
} // namespace unitex

#endif

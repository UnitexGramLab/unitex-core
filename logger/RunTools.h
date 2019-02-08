/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * additional information: http://www.smartversion.com/unitex-contribution/
 * contact : info@winimage.com
 *
 */



#ifndef NO_UNITEX_LOGGER



#ifndef _RUN_TOOLS_H
#define _RUN_TOOLS_H 1

#include "AbstractCallbackFuncModifier.h"

#ifdef __cplusplus
#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

    namespace logger {
        //extern "C" {
#endif

            void do_convert_command_line_synth_to_std(
                const char*file_synth,
                size_t size_synth,
                char** ptr_converted,
                size_t *size_file_converted);

            char** do_convert_command_line_synth_to_std_arg(const char*cmd_line_synth,
                size_t size_cmd_line_synth, int* param_argc);

            void free_std_arg_converted(char** ptr);

// fill_unique_string_for_pointer needed size (zero terminal included) : (0x10+1) with margin so 0x20
#define UNIQUE_STRING_FOR_POINTER_MAX_SIZE 0x20
            void fill_unique_string_for_pointer(const void* uniquePtr, char* unique_string);

#ifdef __cplusplus
        //} // extern "C"
    } // namespace logger
} // namespace unitex
#endif






#endif

#endif

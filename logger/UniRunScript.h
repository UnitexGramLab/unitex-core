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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS)
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.smartversion.com/unitex-contribution/
 * contact : info@winimage.com
 *
 */






#if ((!(defined(NO_UNITEX_LOGGER))) && (!(defined(NO_UNITEX_RUNLOGGER_AUTOINSTALL))))

#ifndef _UNI_RUN_SCRIPT_H_INCLUDED
#define _UNI_RUN_SCRIPT_H_INCLUDED 1

#include "UnitexGetOpt.h"


#ifdef __cplusplus
#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#endif



#ifdef __cplusplus
    extern "C" {
#endif

        // function export api


#ifdef __cplusplus
    } // extern "C"
#endif




    extern const char* optstring_UniRunScript;
    extern const struct option_TS lopts_UniRunScript[];
    extern const char* usage_UniRunScript;

    int main_UniRunScript(int argc, char* const argv[]);


    extern const char* optstring_UniBatchRunScript;
    extern const struct option_TS lopts_UniBatchRunScript[];
    extern const char* usage_UniBatchRunScript;

    int main_UniBatchRunScript(int argc, char* const argv[]);

    
    extern const char* optstring_UniBatchFieldRunScript;
    extern const struct option_TS lopts_UniBatchFieldRunScript[];
    extern const char* usage_UniBatchFieldRunScript;

    int main_UniBatchFieldRunScript(int argc, char* const argv[]);

#ifdef __cplusplus
    //        } // extern "C"
    //    } // namespace logger
} // namespace unitex
#endif

#endif

#endif



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




#ifndef NO_UNITEX_LOGGER


#ifndef _UNI_LOGGER_H_INCLUDED
#define _UNI_LOGGER_H_INCLUDED 1


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
extern "C" {
#endif

/* */


struct UniLoggerSpace {
    int size_of_struct;
    void* privateUnloggerData;

    int store_file_out_content;
    int store_list_file_out_content;

    int store_file_in_content;
    int store_list_file_in_content;

    int store_std_out_content;
    int store_std_err_content;

    int auto_increment_logfilename;

    const char* szPathLog;
    const char* szNameLog;
} ;

static const struct UniLoggerSpace ule_default_init = { sizeof(struct UniLoggerSpace),NULL,0,1,1,1,1,1,0,NULL,NULL };

UNITEX_FUNC int UNITEX_CALL AddActivityLogger(struct UniLoggerSpace *p_ule);
UNITEX_FUNC int UNITEX_CALL RemoveActivityLogger(struct UniLoggerSpace *p_ule);

UNITEX_FUNC int UNITEX_CALL SelectNextLogName(struct UniLoggerSpace *p_ule,const char* LogName,const char* portion_ignore_pathname);


#ifdef __cplusplus
} // extern "C"
} // namespace logger
} // namespace unitex
#endif

#endif

#endif

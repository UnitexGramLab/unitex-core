 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


#ifndef NO_UNITEX_LOGGER


#ifndef _UNI_LOGGER_H_INCLUDED
#define _UNI_LOGGER_H_INCLUDED 1


#include "AbstractCallbackFuncModifier.h"

#ifdef __cplusplus
extern "C" {
#endif

/* */


struct UniLoggerSpace {
    void* privateUnloggerData;

    int store_file_out_content;
    int store_list_file_out_content;

    int store_file_in_content;
    int store_list_file_in_content;

    int auto_increment_logfilename;

    char* szPathLog;
} ;

UNITEX_FUNC int UNITEX_CALL AddActivityLogger(struct UniLoggerSpace *p_ule);
UNITEX_FUNC int UNITEX_CALL RemoveActivityLogger(struct UniLoggerSpace *p_ule);

UNITEX_FUNC int UNITEX_CALL SelectNextLogName(struct UniLoggerSpace *p_ule,const char* LogName,const char* portion_ignore_pathname);


#ifdef __cplusplus
}
#endif

#endif

#endif

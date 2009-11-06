 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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



#ifndef _UNI_RUN_LOGGER_H_INCLUDED
#define _UNI_RUN_LOGGER_H_INCLUDED 1


#include "AbstractCallbackFuncModifier.h"

#ifdef __cplusplus
extern "C" {
#endif

/* */

typedef enum {
    EXEC_NOTRUN,
    EXEC_NOTRUN_UNWANTEDTOOL,
    EXEC_COMPARE_ERROR,
    EXEC_COMPARE_WARNING,
    EXEC_COMPARE_OK
} Exec_status;


UNITEX_FUNC int UNITEX_CALL RunLogParam(const char* LogNameRead,const char* FileRunPath,const char* LogNameWrite,
                                        const char* SelectTool,
                                        int clean_file,
                                        int real_content_in_log,
                                        const char* /* LocationUnfoundVirtualRessource */,
                                        char** summaryInfo,
                                        char** summaryInfoErrorOnly,
                                        int *pReturn,unsigned int*pTimeElapsed,
                                        Exec_status* p_exec_status);

UNITEX_FUNC int UNITEX_CALL RunLog(const char* LogNameRead,const char* FileRunPath,const char* LogNameWrite);




extern const char* optstring_RunLog;
extern const struct option_TS lopts_RunLog[];
extern const char* usage_RunLog;

int main_RunLog(int argc,char* argv[]);


#ifdef __cplusplus
}
#endif

#endif

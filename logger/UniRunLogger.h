/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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




#if ((!(defined(NO_UNITEX_LOGGER))) && (!(defined(NO_UNITEX_RUNLOGGER_AUTOINSTALL))))

#ifndef _UNI_RUN_LOGGER_H_INCLUDED
#define _UNI_RUN_LOGGER_H_INCLUDED 1


#include "AbstractCallbackFuncModifier.h"
#include "UnitexTool.h"

#ifdef __cplusplus
namespace unitex {
namespace logger {
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
                                        int benchmark,
                                        int *pReturn,unsigned int*pTimeElapsed,
                                        Exec_status* p_exec_status);

UNITEX_FUNC int UNITEX_CALL RunLog(const char* LogNameRead,const char* FileRunPath,const char* LogNameWrite);

UNITEX_FUNC int UNITEX_CALL GetRunLogInfo(mainFunc** pfunc,const char** usage,const char** optstring,const struct option_TS **lopts);

UNITEX_FUNC int UNITEX_CALL RunLog_run_main(int argc,char* const argv[]);


extern const char* optstring_RunLog;
extern const struct option_TS lopts_RunLog[];
extern const char* usage_RunLog;

int main_RunLog(int argc,char* const argv[]);


#ifdef __cplusplus
} // extern "C"
} // namespace logger
} // namespace unitex
#endif

#endif

#endif

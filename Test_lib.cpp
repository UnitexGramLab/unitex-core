/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdio.h>
#include <stdlib.h>
#include "IOBuffer.h"
#include "Unicode.h"
#include "Reg2Grf.h"
#include "Locate.h"
#include "ProgramInvoker.h"
#include "RegExFacade.h"
#include "UnusedParameter.h"
#include "UnitexTool.h"
#include "logger/UniLogger.h"
#include "logger/UniRunLogger.h"
#include "logger/UniLoggerAutoInstall.h"
#include "UnitexLibIO.h"
#include "IOBuffer.h"

#ifdef HAS_UNITEX_NAMESPACE
using namespace unitex;
#endif

#ifdef HAS_LOGGER_NAMESPACE
using namespace logger;
#endif

/**
 * This program is an example of compilation using the unitex library (unitex.dll/libunitex.so).
 * It prints the .grf file corresponding to "a+(b.c)".
 */

#ifdef TESTLIB_MAIN_AS_TESTLIB
int testlib_main(int argc,char *argv[])
#else
int main(int argc,char *argv[])
#endif
{
SetUnitexBufferMode();
int retValue = 0;
INSTALLLOGGER pInstallLogger = NULL;

if (argc>=3) {
    int skip_arg = 0;
    if ((strcmp(argv[1], "{") == 0) && (strcmp(argv[2], "CreateLog") == 0))
    {
        for (int i = 2;i<argc;i++) {
            if (strcmp(argv[i], "}") == 0) {
                skip_arg = i;
                break;
            }
        }
    }

    if (skip_arg == 0) {
        pInstallLogger = BuildLogger();
    }
    else {
        pInstallLogger = BuildLoggerFromArgs(skip_arg - 2, argv + 2);
    }
    argc -= skip_arg;
    argv += skip_arg;
}


if (argc>1) {
  if (strcmp(argv[1],"RunLog")==0) {
    int ret_value = RunLog_run_main(argc-1,argv+1);
    if (pInstallLogger != NULL) {
        RemoveLoggerFromParamFile(pInstallLogger);
    }
    return ret_value;
  }
  if (strcmp(argv[1],"UnitexTool")==0) {
    int ret_value = UnitexTool_public_run(argc-1,argv+1,NULL,NULL);
    if (pInstallLogger != NULL) {
        RemoveLoggerFromParamFile(pInstallLogger);
    }
    return ret_value;
  }
  if (strcmp(argv[1], "{") == 0) {
    int ret_value = UnitexTool_public_run(argc, argv, NULL, NULL);
    if (pInstallLogger != NULL) {
        RemoveLoggerFromParamFile(pInstallLogger);
    }
    return ret_value;
  }
  if (UnitexTool_public_GetToolInfo_byname(argv[1], NULL, NULL, NULL, NULL) != -1) {
    int ret_value = UnitexTool_public_run(argc, argv, NULL, NULL);
    if (pInstallLogger != NULL) {
        RemoveLoggerFromParamFile(pInstallLogger);
    }
    return ret_value;
  }
}

const char* argv_VersionInfo[] = { "UnitexTool", "VersionInfo", NULL };
UnitexTool_public_run(2, (char**)argv_VersionInfo, NULL, NULL);

UnitexTool_public_run_string("UnitexTool VersionInfo -m -\"r\" -p");

/* These lines are just here to test if the TRE library was correctly linked. */
if (CheckRegexLibInUnitex()) {
    puts("Regex Library is functionnal.");
} else {
    puts("Regex Library is NOT functionnal.");
    retValue = 1;
}


const char* name="biniou";
const char* content = "a+(b.c)";
// write UTF8 file with BOM
if (WriteUnitexFile(name, NULL, 0, content, strlen(content)) != 0) {
    puts("cannot create test file");
}



const char* argv_Reg2Grf[] = { "UnitexTool", "Reg2Grf", name, "-q","UTF8_NO_BOM",NULL };
int reg_grf = UnitexTool_public_run(5, (char**)argv_Reg2Grf, NULL, NULL);

const char* grf="regexp.grf";
char message[0x100];
sprintf(message, "Reg2Grf exit code: %d\n\n", reg_grf);
puts(message);

UNITEXFILEMAPPED* umf = NULL;
const void* buffer = NULL;
size_t size_file = 0;
GetUnitexFileReadBuffer(grf, &umf, &buffer, &size_file);

if (umf != NULL) {
    for (size_t i = 0; i < size_file; i++) {
        int c = *(((unsigned char*)buffer) + i);
        putchar(c);
    }
    CloseUnitexFileReadBuffer(umf, buffer, size_file);
}

RemoveUnitexFile(name);
RemoveUnitexFile(grf);

if (pInstallLogger != NULL) {
    RemoveLoggerFromParamFile(pInstallLogger);
}
return retValue;
}

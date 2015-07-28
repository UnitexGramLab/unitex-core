/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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




#include "IOBuffer.h"
#include "UnitexTool.h"
#include "Copyright.h"


#include "ActivityLogger.h"
#include "ActivityLoggerPlugCallback.h"

#include "logger/UniLogger.h"
#include "logger/UniRunLogger.h"
#include "logger/UniLoggerAutoInstall.h"

#include "logger/MzRepairUlp.h"

#include "logger/UnpackFile.h"
#include "logger/PackFile.h"

#include "logger/InstallLingResourcePackage.h"

#include <stdlib.h>
#include <string.h>

using namespace unitex;
using namespace unitex::logger;

const char*	usage_RunLog_mini = "Usage: UnitexToolLogger RunLog [OPTIONS] <ulp>\n"
                                "or \n"
                                "Usage: UnitexToolLogger InstallLingResourcePackage [OPTIONS]\n"
                                "or \n"
                                "Usage: UnitexToolLogger MzRepairUlp [OPTIONS] <ulpfile>\n"
                                "or \n"
                                "Usage: UnitexToolLogger { CreateLog [OPTIONS] } <Utility> [OPTIONS]\n"
                                "or \n";


static void disp_usage_RunLog_mini() {
  display_copyright_notice();
  u_printf(usage_RunLog_mini);
}

int main(int argc,char* argv[]) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

if (argc <= 1) {
    disp_usage_RunLog_mini();
    unitex_tool_usage(1,0);
    return SUCCESS_RETURN_CODE;
}

int return_value=SUCCESS_RETURN_CODE;
int done=0;
int skip_arg=0;

InstallLogger* pInstallLogger=NULL;

if (argc>3) {
    if ((strcmp(argv[1],"{")==0) && (strcmp(argv[2],"CreateLog")==0))
    {
        for (int i=2;i<argc;i++) {
            if (strcmp(argv[i],"}") == 0) {
                skip_arg=i;
                break;
            }
        }
    }
    
    if (skip_arg == 0) {
        pInstallLogger = new InstallLogger();
    }
    else {
        pInstallLogger = new InstallLogger(skip_arg-2,argv+2);
    }

}

if ((argc-skip_arg)>1) {
    if (strcmp(argv[1+skip_arg],"UnpackFile")==0)
    {
        done = 1;
        return_value = main_UnpackFile(argc-(skip_arg+1),argv+skip_arg+1);
    }

 
    if (strcmp(argv[1+skip_arg],"PackFile")==0)
    {
        done = 1;
        return_value = main_PackFile(argc-(skip_arg+1),argv+skip_arg+1);
    }

 
    if (strcmp(argv[1+skip_arg],"InstallLingResourcePackage")==0)
    {
        done = 1;
        return_value = main_InstallLingResourcePackage(argc - (skip_arg + 1), argv + skip_arg + 1);
    }

    if (strcmp(argv[1+skip_arg],"MzRepairUlp")==0)
    {
        done = 1;
        return_value = main_MzRepairUlp(argc-(skip_arg+1),argv+skip_arg+1);
    }


    if (strcmp(argv[1+skip_arg],"RunLog")==0)
    {
        done = 1;
        return_value = main_RunLog(argc-(skip_arg+1),argv+skip_arg+1);
    }
}

if (((argc-skip_arg)>3) && (done == 0)) {
    if (strcmp(argv[1+skip_arg],"RunUnitexLog")==0)
    {
        return_value = RunLog(argv[2+skip_arg],argv[3+skip_arg],((argc-skip_arg) > 4) ? argv[4+skip_arg] : NULL);
        done = 1;
    }
}

if (done == 0) {
  return_value = main_UnitexTool(argc-skip_arg,argv+skip_arg);
}

if (pInstallLogger != NULL) {
    delete(pInstallLogger);
}
return return_value;
}

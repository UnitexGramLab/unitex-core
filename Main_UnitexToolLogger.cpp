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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS) in the framework 
 * of UNITEX optimization and UNITEX industrialization / reliability
 *
 * More information : http://www.ergonotics.com/unitex-contribution/
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

#include <stdlib.h>
#include <string.h>


const char*	usage_RunLog_mini = "Usage: UnitexToolLogger RunLog [OPTIONS] <ulp>\n"
                                "or \n"
                                "Usage: UnitexToolLogger { CreateLog [OPTIONS] } <Utility> [OPTIONS]\n"
                                "or \n";
static void disp_usage_RunLog_mini() {
u_printf("%S",COPYRIGHT);
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
    return 0;
}

int ret=0;
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
    if (strcmp(argv[1+skip_arg],"RunLog")==0)
    {
        done = 1;
        ret = main_RunLog(argc-(skip_arg+1),argv+skip_arg+1);
    }
}

if (((argc-skip_arg)>3) && (done == 0)) {
    if (strcmp(argv[1+skip_arg],"RunUnitexLog")==0)
    {
        ret = RunLog(argv[2+skip_arg],argv[3+skip_arg],((argc-skip_arg) > 4) ? argv[4+skip_arg] : NULL);
        done = 1;
    }
}

if (done == 0) {
  ret = main_UnitexTool(argc-skip_arg,argv+skip_arg);
}

if (pInstallLogger != NULL)
    delete(pInstallLogger);
return ret;
}

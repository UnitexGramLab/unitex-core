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

#include "IOBuffer.h"
#include "UnitexTool.h"
#include "Copyright.h"
#include "logger/UniRunLogger.h"


#include <stdlib.h>
#include <string.h>


const char*	usage_RunLog_mini = "Usage: UnitexToolLogger RunLog\n"
                                "or \n";;

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

if (argc>1) {
    if (strcmp(argv[1],"RunLog")==0)
    {
        return main_RunLog(argc-1,argv+1);
    }
}

if (argc>3) {
    if (strcmp(argv[1],"RunUnitexLog")==0)
    {
        return RunLog(argv[2],argv[3],(argc > 4) ? argv[4] : NULL);
    }
}

return main_UnitexTool(argc,argv);
}

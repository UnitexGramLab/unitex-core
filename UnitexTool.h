/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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



#ifndef UnitexToolH
#define UnitexToolH

#include "UnitexGetOpt.h"
#include "AbstractCallbackFuncModifier.h"

#define _UNITEX_VER 0210

#ifdef __cplusplus
extern "C" {
#endif

void unitex_tool_usage(int several, int display_copyright);

int check_Utility(const char* name);

int main_UnitexTool_single(int argc,char* const argv[]);


struct pos_tools_in_arg {
	int tool_number;
	int argcpos;
	int nbargs;
	int ret;
} ;

int UnitexTool_several_info(int argc,char* const argv[],int* p_number_done,struct pos_tools_in_arg* ptia);
int UnitexTool_several(int argc,char* const argv[],int* p_number_done);
int main_UnitexTool(int argc,char* const argv[]);


typedef int mainFunc(int argc,char* const argv[]) ;


int GetNumberOfTool();
int GetToolInfo_byname(const char* toolname,mainFunc** pfunc,const char** usage,const char** optstring,const struct option_TS **lopts);
int GetToolInfo_bynumber(int toolnumber,const char**toolname,mainFunc** pfunc,const char** usage,const char** optstring,const struct option_TS **lopts);


int main_UnitexTool_C(int argc,char* const argv[]);


UNITEX_FUNC int UNITEX_CALL UnitexTool_public_run(int argc,char* const argv[],int* p_number_done,struct pos_tools_in_arg* ptia);
UNITEX_FUNC int UNITEX_CALL UnitexTool_public_run_one_tool(const char*toolname,int argc,char* const argv[]);

UNITEX_FUNC int UNITEX_CALL UnitexTool_public_GetNumberOfTool();
UNITEX_FUNC int UNITEX_CALL UnitexTool_public_GetToolInfo_byname(const char* toolname,mainFunc** pfunc,const char** usage,const char** optstring,const struct option_TS **lopts);
UNITEX_FUNC int UNITEX_CALL UnitexTool_public_GetToolInfo_bynumber(int toolnumber,const char**toolname,mainFunc** pfunc,const char** usage,const char** optstring,const struct option_TS **lopts);

#ifdef __cplusplus
}
#endif


#endif


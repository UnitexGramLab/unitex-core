 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include <stdlib.h>
#include <string.h>

#include "Unicode.h"

#include "CheckDic.h"
#include "Compress.h"
#include "Concord.h"
#include "ConcorDiff.h"
#include "Convert.h"
#include "Dico.h"
#include "DuplicateFile.h"
#include "Elag.h"
#include "ElagComp.h"
#include "Evamb.h"
#include "Extract.h"
#include "Flatten.h"
#include "Fst2Chk.h"
#include "Fst2List.h"
#include "Fst2Txt.h"
#include "Grf2Fst2.h"
#include "ImplodeTfst.h"
#include "Locate.h"
#include "LocateTfst.h"
#include "MultiFlex.h"
#include "Normalize.h"
#include "PolyLex.h"
#include "RebuildTfst.h"
#include "Reconstrucao.h"
#include "Reg2Grf.h"
#include "SortTxt.h"
#include "Stats.h"
#include "Table2Grf.h"
#include "TagsetNormTfst.h"
#include "TEI2Txt.h"
#include "Tfst2Grf.h"
#include "Tfst2Unambig.h"
#include "Tokenize.h"
#include "Txt2Tfst.h"
#include "Uncompress.h"
#include "XMLizer.h"

#include "Copyright.h"

#include "UnitexTool.h"
#include "ActivityLogger.h"


#include "UserCancelling.h"

struct utility_item {
	const char* name;
	int len_name;
	mainFunc* fnc;

	const char* usage;
	const char* optstring;
	const struct option_TS *lopts;
} ;

#ifndef UNITEX_NO_KOREAN_TOOL
#include "CompressKr.h"
#include "ConsultDic.h"
#include "ExtractChar.h"
#include "InflectKr.h"
#include "Jamo2Syl.h"
#include "MergeBin.h"
#include "SortMorph.h"
#include "SufForm2Rac.h"
#include "Syl2Jamo.h"
#include "Txt2Fst2Kr.h"
#endif

const struct utility_item utility_array[]=
{
	{ "CheckDic", 8, &main_CheckDic, usage_CheckDic, optstring_CheckDic, lopts_CheckDic } ,
	{ "Compress", 8, &main_Compress, usage_Compress, optstring_Compress, lopts_Compress } ,
	{ "Concord", 7, &main_Concord, usage_Concord, optstring_Concord, lopts_Concord } ,
#if ((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))))
	{ "ConcorDiff", 10, &main_ConcorDiff, usage_ConcorDiff, optstring_ConcorDiff, lopts_ConcorDiff } ,
#endif
#if ((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))))
	{ "Convert", 7, &main_Convert, usage_Convert, optstring_Convert, lopts_Convert } ,
#endif
	{ "Dico", 4, &main_Dico, usage_Dico, optstring_Dico, lopts_Dico } ,
#if ((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))))
	{ "DuplicateFile", 13, &main_DuplicateFile, usage_DuplicateFile, optstring_DuplicateFile, lopts_DuplicateFile } ,
	{ "Elag", 4, &main_Elag, usage_Elag, optstring_Elag, lopts_Elag } ,
	{ "ElagComp", 8, &main_ElagComp, usage_ElagComp, optstring_ElagComp, lopts_ElagComp } ,
	{ "Evamb", 5, &main_Evamb, usage_Evamb, optstring_Evamb, lopts_Evamb } ,
#endif
#if ((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))))
	{ "Extract", 7, &main_Extract, usage_Extract, optstring_Extract, lopts_Extract } ,
#endif
#if ((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))))
	{ "Flatten", 7, &main_Flatten, usage_Flatten, optstring_Flatten, lopts_Flatten } ,
	{ "Fst2Check", 9, &main_Fst2Check, usage_Fst2Check, optstring_Fst2Check, lopts_Fst2Check } ,
	{ "Fst2List", 8, &main_Fst2List, usage_Fst2List, NULL, NULL } ,
#endif
	{ "Fst2Txt", 7, &main_Fst2Txt, usage_Fst2Txt, optstring_Fst2Txt, lopts_Fst2Txt } ,
#if ((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))))
	{ "Grf2Fst2", 8, &main_Grf2Fst2, usage_Grf2Fst2, optstring_Grf2Fst2, lopts_Grf2Fst2 } ,
	{ "ImplodeTfst", 11, &main_ImplodeTfst, usage_ImplodeTfst, optstring_ImplodeTfst, lopts_ImplodeTfst } ,
#endif
	{ "Locate", 6, &main_Locate, usage_Locate, optstring_Locate, lopts_Locate } ,
#if ((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))))
	{ "LocateTfst", 10, &main_LocateTfst, usage_LocateTfst, optstring_LocateTfst, lopts_LocateTfst } ,
#endif
#if ((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))))
	{ "MultiFlex", 9, &main_MultiFlex, usage_MultiFlex, optstring_MultiFlex, lopts_MultiFlex } ,
#endif
	{ "Normalize", 9, &main_Normalize, usage_Normalize, optstring_Normalize, lopts_Normalize } ,
#if ((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))))
	{ "PolyLex", 7, &main_PolyLex, usage_PolyLex, optstring_PolyLex, lopts_PolyLex } ,
	{ "RebuildTfst", 11, &main_RebuildTfst, usage_RebuildTfst, optstring_RebuildTfst, lopts_RebuildTfst } ,
	{ "Reconstrucao", 12, &main_Reconstrucao, usage_Reconstrucao, optstring_Reconstrucao, lopts_Reconstrucao } ,
	{ "Reg2Grf", 7, &main_Reg2Grf, usage_Reg2Grf, optstring_Reg2Grf, lopts_Reg2Grf } ,
#endif
	{ "SortTxt", 7, &main_SortTxt, usage_SortTxt, optstring_SortTxt, lopts_SortTxt } ,
#if ((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))))
	{ "Stats", 5, &main_Stats, usage_Stats, optstring_Stats, lopts_Stats } ,
	{ "Table2Grf", 9, &main_Table2Grf, usage_Table2Grf, optstring_Table2Grf, lopts_Table2Grf } ,
	{ "TagsetNormTfst", 14, &main_TagsetNormTfst, usage_TagsetNormTfst, optstring_TagsetNormTfst, lopts_TagsetNormTfst } ,
	{ "TEI2Txt", 7, &main_TEI2Txt, usage_TEI2Txt, optstring_TEI2Txt, lopts_TEI2Txt } ,
	{ "Tfst2Grf", 8, &main_Tfst2Grf, usage_Tfst2Grf, optstring_Tfst2Grf, lopts_Tfst2Grf } ,
	{ "Tfst2Unambig", 12, &main_Tfst2Unambig, usage_Tfst2Unambig, optstring_Tfst2Unambig, lopts_Tfst2Unambig } ,
#endif
	{ "Tokenize", 8, &main_Tokenize, usage_Tokenize, optstring_Tokenize, lopts_Tokenize } ,
#if ((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))))
	{ "Txt2Tfst", 8, &main_Txt2Tfst, usage_Txt2Tfst, optstring_Txt2Tfst, lopts_Txt2Tfst } ,
	{ "Uncompress", 10, &main_Uncompress, usage_Uncompress, optstring_Uncompress, lopts_Uncompress } ,
	{ "XMLizer", 7, &main_XMLizer, usage_XMLizer, optstring_XMLizer, lopts_XMLizer } ,
#endif

#ifndef UNITEX_NO_KOREAN_TOOL
	{ "CompressKr" ,10, &main_CompressKr, usage_CompressKr, NULL, NULL } ,
	{ "ConsultDic" ,10, &main_ConsultDic, usage_ConsultDic, NULL, NULL } ,
	{ "ExtractChar" ,11, &main_ExtractChar, usage_ExtractChar, NULL, NULL } ,
	{ "InflectKr" ,9, &main_InflectKr, usage_InflectKr, NULL, NULL } ,
	{ "Jamo2Syl" ,8, &main_Jamo2Syl, usage_Jamo2Syl, NULL, NULL } ,
	{ "MergeBin" ,8, &main_MergeBin, usage_MergeBin, NULL, NULL } ,
	{ "SortMorph" ,9, &main_SortMorph, usage_SortMorph, NULL, NULL } ,
	{ "SufForm2Rac" ,11, &main_SufForm2Rac, usage_SufForm2Rac, NULL, NULL } ,
	{ "Syl2Jamo" ,8, &main_Syl2Jamo, usage_Syl2Jamo, NULL, NULL } ,
	{ "Txt2Fst2Kr" ,10, &main_Txt2Fst2Kr, usage_Txt2Fst2Kr, NULL, NULL } ,
#endif
	{ "", 0, NULL, NULL, NULL, NULL} 
};

const struct utility_item* found_utility(const char* search)
{
	int len_search = (int)strlen(search);
	int i=0;
	while (utility_array[i].len_name > 0)
	{
		if (utility_array[i].len_name == len_search)
			if (strcmp(utility_array[i].name,search)==0)
				return &utility_array[i];
		i++;
	}
	return NULL;
}


int GetToolInfo_byname(const char* toolname,mainFunc** pfunc,const char** usage,const char** optstring,const struct option_TS **lopts)
{
const struct utility_item* utility_called = found_utility(toolname);
if (utility_called == NULL)
  return -1;
else {
	if (usage != NULL) *usage=utility_called->usage;
	if (optstring != NULL) *optstring = utility_called->optstring;
	if (lopts != NULL) *lopts = utility_called->lopts;
    if (pfunc != NULL) *pfunc = utility_called->fnc;
  return 0;
}
}

int GetToolInfo_bynumber(int toolnumber,const char**toolname,mainFunc** pfunc,const char** usage,const char** optstring,const struct option_TS **lopts)
{
const struct utility_item* utility_called = &(utility_array[toolnumber]);
{
	if (toolname != NULL) *toolname = utility_called->name;
	if (usage != NULL) *usage=utility_called->usage;
	if (optstring != NULL) *optstring = utility_called->optstring;
	if (lopts != NULL) *lopts = utility_called->lopts;
    if (pfunc != NULL) *pfunc = utility_called->fnc;
  return 0;
}
}

int GetNumberOfTool()
{
	int i=0;
	while (utility_array[i].len_name > 0)
	{		
		i++;
	}
	return i;
}



void list_unused_option_letter()
{
    int optstring_lt[32];
    int lopts[32];
    int i;

    for (i=0;i<32;i++)
        optstring_lt[i]=lopts[i]=0;

    i=0;
    while (utility_array[i].len_name > 0)
	{
		const char* optstring_browse = utility_array[i].optstring;
        if (optstring_browse!=NULL)
            while ((*optstring_browse)!=0)
            {
                const char c=*optstring_browse;
                if ((c>='a') && (c<='z'))
                    optstring_lt[c-'a']=1;

                optstring_browse++;
            }

        int j=0;
        if (utility_array[i].lopts != NULL)
            while (utility_array[i].lopts[j].name!=NULL)
            {
                int val = utility_array[i].lopts[j].val;
                if ((val>='a') && (val<='z'))
                    lopts[val-'a']=1;
                j++;
            }
		i++;
	}

    u_printf("unused letter for optsting : ");
    for (i=0;i<26;i++)
        if (optstring_lt[i]==0)
            u_printf("%c",i+'a');
    u_printf("\n\n");

    u_printf("unused letter for lopts : ");
    for (i=0;i<26;i++)
        if (lopts[i]==0)
            u_printf("%c",i+'a');
    u_printf("\n\n");

}

const char* usage_UnitexTool_prefix =
    "Usage: UnitexTool <Utility> [OPTIONS]\n" 
		          "where OPTIONS can be -h/--help to display help\n"
			       "and Utility is from this list :\n";

void unitex_tool_usage(int several, int display_copyright)
{
	int i=0;
    if (display_copyright != 0)
	  u_printf("%S",COPYRIGHT);
	u_printf(usage_UnitexTool_prefix);
	while (utility_array[i].len_name > 0)
	{
		u_printf("%s\n",utility_array[i].name);
		i++;
	}

	if (several != 0)
		u_printf(
		   "\n"
		   "You can chain several utility call by using\n"
		   "UnitexTool { <Utility> [OPTIONS] } { <Utility> [OPTIONS] } ...\n");
    //list_unused_option_letter();
}

int check_Utility(const char* name)
{
const struct utility_item* utility_called = found_utility(name);
if (utility_called != NULL)
  return 1;
else
  return 0;
}

static int CallToolLogged(mainFunc* fnc,int argc,char* argv[])
{
    int ret;
    Call_logger_fnc_before_calling_tool(fnc,argc,argv);
    if (is_cancelling_requested() != 0)
        ret = 0;
    else
        ret = (*fnc)(argc,argv);
    Call_logger_fnc_after_calling_tool(fnc,argc,argv,ret);
    return ret;
}

int main_UnitexTool_single(int argc,char* argv[]) {
const struct utility_item* utility_called = NULL;
if (argc>1)
  utility_called = found_utility(argv[1]);
if (utility_called==NULL) {
   unitex_tool_usage(0,1);
   return 0;
}

return CallToolLogged(utility_called->fnc,argc-1,((char**)argv)+1);
}


int UnitexTool_several_info(int argc,char* argv[],int* p_number_done,struct pos_tools_in_arg* ptia)
{
	int ret=0;
	int number_done_dummy=0;
	int pos = 1;
	int next_num_util = 0;
	pos_tools_in_arg tia_dummy;

	if (p_number_done == NULL)
		p_number_done = &number_done_dummy;
	if (ptia == NULL)
		ptia = &tia_dummy;

	ptia->tool_number = 0;
	ptia->argcpos = 0;
	ptia->nbargs = 0;
	ptia->ret = 0;
	

#ifdef DEBUG
	int icount;
	for (icount=0;icount<argc;icount++) u_printf("%s ",argv[icount]);
	u_printf("\n");
#endif
	
	if (argc <= 1)
	{
		unitex_tool_usage(1,1);
	}
	else
	while ((pos<argc) && (ret == 0))
	{
		if ((strcmp(argv[pos],"{")==0) && ((pos+1) < argc))
		{
			int j;
			for (j=pos;j<argc;j++)
			{
				if (strcmp(argv[j],"}")==0) {
					next_num_util++;
					break;
				}
			}

			if (j==argc)
			{
				ret = -1;
				break;
			}
			else
			{
				const struct utility_item* utility_called = found_utility(argv[pos+1]);
				if (utility_called != NULL) {
					ptia->argcpos = pos+1;
					ptia->nbargs = j-(pos+1);
					ptia->tool_number = next_num_util;
					ptia->ret = ret = CallToolLogged(utility_called->fnc,ptia->nbargs,((char**)argv)+ptia->argcpos);
				}
				else
					ret = 1 ;

				if (ret == 0)
					(*p_number_done)++;
				else
					break;
				pos = j + 1;
			}
		}
		else
		{
			const struct utility_item* utility_called = found_utility(argv[pos]);
			if (utility_called != NULL) {
				ptia->argcpos = 1;
				ptia->nbargs = argc-1;
				ptia->ret = ret = CallToolLogged(utility_called->fnc,ptia->nbargs,((char**)argv)+ptia->argcpos);
			}
			else
			{
				unitex_tool_usage(1,1);
				ret = 1;
			}

			if (ret == 0)
				(*p_number_done)++;
			break;
		}
	}

	return ret;
}

int UnitexTool_several(int argc,char* argv[],int* p_number_done)
{
	return UnitexTool_several_info(argc,argv,p_number_done,NULL);
}

int main_UnitexTool(int argc,char* argv[])
{
	return UnitexTool_several_info(argc,argv,NULL,NULL);
}

int main_UnitexTool_C(int argc,char* argv[])
{
	return UnitexTool_several_info(argc,argv,NULL,NULL);
}


UNITEX_FUNC int UNITEX_CALL UnitexTool_public_run(int argc,char* argv[],int* p_number_done,struct pos_tools_in_arg* ptia)
{
    return UnitexTool_several_info(argc,argv,p_number_done,ptia);
}

UNITEX_FUNC int UNITEX_CALL UnitexTool_public_run_one_tool(const char*toolname,int argc,char* argv[])
{
	const struct utility_item* utility_called = found_utility(toolname);
	if (utility_called != NULL) {
		return CallToolLogged(utility_called->fnc,argc,argv);
	}
	else {
		return 0;
	}
}


UNITEX_FUNC int UNITEX_CALL UnitexTool_public_GetNumberOfTool()
{
    return GetNumberOfTool();
}

UNITEX_FUNC int UNITEX_CALL UnitexTool_public_GetToolInfo_byname(const char* toolname,mainFunc** pfunc,const char** usage,const char** optstring,const struct option_TS **lopts)
{
    return GetToolInfo_byname(toolname,pfunc,usage,optstring,lopts);
}

UNITEX_FUNC int UNITEX_CALL UnitexTool_public_GetToolInfo_bynumber(int toolnumber,const char**toolname,mainFunc** pfunc,const char** usage,const char** optstring,const struct option_TS **lopts)
{
    return GetToolInfo_bynumber(toolnumber,toolname, pfunc, usage, optstring, lopts);
}

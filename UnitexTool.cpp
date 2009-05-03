 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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


#include "CheckDic.h"
#include "Compress.h"
#include "Concord.h"
#include "ConcorDiff.h"
#include "Convert.h"
#include "Dico.h"
#include "Elag.h"
#include "ElagComp.h"
#include "Evamb.h"
#include "Extract.h"
#include "Flatten.h"
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
#include "Table2Grf.h"
#include "TagsetNormTfst.h"
#include "TEI2Txt.h"
#include "Tfst2Grf.h"
#include "Tfst2Unambig.h"
#include "Tokenize.h"
#include "Txt2Tfst.h"
#include "XMLizer.h"

#include "UnitexTool.h"

#include "Unicode.h"
#include "Copyright.h"

typedef int mainFunc(int argc,char* argv[]) ;

struct utility_item {
	const char* name;
	int len_name;
	mainFunc* fnc;
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
	{ "CheckDic", 8, &main_CheckDic} ,
#ifndef UNITEX_ONLY_EXEC_GRAPH_TOOLS
	{ "Compress", 8, &main_Compress} ,
#endif
	{ "Concord", 7, &main_Concord} ,
#ifndef UNITEX_ONLY_EXEC_GRAPH_TOOLS
	{ "ConcorDiff", 10, &main_ConcorDiff} ,
	{ "Convert", 7, &main_Convert} ,
#endif
	{ "Dico", 4, &main_Dico} ,
#ifndef UNITEX_ONLY_EXEC_GRAPH_TOOLS
	{ "Elag", 4, &main_Elag} ,
	{ "ElagComp", 8, &main_ElagComp} ,
	{ "Evamb", 5, &main_Evamb} ,
	{ "Extract", 7, &main_Extract} ,
	{ "Flatten", 7, &main_Flatten} ,
	{ "Fst2List", 8, &main_Fst2List} ,
	{ "Fst2Txt", 7, &main_Fst2Txt} ,
	{ "Grf2Fst2", 8, &main_Grf2Fst2} ,
	{ "ImplodeTfst", 11, &main_ImplodeTfst} ,
#endif
	{ "Locate", 6, &main_Locate} ,
#ifndef UNITEX_ONLY_EXEC_GRAPH_TOOLS
	{ "LocateTfst", 10, &main_LocateTfst} ,
	{ "MultiFlex", 9, &main_MultiFlex} ,
#endif
	{ "Normalize", 9, &main_Normalize} ,
#ifndef UNITEX_ONLY_EXEC_GRAPH_TOOLS
	{ "PolyLex", 7, &main_PolyLex} ,
	{ "RebuildTfst", 11, &main_RebuildTfst} ,
	{ "Reconstrucao", 12, &main_Reconstrucao} ,
	{ "Reg2Grf", 7, &main_Reg2Grf} ,
#endif
	{ "SortTxt", 7, &main_SortTxt} ,
#ifndef UNITEX_ONLY_EXEC_GRAPH_TOOLS
	{ "Table2Grf", 9, &main_Table2Grf} ,
	{ "TagsetNormTfst", 14, &main_TagsetNormTfst} ,
	{ "TEI2Txt", 7, &main_TEI2Txt} ,
	{ "Tfst2Grf", 8, &main_Tfst2Grf} ,
	{ "Tfst2Unambig", 12, &main_Tfst2Unambig} ,
	{ "Tokenize", 8, &main_Tokenize} ,
	{ "Txt2Tfst", 8, &main_Txt2Tfst} ,
	{ "XMLizer", 7, &main_XMLizer} ,
#endif

#ifndef UNITEX_NO_KOREAN_TOOL
	{ "CompressKr" ,10, &main_CompressKr} ,
	{ "ConsultDic" ,10, &main_ConsultDic} ,
	{ "ExtractChar" ,11, &main_ExtractChar} ,
	{ "InflectKr" ,9, &main_InflectKr} ,
	{ "Jamo2Syl" ,8, &main_Jamo2Syl} ,
	{ "MergeBin" ,8, &main_MergeBin} ,
	{ "SortMorph" ,9, &main_SortMorph} ,
	{ "SufForm2Rac" ,11, &main_SufForm2Rac} ,
	{ "Syl2Jamo" ,8, &main_Syl2Jamo} ,
	{ "Txt2Fst2Kr" ,10, &main_Txt2Fst2Kr} ,
#endif
	{ "", 0, NULL} 
};

const struct utility_item* found_utility(const char* search)
{
	int len_search = strlen(search);
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

void unitex_tool_usage(int several)
{
	int i=0;
	u_printf("%S",COPYRIGHT);
	u_printf("Usage: UnitexTool <Utility> [OPTIONS]\n" 
		     "where OPTIONS can be -h/--help to display help"
			 "and Utility is from this list :\n");
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
}

int check_Utility(const char* name)
{
const struct utility_item* utility_called = found_utility(name);
if (utility_called != NULL)
  return 1;
else
  return 0;
}

int main_UnitexTool_single(int argc,char* argv[]) {
const struct utility_item* utility_called = NULL;
if (argc>1)
  utility_called = found_utility(argv[1]);
if (utility_called==NULL) {
   unitex_tool_usage(0);
   return 0;
}

return (*(utility_called->fnc))(argc-1,((char**)argv)+1);
}


int UnitexTool_several(int argc,char* argv[],int* p_number_done)
{
	int ret=0;
	int number_done=0;
	int pos = 1;

	if (argc <= 1)
	{
		unitex_tool_usage(1);
	}
	else
	while ((pos<argc) && (ret == 0))
	{
		if ((strcmp(argv[pos],"{")==0) && ((pos+1) < argc))
		{
			int j;
			for (j=pos;j<argc;j++)
			{
				if (strcmp(argv[j],"}")==0)
					break;
			}

			if (j==argc)
			{
				ret = -1;
				break;
			}
			else
			{
				const struct utility_item* utility_called = found_utility(argv[pos+1]);
				if (utility_called != NULL)
					ret = (*(utility_called->fnc))(j-(pos+1),((char**)argv)+pos+1);
				else
					ret = 1 ;

				if (ret == 0)
					number_done++;
				else
					break;
				pos = j + 1;
			}
		}
		else
		{
			const struct utility_item* utility_called = found_utility(argv[pos]);
			if (utility_called != NULL)
				ret = (*(utility_called->fnc))(argc-1,((char**)argv)+1);
			else
			{
				unitex_tool_usage(1);
				ret = 1;
			}

			if (ret == 0)
				number_done++;
			break;
		}
	}
	if (p_number_done != NULL)
		*p_number_done = number_done;

	return ret;
}

int main_UnitexTool(int argc,char* argv[])
{
	return UnitexTool_several(argc,argv,NULL);
}

int main_UnitexTool_C(int argc,char* argv[])
{
	return UnitexTool_several(argc,argv,NULL);
}

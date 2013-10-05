/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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




#include <stdlib.h>
#include <string.h>

#include "Unicode.h"
#include "SyncTool.h"

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_BUILDKRMWUDIC))) || defined(TOOL_BUILDKRMWUDIC))
#include "BuildKrMwuDic.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_CASSYS))) || defined(TOOL_CASSYS))
#include "Cassys.h"
#endif

#if (((!defined(NO_TOOL_CHECKDIC))) || defined(TOOL_CHECKDIC))
#include "CheckDic.h"
#endif

#if (((!defined(NO_TOOL_COMPRESS))) || defined(TOOL_COMPRESS))
#include "Compress.h"
#endif

#if (((!defined(NO_TOOL_CONCORD))) || defined(TOOL_CONCORD))
#include "Concord.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_CONCORDIFF))) || defined(TOOL_CONCORDIFF))
#include "ConcorDiff.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!defined(NO_TOOL_CONVERT))) || defined(TOOL_CONVERT))
#include "Convert.h"
#endif

#if (((!defined(NO_TOOL_DICO))) || defined(TOOL_DICO))
#include "Dico.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_DUPLICATEFILE))) || defined(TOOL_DUPLICATEFILE))
#include "DuplicateFile.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_ELAG))) || defined(TOOL_ELAG))
#include "Elag.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_ELAGCOMP))) || defined(TOOL_ELAGCOMP))
#include "ElagComp.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_EVAMB))) || defined(TOOL_EVAMB))
#include "Evamb.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!defined(NO_TOOL_EXTRACT))) || defined(TOOL_EXTRACT))
#include "Extract.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_FLATTEN))) || defined(TOOL_FLATTEN))
#include "Flatten.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_FST2CHECK))) || defined(TOOL_FST2CHECK))
#include "Fst2Check.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_FST2LIST))) || defined(TOOL_FST2LIST))
#include "Fst2List.h"
#endif

#if (((!defined(NO_TOOL_FST2TXT))) || defined(TOOL_FST2TXT))
#include "Fst2Txt.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_GRF2FST2))) || defined(TOOL_GRF2FST2))
#include "Grf2Fst2.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_GRFDIFF))) || defined(TOOL_GRFDIFF))
#include "GrfDiff.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_GRFDIFF3))) || defined(TOOL_GRFDIFF3))
#include "GrfDiff3.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_GRFTEST))) || defined(TOOL_GRFTEST))
#include "GrfTest.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_IMPLODETFST))) || defined(TOOL_IMPLODETFST))
#include "ImplodeTfst.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_KEYWORDS))) || defined(TOOL_KEYWORDS))
#include "KeyWords.h"
#endif

#if (((!defined(NO_TOOL_LOCATE))) || defined(TOOL_LOCATE))
#include "Locate.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!defined(NO_TOOL_LOCATETFST))) || defined(TOOL_LOCATETFST))
#include "LocateTfst.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_MULTIFLEX))) || defined(TOOL_MULTIFLEX))
#include "MultiFlex.h"
#endif

#if (((!defined(NO_TOOL_NORMALIZE))) || defined(TOOL_NORMALIZE))
#include "Normalize.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_POLYLEX))) || defined(TOOL_POLYLEX))
#include "PolyLex.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_REBUILDTFST))) || defined(TOOL_REBUILDTFST))
#include "RebuildTfst.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_RECONSTRUCTAO))) || defined(TOOL_RECONSTRUCTAO))
#include "Reconstrucao.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_REG2GRF))) || defined(TOOL_REG2GRF))
#include "Reg2Grf.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_SEQ2GRF))) || defined(TOOL_SEQ2GRF))
#include "Seq2Grf.h"
#endif

#if (((!defined(NO_TOOL_SORTTXT))) || defined(TOOL_SORTTXT))
#include "SortTxt.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_STATS))) || defined(TOOL_STATS))
#include "Stats.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TABLE2GRF))) || defined(TOOL_TABLE2GRF))
#include "Table2Grf.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TAGGER))) || defined(TOOL_TAGGER))
#include "Tagger.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TAGSETNORMTFST))) || defined(TOOL_TAGSETNORMTFST))
#include "TagsetNormTfst.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TEI2TXT))) || defined(TOOL_TEI2TXT))
#include "TEI2Txt.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TFST2GRF))) || defined(TOOL_TFST2GRF))
#include "Tfst2Grf.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TFSTTAG))) || defined(TOOL_TFSTTAG))
#include "TfstTag.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TFST2UNAMBIG))) || defined(TOOL_TFST2UNAMBIG))
#include "Tfst2Unambig.h"
#endif

#if (((!defined(NO_TOOL_TOKENIZE))) || defined(TOOL_TOKENIZE))
#include "Tokenize.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TRAININGTAGGER))) || defined(TOOL_TRAININGTAGGER))
#include "TrainingTagger.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TXT2TFST))) || defined(TOOL_TXT2TFST))
#include "Txt2Tfst.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_UNCOMPRESS))) || defined(TOOL_UNCOMPRESS))
#include "Uncompress.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_UNTOKENIZE))) || defined(TOOL_UNTOKENIZE))
#include "Untokenize.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_UNXMLIZE))) || defined(TOOL_UNXMLIZE))
#include "Unxmlize.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_VERSIONINFO))) || defined(TOOL_VERSIONINFO))
#include "VersionInfo.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_XMLIZER))) || defined(TOOL_XMLIZER))
#include "XMLizer.h"
#endif


#ifdef UNITEXTOOL_TOOL_FROM_LOGGER
#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_PACKFILE))) || defined(TOOL_PACKFILE))
#include "logger/PackFile.h"
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_PACKFILE))) || defined(TOOL_PACKFILE))
#include "logger/UnpackFile.h"
#endif
#endif

#include "Copyright.h"

#include "UnitexTool.h"
#include "ActivityLogger.h"


#include "UserCancelling.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#ifdef UNITEXTOOL_TOOL_FROM_LOGGER
#ifdef HAS_LOGGER_NAMESPACE
using namespace logger;
#endif
#endif

struct utility_item {
	const char* name;
	int len_name;
	mainFunc* fnc;

	const char* usage;
	const char* optstring;
	const struct option_TS *lopts;
} ;

const struct utility_item utility_array[]=
{
#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_BUILDKRMWUDIC))) || defined(TOOL_BUILDKRMWUDIC))
	{ "BuildKrMwuDic",13,&main_BuildKrMwuDic,usage_BuildKrMwuDic, optstring_BuildKrMwuDic, lopts_BuildKrMwuDic},
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_CASSYS))) || defined(TOOL_CASSYS))
	{ "Cassys",6,&main_Cassys,usage_Cassys, optstring_Cassys, lopts_Cassys},
#endif

#if (((!defined(NO_TOOL_CHECKDIC))) || defined(TOOL_CHECKDIC))
	{ "CheckDic", 8, &main_CheckDic, usage_CheckDic, optstring_CheckDic, lopts_CheckDic } ,
#endif

#if (((!defined(NO_TOOL_COMPRESS))) || defined(TOOL_COMPRESS))
	{ "Compress", 8, &main_Compress, usage_Compress, optstring_Compress, lopts_Compress } ,
#endif

#if (((!defined(NO_TOOL_CONCORD))) || defined(TOOL_CONCORD))
	{ "Concord", 7, &main_Concord, usage_Concord, optstring_Concord, lopts_Concord } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_CONCORDIFF))) || defined(TOOL_CONCORDIFF))
	{ "ConcorDiff", 10, &main_ConcorDiff, usage_ConcorDiff, optstring_ConcorDiff, lopts_ConcorDiff } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!defined(NO_TOOL_CONVERT))) || defined(TOOL_CONVERT))
	{ "Convert", 7, &main_Convert, usage_Convert, optstring_Convert, lopts_Convert } ,
#endif

#if (((!defined(NO_TOOL_DICO))) || defined(TOOL_DICO))
	{ "Dico", 4, &main_Dico, usage_Dico, optstring_Dico, lopts_Dico } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_DUPLICATEFILE))) || defined(TOOL_DUPLICATEFILE))
	{ "DuplicateFile", 13, &main_DuplicateFile, usage_DuplicateFile, optstring_DuplicateFile, lopts_DuplicateFile } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_ELAG))) || defined(TOOL_ELAG))
	{ "Elag", 4, &main_Elag, usage_Elag, optstring_Elag, lopts_Elag } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_ELAGCOMP))) || defined(TOOL_ELAGCOMP))
	{ "ElagComp", 8, &main_ElagComp, usage_ElagComp, optstring_ElagComp, lopts_ElagComp } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_EVAMB))) || defined(TOOL_EVAMB))
	{ "Evamb", 5, &main_Evamb, usage_Evamb, optstring_Evamb, lopts_Evamb } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!defined(NO_TOOL_EXTRACT))) || defined(TOOL_EXTRACT))
	{ "Extract", 7, &main_Extract, usage_Extract, optstring_Extract, lopts_Extract } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_FLATTEN))) || defined(TOOL_FLATTEN))
	{ "Flatten", 7, &main_Flatten, usage_Flatten, optstring_Flatten, lopts_Flatten } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_FST2CHECK))) || defined(TOOL_FST2CHECK))
	{ "Fst2Check", 9, &main_Fst2Check, usage_Fst2Check, optstring_Fst2Check, lopts_Fst2Check } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_FST2LIST))) || defined(TOOL_FST2LIST))
	{ "Fst2List", 8, &main_Fst2List, usage_Fst2List, NULL, NULL } ,
#endif

#if (((!defined(NO_TOOL_FST2TXT))) || defined(TOOL_FST2TXT))
	{ "Fst2Txt", 7, &main_Fst2Txt, usage_Fst2Txt, optstring_Fst2Txt, lopts_Fst2Txt } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_GRF2FST2))) || defined(TOOL_GRF2FST2))
	{ "Grf2Fst2", 8, &main_Grf2Fst2, usage_Grf2Fst2, optstring_Grf2Fst2, lopts_Grf2Fst2 } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_GRFDIFF))) || defined(TOOL_GRFDIFF))
	{ "GrfDiff", 7, &main_GrfDiff, usage_GrfDiff, optstring_GrfDiff, lopts_GrfDiff } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_GRFDIFF3))) || defined(TOOL_GRFDIFF3))
	{ "GrfDiff3", 8, &main_GrfDiff3, usage_GrfDiff3, optstring_GrfDiff3, lopts_GrfDiff3 } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_GRFTEST))) || defined(TOOL_GRFTEST))
	{ "GrfTest", 7, &main_GrfTest, usage_GrfTest, optstring_GrfTest, lopts_GrfTest } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_IMPLODETFST))) || defined(TOOL_IMPLODETFST))
	{ "ImplodeTfst", 11, &main_ImplodeTfst, usage_ImplodeTfst, optstring_ImplodeTfst, lopts_ImplodeTfst } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!defined(NO_TOOL_KEYWORDS))) || defined(TOOL_KEYWORDS))
	{ "KeyWords", 8, &main_KeyWords, usage_KeyWords, optstring_KeyWords, lopts_KeyWords } ,
#endif

#if (((!defined(NO_TOOL_LOCATE))) || defined(TOOL_LOCATE))
	{ "Locate", 6, &main_Locate, usage_Locate, optstring_Locate, lopts_Locate } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!defined(NO_TOOL_LOCATETFST))) || defined(TOOL_LOCATETFST))
	{ "LocateTfst", 10, &main_LocateTfst, usage_LocateTfst, optstring_LocateTfst, lopts_LocateTfst } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_MULTIFLEX))) || defined(TOOL_MULTIFLEX))
	{ "MultiFlex", 9, &main_MultiFlex, usage_MultiFlex, optstring_MultiFlex, lopts_MultiFlex } ,
#endif

#if (((!defined(NO_TOOL_NORMALIZE))) || defined(TOOL_NORMALIZE))
	{ "Normalize", 9, &main_Normalize, usage_Normalize, optstring_Normalize, lopts_Normalize } ,
#endif

#ifdef UNITEXTOOL_TOOL_FROM_LOGGER
#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_PACKFILE))) || defined(TOOL_PACKFILE))
	{ "PackFile", 8, &main_PackFile, usage_PackFile, optstring_PackFile, lopts_PackFile } ,
#endif
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_POLYLEX))) || defined(TOOL_POLYLEX))
	{ "PolyLex", 7, &main_PolyLex, usage_PolyLex, optstring_PolyLex, lopts_PolyLex } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_REBUILDTFST))) || defined(TOOL_REBUILDTFST))
	{ "RebuildTfst", 11, &main_RebuildTfst, usage_RebuildTfst, optstring_RebuildTfst, lopts_RebuildTfst } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_RECONSTRUCTAO))) || defined(TOOL_RECONSTRUCTAO))
	{ "Reconstrucao", 12, &main_Reconstrucao, usage_Reconstrucao, optstring_Reconstrucao, lopts_Reconstrucao } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_REG2GRF))) || defined(TOOL_REG2GRF))
	{ "Reg2Grf", 7, &main_Reg2Grf, usage_Reg2Grf, optstring_Reg2Grf, lopts_Reg2Grf } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_SEQ2GRF))) || defined(TOOL_SEQ2GRF))
	{ "Seq2Grf", 7, &main_Seq2Grf, usage_Seq2Grf, optstring_Seq2Grf, lopts_Seq2Grf } ,
#endif

#if (((!defined(NO_TOOL_SORTTXT))) || defined(TOOL_SORTTXT))
	{ "SortTxt", 7, &main_SortTxt, usage_SortTxt, optstring_SortTxt, lopts_SortTxt } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_STATS))) || defined(TOOL_STATS))
	{ "Stats", 5, &main_Stats, usage_Stats, optstring_Stats, lopts_Stats } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TABLE2GRF))) || defined(TOOL_TABLE2GRF))
	{ "Table2Grf", 9, &main_Table2Grf, usage_Table2Grf, optstring_Table2Grf, lopts_Table2Grf } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TAGGER))) || defined(TOOL_TAGGER))
	{ "Tagger", 6, &main_Tagger, usage_Tagger, optstring_Tagger, lopts_Tagger } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TAGSETNORMTFST))) || defined(TOOL_TAGSETNORMTFST))
	{ "TagsetNormTfst", 14, &main_TagsetNormTfst, usage_TagsetNormTfst, optstring_TagsetNormTfst, lopts_TagsetNormTfst } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TEI2TXT))) || defined(TOOL_TEI2TXT))
	{ "TEI2Txt", 7, &main_TEI2Txt, usage_TEI2Txt, optstring_TEI2Txt, lopts_TEI2Txt } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TFST2GRF))) || defined(TOOL_TFST2GRF))
	{ "Tfst2Grf", 8, &main_Tfst2Grf, usage_Tfst2Grf, optstring_Tfst2Grf, lopts_Tfst2Grf } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TFSTTAG))) || defined(TOOL_TFSTTAG))
	{ "TfstTag", 7, &main_TfstTag, usage_TfstTag, optstring_TfstTag, lopts_TfstTag } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TFST2UNAMBIG))) || defined(TOOL_TFST2UNAMBIG))
	{ "Tfst2Unambig", 12, &main_Tfst2Unambig, usage_Tfst2Unambig, optstring_Tfst2Unambig, lopts_Tfst2Unambig } ,
#endif

#if (((!defined(NO_TOOL_TOKENIZE))) || defined(TOOL_TOKENIZE))
	{ "Tokenize", 8, &main_Tokenize, usage_Tokenize, optstring_Tokenize, lopts_Tokenize } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TRAININGTAGGER))) || defined(TOOL_TRAININGTAGGER))
	{ "TrainingTagger", 14, &main_TrainingTagger, usage_TrainingTagger, optstring_TrainingTagger, lopts_TrainingTagger } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_TXT2TFST))) || defined(TOOL_TXT2TFST))
	{ "Txt2Tfst", 8, &main_Txt2Tfst, usage_Txt2Tfst, optstring_Txt2Tfst, lopts_Txt2Tfst } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_UNCOMPRESS))) || defined(TOOL_UNCOMPRESS))
	{ "Uncompress", 10, &main_Uncompress, usage_Uncompress, optstring_Uncompress, lopts_Uncompress } ,
#endif

#ifdef UNITEXTOOL_TOOL_FROM_LOGGER
#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_PACKFILE))) || defined(TOOL_PACKFILE))
	{ "UnpackFile", 10, &main_UnpackFile, usage_UnpackFile, optstring_UnpackFile, lopts_UnpackFile } ,
#endif
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_UNTOKENIZE))) || defined(TOOL_UNTOKENIZE))
	{ "Untokenize", 10, &main_Untokenize, usage_Untokenize, optstring_Untokenize, lopts_Untokenize } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_UNXMLIZE))) || defined(TOOL_UNXMLIZE))
	{ "Unxmlize", 8, &main_Unxmlize, usage_Unxmlize, optstring_Unxmlize, lopts_Unxmlize } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_VERSIONINFO))) || defined(TOOL_VERSIONINFO))
	{ "VersionInfo", 11, &main_VersionInfo, usage_VersionInfo, optstring_VersionInfo, lopts_VersionInfo } ,
#endif

#if (((!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS))) && (!(defined(UNITEX_ONLY_EXEC_GRAPH_TOOLS_RICH))) && (!defined(NO_TOOL_XMLIZER))) || defined(TOOL_XMLIZER))
	{ "XMLizer", 7, &main_XMLizer, usage_XMLizer, optstring_XMLizer, lopts_XMLizer } ,
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

    u_printf("unused letter for optstring : ");
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

#ifdef HAVE_TIMEBOMB_FUNCTION
extern "C" int CheckDateTimeBomb();
#else
static int CheckDateTimeBomb()
{
	return 1;
}
#endif

static int CallToolLogged(mainFunc* fnc,int argc,char* const argv[])
{
    int ret;
    Call_logger_fnc_before_calling_tool(fnc,argc,argv);
    if ((!CheckDateTimeBomb()) || (is_cancelling_requested() != 0))
        ret = 0;
    else
        ret = (*fnc)(argc,argv);
    Call_logger_fnc_after_calling_tool(fnc,argc,argv,ret);
    return ret;
}

int main_UnitexTool_single(int argc,char* const argv[]) {
const struct utility_item* utility_called = NULL;
if (argc>1)
  utility_called = found_utility(argv[1]);
if (utility_called==NULL) {
   unitex_tool_usage(0,1);
   return 0;
}

return CallToolLogged(utility_called->fnc,argc-1,((char**)argv)+1);
}


int UnitexTool_several_info(int argc,char* const argv[],int* p_number_done,struct pos_tools_in_arg* ptia)
{
	int ret=0;
	int number_done_dummy=0;
	int pos = 1;
	int next_num_util = 0;
	pos_tools_in_arg tia_dummy;

	const char* fTime=NULL;
	hTimeElapsed startTime=NULL;
	const char* firstArg=NULL;
	if (argc>1)
		firstArg=argv[1];

	if ((firstArg!=NULL) && (strstr(firstArg,"--time=")==firstArg)) {
		fTime=firstArg+7;
		startTime=SyncBuidTimeMarkerObject();
		argc--;
		argv++;
	}

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

	if (fTime!=NULL) {
		double msec=(double)(SyncGetMSecElapsed(startTime)/((double)1000.));
		U_FILE* f=u_fopen(UTF8,fTime,U_WRITE);
		if (f==NULL) {
			fatal_error("Unable to open time file %s\n",fTime);
		}
		u_fprintf(f,"%.4g\n",msec);
		u_fclose(f);
	}
	return ret;
}

int UnitexTool_several(int argc,char* const argv[],int* p_number_done)
{
	return UnitexTool_several_info(argc,argv,p_number_done,NULL);
}

int main_UnitexTool(int argc,char* const argv[])
{
	return UnitexTool_several_info(argc,argv,NULL,NULL);
}

int main_UnitexTool_C(int argc,char* const argv[])
{
	return UnitexTool_several_info(argc,argv,NULL,NULL);
}


UNITEX_FUNC int UNITEX_CALL UnitexTool_public_run(int argc,char* const argv[],int* p_number_done,struct pos_tools_in_arg* ptia)
{
    return UnitexTool_several_info(argc,argv,p_number_done,ptia);
}

UNITEX_FUNC int UNITEX_CALL UnitexTool_public_run_one_tool(const char*toolname,int argc,char* const argv[])
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

} // namespace unitex

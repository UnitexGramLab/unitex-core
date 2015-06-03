#
# Unitex
#
# Copyright (C) 2001-2014 Universite Paris-Est Marne-la-Vallee <unitex@univ-mlv.fr>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
# 

# 
# File created and contributed by Gilles Vollant (Ergonotics SAS)
# as part of an UNITEX optimization and reliability effort
#
# additional information: http://www.ergonotics.com/unitex-contribution/
# https://github.com/ergonotics/JNI-for-Unitex-2.1
# contact : unitex-contribution@ergonotics.com
#


# To compile the Android JNI
# create a folder named jni and put this Android.mk file info
# copy Unitex source in a Unitex-C++ subfolder of jni folder (so jni/Unitex-C++)
# uncompress tre-0.8.0 into jni/Unitex-C++/build/tre-0.8.0 subfolder
# run bash, go to folder upper to jni folder and run "ndk-build" script
#  more info on http://developer.android.com/sdk/ndk/index.html

JNI_DIR := $(call my-dir)
LOCAL_PATH := $(JNI_DIR)


include $(CLEAR_VARS)


# Here we give our module name and source file(s)
LOCAL_MODULE    := UnitexJNI
LOCAL_CFLAGS := -D_NOT_UNDER_WINDOWS -DHAVE_CONFIG_H -DUNITEX_LIBRARY -DUNITEX_LIBRARY_VF_IMPORT -DUNITEX_LIBRARY_VF_IMPORT -DANDROID -I$(JNI_DIR)/Unitex-C++ -I$(JNI_DIR)/Unitex-C++/logger -I$(JNI_DIR)/Unitex-C++/include_tre

LOCAL_SRC_FILES := \
Unitex-C++/UnitexLibAndJni/fr_umlv_unitex_jni_UnitexJni.cpp \
Unitex-C++/UnitexLibDirPosix.cpp \
Unitex-C++/UnitexLibIO.cpp \
Unitex-C++/SyncToolPosix.cpp \
Unitex-C++/CompressedDic.cpp Unitex-C++/Copyright.cpp Unitex-C++/DebugMode.cpp Unitex-C++/GrfBeauty.cpp \
Unitex-C++/GrfDiff.cpp Unitex-C++/GrfDiff3.cpp Unitex-C++/GrfSvn_lib.cpp \
Unitex-C++/GrfTest.cpp Unitex-C++/GrfTest_lib.cpp Unitex-C++/TfstTag.cpp \
Unitex-C++/Grf_lib.cpp Unitex-C++/Keyboard.cpp Unitex-C++/LoadInf.cpp \
Unitex-C++/Offsets.cpp Unitex-C++/Overlap.cpp Unitex-C++/Persistence.cpp Unitex-C++/PersistenceInterface.cpp \
Unitex-C++/Seq2Grf.cpp Unitex-C++/SpellCheck.cpp Unitex-C++/SpellChecking.cpp \
Unitex-C++/UnitexRevisionInfo.cpp Unitex-C++/Unxmlize.cpp Unitex-C++/VersionInfo.cpp \
Unitex-C++/VirtualFiles.cpp Unitex-C++/Xml.cpp \
Unitex-C++/KeyWords.cpp Unitex-C++/KeyWords_lib.cpp Unitex-C++/RegExFacade.cpp \
Unitex-C++/PRLG.cpp \
Unitex-C++/build/tre-0.8.0/lib/regcomp.c \
Unitex-C++/build/tre-0.8.0/lib/regerror.c \
Unitex-C++/build/tre-0.8.0/lib/regexec.c \
Unitex-C++/build/tre-0.8.0/lib/tre-ast.c \
Unitex-C++/build/tre-0.8.0/lib/tre-compile.c \
Unitex-C++/build/tre-0.8.0/lib/tre-match-approx.c \
Unitex-C++/build/tre-0.8.0/lib/tre-match-backtrack.c \
Unitex-C++/build/tre-0.8.0/lib/tre-match-parallel.c \
Unitex-C++/build/tre-0.8.0/lib/tre-mem.c \
Unitex-C++/build/tre-0.8.0/lib/tre-parse.c \
Unitex-C++/build/tre-0.8.0/lib/tre-stack.c \
Unitex-C++/build/tre-0.8.0/lib/xmalloc.c \
Unitex-C++/logger/InstallLingResourcePackage.cpp \
Unitex-C++/logger/LingResourcePackage.cpp \
Unitex-C++/logger/MzRepairUlp.cpp \
Unitex-C++/logger/MzToolsUlp.cpp \
Unitex-C++/logger/FilePack.cpp \
Unitex-C++/logger/FilePackCrc32.cpp \
Unitex-C++/logger/FilePackIo.cpp \
Unitex-C++/logger/FileUnPack.cpp \
Unitex-C++/logger/UnpackFileTool.cpp \
Unitex-C++/logger/ReworkArg.cpp \
Unitex-C++/logger/SyncLoggerPosix.cpp \
Unitex-C++/logger/UniLogger.cpp \
Unitex-C++/logger/UniLoggerAutoInstall.cpp \
Unitex-C++/logger/UniRunLogger.cpp \
Unitex-C++/logger/RunTools.cpp \
Unitex-C++/logger/UniRunScript.cpp \
Unitex-C++/XMLizer.cpp \
Unitex-C++/AbstractAllocator.cpp \
Unitex-C++/AbstractDelaLoad.cpp \
Unitex-C++/AbstractFst2Load.cpp \
Unitex-C++/ActivityLogger.cpp \
Unitex-C++/Af_stdio.cpp \
Unitex-C++/Alphabet.cpp \
Unitex-C++/ApplyDic.cpp \
Unitex-C++/Arabic.cpp \
Unitex-C++/AsciiSearchTree.cpp \
Unitex-C++/AutComplementation.cpp \
Unitex-C++/AutConcat.cpp \
Unitex-C++/AutDeterminization.cpp \
Unitex-C++/AutIntersection.cpp \
Unitex-C++/AutMinimization.cpp \
Unitex-C++/AutomatonDictionary2Bin.cpp \
Unitex-C++/BitArray.cpp \
Unitex-C++/BitMasks.cpp \
Unitex-C++/Buffer.cpp \
Unitex-C++/BuildKrMwuDic.cpp \
Unitex-C++/BuildTextAutomaton.cpp \
Unitex-C++/Cassys.cpp \
Unitex-C++/Cassys_concord.cpp \
Unitex-C++/Cassys_external_program.cpp \
Unitex-C++/Cassys_io.cpp \
Unitex-C++/Cassys_lexical_tags.cpp \
Unitex-C++/Cassys_tokens.cpp \
Unitex-C++/Cassys_transducer.cpp \
Unitex-C++/Cassys_xml_output.cpp \
Unitex-C++/CheckDic.cpp \
Unitex-C++/CodePages.cpp \
Unitex-C++/CompoundWordHashTable.cpp \
Unitex-C++/CompoundWordTree.cpp \
Unitex-C++/Compress.cpp \
Unitex-C++/Concord.cpp \
Unitex-C++/Concordance.cpp \
Unitex-C++/ConcorDiff.cpp \
Unitex-C++/Contexts.cpp \
Unitex-C++/Convert.cpp \
Unitex-C++/DELA.cpp \
Unitex-C++/DELA_tree.cpp \
Unitex-C++/Dico.cpp \
Unitex-C++/DictionaryTree.cpp \
Unitex-C++/DicVariables.cpp \
Unitex-C++/Diff.cpp \
Unitex-C++/DirHelperPosix.cpp \
Unitex-C++/DumpOffsets.cpp \
Unitex-C++/DuplicateFile.cpp \
Unitex-C++/DutchCompounds.cpp \
Unitex-C++/Elag.cpp \
Unitex-C++/ElagComp.cpp \
Unitex-C++/ElagDebug.cpp \
Unitex-C++/ElagFstFilesIO.cpp \
Unitex-C++/ElagFunctions.cpp \
Unitex-C++/ElagRulesCompilation.cpp \
Unitex-C++/ElagStateSet.cpp \
Unitex-C++/Error.cpp \
Unitex-C++/Evamb.cpp \
Unitex-C++/Extract.cpp \
Unitex-C++/ExtractUnits.cpp \
Unitex-C++/FIFO.cpp \
Unitex-C++/File.cpp \
Unitex-C++/Flatten.cpp \
Unitex-C++/FlattenFst2.cpp \
Unitex-C++/Fst2.cpp \
Unitex-C++/Fst2Automaton.cpp \
Unitex-C++/Fst2Check.cpp \
Unitex-C++/Fst2Check_lib.cpp \
Unitex-C++/Fst2List.cpp \
Unitex-C++/Fst2Txt.cpp \
Unitex-C++/Fst2Txt_TokenTree.cpp \
Unitex-C++/Fst2TxtAsRoutine.cpp \
Unitex-C++/GeneralDerivation.cpp \
Unitex-C++/GermanCompounds.cpp \
Unitex-C++/Grf2Fst2.cpp \
Unitex-C++/Grf2Fst2_lib.cpp \
Unitex-C++/HashTable.cpp \
Unitex-C++/HTMLCharacters.cpp \
Unitex-C++/ImplodeTfst.cpp \
Unitex-C++/IOBuffer.cpp \
Unitex-C++/Korean.cpp \
Unitex-C++/KrMwuDic.cpp \
Unitex-C++/LanguageDefinition.cpp \
Unitex-C++/LemmaTree.cpp \
Unitex-C++/LinearAutomaton2Txt.cpp \
Unitex-C++/List_int.cpp \
Unitex-C++/List_pointer.cpp \
Unitex-C++/List_ustring.cpp \
Unitex-C++/Locate.cpp \
Unitex-C++/LocateCache.cpp \
Unitex-C++/LocateFst2Tags.cpp \
Unitex-C++/LocateMatches.cpp \
Unitex-C++/LocatePattern.cpp \
Unitex-C++/LocateTfst.cpp \
Unitex-C++/LocateTfst_lib.cpp \
Unitex-C++/LocateTfstMatches.cpp \
Unitex-C++/LocateTrace.cpp \
Unitex-C++/MappedFileHelperPosix.cpp \
Unitex-C++/Match.cpp \
Unitex-C++/MF_DicoMorpho.cpp \
Unitex-C++/MF_DLC_inflect.cpp \
Unitex-C++/MF_FormMorpho.cpp \
Unitex-C++/MF_Global.cpp \
Unitex-C++/MF_LangMorpho.cpp \
Unitex-C++/MF_MU_graph.cpp \
Unitex-C++/MF_MU_morpho.cpp \
Unitex-C++/MF_Operators_Util.cpp \
Unitex-C++/MF_SU_morpho.cpp \
Unitex-C++/MF_Unif.cpp \
Unitex-C++/MF_Util.cpp \
Unitex-C++/MorphologicalFilters.cpp \
Unitex-C++/MorphologicalLocate.cpp \
Unitex-C++/MultiFlex.cpp \
Unitex-C++/NewLineShifts.cpp \
Unitex-C++/NormalizationFst2.cpp \
Unitex-C++/Normalize.cpp \
Unitex-C++/NormalizeAsRoutine.cpp \
Unitex-C++/NorwegianCompounds.cpp \
Unitex-C++/OptimizedFst2.cpp \
Unitex-C++/OptimizedTfstTagMatching.cpp \
Unitex-C++/OutputTransductionVariables.cpp \
Unitex-C++/ParsingInfo.cpp \
Unitex-C++/Pattern.cpp \
Unitex-C++/PatternTree.cpp \
Unitex-C++/PolyLex.cpp \
Unitex-C++/PortugueseNormalization.cpp \
Unitex-C++/ProgramInvoker.cpp \
Unitex-C++/RebuildTfst.cpp \
Unitex-C++/Reconstrucao.cpp \
Unitex-C++/Reg2Grf.cpp \
Unitex-C++/RegularExpressions.cpp \
Unitex-C++/RussianCompounds.cpp \
Unitex-C++/SelectOutput.cpp \
Unitex-C++/Sentence2Grf.cpp \
Unitex-C++/SingleGraph.cpp \
Unitex-C++/Snt.cpp \
Unitex-C++/SortTxt.cpp \
Unitex-C++/Stack_int.cpp \
Unitex-C++/Stack_pointer.cpp \
Unitex-C++/Stack_unichar.cpp \
Unitex-C++/Stats.cpp \
Unitex-C++/String_hash.cpp \
Unitex-C++/StringParsing.cpp \
Unitex-C++/symbol.cpp \
Unitex-C++/symbol_op.cpp \
Unitex-C++/SymbolAlphabet.cpp \
Unitex-C++/Table2Grf.cpp \
Unitex-C++/Tagger.cpp \
Unitex-C++/TaggingProcess.cpp \
Unitex-C++/Tagset.cpp \
Unitex-C++/TagsetNormTfst.cpp \
Unitex-C++/TEI2Txt.cpp \
Unitex-C++/Text_parsing.cpp \
Unitex-C++/Text_tokens.cpp \
Unitex-C++/Tfst.cpp \
Unitex-C++/Tfst2Grf.cpp \
Unitex-C++/Tfst2Unambig.cpp \
Unitex-C++/TfstStats.cpp \
Unitex-C++/Thai.cpp \
Unitex-C++/Tokenization.cpp \
Unitex-C++/Tokenize.cpp \
Unitex-C++/TrainingProcess.cpp \
Unitex-C++/TrainingTagger.cpp \
Unitex-C++/TransductionStack.cpp \
Unitex-C++/TransductionStackTfst.cpp \
Unitex-C++/TransductionVariables.cpp \
Unitex-C++/Transitions.cpp \
Unitex-C++/Txt2Tfst.cpp \
Unitex-C++/Uncompress.cpp \
Unitex-C++/Unicode.cpp \
Unitex-C++/UnitexGetOpt.cpp \
Unitex-C++/UnitexTool.cpp \
Unitex-C++/Untokenize.cpp \
Unitex-C++/UserCancelling.cpp \
Unitex-C++/ustring.cpp \
Unitex-C++/VariableUtils.cpp

include $(BUILD_SHARED_LIBRARY)

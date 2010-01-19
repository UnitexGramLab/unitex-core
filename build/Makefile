#
# This is the multi-system makefile of Unitex (http://igm.univ-mlv.fr/~unitex/)
# Compilation under Windows is supposed to be done with Dev-C++. To compile under
# Windows, type:
#
#    make SYSTEM=windows
#
# If have a 64 bits machine, you must use '64BITS=yes':
#
#    make 64BITS=yes
#
#
# By default, 'make' will compile Unitex for a 32 bits Linux.
#
#
# The special command
#
#    make SYSTEM=mingw32
#
# can be used to generate Windows executables under Linux, using the mingw32 tools.
#
# The option
#    make UNITEX_RELEASE_MEMORY_AT_EXIT=yes
# request freeing all memory possible at exit (
#
# The option
#    make UNITEXTOOLONLY=yes
# build only the UnitexTool executable target
#
#    you can mix somes parameters
#
#    make SYSTEM=windows UNITEXTOOLONLY=yes
# build only the UnitexTool executable target under Windows
#
#    make SYSTEM=mingw32 UNITEXTOOLONLY=yes
# build only the UnitexTool Windows executable under Linux
#
#    make DEBUG=yes UNITEX_RELEASE_MEMORY_AT_EXIT=yes
# request freeing all memory possible at exit and produce debug version (useful for valgrind)
#
#-----------------------------------
#
# If you want to generate a dynamic library, use:
#
# Linux:    make LIBRARY=yes
# => creates libunitex.so
#
# Windows:  make SYSTEM=windows LIBRARY=yes
# mingw32:  make SYSTEM=mingw32 LIBRARY=yes
# => creates unitex.dll
#
#


################################
##  COMPILATION INSTRUCTIONS  ##
################################
################################

SYSTEM = linux-like
64BITS = no

OBJ_DIR = build/
BIN_DIR = ../bin/


ifeq ($(SYSTEM),linux-like)
###################
###################
##  BEGIN LINUX  ##
###################
###################
CC      = g++
CC_GCC   = gcc
#CC      = gcc -x c
RM      = rm -f
INSTALL = cp
NAME    = unitex-src
VERSION = $(shell date +"%Y-%m-%d")
BUILD   = $(NAME)-$(VERSION)


TAR_COMMAND = tar
TAR_PARAM = xzf

INSTALL_COMMAND = make install
TRE_SRC = tre-0.8.0
TRE_TGZ = ../tre-0.8.0.tar.gz
TRE_TEMP = $(TRE_SRC)/TEMP
INSTALL_COMMAND = make install

SYSLIBLOGGER = DirLoggerPosix.o SyncLoggerPosix.o

#APPDIR = where to install binaries
APPDIR = ../../../App

#
# USING TRE LIBRARY
#
DEFFLAGS = -D_NOT_UNDER_WINDOWS
#                                 -DDEBUG_STAT
#                                 -DBDB
CFLAGS   = -Wall -W
ifeq ($(DEBUG), yes)
CFLAGS += -g -O0
else
CFLAGS += -O2
endif

ifeq ($(LIBRARY),yes)
CFLAGS += -fPIC
DEFFLAGS += -DUNITEX_LIBRARY
endif

ifeq ($(UNITEX_RELEASE_MEMORY_AT_EXIT),yes)
DEFFLAGS += -DUNITEX_RELEASE_MEMORY_AT_EXIT
endif


override CFLAGS   += $(DEFFLAGS) -I"../include_tre" -Ilibtre/include -I"../korean" -I".."
LIBTRE   = .libtre-done
CLEAN-LIBTRE   = clean-libtre
LIBS   = -Llibtre/lib -ltre -lpthread


OPTIONS = $(CFLAGS) $(LDFLAGS) $(LIBS)
EXTENSION =

###################
###################
##  END LINUX  ##
###################
###################

else

ifeq ($(SYSTEM), mingw32)

#####################
#####################
##  MINGW32        ##
#####################
#####################

ifeq ($(COMMANDPREFIXDEFINED),yes)
CC     = $(COMMANDPREFIX)g++
CC_GCC = $(COMMANDPREFIX)gcc
AR     = $(COMMANDPREFIX)ar
else
CC     = i386-mingw32msvc-g++
CC_GCC = i386-mingw32msvc-gcc
AR     = i386-mingw32msvc-ar
endif

RM      = rm -f
INSTALL = cp
TAR_COMMAND = tar
TAR_PARAM = xzf

NAME    = unitex-src
VERSION = $(shell date +"%Y-%m-%d")
BUILD   = $(NAME)-$(VERSION)

TRE_OPTIONS = -DHAVE_CONFIG_H=1 -I../include_tre -O3 -Wall -c -fmessage-length=0

#
# Compiling instructions for the TRE library
#
INSTALL_COMMAND = make install

LIBS =  -L "." -ltre
INCS =  -I"../korean" -I".." -I"../include_tre"
#
# Note: the -I"." inclusion is intended to deal with compilation directory problem
#       while compiling Korean programs
#
CXXINCS =  -I"../korean" -I".."
CXXFLAGS = $(CXXINCS)  -Wall
CFLAGS = $(INCS)  -Wall

ifeq ($(DEBUG), yes)
CFLAGS += -g -O0
else
CFLAGS += -O2
endif


LIBTRE   = libtre.a

# -fPIC is not required because all code is position independent under Windows
#ifeq ($(LIBRARY),yes)
#CFLAGS += -fPIC
#endif

ifeq ($(LIBRARY),yes)
CFLAGS += -DUNITEX_LIBRARY
endif

SYSLIBLOGGER = DirLoggerWin.o SyncLoggerWin.o

APPDIR = ../../../App

OPTIONS = $(CFLAGS) $(LDFLAGS) $(LIBS)
EXTENSION = .exe


#####################
#####################
##  END MINGW32    ##
#####################
#####################


else


#####################
#####################
##  BEGIN WINDOWS  ##
#####################
#####################

ifeq ($(COMMANDPREFIXDEFINED),yes)
CC     = $(COMMANDPREFIX)g++.exe
CC_GCC = $(COMMANDPREFIX)gcc.exe
AR     = $(COMMANDPREFIX)ar.exe
else
CC   = g++.exe
CC_GCC = gcc.exe
AR   = ar.exe
endif

TAR_COMMAND = untgz.exe
TAR_PARAM =

SYSLIBLOGGER = DirLoggerWin.o SyncLoggerWin.o

APPDIR = ../../../App

ifneq ($(DEVCPP_PATHDEFINED),yes)
DEVCPP_PATH = C:/Dev-Cpp
endif



TRE_OPTIONS = -DHAVE_CONFIG_H=1 -I../include_tre -O3 -Wall -c -fmessage-length=0


#
# USING TRE STATIC LIBRARY
#
LIBS =  -L"$(DEVCPP_PATH)/lib" -L"." -ltre
INCS =  -I"$(DEVCPP_PATH)/include" -I"../include_tre" -I"../korean" -I".."
# Note: the -I".." inclusion is intended to deal with compilation directory problem
#       while compiling Korean programs
CXXINCS =  -I"$(DEVCPP_PATH)/include/c++"  -I"$(DEVCPP_PATH)/include/c++/mingw32"  -I"$(DEVCPP_PATH)/include/c++/backward"  -I"$(DEVCPP_PATH)/include"  -I"../include_tre" -I"../korean" -I".."
CXXFLAGS = $(CXXINCS)  -Wall
CFLAGS = $(INCS)  -Wall

ifeq ($(DEBUG), yes)
CFLAGS += -g -O0
else
CFLAGS += -O2
endif

LIBTRE   = libtre.a

# -fPIC is not required because all code is position independent under Windows
#ifeq ($(LIBRARY),yes)
#CFLAGS += -fPIC
#endif

ifeq ($(LIBRARY),yes)
CFLAGS += -DUNITEX_LIBRARY
endif

OPTIONS = $(LIBS) $(CFLAGS) $(LDFLAGS)
EXTENSION = .exe

###################
###################
##  END WINDOWS  ##
###################
###################
endif

endif ## MINGW (ifeq)

##################################
##################################
##  OBJECTS NEEDED BY PROGRAMS  ##
##################################
##################################

TEST      = Test
TEST_OBJS = Main_Test.o Af_stdio.o ActivityLogger.o Unicode.o Pattern.o String_hash.o IOBuffer.o Error.o \
			List_ustring.o StringParsing.o PatternTree.o List_pointer.o DELA.o\
			Alphabet.o Fst2.o Transitions.o List_int.o Tfst.o File.o SingleGraph.o\
			BitArray.o HashTable.o FIFO.o BitMasks.o Ustring.o Symbol.o\
			LanguageDefinition.o Tagset.o AbstractFst2Load.o

BUILDKRMWUDIC      = BuildKrMwuDic
BUILDKRMWUDIC_OBJS = Main_BuildKrMwuDic.o BuildKrMwuDic.o IOBuffer.o Af_stdio.o ActivityLogger.o\
					 Unicode.o Error.o getopt.o

CHECKDIC      = CheckDic
CHECKDIC_OBJS = Main_CheckDic.o CheckDic.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o File.o DELA.o Alphabet.o String_hash.o\
				List_ustring.o Error.o StringParsing.o getopt.o

COMPRESS      = Compress
COMPRESS_OBJS = Main_Compress.o Compress.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o DELA.o DictionaryTree.o Alphabet.o\
				String_hash.o AutomatonDictionary2Bin.o File.o List_int.o\
				List_ustring.o Error.o StringParsing.o getopt.o ProgramInvoker.o

CONCORD      = Concord
CONCORD_OBJS = Main_Concord.o Concord.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Text_tokens.o String_hash.o\
				List_int.o Alphabet.o LocateMatches.o Match.o Concordance.o File.o\
				Text_parsing.o UserCancelling.o MorphologicalLocate.o LocatePattern.o\
				TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o\
				CompoundWordTree.o MorphologicalFilters.o DELA.o List_ustring.o Fst2.o \
				AbstractDelaLoad.o AbstractFst2Load.o Pattern.o LocateFst2Tags.o Error.o SortTxt.o\
				Buffer.o StringParsing.o Snt.o BitArray.o LemmaTree.o\
				List_pointer.o PatternTree.o BitMasks.o Tokenization.o Thai.o Contexts.o\
				Stack_unichar.o Transitions.o DicVariables.o\
				Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o\
				ProgramInvoker.o NewLineShifts.o jamoCodage.o codeForKorean.o pro_type.o \
                  orgUniMbcsMap.o segtype.o etc.o Korean.o bitmap.o

CONCORDIFF      = ConcorDiff
CONCORDIFF_OBJS = Main_ConcorDiff.o ConcorDiff.o IOBuffer.o Diff.o Concord.o Af_stdio.o ActivityLogger.o Unicode.o Text_tokens.o String_hash.o List_int.o\
				Alphabet.o LocateMatches.o Match.o Concordance.o File.o\
				Text_parsing.o UserCancelling.o MorphologicalLocate.o LocatePattern.o TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o\
				CompoundWordTree.o MorphologicalFilters.o\
				DELA.o List_ustring.o Fst2.o AbstractDelaLoad.o AbstractFst2Load.o Pattern.o LocateFst2Tags.o\
				Error.o SortTxt.o\
				Buffer.o StringParsing.o Snt.o BitArray.o LemmaTree.o List_pointer.o BitMasks.o\
				PatternTree.o Tokenization.o Thai.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
				Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o\
				ProgramInvoker.o NewLineShifts.o jamoCodage.o codeForKorean.o pro_type.o \
                  orgUniMbcsMap.o segtype.o etc.o Korean.o bitmap.o

CONVERT      = Convert
CONVERT_OBJS = Main_Convert.o Convert.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o File.o CodePages.o orgUniMbcsMap.o Error.o\
				HTMLCharacters.o AsciiSearchTree.o getopt.o

DICO      = Dico
DICO_OBJS = Main_Dico.o Dico.o List_int.o DELA.o ApplyDic.o File.o Alphabet.o String_hash.o Text_tokens.o \
            List_ustring.o Af_stdio.o ActivityLogger.o Unicode.o CompoundWordHashTable.o Fst2.o AbstractDelaLoad.o AbstractFst2Load.o \
            IOBuffer.o Error.o CompoundWordTree.o\
            Pattern.o ParsingInfo.o LocatePattern.o LocateMatches.o Match.o OptimizedFst2.o \
            Text_parsing.o UserCancelling.o MorphologicalLocate.o TransductionVariables.o TransductionStack.o MorphologicalFilters.o Locate.o\
            StringParsing.o Buffer.o BitArray.o Snt.o LemmaTree.o List_pointer.o PatternTree.o\
            LocateFst2Tags.o BitMasks.o Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
            Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o ProgramInvoker.o\
            jamoCodage.o codeForKorean.o pro_type.o \
            orgUniMbcsMap.o segtype.o etc.o Korean.o bitmap.o SortTxt.o Thai.o Compress.o DictionaryTree.o\
            AutomatonDictionary2Bin.o 

ELAG      = Elag
ELAG_OBJS = Main_Elag.o Elag.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o ElagFunctions.o ElagFstFilesIO.o AutDeterminization.o \
            AutMinimization.o AutConcat.o Fst2Automaton.o \
            Symbol.o String_hash.o Tagset.o Ustring.o \
   		    ElagStateSet.o SymbolAlphabet.o Symbol_op.o Error.o LanguageDefinition.o\
            AutIntersection.o Alphabet.o StringParsing.o DELA.o List_ustring.o\
            List_int.o SingleGraph.o HashTable.o BitArray.o Fst2.o Pattern.o\
            BitMasks.o FIFO.o Transitions.o ElagDebug.o File.o getopt.o Tfst.o

ELAGCOMP      = ElagComp
ELAGCOMP_OBJS = Main_ElagComp.o ElagComp.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o ElagRulesCompilation.o Fst2Automaton.o\
                ElagFstFilesIO.o String_hash.o\
                Symbol.o Ustring.o AutDeterminization.o AutIntersection.o\
                ElagStateSet.o Symbol_op.o LanguageDefinition.o Tagset.o \
	            AutMinimization.o SymbolAlphabet.o AutConcat.o AutComplementation.o\
	            Alphabet.o Error.o StringParsing.o DELA.o List_ustring.o List_int.o\
	            SingleGraph.o HashTable.o BitArray.o Fst2.o Pattern.o BitMasks.o FIFO.o File.o\
	            Transitions.o ElagDebug.o getopt.o Tfst.o

EVAMB      = Evamb
EVAMB_OBJS = Main_Evamb.o Evamb.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o String_hash.o\
             Alphabet.o Error.o StringParsing.o Pattern.o List_ustring.o\
             List_int.o BitMasks.o DELA.o List_pointer.o SingleGraph.o\
             BitArray.o HashTable.o FIFO.o Transitions.o\
             Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o Tfst.o File.o

EXTRACT      = Extract
EXTRACT_OBJS = Main_Extract.o Extract.o IOBuffer.o LocatePattern.o Text_parsing.o UserCancelling.o MorphologicalLocate.o TransductionStack.o OptimizedFst2.o TransductionVariables.o \
               ParsingInfo.o CompoundWordTree.o  List_int.o Concordance.o \
               DELA.o List_ustring.o Fst2.o Pattern.o Af_stdio.o ActivityLogger.o Unicode.o AbstractDelaLoad.o AbstractFst2Load.o\
               Text_tokens.o File.o ExtractUnits.o Alphabet.o String_hash.o LocateMatches.o Match.o\
	           MorphologicalFilters.o Error.o Buffer.o SortTxt.o StringParsing.o BitArray.o\
	           LemmaTree.o Snt.o List_pointer.o PatternTree.o LocateFst2Tags.o BitMasks.o\
	           Tokenization.o Thai.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
	           Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o\
	           ProgramInvoker.o NewLineShifts.o jamoCodage.o codeForKorean.o pro_type.o \
               orgUniMbcsMap.o segtype.o etc.o Korean.o bitmap.o

FLATTEN      = Flatten
FLATTEN_OBJS = Main_Flatten.o Flatten.o IOBuffer.o Alphabet.o Af_stdio.o ActivityLogger.o Unicode.o Fst2.o Grf2Fst2_lib.o\
               AbstractFst2Load.o FlattenFst2.o List_int.o Error.o File.o SingleGraph.o\
               BitMasks.o BitArray.o HashTable.o FIFO.o Pattern.o List_ustring.o\
               String_hash.o StringParsing.o Transitions.o DELA.o\
               Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o

FST2LIST      = Fst2List
FST2LIST_OBJS = Main_Fst2List.o Fst2List.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Fst2.o AbstractFst2Load.o Alphabet.o\
                File.o String_hash.o Error.o TransductionVariables.o\
                StringParsing.o Pattern.o List_ustring.o List_int.o BitMasks.o\
                Transitions.o DELA.o Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o

FST2TXT      = Fst2Txt
FST2TXT_OBJS = Main_Fst2Txt.o Fst2Txt.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Fst2.o\
               AbstractFst2Load.o Alphabet.o ParsingInfo.o\
               TransductionVariables.o String_hash.o Error.o StringParsing.o\
               Pattern.o List_ustring.o List_int.o BitMasks.o BitArray.o\
               Fst2Txt_TokenTree.o Buffer.o Transitions.o File.o \
	           Fst2TxtAsRoutine.o DELA.o DicVariables.o TransductionStack.o\
	           Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o Stack_unichar.o\
	           jamoCodage.o codeForKorean.o pro_type.o \
               orgUniMbcsMap.o segtype.o etc.o Korean.o bitmap.o

GRF2FST2      = Grf2Fst2
GRF2FST2_OBJS = Main_Grf2Fst2.o Grf2Fst2.o IOBuffer.o Alphabet.o Af_stdio.o ActivityLogger.o Unicode.o Fst2.o AbstractFst2Load.o \
                Fst2Check.o Grf2Fst2_lib.o Error.o File.o SingleGraph.o\
                BitMasks.o BitArray.o List_int.o HashTable.o FIFO.o Pattern.o\
                List_ustring.o String_hash.o StringParsing.o Transitions.o DELA.o\
                Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o\
                ProgramInvoker.o


IMPLODETFST      = ImplodeTfst
IMPLODETFST_OBJS = Main_ImplodeTfst.o ImplodeTfst.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Tfst.o File.o SingleGraph.o\
                   String_hash.o Alphabet.o Error.o StringParsing.o Pattern.o List_ustring.o\
                   List_int.o BitMasks.o DELA.o List_pointer.o Transitions.o\
                   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o\
                   BitArray.o HashTable.o FIFO.o Match.o

LOCATE      = Locate
LOCATE_OBJS = Main_Locate.o Locate.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Alphabet.o DELA.o List_ustring.o String_hash.o \
              LocatePattern.o AbstractDelaLoad.o AbstractFst2Load.o Fst2.o Text_tokens.o List_int.o\
              Pattern.o CompoundWordTree.o LocateFst2Tags.o \
              OptimizedFst2.o Text_parsing.o UserCancelling.o MorphologicalLocate.o LocateMatches.o \
              Match.o TransductionVariables.o TransductionStack.o\
              ParsingInfo.o File.o MorphologicalFilters.o Error.o StringParsing.o\
              BitArray.o LemmaTree.o List_pointer.o PatternTree.o BitMasks.o Buffer.o\
              Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
              Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o \
              ProgramInvoker.o jamoCodage.o codeForKorean.o pro_type.o \
              orgUniMbcsMap.o segtype.o etc.o Korean.o bitmap.o

LOCATETFST      = LocateTfst
LOCATETFST_OBJS = Main_LocateTfst.o LocateTfst.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Error.o getopt.o LocateTfst_lib.o \
                  Tfst.o File.o Ustring.o SingleGraph.o List_int.o BitMasks.o FIFO.o Transitions.o \
                  Symbol.o LanguageDefinition.o Tagset.o String_hash.o StringParsing.o DELA.o \
                  Alphabet.o List_ustring.o HashTable.o BitArray.o Fst2.o AbstractFst2Load.o Pattern.o \
                  LocateTfstMatches.o Match.o MorphologicalFilters.o jamoCodage.o codeForKorean.o pro_type.o \
                  orgUniMbcsMap.o segtype.o etc.o Korean.o OptimizedTfstTagMatching.o List_pointer.o\
                  TransductionVariables.o TransductionStackTfst.o Contexts.o bitmap.o


MULTIFLEX      = MultiFlex
MULTIFLEX_OBJS = Main_MultiFlex.o MultiFlex.o Alphabet.o MF_DLC_inflect.o MF_DicoMorpho.o MF_FormMorpho.o MF_LangMorpho.o\
                 MF_InflectTransd.o  MF_MU_graph.o MF_MU_morpho.o MF_SU_morpho.o MF_Unif.o MF_Util.o\
                 Fst2.o AbstractFst2Load.o Af_stdio.o ActivityLogger.o Unicode.o Error.o IOBuffer.o File.o DELA.o StringParsing.o List_ustring.o\
                 String_hash.o Pattern.o List_int.o BitMasks.o Transitions.o Grf2Fst2.o\
                 Grf2Fst2_lib.o SingleGraph.o BitArray.o FIFO.o HashTable.o Fst2Check.o\
                 Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o\
                 MF_Operators_Util.o getopt.o ProgramInvoker.o jamoCodage.o codeForKorean.o pro_type.o \
                  orgUniMbcsMap.o segtype.o etc.o Korean.o bitmap.o

NORMALIZE      = Normalize
NORMALIZE_OBJS = Main_Normalize.o Normalize.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o File.o DELA.o List_ustring.o String_hash.o\
				Alphabet.o Error.o StringParsing.o Buffer.o NormalizeAsRoutine.o getopt.o

POLYLEX      = PolyLex
POLYLEX_OBJS = Main_PolyLex.o PolyLex.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Alphabet.o DELA.o AbstractDelaLoad.o List_ustring.o String_hash.o\
				NorwegianCompounds.o GermanCompounds.o File.o\
				RussianCompounds.o GeneralDerivation.o Error.o StringParsing.o\
				DutchCompounds.o getopt.o

REBUILDTFST      = RebuildTfst
REBUILDTFST_OBJS = Main_RebuildTfst.o RebuildTfst.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o String_hash.o AbstractFst2Load.o Fst2.o\
                   File.o Alphabet.o Error.o StringParsing.o\
                   Pattern.o List_ustring.o List_int.o BitMasks.o BitArray.o\
                   Transitions.o DELA.o\
                   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o\
                   getopt.o Tfst.o Grf2Fst2.o Grf2Fst2_lib.o SingleGraph.o\
                   HashTable.o FIFO.o Fst2Check.o ProgramInvoker.o

RECONSTRUCAO      = Reconstrucao
RECONSTRUCAO_OBJS = Main_Reconstrucao.o Reconstrucao.o OptimizedFst2.o ParsingInfo.o CompoundWordTree.o\
               DELA.o AbstractDelaLoad.o List_ustring.o Fst2.o AbstractFst2Load.o IOBuffer.o TransductionStack.o Pattern.o \
               Af_stdio.o ActivityLogger.o Unicode.o Alphabet.o LocateMatches.o Match.o File.o NormalizationFst2.o\
	            PortugueseNormalization.o TransductionVariables.o \
               Text_parsing.o UserCancelling.o MorphologicalLocate.o String_hash.o LocatePattern.o Text_tokens.o List_int.o\
               Sentence2Grf.o MorphologicalFilters.o Error.o StringParsing.o BitArray.o\
               LemmaTree.o List_pointer.o PatternTree.o LocateFst2Tags.o BitMasks.o\
               Buffer.o Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
               Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o \
               Tfst.o SingleGraph.o FIFO.o HashTable.o jamoCodage.o codeForKorean.o pro_type.o \
               orgUniMbcsMap.o segtype.o etc.o Korean.o bitmap.o

REG2GRF      = Reg2Grf
REG2GRF_OBJS = Main_Reg2Grf.o Reg2Grf.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o RegularExpressions.o Error.o File.o\
               List_int.o Stack_int.o getopt.o

SORTTXT      = SortTxt
SORTTXT_OBJS = Main_SortTxt.o SortTxt.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Error.o Thai.o getopt.o\
               ProgramInvoker.o

STATS      = Stats
STATS_OBJS = Main_Stats.o Stats.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Error.o Thai.o getopt.o\
               ProgramInvoker.o HashTable.o File.o Text_tokens.o String_hash.o \
               LocateMatches.o Match.o DELA.o \
               Alphabet.o StringParsing.o List_int.o List_ustring.o

TABLE2GRF      = Table2Grf
TABLE2GRF_OBJS = Main_Table2Grf.o Table2Grf.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o File.o Error.o getopt.o

TAGSETNORMTFST = TagsetNormTfst
TAGSETNORMTFST_OBJS = Main_TagsetNormTfst.o TagsetNormTfst.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o ElagFstFilesIO.o Ustring.o \
					String_hash.o Symbol.o LanguageDefinition.o Tagset.o\
					Error.o File.o StringParsing.o DELA.o List_ustring.o Alphabet.o\
					List_int.o SingleGraph.o HashTable.o BitArray.o Fst2.o AbstractFst2Load.o Pattern.o\
					BitMasks.o FIFO.o Transitions.o\
					Symbol_op.o Fst2Automaton.o getopt.o Tfst.o Match.o

TEI2TXT      = TEI2Txt
TEI2TXT_OBJS = Main_TEI2Txt.o TEI2Txt.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Error.o File.o HTMLCharacters.o\
               AsciiSearchTree.o getopt.o

TFST2GRF      = Tfst2Grf
TFST2GRF_OBJS = Main_Tfst2Grf.o Tfst2Grf.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Tfst.o Sentence2Grf.o\
               File.o List_int.o Error.o StringParsing.o Pattern.o\
               List_ustring.o String_hash.o BitMasks.o BitArray.o DELA.o\
               Alphabet.o Transitions.o Symbol.o Symbol_op.o LanguageDefinition.o\
               Ustring.o Tagset.o getopt.o SingleGraph.o HashTable.o FIFO.o

TFST2UNAMBIG      = Tfst2Unambig
TFST2UNAMBIG_OBJS = Main_Tfst2Unambig.o Tfst2Unambig.o Tfst.o Error.o IOBuffer.o LinearAutomaton2Txt.o\
                   Af_stdio.o ActivityLogger.o Unicode.o Pattern.o List_ustring.o StringParsing.o String_hash.o\
                   List_int.o BitMasks.o Transitions.o DELA.o Alphabet.o\
                   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o\
                   getopt.o SingleGraph.o BitArray.o HashTable.o FIFO.o File.o

TOKENIZE      = Tokenize
TOKENIZE_OBJS = Main_Tokenize.o Tokenize.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Alphabet.o String_hash.o\
				File.o DELA.o AbstractDelaLoad.o List_ustring.o HashTable.o Error.o StringParsing.o getopt.o

TXT2TFST      = Txt2Tfst
TXT2TFST_OBJS = Main_Txt2Tfst.o Txt2Tfst.o IOBuffer.o Text_tokens.o Alphabet.o Af_stdio.o ActivityLogger.o Unicode.o DELA_tree.o\
                DELA.o List_ustring.o String_hash.o BuildTextAutomaton.o List_int.o\
                NormalizationFst2.o Fst2.o AbstractFst2Load.o File.o Error.o\
                StringParsing.o Pattern.o BitMasks.o Grf2Fst2_lib.o SingleGraph.o\
                FIFO.o BitArray.o HashTable.o Transitions.o LocateMatches.o Match.o\
                Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o\
                Tfst.o NewLineShifts.o ProgramInvoker.o\
                Txt2Fst2Kr.o bitmap.o etc.o segtype.o Korean.o Jamo2Syl.o jamoCodage.o\
                codeForKorean.o pro_type.o orgUniMbcsMap.o \
                Syl2Jamo.o

XMLIZER      = XMLizer
XMLIZER_OBJS = Main_XMLizer.o XMLizer.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o Fst2.o AbstractFst2Load.o Alphabet.o \
               TransductionVariables.o String_hash.o Error.o StringParsing.o File.o\
               Pattern.o List_ustring.o List_int.o BitMasks.o Fst2TxtAsRoutine.o\
               NormalizeAsRoutine.o DELA.o Buffer.o Fst2Txt_TokenTree.o ParsingInfo.o Transitions.o \
	           getopt.o DicVariables.o TransductionStack.o \
	           Symbol.o Symbol_op.o LanguageDefinition.o Tagset.o Ustring.o Stack_unichar.o\
	           jamoCodage.o codeForKorean.o pro_type.o \
               orgUniMbcsMap.o segtype.o etc.o Korean.o bitmap.o


UNCOMPRESS       = Uncompress
UNCOMPRESS_OBJS  = Main_Uncompress.o Uncompress.o File.o Error.o Unicode.o DELA.o AbstractDelaLoad.o \
                   String_hash.o StringParsing.o List_ustring.o Alphabet.o Af_stdio.o ActivityLogger.o\
                   getopt.o IOBuffer.o


UNITEXTOOL   = UnitexTool
UNITEXTOOL_OBJS = Main_UnitexTool.o UnitexTool.o AbstractDelaLoad.o AbstractFst2Load.o Alphabet.o ApplyDic.o AsciiSearchTree.o \
                  AutComplementation.o AutConcat.o AutDeterminization.o AutIntersection.o AutMinimization.o \
                  AutomatonDictionary2Bin.o BitArray.o BitMasks.o Buffer.o BuildTextAutomaton.o CheckDic.o \
                  CodePages.o CompoundWordHashTable.o CompoundWordTree.o Compress.o Concord.o Concordance.o \
                  ConcorDiff.o Contexts.o Convert.o DELA.o DELA_tree.o Dico.o DictionaryTree.o \
                  DicVariables.o Diff.o DutchCompounds.o Elag.o ElagComp.o ElagDebug.o ElagFstFilesIO.o \
                  ElagFunctions.o ElagRulesCompilation.o ElagStateSet.o Error.o Evamb.o Extract.o ExtractUnits.o \
                  FIFO.o File.o Flatten.o FlattenFst2.o Fst2.o Fst2Automaton.o Fst2Check.o Fst2List.o Fst2Txt.o \
                  Fst2Txt_TokenTree.o Fst2TxtAsRoutine.o GeneralDerivation.o GermanCompounds.o getopt.o \
                  Grf2Fst2.o Grf2Fst2_lib.o HashTable.o HTMLCharacters.o ImplodeTfst.o IOBuffer.o \
                  LanguageDefinition.o LemmaTree.o LinearAutomaton2Txt.o List_int.o List_pointer.o List_ustring.o \
                  Locate.o LocateFst2Tags.o LocatePattern.o LocateTfst.o LocateTfst_lib.o LocateTfstMatches.o \
                  LocateMatches.o Match.o MF_DicoMorpho.o MF_DLC_inflect.o MF_FormMorpho.o MF_InflectTransd.o MF_LangMorpho.o \
                  MF_MU_graph.o MF_MU_morpho.o MF_Operators_Util.o MF_SU_morpho.o MF_Unif.o MF_Util.o \
                  MorphologicalFilters.o MorphologicalLocate.o MultiFlex.o NewLineShifts.o NormalizationFst2.o \
                  Normalize.o NormalizeAsRoutine.o NorwegianCompounds.o OptimizedFst2.o ParsingInfo.o Pattern.o \
                  PatternTree.o PolyLex.o PortugueseNormalization.o ProgramInvoker.o RebuildTfst.o Reconstrucao.o \
                  Reg2Grf.o RegularExpressions.o RussianCompounds.o Sentence2Grf.o SingleGraph.o Snt.o SortTxt.o \
                  Stack_int.o Stack_unichar.o Stats.o String_hash.o StringParsing.o Symbol.o Symbol_op.o \
                  SymbolAlphabet.o Table2Grf.o Tagset.o TagsetNormTfst.o TEI2Txt.o Text_parsing.o UserCancelling.o Text_tokens.o Tfst.o \
                  Tfst2Grf.o Tfst2Unambig.o Thai.o Tokenization.o Tokenize.o TransductionStack.o \
                  TransductionVariables.o Transitions.o Txt2Tfst.o Af_stdio.o ActivityLogger.o Unicode.o Ustring.o XMLizer.o DuplicateFile.o \
                  Txt2Fst2Kr.o bitmap.o etc.o segtype.o Korean.o \
                  codeForKorean.o orgUniMbcsMap.o CompressKr.o ConsultDic.o ExtractChar.o InflectKr.o Jamo2Syl.o \
                  jamoCodage.o MergeBin.o pro_type.o SortMorph.o SufForm2Rac.o Syl2Jamo.o \
                  OptimizedTfstTagMatching.o TransductionStackTfst.o Uncompress.o


UNITEXTOOL_LOGGER   = UnitexToolLogger
UNITEXTOOL_LOGGER_OBJS = FilePack.o FilePackCrc32.o FilePackIo.o UniLogger.o UniLoggerAutoInstall.o \
                  FileUnPack.o ReworkArg.o UniRunLogger.o $(SYSLIBLOGGER) \
                  Main_UnitexToolLogger.o UnitexTool.o AbstractDelaLoad.o AbstractFst2Load.o Alphabet.o ApplyDic.o AsciiSearchTree.o \
                  AutComplementation.o AutConcat.o AutDeterminization.o AutIntersection.o AutMinimization.o \
                  AutomatonDictionary2Bin.o BitArray.o BitMasks.o Buffer.o BuildTextAutomaton.o CheckDic.o \
                  CodePages.o CompoundWordHashTable.o CompoundWordTree.o Compress.o Concord.o Concordance.o \
                  ConcorDiff.o Contexts.o Convert.o DELA.o DELA_tree.o Dico.o DictionaryTree.o \
                  DicVariables.o Diff.o DutchCompounds.o Elag.o ElagComp.o ElagDebug.o ElagFstFilesIO.o \
                  ElagFunctions.o ElagRulesCompilation.o ElagStateSet.o Error.o Evamb.o Extract.o ExtractUnits.o \
                  FIFO.o File.o Flatten.o FlattenFst2.o Fst2.o Fst2Automaton.o Fst2Check.o Fst2List.o Fst2Txt.o \
                  Fst2Txt_TokenTree.o Fst2TxtAsRoutine.o GeneralDerivation.o GermanCompounds.o getopt.o \
                  Grf2Fst2.o Grf2Fst2_lib.o HashTable.o HTMLCharacters.o ImplodeTfst.o IOBuffer.o \
                  LanguageDefinition.o LemmaTree.o LinearAutomaton2Txt.o List_int.o List_pointer.o List_ustring.o \
                  Locate.o LocateFst2Tags.o LocatePattern.o LocateTfst.o LocateTfst_lib.o LocateTfstMatches.o \
                  LocateMatches.o Match.o MF_DicoMorpho.o MF_DLC_inflect.o MF_FormMorpho.o MF_InflectTransd.o MF_LangMorpho.o \
                  MF_MU_graph.o MF_MU_morpho.o MF_Operators_Util.o MF_SU_morpho.o MF_Unif.o MF_Util.o \
                  MorphologicalFilters.o MorphologicalLocate.o MultiFlex.o NewLineShifts.o NormalizationFst2.o \
                  Normalize.o NormalizeAsRoutine.o NorwegianCompounds.o OptimizedFst2.o ParsingInfo.o Pattern.o \
                  PatternTree.o PolyLex.o PortugueseNormalization.o ProgramInvoker.o RebuildTfst.o Reconstrucao.o \
                  Reg2Grf.o RegularExpressions.o RussianCompounds.o Sentence2Grf.o SingleGraph.o Snt.o SortTxt.o \
                  Stack_int.o Stack_unichar.o Stats.o String_hash.o StringParsing.o Symbol.o Symbol_op.o \
                  SymbolAlphabet.o Table2Grf.o Tagset.o TagsetNormTfst.o TEI2Txt.o Text_parsing.o UserCancelling.o Text_tokens.o Tfst.o \
                  Tfst2Grf.o Tfst2Unambig.o Thai.o Tokenization.o Tokenize.o TransductionStack.o \
                  TransductionVariables.o Transitions.o Txt2Tfst.o Af_stdio.o ActivityLogger.o Unicode.o Ustring.o XMLizer.o DuplicateFile.o \
                  Txt2Fst2Kr.o bitmap.o etc.o segtype.o Korean.o \
                  codeForKorean.o orgUniMbcsMap.o CompressKr.o ConsultDic.o ExtractChar.o InflectKr.o Jamo2Syl.o \
                  jamoCodage.o MergeBin.o pro_type.o SortMorph.o SufForm2Rac.o Syl2Jamo.o \
                  OptimizedTfstTagMatching.o TransductionStackTfst.o Uncompress.o

######################
## Korean programs  ##
######################

COMPRESSKR	= CompressKr
COMPRESSKR_OBJS = Main_CompressKr.o CompressKr.o IOBuffer.o etc.o pro_type.o \
                  String_hash.o File.o Af_stdio.o ActivityLogger.o Unicode.o Alphabet.o Error.o StringParsing.o

CONSULTDIC = ConsultDic
CONSULTDIC_OBJS = Main_ConsultDic.o ConsultDic.o IOBuffer.o etc.o pro_type.o \
		    Af_stdio.o ActivityLogger.o Unicode.o Fst2.o File.o Error.o Alphabet.o Pattern.o List_ustring.o\
		    StringParsing.o String_hash.o List_int.o BitMasks.o Transitions.o DELA.o\
		    Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o

INFLECTKR = InflectKr
INFLECTKR_OBJS  = Main_InflectKr.o InflectKr.o IOBuffer.o etc.o pro_type.o AbstractFst2Load.o \
                  Af_stdio.o ActivityLogger.o Unicode.o Fst2.o File.o Error.o Pattern.o List_ustring.o\
                  String_hash.o StringParsing.o List_int.o BitMasks.o Transitions.o DELA.o\
                  Alphabet.o Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o

SUFFORM2RAC  = SufForm2Rac
SUFFORM2RAC_OBJS  = Main_SufForm2Rac.o SufForm2Rac.o IOBuffer.o etc.o Af_stdio.o ActivityLogger.o Unicode.o File.o Error.o

MERGEBIN  = MergeBin
MERGEBIN_OBJS  = Main_MergeBin.o MergeBin.o IOBuffer.o etc.o pro_type.o Af_stdio.o ActivityLogger.o Unicode.o File.o Error.o

SORTMORPH  = SortMorph
SORTMORPH_OBJS  = Main_SortMorph.o SortMorph.o IOBuffer.o etc.o File.o Af_stdio.o ActivityLogger.o Unicode.o Error.o

JAMO2SYL  = Jamo2Syl
JAMO2SYL_OBJS  = Main_Jamo2Syl.o Jamo2Syl.o IOBuffer.o etc.o bitmap.o \
                  codeForKorean.o orgUniMbcsMap.o segtype.o \
                  pro_type.o jamoCodage.o AbstractFst2Load.o \
                  Af_stdio.o ActivityLogger.o Unicode.o File.o Fst2.o Error.o Pattern.o\
                  List_ustring.o String_hash.o StringParsing.o List_int.o BitMasks.o\
                  Transitions.o DELA.o Alphabet.o\
                  Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o

SYL2JAMO  = Syl2Jamo
SYL2JAMO_OBJS  = Main_Syl2Jamo.o Syl2Jamo.o IOBuffer.o jamoCodage.o etc.o \
                 pro_type.o codeForKorean.o orgUniMbcsMap.o segtype.o \
                 Af_stdio.o ActivityLogger.o Unicode.o File.o Error.o

TXT2FST2KR  = Txt2Fst2Kr
TXT2FST2KR_OBJS  = Main_Txt2Fst2Kr.o Txt2Fst2Kr.o IOBuffer.o bitmap.o etc.o segtype.o \
		   Af_stdio.o ActivityLogger.o Unicode.o File.o Fst2.o AbstractFst2Load.o Error.o Pattern.o List_ustring.o\
		   String_hash.o StringParsing.o List_int.o BitMasks.o Transitions.o DELA.o Alphabet.o\
		   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o

EXTRACTCHAR  = ExtractChar
EXTRACTCHAR_OBJS  = Main_ExtractChar.o ExtractChar.o IOBuffer.o codeForKorean.o orgUniMbcsMap.o bitmap.o etc.o segtype.o \
		            jamoCodage.o pro_type.o Af_stdio.o ActivityLogger.o Unicode.o File.o Fst2.o AbstractFst2Load.o Error.o\
		            Pattern.o List_ustring.o String_hash.o StringParsing.o List_int.o BitMasks.o\
		            Transitions.o DELA.o Alphabet.o\
		            Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o


################################
################################
##  PROGRAM AND OBJECT LISTS  ##
################################
################################

PROGS = $(TEST)

ifeq ($(UNITEXTOOLONLY), yes)


PROGS = $(UNITEXTOOL) $(UNITEXTOOL_LOGGER)
OBJS = $(UNITEXTOOL_OBJS) $(UNITEXTOOL_LOGGER_OBJS)

else

PROGS = $(BUILDKRMWUDIC) $(CHECKDIC) $(COMPRESS) $(CONCORD) $(CONCORDIFF) $(CONVERT) $(DICO)\
		$(ELAG) $(ELAGCOMP) $(EVAMB) $(EXTRACT) $(FLATTEN) \
		$(FST2LIST) $(FST2TXT) $(GRF2FST2) $(IMPLODETFST) $(LOCATE) $(LOCATETFST) $(MULTIFLEX)\
		$(NORMALIZE) $(POLYLEX) $(REBUILDTFST) $(RECONSTRUCAO) \
		$(REG2GRF) $(SORTTXT) $(STATS) $(TABLE2GRF) $(TAGSETNORMTFST) $(TEI2TXT) \
		$(TFST2GRF) $(TFST2UNAMBIG) $(TOKENIZE) $(TXT2TFST) $(XMLIZER) \
		$(UNCOMPRESS) $(UNITEXTOOL) $(UNITEXTOOL_LOGGER) $(TEST)

ifeq ($(64BITS),no)
PROGS += $(INFLECTKR) $(SUFFORM2RAC) $(COMPRESSKR) $(MERGEBIN) $(CONSULTDIC) $(SORTMORPH) \
         $(TXT2FST2KR) $(JAMO2SYL) $(SYL2JAMO) $(EXTRACTCHAR)
endif

OBJS  = $(BUILDKRMWUDIC_OBJS) $(CHECKDIC_OBJS) $(COMPRESS_OBJS) $(CONCORD_OBJS) $(CONCORDIFF_OBJS) $(CONVERT_OBJS) \
        $(DICO_OBJS) $(ELAG_OBJS)\
		$(ELAGCOMP_OBJS) $(EVAMB_OBJS) \
		$(EXTRACT_OBJS) $(FLATTEN_OBJS) \
		$(FST2LIST_OBJS) $(FST2TXT_OBJS) $(GRF2FST2_OBJS) $(IMPLODETFST_OBJS) \
		$(LOCATE_OBJS) \
		$(MULTIFLEX_OBJS) $(NORMALIZE_OBJS) $(POLYLEX_OBJS) $(REBUILDTFST_OBJS) \
		$(RECONSTRUCAO_OBJS) $(REG2GRF_OBJS) $(SORTTXT_OBJS) $(STATS_OBJS) $(TABLE2GRF_OBJS)\
		$(TAGSETNORMTFST_OBJS) $(TEST_OBJS) $(TEI2TXT_OBJS) $(TFST2GRF_OBJS) \
		$(TFST2UNAMBIG_OBJS) $(TOKENIZE_OBJS) $(TXT2TFST_OBJS) $(XMLIZER_OBJS) \
		$(UNCOMPRESS_OBJS) $(UNITEXTOOL_OBJS) $(UNITEXTOOL_LOGGER_OBJS)

		#$(COMPRESSKR_OBJS) $(CONSULTDIC_OBJS) $(INFLECTKR_OBJS) $(SUFFORM2RAC_OBJS) \
		#$(MERGEBIN_OBJS) $(SORTMORPH_OBJS) $(TXT2FST2KR_OBJS) \
		#$(JAMO2SYL_OBJS) $(SYL2JAMO_OBJS) $(EXTRACTCHAR_OBJS)

endif

BINARIES = $(patsubst %,$(BIN_DIR)%$(EXTENSION),$(PROGS))


###########################
###########################
##  COMPILATION TARGETS  ##
###########################
###########################

ifeq ($(SYSTEM),linux-like)
#####################
#####################
##  LINUX TARGETS  ##
#####################
#####################

ifeq ($(LIBRARY),yes)
all: $(LIBTRE) .libunitex
else
all: bin
endif

bin: $(LIBTRE) $(BINARIES)
	@echo
	@echo "All is done. Type '$(INSTALL_COMMAND)' to install programs in '$(APPDIR)'."
	@echo

%.o: ../%.cpp
	$(CC) -c $(CFLAGS) $<

$(TRE_SRC): $(TRE_TGZ)
	@echo "unpacking TRE library ..."
	$(TAR_COMMAND) $(TAR_PARAM) $(TRE_TGZ)

.libtre-done: $(TRE_SRC)
	@echo "Making TRE library ..."
	chmod +x $(TRE_SRC)/configure
	cd $(TRE_SRC) && ./configure --disable-shared --enable-static && make
	mkdir -p libtre/lib libtre/include
	cd $(TRE_SRC)/lib && ar rcs libtre.a *.o
	cp $(TRE_SRC)/lib/libtre.a  libtre/lib/libtre.a
	touch .libtre-done

clean-libtre:
	rm -rf libtre
	rm -rf $(TRE_SRC)
	rm -f .libtre-done


install: bin
	$(INSTALL) $(BINARIES) $(APPDIR)


clean: $(CLEAN-LIBTRE)
	rm -f $(OBJS) $(PROGS) *~ korean/*.o korean/*~ Main_*.o

clean-all-but-tre:
	rm -f $(OBJS) $(PROGS) *~ korean/*.o korean/*~ Main_*.o

dist: clean
	if test -d $(BUILD); then rm -rf $(BUILD); fi
	mkdir $(BUILD)
	cp -a $(shell find . -mindepth 1 -maxdepth 1 -name $(BUILD) -prune -o -print) $(BUILD)
	tar cf - $(BUILD)  | gzip > ../$(BUILD).tar.gz
	rm -rf $(BUILD)

zip: clean
	if test -d $(BUILD); then rm -rf $(BUILD); fi
	mkdir $(BUILD)
	cp -a $(shell find . -mindepth 1 -maxdepth 1 -name $(BUILD) -prune -o -print) $(BUILD)
	zip -r -q ../$(BUILD).zip $(BUILD)
	rm -rf $(BUILD)

remake: clean all


##########################
##########################
##  END LINUX  TARGETS  ##
##########################
##########################

else

################################
################################
##  WINDOWS or MINGW TARGETS  ##
################################
################################


ifeq ($(LIBRARY),yes)
all: $(LIBTRE) .libunitex
else
all: bin
endif


TRE_LIB = libtre.a

ifeq ($(LIBRARY),OOOfdsjnfezqez)
tre-0.8.0/lib/tre-stack.c: $(TRE_SRC)

tre-0.8.0/lib/tre-parse.c: $(TRE_SRC)

tre-0.8.0/lib/regexec.c: $(TRE_SRC)

tre-0.8.0/lib/xmalloc.c: $(TRE_SRC)

tre-0.8.0/lib/tre-match-parallel.c: $(TRE_SRC)

tre-0.8.0/lib/tre-match-approx.c: $(TRE_SRC)

tre-0.8.0/lib/tre-ast.c: $(TRE_SRC)

tre-0.8.0/lib/regcomp.c: $(TRE_SRC)

tre-0.8.0/lib/regerror.c: $(TRE_SRC)

tre-0.8.0/lib/tre-compile.c: $(TRE_SRC)

TRE_OBJS = tre-parse.o tre-stack.o regexec.o xmalloc.o tre-match-parallel.o tre-match-approx.o tre-ast.o \
	regcomp.o regerror.o tre-ast.o tre-compile.o tre-match-backtrack.o tre-mem.o

tre-stack.o: tre-0.8.0/lib/tre-stack.c
	$(CC_GCC) -c tre-0.8.0/lib/tre-stack.c $(TRE_OPTIONS)

tre-parse.o: tre-0.8.0/lib/tre-parse.c
	$(CC_GCC) -c tre-0.8.0/lib/tre-parse.c $(TRE_OPTIONS)

regexec.o: tre-0.8.0/lib/regexec.c
	$(CC_GCC) -c tre-0.8.0/lib/regexec.c $(TRE_OPTIONS)

xmalloc.o: tre-0.8.0/lib/xmalloc.c
	$(CC_GCC) -c tre-0.8.0/lib/xmalloc.c $(TRE_OPTIONS)

tre-match-parallel.o: tre-0.8.0/lib/tre-match-parallel.c
	$(CC_GCC) -c tre-0.8.0/lib/tre-match-parallel.c $(TRE_OPTIONS)

tre-match-approx.o: tre-0.8.0/lib/tre-match-approx.c
	$(CC_GCC) -c tre-0.8.0/lib/tre-match-approx.c $(TRE_OPTIONS)

tre-ast.o: tre-0.8.0/lib/tre-ast.c
	$(CC_GCC) -c tre-0.8.0/lib/tre-ast.c $(TRE_OPTIONS)

regcomp.o: tre-0.8.0/lib/regcomp.c
	$(CC_GCC) -c tre-0.8.0/lib/regcomp.c $(TRE_OPTIONS)

regerror.o: tre-0.8.0/lib/regerror.c
	$(CC_GCC) -c tre-0.8.0/lib/regerror.c $(TRE_OPTIONS)

tre-compile.o: tre-0.8.0/lib/tre-compile.c
	$(CC_GCC) -c tre-0.8.0/lib/tre-compile.c $(TRE_OPTIONS)

tre-match-backtrack.o: tre-0.8.0/lib/tre-match-backtrack.c
	$(CC_GCC) -c tre-0.8.0/lib/tre-match-backtrack.c $(TRE_OPTIONS)

tre-mem.o: tre-0.8.0/lib/tre-mem.c
	$(CC_GCC) -c tre-0.8.0/lib/tre-mem.c $(TRE_OPTIONS)

$(TRE_LIB): $(TRE_SRC) $(TRE_OBJS)
	AR -r libtre.a $(TRE_OBJS)

else
TRE_OBJS = tre-parse.o tre-stack.o regexec.o xmalloc.o tre-match-parallel.o tre-match-approx.o tre-ast.o \
	regcomp.o regerror.o tre-ast.o tre-compile.o tre-match-backtrack.o tre-mem.o

$(TRE_LIB): $(TRE_TGZ)
	@echo "unpacking TRE library......"
	$(TAR_COMMAND) $(TAR_PARAM) $(TRE_TGZ)
	$(CC_GCC) -c tre-0.8.0/lib/tre-stack.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-parse.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/regexec.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/xmalloc.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-match-parallel.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-match-approx.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-ast.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/regcomp.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/regerror.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-compile.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-match-backtrack.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-mem.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-stack.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-parse.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/regexec.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/xmalloc.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-match-parallel.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-match-approx.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-ast.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/regcomp.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/regerror.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-compile.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-match-backtrack.c $(TRE_OPTIONS)
	$(CC_GCC) -c tre-0.8.0/lib/tre-mem.c $(TRE_OPTIONS)
	$(AR) -r libtre.a $(TRE_OBJS)
	
endif


TRE_SRC = tre-0.8.0
TRE_TGZ = ../tre-0.8.0.tar.gz


ifeq ($(SYSTEM),windows)

#######################
#######################
##  WINDOWS TARGETS  ##
#######################
#######################


bin: $(LIBTRE) $(BINARIES)
	@echo ----------------------------------------------------------------------------
	@echo All is done. You need to move .exe programs from '$(BIN_DIR)' to '$(APPDIR)'.


TRE_TEMP = $(TRE_SRC)/TEMP

$(TRE_SRC): $(TRE_TGZ)
	@echo "unpacking TRE library...."
	$(TAR_COMMAND) $(TAR_PARAM) $(TRE_TGZ)


LIBTRE: $(TRE_SRC) $(TRE_LIB)
	@echo "Making TRE library..."

CLEAN-LIBTRE   =
LIBS   = -L"." -ltre

%.o: ../%.cpp
	$(CC) -c $(CFLAGS) $<

clean-all-but-tre:
	rm -f *.o $(BIN_DIR)*.exe $(BIN_DIR)unitex.dll

clean: clean-all-but-tre
	rm -rf tre-0.8.0
	rm libtre.a


##########################
##########################
##  END WINDOWS TARGETS ##
##########################
##########################
else

#############################
#############################
##  MINGW WINDOWS TARGETS  ##
#############################
#############################

bin: $(LIBTRE) $(BINARIES)
	@echo
	@echo "All is done. Type '$(INSTALL_COMMAND)' to install programs in '$(APPDIR)'."
	@echo


TRE_TEMP = $(TRE_SRC)/TEMP

$(TRE_SRC): $(TRE_TGZ)
	@echo "Unpacking TRE library...."
	$(TAR_COMMAND) $(TAR_PARAM) $(TRE_TGZ)


LIBTRE: $(TRE_SRC) $(TRE_LIB)
	@echo "Making TRE library..."

CLEAN-LIBTRE   =
LIBS   = -L"." -ltre

%.o: ../%.cpp
	$(CC) -c $(CFLAGS) $<

clean-all-but-tre:
	rm -f $(OBJ_DIR)$(OBJS) $(BIN_DIR)*.exe $(BIN_DIR)unitex.dll Main_*.o

clean: clean-all-but-tre
	rm -rf tre-0.8.0
	rm libtre.a


#################################
#################################
##  END MINGW WINDOWS TARGETS  ##
#################################
#################################
endif
endif

$(BIN_DIR)$(TEST)$(EXTENSION): $(TEST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(BUILDKRMWUDIC)$(EXTENSION): $(BUILDKRMWUDIC_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(CHECKDIC)$(EXTENSION): $(CHECKDIC_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(COMPRESS)$(EXTENSION): $(COMPRESS_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(CONCORD)$(EXTENSION): $(CONCORD_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(CONCORDIFF)$(EXTENSION): $(CONCORDIFF_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(CONVERT)$(EXTENSION): $(CONVERT_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(DICO)$(EXTENSION): $(DICO_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(ELAG)$(EXTENSION): $(ELAG_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(ELAGCOMP)$(EXTENSION): $(ELAGCOMP_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(EVAMB)$(EXTENSION): $(EVAMB_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(EXTRACT)$(EXTENSION): $(EXTRACT_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(FLATTEN)$(EXTENSION): $(FLATTEN_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(FST2LIST)$(EXTENSION): $(FST2LIST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(FST2TXT)$(EXTENSION): $(FST2TXT_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(GRF2FST2)$(EXTENSION): $(GRF2FST2_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(IMPLODETFST)$(EXTENSION): $(IMPLODETFST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(LOCATE)$(EXTENSION): $(LOCATE_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(LOCATETFST)$(EXTENSION): $(LOCATETFST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(MULTIFLEX)$(EXTENSION): $(MULTIFLEX_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(NORMALIZE)$(EXTENSION): $(NORMALIZE_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(POLYLEX)$(EXTENSION): $(POLYLEX_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(REBUILDTFST)$(EXTENSION): $(REBUILDTFST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(RECONSTRUCAO)$(EXTENSION): $(RECONSTRUCAO_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(REG2GRF)$(EXTENSION): $(REG2GRF_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(SORTTXT)$(EXTENSION): $(SORTTXT_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(STATS)$(EXTENSION): $(STATS_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TABLE2GRF)$(EXTENSION): $(TABLE2GRF_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TAGSETNORMTFST)$(EXTENSION): $(TAGSETNORMTFST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TEI2TXT)$(EXTENSION): $(TEI2TXT_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TFST2GRF)$(EXTENSION): $(TFST2GRF_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TFST2UNAMBIG)$(EXTENSION): $(TFST2UNAMBIG_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TOKENIZE)$(EXTENSION): $(TOKENIZE_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TXT2TFST)$(EXTENSION): $(TXT2TFST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(XMLIZER)$(EXTENSION): $(XMLIZER_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(UNCOMPRESS)$(EXTENSION): $(UNCOMPRESS_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(UNITEXTOOL)$(EXTENSION): $(UNITEXTOOL_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(UNITEXTOOL_LOGGER)$(EXTENSION): $(UNITEXTOOL_LOGGER_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(INFLECTKR)$(EXTENSION): $(INFLECTKR_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(SUFFORM2RAC)$(EXTENSION): $(SUFFORM2RAC_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(COMPRESSKR)$(EXTENSION): $(COMPRESSKR_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(MERGEBIN)$(EXTENSION): $(MERGEBIN_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(CONSULTDIC)$(EXTENSION): $(CONSULTDIC_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(SORTMORPH)$(EXTENSION): $(SORTMORPH_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TXT2FST2KR)$(EXTENSION): $(TXT2FST2KR_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(SYL2JAMO)$(EXTENSION): $(SYL2JAMO_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(JAMO2SYL)$(EXTENSION): $(JAMO2SYL_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(EXTRACTCHAR)$(EXTENSION): $(EXTRACTCHAR_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

#
# Compilation instructions for Logger
#

FileUnPack.o: ../logger/FileUnPack.cpp
	$(CC) -c ../logger/FileUnPack.cpp $(OPTIONS)

ReworkArg.o: ../logger/ReworkArg.cpp
	$(CC) -c ../logger/ReworkArg.cpp $(OPTIONS)

UniRunLogger.o: ../logger/UniRunLogger.cpp
	$(CC) -c ../logger/UniRunLogger.cpp $(OPTIONS)

FilePack.o: ../logger/FilePack.cpp
	$(CC) -c ../logger/FilePack.cpp $(OPTIONS)

FilePackCrc32.o: ../logger/FilePackCrc32.cpp
	$(CC) -c ../logger/FilePackCrc32.cpp $(OPTIONS)

FilePackIo.o: ../logger/FilePackIo.cpp
	$(CC) -c ../logger/FilePackIo.cpp $(OPTIONS)

UniLogger.o: ../logger/UniLogger.cpp
	$(CC) -c ../logger/UniLogger.cpp $(OPTIONS)
	
SyncLoggerWin.o: ../logger/SyncLoggerWin.cpp
	$(CC) -c ../logger/SyncLoggerWin.cpp $(OPTIONS)

SyncLoggerPosix.o: ../logger/SyncLoggerPosix.cpp
	$(CC) -c ../logger/SyncLoggerPosix.cpp $(OPTIONS)

SyncLoggerDummy.o: ../logger/SyncLoggerDummy.cpp
	$(CC) -c ../logger/SyncLoggerDummy.cpp $(OPTIONS)

DirLoggerWin.o: ../logger/DirLoggerWin.cpp
	$(CC) -c ../logger/DirLoggerWin.cpp $(OPTIONS)

DirLoggerPosix.o: ../logger/DirLoggerPosix.cpp
	$(CC) -c ../logger/DirLoggerPosix.cpp $(OPTIONS)

UniLoggerAutoInstall.o: ../logger/UniLoggerAutoInstall.cpp
	$(CC) -c ../logger/UniLoggerAutoInstall.cpp $(OPTIONS)


#
# Compilation instructions for Korean .o
#
InflectKr.o: ../korean/InflectKr.cpp
	$(CC) -c ../korean/InflectKr.cpp $(OPTIONS)

CompressKr.o: ../korean/CompressKr.cpp
	$(CC) -c ../korean/CompressKr.cpp $(OPTIONS)

etc.o: ../korean/etc.cpp
	$(CC) -c ../korean/etc.cpp $(OPTIONS)

pro_type.o: ../korean/pro_type.cpp
	$(CC) -c ../korean/pro_type.cpp $(OPTIONS)

codeForKorean.o: ../korean/codeForKorean.cpp
	$(CC) -c ../korean/codeForKorean.cpp $(OPTIONS)

orgUniMbcsMap.o: ../korean/orgUniMbcsMap.cpp
	$(CC) -c ../korean/orgUniMbcsMap.cpp $(OPTIONS)

bitmap.o: ../korean/bitmap.cpp
	$(CC) -c ../korean/bitmap.cpp $(OPTIONS)

ConsultDic.o: ../korean/ConsultDic.cpp ../korean/bin.h ../korean/bin3.h
	$(CC) -c ../korean/ConsultDic.cpp $(OPTIONS)

ExtractChar.o: ../korean/ExtractChar.cpp
	$(CC) -c ../korean/ExtractChar.cpp $(OPTIONS)

Jamo2Syl.o: ../korean/Jamo2Syl.cpp
	$(CC) -c ../korean/Jamo2Syl.cpp $(OPTIONS)

jamoCodage.o: ../korean/jamoCodage.cpp
	$(CC) -c ../korean/jamoCodage.cpp $(OPTIONS)

MergeBin.o: ../korean/MergeBin.cpp ../korean/bin.h ../korean/bin3.h
	$(CC) -c ../korean/MergeBin.cpp $(OPTIONS)

segtype.o: ../korean/segtype.cpp
	$(CC) -c ../korean/segtype.cpp $(OPTIONS)

SortMorph.o: ../korean/SortMorph.cpp
	$(CC) -c ../korean/SortMorph.cpp $(OPTIONS)

SufForm2Rac.o: ../korean/SufForm2Rac.cpp
	$(CC) -c ../korean/SufForm2Rac.cpp $(OPTIONS)

Syl2Jamo.o: ../korean/Syl2Jamo.cpp
	$(CC) -c ../korean/Syl2Jamo.cpp $(OPTIONS)

Txt2Fst2Kr.o: ../korean/Txt2Fst2Kr.cpp ../korean/strToAuto.h
	$(CC) -c ../korean/Txt2Fst2Kr.cpp $(OPTIONS)


# Compiling Unitex as a library under Linux-like systems
ifeq ($(SYSTEM),linux-like)


no_o_files:
	rm -f *.o

.libunitex: no_o_files $(OBJS) Test_lib.o
	rm -f Main_*.o
	$(CC) *.o $(OPTIONS) -shared -o ../bin/libunitex.so
	$(CC) Test_lib.o -L../bin -Wl,-rpath,. -lunitex -o ../bin/Test_lib


else

# Compiling Unitex as a library under Windows
no_o_files:
	rm -f *.o

.libunitex: no_o_files $(LIBTRE) $(OBJS) Test_lib.o
	rm -f Main_*.o
	$(CC) *.o -shared -Wl,--export-all-symbols -L"." -ltre -o ../bin/unitex.dll
	$(CC) Test_lib.o $(OPTIONS) libtre.a ../bin/unitex.dll -o ../bin/Test_lib.exe

endif

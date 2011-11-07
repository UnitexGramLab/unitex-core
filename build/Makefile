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
# request freeing all memory possible at exit
#
# The option
#    make UNITEXTOOLONLY=yes
# build only the UnitexTool and UnitexToolLogger executable target
#    make UNITEXTOOLLOGGERONLY=yes
# build only the UnitexToolLogger executable target
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
#
#  UNITEX AS A JNI:
#  
#  To produce a JNI, you need to have a JDK installed and must provide
#  additional include paths. Those paths correspond to the locations
#  of jni.h and jni_md.h
#
#    make JNILIBRARY=yes ADDITIONAL_INCLUDE=/usr/lib/jvm/java-6-openjdk/include/linux ADDITIONAL_INCLUDE2=/usr/lib/jvm/java-6-openjdk/include
# create the Jni and Library lib libUnitexLibAndJni.so under Linux
#
#    mingw32-make.exe SYSTEM=windows JNILIBRARY=yes ADDITIONAL_INCLUDE=c:\Progra~1\Java\jdk1.6.0_26\include" ADDITIONAL_INCLUDE2=c:\Progra~1\Java\jdk1.6.0_26\include\win32"
# create the Jni and Library lib libUnitexLibAndJni.so under Windows with MinGW32 (fix the path, of course)
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

SYSLIBLOGGER = SyncLoggerPosix.o
SYSLIBMAPPED = MappedFileHelperPosix.o DirHelperPosix.o
SYSLIBDIRIO = UnitexLibDirPosix.o

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
UNITEX_RELEASE_MEMORY_AT_EXIT=yes
else
CFLAGS += -O3
endif

ifneq ($(strip $(ADDITIONAL_INCLUDE)),)
DEFFLAGS += -I$(ADDITIONAL_INCLUDE)
endif

ifneq ($(strip $(ADDITIONAL_INCLUDE2)),)
DEFFLAGS += -I$(ADDITIONAL_INCLUDE2)
endif

ifneq ($(strip $(ADDITIONAL_CFLAG)),)
CFLAGS += $(ADDITIONAL_CFLAG)
endif

ifneq ($(strip $(ADDITIONAL_CFLAG2)),)
CFLAGS += $(ADDITIONAL_CFLAG2)
endif

ifeq ($(LIBRARY),yes)
CFLAGS += -fPIC
DEFFLAGS += -DUNITEX_LIBRARY
endif

ifeq ($(JNILIBRARY),yes)
CFLAGS += -fPIC
DEFFLAGS += -DUNITEX_LIBRARY
endif

ifeq ($(UNITEX_RELEASE_MEMORY_AT_EXIT),yes)
DEFFLAGS += -DUNITEX_RELEASE_MEMORY_AT_EXIT
endif


override CFLAGS   += $(DEFFLAGS) -I"../include_tre" -Ilibtre/include -I".."
LIBTRE   = .libtre-done
CLEAN-LIBTRE   = clean-libtre
LIBS   = -Llibtre/lib -ltre -lpthread


OPTIONS = $(CFLAGS) $(LDFLAGS) $(LIBS)
EXTENSION =

ifneq ($(strip $(ADDITIONAL_OPTIONS)),)
OPTIONS += $(ADDITIONAL_OPTIONS)
endif


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

ifneq ($(strip $(ADDITIONAL_TRE_OPTIONS)),)
TRE_OPTIONS += $(ADDITIONAL_TRE_OPTIONS)
endif

#
# Compiling instructions for the TRE library
#
INSTALL_COMMAND = make install

LIBS =  -L "." -ltre
INCS =  -I".." -I"../include_tre"
CXXINCS =  -I".."
CXXFLAGS = $(CXXINCS)  -Wall
CFLAGS = $(INCS)  -Wall

ifeq ($(DEBUG), yes)
CFLAGS += -g -O0
else
CFLAGS += -O3
endif

ifneq ($(strip $(ADDITIONAL_INCLUDE)),)
CFLAGS += -I$(ADDITIONAL_INCLUDE)
endif

ifneq ($(strip $(ADDITIONAL_INCLUDE2)),)
CFLAGS += -I$(ADDITIONAL_INCLUDE2)
endif

ifneq ($(strip $(ADDITIONAL_CFLAG)),)
CFLAGS += $(ADDITIONAL_CFLAG)
endif

ifneq ($(strip $(ADDITIONAL_CFLAG2)),)
CFLAGS += $(ADDITIONAL_CFLAG2)
endif

LIBTRE   = libtre.a

# -fPIC is not required because all code is position independent under Windows
#ifeq ($(LIBRARY),yes)
#CFLAGS += -fPIC
#endif
#ifeq ($(JNILIBRARY),yes)
#CFLAGS += -fPIC
#endif

ifeq ($(LIBRARY),yes)
CFLAGS += -DUNITEX_LIBRARY
endif

ifeq ($(JNILIBRARY),yes)
CFLAGS += -DUNITEX_LIBRARY
endif

SYSLIBLOGGER = SyncLoggerWin.o
SYSLIBMAPPED = MappedFileHelperWin.o DirHelperWin.o
SYSLIBDIRIO = UnitexLibDirWin.o

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

SYSLIBLOGGER = SyncLoggerWin.o
SYSLIBMAPPED = MappedFileHelperWin.o DirHelperWin.o
SYSLIBDIRIO = UnitexLibDirWin.o

APPDIR = ../../../App

ifneq ($(DEVCPP_PATHDEFINED),yes)
DEVCPP_PATH = C:/Dev-Cpp
endif



TRE_OPTIONS = -DHAVE_CONFIG_H=1 -I../include_tre -O3 -Wall -c -fmessage-length=0

ifneq ($(strip $(ADDITIONAL_TRE_OPTIONS)),)
TRE_OPTIONS += $(ADDITIONAL_TRE_OPTIONS)
endif


#
# USING TRE STATIC LIBRARY
#
LIBS =  -L"$(DEVCPP_PATH)/lib" -L"." -ltre
INCS =  -I"$(DEVCPP_PATH)/include" -I"../include_tre" -I".."
CXXINCS =  -I"$(DEVCPP_PATH)/include/c++"  -I"$(DEVCPP_PATH)/include/c++/mingw32"  -I"$(DEVCPP_PATH)/include/c++/backward"  -I"$(DEVCPP_PATH)/include"  -I"../include_tre" -I".."
CXXFLAGS = $(CXXINCS)  -Wall
CFLAGS = $(INCS)  -Wall

ifeq ($(DEBUG), yes)
CFLAGS += -g -O0
else
CFLAGS += -O3
endif

ifneq ($(strip $(ADDITIONAL_INCLUDE)),)
CFLAGS += -I$(ADDITIONAL_INCLUDE)
endif

ifneq ($(strip $(ADDITIONAL_INCLUDE2)),)
CFLAGS += -I$(ADDITIONAL_INCLUDE2)
endif

ifneq ($(strip $(ADDITIONAL_CFLAG)),)
CFLAGS += $(ADDITIONAL_CFLAG)
endif

ifneq ($(strip $(ADDITIONAL_CFLAG2)),)
CFLAGS += $(ADDITIONAL_CFLAG2)
endif

LIBTRE   = libtre.a

# -fPIC is not required because all code is position independent under Windows
#ifeq ($(LIBRARY),yes)
#CFLAGS += -fPIC
#endif
#ifeq ($(JNILIBRARY),yes)
#CFLAGS += -fPIC
#endif

ifeq ($(LIBRARY),yes)
CFLAGS += -DUNITEX_LIBRARY
endif

ifeq ($(JNILIBRARY),yes)
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

BUILDKRMWUDIC      = BuildKrMwuDic
BUILDKRMWUDIC_OBJS = Main_BuildKrMwuDic.o BuildKrMwuDic.o IOBuffer.o Af_stdio.o ActivityLogger.o\
					 Unicode.o AbstractAllocator.o Error.o UnitexGetOpt.o KrMwuDic.o StringParsing.o DELA.o String_hash.o\
					 Alphabet.o List_ustring.o Korean.o AbstractFst2Load.o Fst2.o\
                     Pattern.o List_int.o Transitions.o Symbol.o LanguageDefinition.o Tagset.o\
                     Ustring.o MF_InflectTransd.o Grf2Fst2.o Grf2Fst2_lib.o SingleGraph.o File.o\
                     BitArray.o HashTable.o FIFO.o BitMasks.o Fst2Check_lib.o ProgramInvoker.o\
                     MF_SU_morpho.o MF_LangMorpho.o MF_Util.o MF_FormMorpho.o\
                     MF_Operators_Util.o MF_DicoMorpho.o Arabic.o AbstractDelaLoad.o \
                     CompressedDic.o LoadInf.o DebugMode.o Grf_lib.o Persistence.o $(SYSLIBMAPPED)\
                     $(SYSLIBLOGGER)

CASSYS		= Cassys
CASSYS_OBJS	= Main_Cassys.o Cassys.o IOBuffer.o Error.o UnitexGetOpt.o Unicode.o Af_stdio.o ActivityLogger.o AbstractAllocator.o ProgramInvoker.o\
			FIFO.o Concord.o Locate.o Concordance.o Snt.o Text_tokens.o File.o Buffer.o LocateMatches.o LocatePattern.o LocateTrace.o Thai.o NewLineShifts.o\
			String_hash.o SortTxt.o StringParsing.o DELA.o List_int.o Alphabet.o BitMasks.o Tokenization.o LemmaTree.o PatternTree.o\
			BitArray.o List_pointer.o CompoundWordTree.o AbstractDelaLoad.o Korean.o UserCancelling.o Stack_unichar.o AbstractFst2Load.o\
			Fst2.o MorphologicalFilters.o LocateFst2Tags.o TransductionVariables.o OptimizedFst2.o Text_parsing.o List_ustring.o ParsingInfo.o\
			DicVariables.o TransductionStack.o MorphologicalLocate.o Pattern.o Contexts.o Transitions.o Symbol.o Ustring.o LanguageDefinition.o\
			Tagset.o Tokenize.o HashTable.o Cassys_tokens.o OutputTransductionVariables.o LocateCache.o \
			Arabic.o Match.o VariableUtils.o Offsets.o Overlap.o CompressedDic.o LoadInf.o \
			DebugMode.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)


CHECKDIC      = CheckDic
CHECKDIC_OBJS = Main_CheckDic.o CheckDic.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o File.o DELA.o Alphabet.o String_hash.o\
				List_ustring.o Error.o StringParsing.o UnitexGetOpt.o AbstractDelaLoad.o CompressedDic.o \
				LoadInf.o Ustring.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

COMPRESS      = Compress
COMPRESS_OBJS = Main_Compress.o Compress.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o DELA.o DictionaryTree.o Alphabet.o\
				String_hash.o AutomatonDictionary2Bin.o File.o List_int.o\
				List_ustring.o Error.o StringParsing.o UnitexGetOpt.o ProgramInvoker.o \
				BitArray.o AbstractDelaLoad.o CompressedDic.o LoadInf.o Ustring.o \
				Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

CONCORD      = Concord
CONCORD_OBJS = Main_Concord.o Concord.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o Text_tokens.o String_hash.o\
				List_int.o Alphabet.o LocateMatches.o Match.o Concordance.o File.o\
				Text_parsing.o UserCancelling.o MorphologicalLocate.o LocatePattern.o LocateTrace.o \
				TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o\
				CompoundWordTree.o MorphologicalFilters.o DELA.o List_ustring.o Fst2.o \
				AbstractDelaLoad.o AbstractFst2Load.o Pattern.o LocateFst2Tags.o Error.o SortTxt.o\
				Buffer.o StringParsing.o Snt.o BitArray.o LemmaTree.o\
				List_pointer.o PatternTree.o BitMasks.o Tokenization.o Thai.o Contexts.o\
				Stack_unichar.o Transitions.o DicVariables.o\
				Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o\
				ProgramInvoker.o NewLineShifts.o Korean.o HashTable.o LocateCache.o\
				Arabic.o OutputTransductionVariables.o VariableUtils.o CompressedDic.o \
				LoadInf.o Offsets.o Overlap.o DebugMode.o Persistence.o $(SYSLIBMAPPED)\
				$(SYSLIBLOGGER)

CONCORDIFF      = ConcorDiff
CONCORDIFF_OBJS = Main_ConcorDiff.o ConcorDiff.o IOBuffer.o Diff.o Concord.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o Text_tokens.o String_hash.o List_int.o\
				Alphabet.o LocateMatches.o Match.o Concordance.o File.o\
				Text_parsing.o UserCancelling.o MorphologicalLocate.o LocatePattern.o LocateTrace.o TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o\
				CompoundWordTree.o MorphologicalFilters.o\
				DELA.o List_ustring.o Fst2.o AbstractDelaLoad.o AbstractFst2Load.o Pattern.o LocateFst2Tags.o\
				Error.o SortTxt.o\
				Buffer.o StringParsing.o Snt.o BitArray.o LemmaTree.o List_pointer.o BitMasks.o\
				PatternTree.o Tokenization.o Thai.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
				Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o\
				ProgramInvoker.o NewLineShifts.o Korean.o HashTable.o LocateCache.o\
				Arabic.o OutputTransductionVariables.o VariableUtils.o CompressedDic.o \
				LoadInf.o Offsets.o Overlap.o DebugMode.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

CONVERT      = Convert
CONVERT_OBJS = Main_Convert.o Convert.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o File.o CodePages.o Error.o\
				HTMLCharacters.o AsciiSearchTree.o UnitexGetOpt.o Arabic.o String_hash.o StringParsing.o\
				Ustring.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

DICO      = Dico
DICO_OBJS = Main_Dico.o Dico.o List_int.o DELA.o ApplyDic.o File.o Alphabet.o String_hash.o Text_tokens.o \
            List_ustring.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o CompoundWordHashTable.o Fst2.o AbstractDelaLoad.o AbstractFst2Load.o \
            IOBuffer.o Error.o CompoundWordTree.o\
            Pattern.o ParsingInfo.o LocatePattern.o LocateTrace.o LocateMatches.o Match.o OptimizedFst2.o \
            Text_parsing.o UserCancelling.o MorphologicalLocate.o TransductionVariables.o TransductionStack.o MorphologicalFilters.o Locate.o\
            StringParsing.o Buffer.o BitArray.o Snt.o LemmaTree.o List_pointer.o PatternTree.o\
            LocateFst2Tags.o BitMasks.o Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
            Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o ProgramInvoker.o\
            Korean.o SortTxt.o Thai.o Compress.o DictionaryTree.o\
            AutomatonDictionary2Bin.o HashTable.o LocateCache.o Arabic.o \
            OutputTransductionVariables.o VariableUtils.o CompressedDic.o \
            DebugMode.o LoadInf.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

ELAG      = Elag
ELAG_OBJS = Main_Elag.o Elag.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o ElagFunctions.o ElagFstFilesIO.o AutDeterminization.o \
            AutMinimization.o AutConcat.o Fst2Automaton.o \
            Symbol.o String_hash.o Tagset.o Ustring.o \
   		    ElagStateSet.o SymbolAlphabet.o Symbol_op.o Error.o LanguageDefinition.o\
            AutIntersection.o Alphabet.o StringParsing.o DELA.o List_ustring.o\
            List_int.o SingleGraph.o HashTable.o BitArray.o Fst2.o Pattern.o\
            BitMasks.o FIFO.o Transitions.o ElagDebug.o File.o UnitexGetOpt.o Tfst.o \
            TfstStats.o AbstractDelaLoad.o CompressedDic.o LoadInf.o Persistence.o $(SYSLIBMAPPED)\
            $(SYSLIBLOGGER)

ELAGCOMP      = ElagComp
ELAGCOMP_OBJS = Main_ElagComp.o ElagComp.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o ElagRulesCompilation.o Fst2Automaton.o\
                ElagFstFilesIO.o String_hash.o\
                Symbol.o Ustring.o AutDeterminization.o AutIntersection.o\
                ElagStateSet.o Symbol_op.o LanguageDefinition.o Tagset.o \
	            AutMinimization.o SymbolAlphabet.o AutConcat.o AutComplementation.o\
	            Alphabet.o Error.o StringParsing.o DELA.o List_ustring.o List_int.o\
	            SingleGraph.o HashTable.o BitArray.o Fst2.o Pattern.o BitMasks.o FIFO.o File.o\
	            Transitions.o ElagDebug.o UnitexGetOpt.o Tfst.o TfstStats.o AbstractDelaLoad.o \
	            CompressedDic.o LoadInf.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

EVAMB      = Evamb
EVAMB_OBJS = Main_Evamb.o Evamb.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o \
             AbstractAllocator.o String_hash.o\
             Alphabet.o Error.o StringParsing.o Pattern.o List_ustring.o\
             List_int.o BitMasks.o DELA.o List_pointer.o SingleGraph.o\
             BitArray.o HashTable.o FIFO.o Transitions.o\
             Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o \
             Tfst.o TfstStats.o File.o AbstractDelaLoad.o CompressedDic.o \
             LoadInf.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

EXTRACT      = Extract
EXTRACT_OBJS = Main_Extract.o Extract.o IOBuffer.o LocatePattern.o LocateTrace.o \
               Text_parsing.o UserCancelling.o MorphologicalLocate.o TransductionStack.o \
               OptimizedFst2.o TransductionVariables.o \
               ParsingInfo.o CompoundWordTree.o  List_int.o Concordance.o \
               DELA.o List_ustring.o Fst2.o Pattern.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o AbstractDelaLoad.o AbstractFst2Load.o\
               Text_tokens.o File.o ExtractUnits.o Alphabet.o String_hash.o LocateMatches.o Match.o\
	           MorphologicalFilters.o Error.o Buffer.o SortTxt.o StringParsing.o BitArray.o\
	           LemmaTree.o Snt.o List_pointer.o PatternTree.o LocateFst2Tags.o BitMasks.o\
	           Tokenization.o Thai.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
	           Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o\
	           ProgramInvoker.o NewLineShifts.o Korean.o HashTable.o LocateCache.o \
	           Arabic.o OutputTransductionVariables.o VariableUtils.o CompressedDic.o \
	           DebugMode.o LoadInf.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

FLATTEN      = Flatten
FLATTEN_OBJS = Main_Flatten.o Flatten.o IOBuffer.o Alphabet.o Af_stdio.o ActivityLogger.o \
               Unicode.o AbstractAllocator.o Fst2.o Grf2Fst2_lib.o\
               AbstractFst2Load.o FlattenFst2.o List_int.o Error.o File.o SingleGraph.o\
               BitMasks.o BitArray.o HashTable.o FIFO.o Pattern.o List_ustring.o\
               String_hash.o StringParsing.o Transitions.o DELA.o\
               Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o \
               UnitexGetOpt.o AbstractDelaLoad.o CompressedDic.o LoadInf.o \
               DebugMode.o Grf_lib.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

FST2CHECK      = Fst2Check
FST2CHECK_OBJS = Main_Fst2Check.o Fst2Check.o Fst2Check_lib.o IOBuffer.o Af_stdio.o \
                ActivityLogger.o Unicode.o AbstractAllocator.o Fst2.o AbstractFst2Load.o Alphabet.o\
                File.o String_hash.o Error.o TransductionVariables.o ProgramInvoker.o UnitexGetOpt.o\
                StringParsing.o Pattern.o List_ustring.o List_int.o BitMasks.o\
                Transitions.o DELA.o Symbol.o Symbol_op.o LanguageDefinition.o \
                Ustring.o Tagset.o AbstractDelaLoad.o CompressedDic.o LoadInf.o \
                Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

FST2LIST      = Fst2List
FST2LIST_OBJS = Main_Fst2List.o Fst2List.o IOBuffer.o Af_stdio.o ActivityLogger.o \
                Unicode.o AbstractAllocator.o Fst2.o AbstractFst2Load.o Alphabet.o\
                File.o String_hash.o Error.o TransductionVariables.o\
                StringParsing.o Pattern.o List_ustring.o List_int.o BitMasks.o\
                Transitions.o DELA.o Symbol.o Symbol_op.o LanguageDefinition.o \
                Ustring.o Tagset.o AbstractDelaLoad.o CompressedDic.o LoadInf.o \
                Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

FST2TXT      = Fst2Txt
FST2TXT_OBJS = Main_Fst2Txt.o Fst2Txt.o IOBuffer.o Af_stdio.o ActivityLogger.o \
               Unicode.o AbstractAllocator.o Fst2.o\
               AbstractFst2Load.o Alphabet.o ParsingInfo.o\
               TransductionVariables.o String_hash.o Error.o StringParsing.o\
               Pattern.o List_ustring.o List_int.o BitMasks.o BitArray.o\
               Fst2Txt_TokenTree.o Buffer.o Transitions.o File.o \
	           Fst2TxtAsRoutine.o DELA.o DicVariables.o TransductionStack.o\
	           Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o Stack_unichar.o\
	           Korean.o HashTable.o OutputTransductionVariables.o VariableUtils.o \
	           Offsets.o Overlap.o AbstractDelaLoad.o CompressedDic.o LoadInf.o \
	           Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

GRF2FST2      = Grf2Fst2
GRF2FST2_OBJS = Main_Grf2Fst2.o Grf2Fst2.o IOBuffer.o Alphabet.o Af_stdio.o ActivityLogger.o \
                Unicode.o AbstractAllocator.o Fst2.o AbstractFst2Load.o \
                Fst2Check_lib.o Grf2Fst2_lib.o Error.o File.o SingleGraph.o\
                BitMasks.o BitArray.o List_int.o HashTable.o FIFO.o Pattern.o\
                List_ustring.o String_hash.o StringParsing.o Transitions.o DELA.o\
                Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o\
                ProgramInvoker.o AbstractDelaLoad.o CompressedDic.o LoadInf.o \
                DebugMode.o Grf_lib.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

GRFDIFF      = GrfDiff
GRFDIFF_OBJS = Main_GrfDiff.o GrfDiff.o Unicode.o Error.o IOBuffer.o Af_stdio.o ActivityLogger.o\
			GrfSvn_lib.o Grf_lib.o Ustring.o UnitexGetOpt.o String_hash.o \
			StringParsing.o AbstractAllocator.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

GRFDIFF3      = GrfDiff3
GRFDIFF3_OBJS = Main_GrfDiff3.o GrfDiff3.o Unicode.o Error.o IOBuffer.o Af_stdio.o ActivityLogger.o\
			GrfSvn_lib.o Grf_lib.o Ustring.o UnitexGetOpt.o String_hash.o \
			StringParsing.o AbstractAllocator.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

GRFTEST      = GrfTest
GRFTEST_OBJS = Main_GrfTest.o GrfTest.o Unicode.o Error.o IOBuffer.o Af_stdio.o ActivityLogger.o\
			GrfSvn_lib.o Grf_lib.o Ustring.o UnitexGetOpt.o Normalize.o Offsets.o File.o \
			Overlap.o NormalizeAsRoutine.o String_hash.o StringParsing.o AbstractAllocator.o\
			DELA.o Alphabet.o CompressedDic.o LoadInf.o List_ustring.o \
			AbstractDelaLoad.o ProgramInvoker.o Fst2Txt.o Fst2TxtAsRoutine.o \
			Fst2Txt_TokenTree.o Transitions.o Symbol.o LanguageDefinition.o \
			Tagset.o List_int.o TransductionStack.o Stack_unichar.o VariableUtils.o \
			TransductionVariables.o OutputTransductionVariables.o \
			DicVariables.o Korean.o HashTable.o ParsingInfo.o SingleGraph.o \
			BitArray.o FIFO.o BitMasks.o Buffer.o Fst2Automaton.o ElagFstFilesIO.o\
			Tfst.o TfstStats.o Fst2.o Pattern.o AbstractFst2Load.o \
			Tokenize.o Dico.o Snt.o Text_tokens.o ApplyDic.o Match.o \
			CompoundWordHashTable.o Tokenization.o LocateMatches.o \
			SortTxt.o Thai.o Compress.o DictionaryTree.o \
			AutomatonDictionary2Bin.o LocatePattern.o Locate.o \
			Text_parsing.o DebugMode.o LocateCache.o UserCancelling.o \
			MorphologicalFilters.o Contexts.o MorphologicalLocate.o Arabic.o \
			List_pointer.o LocateTrace.o PatternTree.o LocateFst2Tags.o \
			CompoundWordTree.o LemmaTree.o OptimizedFst2.o GrfTest_lib.o Grf2Fst2.o\
			Grf2Fst2_lib.o Fst2Check_lib.o Concord.o Concordance.o \
			NewLineShifts.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

IMPLODETFST      = ImplodeTfst
IMPLODETFST_OBJS = Main_ImplodeTfst.o ImplodeTfst.o IOBuffer.o Af_stdio.o ActivityLogger.o \
                   Unicode.o AbstractAllocator.o Tfst.o File.o SingleGraph.o\
                   String_hash.o Alphabet.o Error.o StringParsing.o Pattern.o List_ustring.o\
                   List_int.o BitMasks.o DELA.o List_pointer.o Transitions.o\
                   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o\
                   BitArray.o HashTable.o FIFO.o Match.o TfstStats.o AbstractDelaLoad.o \
                   CompressedDic.o LoadInf.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

LOCATE      = Locate
LOCATE_OBJS = Main_Locate.o Locate.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o \
              AbstractAllocator.o Alphabet.o DELA.o List_ustring.o String_hash.o \
              LocatePattern.o LocateTrace.o AbstractDelaLoad.o AbstractFst2Load.o \
              Fst2.o Text_tokens.o List_int.o\
              Pattern.o CompoundWordTree.o LocateFst2Tags.o \
              OptimizedFst2.o Text_parsing.o UserCancelling.o MorphologicalLocate.o LocateMatches.o \
              Match.o TransductionVariables.o TransductionStack.o\
              ParsingInfo.o File.o MorphologicalFilters.o Error.o StringParsing.o\
              BitArray.o LemmaTree.o List_pointer.o PatternTree.o BitMasks.o Buffer.o\
              Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
              Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o \
              ProgramInvoker.o Korean.o HashTable.o LocateCache.o Arabic.o \
              OutputTransductionVariables.o VariableUtils.o CompressedDic.o \
              LoadInf.o DebugMode.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

LOCATETFST      = LocateTfst
LOCATETFST_OBJS = Main_LocateTfst.o LocateTfst.o IOBuffer.o Af_stdio.o ActivityLogger.o \
                  Unicode.o AbstractAllocator.o Error.o UnitexGetOpt.o LocateTfst_lib.o \
                  Tfst.o File.o Ustring.o SingleGraph.o List_int.o BitMasks.o FIFO.o Transitions.o \
                  Symbol.o LanguageDefinition.o Tagset.o String_hash.o StringParsing.o DELA.o \
                  Alphabet.o List_ustring.o HashTable.o BitArray.o Fst2.o AbstractFst2Load.o Pattern.o \
                  LocateTfstMatches.o Match.o MorphologicalFilters.o Korean.o \
                  OptimizedTfstTagMatching.o List_pointer.o\
                  TransductionVariables.o TransductionStackTfst.o Contexts.o \
                  OutputTransductionVariables.o DicVariables.o TfstStats.o AbstractDelaLoad.o \
                  CompressedDic.o LoadInf.o DebugMode.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)


MULTIFLEX      = MultiFlex
MULTIFLEX_OBJS = Main_MultiFlex.o MultiFlex.o Alphabet.o MF_DLC_inflect.o MF_DicoMorpho.o \
                 MF_FormMorpho.o MF_LangMorpho.o\
                 MF_InflectTransd.o MF_MU_graph.o MF_MU_morpho.o MF_SU_morpho.o MF_Unif.o MF_Util.o\
                 Fst2.o AbstractFst2Load.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o Error.o IOBuffer.o File.o DELA.o StringParsing.o List_ustring.o\
                 String_hash.o Pattern.o List_int.o BitMasks.o Transitions.o Grf2Fst2.o\
                 Grf2Fst2_lib.o SingleGraph.o BitArray.o FIFO.o HashTable.o Fst2Check_lib.o\
                 Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o\
                 MF_Operators_Util.o UnitexGetOpt.o ProgramInvoker.o Korean.o \
                 Arabic.o AbstractDelaLoad.o CompressedDic.o LoadInf.o \
                 DebugMode.o Grf_lib.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

MZREPAIRULP   = MzRepairUlp
MZREPAIRULP_OBJS = Main_MzRepairUlp.o MzToolsUlp.o MzRepairUlp.o UnitexGetOpt.o \
                   Error.o Unicode.o Af_stdio.o ActivityLogger.o IOBuffer.o $(SYSLIBMAPPED)

NORMALIZE      = Normalize
NORMALIZE_OBJS = Main_Normalize.o Normalize.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o \
				AbstractAllocator.o File.o DELA.o List_ustring.o String_hash.o\
				Alphabet.o Error.o StringParsing.o Buffer.o NormalizeAsRoutine.o \
				UnitexGetOpt.o AbstractDelaLoad.o CompressedDic.o LoadInf.o Ustring.o \
				Offsets.o Overlap.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

POLYLEX      = PolyLex
POLYLEX_OBJS = Main_PolyLex.o PolyLex.o IOBuffer.o Af_stdio.o ActivityLogger.o \
                Unicode.o AbstractAllocator.o Alphabet.o DELA.o AbstractDelaLoad.o \
                List_ustring.o String_hash.o\
				NorwegianCompounds.o GermanCompounds.o File.o\
				RussianCompounds.o GeneralDerivation.o Error.o StringParsing.o\
				DutchCompounds.o UnitexGetOpt.o CompressedDic.o LoadInf.o Ustring.o \
				Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

REBUILDTFST      = RebuildTfst
REBUILDTFST_OBJS = Main_RebuildTfst.o RebuildTfst.o IOBuffer.o Af_stdio.o ActivityLogger.o \
                   Unicode.o AbstractAllocator.o String_hash.o AbstractFst2Load.o Fst2.o\
                   File.o Alphabet.o Error.o StringParsing.o\
                   Pattern.o List_ustring.o List_int.o BitMasks.o BitArray.o\
                   Transitions.o DELA.o\
                   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o\
                   UnitexGetOpt.o Tfst.o TfstStats.o Grf2Fst2.o Grf2Fst2_lib.o SingleGraph.o\
                   HashTable.o FIFO.o Fst2Check_lib.o ProgramInvoker.o AbstractDelaLoad.o \
                   CompressedDic.o LoadInf.o DebugMode.o Grf_lib.o Persistence.o $(SYSLIBMAPPED)\
                    $(SYSLIBLOGGER)

RECONSTRUCAO      = Reconstrucao
RECONSTRUCAO_OBJS = Main_Reconstrucao.o Reconstrucao.o OptimizedFst2.o ParsingInfo.o CompoundWordTree.o\
               DELA.o AbstractDelaLoad.o List_ustring.o Fst2.o AbstractFst2Load.o \
               IOBuffer.o TransductionStack.o Pattern.o \
               Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o Alphabet.o \
               LocateMatches.o Match.o File.o NormalizationFst2.o\
	           PortugueseNormalization.o TransductionVariables.o \
               Text_parsing.o UserCancelling.o MorphologicalLocate.o String_hash.o \
               LocatePattern.o LocateTrace.o Text_tokens.o List_int.o\
               Sentence2Grf.o MorphologicalFilters.o Error.o StringParsing.o BitArray.o\
               LemmaTree.o List_pointer.o PatternTree.o LocateFst2Tags.o BitMasks.o\
               Buffer.o Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
               Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o \
               Tfst.o TfstStats.o SingleGraph.o FIFO.o HashTable.o Korean.o LocateCache.o \
               Arabic.o OutputTransductionVariables.o VariableUtils.o CompressedDic.o \
               DebugMode.o LoadInf.o Grf_lib.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

REG2GRF      = Reg2Grf
REG2GRF_OBJS = Main_Reg2Grf.o Reg2Grf.o IOBuffer.o Af_stdio.o ActivityLogger.o \
               Unicode.o AbstractAllocator.o RegularExpressions.o Error.o File.o\
               List_int.o Stack_int.o UnitexGetOpt.o Ustring.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

SEQ2GRF      = Seq2Grf
SEQ2GRF_OBJS = Main_Seq2Grf.o Seq2Grf.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o \
               AbstractAllocator.o Error.o\
               StringParsing.o DELA.o List_ustring.o String_hash.o DELA_tree.o \
               TfstStats.o BitArray.o FIFO.o Grf2Fst2_lib.o\
               Tfst.o SingleGraph.o Txt2Tfst.o BitMasks.o HashTable.o \
               BuildTextAutomaton.o Text_tokens.o Offsets.o Sentence2Grf.o\
               AbstractDelaLoad.o UnitexGetOpt.o Ustring.o Grf_lib.o Match.o \
               Transitions.o List_int.o Korean.o NewLineShifts.o\
               Symbol.o LanguageDefinition.o Tagset.o Overlap.o NormalizationFst2.o \
               AbstractFst2Load.o Fst2.o Pattern.o\
               DebugMode.o CompressedDic.o LoadInf.o LocateMatches.o GrfBeauty.o \
               File.o Alphabet.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

SORTTXT      = SortTxt
SORTTXT_OBJS = Main_SortTxt.o SortTxt.o IOBuffer.o Af_stdio.o ActivityLogger.o \
               Unicode.o AbstractAllocator.o Error.o Thai.o UnitexGetOpt.o\
               ProgramInvoker.o DELA.o StringParsing.o Ustring.o String_hash.o Alphabet.o \
               CompressedDic.o AbstractDelaLoad.o LoadInf.o List_ustring.o File.o \
               Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

SPELLCHECK      = SpellCheck
SPELLCHECK_OBJS = Main_SpellCheck.o SpellCheck.o IOBuffer.o Af_stdio.o ActivityLogger.o \
               Unicode.o AbstractAllocator.o Error.o UnitexGetOpt.o\
               ProgramInvoker.o File.o SpellChecking.o CompressedDic.o AbstractDelaLoad.o \
               LoadInf.o List_ustring.o Ustring.o StringParsing.o DELA.o String_hash.o \
               Alphabet.o Keyboard.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

STATS      = Stats
STATS_OBJS = Main_Stats.o Stats.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o \
               AbstractAllocator.o Error.o Thai.o UnitexGetOpt.o\
               ProgramInvoker.o HashTable.o File.o Text_tokens.o String_hash.o \
               LocateMatches.o Match.o DELA.o \
               Alphabet.o StringParsing.o List_int.o List_ustring.o AbstractDelaLoad.o CompressedDic.o \
               LoadInf.o Ustring.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

TABLE2GRF      = Table2Grf
TABLE2GRF_OBJS = Main_Table2Grf.o Table2Grf.o IOBuffer.o Af_stdio.o ActivityLogger.o \
                 Unicode.o AbstractAllocator.o File.o Error.o UnitexGetOpt.o $(SYSLIBMAPPED)

TAGGER      = Tagger
TAGGER_OBJS = Main_Tagger.o Tagger.o IOBuffer.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o\
			  Tfst.o TfstStats.o Ustring.o SingleGraph.o List_int.o BitMasks.o Error.o UnitexGetOpt.o\
			  Transitions.o FIFO.o HashTable.o BitArray.o Symbol.o DELA.o StringParsing.o\
			  String_hash.o Alphabet.o List_ustring.o LanguageDefinition.o Tagset.o\
			  File.o TaggingProcess.o Match.o ElagFunctions.o AutDeterminization.o\
			  AutMinimization.o ElagFstFilesIO.o AutIntersection.o Fst2Automaton.o\
			  SymbolAlphabet.o ElagStateSet.o Symbol_op.o AbstractDelaLoad.o CompressedDic.o \
			  LoadInf.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

TAGSETNORMTFST = TagsetNormTfst
TAGSETNORMTFST_OBJS = Main_TagsetNormTfst.o TagsetNormTfst.o IOBuffer.o Af_stdio.o \
                    ActivityLogger.o Unicode.o AbstractAllocator.o ElagFstFilesIO.o Ustring.o \
					String_hash.o Symbol.o LanguageDefinition.o Tagset.o\
					Error.o File.o StringParsing.o DELA.o List_ustring.o Alphabet.o\
					List_int.o SingleGraph.o HashTable.o BitArray.o Fst2.o AbstractFst2Load.o Pattern.o\
					BitMasks.o FIFO.o Transitions.o\
					Symbol_op.o Fst2Automaton.o UnitexGetOpt.o Tfst.o TfstStats.o \
					Match.o AbstractDelaLoad.o CompressedDic.o LoadInf.o Persistence.o \
					$(SYSLIBMAPPED) $(SYSLIBLOGGER)

TEI2TXT      = TEI2Txt
TEI2TXT_OBJS = Main_TEI2Txt.o TEI2Txt.o IOBuffer.o Af_stdio.o ActivityLogger.o \
               Unicode.o AbstractAllocator.o Error.o File.o HTMLCharacters.o\
               AsciiSearchTree.o UnitexGetOpt.o $(SYSLIBMAPPED)

TFST2GRF      = Tfst2Grf
TFST2GRF_OBJS = Main_Tfst2Grf.o Tfst2Grf.o IOBuffer.o Af_stdio.o ActivityLogger.o \
               Unicode.o AbstractAllocator.o\
               Tfst.o TfstStats.o Sentence2Grf.o File.o List_int.o Error.o StringParsing.o Pattern.o\
               List_ustring.o String_hash.o BitMasks.o BitArray.o DELA.o\
               Alphabet.o Transitions.o Symbol.o Symbol_op.o LanguageDefinition.o\
               Ustring.o Tagset.o UnitexGetOpt.o SingleGraph.o HashTable.o FIFO.o\
               AbstractDelaLoad.o CompressedDic.o LoadInf.o Grf_lib.o Persistence.o \
               $(SYSLIBMAPPED) $(SYSLIBLOGGER)

TFSTTAG      = TfstTag
TFSTTAG_OBJS = Main_TfstTag.o TfstTag.o IOBuffer.o Af_stdio.o ActivityLogger.o \
               Unicode.o AbstractAllocator.o LocateMatches.o Match.o\
               Tfst.o TfstStats.o File.o List_int.o Error.o StringParsing.o Pattern.o\
               List_ustring.o String_hash.o BitMasks.o BitArray.o DELA.o\
               Alphabet.o Transitions.o Symbol.o Symbol_op.o LanguageDefinition.o\
               Ustring.o Tagset.o UnitexGetOpt.o SingleGraph.o HashTable.o FIFO.o\
               AbstractDelaLoad.o CompressedDic.o LoadInf.o Grf_lib.o Persistence.o \
               $(SYSLIBMAPPED) $(SYSLIBLOGGER)

TFST2UNAMBIG      = Tfst2Unambig
TFST2UNAMBIG_OBJS = Main_Tfst2Unambig.o Tfst2Unambig.o Tfst.o TfstStats.o Error.o \
                   IOBuffer.o LinearAutomaton2Txt.o\
                   Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o \
                   Pattern.o List_ustring.o StringParsing.o String_hash.o\
                   List_int.o BitMasks.o Transitions.o DELA.o Alphabet.o\
                   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o\
                   UnitexGetOpt.o SingleGraph.o BitArray.o HashTable.o FIFO.o File.o \
                   AbstractDelaLoad.o CompressedDic.o LoadInf.o Persistence.o \
                   $(SYSLIBMAPPED) $(SYSLIBLOGGER)

TOKENIZE      = Tokenize
TOKENIZE_OBJS = Main_Tokenize.o Tokenize.o IOBuffer.o Af_stdio.o ActivityLogger.o \
                Unicode.o AbstractAllocator.o Alphabet.o String_hash.o\
				File.o DELA.o AbstractDelaLoad.o List_ustring.o HashTable.o Error.o \
				StringParsing.o UnitexGetOpt.o \
				Offsets.o Overlap.o CompressedDic.o LoadInf.o Ustring.o Persistence.o \
				$(SYSLIBMAPPED) $(SYSLIBLOGGER)

TRAININGTAGGER      = TrainingTagger
TRAININGTAGGER_OBJS = Main_TrainingTagger.o TrainingTagger.o TrainingProcess.o IOBuffer.o\
			  Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o\
			  Tfst.o TfstStats.o Ustring.o SingleGraph.o List_int.o BitMasks.o Error.o UnitexGetOpt.o\
			  Transitions.o FIFO.o HashTable.o BitArray.o Symbol.o DELA.o StringParsing.o\
			  String_hash.o Alphabet.o List_ustring.o LanguageDefinition.o Tagset.o\
			  File.o TaggingProcess.o Match.o Compress.o SortTxt.o ProgramInvoker.o Thai.o\
			  DictionaryTree.o AutomatonDictionary2Bin.o AbstractDelaLoad.o CompressedDic.o \
			  LoadInf.o Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

TXT2TFST      = Txt2Tfst
TXT2TFST_OBJS = Main_Txt2Tfst.o Txt2Tfst.o IOBuffer.o Text_tokens.o Alphabet.o Af_stdio.o\
				ActivityLogger.o Unicode.o AbstractAllocator.o DELA_tree.o\
                DELA.o List_ustring.o String_hash.o BuildTextAutomaton.o List_int.o\
                NormalizationFst2.o Fst2.o AbstractFst2Load.o File.o Error.o\
                StringParsing.o Pattern.o BitMasks.o Grf2Fst2_lib.o SingleGraph.o\
                FIFO.o BitArray.o HashTable.o Transitions.o LocateMatches.o Match.o\
                Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o\
                Tfst.o TfstStats.o NewLineShifts.o Korean.o AbstractDelaLoad.o \
                CompressedDic.o LoadInf.o Offsets.o Overlap.o DebugMode.o Grf_lib.o \
                Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)
                

XMLIZER      = XMLizer
XMLIZER_OBJS = Main_XMLizer.o XMLizer.o IOBuffer.o Af_stdio.o ActivityLogger.o \
               Unicode.o AbstractAllocator.o Fst2.o AbstractFst2Load.o Alphabet.o \
               TransductionVariables.o String_hash.o Error.o StringParsing.o File.o\
               Pattern.o List_ustring.o List_int.o BitMasks.o Fst2TxtAsRoutine.o\
               NormalizeAsRoutine.o DELA.o Buffer.o Fst2Txt_TokenTree.o ParsingInfo.o Transitions.o \
	           UnitexGetOpt.o DicVariables.o TransductionStack.o \
	           Symbol.o Symbol_op.o LanguageDefinition.o Tagset.o Ustring.o Stack_unichar.o\
	           Korean.o HashTable.o OutputTransductionVariables.o VariableUtils.o \
	           Overlap.o AbstractDelaLoad.o CompressedDic.o LoadInf.o Offsets.o \
	           Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)


UNCOMPRESS       = Uncompress
UNCOMPRESS_OBJS  = Main_Uncompress.o Uncompress.o File.o Error.o Unicode.o \
                   AbstractAllocator.o DELA.o AbstractDelaLoad.o \
                   String_hash.o StringParsing.o List_ustring.o Alphabet.o Af_stdio.o ActivityLogger.o\
                   UnitexGetOpt.o IOBuffer.o CompressedDic.o LoadInf.o Ustring.o \
                   Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)


UNTOKENIZE       = Untokenize
UNTOKENIZE_OBJS  = Main_Untokenize.o Untokenize.o File.o Error.o Unicode.o AbstractAllocator.o \
                   DELA.o AbstractDelaLoad.o \
                   String_hash.o StringParsing.o List_int.o List_ustring.o Text_tokens.o \
                   Alphabet.o Af_stdio.o ActivityLogger.o\
                   UnitexGetOpt.o IOBuffer.o CompressedDic.o LoadInf.o Ustring.o \
                   Persistence.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)

UNXMLIZE       = Unxmlize
UNXMLIZE_OBJS  = Main_Unxmlize.o Unxmlize.o File.o Error.o Unicode.o Xml.o Af_stdio.o \
                 ActivityLogger.o AbstractAllocator.o\
                 UnitexGetOpt.o IOBuffer.o Offsets.o Overlap.o HTMLCharacters.o\
                 AsciiSearchTree.o Alphabet.o Ustring.o Persistence.o $(SYSLIBMAPPED)\
                  $(SYSLIBLOGGER)

UNITEXTOOL   = UnitexTool
UNITEXTOOL_OBJS = Main_UnitexTool.o UnitexTool.o AbstractDelaLoad.o AbstractFst2Load.o Alphabet.o ApplyDic.o AsciiSearchTree.o \
                  AutComplementation.o AutConcat.o AutDeterminization.o AutIntersection.o AutMinimization.o \
                  AutomatonDictionary2Bin.o BitArray.o BitMasks.o Buffer.o BuildKrMwuDic.o BuildTextAutomaton.o Cassys.o Cassys_tokens.o CheckDic.o \
                  CodePages.o CompoundWordHashTable.o CompoundWordTree.o Compress.o Concord.o Concordance.o \
                  ConcorDiff.o Contexts.o Convert.o DELA.o DELA_tree.o Dico.o DictionaryTree.o \
                  DicVariables.o Diff.o DutchCompounds.o Elag.o ElagComp.o ElagDebug.o ElagFstFilesIO.o \
                  ElagFunctions.o ElagRulesCompilation.o ElagStateSet.o Error.o Evamb.o Extract.o ExtractUnits.o \
                  FIFO.o File.o Flatten.o FlattenFst2.o Fst2.o Fst2Automaton.o Fst2Check.o Fst2Check_lib.o Fst2List.o Fst2Txt.o \
                  Fst2Txt_TokenTree.o Fst2TxtAsRoutine.o GeneralDerivation.o GermanCompounds.o UnitexGetOpt.o \
                  Grf2Fst2.o Grf2Fst2_lib.o HashTable.o HTMLCharacters.o ImplodeTfst.o IOBuffer.o \
                  KrMwuDic.o LanguageDefinition.o LemmaTree.o LinearAutomaton2Txt.o List_int.o List_pointer.o List_ustring.o \
                  Locate.o LocateFst2Tags.o LocatePattern.o LocateTrace.o LocateTfst.o LocateTfst_lib.o LocateTfstMatches.o \
                  LocateMatches.o Match.o MF_DicoMorpho.o MF_DLC_inflect.o MF_FormMorpho.o MF_InflectTransd.o MF_LangMorpho.o \
                  MF_MU_graph.o MF_MU_morpho.o MF_Operators_Util.o MF_SU_morpho.o MF_Unif.o MF_Util.o \
                  MorphologicalFilters.o MorphologicalLocate.o MultiFlex.o NewLineShifts.o NormalizationFst2.o \
                  Normalize.o NormalizeAsRoutine.o NorwegianCompounds.o Offsets.o OptimizedFst2.o ParsingInfo.o Pattern.o \
                  PatternTree.o PolyLex.o PortugueseNormalization.o ProgramInvoker.o RebuildTfst.o Reconstrucao.o \
                  Reg2Grf.o RegularExpressions.o RussianCompounds.o Sentence2Grf.o SingleGraph.o Snt.o SortTxt.o \
                  Stack_int.o Stack_unichar.o Stats.o String_hash.o StringParsing.o Symbol.o Symbol_op.o \
                  SymbolAlphabet.o Table2Grf.o Tagset.o TagsetNormTfst.o TEI2Txt.o Text_parsing.o UserCancelling.o Text_tokens.o Tfst.o \
                  Tfst2Grf.o Tfst2Unambig.o Thai.o Tokenization.o Tokenize.o TransductionStack.o \
                  TransductionVariables.o Transitions.o Txt2Tfst.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o Ustring.o XMLizer.o DuplicateFile.o \
                  Korean.o OptimizedTfstTagMatching.o TransductionStackTfst.o Uncompress.o Untokenize.o Tagger.o\
                  TaggingProcess.o TrainingTagger.o TrainingProcess.o LocateCache.o Arabic.o \
                  OutputTransductionVariables.o TfstStats.o VariableUtils.o Overlap.o CompressedDic.o LoadInf.o \
                  Unxmlize.o Xml.o GrfDiff.o GrfDiff3.o Grf_lib.o GrfSvn_lib.o DebugMode.o \
                  Seq2Grf.o GrfBeauty.o GrfTest.o GrfTest_lib.o SpellCheck.o SpellChecking.o \
                  Keyboard.o Persistence.o TfstTag.o $(SYSLIBMAPPED) $(SYSLIBLOGGER)


UNITEXTOOL_LOGGER_NO_MAIN_OBJS = FilePack.o FilePackCrc32.o FilePackIo.o UniLogger.o UniLoggerAutoInstall.o \
                  FileUnPack.o ReworkArg.o UniRunLogger.o MzToolsUlp.o MzRepairUlp.o $(SYSLIBLOGGER) \
                  UnitexTool.o AbstractDelaLoad.o AbstractFst2Load.o Alphabet.o ApplyDic.o AsciiSearchTree.o \
                  AutComplementation.o AutConcat.o AutDeterminization.o AutIntersection.o AutMinimization.o \
                  AutomatonDictionary2Bin.o BitArray.o BitMasks.o Buffer.o BuildKrMwuDic.o BuildTextAutomaton.o Cassys.o Cassys_tokens.o CheckDic.o \
                  CodePages.o CompoundWordHashTable.o CompoundWordTree.o Compress.o Concord.o Concordance.o \
                  ConcorDiff.o Contexts.o Convert.o DELA.o DELA_tree.o Dico.o DictionaryTree.o \
                  DicVariables.o Diff.o DutchCompounds.o Elag.o ElagComp.o ElagDebug.o ElagFstFilesIO.o \
                  ElagFunctions.o ElagRulesCompilation.o ElagStateSet.o Error.o Evamb.o Extract.o ExtractUnits.o \
                  FIFO.o File.o Flatten.o FlattenFst2.o Fst2.o Fst2Automaton.o Fst2Check.o Fst2Check_lib.o Fst2List.o Fst2Txt.o \
                  Fst2Txt_TokenTree.o Fst2TxtAsRoutine.o GeneralDerivation.o GermanCompounds.o UnitexGetOpt.o \
                  Grf2Fst2.o Grf2Fst2_lib.o HashTable.o HTMLCharacters.o ImplodeTfst.o IOBuffer.o \
                  KrMwuDic.o LanguageDefinition.o LemmaTree.o LinearAutomaton2Txt.o List_int.o List_pointer.o List_ustring.o \
                  Locate.o LocateFst2Tags.o LocatePattern.o LocateTrace.o LocateTfst.o LocateTfst_lib.o LocateTfstMatches.o \
                  LocateMatches.o Match.o MF_DicoMorpho.o MF_DLC_inflect.o MF_FormMorpho.o MF_InflectTransd.o MF_LangMorpho.o \
                  MF_MU_graph.o MF_MU_morpho.o MF_Operators_Util.o MF_SU_morpho.o MF_Unif.o MF_Util.o \
                  MorphologicalFilters.o MorphologicalLocate.o MultiFlex.o NewLineShifts.o NormalizationFst2.o \
                  Normalize.o NormalizeAsRoutine.o NorwegianCompounds.o Offsets.o OptimizedFst2.o ParsingInfo.o Pattern.o \
                  PatternTree.o PolyLex.o PortugueseNormalization.o ProgramInvoker.o RebuildTfst.o Reconstrucao.o \
                  Reg2Grf.o RegularExpressions.o RussianCompounds.o Sentence2Grf.o SingleGraph.o Snt.o SortTxt.o \
                  Stack_int.o Stack_unichar.o Stats.o String_hash.o StringParsing.o Symbol.o Symbol_op.o \
                  SymbolAlphabet.o Table2Grf.o Tagset.o TagsetNormTfst.o TEI2Txt.o Text_parsing.o UserCancelling.o Text_tokens.o Tfst.o \
                  Tfst2Grf.o Tfst2Unambig.o Thai.o Tokenization.o Tokenize.o TransductionStack.o \
                  TransductionVariables.o Transitions.o Txt2Tfst.o Af_stdio.o ActivityLogger.o Unicode.o AbstractAllocator.o Ustring.o XMLizer.o DuplicateFile.o \
                  Korean.o OptimizedTfstTagMatching.o TransductionStackTfst.o Uncompress.o Untokenize.o Tagger.o\
                  TaggingProcess.o TrainingTagger.o TrainingProcess.o LocateCache.o Arabic.o \
                  OutputTransductionVariables.o TfstStats.o VariableUtils.o Overlap.o \
                  CompressedDic.o LoadInf.o Unxmlize.o Xml.o GrfDiff.o GrfDiff3.o Grf_lib.o \
                  GrfSvn_lib.o DebugMode.o Seq2Grf.o GrfBeauty.o GrfTest.o GrfTest_lib.o SpellCheck.o\
                  SpellChecking.o Keyboard.o Persistence.o TfstTag.o $(SYSLIBMAPPED)

UNITEXTOOL_LOGGER   = UnitexToolLogger
UNITEXTOOL_LOGGER_OBJS = Main_UnitexToolLogger.o $(UNITEXTOOL_LOGGER_NO_MAIN_OBJS)

UNITEX_JNI_OBJS = fr_umlv_unitex_jni_UnitexJni.o UnitexLibIO.o $(SYSLIBDIRIO)

TEST      = Test
TEST_OBJS = Main_Test.o $(UNITEXTOOL_LOGGER_NO_MAIN_OBJS) VirtualFiles.o


################################
################################
##  PROGRAM AND OBJECT LISTS  ##
################################
################################

ifeq ($(UNITEXTOOLLOGGERONLY), yes)


PROGS = $(UNITEXTOOL_LOGGER)
OBJS = $(UNITEXTOOL_LOGGER_OBJS)

else

ifeq ($(UNITEXTOOLONLY), yes)


PROGS = $(UNITEXTOOL) $(UNITEXTOOL_LOGGER)
OBJS = $(UNITEXTOOL_OBJS) $(UNITEXTOOL_LOGGER_OBJS)

else

PROGS = $(BUILDKRMWUDIC) $(CASSYS) $(CHECKDIC) $(COMPRESS) $(CONCORD) $(CONCORDIFF) $(CONVERT) $(DICO)\
		$(ELAG) $(ELAGCOMP) $(EVAMB) $(EXTRACT) $(FLATTEN) $(FST2CHECK) \
		$(FST2LIST) $(FST2TXT) $(GRF2FST2) $(IMPLODETFST) $(LOCATE) $(LOCATETFST) $(MULTIFLEX)\
		$(MZREPAIRULP) $(NORMALIZE) $(POLYLEX) $(REBUILDTFST) $(RECONSTRUCAO) \
		$(REG2GRF) $(SORTTXT) $(STATS) $(TABLE2GRF) $(TAGGER) $(TAGSETNORMTFST) $(TEI2TXT) \
		$(TFST2GRF) $(TFST2UNAMBIG) $(TOKENIZE) $(TRAININGTAGGER) $(TXT2TFST) $(XMLIZER) \
		$(UNCOMPRESS) $(UNTOKENIZE) $(UNITEXTOOL) $(UNITEXTOOL_LOGGER) $(UNXMLIZE) $(TEST)\
		$(GRFDIFF) $(GRFDIFF3) $(SEQ2GRF) $(GRFTEST) $(SPELLCHECK) $(TFSTTAG)

PROGS += $(TEST)


OBJS  = $(BUILDKRMWUDIC_OBJS) $(CASSYS_OBJS) $(CHECKDIC_OBJS) $(COMPRESS_OBJS) $(CONCORD_OBJS) $(CONCORDIFF_OBJS) $(CONVERT_OBJS) \
        $(DICO_OBJS) $(ELAG_OBJS)\
		$(ELAGCOMP_OBJS) $(EVAMB_OBJS) \
		$(EXTRACT_OBJS) $(FLATTEN_OBJS) $(FST2CHECK_OBJS) \
		$(FST2LIST_OBJS) $(FST2TXT_OBJS) $(GRF2FST2_OBJS) $(IMPLODETFST_OBJS) \
		$(LOCATE_OBJS) \
		$(MULTIFLEX_OBJS) $(MZREPAIRULP_OBJS) $(NORMALIZE_OBJS) $(POLYLEX_OBJS) $(REBUILDTFST_OBJS) \
		$(RECONSTRUCAO_OBJS) $(REG2GRF_OBJS) $(SORTTXT_OBJS) $(STATS_OBJS) $(TABLE2GRF_OBJS)\
		$(TAGSETNORMTFST_OBJS) $(TEST_OBJS) $(TEI2TXT_OBJS) $(TFST2GRF_OBJS) \
		$(TFST2UNAMBIG_OBJS) $(TOKENIZE_OBJS) $(TXT2TFST_OBJS) $(XMLIZER_OBJS) \
		$(UNCOMPRESS_OBJS) $(UNTOKENIZE_OBJS) $(UNITEXTOOL_OBJS) $(UNITEXTOOL_LOGGER_OBJS) \
		$(TAGGER_OBJS) $(TRAININGTAGGER_OBJS) $(UNXMLIZE_OBJS) $(GRFDIFF_OBJS) $(GRFDIFF3_OBJS)\
		$(SEQ2GRF_OBJS) $(GRFTEST_OBJS) $(SPELLCHECK_OBJS) $(TFSTTAG_OBJS)

		#$(COMPRESSKR_OBJS) $(CONSULTDIC_OBJS) $(INFLECTKR_OBJS) $(SUFFORM2RAC_OBJS) \
		#$(MERGEBIN_OBJS) $(SORTMORPH_OBJS) $(TXT2FST2KR_OBJS) \
		#$(JAMO2SYL_OBJS) $(SYL2JAMO_OBJS) $(EXTRACTCHAR_OBJS)

endif
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
ifeq ($(JNILIBRARY),yes)
all: $(LIBTRE) .libunitexjni
else
all: bin
endif
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
	rm -f $(OBJS) $(PROGS) *~ Main_*.o

clean-all-but-tre:
	rm -f $(OBJS) $(PROGS) *~ Main_*.o

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
ifeq ($(JNILIBRARY),yes)
all: $(LIBTRE) .libunitexjni
else
all: bin
endif
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

$(BIN_DIR)$(CASSYS)$(EXTENSION): $(CASSYS_OBJS)
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

$(BIN_DIR)$(FST2CHECK)$(EXTENSION): $(FST2CHECK_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(FST2LIST)$(EXTENSION): $(FST2LIST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(FST2TXT)$(EXTENSION): $(FST2TXT_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(GRF2FST2)$(EXTENSION): $(GRF2FST2_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(GRFDIFF)$(EXTENSION): $(GRFDIFF_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(GRFDIFF3)$(EXTENSION): $(GRFDIFF3_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(GRFTEST)$(EXTENSION): $(GRFTEST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(IMPLODETFST)$(EXTENSION): $(IMPLODETFST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(LOCATE)$(EXTENSION): $(LOCATE_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(LOCATETFST)$(EXTENSION): $(LOCATETFST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(MULTIFLEX)$(EXTENSION): $(MULTIFLEX_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(MZREPAIRULP)$(EXTENSION): $(MZREPAIRULP_OBJS)
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

$(BIN_DIR)$(SEQ2GRF)$(EXTENSION): $(SEQ2GRF_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(SORTTXT)$(EXTENSION): $(SORTTXT_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(SPELLCHECK)$(EXTENSION): $(SPELLCHECK_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(STATS)$(EXTENSION): $(STATS_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TABLE2GRF)$(EXTENSION): $(TABLE2GRF_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TAGGER)$(EXTENSION): $(TAGGER_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TAGSETNORMTFST)$(EXTENSION): $(TAGSETNORMTFST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TEI2TXT)$(EXTENSION): $(TEI2TXT_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TFST2GRF)$(EXTENSION): $(TFST2GRF_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TFSTTAG)$(EXTENSION): $(TFSTTAG_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TFST2UNAMBIG)$(EXTENSION): $(TFST2UNAMBIG_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TOKENIZE)$(EXTENSION): $(TOKENIZE_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TRAININGTAGGER)$(EXTENSION): $(TRAININGTAGGER_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TXT2TFST)$(EXTENSION): $(TXT2TFST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(XMLIZER)$(EXTENSION): $(XMLIZER_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(UNCOMPRESS)$(EXTENSION): $(UNCOMPRESS_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(UNTOKENIZE)$(EXTENSION): $(UNTOKENIZE_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(UNXMLIZE)$(EXTENSION): $(UNXMLIZE_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(UNITEXTOOL)$(EXTENSION): $(UNITEXTOOL_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(UNITEXTOOL_LOGGER)$(EXTENSION): $(UNITEXTOOL_LOGGER_OBJS)
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

MzToolsUlp.o: ../logger/MzToolsUlp.cpp
	$(CC) -c ../logger/MzToolsUlp.cpp $(OPTIONS)

MzRepairUlp.o: ../logger/MzRepairUlp.cpp
	$(CC) -c ../logger/MzRepairUlp.cpp $(OPTIONS)

UniLogger.o: ../logger/UniLogger.cpp
	$(CC) -c ../logger/UniLogger.cpp $(OPTIONS)
	
SyncLoggerWin.o: ../logger/SyncLoggerWin.cpp
	$(CC) -c ../logger/SyncLoggerWin.cpp $(OPTIONS)

SyncLoggerPosix.o: ../logger/SyncLoggerPosix.cpp
	$(CC) -c ../logger/SyncLoggerPosix.cpp $(OPTIONS)

SyncLoggerDummy.o: ../logger/SyncLoggerDummy.cpp
	$(CC) -c ../logger/SyncLoggerDummy.cpp $(OPTIONS)

UniLoggerAutoInstall.o: ../logger/UniLoggerAutoInstall.cpp
	$(CC) -c ../logger/UniLoggerAutoInstall.cpp $(OPTIONS)

UnitexLibDirPosix.o : ../UnitexLibAndJni/UnitexLibDirPosix.cpp
	$(CC) -c ../UnitexLibAndJni/UnitexLibDirPosix.cpp $(OPTIONS)

UnitexLibDirWin.o : ../UnitexLibAndJni/UnitexLibDirWin.cpp
	$(CC) -c ../UnitexLibAndJni/UnitexLibDirWin.cpp $(OPTIONS)

UnitexLibIO.o : ../UnitexLibAndJni/UnitexLibIO.cpp
	$(CC) -c ../UnitexLibAndJni/UnitexLibIO.cpp $(OPTIONS)

fr_umlv_unitex_jni_UnitexJni.o : ../UnitexLibAndJni/fr_umlv_unitex_jni_UnitexJni.cpp\
                                       ../UnitexLibAndJni/fr/umlv/unitex/jni/UnitexJni.java
	javac ../UnitexLibAndJni/fr/umlv/unitex/jni/UnitexJni.java
	javah -classpath ../UnitexLibAndJni -o ../UnitexLibAndJni/fr_umlv_unitex_jni_UnitexJni.h fr.umlv.unitex.jni.UnitexJni
	$(CC) ../UnitexLibAndJni/fr_umlv_unitex_jni_UnitexJni.cpp -c $(OPTIONS) -I"../logger"

# Compiling Unitex as a library under Linux-like systems
ifeq ($(SYSTEM),linux-like)


no_o_files:
	rm -f *.o

.libunitex: no_o_files $(OBJS) Test_lib.o
	rm -f Main_*.o
	$(CC) *.o $(OPTIONS) -shared -o ../bin/libunitex.so
	$(CC) Test_lib.o -L../bin -Wl,-rpath,. -lunitex -o ../bin/Test_lib

.libunitexjni: no_o_files $(UNITEX_JNI_OBJS) $(OBJS)
	rm -f Main_*.o
	$(CC) *.o $(OPTIONS) -shared -o ../bin/libUnitexJni.so
	jar cvf ../bin/UnitexJni.jar -C ../UnitexLibAndJni/ fr/umlv/unitex/jni/UnitexJni.class

else

# Compiling Unitex as a library under Windows
no_o_files:
	rm -f *.o

.libunitex: no_o_files $(LIBTRE) $(OBJS) Test_lib.o
	rm -f Main_*.o
	$(CC) *.o -shared -Wl,--export-all-symbols -L"." -ltre -o ../bin/unitex.dll
	$(CC) Test_lib.o $(OPTIONS) libtre.a ../bin/unitex.dll -o ../bin/Test_lib.exe

.libunitexjni: no_o_files $(UNITEX_JNI_OBJS) $(LIBTRE) $(OBJS)
	rm -f Main_*.o
	$(CC) *.o -shared -Wl,--export-all-symbols -L"." -ltre -o ../bin/UnitexJni.dll
	jar cvf ../bin/UnitexJni.jar -C ../UnitexLibAndJni/ fr/umlv/unitex/jni/UnitexJni.class

endif

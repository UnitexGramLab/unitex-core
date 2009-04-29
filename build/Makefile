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
#CC      = gcc -x c 
RM      = rm -f
INSTALL = cp
NAME    = unitex-src
VERSION = $(shell date +"%Y-%m-%d")
BUILD   = $(NAME)-$(VERSION)

INSTALL_COMMAND = make install
TRE_SRC = tre-0.7.5
TRE_TGZ = ../tre-0.7.5.tar.gz
TRE_TEMP = $(TRE_SRC)/TEMP
INSTALL_COMMAND = make install

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
endif

ifeq ($(LIBRARY),yes)
CFLAGS += -fPIC
DEFFLAGS += -DUNITEX_LIBRARY
endif 

override CFLAGS   += $(DEFFLAGS) -I"../Libtre" -Ilibtre/include -I"../korean" -I".."
LIBTRE   = .libtre-done
CLEAN-LIBTRE   = clean-libtre
LIBS   = -Llibtre/lib -ltre 


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

CC      = i386-mingw32msvc-g++
RM      = rm -f
INSTALL = cp

NAME    = unitex-src
VERSION = $(shell date +"%Y-%m-%d")
BUILD   = $(NAME)-$(VERSION)


#
# Compiling instructions for the TRE library
#
INSTALL_COMMAND = make install

LIBS =  -L"../Libtre" ../Libtre/tre.lib 
INCS =  -I"../korean" -I".." -I"../Libtre"
#
# Note: the -I"." inclusion is intended to deal with compilation directory problem
#       while compiling Korean programs
#
CXXINCS =  -I"../korean" -I".." 
CXXFLAGS = $(CXXINCS)  -Wall
CFLAGS = $(INCS)  -Wall

# -fPIC is not required because all code is position independent under Windows
#ifeq ($(LIBRARY),yes)
#CFLAGS += -fPIC
#endif

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

CC   = g++.exe
APPDIR = ../../../App
DEVCPP_PATH = C:/Dev-Cpp

#
# USING TRE LIBRARY
#
LIBS =  -L"$(DEVCPP_PATH)/lib" -L"../Libtre" ../Libtre/tre.lib
INCS =  -I"$(DEVCPP_PATH)/include" -I"../Libtre" -I"../korean" -I".." 
# Note: the -I".." inclusion is intended to deal with compilation directory problem
#       while compiling Korean programs
CXXINCS =  -I"$(DEVCPP_PATH)/include/c++"  -I"$(DEVCPP_PATH)/include/c++/mingw32"  -I"$(DEVCPP_PATH)/include/c++/backward"  -I"$(DEVCPP_PATH)/include"  -I"../Libtre" -I"../korean" -I".." 
CXXFLAGS = $(CXXINCS)  -Wall
CFLAGS = $(INCS)  -Wall

# -fPIC is not required because all code is position independent under Windows
#ifeq ($(LIBRARY),yes)
#CFLAGS += -fPIC
#endif

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

#############################################################################
# IMPORTANT!!! DO NOT MOVE OR REMOVE THE FOLLOWING LINE #<PROG>. IT IS USED
# TO GENERATE AUTOMATICALLY THE DEV-C++ FILES FROM THIS MAKEFILE.
#
# All the informations about objects files needed to compile a given program
# must appear between the lines #<PROG> and #</PROG>. Moreover, they must
# conform to the following rules: 
# - the name of a program must be specified by a line like:
# FOO = Foo
# - the list of the objects needed by this program must be of the form:
# FOO_OBJS = ...
#
#
#<PROG>
#############################################################################

TEST      = Test
TEST_OBJS = Main_Test.o Af_stdio.o Unicode.o Pattern.o String_hash.o IOBuffer.o Error.o \
			List_ustring.o StringParsing.o PatternTree.o List_pointer.o DELA.o\
			Alphabet.o Fst2.o Transitions.o List_int.o Tfst.o File.o SingleGraph.o\
			BitArray.o HashTable.o FIFO.o BitMasks.o Ustring.o Symbol.o\
			LanguageDefinition.o Tagset.o

CHECKDIC      = CheckDic
CHECKDIC_OBJS = Main_CheckDic.o CheckDic.o IOBuffer.o Af_stdio.o Unicode.o File.o DELA.o Alphabet.o String_hash.o\
				List_ustring.o Error.o StringParsing.o getopt.o

COMPRESS      = Compress
COMPRESS_OBJS = Main_Compress.o Compress.o IOBuffer.o Af_stdio.o Unicode.o DELA.o DictionaryTree.o Alphabet.o\
				String_hash.o AutomatonDictionary2Bin.o File.o List_int.o\
				List_ustring.o Error.o StringParsing.o getopt.o

CONCORD      = Concord
CONCORD_OBJS = Main_Concord.o Concord.o IOBuffer.o Af_stdio.o Unicode.o Text_tokens.o String_hash.o\
				List_int.o Alphabet.o Matches.o Concordance.o File.o\
				Text_parsing.o MorphologicalLocate.o LocatePattern.o\
				TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o\
				CompoundWordTree.o MorphologicalFilters.o DELA.o List_ustring.o Fst2.o \
				AbstractDelaLoad.o AbstractFst2Load.o Pattern.o LocateFst2Tags.o Error.o SortTxt.o\
				Buffer.o StringParsing.o Snt.o BitArray.o LemmaTree.o\
				List_pointer.o PatternTree.o BitMasks.o Tokenization.o Thai.o Contexts.o\
				Stack_unichar.o Transitions.o DicVariables.o\
				Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o\
				ProgramInvoker.o NewLineShifts.o

CONCORDIFF      = ConcorDiff
CONCORDIFF_OBJS = Main_ConcorDiff.o ConcorDiff.o IOBuffer.o Diff.o Concord.o Af_stdio.o Unicode.o Text_tokens.o String_hash.o List_int.o\
				Alphabet.o Matches.o Concordance.o File.o\
				Text_parsing.o MorphologicalLocate.o LocatePattern.o TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o\
				CompoundWordTree.o MorphologicalFilters.o\
				DELA.o List_ustring.o Fst2.o AbstractDelaLoad.o AbstractFst2Load.o Pattern.o LocateFst2Tags.o\
				Error.o SortTxt.o\
				Buffer.o StringParsing.o Snt.o BitArray.o LemmaTree.o List_pointer.o BitMasks.o\
				PatternTree.o Tokenization.o Thai.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
				Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o\
				ProgramInvoker.o NewLineShifts.o

CONVERT      = Convert
CONVERT_OBJS = Main_Convert.o Convert.o IOBuffer.o Af_stdio.o Unicode.o File.o CodePages.o Error.o\
				HTMLCharacters.o AsciiSearchTree.o getopt.o

DICO      = Dico
DICO_OBJS = Main_Dico.o Dico.o List_int.o DELA.o ApplyDic.o File.o Alphabet.o String_hash.o Text_tokens.o \
            List_ustring.o Af_stdio.o Unicode.o CompoundWordHashTable.o Fst2.o AbstractDelaLoad.o AbstractFst2Load.o \
            IOBuffer.o Error.o CompoundWordTree.o\
            Pattern.o ParsingInfo.o LocatePattern.o Matches.o OptimizedFst2.o \
            Text_parsing.o MorphologicalLocate.o TransductionVariables.o TransductionStack.o MorphologicalFilters.o Locate.o\
            StringParsing.o Buffer.o BitArray.o Snt.o LemmaTree.o List_pointer.o PatternTree.o\
            LocateFst2Tags.o BitMasks.o Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
            Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o ProgramInvoker.o

ELAG      = Elag
ELAG_OBJS = Main_Elag.o Elag.o IOBuffer.o Af_stdio.o Unicode.o ElagFunctions.o ElagFstFilesIO.o AutDeterminization.o \
            AutMinimization.o AutConcat.o Fst2Automaton.o \
            Symbol.o String_hash.o Tagset.o Ustring.o \
   		    ElagStateSet.o SymbolAlphabet.o Symbol_op.o Error.o LanguageDefinition.o\
            AutIntersection.o Alphabet.o StringParsing.o DELA.o List_ustring.o\
            List_int.o SingleGraph.o HashTable.o BitArray.o Fst2.o Pattern.o\
            BitMasks.o FIFO.o Transitions.o ElagDebug.o File.o getopt.o Tfst.o
 
ELAGCOMP      = ElagComp
ELAGCOMP_OBJS = Main_ElagComp.o ElagComp.o IOBuffer.o Af_stdio.o Unicode.o ElagRulesCompilation.o Fst2Automaton.o\
                ElagFstFilesIO.o String_hash.o\
                Symbol.o Ustring.o AutDeterminization.o AutIntersection.o\
                ElagStateSet.o Symbol_op.o LanguageDefinition.o Tagset.o \
	            AutMinimization.o SymbolAlphabet.o AutConcat.o AutComplementation.o\
	            Alphabet.o Error.o StringParsing.o DELA.o List_ustring.o List_int.o\
	            SingleGraph.o HashTable.o BitArray.o Fst2.o Pattern.o BitMasks.o FIFO.o File.o\
	            Transitions.o ElagDebug.o getopt.o Tfst.o

EVAMB      = Evamb
EVAMB_OBJS = Main_Evamb.o Evamb.o IOBuffer.o Af_stdio.o Unicode.o String_hash.o\
             Alphabet.o Error.o StringParsing.o Pattern.o List_ustring.o\
             List_int.o BitMasks.o DELA.o List_pointer.o SingleGraph.o\
             BitArray.o HashTable.o FIFO.o Transitions.o\
             Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o Tfst.o File.o

EXTRACT      = Extract
EXTRACT_OBJS = Main_Extract.o Extract.o IOBuffer.o LocatePattern.o Text_parsing.o MorphologicalLocate.o TransductionStack.o OptimizedFst2.o TransductionVariables.o \
               ParsingInfo.o CompoundWordTree.o  List_int.o Concordance.o \
               DELA.o List_ustring.o Fst2.o Pattern.o Af_stdio.o Unicode.o AbstractDelaLoad.o AbstractFst2Load.o\
               Text_tokens.o File.o ExtractUnits.o Alphabet.o String_hash.o Matches.o\
	           MorphologicalFilters.o Error.o Buffer.o SortTxt.o StringParsing.o BitArray.o\
	           LemmaTree.o Snt.o List_pointer.o PatternTree.o LocateFst2Tags.o BitMasks.o\
	           Tokenization.o Thai.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
	           Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o\
	           ProgramInvoker.o NewLineShifts.o

FLATTEN      = Flatten
FLATTEN_OBJS = Main_Flatten.o Flatten.o IOBuffer.o Alphabet.o Af_stdio.o Unicode.o Fst2.o Grf2Fst2_lib.o\
               AbstractFst2Load.o FlattenFst2.o List_int.o Error.o File.o SingleGraph.o\
               BitMasks.o BitArray.o HashTable.o FIFO.o Pattern.o List_ustring.o\
               String_hash.o StringParsing.o Transitions.o DELA.o\
               Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o

FST2LIST      = Fst2List
FST2LIST_OBJS = Main_Fst2List.o Fst2List.o IOBuffer.o Af_stdio.o Unicode.o Fst2.o AbstractFst2Load.o Alphabet.o\
                File.o String_hash.o Error.o TransductionVariables.o\
                StringParsing.o Pattern.o List_ustring.o List_int.o BitMasks.o\
                Transitions.o DELA.o Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o

FST2TXT      = Fst2Txt
FST2TXT_OBJS = Main_Fst2Txt.o Fst2Txt.o IOBuffer.o Af_stdio.o Unicode.o Fst2.o\
               AbstractFst2Load.o Alphabet.o ParsingInfo.o\
               TransductionVariables.o String_hash.o Error.o StringParsing.o\
               Pattern.o List_ustring.o List_int.o BitMasks.o BitArray.o\
               Fst2Txt_TokenTree.o Buffer.o Transitions.o File.o \
	           Fst2TxtAsRoutine.o DELA.o DicVariables.o TransductionStack.o\
	           Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o Stack_unichar.o

GRF2FST2      = Grf2Fst2
GRF2FST2_OBJS = Main_Grf2Fst2.o Grf2Fst2.o IOBuffer.o Alphabet.o Af_stdio.o Unicode.o Fst2.o AbstractFst2Load.o \
                Fst2Check.o Grf2Fst2_lib.o Error.o File.o SingleGraph.o\
                BitMasks.o BitArray.o List_int.o HashTable.o FIFO.o Pattern.o\
                List_ustring.o String_hash.o StringParsing.o Transitions.o DELA.o\
                Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o\
                ProgramInvoker.o


IMPLODETFST      = ImplodeTfst
IMPLODETFST_OBJS = Main_ImplodeTfst.o ImplodeTfst.o IOBuffer.o Af_stdio.o Unicode.o Tfst.o File.o SingleGraph.o\
                   String_hash.o Alphabet.o Error.o StringParsing.o Pattern.o List_ustring.o\
                   List_int.o BitMasks.o DELA.o List_pointer.o Transitions.o\
                   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o\
                   BitArray.o HashTable.o FIFO.o

LOCATE      = Locate
LOCATE_OBJS = Main_Locate.o Locate.o IOBuffer.o Af_stdio.o Unicode.o Alphabet.o DELA.o List_ustring.o String_hash.o \
              LocatePattern.o AbstractDelaLoad.o AbstractFst2Load.o Fst2.o Text_tokens.o List_int.o\
              Pattern.o CompoundWordTree.o LocateFst2Tags.o \
              OptimizedFst2.o Text_parsing.o MorphologicalLocate.o Matches.o TransductionVariables.o TransductionStack.o\
              ParsingInfo.o File.o MorphologicalFilters.o Error.o StringParsing.o\
              BitArray.o LemmaTree.o List_pointer.o PatternTree.o BitMasks.o Buffer.o\
              Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
              Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o ProgramInvoker.o

LOCATETFST      = LocateTfst
LOCATETFST_OBJS = Main_LocateTfst.o LocateTfst.o IOBuffer.o Af_stdio.o Unicode.o Error.o getopt.o LocateTfst_lib.o \
                  Tfst.o File.o Ustring.o SingleGraph.o List_int.o BitMasks.o FIFO.o Transitions.o \
                  Symbol.o LanguageDefinition.o Tagset.o String_hash.o StringParsing.o DELA.o \
                  Alphabet.o List_ustring.o HashTable.o BitArray.o Fst2.o AbstractFst2Load.o Pattern.o \
                  LocateTfstMatches.o MorphologicalFilters.o


MULTIFLEX      = MultiFlex
MULTIFLEX_OBJS = Main_MultiFlex.o MultiFlex.o Alphabet.o MF_DLC_inflect.o MF_DicoMorpho.o MF_FormMorpho.o MF_LangMorpho.o\
                 MF_InflectTransd.o  MF_MU_graph.o MF_MU_morpho.o MF_SU_morpho.o MF_Unif.o MF_Util.o\
                 Fst2.o AbstractFst2Load.o Af_stdio.o Unicode.o Error.o IOBuffer.o File.o DELA.o StringParsing.o List_ustring.o\
                 String_hash.o Pattern.o List_int.o BitMasks.o Transitions.o Grf2Fst2.o\
                 Grf2Fst2_lib.o SingleGraph.o BitArray.o FIFO.o HashTable.o Fst2Check.o\
                 Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o\
                 MF_Operators_Util.o getopt.o ProgramInvoker.o

NORMALIZE      = Normalize
NORMALIZE_OBJS = Main_Normalize.o Normalize.o IOBuffer.o Af_stdio.o Unicode.o File.o DELA.o List_ustring.o String_hash.o\
				Alphabet.o Error.o StringParsing.o Buffer.o NormalizeAsRoutine.o getopt.o

POLYLEX      = PolyLex
POLYLEX_OBJS = Main_PolyLex.o PolyLex.o IOBuffer.o Af_stdio.o Unicode.o Alphabet.o DELA.o AbstractDelaLoad.o List_ustring.o String_hash.o\
				NorwegianCompounds.o GermanCompounds.o File.o\
				RussianCompounds.o GeneralDerivation.o Error.o StringParsing.o\
				DutchCompounds.o getopt.o

REBUILDTFST      = RebuildTfst
REBUILDTFST_OBJS = Main_RebuildTfst.o RebuildTfst.o IOBuffer.o Af_stdio.o Unicode.o String_hash.o AbstractFst2Load.o Fst2.o\
                   File.o Alphabet.o Error.o StringParsing.o\
                   Pattern.o List_ustring.o List_int.o BitMasks.o BitArray.o\
                   Transitions.o DELA.o\
                   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o\
                   getopt.o Tfst.o Grf2Fst2.o Grf2Fst2_lib.o SingleGraph.o\
                   HashTable.o FIFO.o Fst2Check.o ProgramInvoker.o

RECONSTRUCAO      = Reconstrucao
RECONSTRUCAO_OBJS = Main_Reconstrucao.o Reconstrucao.o OptimizedFst2.o ParsingInfo.o CompoundWordTree.o\
               DELA.o AbstractDelaLoad.o List_ustring.o Fst2.o AbstractFst2Load.o IOBuffer.o TransductionStack.o Pattern.o \
               Af_stdio.o Unicode.o Alphabet.o Matches.o File.o NormalizationFst2.o\
	            PortugueseNormalization.o TransductionVariables.o \
               Text_parsing.o MorphologicalLocate.o String_hash.o LocatePattern.o Text_tokens.o List_int.o\
               Sentence2Grf.o MorphologicalFilters.o Error.o StringParsing.o BitArray.o\
               LemmaTree.o List_pointer.o PatternTree.o LocateFst2Tags.o BitMasks.o\
               Buffer.o Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
               Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o \
               Tfst.o SingleGraph.o FIFO.o HashTable.o

REG2GRF      = Reg2Grf
REG2GRF_OBJS = Main_Reg2Grf.o Reg2Grf.o IOBuffer.o Af_stdio.o Unicode.o RegularExpressions.o Error.o File.o\
               List_int.o Stack_int.o getopt.o

SORTTXT      = SortTxt
SORTTXT_OBJS = Main_SortTxt.o SortTxt.o IOBuffer.o Af_stdio.o Unicode.o Error.o Thai.o getopt.o\
               ProgramInvoker.o

TABLE2GRF      = Table2Grf
TABLE2GRF_OBJS = Main_Table2Grf.o Table2Grf.o IOBuffer.o Af_stdio.o Unicode.o File.o Error.o getopt.o

TAGSETNORMTFST = TagsetNormTfst
TAGSETNORMTFST_OBJS = Main_TagsetNormTfst.o TagsetNormTfst.o IOBuffer.o Af_stdio.o Unicode.o ElagFstFilesIO.o Ustring.o \
					String_hash.o Symbol.o LanguageDefinition.o Tagset.o\
					Error.o File.o StringParsing.o DELA.o List_ustring.o Alphabet.o\
					List_int.o SingleGraph.o HashTable.o BitArray.o Fst2.o AbstractFst2Load.o Pattern.o\
					BitMasks.o FIFO.o Transitions.o\
					Symbol_op.o Fst2Automaton.o getopt.o Tfst.o

TEI2TXT      = TEI2Txt
TEI2TXT_OBJS = Main_TEI2Txt.o TEI2Txt.o IOBuffer.o Af_stdio.o Unicode.o Error.o File.o HTMLCharacters.o\
               AsciiSearchTree.o getopt.o

TFST2GRF      = Tfst2Grf
TFST2GRF_OBJS = Main_Tfst2Grf.o Tfst2Grf.o IOBuffer.o Af_stdio.o Unicode.o Tfst.o Sentence2Grf.o\
               File.o List_int.o Error.o StringParsing.o Pattern.o\
               List_ustring.o String_hash.o BitMasks.o BitArray.o DELA.o\
               Alphabet.o Transitions.o Symbol.o Symbol_op.o LanguageDefinition.o\
               Ustring.o Tagset.o getopt.o SingleGraph.o HashTable.o FIFO.o

TFST2UNAMBIG      = Tfst2Unambig
TFST2UNAMBIG_OBJS = Main_Tfst2Unambig.o Tfst2Unambig.o Tfst.o Error.o IOBuffer.o LinearAutomaton2Txt.o\
                   Af_stdio.o Unicode.o Pattern.o List_ustring.o StringParsing.o String_hash.o\
                   List_int.o BitMasks.o Transitions.o DELA.o Alphabet.o\
                   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o\
                   getopt.o SingleGraph.o BitArray.o HashTable.o FIFO.o File.o

TOKENIZE      = Tokenize
TOKENIZE_OBJS = Main_Tokenize.o Tokenize.o IOBuffer.o Af_stdio.o Unicode.o Alphabet.o String_hash.o\
				File.o DELA.o AbstractDelaLoad.o List_ustring.o HashTable.o Error.o StringParsing.o getopt.o

TXT2TFST      = Txt2Tfst
TXT2TFST_OBJS = Main_Txt2Tfst.o Txt2Tfst.o IOBuffer.o Text_tokens.o Alphabet.o Af_stdio.o Unicode.o DELA_tree.o\
                DELA.o List_ustring.o String_hash.o BuildTextAutomaton.o List_int.o\
                NormalizationFst2.o Fst2.o AbstractFst2Load.o File.o Error.o\
                StringParsing.o Pattern.o BitMasks.o Grf2Fst2_lib.o SingleGraph.o\
                FIFO.o BitArray.o HashTable.o Transitions.o Matches.o\
                Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o getopt.o\
                Tfst.o NewLineShifts.o ProgramInvoker.o\
                Txt2Fst2Kr.o bitmap.o etc.o segtype.o Korean.o

XMLIZER      = XMLizer
XMLIZER_OBJS = Main_XMLizer.o XMLizer.o IOBuffer.o Af_stdio.o Unicode.o Fst2.o AbstractFst2Load.o Alphabet.o \
               TransductionVariables.o String_hash.o Error.o StringParsing.o File.o\
               Pattern.o List_ustring.o List_int.o BitMasks.o Fst2TxtAsRoutine.o\
               NormalizeAsRoutine.o DELA.o Buffer.o Fst2Txt_TokenTree.o ParsingInfo.o Transitions.o \
	           getopt.o DicVariables.o TransductionStack.o \
	           Symbol.o Symbol_op.o LanguageDefinition.o Tagset.o Ustring.o Stack_unichar.o

UNITEXTOOL   = UnitexTool
UNITEXTOOL_OBJS = Main_UnitexTool.o UnitexTool.o AbstractDelaLoad.o AbstractFst2Load.o Alphabet.o ApplyDic.o AsciiSearchTree.o AutComplementation.o \
                  AutConcat.o AutDeterminization.o AutIntersection.o AutMinimization.o \
                  AutomatonDictionary2Bin.o BitArray.o BitMasks.o Buffer.o BuildTextAutomaton.o CheckDic.o \
                  CodePages.o CompoundWordHashTable.o CompoundWordTree.o Compress.o Concord.o Concordance.o \
                  ConcordanceTfst.o ConcorDiff.o Contexts.o Convert.o DELA.o DELA_tree.o Dico.o DictionaryTree.o \
                  DicVariables.o Diff.o DutchCompounds.o Elag.o ElagComp.o ElagDebug.o ElagFstFilesIO.o \
                  ElagFunctions.o ElagRulesCompilation.o ElagStateSet.o Error.o Evamb.o Extract.o ExtractUnits.o \
                  FIFO.o File.o Flatten.o FlattenFst2.o Fst2.o Fst2Automaton.o Fst2Check.o Fst2List.o Fst2Txt.o \
                  Fst2Txt_TokenTree.o Fst2TxtAsRoutine.o GeneralDerivation.o GermanCompounds.o getopt.o \
                  Grf2Fst2.o Grf2Fst2_lib.o HashTable.o HTMLCharacters.o ImplodeTfst.o IOBuffer.o \
                  LanguageDefinition.o LemmaTree.o LinearAutomaton2Txt.o List_int.o List_pointer.o List_ustring.o \
                  Locate.o LocateFst2Tags.o LocatePattern.o LocateTfst.o LocateTfst_lib.o LocateTfstMatches.o \
                  Matches.o MF_DicoMorpho.o MF_DLC_inflect.o MF_FormMorpho.o MF_InflectTransd.o MF_LangMorpho.o \
                  MF_MU_graph.o MF_MU_morpho.o MF_Operators_Util.o MF_SU_morpho.o MF_Unif.o MF_Util.o \
                  MorphologicalFilters.o MorphologicalLocate.o MultiFlex.o NewLineShifts.o NormalizationFst2.o \
                  Normalize.o NormalizeAsRoutine.o NorwegianCompounds.o OptimizedFst2.o ParsingInfo.o Pattern.o \
                  PatternTree.o PolyLex.o PortugueseNormalization.o ProgramInvoker.o RebuildTfst.o Reconstrucao.o \
                  Reg2Grf.o RegularExpressions.o RussianCompounds.o Sentence2Grf.o SingleGraph.o Snt.o SortTxt.o \
                  Stack_int.o Stack_unichar.o String_hash.o StringParsing.o Symbol.o Symbol_op.o SymbolAlphabet.o \
                  Table2Grf.o Tagset.o TagsetNormTfst.o TEI2Txt.o Text_parsing.o Text_tokens.o Tfst.o Tfst2Grf.o \
                  Tfst2Unambig.o Thai.o Tokenization.o Tokenize.o TransductionStack.o TransductionVariables.o \
                  Transitions.o Txt2Tfst.o Af_stdio.o Unicode.o Ustring.o XMLizer.o Txt2Fst2Kr.o bitmap.o etc.o \
                  segtype.o Korean.o

######################
## Korean programs  ##
######################

COMPRESSKR	= CompressKr
COMPRESSKR_OBJS = Main_CompressKr.o CompressKr.o IOBuffer.o etc.o pro_type.o \
                  String_hash.o File.o Af_stdio.o Unicode.o Alphabet.o Error.o StringParsing.o

CONSULTDIC = ConsultDic
CONSULTDIC_OBJS = Main_ConsultDic.o ConsultDic.o IOBuffer.o etc.o pro_type.o \
		    Af_stdio.o Unicode.o Fst2.o File.o Error.o Alphabet.o Pattern.o List_ustring.o\
		    StringParsing.o String_hash.o List_int.o BitMasks.o Transitions.o DELA.o\
		    Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o

INFLECTKR = InflectKr
INFLECTKR_OBJS  = Main_InflectKr.o InflectKr.o IOBuffer.o etc.o pro_type.o \
                  Af_stdio.o Unicode.o Fst2.o File.o Error.o Pattern.o List_ustring.o\
                  String_hash.o StringParsing.o List_int.o BitMasks.o Transitions.o DELA.o\
                  Alphabet.o Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o

SUFFORM2RAC  = SufForm2Rac
SUFFORM2RAC_OBJS  = Main_SufForm2Rac.o SufForm2Rac.o IOBuffer.o etc.o Af_stdio.o Unicode.o File.o Error.o

MERGEBIN  = MergeBin
MERGEBIN_OBJS  = Main_MergeBin.o MergeBin.o IOBuffer.o etc.o pro_type.o Af_stdio.o Unicode.o File.o Error.o

SORTMORPH  = SortMorph
SORTMORPH_OBJS  = Main_SortMorph.o SortMorph.o IOBuffer.o etc.o File.o Af_stdio.o Unicode.o Error.o

JAMO2SYL  = Jamo2Syl
JAMO2SYL_OBJS  = Main_Jamo2Syl.o Jamo2Syl.o IOBuffer.o etc.o bitmap.o \
                  codeForKorean.o segtype.o \
                  pro_type.o jamoCodage.o\
                  Af_stdio.o Unicode.o File.o Fst2.o Error.o Pattern.o\
                  List_ustring.o String_hash.o StringParsing.o List_int.o BitMasks.o\
                  Transitions.o DELA.o Alphabet.o\
                  Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o

SYL2JAMO  = Syl2Jamo
SYL2JAMO_OBJS  = Main_Syl2Jamo.o Syl2Jamo.o IOBuffer.o jamoCodage.o etc.o \
                 pro_type.o codeForKorean.o segtype.o \
                 Af_stdio.o Unicode.o File.o Error.o

TXT2FST2KR  = Txt2Fst2Kr
TXT2FST2KR_OBJS  = Main_Txt2Fst2Kr.o Txt2Fst2Kr.o IOBuffer.o bitmap.o etc.o segtype.o \
		   Af_stdio.o Unicode.o File.o Fst2.o Error.o Pattern.o List_ustring.o\
		   String_hash.o StringParsing.o List_int.o BitMasks.o Transitions.o DELA.o Alphabet.o\
		   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o

EXTRACTCHAR  = ExtractChar
EXTRACTCHAR_OBJS  = Main_ExtractChar.o ExtractChar.o IOBuffer.o codeForKorean.o bitmap.o etc.o segtype.o \
		            jamoCodage.o pro_type.o Af_stdio.o Unicode.o File.o Fst2.o Error.o\
		            Pattern.o List_ustring.o String_hash.o StringParsing.o List_int.o BitMasks.o\
		            Transitions.o DELA.o Alphabet.o\
		            Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o

#############################################################################
# IMPORTANT!!! DO NOT MOVE OR REMOVE THE FOLLOWING LINE #</PROG>. IT IS USED
# TO GENERATE THE DEV-C++ FILES FROM THIS MAKEFILE.
#</PROG>
#############################################################################


################################
################################
##  PROGRAM AND OBJECT LISTS  ##
################################
################################

PROGS__ = $(LOCATE) 

PROGS = $(CHECKDIC) $(COMPRESS) $(CONCORD) $(CONCORDIFF) $(CONVERT) $(DICO)\
		$(ELAG) $(ELAGCOMP) $(EVAMB) $(EXTRACT) $(FLATTEN) \
		$(FST2LIST) $(FST2TXT) $(GRF2FST2) $(IMPLODETFST) $(LOCATE) $(LOCATETFST) $(MULTIFLEX)\
		$(NORMALIZE) $(POLYLEX) $(REBUILDTFST) $(RECONSTRUCAO) \
		$(REG2GRF) $(SORTTXT) $(TABLE2GRF) $(TAGSETNORMTFST) $(TEI2TXT) \
		$(TFST2GRF) $(TFST2UNAMBIG) $(TOKENIZE) $(TXT2TFST) $(XMLIZER) $(UNITEXTOOL)

ifeq ($(64BITS),no)
PROGS += $(INFLECTKR) $(SUFFORM2RAC) $(COMPRESSKR) $(MERGEBIN) $(CONSULTDIC) $(SORTMORPH) \
         $(TXT2FST2KR) $(JAMO2SYL) $(SYL2JAMO) $(EXTRACTCHAR)
endif


BINARIES = $(patsubst %,$(BIN_DIR)%$(EXTENSION),$(PROGS))

OBJS  = $(CHECKDIC_OBJS) $(COMPRESS_OBJS) $(CONCORD_OBJS) $(CONCORDIFF_OBJS) $(CONVERT_OBJS) \
        $(DICO_OBJS) $(ELAG_OBJS)\
		$(ELAGCOMP_OBJS) $(EVAMB_OBJS) \
		$(EXTRACT_OBJS) $(FLATTEN_OBJS) \
		$(FST2LIST_OBJS) $(FST2TXT_OBJS) $(GRF2FST2_OBJS) $(IMPLODETFST_OBJS) \
		$(LOCATE_OBJS) \
		$(MULTIFLEX_OBJS) $(NORMALIZE_OBJS) $(POLYLEX_OBJS) $(REBUILDTFST_OBJS) \
		$(RECONSTRUCAO_OBJS) $(REG2GRF_OBJS) $(SORTTXT_OBJS) $(TABLE2GRF_OBJS)\
		$(TAGSETNORMTFST_OBJS) $(TEST_OBJS) $(TEI2TXT_OBJS) $(TFST2GRF_OBJS) \
		$(TFST2UNAMBIG_OBJS) $(TOKENIZE_OBJS) $(TXT2TFST_OBJS) $(XMLIZER_OBJS) \
		$(UNITEXTOOL_OBJS)
		
		#$(COMPRESSKR_OBJS) $(CONSULTDIC_OBJS) $(INFLECTKR_OBJS) $(SUFFORM2RAC_OBJS) \
		#$(MERGEBIN_OBJS) $(SORTMORPH_OBJS) $(TXT2FST2KR_OBJS) \
		#$(JAMO2SYL_OBJS) $(SYL2JAMO_OBJS) $(EXTRACTCHAR_OBJS)  \


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
	@echo
	@echo "All is done. Type '$(INSTALL_COMMAND)' to install programs in '$(APPDIR)'."
	@echo
endif

bin: $(LIBTRE) $(BINARIES)

%.o: ../%.cpp
	$(CC) -c $(CFLAGS) $<

$(TRE_SRC): $(TRE_TGZ)
	@echo "unpacking TRE library ..."
	tar xzf $(TRE_TGZ)

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

ifeq ($(SYSTEM),windows)

#######################
#######################
##  WINDOWS TARGETS  ##
#######################
#######################
ifeq ($(LIBRARY),yes)
all: $(LIBTRE) .libunitex
else
all: bin
	@echo ---------------------------------------------------------------------------- 
	@echo All is done. You need to move .exe programs from '$(BIN_DIR)' to '$(APPDIR)'.
endif

bin: $(BINARIES)

%.o: ../%.cpp
	$(CC) -c $(CFLAGS) $<

clean: 
	rm -f *.o $(BIN_DIR)*.exe $(BIN_DIR)unitex.dll

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
ifeq ($(LIBRARY),yes)
all: $(LIBTRE) .libunitex
else
all: bin
	@echo
	@echo "All is done. Type '$(INSTALL_COMMAND)' to install programs in '$(APPDIR)'."
	@echo
endif

bin: $(BINARIES)

%.o: ../%.cpp
	$(CC) -c $(CFLAGS) $<

clean: 
	rm -f $(OBJ_DIR)$(OBJS) $(BIN_DIR)*.exe $(BIN_DIR)unitex.dll Main_*.o

#############################
#############################
##  MINGW WINDOWS TARGETS  ##
#############################
#############################
endif 
endif

$(BIN_DIR)$(TEST)$(EXTENSION): Test.o
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
	
$(BIN_DIR)$(UNITEXTOOL)$(EXTENSION): $(UNITEXTOOL_OBJS)
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

.libunitex: no_o_files $(OBJS) Test_lib.o
	rm -f Main_*.o
	$(CC) *.o -shared -Wl,--export-all-symbols ../Libtre/tre.dll -o ../bin/unitex.dll
	$(CC) Test_lib.o $(OPTIONS) ../Libtre/tre.dll ../bin/unitex.dll -o ../bin/Test_lib.exe

endif
	

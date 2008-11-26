#
# This is the multi-system makefile of Unitex (http://igm.univ-mlv.fr/~unitex/)
# Compilation under Windows is supposed to be done with Dev-C++. To compile under
# Windows, type:
#
#    make SYSTEM=windows
#
# If you don't want to use the TRE library, use the 'USE_TRE_LIBRARY=no' parameter:
#
#    make USE_TRE_LIBRARY=no
#
# If have a 64 bits machine, you must use '64BITS=yes':
#
#    make 64BITS=yes
#
#
# By default, 'make' will compile Unitex for a  32 bits Linux, using the TRE library
#
#
# The special command
#
#    make SYSTEM=mingw32
#
# can be used to generate Windows executables under Linux, using the mingw32 tools.
#


################################
##  COMPILATION INSTRUCTIONS  ##
################################
################################

SYSTEM = linux-like
USE_TRE_LIBRARY = yes
64BITS = no

OBJ_DIR = build/
BIN_DIR = ../bin/


#
# everything judy: 
#
# Note: This variant of Judy is obtained by:
#       svn co http://svn.pugscode.org/pugs/third-party/judy/Judy-1.0.3 -r 10611
#       later revisions seem to have their build system modified to fit better into that of PUGS.
#

#LIBJUDY = Judy
#JUDY_SRC = Judy-1.0.3
#JUDY_TGZ = ../Judy-1.0.3.tar.gz
#CLEAN-JUDY = clean-Judy

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

#
# Compiling instructions for the TRE library
#
# (by default, the TRE library is used)
#
INSTALL_COMMAND = make USE_TRE_LIBRARY=yes install
ifeq  ($(USE_TRE_LIBRARY),yes)
TRE_SRC = tre-0.7.2
TRE_TGZ = ../tre-0.7.2.tar.gz
TRE_TEMP = $(TRE_SRC)/TEMP
INSTALL_COMMAND = make install
endif

#APPDIR = where to install binaries
APPDIR = ../../../App

ifeq ($(USE_TRE_LIBRARY),yes)
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
override CFLAGS   += $(DEFFLAGS) -Ilibtre/include -I"../korean" -I".." #-IJudy/include
LIBTRE   = .libtre-done
CLEAN-LIBTRE   = clean-libtre
LIBS   = -Llibtre/lib -ltre #-LJudy/lib -lJudy #-ldb
else
#
# IGNORING TRE LIBRARY
#
DEFFLAGS = -D_NOT_UNDER_WINDOWS -DDO_NOT_USE_TRE_LIBRARY
CFLAGS   = -Wall -W 
ifeq ($(DEBUG), yes)
CFLAGS += -g -O0 
endif
override CFLAGS   += $(DEFFLAGS) -I"../korean" -I".." #-IJudy/include
LIBTRE   = 
CLEAN-LIBTRE   = 
LIBS   = #-LJudy/lib -lJudy
endif

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
# (by default, the TRE library is used)

INSTALL_COMMAND = make USE_TRE_LIBRARY=yes install



ifeq ($(USE_TRE_LIBRARY),yes)

LIBS =  ../Libtre/tre.lib #-L../$(JUDY_SRC)-win32/lib -lJudy
INCS =  -I"../korean" -I".." #-I"../$(JUDY_SRC)-win32/include"
#
# Note: the -I"." inclusion is intended to deal with compilation directory problem
#       while compiling Korean programs
#
CXXINCS =  -I"../korean" -I".." #-I"../$(JUDY_SRC)-win32/include"
CXXFLAGS = $(CXXINCS)  -Wall
CFLAGS = $(INCS)  -Wall

else

LIBS =  #../$(JUDY_SRC)-win32/lib/Judy.lib 
INCS =  -I"../korean" -I".." -I"../" #-I"../$(JUDY_SRC)-win32/include"
#
# Note: the -I"." inclusion is intended to deal with compilation directory problem
#       while compiling Korean programs
#
CXXINCS =
CXXFLAGS = $(CXXINCS)  -Wall
CFLAGS = $(INCS)  -Wall -DDO_NOT_USE_TRE_LIBRARY

endif

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

ifeq ($(USE_TRE_LIBRARY),yes)
#
# USING TRE LIBRARY
#
LIBS =  -L"$(DEVCPP_PATH)/lib" -L"../Libtre" ../Libtre/tre.lib  #-L../$(JUDY_SRC)-win32/lib -lJudy
INCS =  -I"$(DEVCPP_PATH)/include" -I"../Libtre" -I"../korean" -I".." #-I"../$(JUDY_SRC)-win32/include"
# Note: the -I".." inclusion is intended to deal with compilation directory problem
#       while compiling Korean programs
CXXINCS =  -I"$(DEVCPP_PATH)/include/c++"  -I"$(DEVCPP_PATH)/include/c++/mingw32"  -I"$(DEVCPP_PATH)/include/c++/backward"  -I"$(DEVCPP_PATH)/include"  -I"../Libtre" -I"../korean" -I".." 
CXXFLAGS = $(CXXINCS)  -Wall
CFLAGS = $(INCS)  -Wall
else
#
# IGNORING TRE LIBRARY
#
LIBS =  -L"$(DEVCPP_PATH)/lib" #-L../$(JUDY_SRC)-win32/lib -lJudy
INCS =  -I"$(DEVCPP_PATH)/include" -I"../korean" -I".." #-I"../$(JUDY_SRC)-win32/include"
# Note: the -I"." inclusion is intended to deal with compilation directory problem
#       while compiling Korean programs
CXXINCS =  -I"$(DEVCPP_PATH)/include/c++"  -I"$(DEVCPP_PATH)/include/c++/mingw32"  -I"$(DEVCPP_PATH)/include/c++/backward"  -I"$(DEVCPP_PATH)/include"
CXXFLAGS = $(CXXINCS)  -Wall
CFLAGS = $(INCS)  -Wall -D_NOT_UNDER_WINDOWS -DDO_NOT_USE_TRE_LIBRARY
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
TEST_OBJS = Test.o Unicode.o Pattern.o String_hash.o IOBuffer.o Error.o \
			List_ustring.o StringParsing.o PatternTree.o List_pointer.o DELA.o\
			Alphabet.o Fst2.o Transitions.o List_int.o

CHECKDIC      = CheckDic
CHECKDIC_OBJS = IOBuffer.o CheckDic.o Unicode.o File.o DELA.o Alphabet.o String_hash.o\
				List_ustring.o Error.o StringParsing.o getopt.o

#COLLOC         = Colloc
#COLLOC_OBJS    = IOBuffer.o Colloc.o CollocMain.o Collocation.o Unicode.o Text_tokens.o Array.o  \
#				List_int.o Alphabet.o Matches.o Frequency.o File.o String_hash.o                 \
#				Text_parsing.o MorphologicalLocate.o LocatePattern.o Stack_int.o                                       \
#				TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o        \
#				CompoundWordTree.o MorphologicalFilters.o DELA.o List_ustring.o Fst2.o Pattern.o \
#				LocateFst2Tags.o Error.o SortTxtMain.o                                           \
#				Buffer.o StringParsing.o Snt.o BitArray.o LemmaTree.o                            \
#				List_pointer.o PatternTree.o BitMasks.o Tokenization.o Thai.o Contexts.o         \
#				Stack_unichar.o Transitions.o Buffer_ng.o getopt.o DicVariables.o

COMPRESS      = Compress
COMPRESS_OBJS = IOBuffer.o Compress.o Unicode.o DELA.o DictionaryTree.o Alphabet.o\
				String_hash.o AutomatonDictionary2Bin.o File.o List_int.o\
				List_ustring.o Error.o StringParsing.o getopt.o

CONCORD      = Concord
CONCORD_OBJS = IOBuffer.o Concord.o ConcordMain.o Unicode.o Text_tokens.o String_hash.o\
				List_int.o Alphabet.o Matches.o Concordance.o File.o\
				Text_parsing.o MorphologicalLocate.o LocatePattern.o\
				TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o\
				CompoundWordTree.o MorphologicalFilters.o DELA.o List_ustring.o Fst2.o Pattern.o\
				LocateFst2Tags.o Error.o SortTxtMain.o\
				Buffer.o StringParsing.o Snt.o BitArray.o LemmaTree.o\
				List_pointer.o PatternTree.o BitMasks.o Tokenization.o Thai.o Contexts.o\
				Stack_unichar.o Transitions.o DicVariables.o\
				symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o getopt.o

CONCORDIFF      = ConcorDiff
CONCORDIFF_OBJS = IOBuffer.o Diff.o ConcordMain.o ConcorDiff.o Unicode.o Text_tokens.o String_hash.o List_int.o\
				Alphabet.o Matches.o Concordance.o File.o\
				Text_parsing.o MorphologicalLocate.o LocatePattern.o TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o\
				CompoundWordTree.o MorphologicalFilters.o\
				DELA.o List_ustring.o Fst2.o Pattern.o LocateFst2Tags.o\
				Error.o SortTxtMain.o\
				Buffer.o StringParsing.o Snt.o BitArray.o LemmaTree.o List_pointer.o BitMasks.o\
				PatternTree.o Tokenization.o Thai.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
				symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o getopt.o

CONVERT      = Convert
CONVERT_OBJS = IOBuffer.o Convert.o Unicode.o File.o CodePages.o Error.o\
				HTMLCharacters.o AsciiSearchTree.o getopt.o

DICO      = Dico
DICO_OBJS = Dico.o List_int.o DELA.o ApplyDic.o File.o Alphabet.o String_hash.o Text_tokens.o \
            List_ustring.o Unicode.o CompoundWordHashTable.o Fst2.o IOBuffer.o Error.o \
            CompoundWordTree.o\
            Pattern.o ParsingInfo.o LocatePattern.o Matches.o OptimizedFst2.o \
            Text_parsing.o MorphologicalLocate.o TransductionVariables.o TransductionStack.o MorphologicalFilters.o LocateAsRoutine.o\
            StringParsing.o Buffer.o BitArray.o Snt.o LemmaTree.o List_pointer.o PatternTree.o\
            LocateFst2Tags.o BitMasks.o Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
            symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o getopt.o

ELAG      = Elag
ELAG_OBJS = IOBuffer.o Elag.o Unicode.o ElagFunctions.o ElagFstFilesIO.o AutDeterminization.o \
            AutMinimization.o AutConcat.o Fst2Automaton.o \
            symbol.o hash_str_table.o String_hash.o Tagset.o ustring.o \
   		    ElagStateSet.o SymbolAlphabet.o symbol_op.o Error.o LanguageDefinition.o\
            AutIntersection.o Alphabet.o StringParsing.o DELA.o List_ustring.o\
            List_int.o SingleGraph.o HashTable.o BitArray.o Fst2.o Pattern.o\
            BitMasks.o FIFO.o Transitions.o ElagDebug.o File.o getopt.o
 
ELAGCOMP      = ElagComp
ELAGCOMP_OBJS = IOBuffer.o ElagComp.o Unicode.o ElagRulesCompilation.o Fst2Automaton.o\
               ElagFstFilesIO.o hash_str_table.o String_hash.o\
               symbol.o ustring.o AutDeterminization.o AutIntersection.o ElagStateSet.o symbol_op.o LanguageDefinition.o Tagset.o \
	            AutMinimization.o SymbolAlphabet.o AutConcat.o AutComplementation.o\
	            Alphabet.o Error.o StringParsing.o DELA.o List_ustring.o List_int.o\
	            SingleGraph.o HashTable.o BitArray.o Fst2.o Pattern.o BitMasks.o FIFO.o File.o\
	            Transitions.o ElagDebug.o getopt.o

EVAMB      = Evamb
EVAMB_OBJS = IOBuffer.o Evamb.o Unicode.o Fst2.o String_hash.o\
             Alphabet.o Error.o StringParsing.o Pattern.o List_ustring.o\
             List_int.o BitMasks.o DELA.o List_pointer.o SingleGraph.o\
             BitArray.o HashTable.o FIFO.o Transitions.o\
             symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o getopt.o

EXPLODEFST2      = ExplodeFst2
EXPLODEFST2_OBJS = IOBuffer.o ExplodeFst2.o Unicode.o ExplodeFst2Utils.o Fst2.o String_hash.o\
                   Alphabet.o Error.o StringParsing.o Pattern.o List_ustring.o List_int.o ustring.o\
                   BitMasks.o DELA.o Transitions.o symbol.o symbol_op.o LanguageDefinition.o Tagset.o\
                   getopt.o

EXPORTDIC      = ExportDic
EXPORTDIC_OBJS = IOBuffer.o ExportDic.o Unicode.o File.o DELA.o Alphabet.o String_hash.o\
				List_ustring.o Error.o StringParsing.o


EXTRACT      = Extract
EXTRACT_OBJS = IOBuffer.o Extract.o LocatePattern.o Text_parsing.o MorphologicalLocate.o TransductionStack.o OptimizedFst2.o TransductionVariables.o \
               ParsingInfo.o CompoundWordTree.o  List_int.o Concordance.o \
               DELA.o List_ustring.o Fst2.o Pattern.o \
               Unicode.o Text_tokens.o File.o ExtractUnits.o Alphabet.o String_hash.o Matches.o\
	           MorphologicalFilters.o Error.o Buffer.o SortTxtMain.o StringParsing.o BitArray.o\
	           LemmaTree.o Snt.o List_pointer.o PatternTree.o LocateFst2Tags.o BitMasks.o\
	           Tokenization.o Thai.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
	           symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o getopt.o

FLATTEN      = Flatten
FLATTEN_OBJS = IOBuffer.o Flatten.o Alphabet.o Unicode.o Fst2.o Grf2Fst2_lib.o\
               FlattenFst2.o List_int.o Error.o File.o SingleGraph.o\
               BitMasks.o BitArray.o HashTable.o FIFO.o Pattern.o List_ustring.o\
               String_hash.o StringParsing.o Transitions.o DELA.o\
               symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o getopt.o

#FREQ         = Freq
#FREQ_OBJS    = IOBuffer.o Freq.o FreqMain.o Unicode.o Text_tokens.o String_hash.o                \
#				List_int.o Alphabet.o Matches.o Frequency.o File.o                               \
#				Text_parsing.o MorphologicalLocate.o LocatePattern.o Stack_int.o                                       \
#				TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o        \
#				CompoundWordTree.o MorphologicalFilters.o DELA.o List_ustring.o Fst2.o Pattern.o \
#				LocateFst2Tags.o Error.o SortTxtMain.o                                           \
#				Buffer.o StringParsing.o Snt.o BitArray.o LemmaTree.o                            \
#				List_pointer.o PatternTree.o BitMasks.o Tokenization.o Thai.o Contexts.o         \
#				Stack_unichar.o Transitions.o Buffer_ng.o getopt.o DicVariables.o

FST2GRF      = Fst2Grf
FST2GRF_OBJS = IOBuffer.o Fst2Grf.o Unicode.o Fst2.o Sentence2Grf.o\
               File.o List_int.o Error.o StringParsing.o Pattern.o\
               List_ustring.o String_hash.o BitMasks.o BitArray.o DELA.o\
               Alphabet.o Transitions.o symbol.o symbol_op.o LanguageDefinition.o\
               ustring.o Tagset.o getopt.o

FST2LIST      = Fst2List
FST2LIST_OBJS = IOBuffer.o Fst2List.o Unicode.o Fst2.o Alphabet.o\
                File.o String_hash.o Error.o TransductionVariables.o\
                StringParsing.o Pattern.o List_ustring.o List_int.o BitMasks.o\
                Transitions.o DELA.o symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o

FST2TXT      = Fst2Txt
FST2TXT_OBJS = IOBuffer.o Fst2Txt.o Unicode.o Fst2.o Alphabet.o ParsingInfo.o\
               TransductionVariables.o String_hash.o Error.o StringParsing.o\
               Pattern.o List_ustring.o List_int.o BitMasks.o BitArray.o\
               Fst2Txt_TokenTree.o Buffer.o Transitions.o File.o \
	           Fst2TxtAsRoutine.o DELA.o DicVariables.o\
	           symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o getopt.o

FST2UNAMBIG      = Fst2Unambig
FST2UNAMBIG_OBJS = Fst2.o Error.o Fst2Unambig.o IOBuffer.o LinearAutomaton2Txt.o\
                   Unicode.o Pattern.o List_ustring.o StringParsing.o String_hash.o\
                   List_int.o BitMasks.o Transitions.o DELA.o Alphabet.o\
                   symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o\
                   getopt.o

GRF2FST2      = Grf2Fst2
GRF2FST2_OBJS = IOBuffer.o Grf2Fst2_main.o Grf2Fst2.o Alphabet.o Unicode.o Fst2.o\
                GrfCheck.o Grf2Fst2_lib.o Error.o File.o SingleGraph.o\
                BitMasks.o BitArray.o List_int.o HashTable.o FIFO.o Pattern.o\
                List_ustring.o String_hash.o StringParsing.o Transitions.o DELA.o\
                symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o getopt.o


IMPLODEFST2      = ImplodeFst2
IMPLODEFST2_OBJS = IOBuffer.o ImplodeFst2.o Unicode.o Fst2.o\
                   String_hash.o Alphabet.o Error.o StringParsing.o Pattern.o List_ustring.o\
                   List_int.o BitMasks.o DELA.o List_pointer.o Transitions.o\
                   symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o

LOCATE      = Locate
LOCATE_OBJS = IOBuffer.o Locate.o Unicode.o Alphabet.o DELA.o List_ustring.o String_hash.o \
              LocatePattern.o Fst2.o Text_tokens.o List_int.o\
              Pattern.o CompoundWordTree.o LocateFst2Tags.o \
              OptimizedFst2.o Text_parsing.o MorphologicalLocate.o Matches.o TransductionVariables.o TransductionStack.o\
              ParsingInfo.o File.o MorphologicalFilters.o Error.o StringParsing.o LocateAsRoutine.o\
              BitArray.o LemmaTree.o List_pointer.o PatternTree.o BitMasks.o Buffer.o\
              Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
              symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o getopt.o

MERGETEXTAUTOMATON      = MergeTextAutomaton
MERGETEXTAUTOMATON_OBJS = IOBuffer.o MergeTextAutomaton.o Unicode.o String_hash.o Fst2.o\
                          File.o stack.o qsort.o grf.o Alphabet.o Error.o StringParsing.o\
                          Pattern.o List_ustring.o List_int.o BitMasks.o BitArray.o\
                          Transitions.o DELA.o\
                          symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o

MULTIFLEX      = MultiFlex
MULTIFLEX_OBJS = Alphabet.o MF_DLC_inflect.o MF_DicoMorpho.o MF_FormMorpho.o MF_Inflect.o MF_LangMorpho.o\
                 MF_InflectTransd.o  MF_MU_graph.o MF_MU_morpho.o MF_SU_morpho.o MF_Unif.o MF_Util.o\
                 Fst2.o Unicode.o Error.o IOBuffer.o File.o DELA.o StringParsing.o List_ustring.o\
                 String_hash.o Pattern.o List_int.o BitMasks.o Transitions.o Grf2Fst2_main.o\
                 Grf2Fst2_lib.o SingleGraph.o BitArray.o FIFO.o HashTable.o GrfCheck.o\
                 symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o\
                 MF_Operators_Util.o getopt.o

NORMALIZE      = Normalize
NORMALIZE_OBJS = IOBuffer.o Normalize.o Unicode.o File.o DELA.o List_ustring.o String_hash.o\
				Alphabet.o Error.o StringParsing.o Buffer.o NormalizeAsRoutine.o

POLYLEX      = PolyLex
POLYLEX_OBJS = IOBuffer.o PolyLex.o Unicode.o Alphabet.o DELA.o List_ustring.o String_hash.o\
				NorwegianCompounds.o GermanCompounds.o Compounds.o File.o\
				RussianCompounds.o GeneralDerivation.o Error.o StringParsing.o\
				DutchCompounds.o

RECONSTRUCAO      = Reconstrucao
RECONSTRUCAO_OBJS = IOBuffer.o Reconstrucao.o TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o CompoundWordTree.o\
               DELA.o List_ustring.o Fst2.o Pattern.o \
               Unicode.o Alphabet.o Matches.o File.o NormalizationFst2.o\
	            PortugueseNormalization.o\
               Text_parsing.o MorphologicalLocate.o String_hash.o LocatePattern.o Text_tokens.o List_int.o\
               Sentence2Grf.o MorphologicalFilters.o Error.o StringParsing.o BitArray.o\
               LemmaTree.o List_pointer.o PatternTree.o LocateFst2Tags.o BitMasks.o\
               Buffer.o Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o\
               symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o

REG2GRF      = Reg2Grf
REG2GRF_OBJS = IOBuffer.o Reg2Grf.o Unicode.o RegularExpressions.o Error.o File.o\
					List_int.o Stack_int.o

SORTTXT      = SortTxt
SORTTXT_OBJS = IOBuffer.o SortTxt.o Unicode.o Error.o SortTxtMain.o Thai.o

TABLE2GRF      = Table2Grf
TABLE2GRF_OBJS = IOBuffer.o Table2Grf.o Unicode.o File.o Error.o

TOKENIZE      = Tokenize
TOKENIZE_OBJS = IOBuffer.o Tokenize.o Unicode.o Alphabet.o String_hash.o\
				File.o DELA.o List_ustring.o Table_hash.o Error.o StringParsing.o

TXT2FST2      = Txt2Fst2
TXT2FST2_OBJS = IOBuffer.o Txt2Fst2.o Text_tokens.o Alphabet.o Unicode.o DELA_tree.o\
                DELA.o List_ustring.o String_hash.o TextAutomaton.o List_int.o\
                NormalizationFst2.o Fst2.o File.o Error.o\
                StringParsing.o Pattern.o BitMasks.o Grf2Fst2_lib.o SingleGraph.o\
                FIFO.o BitArray.o HashTable.o Transitions.o Matches.o\
                symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o

TAGSETNORM = TagsetNormFst2
TAGSETNORM_OBJS = IOBuffer.o TagsetNormFst2.o Unicode.o ElagFstFilesIO.o ustring.o \
					String_hash.o symbol.o LanguageDefinition.o Tagset.o\
					Error.o File.o StringParsing.o DELA.o List_ustring.o Alphabet.o\
					List_int.o SingleGraph.o HashTable.o BitArray.o Fst2.o Pattern.o\
					BitMasks.o FIFO.o Transitions.o\
					symbol_op.o Fst2Automaton.o

XMLIZER      = XMLizer
XMLIZER_OBJS = IOBuffer.o Unicode.o Fst2.o Alphabet.o \
               TransductionVariables.o String_hash.o Error.o StringParsing.o File.o\
               Pattern.o List_ustring.o List_int.o BitMasks.o Fst2TxtAsRoutine.o\
               NormalizeAsRoutine.o DELA.o Buffer.o Fst2Txt_TokenTree.o ParsingInfo.o Transitions.o \
	           XMLizer.o getopt.o DicVariables.o\
	           symbol.o symbol_op.o LanguageDefinition.o Tagset.o ustring.o

TEI2TXT      = TEI2Txt
TEI2TXT_OBJS = IOBuffer.o Unicode.o Error.o File.o TEI2Txt.o HTMLCharacters.o\
               AsciiSearchTree.o getopt.o


######################
## Korean programs  ##
######################

COMPRESSKR	= CompressKr
COMPRESSKR_OBJS = IOBuffer.o CompressKr.o etc.o pro_type.o \
                  String_hash.o File.o Unicode.o Alphabet.o Error.o StringParsing.o

CONSULTDIC = ConsultDic
CONSULTDIC_OBJS = IOBuffer.o ConsultDic.o etc.o pro_type.o \
		    Unicode.o Fst2.o File.o Error.o Alphabet.o Pattern.o List_ustring.o\
		    StringParsing.o String_hash.o List_int.o BitMasks.o Transitions.o DELA.o\
		    symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o

INFLECTKR = InflectKr
INFLECTKR_OBJS  = IOBuffer.o InflectKr.o etc.o pro_type.o \
                  Unicode.o Fst2.o File.o Error.o Pattern.o List_ustring.o\
                  String_hash.o StringParsing.o List_int.o BitMasks.o Transitions.o DELA.o\
                  Alphabet.o symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o

SUFFORM2RAC  = SufForm2Rac
SUFFORM2RAC_OBJS  = IOBuffer.o SufForm2Rac.o  etc.o \
		Unicode.o File.o Error.o

MERGEBIN  = MergeBin
MERGEBIN_OBJS  = IOBuffer.o MergeBin.o etc.o pro_type.o \
                  Unicode.o File.o Error.o

SORTMORPH  = SortMorph
SORTMORPH_OBJS  = IOBuffer.o SortMorph.o etc.o \
                  File.o Unicode.o Error.o

JAMO2SYL  = Jamo2Syl
JAMO2SYL_OBJS  = IOBuffer.o Jamo2Syl.o etc.o bitmap.o \
                  codeForKorean.o segtype.o \
                  pro_type.o jamoCodage.o\
                  Unicode.o File.o Fst2.o Error.o Pattern.o\
                  List_ustring.o String_hash.o StringParsing.o List_int.o BitMasks.o\
                  Transitions.o DELA.o Alphabet.o\
                  symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o

SYL2JAMO  = Syl2Jamo
SYL2JAMO_OBJS  = IOBuffer.o Syl2Jamo.o jamoCodage.o etc.o \
                 pro_type.o codeForKorean.o segtype.o \
                 Unicode.o File.o Error.o

TXT2FST2KR  = Txt2Fst2Kr
TXT2FST2KR_OBJS  = IOBuffer.o Txt2Fst2Kr.o bitmap.o etc.o segtype.o \
		   Unicode.o File.o Fst2.o Error.o Pattern.o List_ustring.o\
		   String_hash.o StringParsing.o List_int.o BitMasks.o Transitions.o DELA.o Alphabet.o\
		   symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o

EXTRACTCHAR  = ExtractChar
EXTRACTCHAR_OBJS  = IOBuffer.o ExtractChar.o codeForKorean.o bitmap.o etc.o segtype.o \
		            jamoCodage.o pro_type.o Unicode.o File.o Fst2.o Error.o\
		            Pattern.o List_ustring.o String_hash.o StringParsing.o List_int.o BitMasks.o\
		            Transitions.o DELA.o Alphabet.o\
		            symbol.o symbol_op.o LanguageDefinition.o ustring.o Tagset.o

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

PROGS__ = $(ELAGCOMP) #$(TEST)

PROGS = $(CHECKDIC) $(COMPRESS) $(CONCORD) $(CONCORDIFF) $(DICO)\
		$(EXTRACT) $(FLATTEN) $(FST2GRF) \
		$(FST2TXT) $(GRF2FST2) $(LOCATE) $(MERGETEXTAUTOMATON)\
		$(NORMALIZE) $(POLYLEX) $(RECONSTRUCAO) \
		$(REG2GRF) $(SORTTXT) $(TABLE2GRF) $(TOKENIZE)\
		$(TXT2FST2) $(IMPLODEFST2) \
		$(EXPLODEFST2) $(EVAMB) $(CONVERT) \
		$(TAGSETNORM) \
		$(FST2LIST) $(FST2UNAMBIG) $(MULTIFLEX) \
		$(EXPORTDIC) $(ELAG) $(ELAGCOMP) \
		$(XMLIZER) $(TEI2TXT) #$(COLLOC) $(FREQ)

ifeq ($(64BITS),no)
PROGS += $(INFLECTKR) $(SUFFORM2RAC) $(COMPRESSKR) $(MERGEBIN) $(CONSULTDIC) $(SORTMORPH) \
         $(TXT2FST2KR) $(JAMO2SYL) $(SYL2JAMO) $(EXTRACTCHAR)
endif


BINARIES = $(patsubst %,$(BIN_DIR)%$(EXTENSION),$(PROGS))

OBJS  = $(CHECKDIC_OBJS) $(COMPRESS_OBJS) $(CONCORD_OBJS) $(DICO_OBJS) $(ELAG_OBJS)\
		$(ELAGCOMP_OBJS)  \
		$(EXPLODEFST2_OBJS) $(EXTRACT_OBJS) $(FLATTEN_OBJS) $(FST2GRF_OBJS)\
		$(FST2TXT_OBJS) $(GRF2FST2_OBJS) \
		$(LOCATE_OBJS) $(MERGETEXTAUTOMATON_OBJS)\
		$(NORMALIZE_OBJS) $(POLYLEX_OBJS) $(RECONSTRUCAO_OBJS) \
		$(REG2GRF_OBJS) $(SORTTXT_OBJS) $(TABLE2GRF_OBJS)\
		$(TOKENIZE_OBJS) $(TXT2FST2_OBJS) \
		$(IMPLODEFST2_OBJS) $(EVAMB_OBJS) $(CONVERT_OBJS) \
		$(TAGSETNORM_OBJS) $(CONCORDIFF_OBJS) \
		$(COMPRESSKR_OBJS) $(FST2LIST_OBJS) $(CONSULTDIC_OBJS) $(INFLECTKR_OBJS) $(SUFFORM2RAC_OBJS) \
		$(MERGEBIN_OBJS) $(SORTMORPH_OBJS) $(TXT2FST2KR_OBJS) \
		$(JAMO2SYL_OBJS) $(SYL2JAMO_OBJS) $(EXTRACTCHAR_OBJS) $(FST2UNAMBIG_OBJS) $(MULTIFLEX_OBJS) \
		$(TEST_OBJS) $(EXPORTDIC_OBJS) \
		$(XMLIZER_OBJS) $(TEI2TXT_OBJS) #$(COLLOC_OBJS) $(FREQ_OBJS)


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

all: bin
	@echo
	@echo "All is done. Type '$(INSTALL_COMMAND)' to install programs in '$(APPDIR)'."
	@echo

#bin: $(LIBTRE) $(LIBJUDY) $(BINARIES)
bin: $(LIBTRE) $(BINARIES)

getopt.o: ../getopt.c
	$(CC) -c $(CFLAGS) ../getopt.c

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


#$(JUDY_SRC): $(JUDY_TGZ)
#	tar xzf $(JUDY_TGZ)
#
#.Judy-done:
#	touch .Judy-done
#
#Judy: .Judy-done $(JUDY_SRC)
#	@echo "Making Judy ..."
#	chmod +x $(JUDY_SRC)/configure
#	cd $(JUDY_SRC) && ./configure --disable-shared --enable-static && make
#	mkdir -p Judy/lib Judy/include
#	#cd $(JUDY_SRC)/src/obj/.libs && ar rcs libJudy.a *.o
#	cp $(JUDY_SRC)/src/obj/.libs/libJudy.a  Judy/lib/libJudy.a
#	cp $(JUDY_SRC)/src/Judy.h Judy/include/Judy.h
#
#clean-Judy:
#	rm -rf Judy 
#	rm -rf $(JUDY_SRC)
#	rm -f .Judy-done

install: bin
	$(INSTALL) $(BINARIES) $(APPDIR)


clean: $(CLEAN-LIBTRE) #$(CLEAN-JUDY)
	rm -f $(OBJS) $(PROGS) *~ korean/*.o korean/*~

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
all: bin
	@echo ---------------------------------------------------------------------------- 
	@echo All is done. You need to move .exe programs from '$(BIN_DIR)' to '$(APPDIR)'.


bin: $(BINARIES)

getopt.o: ../getopt.c
	$(CC) -c $(CFLAGS) ../getopt.c

%.o: ../%.cpp
	$(CC) -c $(CFLAGS) $<

clean: 
	rm -f $(OBJ_DIR)$(OBJS) $(BIN_DIR)*.exe

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
all: bin
	@echo
	@echo "All is done. Type '$(INSTALL_COMMAND)' to install programs in '$(APPDIR)'."
	@echo


bin: $(BINARIES)

getopt.o: ../getopt.c
	$(CC) -c $(CFLAGS) ../getopt.c

%.o: ../%.cpp
	$(CC) -c $(CFLAGS) $<

clean: 
	rm -f $(OBJ_DIR)$(OBJS) $(BIN_DIR)*.exe

#############################
#############################
##  MINGW WINDOWS TARGETS  ##
#############################
#############################
endif 
endif

$(BIN_DIR)$(TEST)$(EXTENSION): $(TEST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(CHECKDIC)$(EXTENSION): $(CHECKDIC_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

#$(BIN_DIR)$(COLLOC)$(EXTENSION): $(COLLOC_OBJS)
#	$(CC) -o $@ $+ $(OPTIONS)

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

$(BIN_DIR)$(EXPLODEFST2)$(EXTENSION): $(EXPLODEFST2_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(EXPORTDIC)$(EXTENSION): $(EXPORTDIC_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(EXTRACT)$(EXTENSION): $(EXTRACT_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(FLATTEN)$(EXTENSION): $(FLATTEN_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

#$(BIN_DIR)$(FREQ)$(EXTENSION): $(FREQ_OBJS)
#	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(FST2GRF)$(EXTENSION): $(FST2GRF_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(FST2LIST)$(EXTENSION): $(FST2LIST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(FST2TXT)$(EXTENSION): $(FST2TXT_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(FST2UNAMBIG)$(EXTENSION): $(FST2UNAMBIG_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(GRF2FST2)$(EXTENSION): $(GRF2FST2_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(IMPLODEFST2)$(EXTENSION): $(IMPLODEFST2_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(LOCATE)$(EXTENSION): $(LOCATE_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(MERGETEXTAUTOMATON)$(EXTENSION): $(MERGETEXTAUTOMATON_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(MULTIFLEX)$(EXTENSION): $(MULTIFLEX_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(NORMALIZE)$(EXTENSION): $(NORMALIZE_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(POLYLEX)$(EXTENSION): $(POLYLEX_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(RECONSTRUCAO)$(EXTENSION): $(RECONSTRUCAO_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(REG2GRF)$(EXTENSION): $(REG2GRF_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(SORTTXT)$(EXTENSION): $(SORTTXT_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TABLE2GRF)$(EXTENSION): $(TABLE2GRF_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TOKENIZE)$(EXTENSION): $(TOKENIZE_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TXT2FST2)$(EXTENSION): $(TXT2FST2_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TAGSETNORM)$(EXTENSION): $(TAGSETNORM_OBJS)
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

$(BIN_DIR)$(XMLIZER)$(EXTENSION): $(XMLIZER_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)

$(BIN_DIR)$(TEI2TXT)$(EXTENSION): $(TEI2TXT_OBJS)
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

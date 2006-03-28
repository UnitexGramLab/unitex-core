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
# By default, 'make' will compile Unitex for Linux, using the TRE library
#


################################
##  COMPILATION INSTRUCTIONS  ##
################################
################################

SYSTEM = linux-like
USE_TRE_LIBRARY = yes

OBJ_DIR = build/
BIN_DIR = ../bin/

ifeq ($(SYSTEM),linux-like)
###################
###################
##  BEGIN LINUX  ##
###################
###################
CC = g++
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
TRE_SRC = $(PWD)/../tre-0.7.2
TRE_TGZ = $(PWD)/../tre-0.7.2.tar.gz
TRE_TEMP = $(TRE_SRC)/TEMP
INSTALL_COMMAND = make install
endif

APPDIR = ../../App

ifeq ($(USE_TRE_LIBRARY),yes)
#
# USING TRE LIBRARY
#
DEFFLAGS = -D_NOT_UNDER_WINDOWS
CFLAGS   = -Wall -W $(DEFFLAGS) -Ilibtre/include -I"../korean" -I".."
LIBTRE   = libtre
CLEAN-LIBTRE   = clean-libtre
LIBS   = -Llibtre/lib -ltre
else
#
# IGNORING TRE LIBRARY
#
DEFFLAGS = -D_NOT_UNDER_WINDOWS -DDO_NOT_USE_TRE_LIBRARY
CFLAGS   = -Wall -W $(DEFFLAGS) -I"../korean" -I".."
LIBTRE   = 
CLEAN-LIBTRE   = 
LIBS   =
endif

OPTIONS = $(CFLAGS) $(LDFLAGS) $(LIBS)
EXTENSION = 

###################
###################
##  END LINUX  ##
###################
###################

else

#####################
#####################
##  BEGIN WINDOWS  ##
#####################
#####################

CC   = g++.exe
INSTALL = move
APPDIR = ../../App
DEVCPP_PATH = C:/Dev-Cpp

ifeq ($(USE_TRE_LIBRARY),yes)
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
else
#
# IGNORING TRE LIBRARY
#
LIBS =  -L"$(DEVCPP_PATH)/lib"
INCS =  -I"$(DEVCPP_PATH)/include" -I"../korean" -I".."
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



##################################
##################################
##  OBJECTS NEEDED BY PROGRAMS  ##
##################################
##################################

CHECKDIC      = CheckDic
CHECKDIC_OBJS = IOBuffer.o CheckDic.o unicode.o FileName.o DELA.o Alphabet.o String_hash.o\
				Copyright.o

COMPRESS      = Compress
COMPRESS_OBJS = IOBuffer.o Compress.o Copyright.o unicode.o DELA.o DictionaryTree.o Alphabet.o\
				String_hash.o Arbre_to_bin.o FileName.o Minimize_tree.o Liste_nombres.o

CONCORD      = Concord
CONCORD_OBJS = IOBuffer.o Concord.o ConcordMain.o unicode.o Text_tokens.o String_hash.o\
				Liste_nombres.o Alphabet.o Matches.o Concordance.o FileName.o Copyright.o\
				Text_parsing.o LocatePattern.o Loading_dic.o\
				TransductionStack.o Optimized_fst2.o TransductionVariables.o Liste_num.o\
				Arbre_mots_composes.o GF_lib.o DELA.o AutomateFst2.o Grammatical_codes.o\
				Fst2_tags_optimization.o DLC_optimization.o Pattern_transitions.o\
				Facteurs_interdits.o Flexional_codes.o Error.o Context.o

CONCORDIFF      = ConcorDiff
CONCORDIFF_OBJS = IOBuffer.o Diff.o ConcordMain.o ConcorDiff.o unicode.o Text_tokens.o String_hash.o Liste_nombres.o Alphabet.o Matches.o Concordance.o FileName.o\
               Copyright.o Text_parsing.o LocatePattern.o TransductionStack.o Optimized_fst2.o TransductionVariables.o Liste_num.o\
	       Arbre_mots_composes.o GF_lib.o\
               DELA.o AutomateFst2.o Grammatical_codes.o Loading_dic.o Fst2_tags_optimization.o DLC_optimization.o Pattern_transitions.o\
               Facteurs_interdits.o Flexional_codes.o Error.o Context.o

CONVERT      = Convert
CONVERT_OBJS = IOBuffer.o Convert.o unicode.o FileName.o Copyright.o CodePages.o

DICO      = Dico
DICO_OBJS = Dico.o Liste_nombres.o Copyright.o DELA.o Dico_application.o FileName.o Alphabet.o String_hash.o Text_tokens.o \
            Token_tree.o unicode.o Table_complex_token_hash.o AutomateFst2.o LiberationFst2.o IOBuffer.o Error.o \
            Arbre_mots_composes.o DLC_optimization.o Flexional_codes.o Facteurs_interdits.o Fst2_tags_optimization.o \
            Grammatical_codes.o Loading_dic.o Liste_num.o LocatePattern.o Matches.o Optimized_fst2.o Pattern_transitions.o \
            Text_parsing.o TransductionVariables.o TransductionStack.o GF_lib.o LocateAsRoutine.o Context.o

ELAG      = Elag
ELAG_OBJS = IOBuffer.o Elag.o unicode.o elag-functions.o language.o fst_file.o aut-deter.o \
            aut-minim.o aut-emonde.o aut-concat.o autalmot.o \
            symbol.o hash_str_table.o String_hash.o language_parsing.o ustring.o \
	    state_ens.o aut-alphabet.o symbol_op.o \
            aut-topo.o aut-inter.o aut-eval.o Copyright.o Alphabet.o

ELAGCOMP      = ElagComp
ELAGCOMP_OBJS = IOBuffer.o ElagComp.o unicode.o compgr.o autalmot.o fst_file.o hash_str_table.o String_hash.o\
               symbol.o ustring.o aut-deter.o aut-inter.o state_ens.o symbol_op.o language.o language_parsing.o \
     	       aut-minim.o aut-alphabet.o aut-emonde.o aut-concat.o aut-union.o aut-complement.o Copyright.o\
     	       Alphabet.o

EVAMB      = Evamb
EVAMB_OBJS = IOBuffer.o Evamb.o unicode.o fst2autalmot.o autalmot_old.o implose.o AutomateFst2.o String_hash.o\
             decompte.o Copyright.o Alphabet.o Error.o

EXPLOSEFST2      = ExploseFst2
EXPLOSEFST2_OBJS = IOBuffer.o ExploseFst2.o unicode.o fst2autalmot.o autalmot_old.o AutomateFst2.o String_hash.o\
                   Copyright.o Alphabet.o Error.o

EXTRACT      = Extract
EXTRACT_OBJS = IOBuffer.o Extract.o Copyright.o LocatePattern.o Text_parsing.o TransductionStack.o Optimized_fst2.o TransductionVariables.o \
               Liste_num.o Arbre_mots_composes.o  Liste_nombres.o Concordance.o \
               DELA.o AutomateFst2.o Grammatical_codes.o Loading_dic.o Fst2_tags_optimization.o DLC_optimization.o Pattern_transitions.o\
               Facteurs_interdits.o Flexional_codes.o unicode.o Text_tokens.o FileName.o Extract_units.o Alphabet.o String_hash.o Matches.o\
	            GF_lib.o Error.o Context.o

FLATTEN      = Flatten
FLATTEN_OBJS = IOBuffer.o Flatten.o Alphabet.o Copyright.o unicode.o AutomateFst2.o Grf2Fst2_lib.o\
               FlattenFst2.o Liste_nombres.o Error.o FileName.o

FST2GRF      = Fst2Grf
FST2GRF_OBJS = IOBuffer.o Fst2Grf.o unicode.o AutomateFst2.o LiberationFst2.o Sentence_to_grf.o\
               FileName.o Liste_nombres.o Copyright.o Error.o

FST2LIST      = Fst2List
FST2LIST_OBJS = IOBuffer.o Fst2List.o unicode.o AutomateFst2.o LiberationFst2.o Alphabet.o Liste_num.o\
                UnicharTree.o Copyright.o FileName.o String_hash.o TransductionVariables.o Error.o

FST2TXT      = Fst2Txt
FST2TXT_OBJS = IOBuffer.o Fst2Txt.o unicode.o AutomateFst2.o LiberationFst2.o Alphabet.o Liste_num.o UnicharTree.o\
               Copyright.o TransductionVariables.o String_hash.o Error.o

FST2UNAMBIG      = Fst2Unambig
FST2UNAMBIG_OBJS = AutomateFst2.o Copyright.o Error.o Fst2Unambig.o IOBuffer.o LinearAutomaton2Txt.o\
                   unicode.o

GRF2FST2      = Grf2Fst2
GRF2FST2_OBJS = IOBuffer.o Grf2Fst2.o Alphabet.o unicode.o AutomateFst2.o LiberationFst2.o\
                VerifierRecursion.o Copyright.o Grf2Fst2_lib.o Error.o FileName.o


IMPLOSEFST2      = ImploseFst2
IMPLOSEFST2_OBJS = IOBuffer.o ImploseFst2.o implose.o unicode.o fst2autalmot.o autalmot_old.o AutomateFst2.o\
                   String_hash.o Copyright.o Alphabet.o Error.o


INFLECT      = Inflect
INFLECT_OBJS = IOBuffer.o Inflect.o Copyright.o unicode.o AutomateFst2.o LiberationFst2.o Error.o

LOCATE      = Locate
LOCATE_OBJS = IOBuffer.o Locate.o Copyright.o unicode.o Alphabet.o DELA.o String_hash.o LocatePattern.o AutomateFst2.o Text_tokens.o Liste_nombres.o\
              Grammatical_codes.o Flexional_codes.o Facteurs_interdits.o Loading_dic.o Arbre_mots_composes.o Fst2_tags_optimization.o \
              DLC_optimization.o Optimized_fst2.o Pattern_transitions.o Text_parsing.o Matches.o TransductionVariables.o TransductionStack.o\
              Liste_num.o FileName.o GF_lib.o Error.o Context.o

MERGETEXTAUTOMATON      = MergeTextAutomaton
MERGETEXTAUTOMATON_OBJS = IOBuffer.o MergeTextAutomaton.o Copyright.o unicode.o String_hash.o AutomateFst2.o\
                          FileName.o stack.o qsort.o grf.o Alphabet.o Error.o

NORMALIZE      = Normalize
NORMALIZE_OBJS = IOBuffer.o Normalize.o Copyright.o unicode.o FileName.o DELA.o String_hash.o Alphabet.o


POLYLEX      = PolyLex
POLYLEX_OBJS = IOBuffer.o PolyLex.o Copyright.o unicode.o Alphabet.o DELA.o String_hash.o NorwegianCompounds.o GermanCompounds.o Compounds.o FileName.o RussianCompounds.o GeneralDerivation.o

RECONSTRUCAO      = Reconstrucao
RECONSTRUCAO_OBJS = IOBuffer.o Copyright.o Reconstrucao.o TransductionStack.o Optimized_fst2.o TransductionVariables.o Liste_num.o Arbre_mots_composes.o\
               DELA.o AutomateFst2.o Grammatical_codes.o Loading_dic.o Fst2_tags_optimization.o DLC_optimization.o Pattern_transitions.o\
               Facteurs_interdits.o Flexional_codes.o unicode.o Alphabet.o Matches.o FileName.o Normalization_transducer.o\
	            PortugueseNormalization.o\
               Text_parsing.o String_hash.o LocatePattern.o Text_tokens.o Liste_nombres.o String_list.o\
               Sentence_to_grf.o GF_lib.o Error.o Context.o

REG2GRF      = Reg2Grf
REG2GRF_OBJS = IOBuffer.o Reg2Grf.o Copyright.o unicode.o Regular_expression.o

SORTTXT      = SortTxt
SORTTXT_OBJS = IOBuffer.o SortTxt.o Copyright.o unicode.o

TABLE2GRF      = Table2Grf
TABLE2GRF_OBJS = IOBuffer.o Table2Grf.o Copyright.o unicode.o FileName.o

TEXTAUTOMATON2MFT      = TextAutomaton2Mft
TEXTAUTOMATON2MFT_OBJS = IOBuffer.o TextAutomaton2Mft.o Copyright.o unicode.o FileName.o AutomateFst2.o\
                         Fst2_to_Mft.o String_hash.o Alphabet.o Error.o

TOKENIZE      = Tokenize
TOKENIZE_OBJS = IOBuffer.o Tokenize.o Copyright.o unicode.o Alphabet.o String_hash.o FileName.o DELA.o Table_hash.o

TXT2FST2      = Txt2Fst2
TXT2FST2_OBJS = IOBuffer.o Txt2Fst2.o Copyright.o Text_tokens.o Alphabet.o unicode.o Load_DLF_DLC.o\
                DELA.o String_hash.o Text_automaton.o Liste_nombres.o\
                Normalization_transducer.o AutomateFst2.o String_list.o FileName.o Error.o

TAGSETNORM = TagsetNormFst2
TAGSETNORM_OBJS = IOBuffer.o tagsetnorm.o Copyright.o unicode.o autalmot.o fst_file.o ustring.o \
		  hash_str_table.o symbol.o language.o language_parsing.o aut-emonde.o

######################
## Korean programs  ##
######################

COMPRESSKR	= CompressKr
COMPRESSKR_OBJS = IOBuffer.o CompressKr.o etc.o pro_type.o \
                  String_hash.o FileName.o Copyright.o unicode.o Alphabet.o
                  
CONSULTDIC = ConsultDic
CONSULTDIC_OBJS = IOBuffer.o ConsultDic.o etc.o pro_type.o \
		    unicode.o AutomateFst2.o FileName.o Copyright.o Error.o Alphabet.o

INFLECTKR = InflectKr
INFLECTKR_OBJS  = IOBuffer.o InflectKr.o etc.o pro_type.o \
                  unicode.o Copyright.o AutomateFst2.o FileName.o Error.o

SUFFORM2RAC  = SufForm2Rac
SUFFORM2RAC_OBJS  = IOBuffer.o SufForm2Rac.o  etc.o \
		unicode.o Copyright.o FileName.o

MERGEBIN  = MergeBin
MERGEBIN_OBJS  = IOBuffer.o MergeBin.o etc.o pro_type.o \
                  unicode.o FileName.o Copyright.o

SORTMORPH  = SortMorph
SORTMORPH_OBJS  = IOBuffer.o SortMorph.o etc.o \
                  Copyright.o FileName.o unicode.o

JAMO2SYL  = Jamo2Syl
JAMO2SYL_OBJS  = IOBuffer.o Jamo2Syl.o etc.o bitmap.o \
                  codeForKorean.o segtype.o \
                  pro_type.o jamoCodage.o\
                  unicode.o FileName.o AutomateFst2.o Copyright.o Error.o

SYL2JAMO  = Syl2Jamo
SYL2JAMO_OBJS  = IOBuffer.o Syl2Jamo.o jamoCodage.o etc.o \
                 pro_type.o codeForKorean.o segtype.o \
                 unicode.o FileName.o Copyright.o

TXT2FST2KR  = Txt2Fst2Kr
TXT2FST2KR_OBJS  = IOBuffer.o Txt2Fst2Kr.o bitmap.o etc.o segtype.o \
		   unicode.o FileName.o AutomateFst2.o Copyright.o Error.o

EXTRACTCHAR  = ExtractChar
EXTRACTCHAR_OBJS  = IOBuffer.o ExtractChar.o codeForKorean.o bitmap.o etc.o segtype.o \
		            jamoCodage.o pro_type.o unicode.o FileName.o AutomateFst2.o Copyright.o Error.o


      
################################
################################
##  PROGRAM AND OBJECT LISTS  ##
################################
################################

PROGS = $(CHECKDIC) $(COMPRESS) $(CONCORD) $(CONCORDIFF) $(DICO) $(ELAGCOMP)\
		$(EXTRACT) $(FLATTEN) $(FST2GRF) \
		$(FST2TXT) $(GRF2FST2) $(INFLECT) $(LOCATE) $(MERGETEXTAUTOMATON)\
		$(NORMALIZE) $(POLYLEX) $(RECONSTRUCAO) \
		$(REG2GRF) $(SORTTXT) $(TABLE2GRF) $(TEXTAUTOMATON2MFT) $(TOKENIZE)\
		$(TXT2FST2) $(IMPLOSEFST2) \
		$(EXPLOSEFST2) $(LOADFST) $(EVAMB) $(CONVERT) \
		$(TAGSETNORM) \
		$(INFLECTKR) $(FST2LIST) $(SUFFORM2RAC) $(COMPRESSKR)\
		$(MERGEBIN) $(CONSULTDIC) $(SORTMORPH) $(TXT2FST2KR) \
		$(JAMO2SYL) $(SYL2JAMO) $(EXTRACTCHAR) $(FST2UNAMBIG)

BINARIES = $(patsubst %,$(BIN_DIR)%$(EXTENSION),$(PROGS))
	
OBJS  = $(CHECKDIC_OBJS) $(COMPRESS_OBJS) $(CONCORD_OBJS) $(DICO_OBJS) $(ELAG_OBJS)\
		$(ELAGCOMP_OBJS) \
		$(EXPLOSEFST2_OBJS) $(EXTRACT_OBJS) $(FLATTEN_OBJS) $(FST2GRF_OBJS)\
		$(FST2TXT_OBJS) $(GRF2FST2_OBJS) \
		$(INFLECT_OBJS) $(LANG_OBJS) $(LOCATE_OBJS) $(MERGETEXTAUTOMATON_OBJS)\
		$(NORMALIZE_OBJS) $(POLYLEX_OBJS) $(RECONSTRUCAO_OBJS) \
		$(REG2GRF_OBJS) $(SORTTXT_OBJS) $(TABLE2GRF_OBJS) $(TEXTAUTOMATON2MFT_OBJS)\
		$(TOKENIZE_OBJS) $(TXT2FST2_OBJS) \
		$(IMPLOSEFST2_OBJS) $(LOADFST_OBJS) $(EVAMB_OBJS) $(CONVERT_OBJS) $(SPELLCHECK_OBJS) \
		$(TAGSETNORM_OBJS) $(CONCORDIFF_OBJS) \
		$(COMPRESSKR_OBJS) $(FST2LIST_OBJS) $(CONSULTDIC_OBJS) $(INFLECTKR_OBJS) $(SUFFORM2RAC_OBJS) \
		$(MERGEBIN_OBJS) $(SORTMORPH_OBJS) $(TXT2FST2KR_OBJS) \
		$(JAMO2SYL_OBJS) $(SYL2JAMO_OBJS) $(EXTRACTCHAR_OBJS) $(FST2UNAMBIG_OBJS)



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

bin: $(LIBTRE) $(BINARIES)

%.o: ../%.cpp
	$(CC) -c $(CFLAGS) $<

.libtre-done:
	touch .libtre-done


$(TRE_SRC): $(TRE_TGZ)
	@echo "unpacking TRE library ..."
	cd .. && (cat $(TRE_TGZ) | gunzip | tar xf -)

libtre: .libtre-done $(TRE_SRC)
	@echo "Making TRE library ..."
	chmod +x $(TRE_SRC)/configure
	cd $(TRE_SRC) && ./configure --disable-shared --enable-static && make
	mkdir -p libtre/lib libtre/include
	cd $(TRE_SRC)/lib && ar rcs libtre.a *.o
	cp $(TRE_SRC)/lib/libtre.a  libtre/lib/libtre.a

clean-libtre:
	rm -rf libtre
	rm -rf $(TRE_SRC)
	rm -f .libtre-done

install: bin
	$(INSTALL) $(PROGS) $(APPDIR)


clean: $(CLEAN-LIBTRE)
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

#######################
#######################
##  WINDOWS TARGETS  ##
#######################
#######################
all: bin
	@echo ------------ 
	@echo All is done.

install: bin
	$(INSTALL) *.exe $(APPDIR)

bin: $(BINARIES)

%.o: ../%.cpp
	$(CC) -c $(CFLAGS) $<

clean: 
	rm -f $(OBJ_DIR)$(OBJS) $(BIN_DIR)*.exe

##########################
##########################
##  END WINDOWS TARGETS ##
##########################
##########################
endif



$(BIN_DIR)$(CHECKDIC)$(EXTENSION): $(CHECKDIC_OBJS)
	$(CC) -o $(BIN_DIR)$(CHECKDIC)$(EXTENSION) $(CHECKDIC_OBJS) $(OPTIONS)

$(BIN_DIR)$(COMPRESS)$(EXTENSION): $(COMPRESS_OBJS)
	$(CC) -o $(BIN_DIR)$(COMPRESS)$(EXTENSION) $(COMPRESS_OBJS) $(OPTIONS) 

$(BIN_DIR)$(CONCORD)$(EXTENSION): $(CONCORD_OBJS)
	$(CC) -o $(BIN_DIR)$(CONCORD)$(EXTENSION) $(CONCORD_OBJS) $(OPTIONS)

$(BIN_DIR)$(CONCORDIFF)$(EXTENSION): $(CONCORDIFF_OBJS)
	$(CC) -o $(BIN_DIR)$(CONCORDIFF)$(EXTENSION) $(CONCORDIFF_OBJS) $(OPTIONS)

$(BIN_DIR)$(CONVERT)$(EXTENSION): $(CONVERT_OBJS)
	$(CC) -o $(BIN_DIR)$(CONVERT)$(EXTENSION) $(CONVERT_OBJS) $(OPTIONS)

$(BIN_DIR)$(DICO)$(EXTENSION): $(DICO_OBJS)
	$(CC) -o $(BIN_DIR)$(DICO)$(EXTENSION) $(DICO_OBJS) $(OPTIONS)

$(BIN_DIR)$(ELAG)$(EXTENSION): $(ELAG_OBJS)
	$(CC) -o $(BIN_DIR)$(ELAG)$(EXTENSION) $(ELAG_OBJS) $(OPTIONS)

$(BIN_DIR)$(ELAGCOMP)$(EXTENSION): $(ELAGCOMP_OBJS)
	$(CC) -o $(BIN_DIR)$(ELAGCOMP)$(EXTENSION) $(ELAGCOMP_OBJS) $(OPTIONS)

$(BIN_DIR)$(EVAMB)$(EXTENSION): $(EVAMB_OBJS)
	$(CC) -o $(BIN_DIR)$(EVAMB)$(EXTENSION) $(EVAMB_OBJS) $(OPTIONS)

$(BIN_DIR)$(EXPLOSEFST2)$(EXTENSION): $(EXPLOSEFST2_OBJS)
	$(CC) -o $(BIN_DIR)$(EXPLOSEFST2)$(EXTENSION) $(EXPLOSEFST2_OBJS) $(OPTIONS)

$(BIN_DIR)$(EXTRACT)$(EXTENSION): $(EXTRACT_OBJS)
	$(CC) -o $(BIN_DIR)$(EXTRACT)$(EXTENSION) $(EXTRACT_OBJS) $(OPTIONS)

$(BIN_DIR)$(FLATTEN)$(EXTENSION): $(FLATTEN_OBJS)
	$(CC) -o $(BIN_DIR)$(FLATTEN)$(EXTENSION) $(FLATTEN_OBJS) $(OPTIONS)

$(BIN_DIR)$(FST2GRF)$(EXTENSION): $(FST2GRF_OBJS)
	$(CC) -o $(BIN_DIR)$(FST2GRF)$(EXTENSION) $(FST2GRF_OBJS) $(OPTIONS)

$(BIN_DIR)$(FST2LIST)$(EXTENSION): $(FST2LIST_OBJS)
	$(CC) -o $(BIN_DIR)$(FST2LIST)$(EXTENSION) $(FST2LIST_OBJS) $(OPTIONS)

$(BIN_DIR)$(FST2TXT)$(EXTENSION): $(FST2TXT_OBJS)
	$(CC) -o $(BIN_DIR)$(FST2TXT)$(EXTENSION) $(FST2TXT_OBJS) $(OPTIONS)

$(BIN_DIR)$(FST2UNAMBIG)$(EXTENSION): $(FST2UNAMBIG_OBJS)
	$(CC) -o $(BIN_DIR)$(FST2UNAMBIG)$(EXTENSION) $(FST2UNAMBIG_OBJS) $(OPTIONS)

$(BIN_DIR)$(GRF2FST2)$(EXTENSION): $(GRF2FST2_OBJS)
	$(CC) -o $(BIN_DIR)$(GRF2FST2)$(EXTENSION) $(GRF2FST2_OBJS) $(OPTIONS)

$(BIN_DIR)$(IMPLOSEFST2)$(EXTENSION): $(IMPLOSEFST2_OBJS)
	$(CC) -o $(BIN_DIR)$(IMPLOSEFST2)$(EXTENSION) $(IMPLOSEFST2_OBJS) $(OPTIONS)

$(BIN_DIR)$(INFLECT)$(EXTENSION): $(INFLECT_OBJS)
	$(CC) -o $(BIN_DIR)$(INFLECT)$(EXTENSION) $(INFLECT_OBJS) $(OPTIONS)

$(BIN_DIR)$(LOCATE)$(EXTENSION): $(LOCATE_OBJS)
	$(CC) -o $(BIN_DIR)$(LOCATE)$(EXTENSION) $(LOCATE_OBJS) $(OPTIONS)

$(BIN_DIR)$(MERGETEXTAUTOMATON)$(EXTENSION): $(MERGETEXTAUTOMATON_OBJS)
	$(CC) -o $(BIN_DIR)$(MERGETEXTAUTOMATON)$(EXTENSION) $(MERGETEXTAUTOMATON_OBJS) $(OPTIONS)

$(BIN_DIR)$(NORMALIZE)$(EXTENSION): $(NORMALIZE_OBJS)
	$(CC) -o $(BIN_DIR)$(NORMALIZE)$(EXTENSION) $(NORMALIZE_OBJS) $(OPTIONS)

$(BIN_DIR)$(POLYLEX)$(EXTENSION): $(POLYLEX_OBJS)
	$(CC) -o $(BIN_DIR)$(POLYLEX)$(EXTENSION) $(POLYLEX_OBJS) $(OPTIONS)

$(BIN_DIR)$(RECONSTRUCAO)$(EXTENSION): $(RECONSTRUCAO_OBJS)
	$(CC) -o $(BIN_DIR)$(RECONSTRUCAO)$(EXTENSION) $(RECONSTRUCAO_OBJS) $(OPTIONS)

$(BIN_DIR)$(REG2GRF)$(EXTENSION): $(REG2GRF_OBJS)
	$(CC) -o $(BIN_DIR)$(REG2GRF)$(EXTENSION) $(REG2GRF_OBJS) $(OPTIONS)

$(BIN_DIR)$(SORTTXT)$(EXTENSION): $(SORTTXT_OBJS)
	$(CC) -o $(BIN_DIR)$(SORTTXT)$(EXTENSION) $(SORTTXT_OBJS) $(OPTIONS)

$(BIN_DIR)$(TABLE2GRF)$(EXTENSION): $(TABLE2GRF_OBJS)
	$(CC) -o $(BIN_DIR)$(TABLE2GRF)$(EXTENSION) $(TABLE2GRF_OBJS) $(OPTIONS)

$(BIN_DIR)$(TEXTAUTOMATON2MFT)$(EXTENSION): $(TEXTAUTOMATON2MFT_OBJS)
	$(CC) -o $(BIN_DIR)$(TEXTAUTOMATON2MFT)$(EXTENSION) $(TEXTAUTOMATON2MFT_OBJS) $(OPTIONS)

$(BIN_DIR)$(TOKENIZE)$(EXTENSION): $(TOKENIZE_OBJS)
	$(CC) -o $(BIN_DIR)$(TOKENIZE)$(EXTENSION) $(TOKENIZE_OBJS) $(OPTIONS)

$(BIN_DIR)$(TXT2FST2)$(EXTENSION): $(TXT2FST2_OBJS)
	$(CC) -o $(BIN_DIR)$(TXT2FST2)$(EXTENSION) $(TXT2FST2_OBJS) $(OPTIONS)

$(BIN_DIR)$(TAGSETNORM)$(EXTENSION): $(TAGSETNORM_OBJS)
	$(CC) -o $(BIN_DIR)$(TAGSETNORM)$(EXTENSION) $(TAGSETNORM_OBJS) $(OPTIONS)

$(BIN_DIR)$(INFLECTKR)$(EXTENSION): $(INFLECTKR_OBJS)
	$(CC) -o $(BIN_DIR)$(INFLECTKR)$(EXTENSION) $(INFLECTKR_OBJS) $(OPTIONS)

$(BIN_DIR)$(SUFFORM2RAC)$(EXTENSION): $(SUFFORM2RAC_OBJS)
	$(CC) -o $(BIN_DIR)$(SUFFORM2RAC)$(EXTENSION) $(SUFFORM2RAC_OBJS) $(OPTIONS)

$(BIN_DIR)$(COMPRESSKR)$(EXTENSION): $(COMPRESSKR_OBJS)
	$(CC) -o $(BIN_DIR)$(COMPRESSKR)$(EXTENSION) $(COMPRESSKR_OBJS) $(OPTIONS)

$(BIN_DIR)$(MERGEBIN)$(EXTENSION): $(MERGEBIN_OBJS)
	$(CC) -o $(BIN_DIR)$(MERGEBIN)$(EXTENSION) $(MERGEBIN_OBJS) $(OPTIONS)

$(BIN_DIR)$(CONSULTDIC)$(EXTENSION): $(CONSULTDIC_OBJS)
	$(CC) -o $(BIN_DIR)$(CONSULTDIC)$(EXTENSION) $(CONSULTDIC_OBJS) $(OPTIONS)

$(BIN_DIR)$(SORTMORPH)$(EXTENSION): $(SORTMORPH_OBJS)
	$(CC) -o $(BIN_DIR)$(SORTMORPH)$(EXTENSION) $(SORTMORPH_OBJS) $(OPTIONS)

$(BIN_DIR)$(TXT2FST2KR)$(EXTENSION): $(TXT2FST2KR_OBJS)
	$(CC) -o $(BIN_DIR)$(TXT2FST2KR)$(EXTENSION) $(TXT2FST2KR_OBJS) $(OPTIONS)

$(BIN_DIR)$(SYL2JAMO)$(EXTENSION): $(SYL2JAMO_OBJS)
	$(CC) -o $(BIN_DIR)$(SYL2JAMO)$(EXTENSION) $(SYL2JAMO_OBJS) $(OPTIONS)

$(BIN_DIR)$(JAMO2SYL)$(EXTENSION): $(JAMO2SYL_OBJS)
	$(CC) -o $(BIN_DIR)$(JAMO2SYL)$(EXTENSION) $(JAMO2SYL_OBJS) $(OPTIONS)

$(BIN_DIR)$(EXTRACTCHAR)$(EXTENSION): $(EXTRACTCHAR_OBJS)
	$(CC) -o $(BIN_DIR)$(EXTRACTCHAR)$(EXTENSION) $(EXTRACTCHAR_OBJS) $(OPTIONS)

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

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
#
# The special command
#
#    make SYSTEM=mingw32
#
# can be used to generate Windows executables under Linux, using the mingw32 tools.
#
# By installing OSXCROSS from https://github.com/tpoechtrager/osxcross
#  An Intel MacOSX binary can be created on Linux system
#   make SYSTEM=osxcross
#
#   or make SYSTEM=osxcross 64BITS=no
#
#  the <OSXCROSS>/target/bin directory must be added in PATH
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
#    make TRE_DIRECT_LINK=yes
# compile the TRE source file directly from this makefile. This can be useful to compile
# Unitex library on Linux 64 bits with more accurate compile option control
#    make TRE_DIRECT_COMPILE=yes
# compile the TRE source file directly from this makefile, without creating library.
# This can be useful to compile Unitex library more accurate compile option control
# and less conflict with visibility options
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
#    make JNILIBRARY=yes ADDITIONAL_INCLUDE=/usr/lib/jvm/java-6-sun-1.6.0.26/include/linux ADDITIONAL_INCLUDE2=/usr/lib/jvm/java-6-sun-1.6.0.26/include
# create the Jni and Library lib libUnitexLibAndJni.so under Linux
#
#    mingw32-make.exe SYSTEM=windows JNILIBRARY=yes ADDITIONAL_INCLUDE=c:\Progra~1\Java\jdk1.6.0_26\include" ADDITIONAL_INCLUDE2=c:\Progra~1\Java\jdk1.6.0_26\include\win32"
# create the Jni and Library lib libUnitexLibAndJni.so under Windows with MinGW32 (fix the path, of course)
#
#  If you want to have access to the SVN revision number from the JNI or LIBRARY, you may
#  provide this number with the extra option SVN_REVISION=xxx or
#  by overwriting the file Unitex_revision.h with "#define UNITEX_REVISION xxx" inside
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
# If you want to generate a static library, use:
# make STATICLIB=yes
# => creates libunitex.a
#
# With Linux on x86-64 processor, you can have an error message asking -fPIC, you can try:
#   make 64BITS=yes LIBRARY=yes TRE_DIRECT_COMPILE=yes
# or
#   make 64BITS=yes JNILIBRARY=yes TRE_DIRECT_COMPILE=yes ADDITIONAL_INCLUDE=/usr/lib/jvm/java-7-openjdk-amd64/include/
#
# you can add parameters to ./configure when build TRE with
# TRE_CONFIGURE_OPTION (by example for cross compiling) 



################################
##  COMPILATION INSTRUCTIONS  ##
################################
################################


# default value for SYSTEM is linux-like
SYSTEM = linux-like

ifeq ($(SYSTEM),linux)
LINUX_LIKE = yes
#LDFLAGS += -lstdc++
endif

ifeq ($(SYSTEM),linux-like)
LINUX_LIKE = yes
endif

ifeq ($(SYSTEM),osxcross)
TARGET_OSX = yes
LINUX_LIKE = yes
endif



# SYSTEM = linux-like
# LINUX_LIKE = yes


ifeq ($(SYSTEM),linux-like)
UNAME = $(shell uname -s)
ifeq ($(UNAME),Darwin)
TARGET_OSX = yes
LINUX_LIKE = yes
endif
endif

# 64 bits is default for OSX
ifeq ($(TARGET_OSX),yes)
ifneq ($(64BITS),yes)
ifneq ($(64BITS),no)
64BITS = yes
endif
endif
endif


ifneq ($(TARGET_OSX),yes)
ifneq ($(64BITS),yes)
ifneq ($(64BITS),no)
64BITS = no
endif
endif
endif


ifeq ($(SYSTEM),osxcross)
TRE_DIRECT_COMPILE = yes
LDFLAGS += -lstdc++


COMPILER_DEFINED = yes


ifeq ($(64BITS),yes)
ifeq ($(COMMANDPREFIXDEFINED),yes)
CC     = $(COMMANDPREFIX)o64-clang++
CXX    = $(COMMANDPREFIX)o64-clang++
CC_GCC = $(COMMANDPREFIX)o64-clang
else
CC     = o64-clang++
CXX    = o64-clang++
CC_GCC = o64-clang
endif

TRE_CONFIGURE_OPTION='--host=x86_64-apple-darwinXX'

else

ifeq ($(COMMANDPREFIXDEFINED),yes)
CC     = $(COMMANDPREFIX)o32-clang++
CXX    = $(COMMANDPREFIX)o32-clang++
CC_GCC = $(COMMANDPREFIX)o32-clang
else
CC     = o32-clang++
CXX    = o32-clang++
CC_GCC = o32-clang
endif

TRE_CONFIGURE_OPTION='--host=i386-apple-darwinXX'

endif
endif






OBJ_DIR = build/
BIN_DIR = ../bin/


ifeq ($(LINUX_LIKE),yes)
###################
###################
##  BEGIN LINUX  ##
###################
###################

ifneq ($(COMPILER_DEFINED),yes)
CC      = g++
CC_GCC  = gcc
endif

AR      = ar
#CC     = gcc -x c
RM      = rm -f
RMR     = rm -rf
INSTALL = cp
NAME    = unitex-src
VERSION = $(shell date +"%Y-%m-%d")
BUILD   = $(NAME)-$(VERSION)


TAR_COMMAND = tar
TAR_PARAM = xzf

INSTALL_COMMAND = make install
TRE_SRC = tre-0.8.0
TRE_TGZ = ../vendor/tre-0.8.0.tar.gz
TRE_TEMP = $(TRE_SRC)/TEMP
YAML_SRC = yaml-0.1.6
YAML_TGZ = ../vendor/yaml-0.1.6.tar.gz
YAML_TEMP = $(YAML_SRC)/TEMP

ifeq ($(TRE_DIRECT_COMPILE), yes)
TRE_LINK_LIB =
else
TRE_LINK_LIB = -ltre
endif



INSTALL_COMMAND = make install

SYSLIBSYNCTOOL = SyncToolPosix.o
SYSLIBLOGGER = SyncLoggerPosix.o
SYSLIBMAPPED = MappedFileHelperPosix.o DirHelperPosix.o
SYSLIBDIRIO = UnitexLibDirPosix.o

#APPDIR = where to install binaries
APPDIR = ../../../App

#
# USING TRE LIBRARY
#
DEFFLAGS += -D_NOT_UNDER_WINDOWS
#                                 -DDEBUG_STAT
#                                 -DBDB



ifneq ($(strip $(YAML_GCC_CFLAGS)),)
YAML_GCC_OPTIONS = -DHAVE_CONFIG_H=1 $(YAML_GCC_CFLAGS)
else
YAML_GCC_OPTIONS = -DHAVE_CONFIG_H=1 $(CFLAGS)
endif


CFLAGS   =  -Wall -D_POSIX_SOURCE #-pedantic
ifeq ($(DEBUG), yes)
CFLAGS += -g3 -O0 -DDEBUG=1 -fuse-ld=gold -Wl,--disable-new-dtags
UNITEX_RELEASE_MEMORY_AT_EXIT=yes
else
CFLAGS += -O3
endif

ifeq ($(PREVENT_LINK_YAML),yes)
else
DEFFLAGS += -I./$(YAML_SRC)/include
endif

ifneq ($(strip $(ADDITIONAL_INCLUDE)),)
DEFFLAGS += -I$(ADDITIONAL_INCLUDE)
endif

ifneq ($(strip $(ADDITIONAL_INCLUDE2)),)
DEFFLAGS += -I$(ADDITIONAL_INCLUDE2)
endif

ifneq ($(strip $(ADDITIONAL_CFLAG)),)
DEFFLAGS += $(ADDITIONAL_CFLAG)
endif

ifneq ($(strip $(ADDITIONAL_CFLAG2)),)
CFLAGS += $(ADDITIONAL_CFLAG2)
endif

ifneq ($(strip $(ADDITIONAL_DEFFLAGS)),)
DEFFLAGS += $(ADDITIONAL_DEFFLAGS)
endif


ifeq ($(UNITEXTOOLLOGGERONLY),yes)
DEFFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(LIBRARY),yes)
CFLAGS += -fPIC
DEFFLAGS += -DUNITEX_LIBRARY
DEFFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(JNILIBRARY),yes)
CFLAGS += -fPIC
DEFFLAGS += -DUNITEX_LIBRARY
DEFFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(STATICLIB),yes)
CFLAGS += -fPIC
DEFFLAGS += -DUNITEX_LIBRARY
DEFFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifneq ($(SVN_REVISION),)
DEFFLAGS += -DSVN_REVISION=$(SVN_REVISION)
endif

ifeq ($(UNITEX_RELEASE_MEMORY_AT_EXIT),yes)
DEFFLAGS += -DUNITEX_RELEASE_MEMORY_AT_EXIT
endif

ifeq ($(UNITEX_EXPERIMENTAL_MSGLOGGER),yes)
DEFFLAGS += -DUNITEX_EXPERIMENTAL_MSGLOGGER
endif

override CFLAGS   += $(DEFFLAGS) -I"../include_tre" -Ilibtre/include -I".." -Llibtre/lib `pkg-config --libs-only-L luajit` `pkg-config --cflags luajit` -I"../"
LIBTRE   = .libtre-done
CLEAN-LIBTRE   = clean-libtre
LIBS   = -Llibtre/lib $(TRE_LINK_LIB) $(ADDITIONAL_LIB1) $(ADDITIONAL_LIB2) -lpthread -lluajit-5.1 #-lunitex #-lboost_thread -lboost_system


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
RMR     = rm -rf
INSTALL = cp
TAR_COMMAND = tar
TAR_PARAM = xzf

NAME    = unitex-src
VERSION = $(shell date +"%Y-%m-%d")
BUILD   = $(NAME)-$(VERSION)

TRE_OPTIONS = -DHAVE_CONFIG_H=1 -I../include_tre -O3 -Wall -c -fmessage-length=0
YAML_OPTIONS = -DHAVE_CONFIG_H=1 -I./$(YAML_SRC)/include -I./$(YAML_SRC)/src -I./$(YAML_SRC)/win32 -DYAML_DECLARE_STATIC -O3 -Wall -c -fmessage-length=0

ifneq ($(strip $(ADDITIONAL_TRE_OPTIONS)),)
	TRE_OPTIONS += $(ADDITIONAL_TRE_OPTIONS)
endif

ifneq ($(strip $(ADDITIONAL_YAML_OPTIONS)),)
	YAML_OPTIONS += $(ADDITIONAL_YAML_OPTIONS)
endif

YAML_OPTIONS += $(YAML_GCC_OPTIONS)

#
# Compiling instructions for the TRE library
#
INSTALL_COMMAND = make install

LIBS =  -L "." $(TRE_LINK_LIB) $(ADDITIONAL_LIB1) $(ADDITIONAL_LIB2)
INCS =  -I".." -I"../include_tre"
CXXINCS =  -I".."
CXXFLAGS = $(CXXINCS)  -Wall -D_JNI_IMPLEMENTATION_
CFLAGS = $(INCS)  -Wall -D_JNI_IMPLEMENTATION_

ifeq ($(DEBUG), yes)
CFLAGS += -g3 -O0 -DDEBUG=1 -Wl,--add-stdcall-alias
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

ifneq ($(strip $(ADDITIONAL_DEFFLAGS)),)
DEFFLAGS += $(ADDITIONAL_DEFFLAGS)
endif



ifeq ($(TRE_DIRECT_COMPILE), yes)
LIBTRE   = .libtre-done
else
LIBTRE   = libtre.a
endif

# -fPIC is not required because all code is position independent under Windows
#ifeq ($(LIBRARY),yes)
#CFLAGS += -fPIC
#endif
#ifeq ($(JNILIBRARY),yes)
#CFLAGS += -fPIC
#endif


ifeq ($(UNITEXTOOLLOGGERONLY),yes)
CFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(LIBRARY),yes)
CFLAGS += -DUNITEX_LIBRARY
CFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(JNILIBRARY),yes)
CFLAGS += -DUNITEX_LIBRARY
CFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(STATICLIB),yes)
CFLAGS += -DUNITEX_LIBRARY
CFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(UNITEX_MINIMAL_WIN_SYSTEM),XP)
DEFFLAGS += -D_WIN32_WINNT=0x0501
endif

ifeq ($(UNITEX_MINIMAL_WIN_SYSTEM),VISTA)
DEFFLAGS += -D_WIN32_WINNT=0x0600
endif
ifneq ($(SVN_REVISION),)
CFLAGS += -DSVN_REVISION=$(SVN_REVISION)
endif

SYSLIBSYNCTOOL = SyncToolWin.o
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

ifeq ($(SYSTEM), cygwin)

#####################
#####################
##  BEGIN CYGWIN   ##
#####################
#####################

ifeq ($(COMMANDPREFIXDEFINED),yes)
	CC     = $(COMMANDPREFIX)g++
	CC_GCC = $(COMMANDPREFIX)gcc
	AR     = $(COMMANDPREFIX)ar
else
	CC      = g++
	CC_GCC  = gcc
	AR      = AR
endif

RM      = rm -f
RMR     = rm -rf
INSTALL = cp
TAR_COMMAND = tar
TAR_PARAM = xzf

NAME    = unitex-src
VERSION = $(shell date +"%Y-%m-%d")
BUILD   = $(NAME)-$(VERSION)

TRE_OPTIONS = -DHAVE_CONFIG_H=1 -I../include_tre -O3 -Wall -c -fmessage-length=0
YAML_OPTIONS = -DHAVE_CONFIG_H=1 -I./$(YAML_SRC)/include -I./$(YAML_SRC)/src -I./$(YAML_SRC)/win32 -DYAML_DECLARE_STATIC -O3 -Wall -c -fmessage-length=0

ifneq ($(strip $(ADDITIONAL_TRE_OPTIONS)),)
	TRE_OPTIONS += $(ADDITIONAL_TRE_OPTIONS)
endif

ifneq ($(strip $(ADDITIONAL_YAML_OPTIONS)),)
	YAML_OPTIONS += $(ADDITIONAL_YAML_OPTIONS)
endif

YAML_OPTIONS += $(YAML_GCC_OPTIONS)

#
# Compiling instructions for the TRE library
#
INSTALL_COMMAND = make install

LIBS =  -L "." $(TRE_LINK_LIB) $(ADDITIONAL_LIB1) $(ADDITIONAL_LIB2)
INCS =  -I".." -I"../include_tre"
CXXINCS =  -I".."
CXXFLAGS = $(CXXINCS)  -Wall -D_JNI_IMPLEMENTATION_
CFLAGS = $(INCS)  -Wall -D_JNI_IMPLEMENTATION_

ifeq ($(DEBUG), yes)
CFLAGS += -g3 -O0 -DDEBUG=1 -Wl,--add-stdcall-alias
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

ifneq ($(strip $(ADDITIONAL_DEFFLAGS)),)
DEFFLAGS += $(ADDITIONAL_DEFFLAGS)
endif

ifeq ($(TRE_DIRECT_COMPILE), yes)
LIBTRE   = .libtre-done
else
LIBTRE   = libtre.a
endif


# -fPIC is not required because all code is position independent under Windows
#ifeq ($(LIBRARY),yes)
#CFLAGS += -fPIC
#endif
#ifeq ($(JNILIBRARY),yes)
#CFLAGS += -fPIC
#endif


ifeq ($(UNITEXTOOLLOGGERONLY),yes)
CFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(LIBRARY),yes)
CFLAGS += -DUNITEX_LIBRARY
CFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(JNILIBRARY),yes)
CFLAGS += -DUNITEX_LIBRARY
CFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(STATICLIB),yes)
CFLAGS += -DUNITEX_LIBRARY
CFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(UNITEX_MINIMAL_WIN_SYSTEM),XP)
DEFFLAGS += -D_WIN32_WINNT=0x0501
endif

ifeq ($(UNITEX_MINIMAL_WIN_SYSTEM),VISTA)
DEFFLAGS += -D_WIN32_WINNT=0x0600
endif
ifneq ($(SVN_REVISION),)
CFLAGS += -DSVN_REVISION=$(SVN_REVISION)
endif

SYSLIBSYNCTOOL = SyncToolPosix.o
SYSLIBLOGGER = SyncLoggerPosix.o
SYSLIBMAPPED = MappedFileHelperPosix.o DirHelperPosix.o
SYSLIBDIRIO = UnitexLibDirPosix.o

APPDIR = ../../../App

OPTIONS = $(CFLAGS) $(LDFLAGS) $(LIBS)
EXTENSION = .exe

#####################
#####################
##  END   CYGWIN   ##
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

SYSLIBSYNCTOOL = SyncToolWin.o
SYSLIBLOGGER = SyncLoggerWin.o
SYSLIBMAPPED = MappedFileHelperWin.o DirHelperWin.o
SYSLIBDIRIO = UnitexLibDirWin.o

APPDIR = ../../../App

ifneq ($(DEVCPP_PATHDEFINED),yes)
DEVCPP_PATH = C:/Dev-Cpp
endif



TRE_OPTIONS = -DHAVE_CONFIG_H=1 -I../include_tre -O3 -Wall -c -fmessage-length=0
YAML_OPTIONS = -DHAVE_CONFIG_H=1 -I./$(YAML_SRC)/include -I./$(YAML_SRC)/src -I./$(YAML_SRC)/win32 -DYAML_DECLARE_STATIC -O3 -Wall -c -fmessage-length=0

ifneq ($(strip $(ADDITIONAL_TRE_OPTIONS)),)
	TRE_OPTIONS += $(ADDITIONAL_TRE_OPTIONS)
endif

ifneq ($(strip $(ADDITIONAL_YAML_OPTIONS)),)
	YAML_OPTIONS += $(ADDITIONAL_YAML_OPTIONS)
endif

YAML_OPTIONS += $(YAML_GCC_OPTIONS)

#
# USING TRE STATIC LIBRARY
#
LIBS =  -L"$(DEVCPP_PATH)/lib" -L"." $(TRE_LINK_LIB) $(ADDITIONAL_LIB1) $(ADDITIONAL_LIB2)
INCS =  -I"$(DEVCPP_PATH)/include" -I"../include_tre" -I".."
CXXINCS =  -I"$(DEVCPP_PATH)/include/c++"  -I"$(DEVCPP_PATH)/include/c++/mingw32"  -I"$(DEVCPP_PATH)/include/c++/backward"  -I"$(DEVCPP_PATH)/include"  -I"../include_tre" -I".."
CXXFLAGS = $(CXXINCS)  -Wall
CFLAGS = $(INCS)  -Wall

ifeq ($(DEBUG), yes)
CFLAGS += -g3 -O0
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

ifneq ($(strip $(ADDITIONAL_DEFFLAGS)),)
DEFFLAGS += $(ADDITIONAL_DEFFLAGS)
endif

ifeq ($(TRE_DIRECT_COMPILE), yes)
LIBTRE   = .libtre-done
else
LIBTRE   = libtre.a
endif


# -fPIC is not required because all code is position independent under Windows
#ifeq ($(LIBRARY),yes)
#CFLAGS += -fPIC
#endif
#ifeq ($(JNILIBRARY),yes)
#CFLAGS += -fPIC
#endif

ifeq ($(UNITEXTOOLLOGGERONLY),yes)
CFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(LIBRARY),yes)
CFLAGS += -DUNITEX_LIBRARY
CFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(JNILIBRARY),yes)
CFLAGS += -DUNITEX_LIBRARY
CFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(STATICLIB),yes)
CFLAGS += -DUNITEX_LIBRARY
CFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
endif

ifeq ($(UNITEX_MINIMAL_WIN_SYSTEM),XP)
DEFFLAGS += -D_WIN32_WINNT=0x0501
endif

ifeq ($(UNITEX_MINIMAL_WIN_SYSTEM),VISTA)
DEFFLAGS += -D_WIN32_WINNT=0x0600
endif
ifneq ($(SVN_REVISION),)
CFLAGS += -DSVN_REVISION=$(SVN_REVISION)
endif

OPTIONS = $(LIBS) $(CFLAGS) $(LDFLAGS)
EXTENSION = .exe

###################
###################
##  END WINDOWS  ##
###################
###################
endif

endif ## CYGWIN (ifeq)

endif ## MINGW (ifeq)

# Cassys set of object files.
CASSYS_FILE_OBJS = Cassys.o Cassys_tokens.o Cassys_xml_output.o Cassys_external_program.o Cassys_transducer.o Cassys_io.o \
	                Cassys_lexical_tags.o Cassys_concord.o


##################################
##################################
##  OBJECTS NEEDED BY PROGRAMS  ##
##################################
##################################

ifeq ($(TRE_DIRECT_COMPILE), yes)
TRE_LINK_OBJS = regcomp.o regerror.o regexec.o tre-ast.o tre-compile.o tre-match-approx.o tre-match-backtrack.o tre-match-parallel.o \
                tre-mem.o tre-parse.o tre-stack.o xmalloc.o

else
TRE_LINK_OBJS =
endif

ifeq ($(PREVENT_LINK_YAML),yes)
YAML_LINK_OBJS = 
else
YAML_LINK_OBJS = api.o writer.o loader.o reader.o parser.o scanner.o emitter.o dumper.o
endif

BUILDKRMWUDIC      = BuildKrMwuDic
BUILDKRMWUDIC_OBJS = Main_BuildKrMwuDic.o BuildKrMwuDic.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
                     common.o ELG.o table.o utf8.o case.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o UnitexRevisionInfo.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Error.o UnitexGetOpt.o KrMwuDic.o StringParsing.o DELA.o String_hash.o \
                     Alphabet.o List_ustring.o Korean.o AbstractFst2Load.o PackFst2.o Fst2.o \
                     Pattern.o List_int.o Transitions.o Symbol.o LanguageDefinition.o Tagset.o \
                     Ustring.o Grf2Fst2.o Grf2Fst2_lib.o SingleGraph.o File.o \
                     BitArray.o HashTable.o FIFO.o BitMasks.o Fst2Check_lib.o ProgramInvoker.o \
                     MF_SU_morpho.o MF_LangMorpho.o MF_Util.o MF_FormMorpho.o MF_Global.o \
                     MF_Operators_Util.o MF_DicoMorpho.o Arabic.o AbstractDelaLoad.o PackInf.o List_pointer.o \
                     CompressedDic.o LoadInf.o DebugMode.o Grf_lib.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                     $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

CASSYS      = Cassys
CASSYS_OBJS = Main_Cassys.o $(CASSYS_FILE_OBJS) IOBuffer.o Copyright.o Error.o UnitexGetOpt.o common.o ELG.o table.o utf8.o case.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o UnitexRevisionInfo.o safe_bool.o chrono.o severity_level.o MsgLogger.o Af_stdio.o ActivityLogger.o AbstractAllocator.o ProgramInvoker.o \
            FIFO.o Concord.o Locate.o Concordance.o Snt.o Text_tokens.o File.o Buffer.o LocateMatches.o LocatePattern.o LocateTrace.o Thai.o NewLineShifts.o \
            String_hash.o SortTxt.o StringParsing.o DELA.o List_int.o Alphabet.o BitMasks.o Tokenization.o LemmaTree.o PatternTree.o \
            BitArray.o List_pointer.o CompoundWordTree.o AbstractDelaLoad.o PackInf.o Korean.o UserCancelling.o Stack_unichar.o AbstractFst2Load.o PackFst2.o \
            Fst2.o MorphologicalFilters.o LocateFst2Tags.o TransductionVariables.o OptimizedFst2.o Text_parsing.o List_ustring.o ParsingInfo.o \
            DicVariables.o TransductionStack.o MorphologicalLocate.o Pattern.o Contexts.o Transitions.o Symbol.o Ustring.o LanguageDefinition.o \
            Tagset.o Tokenize.o HashTable.o OutputTransductionVariables.o LocateCache.o \
            Grf2Fst2.o Grf2Fst2_lib.o SingleGraph.o Grf_lib.o Fst2Check_lib.o \
            Arabic.o Match.o VariableUtils.o Offsets.o Overlap.o CompressedDic.o LoadInf.o RegExFacade.o $(TRE_LINK_OBJS) \
            DebugMode.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o PRLG.o $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)


CHECKDIC      = CheckDic
CHECKDIC_OBJS = Main_CheckDic.o CheckDic.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o case.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o UnitexRevisionInfo.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o File.o DELA.o Alphabet.o String_hash.o \
                List_ustring.o Error.o StringParsing.o UnitexGetOpt.o AbstractDelaLoad.o PackInf.o CompressedDic.o \
                LoadInf.o Ustring.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

COMPRESS      = Compress
COMPRESS_OBJS = Main_Compress.o Compress.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o case.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o UnitexRevisionInfo.o safe_bool.o chrono.o severity_level.o MsgLogger.o \
                AbstractAllocator.o DELA.o DictionaryTree.o Alphabet.o \
                String_hash.o AutomatonDictionary2Bin.o File.o List_int.o \
                List_ustring.o Error.o StringParsing.o UnitexGetOpt.o ProgramInvoker.o \
                BitArray.o AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o Ustring.o \
                VirtualFiles.o Persistence.o UnitexRevisionInfo.o $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

CONCORD      = Concord
CONCORD_OBJS = Main_Concord.o Concord.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o case.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o UnitexRevisionInfo.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Text_tokens.o String_hash.o \
                List_int.o Alphabet.o LocateMatches.o Match.o Concordance.o File.o \
                Text_parsing.o UserCancelling.o MorphologicalLocate.o LocatePattern.o LocateTrace.o \
                TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o \
                CompoundWordTree.o MorphologicalFilters.o DELA.o List_ustring.o Fst2.o \
                AbstractDelaLoad.o PackInf.o AbstractFst2Load.o PackFst2.o Pattern.o LocateFst2Tags.o Error.o SortTxt.o \
                Buffer.o StringParsing.o Snt.o BitArray.o LemmaTree.o \
                List_pointer.o PatternTree.o BitMasks.o Tokenization.o Thai.o Contexts.o \
                Stack_unichar.o Transitions.o DicVariables.o \
                Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o \
                ProgramInvoker.o NewLineShifts.o Korean.o HashTable.o LocateCache.o \
                Arabic.o OutputTransductionVariables.o VariableUtils.o CompressedDic.o \
                LoadInf.o Offsets.o Overlap.o RegExFacade.o DebugMode.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o PRLG.o \
                $(TRE_LINK_OBJS) $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

CONCORDIFF      = ConcorDiff
CONCORDIFF_OBJS = Main_ConcorDiff.o ConcorDiff.o IOBuffer.o Copyright.o Diff.o Concord.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o case.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o UnitexRevisionInfo.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Text_tokens.o String_hash.o List_int.o \
                Alphabet.o LocateMatches.o Match.o Concordance.o File.o \
                Text_parsing.o UserCancelling.o MorphologicalLocate.o LocatePattern.o LocateTrace.o TransductionStack.o OptimizedFst2.o TransductionVariables.o ParsingInfo.o \
                CompoundWordTree.o MorphologicalFilters.o \
                DELA.o List_ustring.o Fst2.o AbstractDelaLoad.o PackInf.o AbstractFst2Load.o PackFst2.o Pattern.o LocateFst2Tags.o \
                Error.o SortTxt.o \
                Buffer.o StringParsing.o Snt.o BitArray.o LemmaTree.o List_pointer.o BitMasks.o \
                PatternTree.o Tokenization.o Thai.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o \
                Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o \
                ProgramInvoker.o NewLineShifts.o Korean.o HashTable.o LocateCache.o \
                Arabic.o OutputTransductionVariables.o VariableUtils.o CompressedDic.o \
                LoadInf.o Offsets.o Overlap.o RegExFacade.o DebugMode.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o PRLG.o \
                $(TRE_LINK_OBJS) $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

CONVERT      = Convert
CONVERT_OBJS = Main_Convert.o Convert.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o File.o CodePages.o Error.o \
                HTMLCharacters.o AsciiSearchTree.o UnitexGetOpt.o Arabic.o String_hash.o StringParsing.o \
                Ustring.o $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

DICO      = Dico
DICO_OBJS = Main_Dico.o Dico.o List_int.o DELA.o ApplyDic.o File.o Alphabet.o String_hash.o Text_tokens.o \
            List_ustring.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o CompoundWordHashTable.o Fst2.o AbstractDelaLoad.o AbstractFst2Load.o \
            IOBuffer.o Copyright.o Error.o CompoundWordTree.o \
            Pattern.o ParsingInfo.o LocatePattern.o LocateTrace.o LocateMatches.o Match.o OptimizedFst2.o \
            Text_parsing.o UserCancelling.o MorphologicalLocate.o TransductionVariables.o TransductionStack.o MorphologicalFilters.o Locate.o \
            StringParsing.o Buffer.o BitArray.o Snt.o LemmaTree.o List_pointer.o PatternTree.o \
            LocateFst2Tags.o BitMasks.o Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o \
            Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o ProgramInvoker.o \
            Korean.o SortTxt.o Thai.o Compress.o DictionaryTree.o \
            AutomatonDictionary2Bin.o HashTable.o LocateCache.o Arabic.o \
            OutputTransductionVariables.o VariableUtils.o CompressedDic.o \
            DebugMode.o LoadInf.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o BuildTextAutomaton.o Tfst.o SingleGraph.o \
            FIFO.o TfstStats.o RegExFacade.o \
            $(TRE_LINK_OBJS) $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

DUMPOFFSETS      = DumpOffsets
DUMPOFFSETS_OBJS = Main_DumpOffsets.o DumpOffsets.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o \
                AbstractAllocator.o File.o List_ustring.o String_hash.o \
                Error.o StringParsing.o Buffer.o UnitexGetOpt.o Ustring.o \
                Offsets.o Overlap.o VirtualFiles.o \
                $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

ELAG      = Elag
ELAG_OBJS = Main_Elag.o Elag.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o ElagFunctions.o ElagFstFilesIO.o AutDeterminization.o \
            AutMinimization.o AutConcat.o Fst2Automaton.o \
            Symbol.o String_hash.o Tagset.o Ustring.o \
            ElagStateSet.o SymbolAlphabet.o Symbol_op.o Error.o LanguageDefinition.o \
            AutIntersection.o Alphabet.o StringParsing.o DELA.o List_ustring.o \
            List_int.o SingleGraph.o HashTable.o BitArray.o Fst2.o Pattern.o \
            BitMasks.o FIFO.o Transitions.o ElagDebug.o File.o UnitexGetOpt.o Tfst.o \
            TfstStats.o AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
            $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

ELAGCOMP      = ElagComp
ELAGCOMP_OBJS = Main_ElagComp.o ElagComp.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o ElagRulesCompilation.o Fst2Automaton.o \
                ElagFstFilesIO.o String_hash.o \
                Symbol.o Ustring.o AutDeterminization.o AutIntersection.o \
                ElagStateSet.o Symbol_op.o LanguageDefinition.o Tagset.o \
                AutMinimization.o SymbolAlphabet.o AutConcat.o AutComplementation.o \
                Alphabet.o Error.o StringParsing.o DELA.o List_ustring.o List_int.o \
                SingleGraph.o HashTable.o BitArray.o Fst2.o Pattern.o BitMasks.o FIFO.o File.o \
                Transitions.o ElagDebug.o UnitexGetOpt.o Tfst.o TfstStats.o AbstractDelaLoad.o PackInf.o \
                CompressedDic.o LoadInf.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

EVAMB      = Evamb
EVAMB_OBJS = Main_Evamb.o Evamb.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o \
             AbstractAllocator.o String_hash.o \
             Alphabet.o Error.o StringParsing.o Pattern.o List_ustring.o \
             List_int.o BitMasks.o DELA.o List_pointer.o SingleGraph.o \
             BitArray.o HashTable.o FIFO.o Transitions.o \
             Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o \
             Tfst.o TfstStats.o File.o AbstractDelaLoad.o PackInf.o CompressedDic.o \
             LoadInf.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
             $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

EXTRACT      = Extract
EXTRACT_OBJS = Main_Extract.o Extract.o IOBuffer.o Copyright.o LocatePattern.o LocateTrace.o \
               Text_parsing.o UserCancelling.o MorphologicalLocate.o TransductionStack.o \
               OptimizedFst2.o TransductionVariables.o \
               ParsingInfo.o CompoundWordTree.o List_int.o Concordance.o Offsets.o \
               DELA.o List_ustring.o Fst2.o Pattern.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o PackInf.o AbstractDelaLoad.o PackFst2.o AbstractFst2Load.o \
               Text_tokens.o File.o ExtractUnits.o Alphabet.o String_hash.o LocateMatches.o Match.o \
               MorphologicalFilters.o Error.o Buffer.o SortTxt.o StringParsing.o BitArray.o \
               LemmaTree.o Snt.o List_pointer.o PatternTree.o LocateFst2Tags.o BitMasks.o \
               Tokenization.o Thai.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o \
               Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o \
               ProgramInvoker.o NewLineShifts.o Korean.o HashTable.o LocateCache.o \
               Arabic.o OutputTransductionVariables.o VariableUtils.o CompressedDic.o RegExFacade.o $(TRE_LINK_OBJS) \
               DebugMode.o LoadInf.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o PRLG.o \
               $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

FLATTEN      = Flatten
FLATTEN_OBJS = Main_Flatten.o Flatten.o IOBuffer.o Copyright.o Alphabet.o Af_stdio.o ActivityLogger.o \
               common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Fst2.o Grf2Fst2_lib.o \
               AbstractFst2Load.o PackFst2.o FlattenFst2.o List_int.o Error.o File.o SingleGraph.o \
               BitMasks.o BitArray.o HashTable.o FIFO.o Pattern.o List_ustring.o \
               String_hash.o StringParsing.o Transitions.o DELA.o \
               Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o \
               UnitexGetOpt.o AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o \
               DebugMode.o Grf_lib.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
               $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

FST2CHECK      = Fst2Check
FST2CHECK_OBJS = Main_Fst2Check.o Fst2Check.o Fst2Check_lib.o IOBuffer.o Copyright.o Af_stdio.o \
                ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Fst2.o AbstractFst2Load.o PackFst2.o Alphabet.o \
                File.o String_hash.o Error.o TransductionVariables.o ProgramInvoker.o UnitexGetOpt.o \
                StringParsing.o Pattern.o List_ustring.o List_int.o BitMasks.o \
                Transitions.o DELA.o Symbol.o Symbol_op.o LanguageDefinition.o \
                Ustring.o Tagset.o AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o \
                VirtualFiles.o Persistence.o UnitexRevisionInfo.o SingleGraph.o BitArray.o FIFO.o HashTable.o List_pointer.o \
                $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

FST2LIST      = Fst2List
FST2LIST_OBJS = Main_Fst2List.o Fst2List.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
                common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Fst2.o AbstractFst2Load.o PackFst2.o Alphabet.o \
                File.o String_hash.o Error.o TransductionVariables.o \
                StringParsing.o Pattern.o List_ustring.o List_int.o BitMasks.o \
                Transitions.o DELA.o Symbol.o Symbol_op.o LanguageDefinition.o \
                Ustring.o Tagset.o AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o \
                VirtualFiles.o Persistence.o UnitexRevisionInfo.o UnitexGetOpt.o \
                $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

FST2TXT      = Fst2Txt
FST2TXT_OBJS = Main_Fst2Txt.o Fst2Txt.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
               common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Fst2.o SingleGraph.o FIFO.o \
               AbstractFst2Load.o PackFst2.o Alphabet.o ParsingInfo.o \
               TransductionVariables.o String_hash.o Error.o StringParsing.o \
               Pattern.o List_ustring.o List_int.o BitMasks.o BitArray.o \
               Fst2Txt_TokenTree.o Buffer.o Transitions.o File.o \
               Fst2TxtAsRoutine.o DELA.o DicVariables.o TransductionStack.o \
               Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o Stack_unichar.o \
               Korean.o HashTable.o OutputTransductionVariables.o VariableUtils.o \
               Offsets.o Overlap.o AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o List_pointer.o \
               VirtualFiles.o Persistence.o UnitexRevisionInfo.o Fst2Check_lib.o \
               $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

GRF2FST2      = Grf2Fst2
GRF2FST2_OBJS = Main_Grf2Fst2.o Grf2Fst2.o IOBuffer.o Copyright.o Alphabet.o Af_stdio.o ActivityLogger.o \
                common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Fst2.o AbstractFst2Load.o PackFst2.o \
                Fst2Check_lib.o Grf2Fst2_lib.o Error.o File.o SingleGraph.o \
                BitMasks.o BitArray.o List_int.o HashTable.o FIFO.o Pattern.o \
                List_ustring.o String_hash.o StringParsing.o Transitions.o DELA.o \
                Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o \
                ProgramInvoker.o AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o List_pointer.o \
                DebugMode.o Grf_lib.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

GRFDIFF      = GrfDiff
GRFDIFF_OBJS = Main_GrfDiff.o GrfDiff.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o Error.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
            GrfSvn_lib.o Grf_lib.o Ustring.o UnitexGetOpt.o String_hash.o \
            StringParsing.o AbstractAllocator.o \
            $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

GRFDIFF3      = GrfDiff3
GRFDIFF3_OBJS = Main_GrfDiff3.o GrfDiff3.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o Error.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
            GrfSvn_lib.o Grf_lib.o Ustring.o UnitexGetOpt.o String_hash.o \
            StringParsing.o AbstractAllocator.o \
            $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

GRFTEST      = GrfTest
GRFTEST_OBJS = Main_GrfTest.o GrfTest.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o Error.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
            GrfSvn_lib.o Grf_lib.o Ustring.o UnitexGetOpt.o Normalize.o Offsets.o File.o \
            Overlap.o NormalizeAsRoutine.o String_hash.o StringParsing.o AbstractAllocator.o \
            DELA.o Alphabet.o CompressedDic.o LoadInf.o List_ustring.o \
            AbstractDelaLoad.o PackInf.o ProgramInvoker.o Fst2Txt.o Fst2TxtAsRoutine.o \
            Fst2Txt_TokenTree.o Transitions.o Symbol.o LanguageDefinition.o \
            Tagset.o List_int.o TransductionStack.o Stack_unichar.o VariableUtils.o \
            TransductionVariables.o OutputTransductionVariables.o \
            DicVariables.o Korean.o HashTable.o ParsingInfo.o SingleGraph.o \
            BitArray.o FIFO.o BitMasks.o Buffer.o Fst2Automaton.o ElagFstFilesIO.o \
            Tfst.o TfstStats.o Fst2.o Pattern.o AbstractFst2Load.o PackFst2.o \
            Tokenize.o Dico.o Snt.o Text_tokens.o ApplyDic.o Match.o \
            CompoundWordHashTable.o Tokenization.o LocateMatches.o \
            SortTxt.o Thai.o Compress.o DictionaryTree.o \
            AutomatonDictionary2Bin.o LocatePattern.o Locate.o \
            Text_parsing.o DebugMode.o LocateCache.o UserCancelling.o \
            MorphologicalFilters.o Contexts.o MorphologicalLocate.o Arabic.o \
            List_pointer.o LocateTrace.o PatternTree.o LocateFst2Tags.o \
            CompoundWordTree.o LemmaTree.o OptimizedFst2.o GrfTest_lib.o Grf2Fst2.o \
            Grf2Fst2_lib.o Fst2Check_lib.o Concord.o Concordance.o \
            NewLineShifts.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o PRLG.o BuildTextAutomaton.o RegExFacade.o \
            $(TRE_LINK_OBJS) $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

IMPLODETFST      = ImplodeTfst
IMPLODETFST_OBJS = Main_ImplodeTfst.o ImplodeTfst.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
                   common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Tfst.o File.o SingleGraph.o \
                   String_hash.o Alphabet.o Error.o StringParsing.o Pattern.o List_ustring.o \
                   List_int.o BitMasks.o DELA.o List_pointer.o Transitions.o \
                   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o \
                   BitArray.o HashTable.o FIFO.o Match.o TfstStats.o AbstractDelaLoad.o PackInf.o \
                   CompressedDic.o LoadInf.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                   $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

KEYWORDS      = KeyWords
KEYWORDS_OBJS = Main_KeyWords.o KeyWords.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o \
            AbstractAllocator.o Alphabet.o StringParsing.o DELA.o File.o UnitexGetOpt.o \
            VirtualFiles.o Persistence.o UnitexRevisionInfo.o Error.o String_hash.o Ustring.o List_ustring.o \
            CompressedDic.o AbstractDelaLoad.o PackInf.o LoadInf.o KeyWords_lib.o Tokenization.o \
            $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

LOCATE      = Locate
LOCATE_OBJS = Main_Locate.o Locate.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o \
              AbstractAllocator.o Alphabet.o DELA.o List_ustring.o String_hash.o \
              LocatePattern.o LocateTrace.o AbstractDelaLoad.o PackInf.o AbstractFst2Load.o PackFst2.o \
              Fst2.o Text_tokens.o List_int.o \
              Pattern.o CompoundWordTree.o LocateFst2Tags.o \
              OptimizedFst2.o Text_parsing.o UserCancelling.o MorphologicalLocate.o LocateMatches.o \
              Match.o TransductionVariables.o TransductionStack.o \
              ParsingInfo.o File.o MorphologicalFilters.o Error.o StringParsing.o \
              BitArray.o LemmaTree.o List_pointer.o PatternTree.o BitMasks.o Buffer.o \
              Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o \
              Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o \
              ProgramInvoker.o Korean.o HashTable.o LocateCache.o Arabic.o \
              OutputTransductionVariables.o VariableUtils.o CompressedDic.o RegExFacade.o $(TRE_LINK_OBJS) \
              LoadInf.o DebugMode.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
              $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

LOCATETFST      = LocateTfst
LOCATETFST_OBJS = Main_LocateTfst.o LocateTfst.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
                  common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Error.o UnitexGetOpt.o LocateTfst_lib.o \
                  Tfst.o File.o Ustring.o SingleGraph.o List_int.o BitMasks.o FIFO.o Transitions.o \
                  Symbol.o LanguageDefinition.o Tagset.o String_hash.o StringParsing.o DELA.o \
                  Alphabet.o List_ustring.o HashTable.o BitArray.o Fst2.o AbstractFst2Load.o PackFst2.o Pattern.o \
                  LocateTfstMatches.o Match.o MorphologicalFilters.o Korean.o \
                  OptimizedTfstTagMatching.o List_pointer.o \
                  TransductionVariables.o TransductionStackTfst.o Contexts.o \
                  OutputTransductionVariables.o DicVariables.o TfstStats.o AbstractDelaLoad.o PackInf.o \
                  CompressedDic.o LoadInf.o DebugMode.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o RegExFacade.o \
                  $(TRE_LINK_OBJS) $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)


MULTIFLEX      = MultiFlex
MULTIFLEX_OBJS = Main_MultiFlex.o MultiFlex.o Alphabet.o MF_DLC_inflect.o MF_DicoMorpho.o \
                 MF_FormMorpho.o MF_LangMorpho.o List_pointer.o \
                 MF_MU_graph.o MF_MU_morpho.o MF_SU_morpho.o MF_Unif.o MF_Util.o \
                 Fst2.o AbstractFst2Load.o PackFst2.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Error.o IOBuffer.o Copyright.o File.o DELA.o StringParsing.o List_ustring.o \
                 String_hash.o Pattern.o List_int.o BitMasks.o Transitions.o Grf2Fst2.o \
                 Grf2Fst2_lib.o SingleGraph.o BitArray.o FIFO.o HashTable.o Fst2Check_lib.o \
                 Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o \
                 MF_Operators_Util.o UnitexGetOpt.o ProgramInvoker.o Korean.o \
                 Arabic.o AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o MF_Global.o \
                 DebugMode.o Grf_lib.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                 $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

MZREPAIRULP      = MzRepairUlp
MZREPAIRULP_OBJS = Main_MzRepairUlp.o MzToolsUlp.o MzRepairUlp.o UnitexGetOpt.o \
                   Error.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o Af_stdio.o ActivityLogger.o IOBuffer.o Copyright.o \
                   $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBDIRIO)

NORMALIZE      = Normalize
NORMALIZE_OBJS = Main_Normalize.o Normalize.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o \
                AbstractAllocator.o File.o DELA.o List_ustring.o String_hash.o \
                Alphabet.o Error.o StringParsing.o Buffer.o NormalizeAsRoutine.o \
                UnitexGetOpt.o AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o Ustring.o \
                Offsets.o Overlap.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

POLYLEX      = PolyLex
POLYLEX_OBJS = Main_PolyLex.o PolyLex.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
                common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Alphabet.o DELA.o AbstractDelaLoad.o PackInf.o \
                List_ustring.o String_hash.o \
                NorwegianCompounds.o GermanCompounds.o File.o \
                RussianCompounds.o GeneralDerivation.o Error.o StringParsing.o \
                DutchCompounds.o UnitexGetOpt.o CompressedDic.o LoadInf.o Ustring.o \
                VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

REBUILDTFST      = RebuildTfst
REBUILDTFST_OBJS = Main_RebuildTfst.o RebuildTfst.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
                   common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o String_hash.o AbstractFst2Load.o PackFst2.o Fst2.o \
                   File.o Alphabet.o Error.o StringParsing.o \
                   Pattern.o List_ustring.o List_int.o BitMasks.o BitArray.o \
                   Transitions.o DELA.o List_pointer.o \
                   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o \
                   UnitexGetOpt.o Tfst.o TfstStats.o Grf2Fst2.o Grf2Fst2_lib.o SingleGraph.o \
                   HashTable.o FIFO.o Fst2Check_lib.o ProgramInvoker.o AbstractDelaLoad.o PackInf.o \
                   CompressedDic.o LoadInf.o DebugMode.o Grf_lib.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                   $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

RECONSTRUCAO      = Reconstrucao
RECONSTRUCAO_OBJS = Main_Reconstrucao.o Reconstrucao.o OptimizedFst2.o ParsingInfo.o CompoundWordTree.o \
               DELA.o AbstractDelaLoad.o PackInf.o List_ustring.o Fst2.o AbstractFst2Load.o PackFst2.o \
               IOBuffer.o Copyright.o TransductionStack.o Pattern.o \
               Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Alphabet.o \
               LocateMatches.o Match.o File.o NormalizationFst2.o \
               PortugueseNormalization.o TransductionVariables.o \
               Text_parsing.o UserCancelling.o MorphologicalLocate.o String_hash.o \
               LocatePattern.o LocateTrace.o Text_tokens.o List_int.o \
               Sentence2Grf.o MorphologicalFilters.o Error.o StringParsing.o BitArray.o \
               LemmaTree.o List_pointer.o PatternTree.o LocateFst2Tags.o BitMasks.o \
               Buffer.o Tokenization.o Contexts.o Stack_unichar.o Transitions.o DicVariables.o \
               Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o \
               Tfst.o TfstStats.o SingleGraph.o FIFO.o HashTable.o Korean.o LocateCache.o \
               Arabic.o OutputTransductionVariables.o VariableUtils.o CompressedDic.o \
               DebugMode.o LoadInf.o Grf_lib.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o RegExFacade.o \
               $(TRE_LINK_OBJS) $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

REG2GRF      = Reg2Grf
REG2GRF_OBJS = Main_Reg2Grf.o Reg2Grf.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
               common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o RegularExpressions.o Error.o File.o \
               List_int.o Stack_int.o UnitexGetOpt.o Ustring.o \
               $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

SEQ2GRF      = Seq2Grf
SEQ2GRF_OBJS = Main_Seq2Grf.o Seq2Grf.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o \
               AbstractAllocator.o Error.o \
               List_ustring.o String_hash.o Sentence2Grf.o TfstTag.o Tfst.o \
               BitArray.o FIFO.o Grf2Fst2_lib.o \
               SingleGraph.o BitMasks.o HashTable.o LocateMatches.o Match.o \
               UnitexGetOpt.o Ustring.o Grf_lib.o TfstStats.o \
               AbstractDelaLoad.o PackInf.o DebugMode.o Transitions.o List_int.o \
               Symbol.o LanguageDefinition.o Tagset.o Overlap.o \
               AbstractFst2Load.o PackFst2.o Fst2.o Pattern.o \
               DELA.o StringParsing.o CompressedDic.o LoadInf.o GrfBeauty.o \
               File.o Alphabet.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
               $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

SORTTXT      = SortTxt
SORTTXT_OBJS = Main_SortTxt.o SortTxt.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
               common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Error.o Thai.o UnitexGetOpt.o \
               ProgramInvoker.o DELA.o StringParsing.o Ustring.o String_hash.o Alphabet.o \
               CompressedDic.o AbstractDelaLoad.o PackInf.o LoadInf.o List_ustring.o File.o \
               VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
               $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

SPELLCHECK      = SpellCheck
SPELLCHECK_OBJS = Main_SpellCheck.o SpellCheck.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
               common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Error.o UnitexGetOpt.o \
               ProgramInvoker.o File.o SpellChecking.o CompressedDic.o AbstractDelaLoad.o PackInf.o \
               LoadInf.o List_ustring.o Ustring.o StringParsing.o DELA.o String_hash.o \
               Alphabet.o Keyboard.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
               $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

STATS      = Stats
STATS_OBJS = Main_Stats.o Stats.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o \
               AbstractAllocator.o Error.o Thai.o UnitexGetOpt.o \
               ProgramInvoker.o HashTable.o File.o Text_tokens.o String_hash.o \
               LocateMatches.o Match.o DELA.o \
               Alphabet.o StringParsing.o List_int.o List_ustring.o AbstractDelaLoad.o PackInf.o CompressedDic.o \
               LoadInf.o Ustring.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
               $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

TABLE2GRF      = Table2Grf
TABLE2GRF_OBJS = Main_Table2Grf.o Table2Grf.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
                 common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o File.o Error.o UnitexGetOpt.o \
                 $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBDIRIO)

TAGGER      = Tagger
TAGGER_OBJS = Main_Tagger.o Tagger.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o \
              Tfst.o TfstStats.o Ustring.o SingleGraph.o List_int.o BitMasks.o Error.o UnitexGetOpt.o \
              Transitions.o FIFO.o HashTable.o BitArray.o Symbol.o DELA.o StringParsing.o \
              String_hash.o Alphabet.o List_ustring.o LanguageDefinition.o Tagset.o \
              File.o TaggingProcess.o Match.o ElagFunctions.o AutDeterminization.o \
              AutMinimization.o ElagFstFilesIO.o AutIntersection.o Fst2Automaton.o \
              SymbolAlphabet.o ElagStateSet.o Symbol_op.o AbstractDelaLoad.o PackInf.o CompressedDic.o \
              LoadInf.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
              $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

TAGSETNORMTFST = TagsetNormTfst
TAGSETNORMTFST_OBJS = Main_TagsetNormTfst.o TagsetNormTfst.o IOBuffer.o Copyright.o Af_stdio.o \
                    ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o ElagFstFilesIO.o Ustring.o \
                    String_hash.o Symbol.o LanguageDefinition.o Tagset.o \
                    Error.o File.o StringParsing.o DELA.o List_ustring.o Alphabet.o \
                    List_int.o SingleGraph.o HashTable.o BitArray.o Fst2.o AbstractFst2Load.o PackFst2.o  Pattern.o \
                    BitMasks.o FIFO.o Transitions.o \
                    Symbol_op.o Fst2Automaton.o UnitexGetOpt.o Tfst.o TfstStats.o \
                    Match.o AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                    $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

TEI2TXT      = TEI2Txt
TEI2TXT_OBJS = Main_TEI2Txt.o TEI2Txt.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
               common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Error.o File.o HTMLCharacters.o \
               AsciiSearchTree.o UnitexGetOpt.o $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBDIRIO)

TFST2GRF      = Tfst2Grf
TFST2GRF_OBJS = Main_Tfst2Grf.o Tfst2Grf.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
               common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o \
               Tfst.o TfstStats.o Sentence2Grf.o File.o List_int.o Error.o StringParsing.o Pattern.o \
               List_ustring.o String_hash.o BitMasks.o BitArray.o DELA.o \
               Alphabet.o Transitions.o Symbol.o Symbol_op.o LanguageDefinition.o \
               Ustring.o Tagset.o UnitexGetOpt.o SingleGraph.o HashTable.o FIFO.o \
               AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o Grf_lib.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
               $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

TFSTTAG      = TfstTag
TFSTTAG_OBJS = Main_TfstTag.o TfstTag.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
               common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o LocateMatches.o Match.o \
               Tfst.o TfstStats.o File.o List_int.o Error.o StringParsing.o Pattern.o \
               List_ustring.o String_hash.o BitMasks.o BitArray.o DELA.o \
               Alphabet.o Transitions.o Symbol.o Symbol_op.o LanguageDefinition.o \
               Ustring.o Tagset.o UnitexGetOpt.o SingleGraph.o HashTable.o FIFO.o \
               AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o Grf_lib.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
               $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

TFST2UNAMBIG      = Tfst2Unambig
TFST2UNAMBIG_OBJS = Main_Tfst2Unambig.o Tfst2Unambig.o Tfst.o TfstStats.o Error.o \
                   IOBuffer.o Copyright.o LinearAutomaton2Txt.o \
                   Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o \
                   Pattern.o List_ustring.o StringParsing.o String_hash.o \
                   List_int.o BitMasks.o Transitions.o DELA.o Alphabet.o \
                   Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o \
                   UnitexGetOpt.o SingleGraph.o BitArray.o HashTable.o FIFO.o File.o \
                   AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                   $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

TOKENIZE      = Tokenize
TOKENIZE_OBJS = Main_Tokenize.o Tokenize.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
                common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Alphabet.o String_hash.o \
                File.o DELA.o AbstractDelaLoad.o PackInf.o List_ustring.o HashTable.o Error.o \
                StringParsing.o UnitexGetOpt.o \
                Offsets.o Overlap.o CompressedDic.o LoadInf.o Ustring.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

TRAININGTAGGER      = TrainingTagger
TRAININGTAGGER_OBJS = Main_TrainingTagger.o TrainingTagger.o TrainingProcess.o IOBuffer.o Copyright.o \
              Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o \
              Tfst.o TfstStats.o Ustring.o SingleGraph.o List_int.o BitMasks.o Error.o UnitexGetOpt.o \
              Transitions.o FIFO.o HashTable.o BitArray.o Symbol.o DELA.o StringParsing.o \
              String_hash.o Alphabet.o List_ustring.o LanguageDefinition.o Tagset.o \
              File.o TaggingProcess.o Match.o Compress.o SortTxt.o ProgramInvoker.o Thai.o \
              DictionaryTree.o AutomatonDictionary2Bin.o AbstractDelaLoad.o PackInf.o CompressedDic.o \
              LoadInf.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
              $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

TXT2TFST      = Txt2Tfst
TXT2TFST_OBJS = Main_Txt2Tfst.o Txt2Tfst.o IOBuffer.o Copyright.o Text_tokens.o Alphabet.o Af_stdio.o \
                ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o DELA_tree.o \
                DELA.o List_ustring.o String_hash.o BuildTextAutomaton.o List_int.o \
                NormalizationFst2.o Fst2.o AbstractFst2Load.o PackFst2.o File.o Error.o \
                StringParsing.o Pattern.o BitMasks.o Grf2Fst2_lib.o SingleGraph.o \
                FIFO.o BitArray.o HashTable.o Transitions.o LocateMatches.o Match.o \
                Symbol.o Symbol_op.o LanguageDefinition.o Ustring.o Tagset.o UnitexGetOpt.o \
                Tfst.o TfstStats.o NewLineShifts.o Korean.o AbstractDelaLoad.o PackInf.o \
                CompressedDic.o LoadInf.o Offsets.o Overlap.o DebugMode.o Grf_lib.o \
                VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)


XMLIZER      = XMLizer
XMLIZER_OBJS = Main_XMLizer.o XMLizer.o IOBuffer.o Copyright.o Af_stdio.o ActivityLogger.o \
               common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o Fst2.o AbstractFst2Load.o PackFst2.o Alphabet.o \
               TransductionVariables.o String_hash.o Error.o StringParsing.o File.o \
               Pattern.o List_ustring.o List_int.o BitMasks.o Fst2TxtAsRoutine.o \
               NormalizeAsRoutine.o DELA.o Buffer.o Fst2Txt_TokenTree.o ParsingInfo.o Transitions.o \
               UnitexGetOpt.o DicVariables.o TransductionStack.o SingleGraph.o FIFO.o \
               Symbol.o Symbol_op.o LanguageDefinition.o Tagset.o Ustring.o Stack_unichar.o \
               Korean.o HashTable.o OutputTransductionVariables.o VariableUtils.o List_pointer.o \
               Overlap.o AbstractDelaLoad.o PackInf.o CompressedDic.o LoadInf.o Offsets.o BitArray.o \
               VirtualFiles.o Persistence.o UnitexRevisionInfo.o Fst2Check_lib.o \
               $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)


UNCOMPRESS       = Uncompress
UNCOMPRESS_OBJS  = Main_Uncompress.o Uncompress.o File.o Error.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o \
                   AbstractAllocator.o DELA.o AbstractDelaLoad.o PackInf.o \
                   String_hash.o StringParsing.o List_ustring.o Alphabet.o Af_stdio.o ActivityLogger.o \
                   UnitexGetOpt.o IOBuffer.o Copyright.o CompressedDic.o LoadInf.o Ustring.o \
                   VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                   $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)


UNTOKENIZE       = Untokenize
UNTOKENIZE_OBJS  = Main_Untokenize.o Untokenize.o File.o Error.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o \
                   DELA.o AbstractDelaLoad.o PackInf.o \
                   String_hash.o StringParsing.o List_int.o List_ustring.o Text_tokens.o \
                   Alphabet.o Af_stdio.o ActivityLogger.o \
                   UnitexGetOpt.o IOBuffer.o Copyright.o CompressedDic.o LoadInf.o Ustring.o \
                   VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                   $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

UNXMLIZE       = Unxmlize
UNXMLIZE_OBJS  = Main_Unxmlize.o Unxmlize.o File.o Error.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o Xml.o Af_stdio.o \
                 ActivityLogger.o AbstractAllocator.o \
                 UnitexGetOpt.o IOBuffer.o Copyright.o Offsets.o Overlap.o HTMLCharacters.o \
                 AsciiSearchTree.o Alphabet.o Ustring.o VirtualFiles.o Persistence.o UnitexRevisionInfo.o \
                 $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBDIRIO)

UNITEXTOOL   = UnitexTool
UNITEXTOOL_OBJS = Main_UnitexTool.o \
                  UnitexTool.o UnitexRevisionInfo.o \
                  AbstractDelaLoad.o PackInf.o AbstractFst2Load.o PackFst2.o Alphabet.o ApplyDic.o AsciiSearchTree.o \
                  AutComplementation.o AutConcat.o AutDeterminization.o AutIntersection.o AutMinimization.o \
                  AutomatonDictionary2Bin.o BitArray.o BitMasks.o Buffer.o BuildKrMwuDic.o BuildTextAutomaton.o \
                  $(CASSYS_FILE_OBJS) CheckDic.o \
                  CodePages.o CompoundWordHashTable.o CompoundWordTree.o Compress.o Concord.o Concordance.o \
                  ConcorDiff.o Contexts.o Convert.o DELA.o DELA_tree.o Dico.o DictionaryTree.o \
                  DicVariables.o Diff.o DutchCompounds.o Elag.o ElagComp.o ElagDebug.o ElagFstFilesIO.o \
                  ElagFunctions.o ElagRulesCompilation.o ElagStateSet.o Error.o Evamb.o Extract.o ExtractUnits.o \
                  FIFO.o File.o Flatten.o FlattenFst2.o Fst2.o Fst2Automaton.o Fst2Check.o Fst2Check_lib.o Fst2List.o Fst2Txt.o \
                  Fst2Txt_TokenTree.o Fst2TxtAsRoutine.o GeneralDerivation.o GermanCompounds.o UnitexGetOpt.o \
                  Grf2Fst2.o Grf2Fst2_lib.o HashTable.o HTMLCharacters.o ImplodeTfst.o IOBuffer.o Copyright.o \
                  KrMwuDic.o LanguageDefinition.o LemmaTree.o LinearAutomaton2Txt.o List_int.o List_pointer.o List_ustring.o \
                  Locate.o LocateFst2Tags.o LocatePattern.o LocateTrace.o LocateTfst.o LocateTfst_lib.o LocateTfstMatches.o \
                  LocateMatches.o Match.o MF_DicoMorpho.o MF_Global.o MF_DLC_inflect.o MF_FormMorpho.o MF_LangMorpho.o \
                  MF_MU_graph.o MF_MU_morpho.o MF_Operators_Util.o MF_SU_morpho.o MF_Unif.o MF_Util.o \
                  MorphologicalFilters.o MorphologicalLocate.o MultiFlex.o NewLineShifts.o NormalizationFst2.o \
                  Normalize.o NormalizeAsRoutine.o NorwegianCompounds.o Offsets.o OptimizedFst2.o ParsingInfo.o Pattern.o \
                  PatternTree.o PolyLex.o PortugueseNormalization.o ProgramInvoker.o RebuildTfst.o Reconstrucao.o \
                  Reg2Grf.o RegularExpressions.o RussianCompounds.o Sentence2Grf.o SingleGraph.o Snt.o SortTxt.o \
                  Stack_int.o Stack_unichar.o Stats.o String_hash.o StringParsing.o Symbol.o Symbol_op.o \
                  SymbolAlphabet.o Table2Grf.o Tagset.o TagsetNormTfst.o TEI2Txt.o Text_parsing.o UserCancelling.o Text_tokens.o Tfst.o \
                  Tfst2Grf.o Tfst2Unambig.o Thai.o Tokenization.o Tokenize.o TransductionStack.o \
                  TransductionVariables.o Transitions.o Txt2Tfst.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o \
                  Ustring.o VersionInfo.o XMLizer.o DuplicateFile.o DumpOffsets.o Seq2Grf.o \
                  Korean.o OptimizedTfstTagMatching.o TransductionStackTfst.o Uncompress.o Untokenize.o Tagger.o \
                  TaggingProcess.o TrainingTagger.o TrainingProcess.o LocateCache.o Arabic.o \
                  OutputTransductionVariables.o TfstStats.o VariableUtils.o Overlap.o CompressedDic.o LoadInf.o \
                  Unxmlize.o Xml.o GrfDiff.o GrfDiff3.o Grf_lib.o GrfSvn_lib.o DebugMode.o \
                  GrfBeauty.o GrfTest.o GrfTest_lib.o SpellCheck.o SpellChecking.o \
                  Keyboard.o VirtualFiles.o Persistence.o PersistenceInterface.o PersistResource.o TfstTag.o PRLG.o \
                  KeyWords.o KeyWords_lib.o \
                  RegExFacade.o $(TRE_LINK_OBJS) SelectOutput.o UnitexLibIO.o $(SYSLIBDIRIO) $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL)


UNITEXTOOL_LOGGER_NO_MAIN_OBJS = FilePack.o FilePackCrc32.o FilePackIo.o UniLogger.o UniLoggerAutoInstall.o \
                  UnpackFile.o UnpackFileTool.o PackFile.o PackFileTool.o \
                  LingResourcePackage.o InstallLingResourcePackage.o \
                  FileUnPack.o ReworkArg.o UniRunLogger.o RunTools.o UniRunScript.o MzToolsUlp.o MzRepairUlp.o \
                  UnitexTool.o UnitexRevisionInfo.o Seq2Grf.o \
                  AbstractDelaLoad.o PackInf.o AbstractFst2Load.o PackFst2.o Alphabet.o ApplyDic.o AsciiSearchTree.o \
                  AutComplementation.o AutConcat.o AutDeterminization.o AutIntersection.o AutMinimization.o \
                  AutomatonDictionary2Bin.o BitArray.o BitMasks.o Buffer.o BuildKrMwuDic.o BuildTextAutomaton.o \
                  $(CASSYS_FILE_OBJS) CheckDic.o \
                  CodePages.o CompoundWordHashTable.o CompoundWordTree.o Compress.o Concord.o Concordance.o \
                  ConcorDiff.o Contexts.o Convert.o DELA.o DELA_tree.o Dico.o DictionaryTree.o \
                  DicVariables.o Diff.o DutchCompounds.o Elag.o ElagComp.o ElagDebug.o ElagFstFilesIO.o \
                  ElagFunctions.o ElagRulesCompilation.o ElagStateSet.o Error.o Evamb.o Extract.o ExtractUnits.o \
                  FIFO.o File.o Flatten.o FlattenFst2.o Fst2.o Fst2Automaton.o Fst2Check.o Fst2Check_lib.o Fst2List.o Fst2Txt.o \
                  Fst2Txt_TokenTree.o Fst2TxtAsRoutine.o GeneralDerivation.o GermanCompounds.o UnitexGetOpt.o \
                  Grf2Fst2.o Grf2Fst2_lib.o HashTable.o HTMLCharacters.o ImplodeTfst.o IOBuffer.o Copyright.o \
                  KrMwuDic.o LanguageDefinition.o LemmaTree.o LinearAutomaton2Txt.o List_int.o List_pointer.o List_ustring.o \
                  Locate.o LocateFst2Tags.o LocatePattern.o LocateTrace.o LocateTfst.o LocateTfst_lib.o LocateTfstMatches.o \
                  LocateMatches.o Match.o MF_DicoMorpho.o MF_DLC_inflect.o MF_FormMorpho.o MF_LangMorpho.o \
                  MF_MU_graph.o MF_MU_morpho.o MF_Global.o MF_Operators_Util.o MF_SU_morpho.o MF_Unif.o MF_Util.o \
                  MorphologicalFilters.o MorphologicalLocate.o MultiFlex.o NewLineShifts.o NormalizationFst2.o \
                  Normalize.o NormalizeAsRoutine.o NorwegianCompounds.o Offsets.o OptimizedFst2.o ParsingInfo.o Pattern.o \
                  PatternTree.o PolyLex.o PortugueseNormalization.o ProgramInvoker.o RebuildTfst.o Reconstrucao.o \
                  Reg2Grf.o RegularExpressions.o RussianCompounds.o Sentence2Grf.o SingleGraph.o Snt.o SortTxt.o \
                  Stack_int.o Stack_unichar.o Stats.o String_hash.o StringParsing.o Symbol.o Symbol_op.o \
                  SymbolAlphabet.o Table2Grf.o Tagset.o TagsetNormTfst.o TEI2Txt.o Text_parsing.o UserCancelling.o Text_tokens.o Tfst.o \
                  Tfst2Grf.o Tfst2Unambig.o Thai.o Tokenization.o Tokenize.o TransductionStack.o \
                  TransductionVariables.o Transitions.o Txt2Tfst.o Af_stdio.o ActivityLogger.o common.o ELG.o table.o utf8.o Unicode.o safe_bool.o chrono.o severity_level.o MsgLogger.o AbstractAllocator.o \
                  Ustring.o VersionInfo.o XMLizer.o DuplicateFile.o DumpOffsets.o \
                  Korean.o OptimizedTfstTagMatching.o TransductionStackTfst.o Uncompress.o Untokenize.o Tagger.o \
                  TaggingProcess.o TrainingTagger.o TrainingProcess.o LocateCache.o Arabic.o \
                  OutputTransductionVariables.o TfstStats.o VariableUtils.o Overlap.o \
                  CompressedDic.o LoadInf.o Unxmlize.o Xml.o GrfDiff.o GrfDiff3.o Grf_lib.o \
                  GrfSvn_lib.o DebugMode.o GrfBeauty.o GrfTest.o GrfTest_lib.o SpellCheck.o \
                  SpellChecking.o Keyboard.o VirtualFiles.o Persistence.o PersistenceInterface.o PersistResource.o \
                  TfstTag.o PRLG.o KeyWords.o KeyWords_lib.o \
                  RegExFacade.o SelectOutput.o UnitexLibIO.o $(TRE_LINK_OBJS) \
                  $(YAML_LINK_OBJS) $(SYSLIBDIRIO) $(ADDITIONAL_OBJECT) $(SYSLIBMAPPED) $(SYSLIBSYNCTOOL) $(SYSLIBLOGGER)

UNITEXTOOL_LOGGER   = UnitexToolLogger
UNITEXTOOL_LOGGER_OBJS = Main_UnitexToolLogger.o $(UNITEXTOOL_LOGGER_NO_MAIN_OBJS)

UNITEXLIB_IO_OBJS = UnitexLibIO.o $(SYSLIBDIRIO)

UNITEXLIB_PACK_RUNLOG_OBJS = UnpackFile.o UnpackFileTool.o PackFile.o PackFileTool.o FilePack.o FileUnPack.o FilePackCrc32.o FilePackIo.o \
                  LingResourcePackage.o InstallLingResourcePackage.o \
                  ReworkArg.o UniRunLogger.o RunTools.o UniRunScript.o MzToolsUlp.o MzRepairUlp.o

UNITEX_JNI_OBJS = fr_umlv_unitex_jni_UnitexJni.o


TEST      = Test
TEST_OBJS = Main_Test.o $(UNITEXTOOL_LOGGER_NO_MAIN_OBJS)

################################
################################
##  PROGRAM AND OBJECT LISTS  ##
################################
################################




ifeq ($(LIBRARY),yes)
OBJS = $(UNITEXTOOL_LOGGER_OBJS)
ADDITIONAL_DEFFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
PROGS = $(TEST)

else
ifeq ($(JNILIBRARY),yes)
OBJS = $(UNITEXTOOL_LOGGER_OBJS)
ADDITIONAL_DEFFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
PROGS = $(TEST)

else
ifeq ($(STATICLIB),yes)
OBJS = $(UNITEXTOOL_LOGGER_OBJS)
ADDITIONAL_DEFFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER
PROGS = $(TEST)

else

ifeq ($(UNITEXTOOLLOGGERONLY), yes)
PROGS = $(UNITEXTOOL_LOGGER)
OBJS = $(UNITEXTOOL_LOGGER_OBJS)
ADDITIONAL_DEFFLAGS += -DUNITEXTOOL_TOOL_FROM_LOGGER

else

ifeq ($(UNITEXTOOLONLY), yes)


PROGS = $(UNITEXTOOL) $(UNITEXTOOL_LOGGER)
OBJS = $(UNITEXTOOL_OBJS) $(UNITEXTOOL_LOGGER_OBJS)
else
ifeq ($(UNITEX_EXPERIMENTAL_MSGLOGGER), yes)
PROGS = $(LOCATE)
OBJS  = $(LOCATE_OBJS)

else

PROGS = $(BUILDKRMWUDIC) $(CASSYS) $(CHECKDIC) $(COMPRESS) $(CONCORD) $(CONCORDIFF) $(CONVERT) $(DICO)\
	    $(DUMPOFFSETS) $(ELAG) $(ELAGCOMP) $(EVAMB) $(EXTRACT) $(FLATTEN) $(FST2CHECK) \
	    $(FST2LIST) $(FST2TXT) $(GRF2FST2) $(IMPLODETFST) $(LOCATE) $(LOCATETFST) $(MULTIFLEX)\
	    $(MZREPAIRULP) $(NORMALIZE) $(POLYLEX) $(REBUILDTFST) $(RECONSTRUCAO) \
	    $(REG2GRF) $(SORTTXT) $(STATS) $(TABLE2GRF) $(TAGGER) $(TAGSETNORMTFST) $(TEI2TXT) \
	    $(TFST2GRF) $(TFST2UNAMBIG) $(TOKENIZE) $(TRAININGTAGGER) $(TXT2TFST) $(XMLIZER) \
	    $(UNCOMPRESS) $(UNTOKENIZE) $(UNITEXTOOL) $(UNITEXTOOL_LOGGER) $(UNXMLIZE) $(TEST)\
	    $(GRFDIFF) $(GRFDIFF3) $(SEQ2GRF) $(GRFTEST) $(SPELLCHECK) $(TFSTTAG) $(KEYWORDS)

PROGS += $(TEST)


OBJS  = $(BUILDKRMWUDIC_OBJS) $(CASSYS_OBJS) $(CHECKDIC_OBJS) $(COMPRESS_OBJS) $(CONCORD_OBJS) $(CONCORDIFF_OBJS) $(CONVERT_OBJS) \
	    $(DUMPOFFSETS_OBJS) $(DICO_OBJS) $(ELAG_OBJS)\
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
	    $(SEQ2GRF_OBJS) $(GRFTEST_OBJS) $(SPELLCHECK_OBJS) $(TFSTTAG_OBJS) \
	    \
	    $(UNITEXLIB_IO_OBJS)

	    #$(COMPRESSKR_OBJS) $(CONSULTDIC_OBJS) $(INFLECTKR_OBJS) $(SUFFORM2RAC_OBJS) \
	    #$(MERGEBIN_OBJS) $(SORTMORPH_OBJS) $(TXT2FST2KR_OBJS) \
	    #$(JAMO2SYL_OBJS) $(SYL2JAMO_OBJS) $(EXTRACTCHAR_OBJS) $(KEYWORDS_OBJS)


endif
endif
endif
endif
endif
endif

BINARIES = $(patsubst %,$(BIN_DIR)%$(EXTENSION),$(PROGS))


###########################
###########################
##  COMPILATION TARGETS  ##
###########################
###########################

ifeq ($(LINUX_LIKE),yes)
#####################
#####################
##  LINUX TARGETS  ##
#####################
#####################

YAML_OPTIONS = -DHAVE_CONFIG_H=1 -I./$(YAML_SRC)/include -I./$(YAML_SRC)/src -I./$(YAML_SRC)/win32 -DYAML_DECLARE_STATIC -O3 -Wall -c -fmessage-length=0

ifneq ($(strip $(ADDITIONAL_YAML_OPTIONS)),)
	YAML_OPTIONS += $(ADDITIONAL_YAML_OPTIONS)
endif

YAML_OPTIONS += $(YAML_GCC_OPTIONS)

ifeq ($(LIBRARY),yes)
all: $(LIBTRE) .libunitex
else
ifeq ($(JNILIBRARY),yes)
all: $(LIBTRE) .libunitexjni
else
ifeq ($(STATICLIB),yes)
all: $(LIBTRE) .libunitexstatic
else
all: bin
endif
endif
endif

bin: $(LIBTRE) $(BINARIES)
	@echo
	@echo "All is done. Type '$(INSTALL_COMMAND)' to install programs in '$(APPDIR)'."
	@echo

%.o: ../%.cpp ../*.h ../ELGLib/*.h ../ELGLib/uString/*.h
	$(CC) -c $(CFLAGS) $<

$(TRE_SRC): $(TRE_TGZ)
	@echo "unpacking TRE library ..."
	$(TAR_COMMAND) $(TAR_PARAM) $(TRE_TGZ)

$(YAML_SRC): $(YAML_TGZ)
	@echo "unpacking YAML library ..."
	$(TAR_COMMAND) $(TAR_PARAM) $(YAML_TGZ)

ifeq ($(TRE_DIRECT_COMPILE), yes)
.libtre-done: $(TRE_SRC)
	@echo "Configure TRE for direct build..."
	chmod +x $(TRE_SRC)/configure
	cd $(TRE_SRC) && ./configure --disable-shared --enable-static $(TRE_CONFIGURE_OPTION)
	touch .libtre-done
else

ifeq ($(TRE_DIRECT_LINK), yes)

ifneq ($(strip $(TRE_GCC_CFLAGS)),)
TRE_GCC_OPTIONS = -DHAVE_CONFIG_H=1 $(TRE_GCC_CFLAGS)
else
TRE_GCC_OPTIONS = -DHAVE_CONFIG_H=1 $(CFLAGS)
endif


ifneq ($(strip $(ADDITIONAL_TRE_OPTIONS)),)
TRE_GCC_OPTIONS += $(ADDITIONAL_TRE_OPTIONS)
endif




.libtre-done: $(TRE_SRC)
	@echo "Making TRE library ..."
	chmod +x $(TRE_SRC)/configure
	cd $(TRE_SRC) && ./configure --disable-shared --enable-static $(TRE_CONFIGURE_OPTION)
	mkdir -p libtre/lib libtre/include libtre/obj
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-stack.c $(TRE_GCC_OPTIONS) -o libtre/obj/tre-stack.o
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-parse.c $(TRE_GCC_OPTIONS) -o libtre/obj/tre-parse.o
	$(CC_GCC) -c $(TRE_SRC)/lib/regexec.c $(TRE_GCC_OPTIONS) -o libtre/obj/regexec.o
	$(CC_GCC) -c $(TRE_SRC)/lib/xmalloc.c $(TRE_GCC_OPTIONS) -o libtre/obj/xmalloc.o
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-match-parallel.c $(TRE_GCC_OPTIONS) -o libtre/obj/tre-match-parallel.o
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-match-approx.c $(TRE_GCC_OPTIONS) -o libtre/obj/tre-match-approx.o
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-ast.c $(TRE_GCC_OPTIONS) -o libtre/obj/tre-ast.o
	$(CC_GCC) -c $(TRE_SRC)/lib/regcomp.c $(TRE_GCC_OPTIONS) -o libtre/obj/regcomp.o
	$(CC_GCC) -c $(TRE_SRC)/lib/regerror.c $(TRE_GCC_OPTIONS) -o libtre/obj/regerror.o
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-compile.c $(TRE_GCC_OPTIONS) -o libtre/obj/tre-compile.o
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-match-backtrack.c $(TRE_GCC_OPTIONS) -o libtre/obj/tre-match-backtrack.o
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-mem.c $(TRE_GCC_OPTIONS) -o libtre/obj/tre-mem.o
	cd libtre/obj && $(AR) rcs ../lib/libtre.a *.o
	touch .libtre-done
else
.libtre-done: $(TRE_SRC)
	@echo "Making TRE library using TRE makefile ..."
	chmod +x $(TRE_SRC)/configure
	cd $(TRE_SRC) && ./configure --disable-shared --enable-static $(TRE_CONFIGURE_OPTION) && make
	mkdir -p libtre/lib libtre/include
	cd $(TRE_SRC)/lib && $(AR) rcs libtre.a *.o
	cp $(TRE_SRC)/lib/libtre.a  libtre/lib/libtre.a
	touch .libtre-done
endif

endif


clean-libtre:
	$(RMR) libtre
	$(RMR) $(TRE_SRC)
	$(RM) .libtre-done


install: bin
	$(INSTALL) $(BINARIES) $(APPDIR)


clean: $(CLEAN-LIBTRE)
	$(RM) $(OBJS) $(PROGS) $(UNITEXLIB_IO_OBJS) $(YAML_LINK_OBJS) $(UNITEXLIB_PACK_RUNLOG_OBJS) *~ Main_*.o

clean-all-but-tre:
	$(RM) $(OBJS) $(PROGS) $(UNITEXLIB_IO_OBJS) $(YAML_LINK_OBJS) $(UNITEXLIB_PACK_RUNLOG_OBJS) *~ Main_*.o

dist: clean
	if test -d $(BUILD); then $(RMR) $(BUILD); fi
	mkdir $(BUILD)
	cp -a $(shell find . -mindepth 1 -maxdepth 1 -name $(BUILD) -prune -o -print) $(BUILD)
	tar cf - $(BUILD)  | gzip > ../$(BUILD).tar.gz
	$(RMR) $(BUILD)

zip: clean
	if test -d $(BUILD); then $(RMR) $(BUILD); fi
	mkdir $(BUILD)
	cp -a $(shell find . -mindepth 1 -maxdepth 1 -name $(BUILD) -prune -o -print) $(BUILD)
	zip -r -q ../$(BUILD).zip $(BUILD)
	$(RMR) $(BUILD)

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
ifeq ($(STATICLIB),yes)
all: $(LIBTRE) .libunitexstatic
else
all: bin
endif
endif
endif



TRE_SRC = tre-0.8.0
TRE_TGZ = ../vendor/tre-0.8.0.tar.gz
YAML_SRC = yaml-0.1.6
YAML_TGZ = ../vendor/yaml-0.1.6.tar.gz

ifeq ($(TRE_DIRECT_COMPILE), yes)
TRE_LIB =
.libtre-done: $(TRE_SRC)
	@echo "unpacking TRE library ......"
	$(TAR_COMMAND) $(TAR_PARAM) $(TRE_TGZ)
	@echo > .libtre-done

else


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
	$(AR) -r libtre.a $(TRE_OBJS)

endif
endif


ifeq ($(TRE_DIRECT_COMPILE), yes)
TRE_LINK_LIB =
else
TRE_LINK_LIB = -ltre
endif



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

$(YAML_SRC): $(YAML_TGZ)
	@echo "unpacking YAML library ..."
	$(TAR_COMMAND) $(TAR_PARAM) $(YAML_TGZ)

LIBTRE: $(TRE_SRC) $(TRE_LIB)
	@echo "Making TRE library..."

CLEAN-LIBTRE   =
LIBS   = -L"." $(TRE_LINK_LIB) $(ADDITIONAL_LIB1) $(ADDITIONAL_LIB2)

%.o: ../%.cpp
	$(CC) -c $(CFLAGS) $<

clean-all-but-tre:
	$(RM) *.o $(BIN_DIR)*.exe $(BIN_DIR)unitex.dll

clean: clean-all-but-tre
	$(RMR) tre-0.8.0
	$(RMR) libtre.a


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

$(YAML_SRC): $(YAML_TGZ)
	@echo "unpacking YAML library ..."
	$(TAR_COMMAND) $(TAR_PARAM) $(YAML_TGZ)

LIBTRE: $(TRE_SRC) $(TRE_LIB)
	@echo "Making TRE library..."

CLEAN-LIBTRE   =
LIBS   = -L"." $(TRE_LINK_LIB) $(ADDITIONAL_LIB1) $(ADDITIONAL_LIB2)

%.o: ../%.cpp
	$(CC) -c $(CFLAGS) $<

clean-all-but-tre:
	$(RM) $(OBJ_DIR)$(OBJS) $(BIN_DIR)*.exe $(BIN_DIR)unitex.dll Main_*.o

clean: clean-all-but-tre
	$(RMR) tre-0.8.0
	$(RMR) libtre.a


#################################
#################################
##  END MINGW WINDOWS TARGETS  ##
#################################
#################################
endif
endif

ifeq ($(DEBUG), yes)
$(BIN_DIR)$(CASSYS_TEST)$(EXTENSION): $(CASSYS_TEST_OBJS)
	$(CC) -o $@ $+ $(OPTIONS)
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

$(BIN_DIR)$(DUMPOFFSETS)$(EXTENSION): $(DUMPOFFSETS_OBJS)
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

$(BIN_DIR)$(KEYWORDS)$(EXTENSION): $(KEYWORDS_OBJS)
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
# Compilation instructions for base
#
table.o: ../base/unicode/table.cpp
	$(CC) -c ../base/unicode/table.cpp $(OPTIONS)

utf8.o: ../base/unicode/utf8.cpp
	$(CC) -c ../base/unicode/utf8.cpp $(OPTIONS)
	
case.o: ../base/unicode/case.cpp
	$(CC) -c ../base/unicode/case.cpp $(OPTIONS)	

severity_level.o: ../base/unilog/severity_level.cpp
	$(CC) -c ../base/unilog/severity_level.cpp $(OPTIONS)

chrono.o: ../base/time/chrono.cpp
	$(CC) -c ../base/time/chrono.cpp $(OPTIONS)

safe_bool.o: ../base/boolean/safe_bool.cpp
	$(CC) -c ../base/boolean/safe_bool.cpp $(OPTIONS)
	
#
# Compilation instructions for ELG
#
common.o: ../ELGLib/common.cpp
	$(CC) -c ../ELGLib/common.cpp $(OPTIONS)	

#once.o: ../base/thread/once.cpp
#	$(CC) -c ../base/thread/once.cpp $(OPTIONS)

#
# Compilation instructions for ELG
#

#
# Compilation instructions for Logger
#

FileUnPack.o: ../logger/FileUnPack.cpp
	$(CC) -c ../logger/FileUnPack.cpp $(OPTIONS)

ReworkArg.o: ../logger/ReworkArg.cpp
	$(CC) -c ../logger/ReworkArg.cpp $(OPTIONS)

UniRunLogger.o: ../logger/UniRunLogger.cpp
	$(CC) -c ../logger/UniRunLogger.cpp $(OPTIONS)

RunTools.o: ../logger/RunTools.cpp
	$(CC) -c ../logger/RunTools.cpp $(OPTIONS)

UniRunScript.o: ../logger/UniRunScript.cpp
	$(CC) -c ../logger/UniRunScript.cpp $(OPTIONS)

FilePack.o: ../logger/FilePack.cpp
	$(CC) -c ../logger/FilePack.cpp $(OPTIONS)

FilePackCrc32.o: ../logger/FilePackCrc32.cpp
	$(CC) -c ../logger/FilePackCrc32.cpp $(OPTIONS)

FilePackIo.o: ../logger/FilePackIo.cpp
	$(CC) -c ../logger/FilePackIo.cpp $(OPTIONS)

PackFile.o: ../logger/PackFile.cpp
	$(CC) -c ../logger/PackFile.cpp $(OPTIONS)

PackFileTool.o: ../logger/PackFileTool.cpp
	$(CC) -c ../logger/PackFileTool.cpp $(OPTIONS)

UnpackFile.o: ../logger/UnpackFile.cpp
	$(CC) -c ../logger/UnpackFile.cpp $(OPTIONS)

UnpackFileTool.o: ../logger/UnpackFileTool.cpp
	$(CC) -c ../logger/UnpackFileTool.cpp $(OPTIONS)

InstallLingResourcePackage.o: ../logger/InstallLingResourcePackage.cpp
	$(CC) -c ../logger/InstallLingResourcePackage.cpp $(OPTIONS)

LingResourcePackage.o: ../logger/LingResourcePackage.cpp
	$(CC) -c ../logger/LingResourcePackage.cpp $(OPTIONS)

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


# compilation option for yaml	
api.o: $(YAML_SRC) yaml-0.1.6/src/api.c
	$(CC_GCC) -c yaml-0.1.6/src/api.c $(YAML_OPTIONS)
	
writer.o: $(YAML_SRC) yaml-0.1.6/src/writer.c
	$(CC_GCC) -c yaml-0.1.6/src/writer.c $(YAML_OPTIONS)
	
loader.o: $(YAML_SRC) yaml-0.1.6/src/loader.c
	$(CC_GCC) -c yaml-0.1.6/src/loader.c $(YAML_OPTIONS)
	
reader.o: $(YAML_SRC) yaml-0.1.6/src/reader.c
	$(CC_GCC) -c yaml-0.1.6/src/reader.c $(YAML_OPTIONS)
	
parser.o: $(YAML_SRC) yaml-0.1.6/src/parser.c
	$(CC_GCC) -c yaml-0.1.6/src/parser.c $(YAML_OPTIONS)
	
scanner.o: $(YAML_SRC) yaml-0.1.6/src/scanner.c
	$(CC_GCC) -c yaml-0.1.6/src/scanner.c $(YAML_OPTIONS)
	
emitter.o: $(YAML_SRC) yaml-0.1.6/src/emitter.c
	$(CC_GCC) -c yaml-0.1.6/src/emitter.c $(YAML_OPTIONS)
	
dumper.o: $(YAML_SRC) yaml-0.1.6/src/dumper.c
	$(CC_GCC) -c yaml-0.1.6/src/dumper.c $(YAML_OPTIONS)


	
	
ifeq ($(TRE_DIRECT_COMPILE), yes)


ifneq ($(strip $(TRE_GCC_OPTIONS)),)
TRE_DIRECT_COMPILE_OPTIONS = $(TRE_GCC_OPTIONS) -DHAVE_CONFIG_H=1 -Disdirect=1
else
TRE_DIRECT_COMPILE_OPTIONS = $(OPTIONS) -DHAVE_CONFIG_H=1 -Disdirect=1
endif


regcomp.o : $(TRE_SRC)/lib/regcomp.c
	$(CC_GCC) -c $(TRE_SRC)/lib/regcomp.c $(TRE_DIRECT_COMPILE_OPTIONS)

regerror.o : $(TRE_SRC)/lib/regerror.c
	$(CC_GCC) -c $(TRE_SRC)/lib/regerror.c $(TRE_DIRECT_COMPILE_OPTIONS)

regexec.o : $(TRE_SRC)/lib/regexec.c
	$(CC_GCC) -c $(TRE_SRC)/lib/regexec.c $(TRE_DIRECT_COMPILE_OPTIONS)

tre-ast.o : $(TRE_SRC)/lib/tre-ast.c
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-ast.c $(TRE_DIRECT_COMPILE_OPTIONS)

tre-compile.o : $(TRE_SRC)/lib/tre-compile.c
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-compile.c $(TRE_DIRECT_COMPILE_OPTIONS)

tre-match-approx.o : $(TRE_SRC)/lib/tre-match-approx.c
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-match-approx.c $(TRE_DIRECT_COMPILE_OPTIONS)

tre-match-backtrack.o : $(TRE_SRC)/lib/tre-match-backtrack.c
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-match-backtrack.c $(TRE_DIRECT_COMPILE_OPTIONS)

tre-match-parallel.o : $(TRE_SRC)/lib/tre-match-parallel.c
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-match-parallel.c $(TRE_DIRECT_COMPILE_OPTIONS)

tre-mem.o : $(TRE_SRC)/lib/tre-mem.c
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-mem.c $(TRE_DIRECT_COMPILE_OPTIONS)

tre-parse.o : $(TRE_SRC)/lib/tre-parse.c
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-parse.c $(TRE_DIRECT_COMPILE_OPTIONS)

tre-stack.o : $(TRE_SRC)/lib/tre-stack.c
	$(CC_GCC) -c $(TRE_SRC)/lib/tre-stack.c $(TRE_DIRECT_COMPILE_OPTIONS)

xmalloc.o : $(TRE_SRC)/lib/xmalloc.c
	$(CC_GCC) -c $(TRE_SRC)/lib/xmalloc.c $(TRE_DIRECT_COMPILE_OPTIONS)

endif

fr_umlv_unitex_jni_UnitexJni.o : ../UnitexLibAndJni/fr_umlv_unitex_jni_UnitexJni.cpp\
	                                   ../UnitexLibAndJni/fr/umlv/unitex/jni/UnitexJni.java
	javac ../UnitexLibAndJni/fr/umlv/unitex/jni/UnitexJni.java
	javah -classpath ../UnitexLibAndJni -o ../UnitexLibAndJni/fr_umlv_unitex_jni_UnitexJni.h fr.umlv.unitex.jni.UnitexJni
	$(CC) ../UnitexLibAndJni/fr_umlv_unitex_jni_UnitexJni.cpp -c $(OPTIONS) -I"../logger"

# Compiling Unitex as a library under Linux-like systems
ifeq ($(LINUX_LIKE),yes)

DYLIBEXT = so
ifeq ($(UNAME),Darwin)
DYLIBEXT = dylib
endif

ifeq ($(SYSTEM),osxcross)
DYLIBEXT = dylib
endif

no_o_files:
	$(RM) *.o

.libunitex: no_o_files $(OBJS) Test_lib.o
	$(RM) Main_*.o
	$(CC) *.o $(OPTIONS) -shared -o ../bin/libunitex.$(DYLIBEXT)
	$(CC) Test_lib.o -L../bin -Wl,-rpath,. -lunitex $(OPTIONS) -o ../bin/Test_lib

.libunitexjni: no_o_files $(UNITEX_JNI_OBJS) $(OBJS) Test_lib.o
	$(RM) Main_*.o
	$(CC) *.o $(OPTIONS) -shared -o ../bin/libUnitexJni.$(DYLIBEXT)
	jar cvf ../bin/UnitexJni.jar -C ../UnitexLibAndJni/ fr/umlv/unitex/jni/UnitexJni.class
	$(CC) Test_lib.o -L../bin -Wl,-rpath,. -lUnitexJni $(OPTIONS) -o ../bin/Test_lib

.libunitexstatic: $(OBJS)
	$(RM) Main_*.o
	$(AR) rcs ../bin/libunitex.a *.o
	touch .libunitexstatic
else

# Compiling Unitex as a library under Windows
no_o_files:
	$(RM) -f *.o

.libunitex: no_o_files $(LIBTRE) $(OBJS) Test_lib.o
	$(RM) Main_*.o	
	$(CC) *.o -shared -Wl,--export-all-symbols -L"." $(TRE_LINK_LIB) $(ADDITIONAL_LIB1) $(ADDITIONAL_LIB2) -o ../bin/unitex.dll
	$(CC) Test_lib.o $(OPTIONS) ../bin/unitex.dll -o ../bin/Test_lib.exe

.libunitexjni: no_o_files $(UNITEX_JNI_OBJS) $(UNITEXLIB_IO_OBJS) $(YAML_LINK_OBJS) $(UNITEXLIB_PACK_RUNLOG_OBJS) $(LIBTRE) $(OBJS) Test_lib.o
	-$(RM) Main_*.o
	$(CC) *.o -shared -Wl,--export-all-symbols,--kill-at -L"." $(TRE_LINK_LIB) $(ADDITIONAL_LIB1) $(ADDITIONAL_LIB2) -o ../bin/UnitexJni.dll
	jar cvf ../bin/UnitexJni.jar -C ../UnitexLibAndJni/ fr/umlv/unitex/jni/UnitexJni.class
	$(CC) Test_lib.o $(OPTIONS) ../bin/UnitexJni.dll -o ../bin/Test_lib.exe

.libunitexstatic: $(UNITEXLIB_IO_OBJS) $(YAML_LINK_OBJS) $(UNITEXLIB_PACK_RUNLOG_OBJS) $(OBJS)
	$(RM) Main_*.o
	$(AR) rcs ../bin/libunitex.a *.o
	touch .libunitexstatic

endif

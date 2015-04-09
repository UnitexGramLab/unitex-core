/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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




#include "Unicode.h"
#include "File.h"
#include "Error.h"
#include "Copyright.h"
#include "VersionInfo.h"
#include "RegExFacade.h"
#include "UnitexRevisionInfo.h"
#include "UnitexGetOpt.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_VersionInfo =
         "Usage: VersionInfo [OPTIONS] <outfile>\n"
         "\n"
         "  <outfile>: the destination file\n"
         "\n"
         "OPTIONS:\n"
         "  -o OUTFILE/--output=OUTFILE: path to output file to create\n"
         "  -c/--copyright: display only copyright text\n"
         "  -v/--version: display only version number\n"
         "  -r/--revision: display only SVN revision number (if available)\n"
         "  -p/--platform: platform info\n"
         "  -m/--compiler: compiler used to build unitex info\n"
         "  -h/--help: this help\n"
         "\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_VersionInfo);
}


const char* optstring_VersionInfo=":hmcrvpo:k:q:";
const struct option_TS lopts_VersionInfo[]= {
      {"copyright",no_argument_TS,NULL,'c'},
      {"version",no_argument_TS,NULL,'v'},
      {"revision",no_argument_TS,NULL,'r'},
      {"platform",no_argument_TS,NULL,'p'},
      {"compiler",no_argument_TS,NULL,'m'},
      {"output",required_argument_TS,NULL,'o'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};

static void fill_compiler_info(unichar* s) {
#if defined(__clang__)
    #if defined(__clang_major__) && defined(__clang_minor__) && defined(__clang_patchlevel__) && defined(__clang_version__)
        u_sprintf(s, "Compiler : clang %d.%d.%d - %s\n",__clang_major__,__clang_minor__,__clang_patchlevel__,__clang_version__);
    #elif defined(__clang_major__) && defined(__clang_minor__)
        u_sprintf(s, "Compiler : clang %d.%d.%d : %s\n",__clang_major__,__clang_minor__);
    #else
        u_sprintf(s, "Compiler : clang\n");
    #endif
    /* Clang/LLVM. ---------------------------------------------- */

#elif defined(__ICC) || defined(__INTEL_COMPILER)
    /* Intel ICC/ICPC. ------------------------------------------ */
    #if defined(__INTEL_COMPILER)
        u_sprintf(s, "Compiler : Intel ICC %.2f\n",__INTEL_COMPILER / 100.);
    #else
        u_sprintf(s, "Compiler : Intel ICC\n");
    #endif

#elif defined(__GNUC__) || defined(__GNUG__)

    #if defined(__MINGW32__)
    #define compiler_name "MinGW32 GCC"
    #elif defined(__MINGW64__)
    #define compiler_name "MinGW64 GCC"
    #else
    #define compiler_name "GCC"
    #endif

    #if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__) && defined(__VERSION__)
        u_sprintf(s, "Compiler : %s %d.%d.%d - %s\n",compiler_name,__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__,__VERSION__);
    #elif defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__VERSION__)
        u_sprintf(s, "Compiler : %s %d.%d : %s\n",compiler_name,__GNUC__,__GNUC_MINOR__,__VERSION__);
    #elif defined(__GNUC__) && defined(__GNUC_MINOR__)
        u_sprintf(s, "Compiler : %s %d\n",compiler_name,__GNUC__,__GNUC_MINOR__);
    #else
        u_sprintf(s, "Compiler : %s\n",compiler_name);
    #endif
    /* Clang/LLVM. ---------------------------------------------- */
    /* GNU GCC/G++. --------------------------------------------- */

#elif defined(__HP_cc) || defined(__HP_aCC)
    /* Hewlett-Packard C/aC++. ---------------------------------- */
    #if defined(__HP_aCC)
        u_sprintf(s, "Compiler : Hewlett-Packard C/aC++ %d\n",__HP_aCC);
    #else
        u_sprintf(s, "Compiler : Hewlett-Packard C/aC++\n");
    #endif

#elif defined(__IBMC__) || defined(__IBMCPP__)
    /* IBM XL C/C++. -------------------------------------------- */
    #if defined(__xlc_)
        u_sprintf(s, "Compiler : IBM XL C/C++ %s\n",__xlc_);
    #else
        u_sprintf(s, "Compiler : IBM XL C/C++\n");
    #endif

#elif defined(_MSC_VER)
    /* Microsoft Visual Studio. --------------------------------- */
    #if defined(_MSC_FULL_VER) && defined(_MSC_BUILD)
        u_sprintf(s, "Compiler : Microsoft C/C++ %.2f full version %d build %d\n", _MSC_VER / 100., _MSC_FULL_VER, _MSC_BUILD);
    #else
        u_sprintf(s, "Compiler : Microsoft C/C++ %.2f\n", _MSC_VER / 100.);
    #endif

#elif defined(__PGI)
    #if defined(__PGIC__) && defined(__PGIC_MINOR) && defined(__PGIC_PATCHLEVEL__)
        u_sprintf(s, "Compiler : Group PGCC/PGCPP %d.%d.%\n", __PGIC__,__PGIC_MINOR,__PGIC_PATCHLEVEL__);
    #else
        u_sprintf(s, "Compiler : Group PGCC/PGCPP\n");
    #endif
    /* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
    /* Oracle Solaris Studio. ----------------------------------- */
    #if defined(__SUNPRO_CC)
        u_sprintf(s, "Compiler : Oracle Solaris Studio %x\n",__SUNPRO_CC);
    #else
        u_sprintf(s, "Compiler : Oracle Solaris Studio\n");
    #endif
#else
        u_sprintf(s, "Compiler : unknown\n");
#endif

    u_sprintf(s + u_strlen(s),"Pointer size: %d bits\n",sizeof(void*) * 8);
}


int main_VersionInfo(int argc,char* const argv[]) {
    /*
if (argc==1) {
   usage();
   return 0;
}
*/


const char *output_file = NULL;
int do_version_only=0;
int do_revision_only=0;
int do_copyright_only=0;
int do_platform_info=0;
int do_compiler_info=0;
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_VersionInfo,lopts_VersionInfo,&index,vars))) {
   switch(val) {
   case 'c': do_copyright_only=1; break;
   case 'r': do_revision_only=1; break;
   case 'v': do_version_only=1; break;
   case 'p': do_platform_info = 1; break;
   case 'm': do_compiler_info = 1; break;
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output argument\n");
             }
             output_file = vars->optarg;
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_VersionInfo[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),vars->optarg);
             break;
   case 'k': /* ignore -k and parameter instead make error */
             break;
   }
   index=-1;
}

if (((do_revision_only!=0) && (do_version_only!=0)) ||
    ((do_copyright_only!=0) && (do_version_only!=0)) ||
    ((do_revision_only!=0) && (do_copyright_only!=0))) {
   fatal_error("Incompatible arguments: rerun with --help\n");
}


#define MAX_SIZE_DISPLAY_TEXT 0x1000

unichar* DisplayText = (unichar*)malloc(sizeof(unichar) * (MAX_SIZE_DISPLAY_TEXT + 1));
if (DisplayText == NULL) {
    fatal_alloc_error("main_VersionInfo");
}

int revision = get_unitex_revision();

unsigned int unitexMajorVersion = 0;
unsigned int unitexMinorVersion = 0;
get_unitex_version(&unitexMajorVersion, &unitexMinorVersion);

DisplayText[0] = 0;
if (do_copyright_only) {
    u_sprintf(DisplayText,"%S",COPYRIGHT);
}
else if (do_revision_only) {
    u_sprintf(DisplayText,"%d",revision);
}
else if (do_version_only) {
    u_sprintf(DisplayText,"%u.%u",unitexMajorVersion,unitexMinorVersion);
}
else {
    if (revision == -1) {
        u_sprintf(DisplayText,"%S\nUnitex Version: %u.%u\n",
                COPYRIGHT,unitexMajorVersion,unitexMinorVersion);
    }
    else {
        u_sprintf(DisplayText,"%S\nUnitex Version: %u.%u\nUnitex SVN revision: %d\n",
                COPYRIGHT,unitexMajorVersion,unitexMinorVersion,revision);
    }
}


if (!check_regex_lib_in_unitex()) {
    u_sprintf(DisplayText + u_strlen(DisplayText),"Regex Library is not functionnal\n");
    error("Regex Library is not functionnal\n");
}

if (do_platform_info) {
    union {
        unsigned short int i;
        unsigned char c[8];
    } endianText;

    for (int i=0;i<8;i++) {
        endianText.c[i] = 0;
    }

    endianText.c[0] = 1;
    endianText.c[1] = 2;
    u_sprintf(DisplayText + u_strlen(DisplayText),"\nsizeof(void*):%u, sizeof(int):%u, endianess:%s\n",
                sizeof(void*),sizeof(int),
                ((endianText.i)==0x0102) ? "big endian" :
                  ((endianText.i)==0x0201) ? "little endian" : "unknown endianness");
}

if (do_compiler_info) {
    u_sprintf(DisplayText + u_strlen(DisplayText), "\n");
    fill_compiler_info(DisplayText + u_strlen(DisplayText));
}
if (output_file!=NULL) {
   U_FILE* text = u_fopen(&vec,output_file,U_WRITE);
   if (text==NULL) {
      error("Cannot create text file %s\n",output_file);
      free(DisplayText);
      return 1;
   }
   u_fprintf(text,"%S", DisplayText);
   u_fclose(text);
}
else {
    u_printf("%S", DisplayText);
}

free(DisplayText);
return 0;
}

} // namespace unitex

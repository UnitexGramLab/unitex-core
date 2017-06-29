/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
         "  -n/--newrevision: display only SVN new revision number (if available)\n"
         "  -g/--git: display only git hash (if available)\n"
         "  -b/--build_date: display only build date (if available)\n"
         "  -B/--build_date_yyyy_mm_dd: display only build date (if available) begin with year\n"
         "  -t/--build_date_time: display only build date and time (if available)\n"
         "  -p/--platform: platform info\n"
         "  -m/--compiler: compiler used to build unitex info\n"
         "  -j/--json: revision and version on json string\n"
         "  -x/--xml: revision and version on xml string\n"
         "  -s/--semver: revision and version on semver string\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_VersionInfo);
}

const char* optstring_VersionInfo=":VhmcRrvpjxsubBtngo:k:q:";
const struct option_TS lopts_VersionInfo[]= {
  {"copyright",no_argument_TS,NULL,'c'},
  {"version",no_argument_TS,NULL,'v'},
  {"revision",no_argument_TS,NULL,'r'},
  {"build_date",no_argument_TS,NULL,'b'},
  {"build_date_yyyy_mm_dd",no_argument_TS,NULL,'B'},
  {"build_date_time",no_argument_TS,NULL,'t'},
  {"newrevision",no_argument_TS,NULL,'n'},
  {"git",no_argument_TS,NULL,'g'},
  {"platform",no_argument_TS,NULL,'p'},
  {"compiler",no_argument_TS,NULL,'m'},
  {"xml",no_argument_TS,NULL,'x'},
  {"json",no_argument_TS,NULL,'j'},
  {"semver",no_argument_TS,NULL,'s'},
  {"output",required_argument_TS,NULL,'o'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
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



#ifdef __DATE__
// see https://stackoverflow.com/questions/11697820/how-to-use-date-and-time-predefined-macros-in-as-two-integers-then-stri
// Example of __DATE__ string: "Jul 27 2012"
//                              01234567890
#define BUILD_YEAR_CH0 (__DATE__[ 7])
#define BUILD_YEAR_CH1 (__DATE__[ 8])
#define BUILD_YEAR_CH2 (__DATE__[ 9])
#define BUILD_YEAR_CH3 (__DATE__[10])

#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')

#define BUILD_MONTH_CH0 \
    ((BUILD_MONTH_IS_OCT || BUILD_MONTH_IS_NOV || BUILD_MONTH_IS_DEC) ? '1' : '0')

#define BUILD_MONTH_CH1 \
    ( \
        (BUILD_MONTH_IS_JAN) ? '1' : \
        (BUILD_MONTH_IS_FEB) ? '2' : \
        (BUILD_MONTH_IS_MAR) ? '3' : \
        (BUILD_MONTH_IS_APR) ? '4' : \
        (BUILD_MONTH_IS_MAY) ? '5' : \
        (BUILD_MONTH_IS_JUN) ? '6' : \
        (BUILD_MONTH_IS_JUL) ? '7' : \
        (BUILD_MONTH_IS_AUG) ? '8' : \
        (BUILD_MONTH_IS_SEP) ? '9' : \
        (BUILD_MONTH_IS_OCT) ? '0' : \
        (BUILD_MONTH_IS_NOV) ? '1' : \
        (BUILD_MONTH_IS_DEC) ? '2' : \
        /* error default */    '?' \
    )

#define BUILD_DAY_CH0 ((__DATE__[4] >= '0') ? (__DATE__[4]) : '0')
#define BUILD_DAY_CH1 (__DATE__[ 5])

// Example of __TIME__ string: "21:06:19"
//                              01234567

#define BUILD_HOUR_CH0 (__TIME__[0])
#define BUILD_HOUR_CH1 (__TIME__[1])

#define BUILD_MIN_CH0 (__TIME__[3])
#define BUILD_MIN_CH1 (__TIME__[4])

#define BUILD_SEC_CH0 (__TIME__[6])
#define BUILD_SEC_CH1 (__TIME__[7])

#define COMPUTE_BUILD_YEAR \
    ( \
        (__DATE__[ 7] - '0') * 1000 + \
        (__DATE__[ 8] - '0') *  100 + \
        (__DATE__[ 9] - '0') *   10 + \
        (__DATE__[10] - '0') \
    )

#define COMPUTE_BUILD_DAY \
    ( \
        ((__DATE__[4] >= '0') ? (__DATE__[4] - '0') * 10 : 0) + \
        (__DATE__[5] - '0') \
    )

#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')

#define COMPUTE_BUILD_MONTH \
    ( \
        (BUILD_MONTH_IS_JAN) ?  1 : \
        (BUILD_MONTH_IS_FEB) ?  2 : \
        (BUILD_MONTH_IS_MAR) ?  3 : \
        (BUILD_MONTH_IS_APR) ?  4 : \
        (BUILD_MONTH_IS_MAY) ?  5 : \
        (BUILD_MONTH_IS_JUN) ?  6 : \
        (BUILD_MONTH_IS_JUL) ?  7 : \
        (BUILD_MONTH_IS_AUG) ?  8 : \
        (BUILD_MONTH_IS_SEP) ?  9 : \
        (BUILD_MONTH_IS_OCT) ? 10 : \
        (BUILD_MONTH_IS_NOV) ? 11 : \
        (BUILD_MONTH_IS_DEC) ? 12 : \
        /* error default */  99 \
    )

#define COMPUTE_BUILD_HOUR ((__TIME__[0] - '0') * 10 + __TIME__[1] - '0')
#define COMPUTE_BUILD_MIN  ((__TIME__[3] - '0') * 10 + __TIME__[4] - '0')
#define COMPUTE_BUILD_SEC  ((__TIME__[6] - '0') * 10 + __TIME__[7] - '0')

#define BUILD_DATE_IS_BAD (__DATE__[0] == '?')

#define BUILD_YEAR  ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_YEAR)
#define BUILD_MONTH ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_MONTH)
#define BUILD_DAY   ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_DAY)

#define BUILD_TIME_IS_BAD (__TIME__[0] == '?')

#define BUILD_HOUR  ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_HOUR)
#define BUILD_MIN   ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_MIN)
#define BUILD_SEC   ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_SEC)

const char buildDateTime[] =
{
  BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3,
  '-',
  BUILD_MONTH_CH0, BUILD_MONTH_CH1,
  '-',
  BUILD_DAY_CH0, BUILD_DAY_CH1,
  'T',
  BUILD_HOUR_CH0, BUILD_HOUR_CH1,
  ':',
  BUILD_MIN_CH0, BUILD_MIN_CH1,
  ':',
  BUILD_SEC_CH0, BUILD_SEC_CH1,
  '\0'
};


const char buildDateYYYYMMDD[] =
{
  BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3,
  '-',
  BUILD_MONTH_CH0, BUILD_MONTH_CH1,
  '-',
  BUILD_DAY_CH0, BUILD_DAY_CH1,
  '\0'
};

static const char * get_build_date()
{
    return __DATE__;
}
static const char * get_build_date_time()
{
  return buildDateTime;
}

static const char * get_build_dateYYYYMMDD()
{
  return buildDateYYYYMMDD;
}
#else
static const char * get_build_date()
{
    return("date not available");
}
static const char * get_build_date_time()
{
  return("date not available");
}

static const char * get_build_dateYYYYMMDD()
{
  return("date not available");
}
#endif

int main_VersionInfo(int argc,char* const argv[]) {
    /*
if (argc==1) {
   usage();
   return 0;
}
*/

int retValue = SUCCESS_RETURN_CODE;
const char* output_file = NULL;
int do_version_only=0;
int do_revision_only=0;
int do_copyright_only=0;
int do_platform_info=0;
int do_compiler_info=0;
int do_xml_info=0;
int do_json_info=0;
int do_semver_info=0;
int do_user_friendly_info=0;
int do_build_date_only=0;
int do_build_date_time_only=0;
int do_build_date_yyyymmdd=0;
int do_new_revision_only=0;
int do_git_only=0;
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_VersionInfo,lopts_VersionInfo,&index))) {
   switch(val) {
   case 'c': do_copyright_only=1; break;
   case 'r': do_revision_only=1; break;
   case 'b': do_build_date_only = 1; break;
   case 't': do_build_date_time_only = 1; break;
   case 'B': do_build_date_yyyymmdd = 1; break;
   case 'n': do_new_revision_only = 1; break;
   case 'g': do_git_only = 1; break;
   case 'R': retValue = get_unitex_revision(); break;
   case 'v': do_version_only=1; break;
   case 'p': do_platform_info = 1; break;
   case 'm': do_compiler_info = 1; break;
   case 'j': do_json_info = 1; break;
   case 'x': do_xml_info = 1; break;
   case 's': do_semver_info = 1; break;
   case 'u': do_user_friendly_info = 1; break;
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("Empty output argument\n");
                return USAGE_ERROR_CODE;
             }
             output_file = options.vars()->optarg; // FIXME(gvollant)
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_VersionInfo[index].name);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                return USAGE_ERROR_CODE;
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   case 'k': /* ignore -k and parameter instead make error */
             break;
   }
   index=-1;
}

if ((do_revision_only + do_version_only + do_copyright_only + do_build_date_only + do_new_revision_only + do_git_only)>1) {
   error("Incompatible arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

#define MAX_SIZE_DISPLAY_TEXT 0x1000

unichar* DisplayText = (unichar*)malloc(sizeof(unichar) * (MAX_SIZE_DISPLAY_TEXT + 1));
if (DisplayText == NULL) {
  alloc_error("main_VersionInfo");
  return ALLOC_ERROR_CODE;
}

int revision = get_unitex_revision();
int new_revision = get_unitex_new_revision();
const char* git_revision=get_unitex_core_git_revision();

unsigned int unitexMajorVersion = 0;
unsigned int unitexMinorVersion = 0;
get_unitex_version(&unitexMajorVersion, &unitexMinorVersion);

DisplayText[0] = 0;
if (do_json_info) {
    char buf[0x200]="";
    get_unitex_version_revision_json_string(buf, sizeof(buf));
    u_sprintf(DisplayText,"%s", buf);
} else if (do_xml_info) {
    char buf[0x200]="";
    get_unitex_version_revision_xml_string(buf, sizeof(buf));
    u_sprintf(DisplayText,"%s", buf);
} else if (do_user_friendly_info) {
    char buf[0x200]="";
    strcpy(buf, get_unitex_verbose_version_string());
    u_sprintf(DisplayText,"%s", buf);
} else if (do_semver_info) {
    char buf[0x200]="";
    strcpy(buf,get_unitex_semver_string());
    u_sprintf(DisplayText,"%s", buf);
} else if (do_copyright_only) {
    get_copyright_notice(DisplayText, MAX_SIZE_DISPLAY_TEXT);
}
else if (do_revision_only) {
    u_sprintf(DisplayText,"%d",revision);
}
else if (do_new_revision_only) {
    u_sprintf(DisplayText,"%d",new_revision);
}
else if (do_git_only) {
    u_sprintf(DisplayText,"%s",git_revision);
}
else if (do_build_date_only) {
    u_sprintf(DisplayText,"%s", get_build_date());
}
else if (do_build_date_time_only) {
  u_sprintf(DisplayText, "%s", get_build_date_time());
}
else if (do_build_date_yyyymmdd) {
  u_sprintf(DisplayText, "%s", get_build_dateYYYYMMDD());
}
else if (do_version_only) {
    u_sprintf(DisplayText,"%u.%u",unitexMajorVersion,unitexMinorVersion);
}
else {
    if (revision == -1) {
        get_copyright_notice(DisplayText, MAX_SIZE_DISPLAY_TEXT);
        u_sprintf(DisplayText+u_strlen(DisplayText),"\nUnitex Version: %u.%u\n",
                unitexMajorVersion,unitexMinorVersion);
    }
    else {
        get_copyright_notice(DisplayText, MAX_SIZE_DISPLAY_TEXT);
        u_sprintf(DisplayText+u_strlen(DisplayText),"\nUnitex Version: %u.%u\nUnitex SVN revision: %d\n",
                unitexMajorVersion,unitexMinorVersion,revision);
    }
}

if (!check_regex_lib_in_unitex()) {
    u_sprintf(DisplayText + u_strlen(DisplayText),"Regex Library is not functionnal\n");
    error("Regex Library is not functionnal\n");
    retValue = DEFAULT_ERROR_CODE;
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
      return DEFAULT_ERROR_CODE;
   }
   u_fprintf(text,"%S", DisplayText);
   u_fclose(text);
}
else {
    u_printf("%S\n", DisplayText);
}

free(DisplayText);

// FIXME(gvollant) To conform with the C/C++ standard, isn't a good idea to
// return a custom integer equal to get_unitex_revision() here
// An exit value greater than 255 returns an exit code modulo 256
// see http://www.tldp.org/LDP/abs/html/exitcodes.html#AEN23629
return retValue;
}

} // namespace unitex

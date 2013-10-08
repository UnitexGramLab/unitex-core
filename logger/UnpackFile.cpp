#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include "Unicode.h"
#include "Copyright.h"
#include "UnitexGetOpt.h"

#include "Af_stdio.h"
#include "DirHelper.h"

#include "FilePackType.h"
#include "FileUnPack.h"
#include "UnpackFileTool.h"

#include "UnpackFile.h"

#ifndef NO_UNITEX_LOGGER

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#ifdef HAS_LOGGER_NAMESPACE
using namespace ::unitex::logger;
#endif
/*
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {
*/

extern const char* optstring_UnpackFile;
extern const struct option_TS lopts_UnpackFile[];
extern const char* usage_UnpackFile;


const char* usage_UnpackFile =
         "Usage : UnpackFile [OPTIONS] <ulpfile>\n"
         "\n"
		 "  <ulpfile>: an ulp (or uncompressed zipfile) archive file to extract\n"
         "\n"
         "OPTIONS:\n"
		 "  -d X/--extractdir=X: uses X as prefix for destination (include / or \\ after)\n"
		 "  -j/--junk: remove filepath from filename inside archive\n"
		 "  -p/--pathseparator: transform path separator to current platform standard\n"
		 "  -l/--list: just list content of archive\n"
         "  -m/--quiet: do not emit message when running\n"
         "  -v/--verbose: emit message when running\n"
         "\n";

static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_UnpackFile);
}


const char* optstring_UnpackFile=":hd:f:jpmlk:q:";
const struct option_TS lopts_UnpackFile[]={
   {"extractdir", required_argument_TS, NULL, 'd'},
   {"selectfile", required_argument_TS, NULL, 'f'},
   {"junk",no_argument_TS,NULL,'j'},
   {"pathseparator",no_argument_TS,NULL,'p'},
   {"quiet",no_argument_TS,NULL,'m'},
   {"list",no_argument_TS,NULL,'l'},
   {"input_encoding",required_argument_TS,NULL,'k'},
   {"output_encoding",required_argument_TS,NULL,'q'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};

int main_UnpackFile(int argc,char* const argv[])
{

if (argc==1) {
   usage();
   return 0;
}
char outputDir[FILENAME_MAX+0x20]="";
char selectFile[FILENAME_MAX+0x20]="";

Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
int val,index=-1;
int quiet=0;
int list=0;
int junk_path_in_pack_archive=0;
int transform_path_separator=0;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_UnpackFile,lopts_UnpackFile,&index,vars))) {
   switch(val) {

   case 'm': quiet=1; break;
   case 'l': list=1; break;
   case 'j': junk_path_in_pack_archive=1; break;
   case 'p': transform_path_separator=1; break;
   case 'f': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty selected file\n");
             }
             strcpy(selectFile,vars->optarg);
             break;
   case 'd': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output prefix\n");
             }
             strcpy(outputDir,vars->optarg);
             break;
   case 'k': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input_encoding argument\n");
             }
             decode_reading_encoding_parameter(&mask_encoding_compatibility_input,vars->optarg);
             break;
   case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&encoding_output,&bom_output,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_UnpackFile[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}




const char* ulpFile=argv[vars->optind];

if (ulpFile == NULL) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
if ((*ulpFile)=='\0') {
   fatal_error("Invalid arguments: rerun with --help\n");
}

int retValue = 0;
if (list != 0)
	retValue = do_list_file_in_pack_archive(ulpFile);
else
{
	if (selectFile[0] != '\0')
		retValue = do_extract_from_pack_archive_onefile(ulpFile,junk_path_in_pack_archive,outputDir,selectFile,transform_path_separator,quiet);
	else
		retValue = do_extract_from_pack_archive(ulpFile,junk_path_in_pack_archive,outputDir,transform_path_separator,quiet);
}

if (retValue != UNZ_OK)
{
	error("error in processing %s",ulpFile);
}

return retValue;
}



} // namespace unitex
//} // namespace logger

#endif







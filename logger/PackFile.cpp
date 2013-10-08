#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include "Unicode.h"
#include "Copyright.h"
#include "UnitexGetOpt.h"

#include "FilePackType.h"
#include "MzToolsUlp.h"
#include "PackFile.h"

#include "PackFileTool.h"

#ifndef NO_UNITEX_LOGGER

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/*
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {
*/

#ifdef HAS_LOGGER_NAMESPACE
using namespace ::unitex::logger;
#endif

extern const char* optstring_PackFile;
extern const struct option_TS lopts_PackFile[];
extern const char* usage_PackFile;

/*



int buildPackFile(const char* packFile,int append_status,const char* global_comment,
                  const char* file_or_prefix_to_add,int add_one_file_only,const char* junk_prefix,
				  int quiet);*/


const char* usage_PackFile =
         "Usage : PackFile [OPTIONS] <ulpfile>\n"
         "\n"
		 "  <ulpfile>: a an archive file to create\n"
         "\n"
         "OPTIONS:\n"
         "  -i X/--include=X: uses X as filename or prefix to include\n"
         "  -p/--prefix: mean include value is a prefix and not single filename\n"
         "  -a/--append: append file in existing archive\n"
         "  -m/--quiet: do not emit message when running\n"
		 "  -g X/--global=X: uses X as archive global comment (cosmetic)\n"
		 "  -j X/--junk_prefix=X: remove X at the beginning in the stored filename\n"
         "\n";

static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_PackFile);
}


const char* optstring_PackFile=":hi:pamg:j:k:q:";
const struct option_TS lopts_PackFile[]={
   {"include", required_argument_TS, NULL, 'i'},
   {"prefix",no_argument_TS,NULL,'p'},
   {"append",no_argument_TS,NULL,'a'},
   {"quiet",no_argument_TS,NULL,'m'},
   {"global",required_argument_TS,NULL,'g'},
   {"junk_prefix",required_argument_TS,NULL,'j'},
   {"input_encoding",required_argument_TS,NULL,'k'},
   {"output_encoding",required_argument_TS,NULL,'q'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};

int main_PackFile(int argc,char* const argv[])
{

if (argc==1) {
   usage();
   return 0;
}
char junk_prefix[FILENAME_MAX+0x20]="";
char include_filename[FILENAME_MAX+0x20]="";
char global_comment[FILENAME_MAX+0x20]="";

Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
int val,index=-1;
int quiet=0;
int add_one_file_only=1;
int append=0;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_PackFile,lopts_PackFile,&index,vars))) {
   switch(val) {

   case 'm': quiet=1; break;
   case 'p': add_one_file_only=0; break;
   case 'a': append=1; break;
   case 'i': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty include file name\n");
             }
             strcpy(include_filename,vars->optarg);
             break;
   case 'j': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty junk prefix file name\n");
             }
             strcpy(junk_prefix,vars->optarg);
             break;
   case 'g': if (vars->optarg[0]=='\0') {
                   fatal_error("You must specify a non empty global comment\n");
                }
                strcpy(global_comment,vars->optarg);
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
             else fatal_error("Missing argument for option --%s\n",lopts_PackFile[index].name);
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

int retValue = buildPackFile(ulpFile,append ,
	                          global_comment,
                 include_filename,add_one_file_only,  junk_prefix,
				    quiet);
if (retValue == 0)
{
	error("error in creating %s\n",ulpFile);
	return 1;
}
else
{
	return 0;
}

}

} // namespace unitex

#endif

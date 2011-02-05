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
#include "MzRepairUlp.h"

#ifndef NO_UNITEX_LOGGER


extern const char* optstring_MzRepairUlp;
extern const struct option_TS lopts_MzRepairUlp[];
extern const char* usage_MzRepairUlp;


const char* usage_MzRepairUlp =
         "Usage : MzRepairUlp [OPTIONS] <ulpfile>\n"
         "\n"
		 "  <ulpfile>: a corrupted ulp file (often, a crashing log)\n"
         "\n"
         "OPTIONS:\n"
         "  -o X/--output=X: uses X as filename for fixed .ulp file (<ulpfile>.repair by default)\n"
         "  -t X/--temp=X: uses X as filename for temporary file (<ulpfile>.build by default)\n"
         "  -m/--quiet: do not emit message when running\n"
         "  -v/--verbose: emit message when running\n"

         "\n";

static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_MzRepairUlp);
}


const char* optstring_MzRepairUlp=":hmvo:t:k:q:";
const struct option_TS lopts_MzRepairUlp[]={
   {"output", required_argument_TS, NULL, 'o'},
   {"temp",required_argument_TS,NULL,'t'},
   {"input_encoding",required_argument_TS,NULL,'k'},
   {"output_encoding",required_argument_TS,NULL,'q'},
   {"quiet",no_argument_TS,NULL,'m'},
   {"verbose",no_argument_TS,NULL,'v'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};

int main_MzRepairUlp(int argc,char* const argv[])
{

if (argc==1) {
   usage();
   return 0;
}
char outputFile[FILENAME_MAX+0x20]="";
char tempFile[FILENAME_MAX+0x20]="";

Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
int val,index=-1;
int quiet=0;
int verbose=0;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_MzRepairUlp,lopts_MzRepairUlp,&index,vars))) {
   switch(val) {

   case 'm': quiet=1; break;
   case 'v': verbose=1; break;
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(outputFile,vars->optarg);
             break;
   case 'd': if (vars->optarg[0]=='\0') {
                   fatal_error("You must specify a non empty temp file name\n");
                }
                strcpy(tempFile,vars->optarg);
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
             else fatal_error("Missing argument for option --%s\n",lopts_MzRepairUlp[index].name);
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


if (outputFile[0]=='\0')
{
	strcpy(outputFile,ulpFile);
	strcat(outputFile,".repair");
}

if (tempFile[0]=='\0')
{
	strcpy(tempFile,outputFile);
	strcat(tempFile,".build");
}


int retRepair=0;
uLong nRecovered=0;
uLong bytesRecovered=0;



retRepair=ulpRepair(ulpFile,(const char*)outputFile,(const char*)tempFile,&nRecovered,&bytesRecovered);
if ((retRepair!=0)) {
	if ((verbose==1) || (quiet == 0)) {
		u_printf("error in UlpRepair from %s to %s : return value = %d",ulpFile,outputFile,retRepair);
	}
}

if ((retRepair==0)) {
	if ((verbose==1) && (quiet == 0)) {
		u_printf("success in UlpRepair from %s to %s : return value = %d",ulpFile,outputFile,retRepair);
	}
}
return retRepair;
}

#endif

/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "UnitexRevisionInfo.h"
#include "UnitexGetOpt.h"


const char* usage_VersionInfo =
         "Usage: VersionInfo [OPTIONS] <outfile>\n"
         "\n"
         "  <outfile>: the destination file\n"
         "\n"
         "OPTIONS:\n"
         "  -o OUTFILE/--output=OUTFILE: path to output file to create\n"
		 "  -v/--version: display only version number\n"
		 "  -r/--revision: display only SVN revision number (if available)\n"
		 "  -h/--help: this help\n"
         "\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_VersionInfo);
}


const char* optstring_VersionInfo=":hrvo:k:q:";
const struct option_TS lopts_VersionInfo[]= {
      {"version",no_argument_TS,NULL,'v'},
      {"revision",no_argument_TS,NULL,'r'},
      {"output",required_argument_TS,NULL,'o'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


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
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_VersionInfo,lopts_VersionInfo,&index,vars))) {
   switch(val) {
   case 'r': do_revision_only=1; break;
   case 'v': do_version_only=1; break;
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

if ((do_revision_only!=0) && (do_version_only!=0)) {
   fatal_error("Incompatible arguments: rerun with --help\n");
}


unichar DisplayText[0x400];
int revision = get_unitex_revision();


if (do_revision_only) {
	u_sprintf(DisplayText,"%d",revision);
}
else if (do_version_only) {
	u_sprintf(DisplayText,"%d.%d",UNITEX_MAJOR_VERSION_NUMBER,UNITEX_MINOR_VERSION_NUMBER);
}
else {
	if (revision == -1) {
		u_sprintf(DisplayText,"%S\nUnitex Version: %d.%d\n",
				COPYRIGHT,UNITEX_MAJOR_VERSION_NUMBER,UNITEX_MINOR_VERSION_NUMBER);
	}
	else {
		u_sprintf(DisplayText,"%S\nUnitex Version: %d.%d\nUnitex SVN revision: %d\n",
				COPYRIGHT,UNITEX_MAJOR_VERSION_NUMBER,UNITEX_MINOR_VERSION_NUMBER,revision);
	}
}

if (output_file!=NULL) {
   U_FILE* text = u_fopen(&vec,output_file,U_WRITE);
   if (text==NULL) {
      error("Cannot create text file %s\n",output_file);
	  return 1;
   }
   u_fprintf(text,"%S", DisplayText);
   u_fclose(text);
}
else {
	u_printf("%S", DisplayText);
}


return 0;
}

/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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




#include "Copyright.h"
#include "Error.h"
#include "File.h"
#include "DuplicateFile.h"
#include "UnitexGetOpt.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_DuplicateFile =
         "Usage: DuplicateFile [OPTIONS] <outfile>\n"
         "\n"
         "  <outfile>: the destination file\n"
         "\n"
         "OPTIONS:\n"
         "-i INFILE/--input=INFILE: path to input file to read and copy\n"
         "-m INFILE/--move=INFILE: path to input file to move (rename)\n"
         "-d/--delete: to just delete the outfile\n"
         "\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_DuplicateFile);
}


const char* optstring_DuplicateFile=":di:m:k:q:";
const struct option_TS lopts_DuplicateFile[]= {
      {"delete",no_argument_TS,NULL,'d'},
      {"move",required_argument_TS,NULL,'m'},
      {"input",required_argument_TS,NULL,'i'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {NULL,no_argument_TS,NULL,0}
};


int main_DuplicateFile(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}



const char *input_file = NULL;
const char *output_file = NULL;
int do_delete=0;
int do_move=0;

int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_DuplicateFile,lopts_DuplicateFile,&index,vars))) {
   switch(val) {
   case 'd': do_delete=1; break;
   case 'i': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input argument\n");
             }
             input_file = vars->optarg; 
             break;
   case 'm': if (vars->optarg[0]=='\0') {
                fatal_error("Empty move argument\n");
             }
             input_file = vars->optarg; 
             do_move=1; 
             break;
   case 'h': usage(); free_OptVars(vars); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_DuplicateFile[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   case 'k':
   case 'q': /* ignore -k and -q parameter instead make error */
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

output_file = argv[vars->optind];

if ((input_file==NULL) && (do_delete==0)) {
   fatal_error("You must specify the input_file file\n");
}

if ((input_file!=NULL) && (do_delete==1)) {
   fatal_error("You cannot specify input_file when delete\n");
}
if (output_file==NULL) {
   fatal_error("You must specify the output_file file\n");
}

int result;
if (input_file != NULL) {
    if (do_move == 0) {
        u_printf("copy file %s to %s\n",input_file,output_file);
        /* af_copy return 0 if success, -1 with reading problem, 1 writing problem */
        result=af_copy(input_file,output_file);
    }
    else
    {
        u_printf("move file %s to %s\n",input_file,output_file);
        result=af_rename(input_file,output_file);
    }
}
else {
    u_printf("remove file %s\n",output_file);
    result=af_remove(output_file);
}
u_printf((result==0) ? "Done.\n" : "Unsucessfull.\n");
free_OptVars(vars);
return result;
}

} // namespace unitex

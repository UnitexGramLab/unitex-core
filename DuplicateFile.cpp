 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include "Copyright.h"
#include "Error.h"
#include "File.h"
#include "DuplicateFile.h"
#include "getopt.h"


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: DuplicateFile [OPTIONS] <outfile>\n"
         "\n"
         "  <outfile>: the destination file\n"
         "\n"
         "OPTIONS:\n"
         "-i/--input <infile>: path to input file to read\n"
         "\n");
}



int main_DuplicateFile(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":i:";
const struct option_TS lopts[]= {
      {"input",required_argument_TS,NULL,'i'},
      {NULL,no_argument_TS,NULL,0}
};

const char *input_file = NULL;
const char *output_file = NULL;

int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring,lopts,&index,vars))) {
   switch(val) {
   case 'i': input_file = vars->optarg; break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

output_file = argv[vars->optind];

if (input_file==NULL) {
   fatal_error("You must specify the input_file file\n");
}

if (output_file==NULL) {
   fatal_error("You must specify the output_file file\n");
}
u_printf("copy file %s to %s\n",input_file,output_file);
int result=af_copy(input_file,output_file);
u_printf((result==0) ? "Done.\n" : "Unsucessfull.\n");
return result;
}

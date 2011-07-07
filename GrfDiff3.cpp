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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Unicode.h"
#include "Copyright.h"
#include "Error.h"
#include "IOBuffer.h"
#include "GrfSvn_lib.h"
#include "Grf_lib.h"
#include "UnitexGetOpt.h"
#include "GrfDiff3.h"

const char* usage_GrfDiff3 =
         "Usage: GrfDiff3 <mine> <base> <other>\n"
         "\n"
         "  <mine>: my .grf file\n"
         "  <other>: the other .grf file that may be conflicting\n"
         "  <base>: the common ancestor .grf file\n"
         "\n"
         "OPTIONS:\n"
		 "--output X: saves the result, if any, in X instead of printing it on the output\n"
		 "--conflicts X: saves the description of the conflicts, if any, in X\n"
		 "--only-cosmetic: reports a conflict for any change that is not purely cosmetic\n"
		 "  -h/--help: this help\n"
         "\n"
         "Tries to merge <mine> and <other>. In case of success, the result is printed on the\n"
		 "standard output and 0 is returned. In case of unresolved conflicts, 1 is returned and\n"
		 "nothing is printed. 2 is returned in case of error.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_GrfDiff3);
}


/* Undocumented short options are those given by the svn client. They
 * are listed here just to be safely ignored by getopt */
const char* optstring_GrfDiff3=":hEmL:k:q:";
const struct option_TS lopts_GrfDiff3[]= {
      {"output",required_argument_TS,NULL,1},
      {"conflicts",required_argument_TS,NULL,2},
      {"only-cosmetic",no_argument_TS,NULL,3},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


/**
 * This is the customized diff3 program designed to merge grf files.
 */
int main_GrfDiff3(int argc,char* const argv[]) {
if (argc==1) {
	usage();
	return 0;
}
VersatileEncodingConfig vec={DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT,DEFAULT_ENCODING_OUTPUT,DEFAULT_BOM_OUTPUT};
struct OptVars* vars=new_OptVars();
int val,index=-1;
char output[FILENAME_MAX]="";
char conflicts[FILENAME_MAX]="";
int only_cosmetics=0;
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_GrfDiff3,lopts_GrfDiff3,&index,vars))) {
   switch(val) {
   case 'h': usage(); return 0;
   case 1: {
	   strcpy(output,vars->optarg);
	   break;
   }
   case 2: {
	   strcpy(conflicts,vars->optarg);
	   break;
   }
   case 3: only_cosmetics=1; break;
   case 'k': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input_encoding argument\n");
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),vars->optarg);
             break;
   case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),vars->optarg);
             break;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_GrfDiff3[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}
if (vars->optind!=argc-3) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
U_FILE* f=U_STDOUT;
if (output[0]!='\0') {
	f=u_fopen(&vec,output,U_WRITE);
	if (f==NULL) {
		error("Cannot create file %s\n",output);
		free_OptVars(vars);
		return 2;
	}
}
U_FILE* f_conflicts=NULL;
if (conflicts[0]!='\0') {
	/* There is no point in encoding the conflict file in UTF16 */
	f_conflicts=u_fopen(UTF8,conflicts,U_WRITE);
	if (f_conflicts==NULL) {
		error("Cannot create file %s\n",conflicts);
		free_OptVars(vars);
		return 2;
	}
}
Grf* mine=load_Grf(&vec,argv[vars->optind]);
if (mine==NULL) {
	free_OptVars(vars);
	return 2;
}
Grf* base=load_Grf(&vec,argv[vars->optind+1]);
if (base==NULL) {
	free_Grf(mine);
	free_OptVars(vars);
	return 2;
}
Grf* other=load_Grf(&vec,argv[vars->optind+2]);
if (other==NULL) {
	free_Grf(mine);
	free_Grf(base);
	free_OptVars(vars);
	return 2;
}
free_OptVars(vars);
int res=diff3(f,f_conflicts,mine,base,other,only_cosmetics);
if (f!=U_STDOUT) {
	u_fclose(f);
	if (res!=0) {
		/* If the diff3 failed, we must remove the file */
		af_remove(output);
	}
}
if (f_conflicts!=NULL) {
	u_fclose(f_conflicts);
	if (res==0) {
		/* If the diff3 succeeded, we must remove the file */
		af_remove(conflicts);
	}
}
free_Grf(mine);
free_Grf(base);
free_Grf(other);
return res;
}

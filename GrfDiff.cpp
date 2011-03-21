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
#include "Grf_lib.h"
#include "GrfSvn_lib.h"
#include "UnitexGetOpt.h"
#include "GrfDiff.h"


const char* usage_GrfDiff =
         "Usage: GrfDiff <grf1> <grf2>\n"
         "\n"
         "  <grf1> <grf2>: .grf files to be compared\n"
         "\n"
         "OPTIONS:\n"
		 "--output X: saves the result, if any, in X instead of printing it on the output\n"
         "  -h/--help: this help\n"
         "\n"
         "Compares the given grf files and prints their difference on the standard output.\n"
		 "Returns 0 if they are identical modulo box and transition reordering, 1 if there\n"
		 "are differences, 2 in case of error.\n"
		 "\n"
		 "Here are the diff indications that can be emitted:\n"
		 "P name: a presentation property has changed. name=property name (SIZE, FONT, ...)\n"
		 "M a b:  box moved. a=box number in <grf1>, b=box number in <grf2>\n"
		 "C a b:  box content changed. a=box number in <grf1>, b=box number in <grf2>\n"
		 "A x:    box added. x=box number in <grf2>\n"
		 "R x:    box removed. x=box number in <grf1>\n"
		 "T a b x y: transition added. a,b=src and dst box numbers in <grf1>. x,y=src and dst box numbers in <grf2>\n"
		 "X a b x y: transition removed. a,b=src and dst box numbers in <grf1>. x,y=src and dst box numbers in <grf2>\n"
		 "\n"
		 "Note that transition modifications related to boxes that have been added or removed are not reported.\n";



static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_GrfDiff);
}


/* Undocumented short options are those given by the svn client. They
 * are listed here just to be safely ignored by getopt */
const char* optstring_GrfDiff=":huL:";
const struct option_TS lopts_GrfDiff[] = {
      {"output",required_argument_TS,NULL,1},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


/**
 * This is the customized diff program designed to compare grf files.
 */
int main_GrfDiff(int argc,char* const argv[]) {
if (argc==1) {
	usage();
	return 0;
}
struct OptVars* vars=new_OptVars();
int val,index=-1;
char output[FILENAME_MAX]="";
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_GrfDiff,lopts_GrfDiff,&index,vars))) {
   switch(val) {
   case 'h': usage(); return 0;
   case 1: {
	   strcpy(output,vars->optarg);
	   break;
   }
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_GrfDiff[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}
if (vars->optind!=argc-2) {
	   fatal_error("Invalid arguments: rerun with --help\n");
}
U_FILE* f=U_STDOUT;
if (output[0]!='\0') {
	f=u_fopen_creating_versatile_encoding(UTF8,0,output,U_WRITE);
	if (f==NULL) {
		error("Cannot create file %s\n",output);
		free_OptVars(vars);
		return 2;
	}
}
Grf* a=load_Grf(argv[vars->optind]);
if (a==NULL) {
	free_OptVars(vars);
	return 2;
}
Grf* b=load_Grf(argv[vars->optind+1]);
if (b==NULL) {
	free_Grf(a);
	free_OptVars(vars);
	return 2;
}
free_OptVars(vars);
GrfDiff* diff=grf_diff(a,b);
free_Grf(a);
free_Grf(b);
print_diff(f,diff);
if (f!=U_STDOUT) {
	u_fclose(f);
}
int different=diff->diff_ops->nbelems!=0;
free_GrfDiff(diff);
return different;
}





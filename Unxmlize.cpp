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
#include <stdlib.h>
#include <string.h>
#include "File.h"
#include "Error.h"
#include "Unicode.h"
#include "UnitexGetOpt.h"
#include "Unxmlize.h"
#include "Xml.h"
#include "Copyright.h"
#include "Offsets.h"


const char* usage_Unxmlize =
         "Usage: Unxmlize <xml>\n"
         "\n"
         "  <xml>: an unicode .xml file to be processed\n"
         "\n"
         "OPTIONS:\n"
         "  -o TXT/--output=TXT: output file. By default, foo.xml => foo.txt\n"
		 "  --output_offsets=XXX: specifies the offset file to be produced\n"
         "  -h/--help: this help\n"
         "\n"
         "Removes all xml tags from the given .xml file to produce a text file that\n"
		 "can be processed by Unitex.\n";

static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Unxmlize);
}


const char* optstring_Unxmlize=":o:h:k:q:";
const struct option_TS lopts_Unxmlize[]={
   {"output", required_argument_TS, NULL, 'o'},
   {"output_offsets",required_argument_TS,NULL,1},
   {"input_encoding",required_argument_TS,NULL,'k'},
   {"output_encoding",required_argument_TS,NULL,'q'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};


int main_Unxmlize(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

char output[FILENAME_MAX]="";
char output_offsets[FILENAME_MAX]="";
const VersatileEncodingConfig vec={DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT,DEFAULT_ENCODING_OUTPUT,DEFAULT_BOM_OUTPUT};
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Unxmlize,lopts_Unxmlize,&index,vars))) {
   switch(val) {
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(output,vars->optarg);
             break;
   case 1: if (vars->optarg[0]=='\0') {
                   fatal_error("You must specify a non empty offset file name\n");
                }
                strcpy(output_offsets,vars->optarg);
                break;
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
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Unxmlize[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
U_FILE* f_input;
U_FILE* f_output;
U_FILE* f_offsets=NULL;
vector_offset* offsets=NULL;
if (output[0]=='\0') {
    remove_extension(argv[vars->optind],output);
    strcat(output,".txt");
}

f_input=u_fopen(&vec,argv[vars->optind],U_READ);
if (f_input==NULL) {
	error("Cannot open file %s\n",argv[vars->optind]);
	return 1;
}
int html=0;
char extension[FILENAME_MAX];
get_extension(argv[vars->optind],extension);
if (!strcmp(extension,".html") || !strcmp(extension,".HTML")) {
	html=1;
}
f_output = u_fopen(&vec,output,U_WRITE);
if (f_output==NULL) {
   error("Cannot create text file %s\n",output);
   u_fclose(f_input);
   return 1;
}

if (output_offsets[0]!='\0') {
	f_offsets=u_fopen(&vec,output_offsets,U_WRITE);
	if (f_offsets==NULL) {
	   error("Cannot create offset file %s\n",output_offsets);
	   u_fclose(f_input);
	   u_fclose(f_output);
	   return 1;
	}
	offsets=new_vector_offset();
}
if (!unxmlize(f_input,f_output,offsets,html)) {
	error("The input file was not a valid xml one. Operation aborted.\n");
	u_fclose(f_input);
	u_fclose(f_output);
	u_fclose(f_offsets);
	af_remove(output);
	if (f_output!=NULL) {
		af_remove(output_offsets);
	}
	free_vector_offset(offsets);
	free_OptVars(vars);
	return 1;
}
if (offsets!=NULL) {
	process_offsets(NULL,offsets,f_offsets);
}
u_fclose(f_input);
u_fclose(f_output);
u_fclose(f_offsets);
free_vector_offset(offsets);
free_OptVars(vars);
u_printf("\nDone.\n");
return 0;
}


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
#include "Unicode.h"
#include "File.h"
#include "Copyright.h"
#include "String_hash.h"
#include "Error.h"
#include "NormalizeAsRoutine.h"
#include "UnitexGetOpt.h"
#include "Normalize.h"


const char* usage_Normalize =
         "Usage: Normalize [OPTIONS] <text>\n"
         "\n"
         "  <text>: text file to be normalized\n"
         "\n"
         "OPTIONS:\n"
         "  -n/--no_carriage_return: every separator sequence will be turned into a single space\n"
		 "  --output_offsets=XXX: specifies the offset file to be produced\n"
		 "  --no_separator_normalization: only applies replacement rules specified with -r\n"
         "  -r XXX/--replacement_rules=XXX: specifies a configuration file XXX that contains\n"
         "                                  replacement instructions in the form of lines like:\n"
         "\n"
         "                                     input_sequence TABULATION output_sequence\n"
         "\n"
         "                                  By default, the program only replaces { and } by [ and ]\n"
         "  -h/--help: this help\n"
         "\n"
         "Turns every sequence of separator chars (space, tab, new line) into one.\n"
         "If a separator sequence contains a new line char, it is turned to a single new\n"
         "line (except with --no_carriage_return); if not, it is turned into a single space. As\n"
         "a side effect, new line sequences are converted into the Windows style: \\r\\n.\n"
         "If you specifies replacement rules with -f, they will be applied prior\n"
         "to the separator normalization, so you have to take care if you manipulate\n"
         "separators in your replacement rules.\n"
         "The result is stored in a file with the same name as <text>, but with .snt extension.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Normalize);
}


const char* optstring_Normalize=":nr:hk:q:@:";
const struct option_TS lopts_Normalize[]= {
      {"no_carriage_return",no_argument_TS,NULL,'n'},
      {"replacement_rules",required_argument_TS,NULL,'r'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"output_offsets",required_argument_TS,NULL,'@'},
      {"no_separator_normalization",no_argument_TS,NULL,1},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_Normalize(int argc,char* const argv[]) {
if (argc==1) {
	usage();
	return 0;
}
int mode=KEEP_CARRIAGE_RETURN;
int separator_normalization=1;
char rules[FILENAME_MAX]="";
char offsets[FILENAME_MAX]="";
Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Normalize,lopts_Normalize,&index,vars))) {
   switch(val) {
   case 'n': mode=REMOVE_CARRIAGE_RETURN; break;
   case 'r': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty replacement rule file name\n");
             }
             strcpy(rules,vars->optarg);
             break;
   case 1: separator_normalization=0; break;
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
   case '@': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty offset file name\n");
             }
             strcpy(offsets,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Normalize[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
char tmp_file[FILENAME_MAX];
get_extension(argv[vars->optind],tmp_file);
if (!strcmp(tmp_file, ".snt")) {
   /* If the file to process has allready the .snt extension, we temporary rename it to
	 * .snt.normalizing */
	strcpy(tmp_file,argv[vars->optind]);
	strcat(tmp_file,".normalizing");
	af_rename(argv[vars->optind],tmp_file);
} else {
   strcpy(tmp_file,argv[vars->optind]);
}
/* We set the destination file */
char dest_file[FILENAME_MAX];
remove_extension(argv[vars->optind],dest_file);
strcat(dest_file,".snt");
u_printf("Normalizing %s...\n",argv[vars->optind]);
int result=normalize(tmp_file, dest_file, encoding_output,bom_output,
		mask_encoding_compatibility_input,mode, rules,offsets,separator_normalization);
u_printf("\n");
/* If we have used a temporary file, we delete it */
if (strcmp(tmp_file,argv[vars->optind])) {
   af_remove(tmp_file);
}
free_OptVars(vars);
u_printf((result==0) ? "Done.\n" : "Unsucessfull.\n");
return result;
}




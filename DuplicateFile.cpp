/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "DirHelper.h"

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
         "-a/--make-dir: to create empty directory named outfile\n"
         "-p/--parents: to create empty directory named outfile, creating parent if needed\n"
         "-r/--recursive-delete: to just delete the outfile folder\n"
         "\n";


static void usage() {
display_copyright_notice();
u_printf(usage_DuplicateFile);
}


const char* optstring_DuplicateFile=":aprdi:m:k:q:";
const struct option_TS lopts_DuplicateFile[]= {
      {"delete",no_argument_TS,NULL,'d'},
      {"recursive-delete",no_argument_TS,NULL,'r'},
      {"make-dir", no_argument_TS, NULL,'a'},
      {"parents", no_argument_TS, NULL,'p'},
      {"move",required_argument_TS,NULL,'m'},
      {"input",required_argument_TS,NULL,'i'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {NULL,no_argument_TS,NULL,0}
};


static int mkDirRecursiveIfNeeded(const char* dir_name)
{
	int res_mk = mkDirPortable(dir_name);
	if (res_mk == 0)
		return 0;

	int len_dir = (int)strlen(dir_name);
	int last_separator = -1;
	for (int i = 0;i < len_dir;i++) {
		if ((*(dir_name + i) == '\\') || (*(dir_name + i) == '/'))
			last_separator = i;
	}

	if (last_separator == -1)
		return res_mk;

	char* up_dir_name = (char*)malloc(last_separator + 1);
	memcpy(up_dir_name, dir_name, last_separator);
	*(up_dir_name + last_separator) = '\0';
	mkDirRecursiveIfNeeded(up_dir_name);
	free(up_dir_name);

	return mkDirPortable(dir_name);
}


int main_DuplicateFile(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}



const char *input_file = NULL;
const char *output_file = NULL;
int do_delete=0;
int do_recursive_delete=0;
int do_move=0;
int do_make_dir=0;
int do_make_dir_parent=0;
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_DuplicateFile,lopts_DuplicateFile,&index,vars))) {
   switch(val) {
   case 'a': do_make_dir = 1; break;
   case 'p': do_make_dir_parent = 1; break;
   case 'd': do_delete = 1; break;
   case 'r': do_delete = do_recursive_delete = 1; break;
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

if ((input_file==NULL) && (do_delete==0) && (do_make_dir==0) && (do_make_dir_parent ==0)) {
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
    } else {
        u_printf("move file %s to %s\n",input_file,output_file);
        result=af_rename(input_file,output_file);
    }
}
else
if (do_make_dir != 0) {
    u_printf("make dir %s\n", output_file);
    result = mkDirPortable(output_file);
}
else
if (do_make_dir_parent != 0) {
	u_printf("make dir %s with parent\n", output_file);
	result = mkDirRecursiveIfNeeded(output_file);
}
else {
    if (do_recursive_delete == 0) {
        u_printf("remove file %s\n",output_file);
        result=af_remove(output_file);
    } else {
        u_printf("remove folder %s\n", output_file);
        af_remove_folder(output_file);
        result=0;
    }
}
u_printf((result==0) ? "Done.\n" : "Unsucessfull.\n");
free_OptVars(vars);
return result;
}

} // namespace unitex

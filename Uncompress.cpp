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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "DELA.h"
#include "File.h"
#include "Copyright.h"
#include "Error.h"
#include "getopt.h"


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Uncompress [OPTIONS] <dictionary>\n"
         "\n"
         "  <dictionary>: a .bin dictionary\n"
         "\n"
         "OPTIONS:\n"
         "  -o OUT/--output=OUT: special the output file. By default, it is\n"
         "                       'foo.dic' where 'foo.bin' is the input file.\n"
         "  -h/--help: this help\n"
         "\n"
         "Uncompresses a binary dictionary into a text one.\n\n");
}


int main_Uncompress(int argc, char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

int FLIP=0;
const char* optstring=":o:h";
const struct option_TS lopts[]= {
      {"output",required_argument_TS,NULL,'o'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};
int val,index=-1;
char output[FILENAME_MAX]="";
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring,lopts,&index,vars))) {
   switch(val) {
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(output,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}
if (output[0]=='\0') {
   remove_extension(argv[vars->optind],output);
   strcat(output,".dic");
}
U_FILE* f=u_fopen(UTF16_LE,output,U_WRITE);
if (f==NULL) {
   fatal_error("Cannot open file %s\n",output);
}
char inf_file[FILENAME_MAX];
remove_extension(argv[vars->optind],inf_file);
strcat(inf_file,".inf");

unsigned char* bin=load_BIN_file(argv[vars->optind]);
if (bin==NULL) {
   fatal_error("Problem with file %s\n",argv[vars->optind]);
}
struct INF_codes* inf=load_INF_file(inf_file);
if (inf==NULL) {
   fatal_error("Problem with file %s\n",inf);
}
rebuild_dictionary(bin,inf,f);
u_fclose(f);
free(bin);
free_INF_codes(inf);
u_printf("Done.\n");
return 0;
}


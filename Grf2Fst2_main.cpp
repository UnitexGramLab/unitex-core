 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Fst2.h"
#include "GrfCheck.h"
#include "Copyright.h"
#include "Grf2Fst2_lib.h"
#include "Alphabet.h"
#include "File.h"
#include "LocateConstants.h"
#include "Error.h"
#include "Grf2Fst2_main.h"
#include "getopt.h"


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage : Grf2Fst2 [OPTIONS] <grf>\n"
         "   <grf> : main graph of grammar (must be an absolute path)\n"
         "OPTIONS:\n"      
         " -y/--loop_check: enables the loops/left-recursion detection\n"
         " -n/--no_loop_check: disables the loops/left-recursion detection (default)\n"
         " -e/--no_empty_graph_warning: no warning will be emitted when a graph matches <E>\n"
         " -a ALPH/--alphabet=ALPH: name of the alphabet file to use for tokenizing\n"
         "                          lexical units.\n"
         " -c/--char_by_char: lexical units are single letters. If both -a and -c options are\n"
         "                    unused, lexical units will be sequences of any unicode letters.\n"
         " -d DIR/--pkgdir=DIR: path of the root dir of all grammar packages\n"
         " -h/--help: this help\n"
         "\n"
         "Compiles the grammar <grf> and saves the result in a FST2 file\n"
         "stored in the same directory as <grf>.\n");
}


/**
 * A convenient way to call the main function within a Unitex program.
 */
int pseudo_main_Grf2Fst2(char* name,int yes_or_no,char* alphabet,int no_empty_graph_warning) {
int argc=0;
char** argv=(char**)malloc((4+(no_empty_graph_warning!=0))*sizeof(char*));
if (argv==NULL) {
   fatal_error("Not enough memory in main_Grf2Fst2\n");
}
argv[argc++]=NULL;
argv[argc++]=strdup(name);
argv[argc++]=strdup(yes_or_no?"-y":"-n");
char tmp[FILENAME_MAX];
if (alphabet) {
   sprintf(tmp,"-a=%s",alphabet);
   argv[argc++]=tmp;
}
else argv[3]=NULL;
if (no_empty_graph_warning) {
   argv[argc++]=strdup("-e");
} else {
   argv[argc++]=NULL;
}
int ret=main_Grf2Fst2(argc,argv);
free(argv[1]);
free(argv[2]);
free(argv);
return ret;
}


/**
 * The same than main, but no call to setBufferMode.
 */
int main_Grf2Fst2(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}
struct compilation_info* infos=new_compilation_info();
int check_recursion=0;
const char* optstring=":yna:d:ech";
const struct option lopts[]= {
      {"loop_check",no_argument,NULL,'y'},
      {"no_loop_check",no_argument,NULL,'n'},
      {"alphabet",required_argument,NULL,'a'},
      {"pkgdir",required_argument,NULL,'d'},
      {"no_empty_graph_warning",no_argument,NULL,'e'},
      {"char_by_char",no_argument,NULL,'c'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int val,index=-1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'y': check_recursion=1; break;
   case 'n': check_recursion=0; break;
   case 'e': infos->no_empty_graph_warning=1; break;
   case 'c': infos->tokenization_policy=CHAR_BY_CHAR_TOKENIZATION; break;
   case 'a': infos->tokenization_policy=WORD_BY_WORD_TOKENIZATION;
             if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file\n");
             }
             infos->alphabet=load_alphabet(optarg);
             if (infos->alphabet==NULL) {
                fatal_error("Cannot load alphabet file %s\n",optarg);
             }
             break;
   case 'd': strcpy(infos->repository,optarg); break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",optopt); 
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",optopt); 
             else fatal_error("Invalid option --%s\n",optarg);
             break;
   }
   index=-1;
}
   
if (optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}


char fst2_file_name[FILENAME_MAX];
remove_extension(argv[optind],fst2_file_name);
strcat(fst2_file_name,".fst2");
if ((infos->fst2=u_fopen(fst2_file_name,U_WRITE))==NULL) {
   error("Cannot open file %s\n",fst2_file_name);
   return 1;
}
u_fprintf(infos->fst2,"0000000000\n");
int result=compile_grf(argv[optind],infos);
if (result==0) {
   error("Compilation has failed\n");
   free_compilation_info(infos);
   u_fclose(infos->fst2);
   return 1;
}
free_alphabet(infos->alphabet);
write_tags(infos->fst2,infos->tags);
u_fclose(infos->fst2);
write_number_of_graphs(fst2_file_name,infos->graph_names->size-1);
if (check_recursion) {
   if (!grf_OK(fst2_file_name,infos->no_empty_graph_warning)) {
      return 1;
   }
}
free_compilation_info(infos);
u_printf("Compilation has succeeded\n");
return 0;
}



/**
 * The same than main, but no call to setBufferMode.
 */
int main_Grf2Fst2_old(int argc,char* argv[]) {
if(argc<2 || argc>6) {
   usage();
   return 0;
}
struct compilation_info* infos=new_compilation_info();
if(argc>=4) {
  /* If the penultimate arg is "-d" the library package path is given,
     [y/n] and [ALPH] are not obligatory! */
  if(!strcmp(argv[argc-2],"-d")){      
    strcpy(infos->repository,argv[argc-1]);
    argc -= 2;
  }
}
int check_recursion=0;
int index=0;
if (argc>=3) {
   if (!strcmp(argv[2],"y") || !strcmp(argv[2],"n")) {
      if (!strcmp(argv[2],"y")) check_recursion=1;
      if (argc==4) {index=3;}
   }
   else {
      if (argc==3) {
         index=2;
      } else {
         error("Extra parameter: %s\n",argv[2]);
         return 1;
      }
  }
}
if (index!=0) {
   if (!strcmp(argv[index],"char_by_char")) {
      infos->tokenization_policy=CHAR_BY_CHAR_TOKENIZATION;
   } else {
      infos->alphabet=load_alphabet(argv[index]);
      if (infos->alphabet==NULL) {
         error("Cannot load alphabet file %s\n",argv[index]);
         return 1;
      }
      infos->tokenization_policy=WORD_BY_WORD_TOKENIZATION;
   }
}
char fst2_file_name[FILENAME_MAX];
remove_extension(argv[1],fst2_file_name);
strcat(fst2_file_name,".fst2");
if ((infos->fst2=u_fopen(fst2_file_name,U_WRITE))==NULL) {
   error("Cannot open file %s\n",fst2_file_name);
   return 1;
}
u_fprintf(infos->fst2,"0000000000\n");
int result=compile_grf(argv[1],infos);
if (result==0) {
   error("Compilation has failed\n");
   free_compilation_info(infos);
   u_fclose(infos->fst2);
   return 1;
}
free_alphabet(infos->alphabet);
write_tags(infos->fst2,infos->tags);
u_fclose(infos->fst2);
write_number_of_graphs(fst2_file_name,infos->graph_names->size-1);
if (check_recursion) {
   if (!grf_OK(fst2_file_name,infos->no_empty_graph_warning)) {
      return 1;
   }
}
free_compilation_info(infos);
u_printf("Compilation has succeeded\n");
return 0;
}

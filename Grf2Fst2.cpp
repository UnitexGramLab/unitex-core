/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Fst2Check_lib.h"
#include "Copyright.h"
#include "Grf2Fst2_lib.h"
#include "Alphabet.h"
#include "File.h"
#include "LocateConstants.h"
#include "Error.h"
#include "Grf2Fst2.h"
#include "UnitexGetOpt.h"
#include "ProgramInvoker.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Grf2Fst2 =
         "Usage : Grf2Fst2 [OPTIONS] <grf>\n"
         "\n"
         "  <grf>: main graph of grammar (must be an absolute path)\n"
         "\n"
         "OPTIONS:\n"
         "  -o FST2/--output=FST2: name of destination file\n"
         "  -y/--loop_check: enables the loops/left-recursion detection\n"
         "  -n/--no_loop_check: disables the loops/left-recursion detection (default)\n"
         "  -t/--tfst_check: checks if the given .grf can be considered as a valid sentence\n"
         "                   automaton\n"
         "  -s/--silent_grf_name: do not display the list of .grf files\n"
         "  -e/--no_empty_graph_warning: no warning will be emitted when a graph matches <E>\n"
         "  -a ALPH/--alphabet=ALPH: name of the alphabet file to use for tokenizing\n"
         "                           lexical units.\n"
         "  -c/--char_by_char: lexical units are single letters. If both -a and -c options are\n"
         "                     unused, lexical units will be sequences of any unicode letters.\n"
         "  -d DIR/--pkgdir=DIR: path of the default graph repository\n"
         "  -r XXX/--named_repositories=XXX: declaration of named repositories. XXX is\n"
		 "                                   made of one or more X=Y sequences, separated by ;\n"
		 "                                   where X is the name of the repository denoted by\n"
		 "                                   the pathname Y. You can use this option several times\n"
         "  --debug: compile graphs in debug mode\n"
		 "  -v/--check_variables: checks output validity to avoid malformed variable expressions\n"
		 "  -S/--strict_tokenization: spaces and # will be inserted to force box line tokenization\n"
		 "                            to produce the exact sequence the used typed. For instance,\n"
		 "                            'let pi=3.14' will be tokenized as:\n"
		 "                            'let' space 'pi' # '=' # '3' # '.' # '1' # '4'\n"
         "  -h/--help: this help\n"
         "\n"
         "Compiles the grammar <grf> and saves the result in a FST2 file\n"
         "stored in the same directory as <grf>.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Grf2Fst2);
}


/**
 * A convenient way to call the main function within a Unitex program.
 */
int pseudo_main_Grf2Fst2(const VersatileEncodingConfig* vec,
                         const char* name,int yes_or_no,const char* alphabet,
                         int no_empty_graph_warning,int tfst_check,
                         const char* pkgdir,const char* named_repositories,
                         int strict_tokenization) {
ProgramInvoker* invoker=new_ProgramInvoker(main_Grf2Fst2,"main_Grf2Fst2");
add_argument(invoker,name);
add_argument(invoker,yes_or_no?"-y":"-n");
char tmp[FILENAME_MAX];
{
    tmp[0]=0;
    get_reading_encoding_text(tmp,sizeof(tmp)-1,vec->mask_encoding_compatibility_input);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-k");
        add_argument(invoker,tmp);
    }

    tmp[0]=0;
    get_writing_encoding_text(tmp,sizeof(tmp)-1,vec->encoding_output,vec->bom_output);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-q");
        add_argument(invoker,tmp);
    }
}
if (alphabet!=NULL) {
   sprintf(tmp,"-a=%s",alphabet);
   add_argument(invoker,tmp);
}
if (no_empty_graph_warning) {
   add_argument(invoker,"-e");
}
if (tfst_check) {
   add_argument(invoker,"-t");
}
if (pkgdir!=NULL && pkgdir[0]!='\0') {
   sprintf(tmp,"--pkgdir=%s",pkgdir);
   add_argument(invoker,tmp);
}
if (named_repositories!=NULL && named_repositories[0]!='\0') {
   sprintf(tmp,"--named_repositories=%s",named_repositories);
   add_argument(invoker,tmp);
}
if (strict_tokenization) {
   add_argument(invoker,"-S");
}
int ret=invoke(invoker);
free_ProgramInvoker(invoker);
return ret;
}


const char* optstring_Grf2Fst2=":yntsa:d:echo:k:q:r:vS";
const struct option_TS lopts_Grf2Fst2[]= {
      {"loop_check",no_argument_TS,NULL,'y'},
      {"no_loop_check",no_argument_TS,NULL,'n'},
      {"tfst_check",no_argument_TS,NULL,'t'},
      {"alphabet",required_argument_TS,NULL,'a'},
      {"pkgdir",required_argument_TS,NULL,'d'},
      {"no_empty_graph_warning",no_argument_TS,NULL,'e'},
      {"silent_grf_name",no_argument_TS,NULL,'s'},
      {"char_by_char",no_argument_TS,NULL,'c'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"output",required_argument_TS,NULL,'o'},
      {"debug",no_argument_TS,NULL,1},
      {"help",no_argument_TS,NULL,'h'},
      {"named_repositories",required_argument_TS,NULL,'r'},
      {"check_variables",no_argument_TS,NULL,'v'},
      {"strict_tokenization",no_argument_TS,NULL,'S'},
      {NULL,no_argument_TS,NULL,0}
};


static int count_semi_colons(const char* s) {
int n=0;
for (int i=0;s[i]!='\0';i++) {
   if (s[i]==';') n++;
}
return n;
}


/**
 * Takes a string containing .bin names separated with semi-colons and
 * loads the corresponding dictionaries.
 */
static void deal_with_named_repositories(const char* s,struct string_hash_ptr* hash) {
const char* src=s;
if (s==NULL || s[0]=='\0') {
   return;
}
int n_morpho_dics=1+count_semi_colons(s);
unichar name[FILENAME_MAX];
char value[FILENAME_MAX];
int pos;
for (int i=0;i<n_morpho_dics;i++) {
   pos=0;
   while (*s!='\0' && *s!='=') {
      name[pos++]=*s;
      s++;
   }
   name[pos]='\0';
   if (*s!='=') {
	   fatal_error("Invalid named_repositories: %s\n",src);
   }
   s++;
   pos=0;
   while (*s!='\0' && *s!=';') {
      value[pos++]=*s;
      s++;
   }
   value[pos]='\0';
   if (*s==';') {
      s++;
   }
   get_value_index(name,hash,INSERT_IF_NEEDED,strdup(value));
}
}


/**
 * The same than main, but no call to setBufferMode.
 */
int main_Grf2Fst2(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}
struct compilation_info* infos=new_compilation_info();
int check_recursion=0,tfst_check=0;

infos->vec.mask_encoding_compatibility_input=DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
infos->vec.encoding_output=DEFAULT_ENCODING_OUTPUT;
infos->vec.bom_output=DEFAULT_BOM_OUTPUT;

char* named=NULL;
char fst2_file_name[FILENAME_MAX];
infos->verbose_name_grf=1;
char alph[FILENAME_MAX]="";
int val,index=-1;
struct OptVars* vars=new_OptVars();
fst2_file_name[0]='\0';
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Grf2Fst2,lopts_Grf2Fst2,&index,vars))) {
   switch(val) {
   case 'y': check_recursion=1; break;
   case 'n': check_recursion=0; break;
   case 't': tfst_check=1;
             /* If we have a tfst sentence graph, we must not report
              * compilation failure in the case of an empty graph. It
              * may be because of a sentence graph previously emptied by ELAG */
             infos->no_empty_graph_warning=1;
             break;
   case 's': infos->verbose_name_grf=0; break;
   case 'e': infos->no_empty_graph_warning=1; break;
   case 'c': infos->tokenization_policy=CHAR_BY_CHAR_TOKENIZATION; break;
   case 'a': infos->tokenization_policy=WORD_BY_WORD_TOKENIZATION;
			   strcpy(alph,vars->optarg);
             break;
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output\n");
             }
             strcpy(fst2_file_name,vars->optarg);
             break;
   case 'd': strcpy(infos->repository,vars->optarg); break;
   case 1: infos->debug=1; break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Grf2Fst2[index].name);
   case 'k': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input_encoding argument\n");
             }
             decode_reading_encoding_parameter(&(infos->vec.mask_encoding_compatibility_input),vars->optarg);
             break;
   case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&(infos->vec.encoding_output),&(infos->vec.bom_output),vars->optarg);
             break;
   case 'r': if (named==NULL) {
                  named=strdup(vars->optarg);
                  if (named==NULL) {
                     fatal_alloc_error("main_Grf2Fst2");
                  }
             } else {
            	 named = (char*)realloc((void*)named,strlen(named)+strlen(vars->optarg)+2);
                 if (named==NULL) {
                    fatal_alloc_error("main_Grf2Fst2");
                 }
                 strcat(named,";");
                 strcat(named,vars->optarg);
             }
             break;
   case 'v': infos->check_outputs=1; break;
   case 'S': infos->strict_tokenization=1; break;
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}
deal_with_named_repositories(named,infos->named_repositories);
free(named);
if (alph[0]!='\0') {
	infos->alphabet=load_alphabet(&(infos->vec),alph);
	if (infos->alphabet==NULL) {
		fatal_error("Cannot load alphabet file %s\n",alph);
	}
}
if (vars->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}
if (fst2_file_name[0]=='\0') {
	strcpy(fst2_file_name,argv[vars->optind]);
}
remove_extension(fst2_file_name);
strcat(fst2_file_name,".fst2");
if ((infos->fst2=u_fopen(&(infos->vec),fst2_file_name,U_WRITE))==NULL) {
   error("Cannot open file %s\n",fst2_file_name);
   return 1;
}
u_fprintf(infos->fst2,"0000000000\n");
int result=compile_grf(argv[vars->optind],infos);
if (result==0) {
   error("Compilation has failed\n");
   free_compilation_info(infos);
   u_fclose(infos->fst2);
   return 1;
}
free_alphabet(infos->alphabet);
write_tags(infos->fst2,infos->tags);
u_fclose(infos->fst2);
free_OptVars(vars);
write_number_of_graphs(&(infos->vec),fst2_file_name,infos->current_saved_graph,infos->debug);
/* Now, we may have to renumber graph calls */
if (infos->renumber->tab[infos->graph_names->size-1]!=infos->graph_names->size) {
	Fst2* fst2=load_fst2(&(infos->vec),fst2_file_name,1);
	renumber_graph_calls(fst2,infos->renumber,infos->part_of_precompiled_fst2);
	save_Fst2(&(infos->vec),fst2_file_name,fst2);
	free_Fst2(fst2);
}

if (check_recursion) {
   if (!OK_for_Locate(&(infos->vec),fst2_file_name,infos->no_empty_graph_warning)) {
      free_compilation_info(infos);
      return 1;
   }
}
if (tfst_check) {
   if (!valid_sentence_automaton(&(infos->vec),fst2_file_name)) {
      free_compilation_info(infos);
      return 1;
   }
}
free_compilation_info(infos);
u_printf("Compilation has succeeded\n");
return 0;
}

} // namespace unitex

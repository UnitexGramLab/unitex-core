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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "File.h"
#include "Alphabet.h"
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "GrfTest.h"
#include "ProgramInvoker.h"
#include "Normalize.h"
#include "Fst2Txt.h"
#include "Tokenize.h"
#include "Dico.h"
#include "Grf2Fst2.h"
#include "Locate.h"
#include "Concord.h"
#include "DirHelper.h"
#include "Grf_lib.h"
#include "GrfTest_lib.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define PFX_TOKENIZE "T:"
#define PFX_DICO "D:"
#define PFX_GRF2FST2 "G:"
#define PFX_LOCATE "L:"

const char* optstring_GrfTest=":ho:d:a:w:ck:q:";

const char* usage_GrfTest =
         "Usage: GrfTest [OPTIONS] <grf_1> [<grf_2> <grf_3> ...]\n"
         "\n"
         "  <grf_i>: .grf graphs to be tested\n"
         "\n"
         "OPTIONS:\n"
		 "  -o OUT: file where to report errors (default=stderr)\n"
		 "  -d DIC: the list of dictionaries to be applied by Dico, separated\n"
		 "             with semi-colons\n"
		 "  -a ALPH: alphabet file to use for all programs\n"
		 "  -w DIR: working directory to use\n"
		 "  -c: tells all programs that we work in char by char mode\n"
		 "  -h/--help: this help\n"
		 "\n"
		 "EXTRA OPTIONS:\n"
		 "Additional long options can be passed to Tokenize, Dico, Grf2Fst2 and Locate. To\n"
		 "do that, prefix the long option you want with T: D: G: or L:\n"
		 "For instance, if you want to specify a graph repository TMP that Grf2Fst2 should use,\n"
		 "you must use --G:pkgdir=TMP\n"
		 "Note that this works only with long options.\n"
         "\n"
         "This program looks for unit test patterns in grf files and runs those tests. Returns 0\n"
		 "if there is no problem; 1 otherwise.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_GrfTest);
}


static struct option_TS create_option(char *name,int has_arg,int *flag,int val) {
struct option_TS opt={name,has_arg,flag,val};
return opt;
}


static int get_n_options(const struct option_TS* opt) {
int i=0;
while (opt[i].name!=NULL || opt[i].has_arg!=0 || opt[i].flag!=NULL || opt[i].val!=0) i++;
return i;
}


static void add_options(struct option_TS* opt,int *n,const struct option_TS* lopts,const char* pfx) {
int i=0;
while (lopts[i].name!=NULL || lopts[i].has_arg!=0 || lopts[i].flag!=NULL || lopts[i].val!=0) {
	char* tmp=(char*)malloc(sizeof(char)*(strlen(pfx)+strlen(lopts[i].name)+1));
	if (tmp==NULL) {
		fatal_alloc_error("add_options");
	}
	sprintf(tmp,"%s%s",pfx,lopts[i].name);
	opt[*n].name=tmp;
	opt[*n].has_arg=lopts[i].has_arg;
	opt[*n].flag=NULL;
	opt[*n].val=*n;
	(*n)++;
	i++;
}
}

struct option_TS* filter_options() {
int size=1+get_n_options(lopts_Tokenize)+get_n_options(lopts_Dico)
		+get_n_options(lopts_Grf2Fst2)+get_n_options(lopts_Locate);
struct option_TS* opt=(struct option_TS*)malloc((size+1)*sizeof(struct option_TS));
if (opt==NULL) {
	fatal_alloc_error("filter_options");
}
int current=0;
add_options(opt,&current,lopts_Tokenize,PFX_TOKENIZE);
add_options(opt,&current,lopts_Dico,PFX_DICO);
add_options(opt,&current,lopts_Grf2Fst2,PFX_GRF2FST2);
add_options(opt,&current,lopts_Locate,PFX_LOCATE);
opt[current]=create_option(NULL,0,NULL,0);
return opt;
}

struct option_TS* lopts_GrfTest;



int main_GrfTest(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}
lopts_GrfTest=filter_options();
ProgramInvoker* invoker_Normalize=new_ProgramInvoker(main_Normalize,"Normalize");
ProgramInvoker* invoker_Tokenize=new_ProgramInvoker(main_Tokenize,"Tokenize");
ProgramInvoker* invoker_Dico=new_ProgramInvoker(main_Dico,"Dico");
ProgramInvoker* invoker_Grf2Fst2=new_ProgramInvoker(main_Grf2Fst2,"Grf2Fst2");
ProgramInvoker* invoker_Locate=new_ProgramInvoker(main_Locate,"Locate");
ProgramInvoker* invoker_Concord=new_ProgramInvoker(main_Concord,"Concord");
char* dic_list = (char*)malloc(2);
*dic_list = '\0';
char alphabet[FILENAME_MAX]="";
char working_dir[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
U_FILE* f_output=U_STDERR;
U_FILE* backup_stdout=U_STDOUT;
int char_by_char=0;
int val,index=-1;
int ret=0;
struct OptVars* vars=new_OptVars();
VersatileEncodingConfig vec=VEC_DEFAULT;
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_GrfTest,lopts_GrfTest,&index,vars))) {
   switch(val) {
   case 'h': usage(); free(dic_list); return 0;
   case 'o': strcpy(output,vars->optarg); break;
   case 'd': free(dic_list); dic_list = (char*)malloc(strlen(vars->optarg)+1); strcpy(dic_list,vars->optarg); break;
   case 'a': strcpy(alphabet,vars->optarg); break;
   case 'w': strcpy(working_dir,vars->optarg); break;
   case 'c': char_by_char=1; break;
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
             else fatal_error("Missing argument for option --%s\n",lopts_Normalize[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   default: {
	   if (index==-1) {
		   fatal_error("Internal error in main_GrfTest\n");
	   }
	   if (strstr(lopts_GrfTest[index].name,PFX_TOKENIZE)==lopts_GrfTest[index].name) {
		   add_long_option(invoker_Tokenize,lopts_GrfTest[index].name+strlen(PFX_TOKENIZE),vars->optarg);
	   } else if (strstr(lopts_GrfTest[index].name,PFX_DICO)==lopts_GrfTest[index].name) {
		   add_long_option(invoker_Dico,lopts_GrfTest[index].name+strlen(PFX_DICO),vars->optarg);
	   } else if (strstr(lopts_GrfTest[index].name,PFX_GRF2FST2)==lopts_GrfTest[index].name) {
		   add_long_option(invoker_Grf2Fst2,lopts_GrfTest[index].name+strlen(PFX_GRF2FST2),vars->optarg);
	   } else if (strstr(lopts_GrfTest[index].name,PFX_LOCATE)==lopts_GrfTest[index].name) {
		   add_long_option(invoker_Locate,lopts_GrfTest[index].name+strlen(PFX_LOCATE),vars->optarg);
	   }
   }
   }
   index=-1;
}
if (vars->optind==argc) {
	/* If there is no .grf we report no error, in order not
	 * to block test procedures if the program is called with *.grf
	 * on an empty directory */
	goto end;
}
if (working_dir[0]=='\0') {
	fatal_error("You must specify a working directory\n");
}
if (output[0]!='\0') {
	f_output=u_fopen(&vec,output,U_WRITE);
	if (f_output==NULL) {
		fatal_error("Cannot open output file %s\n",output);
	}
}
char fake_stdout[FILENAME_MAX];
char txt[FILENAME_MAX];
char snt[FILENAME_MAX];
char ind[FILENAME_MAX];
char concord[FILENAME_MAX];
char fst2[FILENAME_MAX];
char offsets_in[FILENAME_MAX];
char offsets_out[FILENAME_MAX];
sprintf(fake_stdout,"%s%sstdout",working_dir,PATH_SEPARATOR_STRING);
/* tmp file, so we can force the encoding */
U_STDOUT=u_fopen(UTF8,fake_stdout,U_WRITE);
if (U_STDOUT==NULL) {
	fatal_error("Cannot create file %s\n",fake_stdout);
}
sprintf(txt,"%s%sgrf_unit_test.txt",working_dir,PATH_SEPARATOR_STRING);
sprintf(snt,"%s%sgrf_unit_test_snt",working_dir,PATH_SEPARATOR_STRING);
sprintf(ind,"%s%sgrf_unit_test_snt%sconcord.ind",working_dir,PATH_SEPARATOR_STRING,PATH_SEPARATOR_STRING);
sprintf(concord,"%s%sgrf_unit_test_snt%sconcord.txt",working_dir,PATH_SEPARATOR_STRING,PATH_SEPARATOR_STRING);
mkDirPortable(snt);
sprintf(snt,"%s%sgrf_unit_test.snt",working_dir,PATH_SEPARATOR_STRING);
sprintf(fst2,"%s%stest.fst2",working_dir,PATH_SEPARATOR_STRING);
/* We prepare all the invokers */
int n_offsets;
n_offsets=0;
/* Normalize */
add_argument(invoker_Normalize,txt);
sprintf(offsets_out,"%s%soffsets%d.txt",working_dir,PATH_SEPARATOR_STRING,n_offsets);
add_long_option(invoker_Normalize,"output_offsets",offsets_out);
n_offsets++;
/* Tokenize */
add_argument(invoker_Tokenize,snt);
if (alphabet[0]!='\0') {
	/* If there is a global alphabet option, we use it */
	add_long_option(invoker_Tokenize,"alphabet",alphabet);
}
if (char_by_char) {
	add_argument(invoker_Tokenize,"-c");
}
sprintf(offsets_in,"%s%soffsets%d.txt",working_dir,PATH_SEPARATOR_STRING,n_offsets-1);
add_long_option(invoker_Tokenize,"input_offsets",offsets_in);
sprintf(offsets_out,"%s%soffsets%d.txt",working_dir,PATH_SEPARATOR_STRING,n_offsets);
add_long_option(invoker_Tokenize,"output_offsets",offsets_out);
n_offsets++;
/* Dico */
if (dic_list[0]=='\0') {
	free_ProgramInvoker(invoker_Dico);
	invoker_Dico=NULL;
} else {
	add_long_option(invoker_Dico,"text",snt);
	if (alphabet[0]!='\0') {
		/* If there is a global alphabet option, we use it */
		add_long_option(invoker_Tokenize,"alphabet",alphabet);
	}
	int pos=0,start,done=0;
	while (!done) {
		start=pos;
		while (dic_list[pos]!='\0' && dic_list[pos]!=';') pos++;
		if (dic_list[pos]=='\0') done=1;
		dic_list[pos]='\0';
		add_argument(invoker_Dico,dic_list+start);
		if (!done) pos++;
	}
}
/* Grf2Fst2 */
add_argument(invoker_Grf2Fst2,"-y");
add_long_option(invoker_Grf2Fst2,"output",fst2);
if (alphabet[0]!='\0' && !char_by_char) {
	/* If there is a global alphabet option, we use it */
	add_long_option(invoker_Grf2Fst2,"alphabet",alphabet);
}
if (char_by_char) {
	add_argument(invoker_Grf2Fst2,"-c");
}
/* Locate */
add_long_option(invoker_Locate,"text",snt);
if (alphabet[0]!='\0') {
	/* If there is a global alphabet option, we use it */
	add_long_option(invoker_Locate,"alphabet",alphabet);
}
if (char_by_char) {
	add_argument(invoker_Locate,"-c");
}
add_argument(invoker_Locate,fst2);
/* Concord */
add_argument(invoker_Concord,ind);
add_long_option(invoker_Concord,"uima",offsets_out);

/* Now we process the grf files */
U_FILE* f;

char line[4096];

for (int i=vars->optind;i<argc;i++) {
	u_fprintf(backup_stdout,"Testing graph %s\n",argv[i]);
	Grf* grf=load_Grf(&vec,argv[i]);
	if (grf==NULL) {
		error("Cannot load graph %s\n",argv[i]);
		continue;
	}
	vector_ptr* tests=get_grf_unit_tests(grf,argv[i],f_output);
	free_Grf(grf);
	if (tests==NULL) {
		goto next;
	}
	add_argument(invoker_Grf2Fst2,argv[i]);
	if (invoke(invoker_Grf2Fst2)) {
		error("Failure: cannot compile graph %s\n",argv[i]);
		ret=1;
		goto next;
	}
	/* We clean the graph name for the next grf */
	remove_last_argument(invoker_Grf2Fst2);
	/* And we launch the loop on all tests */
	for (int j=0;j<tests->nbelems;j++) {
		GrfUnitTest* t=(GrfUnitTest*)(tests->tab[j]);
		/* tmp file, we can force the encoding */
		f=u_fopen(UTF16_LE,txt,U_WRITE);
		if (f==NULL) {
			fatal_error("Cannot create file %s\n",txt);
		}
		u_fprintf(f,"%S",t->text);
		u_fclose(f);
		if (invoke(invoker_Normalize)) {
			build_command_line(invoker_Normalize,line);
			fatal_error("The following command has failed for graph %s:\n%s\n",argv[i],line);
		}
		if (invoke(invoker_Tokenize)) {
			build_command_line(invoker_Tokenize,line);
			fatal_error("The following command has failed for graph %s:\n%s\n",argv[i],line);
		}
		if (invoker_Dico!=NULL) {
			if (invoke(invoker_Dico)) {
				build_command_line(invoker_Dico,line);
				fatal_error("The following command has failed for graph %s:\n%s\n",argv[i],line);
			}
		}
		/* We have to adjust Locate parameters for the current test */
		switch(t->output_policy) {
		case IGNORE_OUTPUTS: add_argument(invoker_Locate,"-I"); break;
		case MERGE_OUTPUTS: add_argument(invoker_Locate,"-M"); break;
		case REPLACE_OUTPUTS: add_argument(invoker_Locate,"-R"); break;
		default: fatal_error("Internal error: invalid output policy in unit test\n");
		}
		switch(t->match_policy) {
		case SHORTEST_MATCHES: add_argument(invoker_Locate,"-S"); break;
		case LONGEST_MATCHES: add_argument(invoker_Locate,"-L"); break;
		case ALL_MATCHES: add_argument(invoker_Locate,"-A"); break;
		default: fatal_error("Internal error: invalid match policy in unit test\n");
		}
		if (invoke(invoker_Locate)) {
			build_command_line(invoker_Locate,line);
			fatal_error("The following command has failed for graph %s:\n%s\n",argv[i],line);
		}
		/* And we clean Locate parameters for the next test */
		remove_last_argument(invoker_Locate);
		remove_last_argument(invoker_Locate);
		if (invoke(invoker_Concord)) {
			build_command_line(invoker_Concord,line);
			fatal_error("The following command has failed for graph %s:\n%s\n",argv[i],line);
		}
		if (!check_test_results(t,concord,argv[i],f_output)) ret=1;
	}
	next:
	/* Don't forget to free the tests */
	free_vector_ptr(tests,(void(*)(void*))free_GrfUnitTest);
}

end:
u_fprintf(backup_stdout,"Completed: %s\n",(ret==0)?"success":"some tests failed");
/* As we made malloc to construct the option array, we have some free to do */
int n=get_n_options(lopts_GrfTest);
for (int i=0;i<n;i++) {
	free((void*)lopts_GrfTest[i].name);
}
free(lopts_GrfTest);
if (backup_stdout != U_STDOUT) {
  u_fclose(U_STDOUT);
  U_STDOUT = backup_stdout;
}
if (f_output!=U_STDERR) u_fclose(f_output);
free_ProgramInvoker(invoker_Normalize);
free_ProgramInvoker(invoker_Tokenize);
free_ProgramInvoker(invoker_Dico);
free_ProgramInvoker(invoker_Grf2Fst2);
free_ProgramInvoker(invoker_Locate);
free_ProgramInvoker(invoker_Concord);
free_OptVars(vars);
free(dic_list);
return ret;
}

} // namespace unitex

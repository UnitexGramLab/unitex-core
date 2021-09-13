/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

const char* optstring_GrfTest=":Vho:d:a:w:ck:q:s:r:n";

const char* usage_GrfTest =
         "Usage: GrfTest [OPTIONS] <grf_1> [<grf_2> <grf_3> ...]\n"
         "\n"
         "  <grf_i>: .grf graphs to be tested\n"
         "\n"
         "OPTIONS:\n"
         "  -o OUT: file where to report errors (default=stderr)\n"
         "  -n: do not redirect stdout when running test\n"
         "  -s STDOUT: file where to report stdout\n"
         "  -r resume_filename: file where to report success or fail\n"
         "  -d DIC: the list of dictionaries to be applied by Dico, separated\n"
         "             with semi-colons\n"
         "  -a ALPH: alphabet file to use for all programs\n"
         "  -w DIR: working directory to use\n"
         "  -c: tells all programs that we work in char by char mode\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
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
  display_copyright_notice();
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


static int add_options(struct option_TS* opt,int *n,const struct option_TS* lopts,const char* pfx) {
int i=0;
while (lopts[i].name!=NULL || lopts[i].has_arg!=0 || lopts[i].flag!=NULL || lopts[i].val!=0) {
  char* tmp=(char*)malloc(sizeof(char)*(strlen(pfx)+strlen(lopts[i].name)+1));
  if (tmp==NULL) {
    alloc_error("add_options");
    return ALLOC_ERROR_CODE;
  }
  sprintf(tmp,"%s%s",pfx,lopts[i].name);
  opt[*n].name=tmp;
  opt[*n].has_arg=lopts[i].has_arg;
  opt[*n].flag=NULL;
  opt[*n].val=*n;
  (*n)++;
  i++;
}
 return SUCCESS_RETURN_CODE;
}

struct option_TS* filter_options() {
int size=1+get_n_options(lopts_Tokenize)+get_n_options(lopts_Dico)
    +get_n_options(lopts_Grf2Fst2)+get_n_options(lopts_Locate);
struct option_TS* opt=(struct option_TS*)malloc((size+1)*sizeof(struct option_TS));
if (opt==NULL) {
  alloc_error("filter_options");
  return NULL;
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
   return SUCCESS_RETURN_CODE;
}

lopts_GrfTest=filter_options();
if(!lopts_GrfTest){
  return ALLOC_ERROR_CODE;
}

ProgramInvoker* invoker_Normalize=new_ProgramInvoker(main_Normalize,"Normalize");
ProgramInvoker* invoker_Tokenize=new_ProgramInvoker(main_Tokenize,"Tokenize");
ProgramInvoker* invoker_Dico=new_ProgramInvoker(main_Dico,"Dico");
ProgramInvoker* invoker_Grf2Fst2=new_ProgramInvoker(main_Grf2Fst2,"Grf2Fst2");
ProgramInvoker* invoker_Locate=new_ProgramInvoker(main_Locate,"Locate");
ProgramInvoker* invoker_Concord=new_ProgramInvoker(main_Concord,"Concord");
char* dic_list = (char*)malloc(1);
*dic_list = '\0';
char alphabet[FILENAME_MAX]="";
char working_dir[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
char fake_stdout[FILENAME_MAX]="";
char resume_out_filename[FILENAME_MAX]="";
U_FILE* f_output=U_STDERR;
U_FILE* backup_stdout=U_STDOUT;
U_FILE* resume_out=NULL;
int char_by_char=0;
int val,index=-1;
int return_value=SUCCESS_RETURN_CODE;
int alter_stdout=1;
VersatileEncodingConfig vec=VEC_DEFAULT;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_GrfTest,lopts_GrfTest,&index))) {
   switch(val) {
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return_value = SUCCESS_RETURN_CODE;
             goto end;
   case 'n': alter_stdout=0; break;
   case 'o': strcpy(output,options.vars()->optarg); break;
   case 'r': strcpy(resume_out_filename,options.vars()->optarg); break;
   case 's': strcpy(fake_stdout,options.vars()->optarg); break;
   case 'd': {
                size_t len_new_string = strlen(dic_list) + strlen(options.vars()->optarg) + 2;
                dic_list = (char*)realloc(dic_list, len_new_string);
                if (dic_list==NULL) {
                  alloc_error("main_GrfTest");
                  return_value = USAGE_ERROR_CODE;
                  goto end;
                }
                if ((*dic_list) != '\0') {
                  strcat(dic_list, ";");
                }
                strcat(dic_list, options.vars()->optarg);
                break;
             }
   case 'a': strcpy(alphabet,options.vars()->optarg); break;
   case 'w': strcpy(working_dir,options.vars()->optarg); break;
   case 'c': char_by_char=1; break;
   case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                return_value = USAGE_ERROR_CODE;
                goto end;
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                return_value = USAGE_ERROR_CODE;
                goto end;
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Normalize[index].name);
             return_value = USAGE_ERROR_CODE;
             goto end;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return_value = USAGE_ERROR_CODE;
             goto end;
   default: {
     if (index==-1) {
       error("Internal error in main_GrfTest\n");
       return_value = DEFAULT_ERROR_CODE;
       goto end;
     }
     if (strstr(lopts_GrfTest[index].name,PFX_TOKENIZE)==lopts_GrfTest[index].name) {
       add_long_option(invoker_Tokenize,lopts_GrfTest[index].name+strlen(PFX_TOKENIZE),options.vars()->optarg);
     } else if (strstr(lopts_GrfTest[index].name,PFX_DICO)==lopts_GrfTest[index].name) {
       add_long_option(invoker_Dico,lopts_GrfTest[index].name+strlen(PFX_DICO),options.vars()->optarg);
     } else if (strstr(lopts_GrfTest[index].name,PFX_GRF2FST2)==lopts_GrfTest[index].name) {
       add_long_option(invoker_Grf2Fst2,lopts_GrfTest[index].name+strlen(PFX_GRF2FST2),options.vars()->optarg);
     } else if (strstr(lopts_GrfTest[index].name,PFX_LOCATE)==lopts_GrfTest[index].name) {
       add_long_option(invoker_Locate,lopts_GrfTest[index].name+strlen(PFX_LOCATE),options.vars()->optarg);
     }
   }
   }
   index=-1;
}
if (options.vars()->optind==argc) {
  /* If there is no .grf we report no error, in order not
   * to block test procedures if the program is called with *.grf
   * on an empty directory */
  return_value = SUCCESS_RETURN_CODE;
  goto end;
}

if (working_dir[0]=='\0') {
  error("You must specify a working directory\n");
  return_value = USAGE_ERROR_CODE;
  goto end;
}

if (only_verify_arguments) {
  return_value = SUCCESS_RETURN_CODE;
  // freeing all allocated memory
  goto end;
}

if (output[0]!='\0') {
  f_output=u_fopen(&vec,output,U_WRITE);
  if (f_output==NULL) {
    error("Cannot open output file %s\n",output);
    return_value = DEFAULT_ERROR_CODE;
    goto end;
  }
}
char txt[FILENAME_MAX];
char snt[FILENAME_MAX];
char ind[FILENAME_MAX];
char concord[FILENAME_MAX];
char fst2[FILENAME_MAX];
char offsets_in[FILENAME_MAX];
char offsets_out[FILENAME_MAX];
if (alter_stdout==1) {
  if (fake_stdout[0]=='\0') {
    sprintf(fake_stdout,"%s%sstdout",working_dir,PATH_SEPARATOR_STRING);
    /* tmp file, so we can force the encoding */
    U_STDOUT=u_fopen(UTF8,fake_stdout,U_WRITE);
  }
  else {
    U_STDOUT=u_fopen(&vec,fake_stdout,U_WRITE);
  }
}
if ((alter_stdout==1) && (U_STDOUT==NULL)) {
  error("Cannot create file %s for stdout redirect\n",fake_stdout);
  return_value = DEFAULT_ERROR_CODE;
  goto end;
}
if (resume_out_filename[0]!='\0') {
  resume_out=u_fopen(&vec,resume_out_filename,U_WRITE);
  if (resume_out==NULL) {
     error("Cannot create file %s for resume file\n",resume_out_filename);
     return_value = DEFAULT_ERROR_CODE;
     goto end;
    }
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

for (int i=options.vars()->optind;i<argc;i++) {
  const char* graphname_display=argv[i];
  if (resume_out!= NULL) {
    for (size_t loop=0;loop<strlen(argv[i]);loop++)
      if (((argv[i])[loop]=='\\') || ((argv[i])[loop]=='/')) graphname_display=(argv[i])+loop+1;
  }
  u_fprintf((resume_out != NULL) ? resume_out : backup_stdout,"Testing graph: %s\n",graphname_display);

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
    return_value=DEFAULT_ERROR_CODE;
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
      error("Cannot create file %s\n",txt);
      return_value = DEFAULT_ERROR_CODE;
      goto end;
    }
    u_fputs(t->text,f);
    u_fclose(f);
    if (invoke(invoker_Normalize)) {
      char* line=build_command_line_alloc(invoker_Normalize);
      error("The following command has failed for graph %s:\n%s\n",argv[i],line);
      free_command_line_alloc(line);
      return_value = DEFAULT_ERROR_CODE;
      goto end;
    }
    if (invoke(invoker_Tokenize)) {
      char* line=build_command_line_alloc(invoker_Tokenize);
      error("The following command has failed for graph %s:\n%s\n",argv[i],line);
      free_command_line_alloc(line);
      return_value = DEFAULT_ERROR_CODE;
      goto end;
    }
    if (invoker_Dico!=NULL) {
      if (invoke(invoker_Dico)) {
        char* line=build_command_line_alloc(invoker_Dico);
        error("The following command has failed for graph %s:\n%s\n",argv[i],line);
        free_command_line_alloc(line);
        return_value = DEFAULT_ERROR_CODE;
        goto end;
      }
    }
    /* We have to adjust Locate parameters for the current test */
    switch(t->output_policy) {
    case IGNORE_OUTPUTS: add_argument(invoker_Locate,"-I"); break;
    case MERGE_OUTPUTS: add_argument(invoker_Locate,"-M"); break;
    case REPLACE_OUTPUTS: add_argument(invoker_Locate,"-R"); break;
    default: {
      error("Internal error: invalid output policy in unit test\n");
      return_value = DEFAULT_ERROR_CODE;
      goto end;
     }
    }
    switch(t->match_policy) {
      case SHORTEST_MATCHES: add_argument(invoker_Locate,"-S"); break;
      case LONGEST_MATCHES: add_argument(invoker_Locate,"-L"); break;
      case ALL_MATCHES: add_argument(invoker_Locate,"-A"); break;
      default: {
        error("Internal error: invalid match policy in unit test\n");
        return_value = DEFAULT_ERROR_CODE;
        goto end;
      }
    }
    if (invoke(invoker_Locate)) {
      char* line=build_command_line_alloc(invoker_Locate);
      error("The following command has failed for graph %s:\n%s\n",argv[i],line);
      free_command_line_alloc(line);
      return_value = DEFAULT_ERROR_CODE;
      goto end;
    }
    /* And we clean Locate parameters for the next test */
    remove_last_argument(invoker_Locate);
    remove_last_argument(invoker_Locate);
    if (invoke(invoker_Concord)) {
      char* line=build_command_line_alloc(invoker_Concord);
      error("The following command has failed for graph %s:\n%s\n",argv[i],line);
      free_command_line_alloc(line);
      return_value = DEFAULT_ERROR_CODE;
      goto end;
    }
    if (!check_test_results(t,concord,argv[i],f_output)) return_value=DEFAULT_ERROR_CODE;
  }
  next:
  /* Don't forget to free the tests */
  free_vector_ptr(tests,(void(*)(void*))free_GrfUnitTest);
}

end:
u_fprintf((resume_out != NULL) ? resume_out : backup_stdout,"Completed: %s\n",(return_value==SUCCESS_RETURN_CODE)?"success":"some tests failed");
/* As we made malloc to construct the option array, we have some free to do */
int n=get_n_options(lopts_GrfTest);
for (int i=0;i<n;i++) {
  free((void*)lopts_GrfTest[i].name);
}
free(lopts_GrfTest);
if (resume_out != NULL) {
  u_fclose(resume_out);
}
if ((alter_stdout==1) && (backup_stdout != U_STDOUT)) {
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
free(dic_list);
return return_value;
}

} // namespace unitex

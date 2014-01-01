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
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "SpellCheck.h"
#include "SpellChecking.h"
#include "CompressedDic.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_SpellCheck =
        "Usage: SpellCheck [OPTIONS] <dic1> [<dic2> <dic3> ...]\n"
        "\n"
		"  <dic_i>: .bin or .bin2 dictionary to lookup in for spellchecking\n"
		"  -s SNT/--snt=SNT: performs spellchecking on the err file of the given snt. When this\n"
		"                    option is used, options -IU and -OA are forced.\n"
		"  -f TXT/--file=TXT: performs spellchecking on the given file, assuming that it\n"
		"                     contains one word per line\n"
		"\n"
        "OPTIONS:\n"
		"  -o OUT/--output=OUT: specifies the file where the dictionary entries should be placed\n"
		"                       With --snt, the default is the 'dlf' file. With --file, the default\n"
		"                       is stdout. If OUT is not specified, results are printed to stdout\n"
		"  -I [DMU]/--input-op=[DMU]: indicates what operation should be performed on the input\n"
		"                             text file: (D)on't modify it, keep (M)atched words or\n"
		"                             keep (U)nmatched words (default=D)\n"
		"  -O [OA]/--output-op=[OA]: if the output file exists, shall we (O)verwrite it or open\n"
		"                            it in (A)ppend mode (default=A)\n"
		"\n"
		"  --max-errors=N: total maximum number of errors per word (default=1)\n"
		"  --max-insert=N: maximum number of inserted letters per word (default=1)\n"
		"  --max-suppr=N: maximum number of deleted letters per word (default=1)\n"
		"  --max-change=N: maximum number of changed letters per word (default=1)\n"
		"  --max-swap=N: maximum number of letter inversions per word (default=1)\n"
		"  --scores=XXX: specifies the scores for each kind of error. See --help-scores\n"
		"  --min-lengths=A,B,C: A, B and C are the minimum word lengths required to\n"
		"                       allow 1, 2 and 3+ errors (default=4,6,12)\n"
		"  --upper-initial=[yes/no]: allows or not errors involving an uppercase letter\n"
		"                            at the beginning of the word (default=no)\n"
		"\n"
		"  --keyboard=XXX: uses keyboard heuristic with the given keyboard configuration name. By\n"
		"                  default, no keyboard heuristic is used\n"
		"  --show-keyboards: displays all available keyboard configurations\n"
		"\n"
		"  -h/--help: this help\n"
        "\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_SpellCheck);
}


static void usage_scores() {
u_printf("%S",COPYRIGHT);
u_printf(
		"Usage: SpellCheck [OPTIONS] <dic1> [<dic2> <dic3> ...]\n"
		"\n"
		"The --scores=XXX option allows you to specify a score for each of the\n"
		"%d supported kinds of error. XXX must be made of %d comma-separated integer\n"
		"values which respectively mean the following:\n"
		"- letter duplication:              devil   => devvil\n"
		"- any other letter insertion:      devil   => degvil\n"
		"- double letter simplification:    battle  => batle\n"
		"- any other letter omission:       battle  => bttle\n"
		"- letter inversion:                happy   => ahppy\n"
		"- diacritic error:                 %Ctaient => etaient\n"
		"- case error:                      London  => london\n"
		"- letter close on keyboard:        battle  => bzttle\n"
		"- any other letter change:         battle  => blttle\n"
		"\n"
		"Each hypothesis is given the sum of its errors' scores. The lower\n"
		"the score, the better the hypothesis. Default values are:\n"
		"--scores=%d",N_SPSubOp,N_SPSubOp,0xE9,default_scores[0]);
for (int i=1;i<N_SPSubOp;i++) {
	u_printf(",%d",default_scores[i]);
}
u_printf("\n\n");
}


const char* optstring_SpellCheck=":hk:q:s:f:o::I:O:";
const struct option_TS lopts_SpellCheck[]= {
	  {"snt",required_argument_TS,NULL,'s'},
	  {"file",required_argument_TS,NULL,'f'},
	  {"output",optional_argument_TS,NULL,'o'},
	  {"input-op",required_argument_TS,NULL,'I'},
	  {"output-op",required_argument_TS,NULL,'O'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"keyboard",required_argument_TS,NULL,1},
      {"show-keyboards",no_argument_TS,NULL,2},
      {"max-errors",required_argument_TS,NULL,10},
      {"max-insert",required_argument_TS,NULL,11},
      {"max-suppr",required_argument_TS,NULL,12},
      {"max-change",required_argument_TS,NULL,13},
      {"max-swap",required_argument_TS,NULL,14},
      {"scores",required_argument_TS,NULL,20},
      {"help-scores",no_argument_TS,NULL,21},
      {"min-lengths",required_argument_TS,NULL,22},
      {"upper-initial",required_argument_TS,NULL,23},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};



int main_SpellCheck(int argc,char* const argv[]) {
if (argc==1) {
	usage();
	return 0;
}
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
char mode=0;
char snt[FILENAME_MAX]="";
char txt[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
char output_set=0;
char output_op='A';
SpellCheckConfig config;
config.max_errors=1;
config.max_SP_INSERT=1;
config.max_SP_SUPPR=1;
config.max_SP_SWAP=1;
config.max_SP_CHANGE=1;
for (int i=0;i<N_SPSubOp;i++) {
	config.score[i]=default_scores[i];
}
config.min_length1=4;
config.min_length2=6;
config.min_length3=12;
config.input_op='D';
config.keyboard=NULL;
config.allow_uppercase_initial=0;
char foo;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_SpellCheck,lopts_SpellCheck,&index,vars))) {
   switch(val) {
   case 's': {
	   strcpy(snt,vars->optarg);
	   mode='s';
	   break;
   }
   case 'f': {
	   strcpy(txt,vars->optarg);
	   mode='f';
	   break;
   }
   case 'o': {
	   if (vars->optarg!=NULL) {
		   strcpy(output,vars->optarg);
	   }
	   output_set=1;
	   break;
   }
   case 'I': {
	   if (!strcmp(vars->optarg,"D") || !strcmp(vars->optarg,"M") || !strcmp(vars->optarg,"U")) {
		   config.input_op=vars->optarg[0];
	   } else {
		   fatal_error("Invalid argument %s for option --input-op: should in [DMU]\n",vars->optarg);
	   }
	   break;
   }
   case 'O': {
	   if (!strcmp(vars->optarg,"O") || !strcmp(vars->optarg,"A")) {
		   output_op=vars->optarg[0];
	   } else {
		   fatal_error("Invalid argument %s for option --output-op: should in [OA]\n",vars->optarg);
	   }
	   break;
   }
   case 1: {
	   config.keyboard=get_Keyboard(vars->optarg);
	   if (config.keyboard==NULL) {
		   fatal_error("Invalid argument %s for option --keyboard:\nUse --show-keyboards to see possible values\n",vars->optarg);
	   }
	   break;
   }
   case 2: {
	   print_available_keyboards(U_STDOUT);
	   exit(0);
   }
   case 10: {
	   if (1!=sscanf(vars->optarg,"%u%c",&config.max_errors,&foo)) {
		   fatal_error("Invalid argument %s for --max-errors: should be an integer >=0\n",vars->optarg);
       }
       break;
   }
   case 11: {
	   if (1!=sscanf(vars->optarg,"%u%c",&config.max_SP_INSERT,&foo)) {
		   fatal_error("Invalid argument %s for --max-insert: should be an integer >=0\n",vars->optarg);
       }
       break;
   }
   case 12: {
	   if (1!=sscanf(vars->optarg,"%u%c",&config.max_SP_SUPPR,&foo)) {
		   fatal_error("Invalid argument %s for --max-suppr: should be an integer >=0\n",vars->optarg);
       }
       break;
   }
   case 13: {
	   if (1!=sscanf(vars->optarg,"%u%c",&config.max_SP_CHANGE,&foo)) {
		   fatal_error("Invalid argument %s for --max-change: should be an integer >=0\n",vars->optarg);
       }
       break;
   }
   case 14: {
	   if (1!=sscanf(vars->optarg,"%u%c",&config.max_SP_SWAP,&foo)) {
		   fatal_error("Invalid argument %s for --max-swap: should be an integer >=0\n",vars->optarg);
       }
       break;
   }
   case 20: {
	   int* scores=config.score;
	   if (N_SPSubOp!=sscanf(vars->optarg,"%d,%d,%d,%d,%d,%d,%d,%d,%d%c",
	   		scores,scores+1,scores+2,scores+3,scores+4,scores+5,
	   		scores+6,scores+7,scores+8,&foo)) {
		   fatal_error("Invalid argument %s for option --scores. See --help-scores\n",vars->optarg);
	   }
	   break;
   }
   case 21: {
	   usage_scores();
	   exit(0);
   }
   case 22: {
	   if (3!=sscanf(vars->optarg,"%u,%u,%u%c",
	   		&config.min_length1,&config.min_length2,&config.min_length3,&foo)) {
		   fatal_error("Invalid argument %s for option --min-lengths\n",vars->optarg);
	   }
	   break;
   }
   case 23: {
	   if (!strcmp(vars->optarg,"yes")) {
		   config.allow_uppercase_initial=1;
	   } else if (!strcmp(vars->optarg,"no")) {
		   config.allow_uppercase_initial=0;
	   } else {
		   fatal_error("Invalid argument %s for option --upper-initial\n",vars->optarg);
	   }
	   break;
   }
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
             else fatal_error("Missing argument for option --%s\n",lopts_SpellCheck[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}
if (vars->optind==argc) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
config.n_dics=argc-vars->optind;
config.dics=(Dictionary**)malloc(config.n_dics*sizeof(Dictionary*));
if (config.dics==NULL) {
	fatal_alloc_error("main_SpellCheck");
}
for (int i=0;i<config.n_dics;i++) {
	config.dics[i]=new_Dictionary(&vec,argv[i+vars->optind]);
	if (config.dics[i]==NULL) {
		error("Cannot load dictionary %s\n",argv[i+vars->optind]);
	}
}
if (mode==0) {
	fatal_error("You must use either --snt or --file\n");
}
config.out=U_STDOUT;
config.n_input_lines=0;
config.n_output_lines=0;
if (mode=='s') {
	/* When working with a .snt, we actually want to work on its err file */
	get_snt_path(snt,txt);
	strcat(txt,"err");
	/* the output must be dlf, and we note the number of lines in the existing
	 * dlf file, if any */
	get_snt_path(snt,output);
	strcat(output,"dlf.n");
	U_FILE* f=u_fopen(&vec,output,U_READ);
	if (f!=NULL) {
		u_fscanf(f,"%d",&(config.n_output_lines));
		u_fclose(f);
	}
	get_snt_path(snt,output);
	strcat(output,"dlf");
	output_set=1;
	/* and we force the values for -I and -O */
	config.input_op='U';
	output_op='A';
} else {
	/* If mode=='f', we don't have anything to do since we already
	 * defined the default output to stdout */
}
if (output_set) {
	if (output_op=='O') {
		config.out=u_fopen(&vec,output,U_WRITE);
	} else {
		config.out=u_fopen(&vec,output,U_APPEND);
	}
	if (config.out==NULL) {
		fatal_error("Cannot open output file %s\n",output);
	}
}
config.modified_input=NULL;
char modified_input[FILENAME_MAX]="";
if (config.input_op!='D') {
	strcpy(modified_input,txt);
	strcat(modified_input,".tmp");
	config.modified_input=u_fopen(&vec,modified_input,U_WRITE);
	if (config.modified_input==NULL) {
		fatal_error("Cannot open tmp file %s\n",modified_input);
	}
}
config.in=u_fopen(&vec,txt,U_READ);
if (config.in==NULL) {
	fatal_error("Cannot open file %s\n",txt);
}
/* We perform spellchecking */
spellcheck(&config);

/* And we clean */
u_fclose(config.in);
for (int i=0;i<config.n_dics;i++) {
	free_Dictionary(config.dics[i]);
}
free(config.dics);
if (config.out!=U_STDOUT) {
	u_fclose(config.out);
}
if (config.modified_input!=NULL) {
	/* If we used a tmp file because the input file has to be modified,
	 * it's now time to actually modify it */
	u_fclose(config.modified_input);
	af_remove(txt);
	af_rename(modified_input,txt);
}
/* Finally, we update the dlf.n and err.n files if mode=='s' */
if (mode=='s') {
	get_snt_path(snt,output);
	strcat(output,"err.n");
	U_FILE* f=u_fopen(&vec,output,U_WRITE);
	if (f!=NULL) {
		u_fprintf(f,"%d",config.n_input_lines);
		u_fclose(f);
	}
	if (config.input_op!='D') {
		get_snt_path(snt,output);
		strcat(output,"dlf.n");
		U_FILE* f=u_fopen(&vec,output,U_WRITE);
		if (f!=NULL) {
			u_fprintf(f,"%d",config.n_output_lines);
			u_fclose(f);
		}
	}
}
free_OptVars(vars);
return 0;
}

} // namespace unitex



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
#include "DELA.h"
#include "String_hash.h"
#include "Unicode.h"
#include "Alphabet.h"
#include "Text_tokens.h"
#include "ApplyDic.h"
#include "List_int.h"
#include "File.h"
#include "CompoundWordHashTable.h"
#include "Fst2.h"
#include "Copyright.h"
#include "Locate.h"
#include "Error.h"
#include "Snt.h"
#include "LocateConstants.h"
#include "UnitexGetOpt.h"
#include "Dico.h"
#include "SortTxt.h"
#include "Compress.h"

/**
 * This enhanced version of Dico was rewritten by Alexis Neme,
 * based on the original version written by Sébastien Paumier
 * 15 Novembre 2005
 * This new version take into account not only compiled dictionnary but also
 * local grammars representent by an FST2 as part of the lexical identification
 * stage.
 */


#define DONT_PRODUCE_MORPHO_DIC 0
#define PRODUCE_MORPHO_DIC_AT_THE_END 1
#define PRODUCE_MORPHO_DIC_NOW 2


int raw_dic_application(const VersatileEncodingConfig*,U_FILE* text,U_FILE* raw_output,Alphabet* alphabet,int ind,char* const argv[]);


const char* usage_Dico =
         "Usage: Dico [OPTIONS] <dic_1> [<dic_2> <dic_3> ...]\n"
         "\n"
         "  <dic_i>: .bin dictionary or .fst2 local grammar to be applied\n"
         "\n"
         "OPTIONS:\n"
         "  -t TXT/--text=TXT: the .snt text file\n"
         "  -a ALPH/--alphabet=ALPH: the alphabet file\n"
         "  -m DIC/--morpho=DIC: specifies that DIC is a .bin dictionary\n"
         "                         to use in Locate's morphological mode. Use as many\n"
         "                         -m XXX as there are .bin to use. You can also\n"
		 "                         separate several .bin with semi-colons.\n"
		 "  -K/--korean: tells Dico that it works on Korean\n"
		 "  -s/--semitic: tells Dico that it works on a semitic language\n"
		 "  -u X/--arabic_rules=X: Arabic typographic rule configuration file\n"
		 "  -r X/--raw=X: indicates that Dico should just produce one output file X containing\n"
		 "                both simple and compound words, without requiring a text directory.\n"
		 "                If X is omitted, results are displayed on the standard output."
		 "  -h/--help: this help\n"
         "\n"
         "Applies dictionaries and/or local grammars to the text and produces \n"
         "6 files, saved in the text directory. These files are:\n"
         "\n"
         "  dlf: simple entry dictionary\n"
         "  dlc: compound entry dictionary\n"
         "  err: unrecognized simple words\n"
         "  tags_err: unrecognized simple words that are neither included in tags.ind's matches\n"
         "  tags.ind: sequences to be inserted in the text automaton\n"
         "  stat_dic.n: file containing the number of simple words, the number\n"
         "              of compound words, and the number of unknown words in the text\n"
         "\n"
         "There are 3 levels of priority. If the dictionary name ends with \"-\",\n"
         "it will be applied with the maximum priority. If the suffix\n"
         "is \"+\", the priority is minimal. If there is no suffix, the priority\n"
         "is normal. Any lexical unit identified in a previous stage will be ignored\n"
         "in the subsequent stages. \n"
         "\n"
         "The local grammars are represented by finite state transducers(.fst2).\n"
         "These grammars will be applied in MERGE mode, except if a .fst2 ends\n"
         "with -r. In that case, it will be applied in REPLACE mode.\n"
         "Note that -r can be combined with - or + priority marks (-r- and -r+)\n"
         "If one grammar name or more is ended by -b -b+ -b- -rb -rb+  or -rb-,\n"
         "all its DELAF outputs will also be copied in a file named morpho.dic located\n"
         "in the text directory. At the end of Dico, this dictionary will be compressed\n"
         "to produce the morpho.bin file that will be used by Locate as a local morphological\n"
         "dictionary for the current text. For additional options, see user manual.\n"
         "\n"
         "The grammar output shall respect the file format of both DLF and DLC.\n"
         "If an output starts with a / character, it will be considered as a tag\n"
         "sequence to be put in the 'tags.ind' file. Such sequences are used\n"
         "by the Txt2Tfst program in order to add paths to the text automaton.\n"
         "\n"
         "The numbers of simple, compound and unknown forms are saved\n"
         "in a file named stat_dic.n which is created in the text directory.\n"
         "\n"
         "Examples:\n"
         " - Dico -t\"c:\\tutu.snt\" -a\"c:\\Alphabet.txt\" Dela.bin MyFilter-.bin\n"
         "This command will apply first MyFilter-.bin and then Dela.bin.\n"
         " - Dico -t\"c:\\tutu.snt\" -a\"c:\\Alphabet.txt\" Dela.bin Rom_numbs-.fst2 numbers+.fst2\n"
         "This command will apply Rom_numbs-.fst2 then Dela.bin and finally\n"
         "numbers+.fst2\n"
         "\n"
         "Note: the 3 resulting files (DLF, DLC and ERR) are stored in the text\n"
         "directory. THEY ARE NOT SORTED AND MAY CONTAIN DUPLICATES. Use the\n"
         "SortTxt program to clean these files.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Dico);
}


/**
 * This function stores some statistics in 'stat_dic.n'.
 */
void save_statistics(const VersatileEncodingConfig* vec,char* name,struct dico_application_info* info) {
U_FILE* f=u_fopen(vec,name,U_WRITE);
if (f==NULL) {
   error("Cannot write stat file %s\n",name);
   return;
}
u_fprintf(f,"%d\n%d\n%d\n",info->SIMPLE_WORDS,info->COMPOUND_WORDS,info->UNKNOWN_WORDS);
u_fclose(f);
}


/**
 * This function gets a string 's', and checks if its substring of length 'len'
 * ends with fst2 dictionary graph options of the form -XYZ, where
 * 		X = [rRmM] : replace or merge mode (optional; default=merge)
 *      Y = [bBzZ] : option about the production of entries in a morphological dictionary
 *      Z = [aAlLsS] : all/longest/shortest matches (default=longest)
 *
 * If the function fails to analyse the options, it does not set any value.
 */
static void analyse_fst2_graph_options(const char* s,int len,OutputPolicy *outputPolicy,
		                               int *export_in_morpho_dic,MatchPolicy *matchPolicy) {
OutputPolicy output=MERGE_OUTPUTS;
int morpho=DONT_PRODUCE_MORPHO_DIC;
MatchPolicy match=LONGEST_MATCHES;
/* NOTE: for all error case tests, we test len==0 and not len<0, because
 *       a fst2 name is supposed to be made not only of options but also with
 *       a real name, thus needing at least one character */
if (len==0) {
	return;
}
switch (s[len]) {
case 'a': case 'A': match=ALL_MATCHES; len--; break;
case 'l': case 'L': match=LONGEST_MATCHES; len--; break;
case 's': case 'S': match=SHORTEST_MATCHES; len--; break;
default: break;
}
if (len==0) {
	return;
}
switch (s[len]) {
case 'b': case 'B': morpho=PRODUCE_MORPHO_DIC_AT_THE_END; len--; break;
case 'z': case 'Z': morpho=PRODUCE_MORPHO_DIC_NOW; len--; break;
default: break;
}
if (len==0) {
	return;
}
switch (s[len]) {
case 'r': case 'R': output=REPLACE_OUTPUTS; len--; break;
case 'm': case 'M': output=MERGE_OUTPUTS; len--; break;
default: break;
}
if (len==0 || s[len]!='-') {
	return;
}
*outputPolicy=output;
*export_in_morpho_dic=morpho;
*matchPolicy=match;
}



const char* optstring_Dico=":t:a:m:Khk:q:u:g:sr::";
const struct option_TS lopts_Dico[]= {
      {"text",required_argument_TS,NULL,'t'},
      {"alphabet",required_argument_TS,NULL,'a'},
      {"morpho",required_argument_TS,NULL,'m'},
      {"korean",no_argument_TS,NULL,'K'},
      {"help",no_argument_TS,NULL,'h'},
      {"negation_operator",required_argument_TS,NULL,'g'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"arabic_rules",required_argument_TS,NULL,'u'},
      {"raw",optional_argument_TS,NULL,'r'},
      {"semitic",no_argument_TS,NULL,'s'},
      {NULL,no_argument_TS,NULL,0}
};


int main_Dico(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

int ret=0;
int val,index=-1;
char alph[FILENAME_MAX]="";
char text[FILENAME_MAX]="";
char arabic_rules[FILENAME_MAX]="";
char raw_output[FILENAME_MAX]="";
char negation_operator[0x20]="";
char* morpho_dic=NULL;
int is_korean=0;
int semitic=0;
U_FILE* f_raw_output=NULL;
VersatileEncodingConfig vec=VEC_DEFAULT;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Dico,lopts_Dico,&index,vars))) {
   switch(val) {
   case 'g': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify an argument for negation operator\n");
             }
             if ((strcmp(vars->optarg,"minus")!=0) && (strcmp(vars->optarg,"-")!=0) && 
                 (strcmp(vars->optarg,"tilde")!=0) && (strcmp(vars->optarg,"~")!=0))
             {
                 fatal_error("You must specify a valid argument for negation operator\n");
             }
             strcpy(negation_operator,vars->optarg);
             break;
   case 't': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty text file name\n");
             }
             strcpy(text,vars->optarg);
             break;
   case 'a': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet name\n");
             }
             strcpy(alph,vars->optarg);
             break;
   case 'm': if (vars->optarg[0]!='\0') {
                if (morpho_dic==NULL) {
                  morpho_dic=strdup(vars->optarg);
                  if (morpho_dic==NULL) {
                     fatal_alloc_error("main_Dico");
                  }
                }
                else
                {
                    morpho_dic = (char*)realloc((void*)morpho_dic,strlen(morpho_dic)+strlen(vars->optarg)+2);
                    if (morpho_dic==NULL) {
                       fatal_alloc_error("main_Dico");
                    }
                    strcat(morpho_dic,";");
                    strcat(morpho_dic,vars->optarg);
                }
             }
             break;
   case 'K': is_korean=1;
             break;
   case 'h': usage(); return 0;
   case 'u': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty arabic rule configuration file name\n");
             }
             strcpy(arabic_rules,vars->optarg);
             break;
   case 's': semitic=1;
             break;
   case 'r': if (vars->optarg==NULL) {
	            /* No argument ? We display on stdout */
	            f_raw_output=U_STDOUT;
	            break;
             }
			 if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(raw_output,vars->optarg);
             break;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Dico[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
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
   }
   index=-1;
}

if (text[0]=='\0') {
   fatal_error("You must specify a .snt text file\n");
}
if (vars->optind==argc) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
Alphabet* alphabet=NULL;
if (alph[0]!='\0') {
   /* We load the alphabet */
   alphabet=load_alphabet(&vec,alph,is_korean);
   if (alphabet==NULL) {
      error("Cannot open alphabet file %s\n",alph);
      return 1;
   }
}

if (raw_output[0]!='\0') {
	f_raw_output=u_fopen(&vec,raw_output,U_WRITE);
	if (f_raw_output==NULL) {
		fatal_error("Cannot create output file %s\n",raw_output);
	}
}
if (f_raw_output!=NULL) {
	U_FILE* f_text=u_fopen(&vec,text,U_READ);
	if ((*text)=='\0') {
		fatal_error("Cannot open text file %s\n",text);
	}
	int ret_applic=raw_dic_application(&vec,f_text,f_raw_output,alphabet,vars->optind,argv);
	u_fclose(f_text);
	if (f_raw_output!=U_STDOUT) u_fclose(f_raw_output);
	free_alphabet(alphabet);
	free_OptVars(vars);
	return ret_applic;
}

struct snt_files* snt_files=new_snt_files(text);
U_FILE* text_cod;
struct text_tokens* tokens;

/* And we create empty files in order to append things to them */
if (!u_fempty(&vec,snt_files->dlf)) {
   fatal_error("Cannot create %s\n",snt_files->dlf);
}
if (!u_fempty(&vec,snt_files->dlc)) {
   fatal_error("Cannot create %s\n",snt_files->dlc);
}
if (!u_fempty(&vec,snt_files->err)) {
   fatal_error("Cannot create %s\n",snt_files->err);
}
if (!u_fempty(&vec,snt_files->tags_err)) {
   fatal_error("Cannot create %s\n",snt_files->tags_err);
}
/* We remove the text morphological dictionary files, if any */
af_remove(snt_files->morpho_dic);
af_remove(snt_files->morpho_bin);
af_remove(snt_files->morpho_inf);
/* We load the text tokens */
tokens=load_text_tokens(&vec,snt_files->tokens_txt);
if (tokens==NULL) {
   free_alphabet(alphabet);
   error("Cannot open token file %s\n",snt_files->tokens_txt);
   return 1;
}
/* We try opening the text.cod file for binary reading */
text_cod=u_fopen(BINARY,snt_files->text_cod,U_READ);
if (text_cod==NULL) {
   free_alphabet(alphabet);
   free_text_tokens(tokens);
   error("Cannot open coded text file %s\n",snt_files->text_cod);
   return 1;
}
u_fclose(text_cod);
u_printf("Initializing...\n");
struct dico_application_info* info=init_dico_application(tokens,NULL,NULL,NULL,NULL,NULL,snt_files->tags_ind,snt_files->text_cod,alphabet,&vec);

/* First of all, we compute the number of occurrences of each token */
u_printf("Counting tokens...\n");
count_token_occurrences(info);
/* We all dictionaries according their priority */
for (int priority=1;priority<4;priority++) {
   /* For a given priority, we apply all concerned dictionaries
    * in their order on the command line */
   for (int i=vars->optind;i<argc;i++) {
      char tmp[FILENAME_MAX];
      remove_extension(argv[i],tmp);
      char priority_mark=tmp[strlen(tmp)-1];
      if ((priority==1 && priority_mark=='-') ||  (priority==2 && priority_mark!='-' && priority_mark!='+') ||  (priority==3 && priority_mark=='+')) {
         /* If we must must process a dictionary, we check its type */
         char tmp2[FILENAME_MAX];
         get_extension(argv[i],tmp2);
         if (!strcmp(tmp2,".bin") || !strcmp(tmp2,".bin2"))    {
            /*
             * If it is a .bin dictionary
             */
	         u_printf("Applying dico  %s...\n",argv[i]);
            /* We open output files: dictionaries in APPEND mode since we
             * can only add entries to them, and 'err' in WRITE mode because
             * each dictionary application may reduce this file */
 
            /* 
             * We are using encoding preference
             */
            info->dlf=u_fopen(&vec,snt_files->dlf,U_APPEND);
            info->dlc=u_fopen(&vec,snt_files->dlc,U_APPEND);
            info->err=u_fopen(&vec,snt_files->err,U_WRITE);
            /* Working... */
            if (dico_application(&vec,argv[i],info,priority) != 0)
              ret = 1;
            /* Dumping and closing output files */
            save_unknown_words(info);
        	   u_fclose(info->dlf);
            u_fclose(info->dlc);
            u_fclose(info->err);
            /* We set file descriptors to NULL so that we can test if files were closed */
            info->dlf=NULL;
            info->dlc=NULL;
            info->err=NULL;
         }
         else if (!strcmp(tmp2,".fst2")) {
            /*
             * If it is a .fst2 dictionary
             */
            int l=(int)(strlen(tmp)-((priority==2)?1:2));
            OutputPolicy outputPolicy=MERGE_OUTPUTS;
            int export_in_morpho_dic=DONT_PRODUCE_MORPHO_DIC;
            MatchPolicy matchPolicy=LONGEST_MATCHES;
            analyse_fst2_graph_options(tmp,l,&outputPolicy,&export_in_morpho_dic,&matchPolicy);
            u_printf("Applying grammar %s...\n",argv[i]);
            /**
             * IMPORTANT!!!
             * dlf, dlc and err must not be open while launch_locate_as_routine
             * is running, because this function tries to read in these files.
             */
            if (0!=launch_locate_as_routine(&vec,
            		text,argv[i],alph,outputPolicy,matchPolicy,morpho_dic,1,is_korean,arabic_rules,negation_operator)) {
            	ret=1;
            }
	         /* We open output files: dictionaries in APPEND mode since we
             * can only add entries to them, and 'err' in WRITE mode because
             * each dictionary application may reduce this file */
            info->dlf=u_fopen(&vec,snt_files->dlf,U_APPEND);
            info->dlc=u_fopen(&vec,snt_files->dlc,U_APPEND);
            info->err=u_fopen(&vec,snt_files->err,U_WRITE);
            if (export_in_morpho_dic!=DONT_PRODUCE_MORPHO_DIC) {
               /* If necessary, we deal with the morpho.dic file */
               if (info->morpho==NULL) {
                  /* We create it if needed */
                  info->morpho=u_fopen(&vec,snt_files->morpho_dic,U_WRITE);
                  if (info->morpho==NULL) {
                     fatal_error("");
                  }
               }
            }
            /* And we merge the Locate results with current dictionaries */
            merge_dic_locate_results(info,snt_files->concord_ind,priority,export_in_morpho_dic);
            if (export_in_morpho_dic==PRODUCE_MORPHO_DIC_NOW) {
               /* If we have to compress right now the local morphological dictionary,
                * we must close it, sort it, call Compress and reopen it in append mode */
               u_fclose(info->morpho);
               pseudo_main_SortTxt(&vec,0,0,NULL,NULL,0,
                                   snt_files->morpho_dic,1);
               /* Then we compress it */
               pseudo_main_Compress(&vec,0,semitic,snt_files->morpho_dic,1);
               info->morpho=u_fopen(&vec,snt_files->morpho_dic,U_APPEND);
               if (info->morpho==NULL) {
                  fatal_error("");
               }
            }
            /* We dump and close output files */
            save_unknown_words(info);
        	u_fclose(info->dlf);
            u_fclose(info->dlc);
            u_fclose(info->err);
            info->dlf=NULL;
            info->dlc=NULL;
            info->err=NULL;
         }
	  }
   }
}
/* We process the tag sequences, if any */
u_printf("Sorting and saving tag sequences...\n");
save_and_sort_tag_sequences(info);

/* Finally, we have to save the definitive list of unknown words */
u_printf("Saving unknown words...\n");
if (info->err==NULL ) {
	info->err=u_fopen(&vec,snt_files->err,U_WRITE);
}
info->tags_err=u_fopen(&vec,snt_files->tags_err,U_WRITE);
save_unknown_words(info);

/* We compute some statistics */
save_statistics(&vec,snt_files->stat_dic_n,info);
u_printf("Done.\n");
/* And we free remaining things */
free_alphabet(alphabet);
free_text_tokens(tokens);
if (info->dlf!=NULL) u_fclose(info->dlf);
if (info->dlc!=NULL) u_fclose(info->dlc);
if (info->err!=NULL) u_fclose(info->err);
if (info->tags_err!=NULL) u_fclose(info->tags_err);
if (info->morpho!=NULL) {
   /* If we have produced a morpho.dic file, it's time to work with it */
   u_fclose(info->morpho);
   /* We sort it to remove duplicates */
   pseudo_main_SortTxt(&vec,0,0,NULL,NULL,0,
                       snt_files->morpho_dic,1);
   /* Then we compress it */
   pseudo_main_Compress(&vec,0,semitic,
		   snt_files->morpho_dic,1);
}

free_dico_application(info);
free_snt_files(snt_files);
if (morpho_dic!=NULL) free(morpho_dic);
free_OptVars(vars);
return ret;
}


/**
 * Looks for the given sequence in the .bin dictionaries. All entries are stored in the output,
 * regardless if they are simple or compound words. fst2 are skipped.
 * Returns 0 in case of success; 1 otherwise.
 */
int raw_dic_application(const VersatileEncodingConfig* vec,U_FILE* text,U_FILE* output,Alphabet* alphabet,int ind,char* const argv[]) {
unichar* sequence=readline_safe(text);
if (sequence==NULL) return 1;
if (sequence[0]=='\0') {
	free(sequence);
	return 1;
}
struct dico_application_info info;
memset(&info,0,sizeof(struct dico_application_info));
info.alphabet=alphabet;
info.dlf=output;
/* We all dictionaries according their priority */
for (int priority=1;priority<4;priority++) {
   /* For a given priority, we apply all concerned dictionaries
    * in their order on the command line */
   for (int i=ind;argv[i]!=NULL;i++) {
      char tmp[FILENAME_MAX];
      remove_extension(argv[i],tmp);
      char priority_mark=tmp[strlen(tmp)-1];
      if ((priority==1 && priority_mark=='-') ||  (priority==2 && priority_mark!='-' && priority_mark!='+') ||  (priority==3 && priority_mark=='+')) {
         /* If we must must process a dictionary, we check its type */
         char tmp2[FILENAME_MAX];
         get_extension(argv[i],tmp2);
         if (!strcmp(tmp2,".bin"))    {
        	 dico_application_simplified(vec,sequence,argv[i],&info);
         }
      }
   }
}
free(sequence);
return 0;
}

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
#include "getopt.h"
#include "Dico.h"



/**
 * This enhanced version of Dico was rewritten by Alexis Neme,
 * based on the original version written by S�bastien Paumier
 * 15 Novembre 2005
 * This new version take into account not only compiled dictionnary but also
 * local grammars representent by an FST2 as part of the lexical identification
 * stage.
 */


const char* usage_Dico =
         "Usage: Dico [OPTIONS] <dic_1> [<dic_2> <dic_3> ...]\n"
         "\n"
         "  <dic_i>: .bin dictionary or .fst2 local grammar to be applied\n"
         "\n"
         "OPTIONS:\n"
         "  -t TXT/--text=TXT: the .snt text file\n"
         "  -a ALPH/--alphabet=ALPH: the alphabet file\n"
         "  -m DICS/--morpho=DICS: specifies that DICS is the .bin dictionary\n"
         "                         list to use in Locate's morphological mode. .bin names are\n"
         "                         supposed to be separated with semi-colons.\n"
		 "  -j TABLE/--jamo=TABLE: specifies the jamo conversion table to use for Korean\n"
         "  -h/--help: this help\n"
         "\n"
         "Applies dictionaries and/or local grammars to the text and produces \n"
         "5 files, saved in the text directory. These files are:\n"
         "\n"
         "  dlf: simple entry dictionary\n"
         "  dlc: compound entry dictionary\n"
         "  err: unrecognized simple words\n"
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
         "\n"
         "The grammar output shall respect the file format of both DLF and DLC.\n"
         "If an output starts with a / character, it will be considered as a tag\n"
         "sequence to be put in the 'tags.ind' file. Such sequences are used\n"
         "by the Txt2Fst2 program in order to add paths to the text automaton.\n"
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
void save_statistics(char* name,struct dico_application_info* info) {
U_FILE* f=u_fopen(UTF16_LE,name,U_WRITE);
if (f==NULL) {
   error("Cannot write stat file %s\n",name);
   return;
}
u_fprintf(f,"%d\n%d\n%d\n",info->SIMPLE_WORDS,info->COMPOUND_WORDS,info->UNKNOWN_WORDS);
u_fclose(f);
}


const char* optstring_Dico=":t:a:m:j:h";
const struct option_TS lopts_Dico[]= {
      {"text",required_argument_TS,NULL,'t'},
      {"alphabet",required_argument_TS,NULL,'a'},
      {"morpho",required_argument_TS,NULL,'m'},
      {"jamo",required_argument_TS,NULL,'j'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_Dico(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

int ret=0;
int val,index=-1;
char alph[FILENAME_MAX]="";
char text[FILENAME_MAX]="";
char* morpho_dic=NULL;
char jamo[FILENAME_MAX]="";
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Dico,lopts_Dico,&index,vars))) {
   switch(val) {
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
                morpho_dic=strdup(vars->optarg);
                if (morpho_dic==NULL) {
                   fatal_alloc_error("main_Dico");
                }
             }
             break;
   case 'j': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty jamo table file name\n");
             }
             strcpy(jamo,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Dico[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
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

struct snt_files* snt_files=new_snt_files(text);
U_FILE* text_cod;
struct text_tokens* tokens;

/* And we create empty files in order to append things to them */
if (!u_fempty(UTF16_LE,snt_files->dlf)) {
   fatal_error("Cannot create %s\n",snt_files->dlf);
}
if (!u_fempty(UTF16_LE,snt_files->dlc)) {
   fatal_error("Cannot create %s\n",snt_files->dlc);
}
if (!u_fempty(UTF16_LE,snt_files->err)) {
   fatal_error("Cannot create %s\n",snt_files->err);
}
Alphabet* alphabet=NULL;
if (alph[0]!='\0') {
   /* We load the alphabet */
   alphabet=load_alphabet(alph);
   if (alphabet==NULL) {
      error("Cannot open alphabet file %s\n",alph);
      return 1;
   }
}
/* We load the text tokens */
tokens=load_text_tokens(snt_files->tokens_txt);
if (tokens==NULL) {
   free_alphabet(alphabet);
   error("Cannot open token file %s\n",snt_files->tokens_txt);
   return 1;
}
/* We open the text.cod file for binary reading */
text_cod=u_fopen(BINARY,snt_files->text_cod,U_READ);
if (text_cod==NULL) {
   free_alphabet(alphabet);
   free_text_tokens(tokens);
   error("Cannot open coded text file %s\n",snt_files->text_cod);
   return 1;
}
u_printf("Initializing...\n");
struct dico_application_info* info=init_dico_application(tokens,NULL,NULL,NULL,snt_files->tags_ind,text_cod,alphabet);

/* First of all, we compute the number of occurrences of each token */
u_printf("Counting tokens...\n");
count_token_occurrences(info);
/* We save optind since it is a global variable that can be modified by Locate */
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
         if (!strcmp(tmp2,".bin"))    {
            /*
             * If it is a .bin dictionary
             */
	         u_printf("Applying dico  %s...\n",argv[i]);
            /* We open output files: dictionaries in APPEND mode since we
             * can only add entries to them, and 'err' in WRITE mode because
             * each dictionary application may reduce this file */
            info->dlf=u_fopen(UTF16_LE,snt_files->dlf,U_APPEND);
            info->dlc=u_fopen(UTF16_LE,snt_files->dlc,U_APPEND);
            info->err=u_fopen(UTF16_LE,snt_files->err,U_WRITE);
            /* Working... */
            if (dico_application(argv[i],info,priority) != 0)
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
         else if (!strcmp(tmp2,".fst2"))       {
            /*
             * If it is a .fst2 dictionary
             */
            int l=(int)(strlen(tmp)-((priority==2)?1:2));
            OutputPolicy policy=MERGE_OUTPUTS;
            if (l>0 && (tmp[l]=='r' || tmp[l]=='R') && tmp[l-1]=='-') {
               policy=REPLACE_OUTPUTS;
            }
            u_printf("Applying grammar %s...\n",argv[i]);
            /**
             * IMPORTANT!!!
             * dlf, dlc and err must not be open while launch_locate_as_routine
             * is running, because this function tries to read in these files.
             */
            launch_locate_as_routine(text,argv[i],alph,policy,morpho_dic,1,(jamo[0]!='\0')?jamo:NULL);
	         /* We open output files: dictionaries in APPEND mode since we
             * can only add entries to them, and 'err' in WRITE mode because
             * each dictionary application may reduce this file */
            info->dlf=u_fopen(UTF16_LE,snt_files->dlf,U_APPEND);
            info->dlc=u_fopen(UTF16_LE,snt_files->dlc,U_APPEND);
            info->err=u_fopen(UTF16_LE,snt_files->err,U_WRITE);
            /* And we merge the Locate results with current dictionaries */
            merge_dic_locate_results(info,snt_files->concord_ind,priority);
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
	info->err=u_fopen(UTF16_LE,snt_files->err,U_WRITE);
}
save_unknown_words(info);
/* We compute some statistics */
save_statistics(snt_files->stat_dic_n,info);
u_printf("Done.\n");
/* And we free remaining things */
free_alphabet(alphabet);
free_text_tokens(tokens);
if (info->dlf!=NULL) u_fclose(info->dlf);
if (info->dlc!=NULL) u_fclose(info->dlc);
if (info->err!=NULL) u_fclose(info->err);
u_fclose(text_cod);
free_dico_application(info);
free_snt_files(snt_files);
if (morpho_dic!=NULL) free(morpho_dic);
free_OptVars(vars);
return ret;
}

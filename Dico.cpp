/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
		 "  -K/--korean: tells Dico that it works on Korean\n"
         "  -u X/--arabic_rules=X: Arabic typographic rule configuration file\n"
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
         "If one grammar name or more is ended by -b -b+ -b- -rb -rb+  or -rb-,\n"
         "all its DELAF outputs will also be copied in a file named morpho.dic located\n"
         "in the text directory. At the end of Dico, this dictionary will be compressed\n"
         "to produce the morpho.bin file that will be used by Locate as a local morphological\n"
         "dictionary for the current text.\n"
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
void save_statistics(Encoding encoding_output,int bom_output,char* name,struct dico_application_info* info) {
U_FILE* f=u_fopen_creating_versatile_encoding(encoding_output,bom_output,name,U_WRITE);
if (f==NULL) {
   error("Cannot write stat file %s\n",name);
   return;
}
u_fprintf(f,"%d\n%d\n%d\n",info->SIMPLE_WORDS,info->COMPOUND_WORDS,info->UNKNOWN_WORDS);
u_fclose(f);
}


const char* optstring_Dico=":t:a:m:Khk:q:u:";
const struct option_TS lopts_Dico[]= {
      {"text",required_argument_TS,NULL,'t'},
      {"alphabet",required_argument_TS,NULL,'a'},
      {"morpho",required_argument_TS,NULL,'m'},
      {"korean",no_argument_TS,NULL,'K'},
      {"help",no_argument_TS,NULL,'h'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"arabic_rules",required_argument_TS,NULL,'u'},
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
char* morpho_dic=NULL;
int is_korean=0;
Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
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
   case 'K': is_korean=1;
             break;
   case 'h': usage(); return 0;
   case 'u': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty arabic rule configuration file name\n");
             }
             strcpy(arabic_rules,vars->optarg);
             break;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Dico[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
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
if (!u_fempty(encoding_output,bom_output,snt_files->dlf)) {
   fatal_error("Cannot create %s\n",snt_files->dlf);
}
if (!u_fempty(encoding_output,bom_output,snt_files->dlc)) {
   fatal_error("Cannot create %s\n",snt_files->dlc);
}
if (!u_fempty(encoding_output,bom_output,snt_files->err)) {
   fatal_error("Cannot create %s\n",snt_files->err);
}
/* We remove the text morphological dictionary files, if any */
remove(snt_files->morpho_dic);
remove(snt_files->morpho_bin);
remove(snt_files->morpho_inf);
Alphabet* alphabet=NULL;
if (alph[0]!='\0') {
   /* We load the alphabet */
   alphabet=load_alphabet(alph,is_korean);
   if (alphabet==NULL) {
      error("Cannot open alphabet file %s\n",alph);
      return 1;
   }
}
/* We load the text tokens */
tokens=load_text_tokens(snt_files->tokens_txt,mask_encoding_compatibility_input);
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
struct dico_application_info* info=init_dico_application(tokens,NULL,NULL,NULL,NULL,snt_files->tags_ind,snt_files->text_cod,alphabet,encoding_output,bom_output,mask_encoding_compatibility_input);

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
         if (!strcmp(tmp2,".bin"))    {
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
            info->dlf=u_fopen_versatile_encoding(encoding_output,bom_output,mask_encoding_compatibility_input | ALL_ENCODING_BOM_POSSIBLE,snt_files->dlf,U_APPEND);
            info->dlc=u_fopen_versatile_encoding(encoding_output,bom_output,mask_encoding_compatibility_input | ALL_ENCODING_BOM_POSSIBLE,snt_files->dlc,U_APPEND);
            info->err=u_fopen_creating_versatile_encoding(encoding_output,bom_output,snt_files->err,U_WRITE);
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
         else if (!strcmp(tmp2,".fst2")) {
            /*
             * If it is a .fst2 dictionary
             */
            int l=(int)(strlen(tmp)-((priority==2)?1:2));
            OutputPolicy policy=MERGE_OUTPUTS;
            int export_in_morpho_dic=DONT_PRODUCE_MORPHO_DIC;
            if (l>0 && (tmp[l]=='b' || tmp[l]=='B' || tmp[l]=='z' || tmp[l]=='Z')
                  && (tmp[l-1]=='-' ||
                        (l>1 && (tmp[l-1]=='r' || tmp[l-1]=='R') && tmp[l-2]=='-'))) {
               export_in_morpho_dic=PRODUCE_MORPHO_DIC_AT_THE_END;
               if (tmp[l]=='z' || tmp[l]=='Z') {
                  export_in_morpho_dic=PRODUCE_MORPHO_DIC_NOW;
               }
               l--;
            }
            if (l>0 && (tmp[l]=='r' || tmp[l]=='R') && tmp[l-1]=='-') {
               policy=REPLACE_OUTPUTS;
            }
            u_printf("Applying grammar %s...\n",argv[i]);
            /**
             * IMPORTANT!!!
             * dlf, dlc and err must not be open while launch_locate_as_routine
             * is running, because this function tries to read in these files.
             */
            launch_locate_as_routine(encoding_output,bom_output,mask_encoding_compatibility_input,
            		text,argv[i],alph,policy,morpho_dic,1,is_korean,arabic_rules);
	         /* We open output files: dictionaries in APPEND mode since we
             * can only add entries to them, and 'err' in WRITE mode because
             * each dictionary application may reduce this file */
            info->dlf=u_fopen_versatile_encoding(encoding_output,bom_output,mask_encoding_compatibility_input | ALL_ENCODING_BOM_POSSIBLE,snt_files->dlf,U_APPEND);
            info->dlc=u_fopen_versatile_encoding(encoding_output,bom_output,mask_encoding_compatibility_input | ALL_ENCODING_BOM_POSSIBLE,snt_files->dlc,U_APPEND);
            info->err=u_fopen_creating_versatile_encoding(encoding_output,bom_output,snt_files->err,U_WRITE);
            if (export_in_morpho_dic!=DONT_PRODUCE_MORPHO_DIC) {
               /* If necessary, we deal with the morpho.dic file */
               if (info->morpho==NULL) {
                  /* We create it if needed */
                  info->morpho=u_fopen_versatile_encoding(encoding_output,bom_output,mask_encoding_compatibility_input | ALL_ENCODING_BOM_POSSIBLE,snt_files->morpho_dic,U_WRITE);
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
               pseudo_main_SortTxt(encoding_output,bom_output,ALL_ENCODING_BOM_POSSIBLE,0,0,NULL,NULL,0,
                                   snt_files->morpho_dic);
               /* Then we compress it */
               pseudo_main_Compress(encoding_output,bom_output,ALL_ENCODING_BOM_POSSIBLE,0,snt_files->morpho_dic);
               info->morpho=u_fopen_versatile_encoding(encoding_output,bom_output,mask_encoding_compatibility_input | ALL_ENCODING_BOM_POSSIBLE,snt_files->morpho_dic,U_APPEND);
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
	info->err=u_fopen_creating_versatile_encoding(encoding_output,bom_output,snt_files->err,U_WRITE);
}
save_unknown_words(info);
/* We compute some statistics */
save_statistics(encoding_output,bom_output,snt_files->stat_dic_n,info);
u_printf("Done.\n");
/* And we free remaining things */
free_alphabet(alphabet);
free_text_tokens(tokens);
if (info->dlf!=NULL) u_fclose(info->dlf);
if (info->dlc!=NULL) u_fclose(info->dlc);
if (info->err!=NULL) u_fclose(info->err);
if (info->morpho!=NULL) {
   /* If we have produced a morpho.dic file, it's time to work with it */
   u_fclose(info->morpho);
   /* We sort it to remove duplicates */
   pseudo_main_SortTxt(encoding_output,bom_output,ALL_ENCODING_BOM_POSSIBLE,0,0,NULL,NULL,0,
                       snt_files->morpho_dic);
   /* Then we compress it */
   pseudo_main_Compress(encoding_output,bom_output,ALL_ENCODING_BOM_POSSIBLE,0,snt_files->morpho_dic);
}

free_dico_application(info);
free_snt_files(snt_files);
if (morpho_dic!=NULL) free(morpho_dic);
free_OptVars(vars);
return ret;
}

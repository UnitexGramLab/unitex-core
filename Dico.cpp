 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "IOBuffer.h"
#include "LocateAsRoutine.h"
#include "Error.h"
#include "Snt.h"
#include "LocateConstants.h"


/**
 * This enhanced  version of Dico was rewritten by Alexis Neme,
 * based on the original version written by Sébastien Paumier
 * 15 Novembre 2005
 * This new version take into account not only compiled dictionnary but also
 * local grammars representent by an FST2 as part of the lexical identification
 * stage.
 */


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Dico <text> <alphabet> <dic-fst_1> [<dic-fst_2> <dic-fst_3> ...]\n");
u_printf("     <text>      : the text file\n");
u_printf("     <alphabet>  : the alphabet file\n");
u_printf("     <dic-fst_i> : name of dictionary or local grammar to be applied\n\n");

u_printf("Applies dictionaries and/or local grammars to the text and produces \n");
u_printf("3 files, saved in the text directory. These files are:\n\n");
u_printf(" DLF : simple entry dictionary\n");
u_printf(" DLC : compound entry dictionary\n");
u_printf(" ERR : unrecognized simple words\n\n");

u_printf("There are 3 levels of priority. If the dictionary name ends with \"-\",\n");
u_printf("it will be applied with the maximum priority. If the suffix\n");
u_printf("is \"+\", the priority is minimal. If there is no suffix, the priority\n");
u_printf("is normal. Any lexical unit identified in a previous stage will be ignored\n");
u_printf("in the subsequent stages. \n\n");

u_printf("The local grammars are represented by finite state transducers(.fst2).\n");
u_printf("These grammars will be applied in MERGE mode, except if a .fst2 ends\n");
u_printf("with -r. In that case, it will be applied in REPLACE mode.\n");
u_printf("Note that -r can be combined with - or + priority marks (-r- and -r+)\n\n");
u_printf("The grammar output shall respect the file format of both DLF and DLC. \n\n");

u_printf("The numbers of simple, compound and unknown forms are saved\n");
u_printf("in a file named stat_dic.n which is created in the text directory.\n");

u_printf("\nExamples:\n");
u_printf(" - Dico \"c:\\tutu.snt\" \"c:\\Alphabet.txt\" Dela.bin MyFilter-.bin\n");
u_printf("This command will apply first MyFilter-.bin and then Dela.bin.\n");
u_printf(" - Dico \"c:\\tutu.snt\" \"c:\\Alphabet.txt\" Dela.bin Rom_numbs-.fst2 numbers+.fst2\n");
u_printf("This command will apply Rom_numbs-.fst2 then Dela.bin and finally\n");
u_printf("numbers+.fst2\n\n");

u_printf("\nNote: the 3 resulting files (DLF, DLC and ERR) are stored in the text\n");
u_printf("directory. THEY ARE NOT SORTED AND MAY CONTAIN DUPLICATES. Use the\n");
u_printf("SortTxt program to clean these files.\n\n");
}


/**
 * This function stores some statistics in 'stat_dic.n'.
 */
void save_statistics(char* name,struct dico_application_info* info) {
FILE* f=u_fopen(name,U_WRITE);
if (f==NULL) {
   error("Cannot write stat file %s\n",name);
   return;
}
u_fprintf(f,"%d\n%d\n%d\n",info->SIMPLE_WORDS,info->COMPOUND_WORDS,info->UNKNOWN_WORDS);
u_fclose(f);
}


int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

if (argc<4) {
   usage();
   return 0;
}

struct snt_files* snt_files=new_snt_files(argv[1]);
FILE* text_cod;
struct text_tokens* tokens;        

/* And we create empty files in order to append things to them */
if (!u_fempty(snt_files->dlf)) {
   fatal_error("Cannot create %s\n",snt_files->dlf);
}
if (!u_fempty(snt_files->dlc)) {
   fatal_error("Cannot create %s\n",snt_files->dlc);
}
if (!u_fempty(snt_files->err)) {
   fatal_error("Cannot create %s\n",snt_files->err);
}
/* We load the alphabet */
Alphabet* alphabet=load_alphabet(argv[2]);
if (alphabet==NULL) {
   error("Cannot open alphabet file %s\n",argv[2]);
   return 1;
}
/* We load the text tokens */
tokens=load_text_tokens(snt_files->tokens_txt);
if (tokens==NULL) {
   free_alphabet(alphabet);
   error("Cannot open token file %s\n",snt_files->tokens_txt);
   return 1;
}
/* We open the text.cod file for binary reading */
text_cod=fopen(snt_files->text_cod,"rb");
if (text_cod==NULL) {
   free_alphabet(alphabet);
   free_text_tokens(tokens);
   error("Cannot open coded text file %s\n",snt_files->text_cod);
   return 1;
}
u_printf("Initializing...\n");
struct dico_application_info* info=init_dico_application(tokens,NULL,NULL,NULL,text_cod,alphabet);
/* First of all, we compute the number of occurrences of each token */
u_printf("Counting tokens...\n");
count_token_occurrences(info);
/* We all dictionaries according their priority */
for (int priority=1;priority<4;priority++) {
   /* For a given priority, we apply all concerned dictionaries 
    * in their order on the command line */
   for (int i=3;i<argc;i++) {
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
            info->dlf=u_fopen(snt_files->dlf,U_APPEND);
            info->dlc=u_fopen(snt_files->dlc,U_APPEND);
            info->err=u_fopen(snt_files->err,U_WRITE);
            /* Working... */
            dico_application(argv[i],info,priority);
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
            int l=strlen(tmp)-((priority==2)?1:2);
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
            launch_locate_as_routine(argv[1],argv[i],argv[2],policy);
	         /* We open output files: dictionaries in APPEND mode since we
             * can only add entries to them, and 'err' in WRITE mode because
             * each dictionary application may reduce this file */
            info->dlf=u_fopen(snt_files->dlf,U_APPEND);
            info->dlc=u_fopen(snt_files->dlc,U_APPEND);
            info->err=u_fopen(snt_files->err,U_WRITE);
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
/* Finally, we have to save the definitive list of unknown words */
u_printf("Saving unknown words...\n");
if (info->err==NULL ) {
	info->err=u_fopen(snt_files->err,U_WRITE);  
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
fclose(text_cod);
free_dico_application(info);
free_snt_files(snt_files);
return 0;
}



 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "DELA.h"
#include "String_hash.h"
#include "unicode.h"
#include "Alphabet.h"
#include "Text_tokens.h"
#include "Dico_application.h"
#include "Liste_nombres.h"
#include "FileName.h"
#include "Table_complex_token_hash.h"
#include "Fst2.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "LocateAsRoutine.h"
#include "Error.h"

//  This enhanced  version of dico.exe was rewritten by Alexis Neme,
//  based on the original version written by Sebastien Paumier
//  15 Novembre 2005
//  This new version take into account not only compiled dictionnary but also
//  local grammars representent by an FST as part of the Lexical identification stage
//---------------------------------------------------------------------------
//
// "e:\my unitex\french\corpus\femme.snt" "e:\my unitex\french\alphabet.txt" e:\unitex\french\dela\DELA.bin
//
// THAI:
// e:\unitex\thai\corpus\corpus.txt e:\unitex\thai\alphabet_thai.txt e:\DELA\thai\DELA.txt
//

//---------------------------------------------------------------------------
void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Dico <text> <alphabet> <dic-fst_1> [<dic-fst_2> <dic-fst_3> ...]\n");
printf("     <text>      : the text file\n");
printf("     <alphabet>  : the alphabet file\n");
printf("     <dic-fst_i> : name of dictionary or local grammar to be applied\n\n");

printf("Applies dictionaries and/or local grammars to the text and produces \n");
printf("3 files, saved in the text directory. These files are:\n\n");
printf(" DLF : simple entry dictionary\n");
printf(" DLC : compound entry dictionary\n");
printf(" ERR : unrecognized simple words\n\n");

printf("There are 3 levels of priority. If the dictionary name ends with \"-\",\n");
printf("it will be applied with the maximum priority. If the suffix\n");
printf("is \"+\", the priority is minimal. If there is no suffix, the priority\n");
printf("is normal. Any lexical unit identified in a previous stage will be ignored\n");
printf("in the subsequent stages. \n\n");

printf("The local grammars are represented by finite state transducers(.fst2).\n");
printf("These grammars will be applied in MERGE mode.\n");
printf("The grammar output shall respect the file format of both DLF and DLC. \n\n");

printf("The numbers of simple, compound and unknown forms are saved\n");
printf("in a file named stat_dic.n which is created in the text directory.\n");

printf("\nExamples:\n");
printf(" - Dico \"c:\\tutu.snt\" \"c:\\Alphabet.txt\" Dela.bin MyFilter-.bin\n");
printf("This command will apply first MyFilter-.bin and then Dela.bin.\n");
printf(" - Dico \"c:\\tutu.snt\" \"c:\\Alphabet.txt\" Dela.bin Rom_numbs-.fst2 numbers+.fst2\n");
printf("This command will apply Rom_numbs-.fst2 then Dela.bin and finally\n");
printf("numbers+.fst2\n\n");

printf("\nNote: the 3 resulting files (DLF, DLC and ERR) are stored in the text\n");
printf("directory. THEY ARE NOT SORTED AND MAY CONTAIN DUPLICATES. Use the\n");
printf("SortTxt program to clean these files.\n\n");
}





/**
 * This function initializes the names of the files associated to a .snt text
 * 
 * Example: text_snt="/tmp/myunitex/English/Corpus/foo.snt"
 * => dlf="/tmp/myunitex/English/Corpus/foo_snt/dlf"
 * => dlc="/tmp/myunitex/English/Corpus/foo_snt/dlc"
 * etc.
 * 
 * @author Alexis Neme
 * Modified by Sébastien Paumier
 */
#warning move this function into a separate file
void set_text_file_names(char* text_snt,char* name_dlf,char* name_dlc,char* name_err,
                         char* name_text_cod,char* name_tokens,char* name_concord_ind) {
char text_snt_path[FILENAME_SIZE];
/* First, we compute the path of the _snt directory of the text */
get_snt_path(text_snt,text_snt_path);
/* And we use it to produce the output file names */
strcpy(name_dlf,text_snt_path);
strcat(name_dlf,"dlf");
strcpy(name_dlc,text_snt_path);
strcat(name_dlc,"dlc");
strcpy(name_err,text_snt_path);
strcat(name_err,"err");
strcpy(name_text_cod,text_snt_path);
strcat(name_text_cod,"text.cod");
strcpy(name_tokens,text_snt_path);
strcat(name_tokens,"tokens.txt");
strcpy(name_concord_ind,text_snt_path);
strcat(name_concord_ind,"concord.ind");
}


void compute_stat_dic(char *text_snt, char *stat_dic_filename, int SIMPLE_WORDS, int COMPOUND_WORDS, int ERRORS)
{
	FILE * stat;
	char tmp_filename[200];
	get_snt_path(text_snt,tmp_filename);
	strcat(tmp_filename,stat_dic_filename);
	stat=u_fopen(tmp_filename,U_WRITE);
	if (stat==NULL) {
	   fprintf(stderr,"Cannot write %s\n",tmp_filename);
	}
	else {
	   unichar tmp[100];
	   u_int_to_string(SIMPLE_WORDS,tmp);
	   u_strcat_char(tmp,"\n");
	   u_fprints(tmp,stat);
	   u_int_to_string(COMPOUND_WORDS,tmp);
	   u_strcat_char(tmp,"\n");
	   u_fprints(tmp,stat);
	   u_int_to_string(ERRORS,tmp);
	   u_strcat_char(tmp,"\n");
	   u_fprints(tmp,stat);
	   u_fclose(stat);
	}

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

/* text.cod is the file that contains the text in the form of a 
 * token number sequence */
char name_text_cod[FILENAME_SIZE]; 
FILE* text_cod;

/* tokens.txt is the file that contains all the distinct tokens of the text */
char name_tokens[FILENAME_SIZE];
struct text_tokens* tokens;        

/* dlf is the output dictionary for simple words */
char name_dlf[FILENAME_SIZE];
/* dlc is the output dictionary for compound words */
char name_dlc[FILENAME_SIZE];
/* err is the output dictionary for unknown words */
char name_err[FILENAME_SIZE];
/* concord.ind is the file produced by Locate */
char name_concord_ind[FILENAME_SIZE];

/* We set the text file names */
set_text_file_names(argv[1],name_dlf,name_dlc,name_err,name_text_cod,name_tokens,name_concord_ind);
/* And we create empty files in order to append things to them */
u_fempty(name_dlf);
u_fempty(name_dlc);
u_fempty(name_err);
/* We load the alphabet */
Alphabet* alphabet=load_alphabet(argv[2]);
if (alphabet==NULL) {
   error("Cannot open alphabet file %s\n",argv[2]);
   return 1;
}
/* We load the text tokens */
tokens=load_text_tokens(name_tokens);
if (tokens==NULL) {
   free_alphabet(alphabet);
   error("Cannot open token file %s\n",name_tokens);
   return 1;
}
/* We open the text.cod file for binary reading */
text_cod=fopen(name_text_cod,"rb");
if (text_cod==NULL) {
   free_alphabet(alphabet);
   free_text_tokens(tokens);
   error("Cannot open coded text file %s\n",name_text_cod);
   return 1;
}
printf("Initializing...\n");
struct dico_application_info* info=init_dico_application(tokens,NULL,NULL,NULL,text_cod,alphabet);
/* We all dictionaries according their priority */
for (int priority=1;priority<4;priority++) {
   /* For a given priority, we apply all concerned dictionaries 
    * in their order on the command line */
   for (int i=3;i<argc;i++) {
      char tmp[FILENAME_SIZE];
      name_without_extension(argv[i],tmp);
      char priority_mark=tmp[strlen(tmp)-1];
      if ((priority==1 && priority_mark=='-') ||  (priority==2 && priority_mark!='-' && priority_mark!='+') ||  (priority==3 && priority_mark=='+')) {
         /* If we must must process a dictionary, we check its type */
         file_name_extension(argv[i],tmp);
         if (!strcmp(tmp,".bin"))    {    
            /*
             * If it is a .bin dictionary
             */
	         printf("Applying dico  %s...\n",argv[i]);
            /* We open output files: dictionaries in APPEND mode since we
             * can only add entries to them, and 'err' in WRITE mode because
             * each dictionary application may reduce this file */
            info->dlf=u_fopen(name_dlf,U_APPEND);
            info->dlc=u_fopen(name_dlc,U_APPEND);
            info->err=u_fopen(name_err,U_WRITE);
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
         else if (!strcmp(tmp,".fst2"))       { 
            /*
             * If it is a .fst2 dictionary
             */
            printf("Applying %s...\n",argv[i]);
            /**
             * IMPORTANT!!!
             * dlf, dlc and err must not be open while launch_locate_as_routine
             * is running, because this function tries to read in these files.
             */
            launch_locate_as_routine(argv[1],argv[i],argv[2]);
	         /* We open output files: dictionaries in APPEND mode since we
             * can only add entries to them, and 'err' in WRITE mode because
             * each dictionary application may reduce this file */
            info->dlf=u_fopen(name_dlf,U_APPEND);
            info->dlc=u_fopen(name_dlc,U_APPEND);
            info->err=u_fopen(name_err,U_WRITE);
            /* And we merge the Locate results with current dictionaries */
            merge_dic_locate_results(info,name_concord_ind,priority);
            // dumping and closing output files
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
/* Finally, we hav to save the definitive list of unknown words */
printf("Saving unknown words...\n");
if (info->err == NULL ) {
	info->err=u_fopen(name_err,U_WRITE);  
} 
#warning compute the number of occurrences of each token, since it is not 
#warning done anymore if we only apply a .fst2 dictionary
save_unknown_words(info);
/* We compute some statistics */
//printf("computing dictionnary statistics ...\n");
//compute_stat_dic(argv[1],"stat_dic.n", SIMPLE_WORDS, COMPOUND_WORDS, ERRORS);
printf("Done.\n");
/* And we free remaining things */
free_alphabet(alphabet);
free_text_tokens(tokens);
if (info->dlf!=NULL) u_fclose(info->dlf);
if (info->dlc!=NULL) u_fclose(info->dlc);
if (info->err!=NULL) u_fclose(info->err);
fclose(text_cod);
free_dico_application(info);
return 0;

}


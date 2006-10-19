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
#include "Matches.h"
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
printf("directory. THEY ARE NOT SORTED. Use the SortTxt program to.\n\n");
}

/**
 * @author Alexis Neme
 */
 void   Launch_dico_application(char *nom_dico,FILE *text,struct text_tokens* tok,Alphabet* alph,
	                            FILE*  dlf,  FILE* dlc, FILE*  err,int priority)
 { 
  struct INF_codes* INF;
  unsigned char* bin; 

  char nom_bin[1000],nom_inf[1000];
 	name_without_extension(nom_dico,nom_inf);
 
    strcpy(nom_bin,nom_inf);
	strcat(nom_inf,".inf");
	strcat(nom_bin,".bin");

	INF=load_INF_file(nom_inf);
	if (INF==NULL) {
	   fprintf(stderr,"Cannot open %s\n",nom_inf);
	}
	else {
	   bin=load_BIN_file(nom_bin);
	   if (bin==NULL) {
		  free_INF_codes(INF);
		  fprintf(stderr,"Cannot open %s\n",nom_bin);
		  }
	   else {
		  dico_application(bin,INF,text,tok,alph,dlf,dlc,err,priority);
		  free_INF_codes(INF);
		  free(bin);
	   }
	}
} 




/**
 * @author Alexis Neme
 */
int	MergeDicLocateResults(char *text_snt, text_tokens* tok, char *Concord_filename, int priority)
{	
FILE *dlf_,*dlc_;
char tmp_filename[2000];
int token_tab_coumpounds[100];                  // cinquante-deux = (1347,35,582,-1) 

printf("Merging dic/locate result...\n");

// open dlf and dlc in append mode
get_snt_path(text_snt, tmp_filename);
strcat(tmp_filename,"dlf") ;
dlf_=u_fopen(tmp_filename,U_APPEND);
get_snt_path(text_snt, tmp_filename);
strcat(tmp_filename,"dlc") ;
dlc_=u_fopen(tmp_filename,U_APPEND);


// open the concord.ind file and load matches in l
get_snt_path(text_snt, tmp_filename);
strcat(tmp_filename,Concord_filename) ;		
FILE* f1=u_fopen(tmp_filename,U_READ);
if (f1==NULL) {
fprintf(stderr,"Cannot	open %s\n",tmp_filename);
	return 0;
}
int MODE1;
struct liste_matches* l = load_match_list(f1,&MODE1);
u_fclose(f1);


unichar tempstr[200];
unichar naked_token[200];

while (l!=NULL) {
    struct dela_entry* foo=tokenize_DELAF_line(l->output,1);
    if (foo!=NULL) {
    free_dic_entry(foo);
    u_extractEntryFromConcordOutput(l->output,naked_token);
   // simple words 
   if(l->debut == l->fin) {  
		// case of simple forms
      int tok_numb= get_token_number( naked_token, tok) ; // 
      int p=token_has_been_processed(tok_numb);

      if (p==0) {     // we save the token only if it hasn't been processed  the good priority
		set_part_of_a_word(tok_numb,priority);     			
		set_has_been_processed(tok_numb,priority);
	  }
	  //write the dlf									
	  u_strcpy(tempstr, l->output);
	  u_strcat_char(tempstr,"\n"); 
	  u_fprints( tempstr, dlf_);
   }

  // coumpounds forms
  if(l->debut < l->fin) 	{
	build_complex_token_tab(naked_token,tok, token_tab_coumpounds); // sans raison = (121,1,1643,-1,priorite)		  
	int w =  was_allready_in_tct_hash(token_tab_coumpounds,tct_h,priority);
	  
	if (w==0 || w==priority) {
     for (int k=0;token_tab_coumpounds[k]!=-1;k++) {
        // if we have matched a compound, then all its part all not unknown words
        set_part_of_a_word(token_tab_coumpounds[k],priority);
     }
    } 
	  u_strcpy(tempstr, l->output);
	  u_strcat_char(tempstr,"\n"); 
	  u_fprints( tempstr, dlc_);
  } 
  }
 l= l->suivant;
} // while l== null

u_fclose(dlf_);
u_fclose(dlc_);
return 1;

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
                         char* name_text_cod,char* name_tokens) {
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

/* Names of the .bin and .inf files that compose a dictionary */
char name_bin[FILENAME_SIZE];
char name_inf[FILENAME_SIZE];

/* text.cod is the file that contains the text in the form of a 
 * token number sequence */
char name_text_cod[FILENAME_SIZE]; 
FILE* text_cod;

/* tokens.txt is the file that contains all the distinct tokens of the text */
char name_tokens[FILENAME_SIZE];
struct text_tokens* tokens;        

/* dlf is the output dictionary for simple words */
char name_dlf[FILENAME_SIZE];
FILE* dlf;
/* dlc is the output dictionary for compound words */
char name_dlc[FILENAME_SIZE];
FILE* dlc;
/* err is the output dictionary for unknown words */
char name_err[FILENAME_SIZE];
FILE* err;

/* We set the text file names */
set_text_file_names(argv[1],name_dlf,name_dlc,name_err,name_text_cod,name_tokens);
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
init_dico_application(tokens,dlf,dlc,err,text_cod,alphabet);
dlf=dlc=err=NULL;
free_text_DICO();

for (int priority=1;priority<4;priority++) {
   for (int i=3;i<argc;i++) {
      // we do a loop to apply several dictionaries
      name_without_extension(argv[i],name_inf);
      char C=name_inf[strlen(name_inf)-1];
	  if ((priority==1 && C=='-') ||  (priority==2 && C!='-' && C!='+') ||  (priority==3 && C=='+')) {
         file_name_extension(argv[i],name_bin);

		// a Dico.bin
		 if (!strcmp(name_bin,".bin"))    {    
	         printf("Applying dico  %s...\n",argv[i]);
	         // opening output files
			 dlf =   u_fopen(name_dlf,U_APPEND);
             dlc =   u_fopen(name_dlc,U_APPEND);  
             err =   u_fopen(name_err,U_WRITE);  
			 assign_text_DICO(dlf,dlc,err);
			 // working...
            Launch_dico_application(argv[i],text_cod,tokens,alphabet,dlf,dlc,err,priority);
            // dumping and closing output files
            sauver_mots_inconnus();
        	u_fclose(dlc); u_fclose(dlf);
            u_fclose(err); 
            dlf=dlc=err=NULL;
            free_text_DICO();                   // DLF, DLC, ERR = NULL
         }  
         // a FST grammar 
         if (!strcmp(name_bin,".fst2"))       { 
            printf("Applying %s...\n",argv[i]);
            /**
             * IMPORTANT!!!
             * dlf, dlc and err must not be open while LaunchLocateAsRoutine
             * is running, because this function tries to read in these files.
             *
             */
			LaunchLocateAsRoutine(argv,i);             // i is the rank of the fst that we need to locates patterns					
	         // opening output files
			 dlf =   u_fopen(name_dlf,U_APPEND);
             dlc =   u_fopen(name_dlc,U_APPEND);  
             err =   u_fopen(name_err,U_WRITE);  
			 assign_text_DICO(dlf,dlc,err);
			MergeDicLocateResults(argv[1], tokens, "concord.ind",  priority);
            // dumping and closing output files
            sauver_mots_inconnus();
        	u_fclose(dlc); u_fclose(dlf);
            u_fclose(err); 
            dlf=dlc=err=NULL;
            free_text_DICO();                   // DLF, DLC, ERR = NULL
         }
	  }
   }
}

// =====  Closing stage 

printf("Saving unknown words...\n");

if (err == NULL ) {  	
    dlf =  u_fopen(name_dlf,U_APPEND);  
	dlc =  u_fopen(name_dlc,U_APPEND);  
	err =  u_fopen(name_err,U_WRITE);  
	assign_text_DICO(dlf,dlc,err);
} 

sauver_mots_inconnus();

//printf("computing dictionnary statistics ...\n");
//compute_stat_dic(argv[1],"stat_dic.n", SIMPLE_WORDS, COMPOUND_WORDS, ERRORS);

printf("Done.\n");

free_dico_application();
free_alphabet(alphabet);
free_text_tokens(tokens);
u_fclose(dlf);
u_fclose(dlc);
u_fclose(err);
fclose(text_cod);
free_text_DICO();
//getchar();
return 0;

}
//---------------------------------------------------------------------------

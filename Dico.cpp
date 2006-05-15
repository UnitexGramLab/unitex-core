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
#include "Token_tree.h"
#include "Table_complex_token_hash.h"
#include "Fst2.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "LocateAsRoutine.h"
#include "Matches.h"

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
    struct dela_entry* foo=tokenize_DELAF_line(l->output);
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
 * @author Alexis Neme
 */
int Init_OutputDic_dlf_dlc_err(char *text_snt,char  *nom_dlf,char  *nom_dlc,char  *nom_err) 
{	// creates a blank files for appending in next stages

	FILE *dico;
	char textpath[1000];

    get_snt_path(text_snt,textpath);
	// delete  the 3 file-dicos for output (simple, compound and unknown words) of the text in the text_snt  directory
	strcpy(nom_dlf,textpath);
	strcat(nom_dlf,"dlf");
	dico=u_fopen(nom_dlf,U_WRITE);
	if (dico ==NULL) {
	   fprintf(stderr,"Cannot create %s\n",nom_dlf);
	   return 1;
	}
	u_fclose(dico);

	strcpy(nom_dlc,textpath);
	strcat(nom_dlc,"dlc");
	dico=u_fopen(nom_dlc,U_WRITE);
	if (dico ==NULL) {
	   fprintf(stderr,"Cannot create %s\n",nom_dlc);
	   return 1;
	}
	u_fclose(dico);

	strcpy(nom_err,textpath);
	strcat(nom_err,"err");
	dico=u_fopen(nom_err,U_WRITE);
	if (dico ==NULL) {
	   fprintf(stderr,"Cannot create %s\n",nom_err);
	   return 1;
	}
    u_fclose(dico); 
	return 0;

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



/**
 * modified and commented by Alexis Neme
 */
int main(int argc, char **argv) {
setBufferMode();

char last_processing[4] = ""  ;			// store either .fst2 or .bin
										// on the first application : the output files(dlf,dlc) are created (U_WRITE) 
                                   
										// on the subsequent apply or locate  : we close them ouput file and we reopen them
										// on the append mode  ( U_APPEND)  
									 

char nom_bin[1000];                     // DICTIONNARIE(S) .bin compiled in the bin format
//unsigned char* bin;                     // bin is the dictionarie(s) that we submit to the processing
char nom_inf[1000];
//struct INF_codes* INF;                  // is the gramatical encoding associated with this dico



FILE *text;                             // text is the trimed input text  after the segmentation .snt extention

char text_cod[2000];                    // after the segmentation , .txt is represented by 3 files in the text_snt directory 
char tokens_txt[2000];                  // .snt(almost identical a .txt) text.cod & tokens.txt
struct text_tokens* tok;        

char nom_dlf[1000];
char nom_dlc[1000];
char nom_err[1000];

FILE *dlc,*dlf,*err;                    // Output files produced by the dico program (dlc= coumpounds words, dlf= simple forms, err= Unknown)

Alphabet* alph;

if (argc<4) {                           
   usage();
   return 0;
}


// prepare the text dico (dlf,dlc will be  blanked then appended
	Init_OutputDic_dlf_dlc_err(argv[1],nom_dlf, nom_dlc, nom_err);
	dlf=u_fopen(nom_dlf,U_WRITE);
	dlc=u_fopen(nom_dlc,U_WRITE);
	err=u_fopen(nom_err,U_WRITE);


alph=load_alphabet(argv[2]);
if (alph==NULL) {
   fprintf(stderr,"Cannot open alphabet file %s\n",argv[2]);
   return 1;
}

// load the text input as result from the  Tokenization phase (tokens.txt, text.cod)

get_snt_path(argv[1],tokens_txt);
strcat(tokens_txt,"tokens.txt");
tok=load_text_tokens(tokens_txt);
if (tok==NULL) {
   free_alphabet(alph);
   fprintf(stderr,"Cannot open token file %s\n",tokens_txt);
   return 1;
}

get_snt_path(argv[1],text_cod);
strcat(text_cod,"text.cod");
text=fopen(text_cod,"rb");
if (text==NULL) {
   free_alphabet(alph);
   free_text_tokens(tok);
   u_fclose(dlf);
   u_fclose(dlc);
   u_fclose(err);
   fprintf(stderr,"Cannot open coded text file %s\n",text_cod);
   return 1;
}


printf("Initializing...\n");
init_dico_application(tok,dlf,dlc,err,text,alph);
u_fclose(dlf); u_fclose(dlc); u_fclose(err);           // we open dlf,dlc, err before each processing
dlf=dlc=err=NULL;                                      // we close all after 
free_text_DICO();

for (int priority=1;priority<4;priority++) {
   for (int i=3;i<argc;i++) {
      // we do a loop to apply several dictionaries
      name_without_extension(argv[i],nom_inf);
      char C=nom_inf[strlen(nom_inf)-1];
	  if ((priority==1 && C=='-') ||  (priority==2 && C!='-' && C!='+') ||  (priority==3 && C=='+')) {
         file_name_extension(argv[i],nom_bin);

		// a Dico.bin
		 if (!strcmp(nom_bin,".bin"))    {    
	         printf("Applying dico  %s...\n",argv[i]);
	         // opening output files
			 dlf =   u_fopen(nom_dlf,U_APPEND);
             dlc =   u_fopen(nom_dlc,U_APPEND);  
             err =   u_fopen(nom_err,U_WRITE);  
			 assign_text_DICO(dlf,dlc,err);
			 // working...
            Launch_dico_application(argv[i],text,tok,alph,dlf,dlc,err,priority);
            // dumping and closing output files
            sauver_mots_inconnus();
        	u_fclose(dlc); u_fclose(dlf);
            u_fclose(err); 
            dlf=dlc=err=NULL;
            free_text_DICO();                   // DLF, DLC, ERR = NULL
         }  
         // a FST grammar 
         if (!strcmp(nom_bin,".fst2"))       { 
            printf("Applying %s...\n",argv[i]);
            /**
             * IMPORTANT!!!
             * dlf, dlc and err must not be open while LaunchLocateAsRoutine
             * is running, because this function tries to read in these files.
             *
             */
			LaunchLocateAsRoutine(argv,i);             // i is the rank of the fst that we need to locates patterns					
	         // opening output files
			 dlf =   u_fopen(nom_dlf,U_APPEND);
             dlc =   u_fopen(nom_dlc,U_APPEND);  
             err =   u_fopen(nom_err,U_WRITE);  
			 assign_text_DICO(dlf,dlc,err);
			MergeDicLocateResults(argv[1], tok, "concord.ind",  priority);
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
    dlf =  u_fopen(nom_dlf,U_APPEND);  
	dlc =  u_fopen(nom_dlc,U_APPEND);  
	err =  u_fopen(nom_err,U_WRITE);  
	assign_text_DICO(dlf,dlc,err);
} 

sauver_mots_inconnus();

//printf("computing dictionnary statistics ...\n");
//compute_stat_dic(argv[1],"stat_dic.n", SIMPLE_WORDS, COMPOUND_WORDS, ERRORS);

printf("Done.\n");

free_dico_application();
free_alphabet(alph);
free_text_tokens(tok);
u_fclose(dlf);
u_fclose(dlc);
u_fclose(err);
fclose(text);
free_text_DICO();
//getchar();
return 0;

}
//---------------------------------------------------------------------------

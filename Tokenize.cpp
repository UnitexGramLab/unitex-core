 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Alphabet.h"
#include "String_hash.h"
#include "File.h"
#include "Copyright.h"
#include "DELA.h"
#include "Table_hash.h"
#include "IOBuffer.h"
#include "Error.h"


#define NORMAL 0
#define CHAR_BY_CHAR 1
#define MAX_TOKENS 1000000
#define MAX_ENTER_CHAR 1000000
#define MAX_TAG_LENGTH 4000


int n_occur[MAX_TOKENS];
int enter_pos[MAX_ENTER_CHAR];
int n_enter_char=0;
int MODE;
int SENTENCES=0;
int TOKENS_TOTAL=0;
int WORDS_TOTAL=0;
int DIFFERENT_WORDS=0;
int DIGITS_TOTAL=0;
int DIFFERENT_DIGITS=0;


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Tokenize <text> <alphabet> [-char_by_char]\n");
u_printf("     <text> : any unicode text file\n");
u_printf("     <alphabet> : the alphabet file\n");
u_printf("     [-char_by_char] : with this option, the program do a char by char\n");
u_printf("     tokenization (except for the tags {S}, {STOP} or like {today,.ADV}). This\n");
u_printf("     mode may be used for languages like Thai.\n\n");
u_printf("Tokenizes the text. The token list is stored into a TOKENS.TXT file and\n");
u_printf("the coded text is stored into a TEXT.COD file.\n");
u_printf("The program also produces 4 files named tok_by_freq.txt, tok_by_alph.txt,\n");
u_printf("stats.n and enter.pos. They contain the token list sorted by frequence and by\n");
u_printf("alphabetical order and stats.n contains some statistics. The file enter.pos\n");
u_printf("contains the position in tokens of all the carridge return sequences. All\n");
u_printf("files are saved in the text_snt directory.\n");
}



void ecrire_nombre_lignes(char name[],int n)
{
  FILE *f;
  int i;
  i=2+9*2; // *2 because of unicode +2 because of FF FE at file start
  f=fopen((char*)name,"r+b");
  do
    {
      fseek(f,i,0);
      i=i-2;
      u_fputc((unichar)((n%10)+'0'),f);
      n=n/10;
    }
  while (n);
  fclose(f);
}




void init_n_occur();
void sort_and_save_by_frequence(FILE*,struct table_hash*);
void sort_and_save_by_alph_order(FILE*,struct table_hash*);
void compute_statistics(FILE*,struct table_hash*,Alphabet*);
void normal_tokenization(FILE*,FILE*,FILE*,Alphabet*,struct table_hash*);
void char_by_char_tokenization(FILE*,FILE*,FILE*,Alphabet*,struct table_hash*);
void sauver_enter_pos(FILE*);



int main(int argc, char **argv) {
setBufferMode();

if (argc<3 || argc>4) {
   usage();
   return 0;
}
FILE* text;
FILE* out;
FILE* tokens;
FILE* enter;
char tokens_txt[2000];
char text_cod[2000];
char enter_pos[2000];
Alphabet* alph;

get_snt_path((const char*)argv[1],text_cod);
strcat(text_cod,"text.cod");
get_snt_path(argv[1],tokens_txt);
strcat(tokens_txt,"tokens.txt");
get_snt_path(argv[1],enter_pos);
strcat(enter_pos,"enter.pos");
text=u_fopen(argv[1],U_READ);
if (text==NULL) {
   error("Cannot open text file %s\n",argv[1]);
   return 1;
}
MODE=NORMAL;
if (argc==4) {
   if (!strcmp(argv[3],"-char_by_char"))
      MODE=CHAR_BY_CHAR;
   else {
      error("Invalid parameter %s\n",argv[3]);
   }
}
alph=load_alphabet(argv[2]);
if (alph==NULL) {
   error("Cannot load alphabet file %s\n",argv[2]);
   u_fclose(text);
   return 1;
}
out=fopen(text_cod,"wb");
if (out==NULL) {
   error("Cannot create file %s\n",text_cod);
   u_fclose(text);
   if (alph!=NULL) {
      free_alphabet(alph);
   }
   return 1;
}
enter=fopen(enter_pos,"wb");
if (enter==NULL) {
   error("Cannot create file %s\n",enter_pos);
   u_fclose(text);
   fclose(out);
   if (alph!=NULL) {
      free_alphabet(alph);
   }
   return 1;
}
tokens=u_fopen(tokens_txt,U_WRITE);
if (tokens==NULL) {
   error("Cannot create file %s\n",tokens_txt);
   u_fclose(text);
   fclose(out);
   fclose(enter);
   if (alph!=NULL) {
      free_alphabet(alph);
   }
   return 1;
}
u_fprintf(tokens,"0000000000\n");

struct table_hash *h_table;
h_table = new_table_hash(HASH_SIZE, HASH_BLOCK_SIZE);

init_n_occur();
u_printf("Tokenizing text...\n");
if (MODE==NORMAL) {
   normal_tokenization(text,out,tokens,alph,h_table);
}
else {
   char_by_char_tokenization(text,out,tokens,alph,h_table);
}
u_printf("\nDone.\n");
sauver_enter_pos(enter);
fclose(enter);
u_fclose(text);
fclose(out);
u_fclose(tokens);

ecrire_nombre_lignes(tokens_txt,h_table->last_token_cod);
// we compute some statistics
get_snt_path(argv[1],tokens_txt);
strcat(tokens_txt,"stats.n");
tokens=u_fopen(tokens_txt,U_WRITE);
if (tokens==NULL) {
   error("Cannot write %s\n",tokens_txt);
}
else {
   compute_statistics(tokens,h_table,alph);
   u_fclose(tokens);
}
// we save the tokens by frequence
get_snt_path(argv[1],tokens_txt);
strcat(tokens_txt,"tok_by_freq.txt");
tokens=u_fopen(tokens_txt,U_WRITE);
if (tokens==NULL) {
   error("Cannot write %s\n",tokens_txt);
}
else {
   sort_and_save_by_frequence(tokens,h_table);
   u_fclose(tokens);
}
// we save the tokens by alphabetical order
get_snt_path(argv[1],tokens_txt);
strcat(tokens_txt,"tok_by_alph.txt");
tokens=u_fopen(tokens_txt,U_WRITE);
if (tokens==NULL) {
   error("Cannot write %s\n",tokens_txt);
}
else {
   sort_and_save_by_alph_order(tokens,h_table);
   u_fclose(tokens);
}

free_table_hash(h_table);
if (alph!=NULL) {
   free_alphabet(alph);
}
return 0;
}
//---------------------------------------------------------------------------



void init_n_occur() {
for (int i=0;i<MAX_TOKENS;i++) {
   n_occur[i]=0;
}
}



void normal_tokenization(FILE* f,FILE* coded_text,FILE* tokens,Alphabet* alph,struct table_hash* h_table) {
int c;
unichar s[MAX_TAG_LENGTH];
int n;
char ENTER;
int COUNT=0;
int current_megabyte=0;
c=u_fgetc(f);
while (c!=EOF) {
   COUNT++;
   if ((COUNT/(1024*512))!=current_megabyte) {
      current_megabyte++;
      int z=(COUNT/(1024*512));
      u_printf("%d megabyte%s read...       \r",z,(z>1)?"s":"");
   }
   if (c==' ' || c==0x0d || c==0x0a) {
      ENTER=0;
      if (c=='\n') ENTER=1;
      // if the char is a separator, we jump all the separators
      while ((c=u_fgetc(f))==' ' || c==0x0d || c==0x0a) {
        if (c=='\n') ENTER=1;
        COUNT++;
      }
      s[0]=' ';
      s[1]='\0';

      if ( (n = find_token_numb(s,h_table)) == -1 ) {
        n = add_token(s, h_table);
        h_table->tab[n]=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(s)));
        u_strcpy(h_table->tab[n],s); 
      }
	  
      if (n==1000000) {
         fatal_error("Array overflow\n");
      }
      n_occur[n]++;
      if (ENTER==1) {
         if (n_enter_char<MAX_ENTER_CHAR) {
            enter_pos[n_enter_char++]=TOKENS_TOTAL;
         }
      }
      TOKENS_TOTAL++;
      fwrite(&n,4,1,coded_text);
   }
   else if (c=='{') {
     s[0]='{';
     int z=1;
     while (z<(MAX_TAG_LENGTH-1) && (c=u_fgetc(f))!='}' && c!='{' && c!='\n') {
        s[z++]=(unichar)c;
        COUNT++;
     }
     if (z==(MAX_TAG_LENGTH-1) || c!='}') {
        // if the tag has no ending }
        s[z]='\0';
        fatal_error("Error: a tag without ending } has been found:\n%S\n",s);
     }
     if (c=='\n') {
        // if the tag contains a return
        fatal_error("Error: a tag containing a new-line sequence has been found\n");
     }
     s[z]='}';
     s[z+1]='\0';
     if (!u_strcmp(s,"{S}")) {
        // if we have found a sentence delimiter
        SENTENCES++;
     } else {
        if (u_strcmp(s,"{STOP}") && !check_tag_token(s)) {
           // if a tag is incorrect, we exit
           fatal_error("The text contains an invalid tag. Unitex cannot process it.");
        }
     }
     
	 if ((n = find_token_numb(s,h_table)) == -1 ) {
           n = add_token(s, h_table);	
           unichar* tmp = (unichar*) malloc(sizeof(unichar)*(1+u_strlen(s)));
           if (tmp == NULL)
           {
             fatal_error("Not enough memory, exiting!\n");
           }
           h_table->tab[n]=tmp;
           u_strcpy(h_table->tab[n],s); 
     }
     
     if (n==1000000) {
        fatal_error("Array overflow\n");
     }
     n_occur[n]++;
     TOKENS_TOTAL++;
     fwrite(&n,4,1,coded_text);
     c=u_fgetc(f);
   }
   else {
      s[0]=(unichar)c;
      n=1;
      if (!is_letter(s[0],alph)) {
         s[1]='\0';
         if ((n = find_token_numb(s,h_table)) == -1 ) {
			n = add_token(s , h_table);	
			h_table->tab[n]=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(s)));
	   		u_strcpy(h_table->tab[n],s); 			 
		 }
         
         if (n==1000000) {
            fatal_error("Array overflow\n");
         }
         n_occur[n]++;
         TOKENS_TOTAL++;
         if (c>='0' && c<='9') DIGITS_TOTAL++;
         fwrite(&n,4,1,coded_text);
         
         c=u_fgetc(f);
      }
      else {
         while ((n<(MAX_TAG_LENGTH-1)) && is_letter((unichar)(c=u_fgetc(f)),alph)) {
           s[n++]=(unichar)c;
           COUNT++;
         }
         if (n==(MAX_TAG_LENGTH-1)) {
            error("Token too long at position %d\n",COUNT);
         }
         s[n]='\0';

         if ((n = find_token_numb(s,h_table)) == -1 ) {
           n = add_token(s, h_table);
           unichar* tmp = (unichar*) malloc(sizeof(unichar)*(1+u_strlen(s)));
           if (tmp == NULL) {
             fatal_error("Not enough memory, exiting!\n");
           }
           h_table->tab[n]=tmp;
           u_strcpy(h_table->tab[n],s); 			 
         }

         if (n==1000000) {
            fatal_error("Array overflow\n");
         }
         n_occur[n]++;
         TOKENS_TOTAL++;
         WORDS_TOTAL++;
         fwrite(&n,4,1,coded_text);
      }
   }
}
for (n=0;n<h_table->last_token_cod;n++) {
   u_fprintf(tokens,"%S\n",h_table->tab[n]);
}
}



void char_by_char_tokenization(FILE* f,FILE* coded_text,FILE* tokens,Alphabet* alph,struct table_hash* h_table) {
int c;
unichar s[MAX_TAG_LENGTH];
int n;
char ENTER;
int COUNT=0;
int current_megabyte=0;
c=u_fgetc(f);
while (c!=EOF) {
   COUNT++;
   if ((COUNT/(1024*512))!=current_megabyte) {
      current_megabyte++;
      u_printf("%d megabytes read...         \r",(COUNT/(1024*512)));
   }
   if (c==' ' || c==0x0d || c==0x0a) {
      ENTER=0;
      if (c==0x0d) ENTER=1;
      // if the char is a separator, we jump all the separators
      while ((c=u_fgetc(f))==' ' || c==0x0d || c==0x0a) {
         if (c==0x0d) ENTER=1;
         COUNT++;
      }
      s[0]=' ';
      s[1]='\0';

      if ((n = find_token_numb(s,h_table)) == -1 ) {
         n = add_token(s , h_table);	
		 h_table->tab[n]=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(s)));
	   	 u_strcpy(h_table->tab[n],s); 			 
      }

      if (n==1000000) {
            fatal_error("Array overflow\n");
         }
      if (ENTER==1) {
         if (n_enter_char<MAX_ENTER_CHAR) {
            enter_pos[n_enter_char++]=TOKENS_TOTAL;
         }
      }
      n_occur[n]++;
      TOKENS_TOTAL++;
      fwrite(&n,4,1,coded_text);
   }
   else if (c=='{') {
     s[0]='{';
     int z=1;
     while (z<(MAX_TAG_LENGTH-1) && (c=u_fgetc(f))!='}' && c!='{' && c!='\n') {
        s[z++]=(unichar)c;
        COUNT++;
     }
     if (z==(MAX_TAG_LENGTH-1) || c!='}') {
        // if the tag has no ending }
        fatal_error("Error: a tag without ending } has been found\n");
     }
     if (c=='\n') {
        // if the tag contains a return
        fatal_error("Error: a tag containing a new-line sequence has been found\n");
     }
     s[z]='}';
     s[z+1]='\0';
     if (!u_strcmp(s,"{S}")) {
        // if we have found a sentence delimiter
        SENTENCES++;
     } else {
        if (u_strcmp(s,"{STOP}") && !check_tag_token(s)) {
           // if a tag is incorrect, we exit
           fatal_error("The text contains an invalid tag. Unitex cannot process it.");
        }
     }

      if ((n = find_token_numb(s,h_table)) == -1 ) {
         n = add_token(s , h_table);	
		 h_table->tab[n]=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(s)));
	   	 u_strcpy(h_table->tab[n],s); 			 
      }

     if (n==1000000) {
        fatal_error("Array overflow\n");
     }
     n_occur[n]++;
     TOKENS_TOTAL++;
     fwrite(&n,4,1,coded_text);
     c=u_fgetc(f);
   }
   else {
      s[0]=(unichar)c;
      s[1]='\0';

      if ((n = find_token_numb(s,h_table)) == -1 ) {
         n = add_token(s , h_table);	
		 h_table->tab[n]=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(s)));
	   	 u_strcpy(h_table->tab[n],s); 			 
      }

      if (n==1000000) {
           fatal_error("Array overflow\n");
        }
        n_occur[n]++;
      TOKENS_TOTAL++;
      if (is_letter((unichar)c,alph)) WORDS_TOTAL++;
      else if (c>='0' && c<='9') DIGITS_TOTAL++;
      fwrite(&n,4,1,coded_text);
      c=u_fgetc(f);
   }
}
for (n=0;n<h_table->last_token_cod;n++) {
   u_fprintf(tokens,"%S\n",h_table->tab[n],tokens);
}
}




int partition_pour_quicksort_by_frequence(int m, int n,struct table_hash* h_table) {
int pivot;
int tmp;
unichar* tmp_char;
int i = m-1;
int j = n+1;         // indice final du pivot
pivot=n_occur[(m+n)/2];
while (true) {
  do j--;
  while ((j>(m-1))&&(pivot>n_occur[j]));
  do i++;
  while ((i<n+1)&&(n_occur[i]>pivot));
  if (i<j) {
    tmp=n_occur[i];
    n_occur[i]=n_occur[j];
    n_occur[j]=tmp;

    tmp_char=h_table->tab[i];
    h_table->tab[i]=h_table->tab[j];
    h_table->tab[j]=tmp_char;
  } else return j;
}
}



void quicksort_by_frequence(int debut,int fin,struct table_hash* h_table) {
int p;
if (debut<fin) {
  p=partition_pour_quicksort_by_frequence(debut,fin,h_table);
  quicksort_by_frequence(debut,p,h_table);
  quicksort_by_frequence(p+1,fin,h_table);
}
}



int partition_pour_quicksort_by_alph_order(int m, int n,struct table_hash* h_table) {
unichar* pivot;
unichar* tmp;
int tmp_int;
int i = m-1;
int j = n+1;         // indice final du pivot
pivot=h_table->tab[(m+n)/2];
while (true) {
  do j--;
  while ((j>(m-1))&&(u_strcmp(pivot,h_table->tab[j])<0));
  do i++;
  while ((i<n+1)&&(u_strcmp(h_table->tab[i],pivot)<0));
  if (i<j) {
    tmp_int=n_occur[i];
    n_occur[i]=n_occur[j];
    n_occur[j]=tmp_int;

    tmp=h_table->tab[i];
    h_table->tab[i]=h_table->tab[j];
    h_table->tab[j]=tmp;
  } else return j;
}
}



void quicksort_by_alph_order(int debut,int fin,struct table_hash* h_table) {
int p;
if (debut<fin) {
  p=partition_pour_quicksort_by_alph_order(debut,fin,h_table);
  quicksort_by_alph_order(debut,p,h_table);
  quicksort_by_alph_order(p+1,fin,h_table);
}
}




void sort_and_save_by_frequence(FILE *f,struct table_hash* h_table) {
quicksort_by_frequence(0,h_table->last_token_cod - 1,h_table);
for (int i=0;i<h_table->last_token_cod;i++) {
   u_fprintf(f,"%d\t%S\n",n_occur[i],h_table->tab[i]);
}
}



void sort_and_save_by_alph_order(FILE *f,struct table_hash* h_table) {
quicksort_by_alph_order(0,h_table->last_token_cod-1,h_table);
for (int i=0;i<h_table->last_token_cod;i++) {
   u_fprintf(f,"%S\t%d\n",h_table->tab[i],n_occur[i]);
}
}



void compute_statistics(FILE *f,struct table_hash* h_table,Alphabet* alph) {
for (int i=0;i<h_table->last_token_cod;i++) {
   if (u_strlen(h_table->tab[i])==1) {
      if (h_table->tab[i][0]>='0' && h_table->tab[i][0]<='9') DIFFERENT_DIGITS++;
   }
   if (is_letter(h_table->tab[i][0],alph)) DIFFERENT_WORDS++;
}
u_fprintf(f,"%d sentence delimiter%s, %d (%d diff) token%s, %d (%d) simple form%s, %d (%d) digit%s\n",SENTENCES,(SENTENCES>1)?"s":"",TOKENS_TOTAL,h_table->last_token_cod,(TOKENS_TOTAL>1)?"s":"",WORDS_TOTAL,DIFFERENT_WORDS,(WORDS_TOTAL>1)?"s":"",DIGITS_TOTAL,DIFFERENT_DIGITS,(DIGITS_TOTAL>1)?"s":"");
}



void sauver_enter_pos(FILE* f) {
fwrite(&enter_pos,sizeof(int),n_enter_char,f);
}

/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Error.h"
#include "Vector.h"
#include "HashTable.h"
#include "UnitexGetOpt.h"
#include "Tokenize.h"
#include "Token.h"
#include "Offsets.h"
#include "Overlap.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define NORMAL 0
#define CHAR_BY_CHAR 1




static void sort_and_save_by_frequence(U_FILE*,vector_ptr*,vector_int*);
static void sort_and_save_by_alph_order(U_FILE*,vector_ptr*,vector_int*);
static void compute_statistics(U_FILE*,vector_ptr*,Alphabet*,int,int,int,int);
static int tokenization(U_FILE*,U_FILE*,U_FILE*,Alphabet*,vector_ptr*,struct hash_table*,vector_int*,
		vector_int*,vector_int*,
		   int*,int*,int*,int*,U_FILE*,vector_offset*,int);
static void save_new_line_positions(U_FILE*,vector_int*);
static void load_token_file(char* filename, const VersatileEncodingConfig*,vector_ptr* tokens,struct hash_table* hashtable,vector_int* n_occur);

void write_number_of_tokens(const VersatileEncodingConfig* vec,const char* name,int n) {
  U_FILE* f;
  char number[11];
  f=u_fopen(vec,name,U_MODIFY);
  number[10]=0;
  int offset=9;
  for (;;) {
      number[offset]=(char)((n%10)+'0');
      n/=10;
      if (offset==0)
          break;
      offset--;
  }
u_fprintf(f,number);
u_fclose(f);
}




const char* usage_Tokenize =
         "Usage: Tokenize [OPTIONS] <txt>\n"
         "\n"
         "  <txt>: any unicode text file\n"
         "\n"
         "OPTIONS:\n"
         "  -a ALPH/--alphabet=ALPH: the alphabet file\n"
         "  -c/--char_by_char: with this option, the program does a char by char tokenization\n"
         "                     (except for the tags {S}, {STOP} or like {today,.ADV}). This\n"
         "                     mode may be used for languages like Thai.\n"
         "  -w/--word_by_word: word by word tokenization (default);\n"
         "  -t TOKENS/--tokens=TOKENS: specifies a tokens.txt file to load and modify, instead of\n"
         "                             creating a new one from scratch;\n"
         "Offset options:\n"
         "  --input_offsets=XXX: base offset file to be used\n"
         "  --output_offsets=XXX: offset file to be produced (at \"uima\" format)\n"
         "\n"
         "  -h/--help: this help\n"
         "\n"
         "Tokenizes the text. The token list is stored into \"tokens.txt\" and\n"
         "the coded text is stored into \"text.cod\".\n"
         "The program also produces 4 files named \"tok_by_freq.txt\", \"tok_by_alph.txt\",\n"
         "\"stats.n\" and \"enter.pos\". They contain the token list sorted by frequence and by\n"
         "alphabetical order and \"stats.n\" contains some statistics. The file \"enter.pos\"\n"
         "contains the position in tokens of all the carriage return sequences. The file\n"
         "\"snt_offsets.pos\" contains the offset shifts between .snt and coded representation.\n"
		 "All files are saved in the XXX_snt directory where XXX is <txt> without its extension.\n";


static void usage() {
display_copyright_notice();
u_printf(usage_Tokenize);
}


const char* optstring_Tokenize=":a:cwt:hk:q:$:@:";
const struct option_TS lopts_Tokenize[]={
   {"alphabet", required_argument_TS, NULL, 'a'},
   {"char_by_char", no_argument_TS, NULL, 'c'},
   {"word_by_word", no_argument_TS, NULL, 'w'},
   {"tokens", required_argument_TS, NULL, 't'},
   {"input_encoding",required_argument_TS,NULL,'k'},
   {"output_encoding",required_argument_TS,NULL,'q'},
   {"input_offsets",required_argument_TS,NULL,'$'},
   {"output_offsets",required_argument_TS,NULL,'@'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};


int main_Tokenize(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

size_t step_filename_buffer = (((FILENAME_MAX / 0x10) + 1) * 0x10);
char* buffer_filename = (char*)malloc(step_filename_buffer * 8);
if (buffer_filename == NULL)
{
	fatal_alloc_error("main_Tokenize");
}
char* alphabet = (buffer_filename + (step_filename_buffer * 0));
char* token_file = (buffer_filename + (step_filename_buffer * 1));
char* in_offsets = (buffer_filename + (step_filename_buffer * 2));
char* out_offsets = (buffer_filename + (step_filename_buffer * 3));
*alphabet = '\0';
*token_file = '\0';
*in_offsets = '\0';
*out_offsets = '\0';

VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
int mode=NORMAL;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Tokenize,lopts_Tokenize,&index,vars))) {
   switch(val) {
   case 'a': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file name\n");
             }
             strcpy(alphabet,vars->optarg);
             break;
   case 'c': mode=CHAR_BY_CHAR; break;
   case 'w': mode=NORMAL; break;
   case 't': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty token file name\n");
             }
             strcpy(token_file,vars->optarg);
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
   case '$': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input_offsets argument\n");
             }
             strcpy(in_offsets,vars->optarg);
             break;
   case '@': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_offsets argument\n");
             }
             strcpy(out_offsets,vars->optarg);
             break;
   case 'h': usage(); free(buffer_filename); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Tokenize[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
U_FILE* text;
U_FILE* out;
U_FILE* output;
U_FILE* enter;
U_FILE* f_out_offsets=NULL;
vector_offset* v_in_offsets=NULL;

char* tokens_txt = (buffer_filename + (step_filename_buffer * 4));
char* text_cod = (buffer_filename + (step_filename_buffer * 5));
char* enter_pos = (buffer_filename + (step_filename_buffer * 6));
char* snt_offsets_pos = (buffer_filename + (step_filename_buffer * 7));
*tokens_txt = '\0';
*text_cod = '\0';
*enter_pos = '\0';
*snt_offsets_pos = '\0';

Alphabet* alph=NULL;

get_snt_path(argv[vars->optind],text_cod);
strcat(text_cod,"text.cod");
get_snt_path(argv[vars->optind],tokens_txt);
strcat(tokens_txt,"tokens.txt");
get_snt_path(argv[vars->optind],enter_pos);
strcat(enter_pos,"enter.pos");
get_snt_path(argv[vars->optind],snt_offsets_pos);
strcat(snt_offsets_pos,"snt_offsets.pos");
text=u_fopen(&vec,argv[vars->optind],U_READ);
if (text==NULL) {
   fatal_error("Cannot open text file %s\n",argv[vars->optind]);
}
if (alphabet[0]!='\0') {
   alph=load_alphabet(&vec,alphabet);
   if (alph==NULL) {
      error("Cannot load alphabet file %s\n",alphabet);
      u_fclose(text);
      free(buffer_filename);
      return 1;
   }
}
out=u_fopen(BINARY,text_cod,U_WRITE);
if (out==NULL) {
   error("Cannot create file %s\n",text_cod);
   u_fclose(text);
   if (alph!=NULL) {
      free_alphabet(alph);
   }
   free(buffer_filename); 
   return 1;
}
enter=u_fopen(BINARY,enter_pos,U_WRITE);
if (enter==NULL) {
   error("Cannot create file %s\n",enter_pos);
   u_fclose(text);
   u_fclose(out);
   if (alph!=NULL) {
      free_alphabet(alph);
   }
   free(buffer_filename); 
   return 1;
}
if (out_offsets[0]!='\0') {
	f_out_offsets=u_fopen(&vec,out_offsets,U_WRITE);
	if (f_out_offsets==NULL) {
		fatal_error("Cannot create file %s\n",out_offsets);
	}
	/* We deal with offsets only if the program is expected to produce some */
	if (in_offsets[0]!='\0') {
		v_in_offsets=load_offsets(&vec,in_offsets);
		if (v_in_offsets==NULL) {
			fatal_error("Cannot load offset file %s\n",in_offsets);
		}
	} else {
		/* If there is no input offset file, we create an empty offset vector
		 * in order to avoid testing whether the vector is NULL or not */
		v_in_offsets=new_vector_offset(1);
	}
}


vector_ptr* tokens=new_vector_ptr(4096);
vector_int* n_occur=new_vector_int(4096);
vector_int* n_enter_pos=new_vector_int(4096);
vector_int* snt_offsets=new_vector_int(4096);
struct hash_table* hashtable=new_hash_table((HASH_FUNCTION)hash_unichar,(EQUAL_FUNCTION)u_equal,
                                            (FREE_FUNCTION)free,NULL,(KEYCOPY_FUNCTION)keycopy);
if (token_file[0]!='\0') {
   load_token_file(token_file,&vec,tokens,hashtable,n_occur);
}

output=u_fopen(&vec,tokens_txt,U_WRITE);
if (output==NULL) {
   error("Cannot create file %s\n",tokens_txt);
   u_fclose(text);
   u_fclose(out);
   u_fclose(enter);
   if (alph!=NULL) {
      free_alphabet(alph);
   }
   free_hash_table(hashtable);
   free_vector_ptr(tokens,free);
   free_vector_int(n_occur);
   free_vector_int(n_enter_pos);
   free_vector_int(snt_offsets);
   free(buffer_filename); 
   return 1;
}
u_fprintf(output,"0000000000\n");

int SENTENCES=0;
int TOKENS_TOTAL=0;
int WORDS_TOTAL=0;
int DIGITS_TOTAL=0;
u_printf("Tokenizing text...\n");
int result_tokenization = tokenization(text,out,output,alph,tokens,hashtable,n_occur,n_enter_pos,
                          snt_offsets,
                          &SENTENCES,&TOKENS_TOTAL,&WORDS_TOTAL,&DIGITS_TOTAL,f_out_offsets,
                          v_in_offsets,(mode!=NORMAL));
u_printf((result_tokenization == 0) ? "\nDone.\n" : "\nGRAVE Tokenization error.\n");
save_new_line_positions(enter,n_enter_pos);
if (!save_snt_offsets(snt_offsets,snt_offsets_pos)) {
	fatal_error("Cannot save snt offsets in file %s\n",snt_offsets_pos);
}
free_vector_int(snt_offsets);
u_fclose(enter);
u_fclose(text);
u_fclose(out);
u_fclose(output);
write_number_of_tokens(&vec,tokens_txt,tokens->nbelems);
// we compute some statistics
get_snt_path(argv[vars->optind],tokens_txt);
strcat(tokens_txt,"stats.n");
output=u_fopen(&vec,tokens_txt,U_WRITE);
if (output==NULL) {
   error("Cannot write %s\n",tokens_txt);
}
else {
   compute_statistics(output,tokens,alph,SENTENCES,TOKENS_TOTAL,WORDS_TOTAL,DIGITS_TOTAL);
   u_fclose(output);
}
// we save the tokens by frequence
get_snt_path(argv[vars->optind],tokens_txt);
strcat(tokens_txt,"tok_by_freq.txt");
output=u_fopen(&vec,tokens_txt,U_WRITE);
if (output==NULL) {
   error("Cannot write %s\n",tokens_txt);
}
else {
   sort_and_save_by_frequence(output,tokens,n_occur);
   u_fclose(output);
}
// we save the tokens by alphabetical order
get_snt_path(argv[vars->optind],tokens_txt);
strcat(tokens_txt,"tok_by_alph.txt");
output=u_fopen(&vec,tokens_txt,U_WRITE);
if (output==NULL) {
   error("Cannot write %s\n",tokens_txt);
}
else {
   sort_and_save_by_alph_order(output,tokens,n_occur);
   u_fclose(output);
}
free_hash_table(hashtable);
free_vector_ptr(tokens,free);
free_vector_int(n_occur);
free_vector_int(n_enter_pos);
free_alphabet(alph);
u_fclose(f_out_offsets);
free_vector_offset(v_in_offsets);
free_OptVars(vars);
free(buffer_filename); 
return result_tokenization;
}
//---------------------------------------------------------------------------


/**
 * Returns the number of the given token, inserting it if needed in the
 * data structures. Its number of occurrences is also updated.
 */
static int get_token_number(unichar* s,vector_ptr* tokens,struct hash_table* hashtable,vector_int* n_occur) {
int ret;
struct any* value=get_value(hashtable,s,HT_INSERT_IF_NEEDED,&ret);
if (ret==HT_KEY_ADDED) {
   /* If the token was not already in the hash table, we must give it
    * a number */
   value->_int=vector_ptr_add(tokens,u_strdup(s));
   vector_int_add(n_occur,0);
}
int n=value->_int;
/* Then we update the number of occurrences */
n_occur->tab[n]++;
return n;
}


/**
 * Loads an existing token file.
 */
static void load_token_file(char* filename, const VersatileEncodingConfig* vec,vector_ptr* tokens,struct hash_table* hashtable,vector_int* n_occur) {
U_FILE* f=u_fopen(vec,filename,U_READ);
if (f==NULL) {
   fatal_error("Cannot open token file %s\n",filename);
}
Ustring* tmp=new_Ustring(1024);
if (EOF==readline(tmp,f)) {
   fatal_error("Unexpected empty token file %s\n",filename);
}
while (EOF!=readline(tmp,f)) {
   int n=get_token_number(tmp->str,tokens,hashtable,n_occur);
   /* We decrease the number of occurrences, in order to have all those numbers equal to 0 */
   n_occur->tab[n]--;
}
free_Ustring(tmp);
u_fclose(f);
}


static void save(U_FILE* f,const unichar* s,int n,int start,int end) {
(void)s;
u_fprintf(f,"%d %d %d <%S>\n",n,start,end,s);
//error("%d %d %d <%S>\n",n,start,end,s);
}


static int save_token_offset(U_FILE* f,const unichar* s,int n,int start,int end,const vector_offset* v, int *index,
		int *shift) {
if (f==NULL) return 0;
for (;;) {
if (*index==v->nbelems) {
	/* If there is no more offsets to take into account, we just save the token */
	save(f,s,n,start+*shift,end+*shift);
	return 0;
}
Offsets x=v->tab[*index];
Overlap o=overlap(x.new_start,x.new_end,start,end);
switch (o) {
case A_BEFORE_B: {
	error("A_BEFORE_B: ");
	error("shift=%d  token=<%S> start=%d end=%d    cur_offset[%d]=%d;%d => %d;%d\n",
			*shift,s,start,end,*index,x.old_start,x.old_end,x.new_start,x.new_end);
	error("Unexpected A_BEFORE_B in save_token_offset\n");
	return 1;
}
case A_AFTER_B: {
	//error("A_AFTER_B:\n");
	save(f,s,n,start+*shift,end+*shift);
	return 0;
}
case A_EQUALS_B: {
	//error("A_EQUALS_B:\n");
	save(f,s,n,x.old_start,x.old_end);
	(*index)++;
	(*shift)=x.old_end-end;
	return 0;
}
case A_BEFORE_B_OVERLAP: {
	//error("A_BEFORE_B_OVERLAP:\n");
	int j;
	for (j=(*index)+1;j<v->nbelems && B_INCLUDES_A==overlap(v->tab[j].new_start,v->tab[j].new_end,start,end);j++) {}
	j--;
	int delta_end=end-v->tab[j].new_end;
	int old_end=v->tab[j].old_end+delta_end;
	save(f,s,n,x.old_start,old_end);
	(*index)=j+1;
	(*shift)=v->tab[j].old_end-end+delta_end;
	return 0;
}
case A_AFTER_B_OVERLAP: {
	//error("A_AFTER_B_OVERLAP:\n");
	int delta=start-x.new_start;
	save(f,s,n,x.old_start+delta,x.old_end);
	return 0;
}
case A_INCLUDES_B: {
	//error("A_INCLUDES_B:\n");
	save(f,s,n,x.old_start,x.old_end);
	if (x.new_end==end) {
		(*index)++;
	}
	(*shift)=x.old_end-end;
	return 0;
}
case B_INCLUDES_A: {
	//error("B_INCLUDES_A:\n");
	int delta_start=start-x.new_start;
	int old_start=x.old_start+delta_start;
	int j;
	Overlap tmp;
	for (j=(*index)+1;j<v->nbelems &&
		(B_INCLUDES_A==(tmp=overlap(v->tab[j].new_start,v->tab[j].new_end,start,end))
			|| A_EQUALS_B==tmp
			|| A_AFTER_B_OVERLAP==tmp);j++) {}
	j--;
	int delta_end=end-v->tab[j].new_end;
	int old_end=v->tab[j].old_end+delta_end;
	save(f,s,n,old_start,old_end);
	(*index)=j+1;
	(*shift)=v->tab[j].old_end-end+delta_end;
	return 0;
}
}
}
}


#define TOKENIZE_WRITE_BUFFER_SIZE 0x80
static void fast_fwrite_raw_flush(U_FILE* f, unsigned char*out_buffer, unsigned int* pos_out_buffer)
{
	if ((*pos_out_buffer) > 0)
		fwrite(out_buffer, 4, *pos_out_buffer, f);
	*pos_out_buffer = 0;
}

static inline void fast_fwrite_raw(U_FILE* f, int n, unsigned char*out_buffer, unsigned int* pos_out_buffer, unsigned int size_out_buffer)
{
	if ((*pos_out_buffer) == size_out_buffer) {
		fast_fwrite_raw_flush(f, out_buffer, pos_out_buffer);
	}
	*((int*)(out_buffer + ((*pos_out_buffer) * 4))) = n;
	(*pos_out_buffer)++;
}

#define TOKENIZE_GET_BUFFER_SIZE 0x200

static inline int fast_u_fgetc_raw(U_FILE* f, unichar*buffer,unsigned int* pos_in_buffer, unsigned int* filled_in_buffer)
{
	for (;;)
	{
		if ((*pos_in_buffer) < (*filled_in_buffer))
		{
			int c = (int)*(buffer + (*pos_in_buffer));
			(*pos_in_buffer)++;
			return c;
		}

		*filled_in_buffer = 0;
		*pos_in_buffer = 0;
		int res = u_fget_unichars_raw(buffer, TOKENIZE_GET_BUFFER_SIZE, f);
		if (res == 0)
			return EOF;
		if (res < 0)
			return res;
		*filled_in_buffer = res;
	}
}

static void enlarge_token_buffer_as_needed(unichar** token_buffer, size_t *buffer_size, size_t size_needed)
{
	if (size_needed <= (*buffer_size))
		return;

	size_t buffer_new_size = *buffer_size;
	while (size_needed > buffer_new_size)
		buffer_new_size *= 2;

	unichar* new_token_buffer = (unichar*)realloc((void*)*token_buffer, buffer_new_size*sizeof(unichar));
	if (new_token_buffer == NULL) {
		fatal_alloc_error("enlarge_token_buffer_as_needed");
	}
	*token_buffer = new_token_buffer;
	*buffer_size = buffer_new_size;
}

static inline void enlarge_token_buffer_if_needed(unichar** token_buffer, size_t *buffer_size, size_t size_needed)
{
	if (size_needed <= (*buffer_size))
		return;
	enlarge_token_buffer_as_needed(token_buffer, buffer_size, size_needed);
}

#define TOKENIZE_ORIGINAL_TOKEN_BUFFER_SIZE 0x400

static int tokenization(U_FILE* f_read,U_FILE* coded_text,U_FILE* output,Alphabet* alph,
                         vector_ptr* tokens,struct hash_table* hashtable,
                         vector_int* n_occur,vector_int* n_enter_pos,
                         /* snt_offsets is used to note shifts induced by separator normalization */
                         vector_int* snt_offsets,
                         int *SENTENCES,int *TOKENS_TOTAL,int *WORDS_TOTAL,
                         int *DIGITS_TOTAL,U_FILE* f_out_offsets,vector_offset* v_in_offsets,
                         int char_by_char) {
int c;
int n;
char ENTER;
int COUNT=0;
int current_megabyte=0;
int shift=0;
unichar read_buffer[TOKENIZE_GET_BUFFER_SIZE];
unsigned char write_buffer[(TOKENIZE_WRITE_BUFFER_SIZE * 4) + 0x10];
unsigned int pos_in_buffer = 0;
unsigned int filled_in_buffer = 0;
unsigned int pos_out_buffer = 0;
c = fast_u_fgetc_raw(f_read, read_buffer, &pos_in_buffer, &filled_in_buffer);
int current_pos;
int snt_offsets_shift=0;
int offset_index=0;
int result=0; // 0 = no error
size_t token_buffer_size = TOKENIZE_ORIGINAL_TOKEN_BUFFER_SIZE;
unichar * token_buffer = (unichar*)malloc(sizeof(unichar)*token_buffer_size);
if (token_buffer == NULL) {
  fatal_alloc_error("tokenization");
}
while ((c!=EOF) && (result == 0)) {
   current_pos=COUNT;
   COUNT++;
   if ((COUNT/(1024*512))!=current_megabyte) {
      current_megabyte++;
      int z=(COUNT/(1024*512));
      u_printf("%d megabyte%s read...       \r",z,(z>1)?"s":"");
   }
   if (c==' ' || c==0x0d || c==0x0a || c=='\t') {
      ENTER=0;
      if (c==0x0d || c==0x0a) ENTER=1;
      // if the char is a separator, we jump all the separators
	  while ((c = fast_u_fgetc_raw(f_read, read_buffer, &pos_in_buffer, &filled_in_buffer)) == ' ' || c == 0x0d || c == 0x0a || c == '\t') {
        if (c==0x0d || c==0x0a) ENTER=1;
        COUNT++;
      }
      token_buffer[0]=' ';
      token_buffer[1]='\0';
      n=get_token_number(token_buffer,tokens,hashtable,n_occur);
      if (COUNT-current_pos!=1) {
    	  /* If there is a shift with the .snt file */
    	  add_snt_offsets(snt_offsets,*TOKENS_TOTAL,snt_offsets_shift,snt_offsets_shift+(COUNT-current_pos-1));
    	  snt_offsets_shift+=(COUNT-current_pos-1);
      }
      result=save_token_offset(f_out_offsets,token_buffer,n,current_pos,COUNT,v_in_offsets,&offset_index,&shift);
      /* If there is a \n, we note it */
      if (ENTER==1) {
         vector_int_add(n_enter_pos,*TOKENS_TOTAL);
      }
      (*TOKENS_TOTAL)++;
	  fast_fwrite_raw(coded_text, n, write_buffer, &pos_out_buffer, TOKENIZE_WRITE_BUFFER_SIZE);
   }
   else if (c=='{') {
     token_buffer[0]='{';
     int z=1;
     bool protected_char = false; // Cassys add
	 while ((((c = fast_u_fgetc_raw(f_read, read_buffer, &pos_in_buffer, &filled_in_buffer)) != '}' && c
					!= '{' && c != '\n') || protected_char)) {
			protected_char = false; // Cassys add
			if (c == '\\') { // Cassys add
				protected_char = true; // Cassys add
			} // Cassys add
			enlarge_token_buffer_if_needed(&token_buffer, &token_buffer_size, z+2);
			token_buffer[z++] = (unichar) c;
			COUNT++;
	}

     if (c!='}') {
        // if the tag has no ending }
        enlarge_token_buffer_if_needed(&token_buffer, &token_buffer_size, z + 1);
        token_buffer[z]='\0';
        fatal_error("Error: a tag without ending } has been found:\n%S\n",token_buffer);
     }
     if (c=='\n') {
        // if the tag contains a return
        fatal_error("Error: a tag containing a new-line sequence has been found\n");
     }
     enlarge_token_buffer_if_needed(&token_buffer, &token_buffer_size, z + 2);
     token_buffer[z]='}';
     token_buffer[z+1]='\0';
     if (!u_strcmp(token_buffer,"{S}")) {
        // if we have found a sentence delimiter
        (*SENTENCES)++;
     } else {
        if (u_strcmp(token_buffer,"{STOP}") && !check_tag_token(token_buffer,1)) {
           // if a tag is incorrect, we exit
           fatal_error("The text contains an invalid tag. Unitex cannot process it.");
        }
     }
     n=get_token_number(token_buffer,tokens,hashtable,n_occur);
     COUNT++;
     result=save_token_offset(f_out_offsets,token_buffer,n,current_pos,COUNT,v_in_offsets,&offset_index,&shift);
     (*TOKENS_TOTAL)++;
	 fast_fwrite_raw(coded_text, n, write_buffer, &pos_out_buffer, TOKENIZE_WRITE_BUFFER_SIZE);
	 c = fast_u_fgetc_raw(f_read, read_buffer, &pos_in_buffer, &filled_in_buffer);
   }
   else {
      token_buffer[0]=(unichar)c;
      n=1;
      if (!is_letter(token_buffer[0],alph) || char_by_char) {
         token_buffer[1]='\0';
         if (is_letter(token_buffer[0],alph)) (*WORDS_TOTAL)++;
         n=get_token_number(token_buffer,tokens,hashtable,n_occur);
         result=save_token_offset(f_out_offsets,token_buffer,n,current_pos,COUNT,v_in_offsets,&offset_index,
        		 &shift);
         (*TOKENS_TOTAL)++;
         if (c>='0' && c<='9') (*DIGITS_TOTAL)++;
		 fast_fwrite_raw(coded_text, n, write_buffer, &pos_out_buffer, TOKENIZE_WRITE_BUFFER_SIZE);
		 c = fast_u_fgetc_raw(f_read, read_buffer, &pos_in_buffer, &filled_in_buffer);
      }
      else {
		  while (EOF != (c = fast_u_fgetc_raw(f_read, read_buffer, &pos_in_buffer, &filled_in_buffer)) && is_letter((unichar)c, alph)) {
           enlarge_token_buffer_if_needed(&token_buffer, &token_buffer_size, n + 2);
           token_buffer[n++]=(unichar)c;
           COUNT++;
         }
         enlarge_token_buffer_if_needed(&token_buffer, &token_buffer_size, n + 1);
         token_buffer[n]='\0';
         n=get_token_number(token_buffer,tokens,hashtable,n_occur);
         result=save_token_offset(f_out_offsets,token_buffer,n,current_pos,COUNT,v_in_offsets,&offset_index,
        		 &shift);
         (*TOKENS_TOTAL)++;
         (*WORDS_TOTAL)++;
		 fast_fwrite_raw(coded_text, n, write_buffer, &pos_out_buffer, TOKENIZE_WRITE_BUFFER_SIZE);
      }
   }
}
fast_fwrite_raw_flush(coded_text, write_buffer, &pos_out_buffer);
for (n=0;n<tokens->nbelems;n++) {
   u_fprintf(output,"%S\n",tokens->tab[n]);
}
if (result!=0) {
   u_printf("Unsucessfull.\n");
}
free(token_buffer);
return result;
}



static int partition_pour_quicksort_by_frequence(int m, int n,vector_ptr* tokens,vector_int* n_occur) {
int pivot;
int tmp;
unichar* tmp_char;
int i = m-1;
int j = n+1; // final pivot index
pivot=n_occur->tab[(m+n)/2];
for (;;) {
  do j--;
  while ((j>(m-1))&&(pivot>n_occur->tab[j]));
  do i++;
  while ((i<n+1)&&(n_occur->tab[i]>pivot));
  if (i<j) {
    tmp=n_occur->tab[i];
    n_occur->tab[i]=n_occur->tab[j];
    n_occur->tab[j]=tmp;

    tmp_char=(unichar*)tokens->tab[i];
    tokens->tab[i]=tokens->tab[j];
    tokens->tab[j]=tmp_char;
  } else return j;
}
}



static void quicksort_by_frequence(int first,int last,vector_ptr* tokens,vector_int* n_occur) {
int p;
if (first<last) {
  p=partition_pour_quicksort_by_frequence(first,last,tokens,n_occur);
  quicksort_by_frequence(first,p,tokens,n_occur);
  quicksort_by_frequence(p+1,last,tokens,n_occur);
}
}



static int partition_pour_quicksort_by_alph_order(int m, int n,vector_ptr* tokens,vector_int* n_occur) {
unichar* pivot;
unichar* tmp;
int tmp_int;
int i = m-1;
int j = n+1; // final pivot index
pivot=(unichar*)tokens->tab[(m+n)/2];
for (;;) {
  do j--;
  while ((j>(m-1))&&(u_strcmp(pivot,(unichar*)tokens->tab[j])<0));
  do i++;
  while ((i<n+1)&&(u_strcmp((unichar*)tokens->tab[i],pivot)<0));
  if (i<j) {
    tmp_int=n_occur->tab[i];
    n_occur->tab[i]=n_occur->tab[j];
    n_occur->tab[j]=tmp_int;

    tmp=(unichar*)tokens->tab[i];
    tokens->tab[i]=tokens->tab[j];
    tokens->tab[j]=tmp;
  } else return j;
}
}



static void quicksort_by_alph_order(int first,int last,vector_ptr* tokens,vector_int* n_occur) {
int p;
if (first<last) {
  p=partition_pour_quicksort_by_alph_order(first,last,tokens,n_occur);
  quicksort_by_alph_order(first,p,tokens,n_occur);
  quicksort_by_alph_order(p+1,last,tokens,n_occur);
}
}




static void sort_and_save_by_frequence(U_FILE *f,vector_ptr* tokens,vector_int* n_occur) {
quicksort_by_frequence(0,tokens->nbelems - 1,tokens,n_occur);
for (int i=0;i<tokens->nbelems;i++) {
   u_fprintf(f,"%d\t%S\n",n_occur->tab[i],tokens->tab[i]);
}
}



static void sort_and_save_by_alph_order(U_FILE *f,vector_ptr* tokens,vector_int* n_occur) {
quicksort_by_alph_order(0,tokens->nbelems - 1,tokens,n_occur);
for (int i=0;i<tokens->nbelems;i++) {
   u_fprintf(f,"%S\t%d\n",tokens->tab[i],n_occur->tab[i]);
}
}



static void compute_statistics(U_FILE *f,vector_ptr* tokens,Alphabet* alph,
		                int SENTENCES,int TOKENS_TOTAL,int WORDS_TOTAL,int DIGITS_TOTAL) {
int DIFFERENT_DIGITS=0;
int DIFFERENT_WORDS=0;
for (int i=0;i<tokens->nbelems;i++) {
   unichar* foo=(unichar*)tokens->tab[i];
   if (u_strlen(foo)==1) {
      if (foo[0]>='0' && foo[0]<='9') DIFFERENT_DIGITS++;
   }
   if (is_letter(foo[0],alph)) DIFFERENT_WORDS++;
}
u_fprintf(f,"%d sentence delimiter%s, %d (%d diff) token%s, %d (%d) simple form%s, %d (%d) digit%s\n",
		SENTENCES,(SENTENCES>1)?"s":"",TOKENS_TOTAL,tokens->nbelems,(TOKENS_TOTAL>1)?"s":"",WORDS_TOTAL,
		DIFFERENT_WORDS,(WORDS_TOTAL>1)?"s":"",DIGITS_TOTAL,DIFFERENT_DIGITS,(DIGITS_TOTAL>1)?"s":"");
}



static void save_new_line_positions(U_FILE* f,vector_int* n_enter_pos) {
fwrite(n_enter_pos->tab,sizeof(int),n_enter_pos->nbelems,f);
}

} // namespace unitex

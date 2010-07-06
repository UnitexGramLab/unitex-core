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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS) in the framework 
 * of UNITEX optimization and UNITEX industrialization / reliability
 *
 * More information : http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
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
#include "getopt.h"
#include "Token.h"
#include "Text_tokens.h"
#include "Untokenize.h"

#define NORMAL 0
#define CHAR_BY_CHAR 1




const char* usage_Untokenize =
         "Usage: Untokenize [OPTIONS] <txt>\n"
         "\n"
         "  <txt>: an unicode text file to create\n"
         "\n"
         "OPTIONS:\n"
         "  -d X/--sntdir=X: uses directory X instead of the text directory; note that X must be\n"
         "                   (back)slash terminated\n"
         "  -n N/--number_token=N: adds tokens number each N token\n"
         "  -r N/--range=N: emits only token from number N to end\n"
         "  -r N,M/--range=N,M: emits only token from number N to M\n"
         "  -h/--help: this help\n"
         "\n"
         "Untokenizes and rebuild the orgininal text. The token list is stored into \"tokens.txt\" and\n"
         "the coded text is stored into \"text.cod\".\n"
         "The file \"enter.pos contains the position in tokens of all the carriage return sequences.\n"
         "These files are located in the XXX_snt directory where XXX is <txt> without its extension.\n";

static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Untokenize);
}


const char* optstring_Untokenize=":a:hn:r:d:k:q:";
const struct option_TS lopts_Untokenize[]={
   {"alphabet", required_argument_TS, NULL, 'a'},
   {"range",required_argument_TS,NULL,'r'},
   {"number_token",required_argument_TS,NULL,'n'},
   {"sntdir",required_argument_TS,NULL,'d'},
   {"input_encoding",required_argument_TS,NULL,'k'},
   {"output_encoding",required_argument_TS,NULL,'q'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};


int main_Untokenize(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

char alphabet[FILENAME_MAX]="";
char token_file[FILENAME_MAX]="";

char dynamicSntDir[FILENAME_MAX]="";

Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
int val,index=-1;
int range_start,range_stop,use_range;
int token_step_number=0;
range_start=range_stop=use_range=0;
char foo=0;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Untokenize,lopts_Untokenize,&index,vars))) {
   switch(val) {
   case 'a': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file name\n");
             }
             strcpy(alphabet,vars->optarg);
             break;
   case 'd': if (vars->optarg[0]=='\0') {
                   fatal_error("You must specify a non empty snt dir name\n");
                }
                strcpy(dynamicSntDir,vars->optarg);
                break;
   case 't': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty token file name\n");
             }
             strcpy(token_file,vars->optarg);
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

   case 'n': if (1!=sscanf(vars->optarg,"%d%c",&token_step_number,&foo) || token_step_number<=0) {
                /* foo is used to check that the search limit is not like "45gjh" */
                fatal_error("Invalid token numbering argument: %s\n",vars->optarg);
             }
             break;
   case 'r': {
                int param1 = 0;
                int param2 = 0;
                int ret_scan = sscanf(vars->optarg,"%d,%d%c",&param1,&param2,&foo);
                if (ret_scan == 2) {
                    range_start = param1;
                    range_stop  = param2;
                    use_range=1;
                    if (((range_start < -1)) || (range_stop < -1)) {
                        /* foo is used to check that the search limit is not like "45gjh" */
                        fatal_error("Invalid stop count argument: %s\n",vars->optarg);
                    }
                }
                else
                    if (1!=sscanf(vars->optarg,"%d%c",&range_start,&foo) || (range_start < -1)) {
                        /* foo is used to check that the search limit is not like "45gjh" */
                        fatal_error("Invalid stop count argument: %s\n",vars->optarg);
                    }
                    use_range=1;
             }
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Untokenize[index].name);
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
char tokens_txt[FILENAME_MAX];
char text_cod[FILENAME_MAX];
char enter_pos[FILENAME_MAX];

Alphabet* alph=NULL;

if (dynamicSntDir[0]=='\0') {
    get_snt_path(argv[vars->optind],dynamicSntDir);
}
strcpy(text_cod,dynamicSntDir);
strcat(text_cod,"text.cod");
strcpy(enter_pos,dynamicSntDir);
strcat(enter_pos,"enter.pos");
strcpy(tokens_txt,dynamicSntDir);
strcat(tokens_txt,"tokens.txt");



if (alphabet[0]!='\0') {
   alph=load_alphabet(alphabet);
   if (alph==NULL) {
      error("Cannot load alphabet file %s\n",alphabet);
      return 1;
   }
}

ABSTRACTMAPFILE* af_text_cod=af_open_mapfile(text_cod,MAPFILE_OPTION_READ,0);
if (af_text_cod==NULL) {
	error("Cannot open file %s\n",text_cod);
	if (alph!=NULL) {
      free_alphabet(alph);
	}
	return 1;
}

ABSTRACTMAPFILE* af_enter_pos=af_open_mapfile(enter_pos,MAPFILE_OPTION_READ,0);

text = u_fopen_creating_versatile_encoding(encoding_output,bom_output,argv[vars->optind],U_WRITE);


if (text==NULL) {
   error("Cannot create text file %s\n",argv[vars->optind]);
   if (alph!=NULL) {
      free_alphabet(alph);
   }
   af_close_mapfile(af_text_cod);
   return 1;
}


struct text_tokens* tok=load_text_tokens(tokens_txt,mask_encoding_compatibility_input);

u_printf("Untokenizing text...\n");

size_t nb_item = af_get_mapfile_size(af_text_cod)/sizeof(int);
const int* buf=(const int*)af_get_mapfile_pointer(af_text_cod);

size_t nb_item_enter_pos=0;
const int* buf_enter=NULL;

if (af_enter_pos!=NULL) {
    buf_enter=(const int*)af_get_mapfile_pointer(af_enter_pos);
    if (buf_enter!=NULL) {
        nb_item_enter_pos=af_get_mapfile_size(af_enter_pos)/sizeof(int);
    }
}

size_t count_pos=0;
for (size_t i=0;i<nb_item;i++) {
    int is_in_range=1;

    if ((use_range!=0) && (i<(size_t)range_start))
        is_in_range=0;

    if ((use_range!=0) && (range_stop!=0) && (i>(size_t)range_stop))
        is_in_range=0;

    int is_newline=0;
    if (count_pos<nb_item_enter_pos) {
        if (i==(size_t)(*(buf_enter+count_pos))) {
            is_newline = 1;
            count_pos++;
        }
    }

    if (is_in_range!=0) {
        if (token_step_number != 0)
            if ((i%token_step_number)==0)
                u_fprintf(text,"\n\nToken %d : ", (int)i);

        if (is_newline!=0) {
            u_fprintf(text,"\n", tok->token[*(buf+i)]);
        }
        else {
            u_fprintf(text,"%S", tok->token[*(buf+i)]);
        }
    }
}



af_release_mapfile_pointer(af_text_cod,buf);
if (buf_enter!=NULL) {
    af_release_mapfile_pointer(af_enter_pos,buf_enter);
}
if (af_enter_pos!=NULL) {
    af_close_mapfile(af_enter_pos);
}
af_close_mapfile(af_text_cod);
   if (alph!=NULL) {
      free_alphabet(alph);
   }




u_fclose(text);


free_text_tokens(tok);

if (alph!=NULL) {
   free_alphabet(alph);
}
free_OptVars(vars);
u_printf("\nDone.\n");
return 0;
}
//---------------------------------------------------------------------------

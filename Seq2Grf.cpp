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
#include "Unicode.h"
#include "Alphabet.h"
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "Grf_lib.h"
#include "GrfBeauty.h"
#include "Seq2Grf.h"
#include "Ustring.h"

static Grf* build_grf_from_sequences(U_FILE* text,Alphabet* alph);

const char* usage_Seq2Grf =
         "Usage: Seq2Grf [OPTIONS] <txt>\n"
         "\n"
         "  <txt>: a sequence text file, one sequence per line\n"
         "\n"
         "OPTIONS:\n"
         "  -a ALPH/--alphabet=ALPH: the alphabet file\n"
         "  -o GRF/--output=GRF: the .grf file to be produced\n"
         "  -h/--help: this help\n"
         "\n"
         "Builds a .grf that recognizes sequences contained in the input file.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Seq2Grf);
}


const char* optstring_Seq2Grf=":a:o:hk:q:";
const struct option_TS lopts_Seq2Grf[]={
   {"alphabet", required_argument_TS, NULL, 'a'},
   {"output", required_argument_TS, NULL, 'o'},
   {"input_encoding",required_argument_TS,NULL,'k'},
   {"output_encoding",required_argument_TS,NULL,'q'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};


int main_Seq2Grf(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

char alphabet[FILENAME_MAX]="";
char output[FILENAME_MAX]="";

Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Seq2Grf,lopts_Seq2Grf,&index,vars))) {
   switch(val) {
   case 'a': strcpy(alphabet,vars->optarg);
             break;
   case 'o': strcpy(output,vars->optarg);
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
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Seq2Grf[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
if (output[0]=='\0') {
	fatal_error("Invalid arguments: missing output file name\n");
}

U_FILE* text;
U_FILE* out;
Alphabet* alph=NULL;
text=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,argv[vars->optind],U_READ);
if (text==NULL) {
   fatal_error("Cannot open text file %s\n",argv[vars->optind]);
}
if (alphabet[0]!='\0') {
   alph=load_alphabet(alphabet);
   if (alph==NULL) {
      error("Cannot load alphabet file %s\n",alphabet);
      u_fclose(text);
      return 1;
   }
}
out=u_fopen_creating_versatile_encoding(encoding_output,bom_output,output,U_WRITE);
if (out==NULL) {
   error("Cannot create file %s\n",output);
   u_fclose(text);
   free_alphabet(alph);
   return 1;
}
Grf* grf=build_grf_from_sequences(text,alph);
save_Grf(out,grf);
free_Grf(grf);
u_fclose(text);
u_fclose(out);
free_alphabet(alph);
free_OptVars(vars);
return 0;
}


/**
 * Tokenizes the given sequence, ignoring spaces.
 */
static vector_ptr* tokenize_sequence(unichar* seq,Alphabet* alph) {
if (seq==NULL) return NULL;
vector_ptr* tokens=new_vector_ptr();
Ustring* tmp=new_Ustring(256);
int i=0;
while (seq[i]!='\0') {
	empty(tmp);
	if (seq[i]==' ') {
		i++;
	} else if (!is_letter(seq[i],alph)) {
		/* Not a letter ? We may have to protect special .grf chars */
		switch(seq[i]) {
			case '<': u_strcpy(tmp,"\\<"); break;
			case '"': u_strcpy(tmp,"\\\\\\\""); break;
			case '\\': u_strcpy(tmp,"\\\\"); break;
			case '+': u_strcpy(tmp,"\\+"); break;
			case ':': u_strcpy(tmp,"\\:"); break;
			case '/': u_strcpy(tmp,"\\/"); break;
			default: u_strcat(tmp,seq[i]); break;
		}
		vector_ptr_add(tokens,u_strdup(tmp->str));
		i++;
	} else {
		while (is_letter(seq[i],alph)) {
			u_strcat(tmp,seq[i]);
			i++;
		}
		vector_ptr_add(tokens,u_strdup(tmp->str));
	}
}
free_Ustring(tmp);
return tokens;
}


/**
 * Creates a grf from a sequence text file.
 */
static Grf* build_grf_from_sequences(U_FILE* text,Alphabet* alph) {
Grf* grf=NULL;
/* ici, conversion du tfst vers le grf */
Ustring* line=new_Ustring(1024);
while (EOF!=readline(line,text)) {
	vector_ptr* tokens=tokenize_sequence(line->str,alph);
	error("line=%S\n",line->str);
	for (int i=0;i<tokens->nbelems;i++) {
		error("  [%S]\n",tokens->tab[i]);
	}

	/* ajoute ici ton code de construction des états dans le tfst */
	free_vector_ptr(tokens,free);
}
free_Ustring(line);
beautify(grf,alph);
return grf;
}

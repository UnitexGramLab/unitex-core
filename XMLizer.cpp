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

#include <stdlib.h>
#include <string.h>
#include "Copyright.h"
#include "Error.h"
#include "File.h"
#include "Fst2Txt_TokenTree.h"
#include "Fst2TxtAsRoutine.h"
#include "LocateConstants.h"
#include "NormalizeAsRoutine.h"
#include "ParsingInfo.h"
#include "Transitions.h"
#include "Unicode.h"
#include "getopt.h"

#define XML 0
#define TEI 1

void xmlize(char*,char*,int);


/* Headers (XML & TEI) Variables
 * ----------------------------- */

static const char *xml_open = "<?xml version='1.0' encoding='UTF-8'?>\n<text>";
static const char *xml_close = "</text>";
static const char *tei_open = "<?xml version='1.0' encoding='UTF-8'?>\n<tei>\n<teiHeader/>\n<text>\n<body>\n<div xml:id=\"d1\">\n";
static const char *tei_close = "</div>\n</body>\n</text>\n</tei>";



static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: XMLizer [OPTIONS] <txt>\n"
         "\n"
         "  <txt>: the input text file\n"
         "\n"
	      "OPTIONS:\n"
         "  -x/--xml: build a simple XML file from a raw text file\n"
	      "  -t/--tei: build a minimal TEI file from a raw text file (default)\n"
	      "  -n XXX/--normalization=XXX: optional configuration file for the normalization process\n"
	      "  -o OUT/--output=OUT: optional output file name (default: file.txt > file.xml)\n"
	      "  -a ALPH/--alphabet=ALPH: alphabet file\n"
         "  -s SEG/--segmentation_grammar=SEG: .fst2 segmentation grammar\n"
         "  -h/--help: this help\n"
         "\n"
	      "Produces a TEI or simple XML file from the given raw text file.\n");
}

int main_XMLizer(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring = ":xtn:o:a:s:h";
const struct option_TS lopts[] = {
   {"xml", no_argument_TS, NULL, 'x'},
   {"tei", no_argument_TS, NULL, 't'},
   {"normalization", required_argument_TS, NULL, 'n'},
   {"output", required_argument_TS, NULL, 'o'},
   {"alphabet", required_argument_TS, NULL, 'a'},
   {"segmentation_grammar", required_argument_TS, NULL, 's'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};

int output_style=TEI;
char output[FILENAME_MAX]="";
char alphabet[FILENAME_MAX]="";
char normalization[FILENAME_MAX]="";
char segmentation[FILENAME_MAX]="";
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring,lopts,&index,vars))) {
   switch(val) {
   case 'x': output_style=XML; break;
   case 't': output_style=TEI; break;
   case 'n': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty normalization grammar name\n");
             }
             strcpy(normalization,vars->optarg);
             break;
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(output,vars->optarg);
             break;
   case 'a': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file name\n");
             }
             strcpy(alphabet,vars->optarg);
             break;
   case 's': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty segmentation grammar name\n");
             }
             strcpy(segmentation,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
if (alphabet[0]=='\0') {
   fatal_error("You must specify the alphabet file\n");
}
if (segmentation[0]=='\0') {
   fatal_error("You must specify the segmentation grammar to use\n");
}
char input[FILENAME_MAX];
strcpy(input,argv[vars->optind]);
char snt[FILENAME_MAX];
remove_extension(input,snt);
strcat(snt,"_tmp.snt");
char tmp[FILENAME_MAX];
remove_extension(input,tmp);
strcat(tmp,".tmp");
normalize(input,snt,KEEP_CARRIDGE_RETURN,normalization);
struct fst2txt_parameters* p=new_fst2txt_parameters();
p->text_file=strdup(snt);
if (p->text_file==NULL) {
   fatal_alloc_error("main_XMLizer");
}
p->temp_file=strdup(tmp);
if (p->temp_file==NULL) {
   fatal_alloc_error("main_XMLizer");
}
p->fst_file=strdup(segmentation);
if (p->fst_file==NULL) {
   fatal_alloc_error("main_XMLizer");
}
p->alphabet_file=strdup(alphabet);
if (p->alphabet_file==NULL) {
   fatal_alloc_error("main_XMLizer");
}
p->output_policy=MERGE_OUTPUTS;
p->tokenization_policy=WORD_BY_WORD_TOKENIZATION;
p->space_policy=DONT_START_WITH_SPACE;
main_fst2txt(p);
free_fst2txt_parameters(p);
if (output[0]=='\0') {
   remove_extension(input,output);
	strcat(output,".xml");
}
xmlize(snt,output,output_style);
af_remove(snt);
af_remove(tmp);
free_OptVars(vars);
return 0;
}



void xmlize(char* fin,char* fout,int ouput_style) {
	U_FILE* input = u_fopen(UTF16_LE, fin, U_READ);
	if (input == NULL) fatal_error("Input file '%s' not found!\n", fin);

	U_FILE* output = u_fopen(UTF8, fout, U_WRITE);
	if (output == NULL) {
		u_fclose(input);
		fatal_error("Cannot open output file '%s'!\n", fout);
	} else

	if(ouput_style==XML) {
	   u_fprintf(output, xml_open);
	}
	else {
	   u_fprintf(output, tei_open);
	}

	int sentence_count = 1;
   int sentence_count_relative = 1;
   int paragraph_count = 1;

	u_fprintf(output, "<p><s id=\"n%d\" xml:id=\"d1p%ds%d\">",sentence_count++,paragraph_count,sentence_count_relative++);

	int current_state = 0;
	unichar c;
	int i;
	while ((i = u_fgetc(input)) != EOF) {
		c = (unichar)i;
		switch (current_state) {
			case 0: {
				if ( c == '{') current_state = 1;
				else if(c == '&') u_fprintf(output, "&amp;");
				else if(c == '<') u_fprintf(output, "&lt;");
				else if(c == '>') u_fprintf(output, "&gt;");
				else u_fputc(c, output);
				break;
			}
			case 1: {
				if (c == 'S') current_state = 2;
				else {
					u_fputc('{', output);
					u_fputc(c, output);
					current_state = 0;
				}
				break;
			}
			case 2: {
				if (c == '}') current_state = 3;
				else {
					u_fputc('{', output);
					u_fputc('S', output);
					u_fputc(c, output);
					current_state = 0;
				}
				break;
			}
			case 3: {
				if (c == '{') current_state = 4;
				else if (c == '\n' || c == ' ' || c == '\t') {
					u_fputc(c, output);
					current_state = 3;
				}
				else {
					u_fprintf(output, "</s><s id=\"n%d\" xml:id=\"d1p%ds%d\">",sentence_count++,paragraph_count,sentence_count_relative++);
					u_fputc(c, output);
					current_state = 0;
				}
				break;
			}
			case 4: {
				if (c == 'S') current_state = 7;
				else if (c == 'P') current_state = 5;
				else {
					u_fputc('{', output);
					u_fputc(c, output);
					current_state = 0;
				}
				break;
			}
			case 5: {
				if (c == '}') {
					u_fprintf(output, "</s></p>\n");
					paragraph_count++;
					sentence_count_relative=1;
					current_state = 6;
				} else {
					u_fputc('{', output);
					u_fputc('P', output);
					u_fputc(c, output);
					current_state = 0;
				}
				break;
			}
			case 6: {
				if (c == '\n' || c == ' ' || c == '\t') u_fputc(c, output);
				else {
					u_fprintf(output, "<p><s id=\"n%d\" xml:id=\"d1p%ds%d\">",sentence_count++,paragraph_count,sentence_count_relative++);
					u_fputc(c, output);
					current_state = 0;
				}
				break;
			}
			case 7: {
				if (c == '}') {
					current_state = 3;
				}
				else {
					u_fputc('{', output);
					u_fputc('S', output);
					u_fputc(c, output);
					current_state = 0;
				}
				break;
			}
		}
	}

	if (current_state == 3) {
		//...
	} else if (current_state == 6) {
		//...
	} else {
		u_fprintf(output, "</s></p>\n");
	}

	if(ouput_style==XML) {
	   u_fprintf(output, xml_close);
	}
	else {
	   u_fprintf(output, tei_close);
	}

	u_fclose(input);
	u_fclose(output);
	u_printf("Done.\n");
}

/*
 * Unitex
 *
 * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

static char *xml_open = "<?xml version='1.0' encoding='UTF-8'?>\n<text>";
static char *xml_close = "</text>";
static char *tei_open = "<?xml version='1.0' encoding='UTF-8'?>\n<tei>\n<teiHeader/>\n<text>\n<body>\n<div xml:id=\"d1\">\n";
static char *tei_close = "</div>\n</body>\n</text>\n</tei>";



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
const struct option lopts[] = {
   {"xml", no_argument, NULL, 'x'},
   {"tei", no_argument, NULL, 't'},
   {"normalization", required_argument, NULL, 'n'},
   {"output", required_argument, NULL, 'o'},
   {"alphabet", required_argument, NULL, 'a'},
   {"segmentation_grammar", required_argument, NULL, 's'},
   {"help", no_argument, NULL, 'h'},
   {NULL, no_argument, NULL, 0}
};

int output_style=TEI;
char output[FILENAME_MAX]="";
char alphabet[FILENAME_MAX]="";
char normalization[FILENAME_MAX]="";
char segmentation[FILENAME_MAX]="";
int val,index=-1;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'x': output_style=XML; break;
   case 't': output_style=TEI; break;
   case 'n': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty normalization grammar name\n");
             }
             strcpy(normalization,optarg);
             break;      
   case 'o': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(output,optarg);
             break;      
   case 'a': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file name\n");
             }
             strcpy(alphabet,optarg);
             break;      
   case 's': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty segmentation grammar name\n");
             }
             strcpy(segmentation,optarg);
             break;      
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",optopt); 
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",optopt); 
             else fatal_error("Invalid option --%s\n",optarg);
             break;
   }
   index=-1;
}

if (optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
if (alphabet[0]=='\0') {
   fatal_error("You must specify the alphabet file\n");
}
if (segmentation[0]=='\0') {
   fatal_error("You must specify the segmentation grammar to use\n");
}
char input[FILENAME_MAX];
strcpy(input,argv[optind]);
char snt[FILENAME_MAX];
remove_extension(input,snt);
strcat(snt,"_tmp.snt");
char tmp[FILENAME_MAX];
remove_extension(input,tmp);
strcat(tmp,".tmp");
normalize(input,snt,KEEP_CARRIDGE_RETURN,normalization);
struct fst2txt_parameters* p=new_fst2txt_parameters();
p->text_file=strdup(snt);
p->temp_file=strdup(tmp);
p->fst_file=strdup(segmentation);
p->alphabet_file=strdup(alphabet);
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
remove(snt);
remove(tmp);
return 0;
}



void xmlize(char* fin,char* fout,int ouput_style) {
	FILE* input = u_fopen(UTF16_LE, fin, U_READ);
	if (input == NULL) fatal_error("Input file '%s' not found!\n", fin);

	FILE* output = u_fopen(UTF8, fout, U_WRITE);
	if (output == NULL) {
		u_fclose(input);
		fatal_error("Cannot open output file '%s'!\n", fout);
	} else

	if(ouput_style==XML) { 
	   u_fprintf(UTF8, output, xml_open);
	}
	else { 
	   u_fprintf(UTF8, output, tei_open);
	}

	int sentence_count = 1;
   int sentence_count_relative = 1;
   int paragraph_count = 1;
   
	u_fprintf(UTF8, output, "<p><s id=\"n%d\" xml:id=\"d1p%ds%d\">",sentence_count++,paragraph_count,sentence_count_relative++);

	int current_state = 0;
	unichar c;
	int i;
	while ((i = u_fgetc(input)) != EOF) {
		c = (unichar)i;
		switch (current_state) {
			case 0: {
				if ( c == '{') current_state = 1;
				else if(c == '&') u_fprintf(UTF8, output, "&amp;");
				else if(c == '<') u_fprintf(UTF8, output, "&lt;");
				else if(c == '>') u_fprintf(UTF8, output, "&gt;");
				else u_fputc(UTF8, c, output);
				break;
			}
			case 1: {
				if (c == 'S') current_state = 2;
				else {
					u_fputc(UTF8, '{', output);
					u_fputc(UTF8, c, output);
					current_state = 0;
				}
				break;
			}
			case 2: {
				if (c == '}') current_state = 3;
				else {
					u_fputc(UTF8, '{', output);
					u_fputc(UTF8, 'S', output);
					u_fputc(UTF8, c, output);
					current_state = 0;
				}
				break;
			}
			case 3: {
				if (c == '{') current_state = 4;
				else if (c == '\n' || c == ' ' || c == '\t') {
					u_fputc(UTF8, c, output);
					current_state = 3;
				}
				else {
					u_fprintf(UTF8, output, "</s><s id=\"n%d\" xml:id=\"d1p%ds%d\">",sentence_count++,paragraph_count,sentence_count_relative++);
					u_fputc(UTF8, c, output);
					current_state = 0;
				}
				break;
			}
			case 4: {
				if (c == 'S') current_state = 7;
				else if (c == 'P') current_state = 5;
				else {
					u_fputc(UTF8, '{', output);
					u_fputc(UTF8, c, output);
					current_state = 0;
				}
				break;
			}
			case 5: {
				if (c == '}') {
					u_fprintf(UTF8, output, "</s></p>\n");
               paragraph_count++;
               sentence_count_relative=1;
					current_state = 6;
				} else {
					u_fputc(UTF8, '{', output);
					u_fputc(UTF8, 'P', output);
					u_fputc(UTF8, c, output);
					current_state = 0;
				}
				break;
			}
			case 6: {
				if (c == '\n' || c == ' ' || c == '\t') u_fputc(UTF8, c, output);
				else {
					u_fprintf(UTF8, output, "<p><s id=\"n%d\" xml:id=\"d1p%ds%d\">",sentence_count++,paragraph_count,sentence_count_relative++);
					u_fputc(UTF8, c, output);
					current_state = 0;
				}
				break;
			}
			case 7: {
				if (c == '}') {
					current_state = 3;
				}
				else {
					u_fputc(UTF8, '{', output);
					u_fputc(UTF8, 'S', output);
					u_fputc(UTF8, c, output);
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
		u_fprintf(UTF8, output, "</s></p>\n");
	}

	if(ouput_style==XML) {
	   u_fprintf(UTF8, output, xml_close);
	}
	else { 
	   u_fprintf(UTF8, output, tei_close);
	}

	u_fclose(input);
	u_fclose(output);
	u_printf("Done.\n");
}

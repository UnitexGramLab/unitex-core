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

#include <stdlib.h>
#include <string.h>
#include "getopt.h"
#include "Copyright.h"
#include "Error.h"
#include "File.h"
#include "Fst2Txt_TokenTree.h"
#include "Fst2TxtAsRoutine.h"
#include "IOBuffer.h"
#include "LocateConstants.h"
#include "NormalizeAsRoutine.h"
#include "ParsingInfo.h"
#include "Transitions.h"
#include "Unicode.h"
#include "XMLizer.h"

/* Getopt variables
 * ---------------- */

static const char *sopts = "hxta:s:o:n:";

static const struct option lopts[] = {
	{"help", no_argument, NULL, 'h'},
	{"xml", no_argument, NULL, 'x'},
	{"tei", no_argument, NULL, 't'},
	{"normalize", required_argument, NULL, 'n'},
	{"alphabet", required_argument, NULL, 'a'},
	{"segment", required_argument, NULL, 's'},
	{"output", required_argument, NULL, 'o'},
	{NULL, no_argument, NULL, 0}
};

typedef struct args {
	int xml;
	int tei;

	char *normalize;
	char *alphabet;
	char *sfst;
	char *output;

	char *input;
	char *dname;
	char *fname;
} params;

/* Headers (XML & TEI) Variables
 * ----------------------------- */

static char *xml_open = "<?xml version='1.0' encoding='UTF-8'?>\n<text>";
static char *xml_close = "</text>";
static char *tei_open = "<?xml version='1.0' encoding='UTF-8'?>\n<tei>\n<teiHeader/>\n<text>\n<body>\n<div xml:id=\"d1\">\n";
static char *tei_close = "</div>\n</body>\n</text>\n</tei>";

void usage() {
	u_printf("%S",COPYRIGHT);
	u_printf("Usage: XMLizer [-x|-t] [-n <norm file>] [-o <output file>] -s <segmentation graph> -a <alphabet> <input file>\n"
	         "------\n"
	         "   -h (--help): print this info\n\n"
	         "   -x (--xml): build a simple XML file from a raw text file\n"
	         "   -t (--tei): build a minimal TEI file from a raw text file\n\n"
	         "   -n (--normalize): optional configuration file for the normalization process\n\n"
	         "   -o (--output): optional output file name (default: file.txt > file.xml)\n\n"
	         "   -a (--alphabet): alphabet file\n"
	         "   -s (--segment): segmentation graph ('.fst2' format)\n\n"
	         "Example:\n"
	         "--------\n"
	         "  XMLizer -t -s Sentence.fst2 -a Alphabet.txt -o result.xml original.txt\n"
	         "      -> produces a TEI file named 'result.xml' from the file 'original.txt'\n");
}

int main (int argc, char **argv) {
	/* Every Unitex program must start by this instruction,
	 * in order to avoid display problems when called from
	 * the graphical interface */
	setBufferMode();

	params options;

	options.xml = 0;
	options.tei = 0;
	options.sfst = NULL;

	options.normalize = NULL;

	options.input = NULL;
	options.output = NULL;

	int opt = 0;
	int idx = 0;

	while ((opt = getopt_long(argc, argv, sopts, lopts, &idx)) != -1) {
		switch (opt) {
			case 0: break;
			case 'h':
				usage();
				exit(0);
			case 'x':
				options.xml = 1;
				break;
			case 't':
				options.tei = 1;
				break;
			case 'a' :
				options.alphabet = optarg;
				break;
			case 's' :
				options.sfst = optarg;
				break;
			case 'o' :
				options.output = optarg;
				break;
			case 'n' :
				options.normalize = optarg;
				break;

			case '?':
				usage();
				exit(0);
			case ':':
				usage();
				exit(0);
			default:
				usage();
				exit(0);
		}
	}

	if(options.xml == options.tei) {
		usage();
		exit(0);
	}

	if(optind < argc) { options.input = argv[optind++]; }
	else {
		usage();
		exit(0);
	}

	//u_printf("F: %s | D: %s\n", options.fname, options.dname);
	//u_printf("[x: %d], [t: %d], [s: %s], [o: %s], [i: %s]\n", options.xml, options.tei, options.sfst, options.output, options.input);

	char snt[FILENAME_MAX];
	remove_extension(options.input, snt);
	strcat(snt,"_tmp.snt");

   char tmp[FILENAME_MAX];
	remove_extension(options.input, tmp);
	strcat(tmp, ".tmp");

   normalize(options.input, snt, KEEP_CARRIDGE_RETURN, options.normalize);

   struct fst2txt_parameters* p=new_fst2txt_parameters();
	p->text_file=snt;
	p->temp_file=tmp;
	p->fst_file=options.sfst;
	p->alphabet_file=options.alphabet;
	p->output_policy=MERGE_OUTPUTS;
	p->parsing_mode=PARSING_WORD_BY_WORD;
	main_fst2txt(p);
   
	if(options.output == NULL) {
		options.output = strdup(options.input);
		remove_extension(options.input, options.output);
		strcat(options.output, ".xml");
	}
	xmlize(snt, options.output, options.xml, options.tei);
   remove(snt);
   remove(tmp);
	return 0;
}

void xmlize(char *fin, char *fout, int xml, int tei) {
	FILE* input = u_fopen(UTF16_LE, fin, U_READ);
	if (input == NULL) fatal_error("Input file '%s' not found!\n", fin);

	FILE* output = u_fopen(UTF8, fout, U_WRITE);
	if (output == NULL) {
		u_fclose(input);
		fatal_error("Cannot open output file '%s'!\n", fout);
	} else

	if(xml) { u_fprintf(UTF8, output, xml_open); }
	else if(tei) { u_fprintf(UTF8, output, tei_open); }

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

	if(xml) { u_fprintf(UTF8, output, xml_close); }
	else if(tei) { u_fprintf(UTF8, output, tei_close); }

	u_fclose(input);
	u_fclose(output);
	u_printf("Done.\n");
}

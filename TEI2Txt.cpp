#include <stdlib.h>
#include "getopt.h"
#include <string.h>

#include "Copyright.h"
#include "Error.h"
#include "File.h"
#include "HTMLCharacters.h"
#include "IOBuffer.h"
#include "Unicode.h"

/* Getopt variables
 * ---------------- */

static const char *sopts = "ho:";

static const struct option lopts[] = {
	{"help", no_argument, NULL, 'h'},
	{"output", required_argument, NULL, 'o'},
	{NULL, no_argument, NULL, 0}
};

typedef struct args {
	char *input;
	char *output;
} params;

void tei2txt(char*, char*);

/* Headers (XML & TEI) Variables
 * ----------------------------- */

void usage() {
	u_printf("%S",COPYRIGHT);
	u_printf("Usage: TEI2Txt [-o <output file>] <input file>\n"
	         "------\n"
	         "   -h (--help): print this info\n\n"
	         "   -o (--output): optional output file name (default: file.xml > file.txt)\n\n"
	         "Example:\n"
	         "--------\n"
	         "  TEI2Txt -o result.txt original.xml\n"
	         "      -> produces a raw text file named 'result.txt' from the TEI file 'original.xml'\n");
}

int main (int argc, char **argv) {
	/* Every Unitex program must start by this instruction,
	 * in order to avoid display problems when called from
	 * the graphical interface */
	setBufferMode();

	params options;
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
			case 'o' :
				options.output = optarg;
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

	if(optind < argc) { options.input = argv[optind++]; }
	else {
		usage();
		exit(0);
	}

	if(options.output == NULL) {
		options.output = (char*)malloc((FILENAME_MAX+1)*sizeof(char));
      if (options.output) {
         fatal_error("Not enough memory in TEI2Txt main\n");
      }
		remove_extension(options.input, options.output);
		strcat(options.output, ".txt");
	}
	tei2txt(options.input, options.output);

	return 0;
}

static const char *body = "body";

void tei2txt(char *fin, char *fout) {
	FILE* input = u_fopen(UTF8, fin, U_READ);
	if (input == NULL) fatal_error("Input file '%s' not found!\n", fin);

	FILE* output = u_fopen(UTF16_LE, fout, U_WRITE);
	if (output == NULL) {
		u_fclose(input);
		fatal_error("Cannot open output file '%s'!\n", fout);
	}

	unichar buffer[5000];

	int i, j, k;
	unichar c;
	if((i = u_fgetc(UTF8, input)) != EOF) {
		c = (unichar)i;

		while(1) {
			while(c != '<' && (i = u_fgetc(UTF8, input)) != EOF)
				c = (unichar)i;

			j = 0;
			while((i = u_fgetc(UTF8, input)) != EOF && (c = (unichar)i) != ' '
               && (c = (unichar)i) != '\t' && (c = (unichar)i) != '\n'
               && (c = (unichar)i) != '>') {
				buffer[j++] = c;
			}
			buffer[j] = '\0';
         if (c!='>') {
            /* We do this because we can find <body ...> */
            while((i = u_fgetc(UTF8, input)) != EOF && (c = (unichar)i) != '>');
         }
			//u_printf("Current tag : <%S>\n", buffer);

			if(!u_strcmp(buffer, body)) break;
			else buffer[0] = '\0';
		}
	} else error("Empty TEI file %s\n", fin);

	char schars[11];

   int first_sentence=1;
	int current_state = 0;
   int inside_sentence=0;
	while ((i = u_fgetc(UTF8, input)) != EOF) {
		c = (unichar)i;
		switch (current_state) {
			case 0: {
				if(c == '<') {
               current_state = 1;
               inside_sentence=0;
            }
				else if(c == '&') current_state = 3;
				else if (inside_sentence) u_fputc(UTF16_LE, c, output);
				break;
			}
			case 1: {
				if(c == 's' || c == 'S') current_state = 2;
				else {
					while((i = u_fgetc(UTF8, input)) != EOF) {
						c = (unichar)i;
						if(c == '>') break;
					}
					current_state = 0;
				}
				break;
			}
			case 2: {
				if(c == ' ' || c == '>') {
					current_state = 0;
               inside_sentence=1;
               if (!first_sentence) {
                  u_fputc(UTF16_LE, '\n', output);
					   u_fputc(UTF16_LE, '{', output);
					   u_fputc(UTF16_LE, 'S', output);
					   u_fputc(UTF16_LE, '}', output);
               } else {
                  first_sentence=0;
               }
				}
				if(c != '>') {
					while((i = u_fgetc(UTF8, input)) != EOF) {
						c = (unichar)i;
						if(c == '>') break;
					}
				}
				break;
			}
			case 3: {
				j = 0;
				while(c != ';' && (i = u_fgetc(UTF8, input)) != EOF) {
					//u_printf("Current S-character: %C\n", c);
					schars[j++] = (char)c;
					c = (unichar)i;
				}
				schars[j] = '\0';
				//u_printf("Current S-chain: %S\n", schars);

				k = get_HTML_character(schars, 1);
				switch (k) {
					case UNKNOWN_CHARACTER: {
						u_fputc(UTF16_LE, '?', output);
						break;
					}
					case MALFORMED_HTML_CODE: {
						error("Malformed HTML character declaration &%s;\n", schars);
						u_fputc(UTF16_LE, '?', output);
						break;
					}
					default: {
						c = (unichar)k;
						u_fputc(UTF16_LE, c, output);
						break;
					}
				}

				schars[0] = '\0';
				current_state = 0;
				break;
			}
		}
	}
	
	u_fclose(input);
	u_fclose(output);
	u_printf("Done.\n");
}


#include <stdlib.h>
#include <string.h>
#include "Copyright.h"
#include "Error.h"
#include "File.h"
#include "HTMLCharacters.h"
#include "Unicode.h"
#include "getopt.h"


void tei2txt(char*, char*);


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: TEI2Txt [OPTIONS] <xml>\n"
         "\n"
         "  <xml>: the input TEI file\n"
         "\n"
         "OPTIONS:\n"
	      "  -o TXT/--output=TXT: optional output file name (default: file.xml > file.txt)\n"
         "  -h/--help: this help\n"
         "\n"
         "Produces a raw text file from the given TEI file.\n");
}

int main_TEI2Txt(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":o:h";
const struct option lopts[]={
   {"output", required_argument, NULL, 'o'},
   {"help", no_argument, NULL, 'h'},
   {NULL, no_argument, NULL, 0}
};
char output[FILENAME_MAX]="";
int val,index=-1;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'o': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(output,optarg);
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

if(output[0]=='\0') {
   remove_extension(argv[optind],output);
	strcat(output,".txt");
}
tei2txt(argv[optind],output);
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
            while((i = u_fgetc(UTF8, input)) != EOF && (c = (unichar)i) != '>') {}
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
                  /* We put a {STOP} tag in order to avoid matches that overlap 2 sentences */
                  u_fputc(UTF16_LE, '\n', output);
					   u_fputc(UTF16_LE, '{', output);
					   u_fputc(UTF16_LE, 'S', output);
					   u_fputc(UTF16_LE, 'T', output);
					   u_fputc(UTF16_LE, 'O', output);
					   u_fputc(UTF16_LE, 'P', output);
					   u_fputc(UTF16_LE, '}', output);
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


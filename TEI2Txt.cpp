/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "HTMLCharacters.h"
#include "Unicode.h"
#include "UnitexGetOpt.h"
#include "TEI2Txt.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

int tei2txt(char*, char*, const VersatileEncodingConfig*);

const char* usage_TEI2Txt =
         "Usage: TEI2Txt [OPTIONS] <xml>\n"
         "\n"
         "  <xml>: the input TEI file\n"
         "\n"
         "OPTIONS:\n"
         "  -o TXT/--output=TXT: optional output file name (default: file.xml > file.txt)\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "Produces a raw text file from the given TEI file.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_TEI2Txt);
}

const char* optstring_TEI2Txt=":o:Vhk:q:";
const struct option_TS lopts_TEI2Txt[]={
  {"output", required_argument_TS, NULL, 'o'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help", no_argument_TS, NULL, 'h'},
  {NULL, no_argument_TS, NULL, 0}
};


int main_TEI2Txt(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

char output[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_TEI2Txt,lopts_TEI2Txt,&index))) {
   switch(val) {
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty output file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(output,options.vars()->optarg);
             break;
   case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                return USAGE_ERROR_CODE;
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                return USAGE_ERROR_CODE;
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_TEI2Txt[index].name);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

if (options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

if(output[0]=='\0') {
  remove_extension(argv[options.vars()->optind],output);
    strcat(output,".txt");
}

int return_value = tei2txt(argv[options.vars()->optind],output,&vec);

return return_value;
}

static const char* body = "body";

int tei2txt(char *fin, char *fout, const VersatileEncodingConfig* vec) {
    void* html_ctx = init_HTML_character_context();
    if (html_ctx == NULL) {
    alloc_error("tei2txt");
    return ALLOC_ERROR_CODE;
  }

    U_FILE* input = u_fopen(vec, fin, U_READ);
    if (input == NULL) {
    error("Input file '%s' not found!\n", fin);
    free_HTML_character_context(html_ctx);
    return DEFAULT_ERROR_CODE;
  }

    U_FILE* output = u_fopen(vec, fout, U_WRITE);
    if (output == NULL) {
    error("Cannot open output file '%s'!\n", fout);
    u_fclose(input);
    free_HTML_character_context(html_ctx);
    return DEFAULT_ERROR_CODE;
    }

    unichar buffer[5000];

    int i, j, k;
    unichar c;
    if((i = u_fgetc(input)) != EOF) {
        c = (unichar)i;

        for (;;) {
            while(c != '<' && (i = u_fgetc(input)) != EOF) {
                c = (unichar)i;
      }

            j = 0;
            while((i = u_fgetc(input)) != EOF && (c = (unichar)i) != ' '
               && (c = (unichar)i) != '\t' && (c = (unichar)i) != '\n'
               && (c = (unichar)i) != '>') {
                buffer[j++] = c;
            }
            buffer[j] = '\0';
         if (c!='>') {
            /* We do this because we can find <body ...> */
            while((i = u_fgetc(input)) != EOF && (c = (unichar)i) != '>') {}
         }
            //u_printf("Current tag : <%S>\n", buffer);

            if(!u_strcmp(buffer, body)) {
        break;
      } else {
        buffer[0] = '\0';
      }
        }
    } else {
    error("Empty TEI file %s\n", fin);
  }

    char schars[11];

  int first_sentence=1;
    int current_state = 0;
  int inside_sentence=0;
    while ((i = u_fgetc(input)) != EOF) {
        c = (unichar)i;
        switch (current_state) {
            case 0: {
                if(c == '<') {
               current_state = 1;
               inside_sentence=0;
        } else if(c == '&') {
          current_state = 3;
        } else if (inside_sentence) {
          u_fputc(c, output);
        }
                break;
            }
            case 1: {
                if(c == 's' || c == 'S') {
          current_state = 2;
                } else {
                    while((i = u_fgetc(input)) != EOF) {
                        c = (unichar)i;
                        if(c == '>') {
              break;
            }
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
             u_fprintf(output,"\n{STOP}{S}");
          } else {
             first_sentence=0;
          }
                }
                if(c != '>') {
                    while((i = u_fgetc(input)) != EOF) {
                        c = (unichar)i;
                        if(c == '>') {
              break;
            }
                    }
                }
                break;
            }
            case 3: {
                j = 0;
                while(c != ';' && (i = u_fgetc(input)) != EOF) {
                    //u_printf("Current S-character: %C\n", c);
                    schars[j++] = (char)c;
                    c = (unichar)i;
                }
                schars[j] = '\0';
                //u_printf("Current S-chain: %S\n", schars);

                k = get_HTML_character(html_ctx,schars, 1);
                switch (k) {
                    case UNKNOWN_CHARACTER: {
                        u_fputc('?', output);
                        break;
                    }
                    case MALFORMED_HTML_CODE: {
                        error("Malformed HTML character declaration &%s;\n", schars);
                        u_fputc('?', output);
                        break;
                    }
                    default: {
                        c = (unichar)k;
                        u_fputc(c, output);
                        break;
                    }
                }

                schars[0] = '\0';
                current_state = 0;
                break;
            }
        }
    }

    u_fclose(output);
    u_fclose(input);
  free_HTML_character_context(html_ctx);
    u_printf("Done.\n");

  return SUCCESS_RETURN_CODE;
}

} // namespace unitex

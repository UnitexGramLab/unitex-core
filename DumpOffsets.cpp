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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS) 
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.ergonotics.com/unitex-contribution/
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
#include "UnitexGetOpt.h"
#include "Token.h"
#include "Text_tokens.h"
#include "DumpOffsets.h"
#include "Offsets.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_DumpOffsets =
         "Usage: DumpOffsets [OPTIONS] <txt>\n"
         "\n"
         "  <txt>: a offset file to read\n"
         "\n"
         "OPTIONS:\n"
         "  -o X/--old=X: name of old file to read\n"
		 "  -n X/--new=X: name of new file to read\n"
         "  -p X/--output=X: name of output dump file to write\n"
         "  -c/--no_escape_sequence: don't escape text sequence\n"
         "  -h/--help: this help\n"
         "\n"
         "DumpOffsets dump sequence offset to study them.\n"
		 "\n" \
		 "Example:\n" \
		 "UnitexToolLogger Normalize -r .\\resource\\Norm.txt .\\work\\text_file.txt --output_offsets .\\work\\text_file_offset.txt\n" \
		 "UnitexToolLogger DumpOffsets -o .\\work\\text_file_offset.txt -n .\\work\\text_file_offset.snt -p .\\work\\dump\\dump_offsets.txt .\\work\\text_file_offset.txt\n" \
		 "\n" \
		 "\n" \
		 "Usage: DumpOffsets [-m/--merge] [OPTIONS] <txt>\n"
		 "\n"
		 "  <txt>: a offset file to read\n"
		 "\n"
		 "OPTIONS:\n"
		 "  -o X/--old=X: name of old offset file to read\n"
		 "  -p X/--output=X: name of output merged offset file to write\n"
		 "  -h/--help: this help\n"
		 "\n"
		 ;

static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_DumpOffsets);
}


const char* optstring_DumpOffsets=":hfmo:n:p:k:q:";
const struct option_TS lopts_DumpOffsets[]={
   {"old",required_argument_TS, NULL,'o'},
   {"new",required_argument_TS,NULL,'n'},
   {"output",required_argument_TS,NULL,'p'},
   {"no_escape_sequence",required_argument_TS,NULL,'c'},
   {"merge",no_argument_TS,NULL,'m'},
   {"full",no_argument_TS,NULL,'f'},
   {"input_encoding",required_argument_TS,NULL,'k'},
   {"output_encoding",required_argument_TS,NULL,'q'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};




#define READ_FILE_BUFFER_SIZE 65536

/**
* This code run well, but in really not optimized
*/
static unichar* read_file(U_FILE *f,int * filesize){

	unichar *text = NULL;
	*filesize = 0;
	
	text = (unichar *)malloc(sizeof(unichar));
	if (text == NULL){
		fatal_alloc_error("malloc");
	}
	text[0] = '\0';

	int total_read = 0;
	int read;
	do {
		unichar buffer[READ_FILE_BUFFER_SIZE + 1];
		memset(buffer, 0, sizeof(unichar)*(READ_FILE_BUFFER_SIZE + 1));

		for (read = 0; read < READ_FILE_BUFFER_SIZE; read++)
		{
			int r = u_fgetc_raw(f);
			if (r == EOF)
				break;
			*(buffer + read) = (unichar)r;
		}
		
		total_read += u_strlen(buffer);
		text = (unichar *)realloc(text, sizeof(unichar)*(total_read + 1));
		if (text == NULL){
			fatal_alloc_error("realloc");
		}
		u_strcat(text, buffer);

	} while (read == READ_FILE_BUFFER_SIZE);

	text[total_read] = '\0';
	*filesize = total_read;
	return text;
}


static void read_file(const VersatileEncodingConfig* cfg, const char*filename, unichar** buffer, int *filesize)
{
	U_FILE* f = u_fopen(cfg, filename, U_READ);
	if (f == NULL) {
		fatal_error("cannot read file %s", filename);
	}
	*buffer = read_file(f, filesize);
	u_fclose(f);
}


static void DumpSequence(U_FILE* f,const unichar* text, int textsize, int start, int end, int escape)
{
	if (end <= start)
	{
		u_fprintf(f, "empty sequence : end before start\n");
	}
	else
	if (end > textsize)
	{
		u_fprintf(f, "invalid sequence : end after end of file\n");
	}
	else
	{
		u_fprintf(f, "'");
		for (int i = start; i < end; i++) {
			unichar c = *(text + i);
			if (escape) {
				if (c >= 0x20) {
					u_fputc(c, f);
				}
				else {
					switch (c)
					{
						case '\r' : u_fprintf(f, "\\r"); break;
						case '\n': u_fprintf(f, "\\n"); break;
						case '\t': u_fprintf(f, "\\t"); break;
						default: u_fprintf(f,"\\x%02x", (unsigned int)c); break;
					}
				}


			}
			else {
				u_fputc(c, f);
			}
		}
		u_fprintf(f, "'\n");
	}
}

int main_DumpOffsets(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

char old_filename[FILENAME_MAX]="";
char new_filename[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
char offset_file_name[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
int escape=1;
int merge=0;
int full=0;
int val,index=-1;
//char foo=0;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_DumpOffsets,lopts_DumpOffsets,&index,vars))) {
   switch(val) {
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty old file name\n");
             }
             strcpy(old_filename, vars->optarg);
             break;
   case 'n': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty new file name\n");
             }
             strcpy(new_filename, vars->optarg);
             break;
   case 'p': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(output, vars->optarg);
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
   case 'c': escape = 0; break;
   case 'f': full = 1; break;
   case 'm': merge = 1; break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_DumpOffsets[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}


strcpy(offset_file_name, argv[vars->optind]);

if (merge) {
	vector_offset* prev_offsets = load_offsets(&vec, old_filename);
	vector_offset* offsets = load_offsets(&vec, offset_file_name);

	U_FILE* f_output_offsets = u_fopen(&vec, output, U_WRITE);
	if (f_output_offsets == NULL) {
		error("Cannot create offset file %s\n", output);
		return 1;
	}
	process_offsets(prev_offsets, offsets, f_output_offsets);
	u_fclose(f_output_offsets);
	u_printf("\nDumpOffsets dump done, file %s created.\n", output);
}
else {
	unichar* old_text = NULL;
	int old_size = 0;
	read_file(&vec, old_filename, &old_text, &old_size);

	unichar* new_text = NULL;
	int new_size = 0;
	read_file(&vec, new_filename, &new_text, &new_size);

	vector_offset* offsets = load_offsets(&vec, offset_file_name);
	if (offsets == NULL) {
		fatal_error("cannot read file %s", offset_file_name);
	}

	U_FILE* fout = u_fopen(&vec, output, U_WRITE);
	for (int i = 0; i < offsets->nbelems; i++) {
		Offsets curOffset = offsets->tab[i];
		if (i > 0) {
			if (full) {
				Offsets prevOffset = offsets->tab[i-1];
				u_fprintf(fout, "===========================================\n\n");
				u_fprintf(fout, "Common zone:\n\n");
				DumpSequence(fout, old_text, old_size, prevOffset.old_end+1, curOffset.old_start-1, escape);
				DumpSequence(fout, new_text, new_size, prevOffset.new_end + 1, curOffset.new_start - 1, escape);
			}
			u_fprintf(fout, "-------------------------------------------\n\n");
		}
		u_fprintf(fout, "%8d: %d.%d -> %d,%d\n", i, curOffset.old_start, curOffset.old_end, curOffset.new_start, curOffset.new_end);
		DumpSequence(fout, old_text, old_size, curOffset.old_start, curOffset.old_end, escape);
		DumpSequence(fout, new_text, new_size, curOffset.new_start, curOffset.new_end, escape);
	}

	u_fclose(fout);
	free_vector_offset(offsets);
	free(old_text);
	free(new_text);
	u_printf("\nDumpOffsets dump done, file %s created.\n", output);
}
free_OptVars(vars);

return 0;
}

} // namespace unitex

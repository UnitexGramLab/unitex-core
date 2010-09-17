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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "DELA.h"
#include "DictionaryTree.h"
#include "String_hash.h"
#include "AutomatonDictionary2Bin.h"
#include "File.h"
#include "Copyright.h"
#include "Error.h"
#include "getopt.h"
#include "Compress.h"
#include "ProgramInvoker.h"

const char* usage_Compress =
         "Usage: Compress [OPTIONS] <dictionary>\n"
         "\n"
         "  <dictionary>: any unicode DELAF or DELACF dictionary\n"
         "\n"
         "OPTIONS:\n"
         "  -o file.bin/--output==file.bin: name of destination file\n"
         "  -f/--flip: specifies that the inflected and lemma forms must be swapped\n"
         "  -s/--semitic: uses the semitic compression algorithm\n"
         "  -h/--help: this help\n"
         "\n"
         "Compresses a dictionary into an finite state automaton. This automaton\n"
         "is stored is a .bin file, and the associated flexional codes are\n"
         "written in a .inf file.\n\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Compress);
}


/**
 * This function writes the number of INF codes 'n' at the beginning
 * of the file named 'name'. This file is supposed to be a UTF-16
 * Little-Endian one, starting by a sequence made of ten zeros.
 */
void write_INF_file_header(char* name,int n) {
U_FILE* f;

f=u_fopen_existing_unitex_text_format(name,U_MODIFY);

char number[11];
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


int pseudo_main_Compress(Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input,
                         int flip,int semitic,char* dic) {
ProgramInvoker* invoker=new_ProgramInvoker(main_Compress,"main_Compress");
char tmp[200];
{
    tmp[0]=0;
    get_reading_encoding_text(tmp,sizeof(tmp)-1,mask_encoding_compatibility_input);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-k");
        add_argument(invoker,tmp);
    }

    tmp[0]=0;
    get_writing_encoding_text(tmp,sizeof(tmp)-1,encoding_output,bom_output);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-q");
        add_argument(invoker,tmp);
    }
}
if (flip) {
   add_argument(invoker,"-f");
}
if (semitic) {
   add_argument(invoker,"-s");
}
add_argument(invoker,dic);
int ret=invoke(invoker);
free_ProgramInvoker(invoker);
return ret;
}


const char* optstring_Compress=":fshk:q:o:";
const struct option_TS lopts_Compress[]= {
      {"flip",no_argument_TS,NULL,'f'},
      {"semitic",no_argument_TS,NULL,'s'},
      {"help",no_argument_TS,NULL,'h'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"output",required_argument_TS,NULL,'o'},
      {NULL,no_argument_TS,NULL,0}
};


/**
 * This program reads a .dic file and compress it into a .bin and a .inf file.
 * First, it builds a tree with all the entries, and then, it builds a minimal
 * transducer from this tree, using the Dominique Revuz's algorithm.
 */
int main_Compress(int argc, char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

int FLIP=0;
int semitic=0;
char bin[DIC_WORD_SIZE];
char inf[DIC_WORD_SIZE];
Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
bin[0]='\0';
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Compress,lopts_Compress,&index,vars))) {
   switch(val) {
   case 'f': FLIP=1; break;
   case 's': semitic=1; break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Compress[index].name);
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output\n");
             }
             strcpy(bin,vars->optarg);
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
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}

U_FILE* f;
U_FILE* INF_file;
unichar s[DIC_WORD_SIZE];
struct dela_entry* entry;
struct dictionary_node* root; /* Root of the dictionary tree */
struct string_hash* INF_codes; /* Structure that will contain all the INF codes */
int line=0; /* Current line number */

f=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,argv[vars->optind],U_READ);
if (f==NULL) {
	fatal_error("Cannot open %s\n",argv[vars->optind]);
}
/* We compute the name of the output .bin and .inf files */
if (bin[0]=='\0') {
	strcpy(bin,argv[vars->optind]);
}
remove_extension(bin);
strcat(bin,".bin");
remove_extension(bin,inf);
strcat(inf,".inf");
INF_file=u_fopen_creating_unitex_text_format(encoding_output,bom_output,inf,U_WRITE);
if (INF_file==NULL) {
	u_fclose(f);
	fatal_error("Cannot create %s\n",inf);
}
/* First, we print a sequence of zeros at the beginning of the .inf file
 * in order to book some place, so that we can later come and write there
 * the number of lines of this file. */
u_fprintf(INF_file,"0000000000\n");
root=new_dictionary_node();
INF_codes=new_string_hash();
unichar tmp[DIC_WORD_SIZE];
u_printf("Compressing...\n");
/* We read until there is no more lines in the .dic file */
while(EOF!=u_fgets_limit2(s,DIC_WORD_SIZE,f)) {
	if (s[0]=='\0') {
		/* Empty lines should not appear in a .dic file */
		error("Line %d: empty line\n",line);
	}
	else if (s[0]=='/') {
		/* We do nothing if the line begins by a '/', because
		 * it is considered as a comment line. */
	}
	else {
		/* If we have a line, we tokenize it */
		entry=tokenize_DELAF_line(s,1,1,NULL);
		if (entry!=NULL) {
			/* If the entry is well-formed */

			if (FLIP) {
				/* If the "-flip" parameter has been used, we flip
				 * the inflected form and the lemma of the entry */
				unichar* o=entry->inflected;
				entry->inflected=entry->lemma;
				entry->lemma=o;
			}

			if (contains_unprotected_equal_sign(entry->inflected)
				|| contains_unprotected_equal_sign(entry->lemma)) {
				/* If the inflected form or lemma contains any unprotected = sign,
				 * we must insert the space entry and the - entry:
				 * pomme=de=terre,.N  ->  pomme de terre,pomme de terre.N
				 *                        pomme-de-terre,pomme-de-terre.N
				 */
				unichar inf_tmp[DIC_WORD_SIZE];
				unichar lem_tmp[DIC_WORD_SIZE];
				u_strcpy_sized(inf_tmp,DIC_WORD_SIZE,entry->inflected);
				u_strcpy_sized(lem_tmp,DIC_WORD_SIZE,entry->lemma);
				/* We replace the unprotected = signs by spaces */
				replace_unprotected_equal_sign(entry->inflected,(unichar)' ');
				replace_unprotected_equal_sign(entry->lemma,(unichar)' ');
				/* And then we unprotect the other = signs */
				unprotect_equal_signs(entry->inflected);
				unprotect_equal_signs(entry->lemma);
				/* We insert "pomme de terre,pomme de terre.N" */
				get_compressed_line(entry,tmp,semitic);
				add_entry_to_dictionary_tree(entry->inflected,tmp,root,INF_codes);
				/* And then we insert "pomme-de-terre,pomme-de-terre.N" */
				u_strcpy(entry->inflected,inf_tmp);
				u_strcpy(entry->lemma,lem_tmp);
				/* We replace the unprotected = signs by minus */
				replace_unprotected_equal_sign(entry->inflected,(unichar)'-');
				replace_unprotected_equal_sign(entry->lemma,(unichar)'-');
				/* And then we unprotect the other = signs */
				unprotect_equal_signs(entry->inflected);
				unprotect_equal_signs(entry->lemma);
				get_compressed_line(entry,tmp,semitic);
				add_entry_to_dictionary_tree(entry->inflected,tmp,root,INF_codes);
			}
			else {
				/* If the entry does not contain any unprotected = sign,
				 * we unprotect the = signs */
				unprotect_equal_signs(entry->inflected);
				unprotect_equal_signs(entry->lemma);
				get_compressed_line(entry,tmp,semitic);
				add_entry_to_dictionary_tree(entry->inflected,tmp,root,INF_codes);
			}
			/* and last, but not least: don't forget to free your memory
			 * or it would be impossible to compress large dictionaries */
			 free_dela_entry(entry);
		}
	}
	/* We print something at regular intervals in order to show
	 * that the program actually works */
	if (line%10000==0) {
		u_printf("%d line%s read...       \r",line,(line>1)?"s":"");
	}
	line++;
}
u_fclose(f);
/* Now we can dump the INF codes into the .inf file */
dump_values(INF_file,INF_codes);
u_fclose(INF_file);
/* We build a minimal transducer from the entry tree */
minimize_tree(root);
int n_states;
int n_transitions;
int bin_size;
/* And we dump it into the .bin file */
create_and_save_bin(root,bin,&n_states,&n_transitions,&bin_size);
u_printf("Binary file: %d bytes\n",bin_size);
u_printf("%d line%s read            \n"
         "%d INF entr%s created\n",
         line,
         (line!=1)?"s":"",
         INF_codes->size,
         (INF_codes->size!=1)?"ies":"y");
u_printf("%d states, %d transitions\n",n_states,n_transitions);
write_INF_file_header(inf,INF_codes->size);
free_OptVars(vars);
/*
 * WARNING: we do not free the 'INF_codes' structure because of a slowness
 *          problem with very large INF lines.
 */

#if (defined(UNITEX_LIBRARY) || defined(UNITEX_RELEASE_MEMORY_AT_EXIT))
/* cleanup for no leak on library */
free_string_hash(INF_codes);
free_dictionary_node(root);
#endif
return 0;
}


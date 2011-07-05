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
#include "DELA.h"
#include "DictionaryTree.h"
#include "String_hash.h"
#include "AutomatonDictionary2Bin.h"
#include "File.h"
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "Compress.h"
#include "ProgramInvoker.h"
#include "BitArray.h"
#include "CompressedDic.h"
#include "Ustring.h"

const char* usage_Compress =
         "Usage: Compress [OPTIONS] <dictionary>\n"
         "\n"
         "  <dictionary>: any unicode DELAF or DELACF dictionary\n"
         "\n"
         "OPTIONS:\n"
         "  -o BIN/--output=BIN: name of destination file (by default, xxx.dic produces\n"
		 "                       xxx.bin)"
         "  -f/--flip: specifies that the inflected and lemma forms must be swapped\n"
         "  -s/--semitic: uses the semitic compression algorithm\n"
		 "  --v1: produces an old style .bin file\n"
		 "  --v2: produces a new style .bin file, with no file size limitation to 16Mb\n"
		 "        and a smaller size (default)\n"
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
void write_INF_file_header(VersatileEncodingConfig* vec,char* name,int n) {
U_FILE* f=u_fopen(vec,name,U_MODIFY);
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


int pseudo_main_Compress(VersatileEncodingConfig* vec,
                         int flip,int semitic,char* dic,int new_style_bin) {
ProgramInvoker* invoker=new_ProgramInvoker(main_Compress,"main_Compress");
char tmp[200];
tmp[0]=0;
get_reading_encoding_text(tmp,sizeof(tmp)-1,vec->mask_encoding_compatibility_input);
if (tmp[0] != '\0') {
    add_argument(invoker,"-k");
    add_argument(invoker,tmp);
}
tmp[0]=0;
get_writing_encoding_text(tmp,sizeof(tmp)-1,vec->encoding_output,vec->bom_output);
if (tmp[0] != '\0') {
    add_argument(invoker,"-q");
    add_argument(invoker,tmp);
}
if (flip) {
   add_argument(invoker,"-f");
}
if (semitic) {
   add_argument(invoker,"-s");
}
if (new_style_bin) {
	add_argument(invoker,"--v2");
} else {
	add_argument(invoker,"--v1");
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
      {"v1",no_argument_TS,NULL,1},
      {"v2",no_argument_TS,NULL,2},
      {"bin2",no_argument_TS,NULL,3},
      {NULL,no_argument_TS,NULL,0}
};

extern void rebuild_token_semitic(unichar* inflected,unichar* compress_info);


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
BinType bin_type=BIN_CLASSIC;
int new_style_bin=1;
int semitic=0;
char bin[DIC_WORD_SIZE];
char inf[DIC_WORD_SIZE];
VersatileEncodingConfig vec={DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT,DEFAULT_ENCODING_OUTPUT,DEFAULT_BOM_OUTPUT};
bin[0]='\0';
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Compress,lopts_Compress,&index,vars))) {
   switch(val) {
   case 'f': FLIP=1; break;
   case 's': semitic=1; break;
   case 1: new_style_bin=0; bin_type=BIN_CLASSIC; break;
   case 2: new_style_bin=1; bin_type=BIN_CLASSIC; break;
   case 3: new_style_bin=1; bin_type=BIN_BIN2; break;
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
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),vars->optarg);
             break;
   case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),vars->optarg);
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


Abstract_allocator compress_abstract_allocator=NULL;
Abstract_allocator compress_tokenize_abstract_allocator=NULL;
int tokenize_allocator_has_clean = 0;

compress_abstract_allocator=create_abstract_allocator("main_Compress",AllocatorCreationFlagAutoFreePrefered);
compress_tokenize_abstract_allocator=create_abstract_allocator("main_Compress_tokenize_first",AllocatorCreationFlagAutoFreePrefered | AllocatorCreationFlagCleanPrefered);
tokenize_allocator_has_clean = ((get_allocator_flag(compress_tokenize_abstract_allocator) & AllocatorCleanPresent) != 0);

U_FILE* f;
U_FILE* INF_file=NULL;
struct dela_entry* entry;
struct dictionary_node* root; /* Root of the dictionary tree */
struct string_hash* INF_codes; /* Structure that will contain all the INF codes */
int line=0; /* Current line number */

f=u_fopen(&vec,argv[vars->optind],U_READ);
if (f==NULL) {
	fatal_error("Cannot open %s\n",argv[vars->optind]);
}
/* We compute the name of the output .bin and .inf files */
if (bin[0]=='\0') {
	strcpy(bin,argv[vars->optind]);
	remove_extension(bin);
	strcat(bin,".bin");
	if (bin_type==BIN_BIN2) {
		strcat(bin,"2");
	}
}
remove_extension(bin,inf);
strcat(inf,".inf");
if (bin_type==BIN_CLASSIC) {
	INF_file=u_fopen(&vec,inf,U_WRITE);
	if (INF_file==NULL) {
		u_fclose(f);
		fatal_error("Cannot create %s\n",inf);
	}
	/* First, we print a sequence of zeros at the beginning of the .inf file
	 * in order to book some place, so that we can later come and write there
	 * the number of lines of this file. */
	u_fprintf(INF_file,"0000000000\n");
}
root=new_dictionary_node(compress_abstract_allocator);
INF_codes=new_string_hash();
unichar tmp[DIC_WORD_SIZE];
u_printf("Compressing...\n");
/* We read until there is no more lines in the .dic file */
Ustring* s=new_Ustring(DIC_WORD_SIZE);
while(EOF!=readline(s,f)) {
	if (s->str[0]=='\0') {
		/* Empty lines should not appear in a .dic file */
		error("Line %d: empty line\n",line);
	}
	else if (s->str[0]=='/') {
		/* We do nothing if the line begins by a '/', because
		 * it is considered as a comment line. */
	}
	else {
		/* If we have a line, we tokenize it */
		entry=tokenize_DELAF_line(s->str,1,1,NULL,compress_tokenize_abstract_allocator);
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
				add_entry_to_dictionary_tree(entry->inflected,tmp,root,INF_codes,line,compress_abstract_allocator);
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
				add_entry_to_dictionary_tree(entry->inflected,tmp,root,INF_codes,line,compress_abstract_allocator);
			}
			else {
				/* If the entry does not contain any unprotected = sign,
				 * we unprotect the = signs */
				unprotect_equal_signs(entry->inflected);
				unprotect_equal_signs(entry->lemma);
				get_compressed_line(entry,tmp,semitic);
				add_entry_to_dictionary_tree(entry->inflected,tmp,root,INF_codes,line,compress_abstract_allocator);
			}
			/* and last, but not least: don't forget to free your memory
			 * or it would be impossible to compress large dictionaries */
			if (tokenize_allocator_has_clean == 0) {
				free_dela_entry(entry,compress_tokenize_abstract_allocator);
			}
			else {
				clean_allocator(compress_tokenize_abstract_allocator);
			}
		}
	}
	/* We print something at regular intervals in order to show
	 * that the program actually works */
	if (line%10000==0) {
		u_printf("%d line%s read...       \r",line,(line>1)?"s":"");
		if (compress_tokenize_abstract_allocator != NULL)
			if (tokenize_allocator_has_clean == 0)
			{
				close_abstract_allocator(compress_tokenize_abstract_allocator);
				compress_tokenize_abstract_allocator=create_abstract_allocator("main_Compress_tokenize",AllocatorCreationFlagAutoFreePrefered | AllocatorCreationFlagCleanPrefered);
			}
	}
	line++;
}
free_Ustring(s);
u_fclose(f);
struct bit_array* used_inf_values=new_bit_array(INF_codes->size,ONE_BIT);
if (bin_type==BIN_BIN2) {
	/* For a .bin2 dictionary, we have to place the inf codes on transitions outputs */
	move_outputs_on_transitions(root,INF_codes);
}
/* We build a minimal transducer from the entry tree */
minimize_tree(root,used_inf_values,compress_abstract_allocator);
int* inf_indirection=NULL;
int n_used_inf_codes=0;
if (INF_file!=NULL) {
	/* Now we reorder INF codes in order to group the ones that are actually
	 * used so that we can save space in the .inf file by not saving codes
	 * that are never referenced in the .bin file */
	inf_indirection=(int*)malloc(sizeof(int)*INF_codes->size);
	int last=INF_codes->size-1;
	/* This -1 initialization is used for safety checking */
	for (int i=0;i<INF_codes->size;i++) {
		inf_indirection[i]=-1;
	}
	for (int i=0;i<INF_codes->size && i<=last;i++) {
		if (get_value(used_inf_values,i)) {
			/* A used INF value stays at its place */
			n_used_inf_codes++;
			inf_indirection[i]=i;
		} else {
			/* We have found an unused INF code. We look for a used one at
			 * the end of the array to swap them */
			while (last>i && !get_value(used_inf_values,last)) {
				last--;
			}
			if (last==i) {
				/* We have finished */
				break;
			}
			n_used_inf_codes++;
			/* We redirect the old used INF code */
			inf_indirection[last]=i;
			/* And we swap codes */
			unichar* tmpInfValue=INF_codes->value[i];
			INF_codes->value[i]=INF_codes->value[last];
			INF_codes->value[last]=tmpInfValue;
			last--;
		}
	}
	int old_size=INF_codes->size;
	INF_codes->size=n_used_inf_codes;
	/* Now we can dump the INF codes into the .inf file */
	dump_values(INF_file,INF_codes);
	u_fclose(INF_file);
	write_INF_file_header(&vec,inf,INF_codes->size);
	INF_codes->size=old_size;
} /* End of dealing with the .inf file, if needed */

/* And we dump it into the .bin file */
int n_states;
int n_transitions;
int bin_size;
create_and_save_bin(root,bin,&n_states,&n_transitions,&bin_size,inf_indirection,new_style_bin,bin_type);
free(inf_indirection);
free_bit_array(used_inf_values);
u_printf("Binary file: %d bytes\n",bin_size);
if (bin_type==BIN_CLASSIC) {
	u_printf("%d line%s read            \n"
         "%d INF entr%s created\n",
         line,
         (line!=1)?"s":"",
         n_used_inf_codes,
         (n_used_inf_codes!=1)?"ies":"y");
} else {
	u_printf("%d line%s read            \n",
         line,
         (line!=1)?"s":"");
}
u_printf("%d states, %d transitions\n",n_states,n_transitions);
free_OptVars(vars);
/*
 * WARNING: we do not free the 'INF_codes' structure because of a slowness
 *          problem with very large INF lines.
 */

#if (defined(UNITEX_LIBRARY) || defined(UNITEX_RELEASE_MEMORY_AT_EXIT))
/* cleanup for no leak on library */
free_string_hash(INF_codes);
free_dictionary_node(root,compress_abstract_allocator);
close_abstract_allocator(compress_abstract_allocator);
close_abstract_allocator(compress_tokenize_abstract_allocator);
compress_abstract_allocator=NULL;
#endif
return 0;
}


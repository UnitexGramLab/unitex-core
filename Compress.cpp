 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "unicode.h"
#include "DELA.h"
#include "DictionaryTree.h"
#include "String_hash.h"
#include "AutomatonDictionary2Bin.h"
#include "FileName.h"
#include "Minimize_tree.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"


void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Compress <dictionary> [-flip]\n");
printf("   <dictionary> : any unicode DELAF or DELACF dictionary\n");
printf("   -flip : this optional parameter specifies that the inflected and lemma\n");
printf("           forms must be swapped\n\n");
printf("Compresses a dictionary into an finite state automaton. This automaton\n");
printf("is stored is a .bin file, and the associated flexional codes are\n");
printf("written in a .inf file.\n\n");
}


/**
 * This function writes the number of INF codes 'n' at the beginning
 * of the file named 'name'. This file is supposed to be a UTF-16
 * Little-Endian one, starting by a sequence made of ten zeros.
 */
void write_INF_file_header(char* name,int n) {
FILE* f;
/* First, we set the offset on the last zero */
int offset=2+9*2; /* *2 because of Unicode and +2 because of FF FE at file start */
/* The file must be opened as a binary one */
f=fopen((char*)name,"r+b");
do {
	/* We go at the given offset */
	fseek(f,offset,0);
	/* And we write the appropriate digit */
	u_fputc((unichar)((n%10)+'0'),f);
	n=n/10;
	/* And we update the offset (-2 because of Unicode) */
	offset=offset-2;
} while (n); /* We loop until there is no more digit to print */
fclose(f);
}


/**
 * This program reads a .dic file and compress it into a .bin and a .inf file.
 * First, it builds a tree with all the entries, and then, it builds a minimal
 * transducer from this tree, using the Dominique Revuz's algorithm.
 */
int main(int argc, char** argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();
if (argc!=2 && argc!=3) {
	/* We print the synopsis if the number arguments
	 * if not correct */
	usage();
	return 0;
}
FILE* f;
FILE* INF_file;
unichar s[DIC_WORD_SIZE];
struct dela_entry* entry;
char bin[DIC_WORD_SIZE];
char inf[DIC_WORD_SIZE];
struct dictionary_node* root; /* Root of the dictionary tree */
struct string_hash* INF_codes; /* Structure that will contain all the INF codes */
int line=0; /* Current line number */

f=u_fopen(argv[1],U_READ);
if (f==NULL) {
	fatal_error("Cannot open %s\n",argv[1]);
}
int FLIP=0;
if (argc==3) {
	/* If there is an extra parameter, we look if it is "-flip" */
	if (!strcmp(argv[2],"-flip")) {
		FLIP=1;
	} else {
		/* If not, we print a message and we raise a fatal error */
		u_fclose(f);
		fatal_error("Invalid parameter: %s\n",argv[2]);
	}
}
/* We compute the name of the output .bin and .inf files */
remove_extension(argv[1],bin);
strcat(bin,".bin");
remove_extension(argv[1],inf);
strcat(inf,".inf");
INF_file=u_fopen(inf,U_WRITE);
if (INF_file==NULL) {
	u_fclose(f);
	fatal_error("Cannot create %s\n",inf);
}
/* First, we print a sequence of zeros at the beginning of the .inf file
 * in order to book some place, so that we can later come and write there
 * the number of lines of this file. */
u_fprints_char("0000000000\n",INF_file);
root=new_dictionary_node();
INF_codes=new_string_hash();
unichar tmp[DIC_WORD_SIZE];
printf("Compressing...\n");
/* We read until there is no more lines in the .dic file */
while(u_read_line(f,s)) {
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
		if (FLIP) {
			/* If the "-flip" parameter has been used, we flip
			 * the inflected form and the lemma of the entry */
			unichar* o=entry->inflected;
			entry->inflected=entry->lemma;
			entry->lemma=o;
		}
		if (entry!=NULL) {
			/* If the entry is well-formed */
			if (contains_unprotected_equal_sign(entry->inflected)
				|| contains_unprotected_equal_sign(entry->lemma)) {
				/* If the inflected form or lemma contains any unprotected = sign,
				 * we must insert the space entry and the - entry:
				 * pomme=de=terre,.N  ->  pomme de terre,pomme de terre.N
				 *                        pomme-de-terre,pomme-de-terre.N
				 */
				unichar inf_tmp[DIC_WORD_SIZE];
				unichar lem_tmp[DIC_WORD_SIZE];
				u_strcpy(inf_tmp,entry->inflected);
				u_strcpy(lem_tmp,entry->lemma);
				/* We replace the unprotected = signs by spaces */
				replace_unprotected_equal_sign(entry->inflected,(unichar)' ');
				replace_unprotected_equal_sign(entry->lemma,(unichar)' ');
				/* And then we unprotect the other = signs */
				unprotect_equal_signs(entry->inflected);
				unprotect_equal_signs(entry->lemma);
				/* We insert "pomme de terre,pomme de terre.N" */
				get_compressed_line(entry,tmp);
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
				get_compressed_line(entry,tmp);
				add_entry_to_dictionary_tree(entry->inflected,tmp,root,INF_codes);
			}
			else {
				/* If the entry does not contain any unprotected = sign,
				 * we unprotect the = signs */
				unprotect_equal_signs(entry->inflected);
				unprotect_equal_signs(entry->lemma);
				get_compressed_line(entry,tmp);
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
		printf("%d line%s read...       \r",line,(line>1)?"s":"");
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
printf("Binary file: %d bytes\n",bin_size);
printf("%d line%s read            \n"
		"%d INF entr%s created\n",
		line,
		(line!=1)?"s":"",
		INF_codes->size,
		(INF_codes->size!=1)?"ies":"y");
printf("%d states, %d transitions\n",n_states,n_transitions);
write_INF_file_header(inf,INF_codes->size);
/*
 * WARNING: we do not free the 'INF_codes' structure because of a slowness
 *          problem with very large INF lines.
 */
return 0;
}


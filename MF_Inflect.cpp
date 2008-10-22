/*
  * Unitex 
  *
  * Copyright (C) 2001-2003 Universit<E9> Paris-Est Marne-la-Vall<E9>e <unitex@univ-mlv.fr>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  *
  */

/* Created by Agata Savary (savary@univ-tours.fr)
 * Last modification on September 12 2005
 */


#include <stdio.h>
#include <string.h>
#include "MF_LangMorpho.h"
#include "Unicode.h"
#include "MF_MU_graph.h"
#include "Alphabet.h"
#include "MF_DicoMorpho.h"
#include "MF_DLC_inflect.h"
#include "IOBuffer.h"
#include "File.h"
#include "Copyright.h"
#include "Error.h"


//Current language's alphabet
Alphabet* alph;

// Directory containing the inflection tranducers and the 'Morphology' file
extern char inflection_directory[FILENAME_MAX];



void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: MultiFlex <dela> <delaf> <alpha> <dir>\n");
u_printf("     <dela> : the unicode DELAS or DELAC file to be inflected\n");
u_printf("     <delaf> : the unicode resulting DELAF or DELACF dictionary \n");
u_printf("     <alpha> : the alphabet file \n");
u_printf("     <dir> : the directory containing 'Morphology' and 'Equivalences'\n");
u_printf("              files and inflection graphs for single and compound words.\n");
u_printf("\nInflects a DELAS or DELAC into a DELAF or DELACF. Note that you can merge\n");
u_printf("simple and compound words in a same dictionary.\n");
}


int main(int argc, char* argv[]) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

int err;  //0 if a function completes with no error 
if (argc!= 5) {
   usage();
   return 0;
}
//Load morphology description
char morphology[FILENAME_MAX];
new_file(argv[4],"Morphology.txt",morphology);
err=read_language_morpho(morphology);
if (err) {
   config_files_status=CONFIG_FILES_ERROR;
}
print_language_morpho();
//Load alphabet
alph=load_alphabet(argv[3]);  //To be done once at the beginning of the inflection
if (alph==NULL) {
   error("Cannot open alphabet file %s\n",argv[3]);
   free_language_morpho();
   free_alphabet(alph);    
   return 1;
}
//Init equivalence files
char equivalences[FILENAME_MAX];
new_file(argv[4],"Equivalences.txt",equivalences);
err=d_init_morpho_equiv(equivalences);
if (err) {
   config_files_status=CONFIG_FILES_ERROR;
}
d_init_class_equiv();
//Initialize the structure for inflection transducers
strcpy(inflection_directory,argv[4]);
err=MU_graph_init_graphs();
if (err) {
   MU_graph_free_graphs();
   return 1;
}
//DELAC inflection
err=inflect(argv[1],argv[2]);
MU_graph_free_graphs();
free_alphabet(alph);
free_language_morpho();
u_printf("Done.\n");
return 0;
}



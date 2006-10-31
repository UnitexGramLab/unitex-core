/*
  * Unitex 
  *
  * Copyright (C) 2001-2003 Universit<E9> de Marne-la-Vall<E9>e <unitex@univ-mlv.fr>
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
////////////////////////////////////////////////////////////////////
// TESTING MODULE


#include <stdio.h>
#include <string.h>
#include "MF_LangMorpho.h"
#include "unicode.h"
#include "MF_MU_graph.h"
#include "Alphabet.h"
#include "MF_DicoMorpho.h"
#include "MF_DLC_inflect.h"
#include "IOBuffer.h"

//Current language's alphabet
Alphabet* alph;

// Directory containing the inflection tranducers and the 'Morphology' file
extern char repertoire[1000];

void usage();

///////////////////////////////////
// Inflection of a DELAC to a DELACF
// argv[1] = path and name of the DELAC file
// argv[2] = path and name of the DELACF file
// argv[3] = path and name of the alphabet file
// argv[4] = path and name of the inflection directory 
//           containing the 'Morphology' file and the inflection transducers
//           for simple and compound words
int main(int argc, char* argv[]) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();
   
   
   
  int err;  //0 if a function completes with no error 
  
  if (argc != 5) {
    usage();
    return 1;
  }
  //Load morphology description
  err = read_language_morpho("Morphology", argv[4]);
  if (err) {
    free_language_morpho();
    return 1;
  }
  print_language_morpho();
  
  //Load alphabet
  char alphabet[1000];  //Path and name of the alphabet file
  strcpy(alphabet,argv[3]);
  alph = load_alphabet(alphabet);  //To be done once at the beginning of the DELAC inflection
  if (alph==NULL) {
    fprintf(stderr,"Cannot open alphabet file %s\n",alphabet);
    free_language_morpho();
    free_alphabet(alph);    
    return 1;
  }
  
  //Init equivalence files
  err = d_init_morpho_equiv("Equivalences", argv[4]);
  if (err) {
    free_language_morpho();
    free_alphabet(alph);    
    return 1;
  }
  d_print_morpho_equiv();

  d_init_class_equiv();
  
  //Initialize the structure for inflection transducers
  strcpy(repertoire,argv[4]);
  if (strlen(repertoire) && repertoire[strlen(repertoire)-1] != '/')
    strcat(repertoire,"/");
  err = MU_graph_init_graphs();
  if (err) {
    MU_graph_free_graphs();
    return 1;
  }
  
  //DELAC inflection
  err = DLC_inflect(argv[1],argv[2]);

  MU_graph_free_graphs();
  free_alphabet(alph);
  free_language_morpho();
  return 0;
}

void usage() {
printf("Usage: DlcInflect <delac> <delacf> <alpha> <dir>\n");
printf("     <delac> : the unicode DELAC file to be inflected\n");
printf("     <delacf> : the unicode resulting DELACF dictionary \n");
printf("     <alpha> : the alphabet file \n");
printf("     <dir> : the directory containing 'Morphology' file and \n");
printf("             inflection graphs for single and compound words.\n");
printf("\nInflects a DELAC into a DELACF.\n");
}


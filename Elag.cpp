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

#ifdef __GNUC__  // gcc 
#include <unistd.h>

#elif  defined(__VISUALC__)

//  #ifdef __VISUALC__  // visual studio

#include <DIRECT.H>

#else    // Borland

#include <dir.h>
#endif


#include "Copyright.h"
#include "const.h"
#include "autalmot.h"
#include "list_aut.h"
#include "elag-functions.h"
#include "utils.h"
#include "IOBuffer.h"


void usage() {
  printf("%s", COPYRIGHT);
  printf("Usage: Elag <txtauto> -l <LANG> -g <rules> -o <output> [-d <dir>]\n"
         "\n"
         "whith:\n"
         "<txtauto>       :    input text automaton FST2 file,\n"
         "<LANG>          :    the current language definition file,\n"
         "<rules>         :    compiled elag rules file,\n"
         "<output>        :    the resulting output FST2 file,\n"
         "<dir>           :    (optional) directory where elag rules are located.\n"
         "\n"
         "Disambiguate the input text automaton <txtauto> using the specified compiled elag rules.\n"
         "The resulting automaton is stored in <output>.\n\n");
}


int main(int argc, char ** argv) {

  setBufferMode();

  char * progname   = *argv;
  char * txtauto    = NULL;
  char * grammardir = NULL;
  char * grammars   = NULL;
  char * output     = NULL;

  char * langname   = NULL;

  argv++, argc--;

  if (argc == 0) { usage(); return 0; }
  
  while (argc) {

    if (**argv != '-') { // text automaton
      
      txtauto = *argv;

    } else {

      if (strcmp(*argv, "-o") == 0) { // output
	
	argv++, argc--;
	if (argc == 0) { fatal_error("-o needs an arg\n"); }

	output = *argv;

      } else if (strcmp(*argv, "-d") == 0) { // grammars directory

	argv++, argc--;
	if (argc == 0) { fatal_error("-d needs an arg\n"); }

	grammardir = *argv;

      } else if (strcmp(*argv, "-l") == 0) { // file of compiled grammar names

	argv++, argc--;
	if (argc == 0) { fatal_error("-l needs an arg\n"); }
	langname = *argv;

      } else if (strcmp(*argv, "-g") == 0) { // 1 grammar already compiled

	argv++, argc--;
	if (argc == 0) { fatal_error("-g needs an arg\n"); }

	grammars = *argv;

      } else if (strcmp(*argv, "-h") == 0) {

	printf("\nusage: %s <txtauto> -l <LANG> -d <gramdir> [ -g <gramlist> ] -o <output>\n\n",
               progname);
	return 0;

      }	else { fatal_error("unknow arg: '%s'\n", *argv); }

    }

    argv++, argc--;
  }	


  if (! langname) { fatal_error("no LANGUAGE specified\n"); }
  if (txtauto == NULL) { fatal_error("no text automaton specified\n"); }
  if (! grammars) { fatal_error("-g option should be used\n"); }


  printf("loading %s langage definition ...\n", langname);

  language_t * lang = language_load(langname);
  set_current_language(lang);


  char buf[MAX_PATH];

  if (output == NULL) {

    strcpy(buf, txtauto);
    int len = strlen(buf);

    if (strcmp(buf + len - 5, ".fst2") == 0) {
      len = len - 5;
    }
    strcpy(buf + len, "-elag.fst2");
    output = buf;
  }


  if (grammardir == NULL) {
    grammardir = dirname(strdup(grammars));
    grammars   = basename(grammars);
  }

  debug("changing to %s directory\n", grammardir);

  if (chdir(grammardir) == -1) { error("unable to change to %s directory.\n", grammardir); }

  list_aut * gramm;
  if ((gramm = chargeGramm(grammars)) == NULL) { fatal_error("unable to load grammar %s", grammars); }

  printf("Grammars are loaded.\n") ;

  
  leve_ambiguite(txtauto, gramm, output);

  return 0;
}

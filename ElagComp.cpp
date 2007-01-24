 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifdef __GNUC__  // gcc (i.e. UNIX)

#include <unistd.h>

#elif defined(__VISUALC__)

#include <DIRECT.H>

#else    // Borldand

#include <dir.h>

#endif

#include "Copyright.h"
#include "language.h"
#include "autalmot.h"
#include "compgr.h"
#include "utils.h"
#include "IOBuffer.h"



void usage() {
  printf("%s", COPYRIGHT);
  printf("Usage: ElagComp [ -r <ruleslist> | -g <grammar> ] -l <LANG> [ -o <output> ] [ -d <rulesdir> ]\n"
         "\n"
         "with :\n"
         "<ruleslist>     :     Elag grammars list file,\n"
         "<LANG>          :     Elag language description file,\n"
         "<output>        :     (optional) file where the resulting compiled grammar is stored\n"
         "                      the default name is same as <rulelist> except for the .rul extension,\n"
         "<rulesdir>      :     (optional) directory where Elag grammars are located.\n"
         "\n"
         "ElagComp compile one Elag grammar specified by <grammar> or all the grammars specified in the <ruleslist> file.\n"
         "The result is stored into the file <output> for later use by the Elag text disambiguation program.\n\n");
}


static inline void strip_extension(char * s) {

  char * p = s + strlen(s) - 1;
  
  while (p > s) {
    if (*p == '.') { *p = 0; break; }
    if (*p == '/' || *p == '\\') { break; }
    p--;
  }
}

int main(int argc, char ** argv ) {

  setBufferMode();

  static char buf[1024];

  char * compilename = NULL;
  char * ruledir     = NULL;
  char * rules       = NULL;
  char * langname    = NULL;
  char * grammar     = NULL;

  if (argc == 1) { usage(); return 0; }

  argv++, argc--;


  while (argc) {

    if (**argv != '-') {

      rules = *argv;

    } else {

      if (strcmp(*argv, "-o") == 0) { // nom du fichier des grammaires compilee

	argv++, argc--;
	if (argc == 0) { fatal_error("-o argument needs a parameter\n"); }
      
	compilename = *argv;

      } else if (strcmp(*argv, "-d") == 0) { // rules directory

	argv++, argc--;
	if (argc == 0) { fatal_error("-d argument needs a parameter\n"); }

	ruledir = *argv;

      } else if (strcmp(*argv, "-r") == 0) { // rules file

	argv++, argc--;
	if (argc == 0) { fatal_error("-r argument needs a parameter\n"); }

	rules = *argv;

      } else if (strcmp(*argv, "-g") == 0) { // rules file

	argv++, argc--;
	if (argc == 0) { fatal_error("-g argument needs a parameter\n"); }

	grammar = *argv;

      } else if (strcmp(*argv, "-h") == 0) {

	usage();
	return 0;

      } else if (strcmp(*argv, "-l") == 0) {

	argv++, argc--;
	if (argc == 0) { fatal_error("-l argument needs a parameter\n"); }

	langname = *argv;

      } else {

	fatal_error("unknow argument: '%s'\n", *argv);
      }
    }

    argv++, argc--;
  }


  if (! langname) { fatal_error("No language specified\n"); }

  language_t * lang = language_load(langname);

  set_current_language(lang);


  if (! rules && ! grammar) { fatal_error("you must specified a grammar or a rules file name\n"); }

  if (rules && grammar) { fatal_error("cannot handle list file and grammar in the same time.\n"); }

  if (rules) {
  
    if (ruledir == NULL) {
      ruledir = dirname(strdup(rules));
      rules   = basename(rules);
    }

    debug("changing to %s directory\n", ruledir);
    if (chdir(ruledir) == -1) { error("unable to change to %s directory.\n", ruledir); }

    if (compilename == NULL) {

      int l = strlen(rules);

      if (strcmp(rules + l - 4, ".lst") == 0) {
        strcpy(buf, rules);
        strcpy(buf + l - 4, ".rul");
      } else {
        sprintf(buf, "%s.rul", rules);
      }
      compilename = buf;
    }


    if (compile_rules(rules, compilename) == -1) {
      error("An error occured\n");
      return 1;
    }

    printf("\nElag grammars are compiled in %s.\n", compilename);
  
  } else { // compile one grammar
  
   printf("Compiling %s ...\n", grammar);
 
   int l = strlen(grammar);
   
   if (strcmp(grammar + l - 5, ".fst2") != 0) {
     fatal_error("grammar '%s' should be a FST2 file\n");
   }

   strcpy(buf, grammar); 
   strip_extension(buf);
   strcat(buf, ".elg");

   if (compile_grammar(grammar, buf) == -1) {
     error("An error occured while compiling %s\n", grammar);
     return 1;
   }

   printf("Elag grammar is compiled into %s.\n", buf);
  }

  fflush(stdout);

  return 0 ;
}


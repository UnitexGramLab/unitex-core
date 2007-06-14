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

#include "Unicode.h"
#include "Copyright.h"
#include "LanguageDefinition.h"
#include "autalmot.h"
#include "compgr.h"
#include "utils.h"
#include "File.h"
#include "IOBuffer.h"



void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: ElagComp [-r <rlist>|-g <grammar>] -l <lang> [-o <output>] [-d <rdir>]\n"
         "\n"
         "<rlist>   : Elag .fst2 grammar list file\n"
         "<grammar> : Elag .fst2 grammar\n"
         "<lang>    : Elag language description file\n"
         "<output>  : (optional) file where the resulting compiled grammar is stored.\n"
         "            The default name is same as <rlist> except for the .rul extension\n"
         "<rdir>    : (optional) directory where Elag grammars are located\n"
         "\n"
         "ElagComp compiles one Elag grammar specified by <grammar> or all the grammars\n"
         "specified in the <rlist> file. The result is stored into the file <output>\n"
         "for later use by the Elag text disambiguation program.\n");
}


static inline void strip_extension(char * s) {

  char * p = s + strlen(s) - 1;
  
  while (p > s) {
    if (*p == '.') { *p = 0; break; }
    if (*p == '/' || *p == '\\') { break; }
    p--;
  }
}



int main(int argc,char** argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

char buf[FILENAME_MAX];
char* compilename=NULL;
char* ruledir=NULL;
char* rules=NULL;
char* langname=NULL;
char* grammar=NULL;
if (argc==1) {
   usage();
   return 0;
}
argv++;
argc--;
while (argc!=0) {
   if (*argv[0]!='-') {
      rules=*argv;
   } else {
      if (!strcmp(*argv,"-o")) {
         argv++;
         argc--;
         if (argc==0) {
            fatal_error("-o argument needs a parameter\n");
         }
         compilename=*argv;
      } else if (!strcmp(*argv,"-d")) {
         argv++;
         argc--;
         if (argc==0) {
            fatal_error("-d argument needs a parameter\n");
         }
         ruledir=*argv;
      } else if (!strcmp(*argv,"-r")) {
         argv++;
         argc--;
         if (argc==0) {
            fatal_error("-r argument needs a parameter\n");
         }
         rules=*argv;
      } else if (!strcmp(*argv,"-g")) {
         argv++;
         argc--;
         if (argc==0) {
            fatal_error("-g argument needs a parameter\n");
         }
         grammar=*argv;
      } else if (!strcmp(*argv,"-h")) {
         usage();
         return 0;
      } else if (!strcmp(*argv,"-l")) {
         argv++;
         argc--;
         if (argc==0) {
            fatal_error("-l argument needs a parameter\n");
         }
         langname=*argv;
      } else {
         fatal_error("Unknown argument: '%s'\n",*argv);
      }
   }
   argv++;
   argc--;
}
if (langname==NULL) {
   fatal_error("No language definition file specified\n");
}
language_t* language=load_language_definition(langname);
set_current_language(language);
if (rules==NULL && grammar==NULL) {
   fatal_error("You must specified a grammar or a rule file name\n");
}
if (rules!=NULL && grammar!=NULL) {
   fatal_error("Cannot handle both a rule file and a grammar\n");
}
if (rules!=NULL) {
   /* If we work with a rule list */
   if (ruledir==NULL) {
      ruledir = dirname(strdup(rules));
      rules   = basename(rules);
    }

    error("changing to %s directory\n", ruledir);
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
      error("An error occurred\n");
      return 1;
    }

    u_printf("\nElag grammars are compiled in %s.\n", compilename);
  
} else {
   /* If we must compile a single grammar */
   u_printf("Compiling %s...\n",grammar);
   char elg_file[FILENAME_MAX];
   get_extension(grammar,elg_file);
   if (strcmp(elg_file,".fst2")) {
     fatal_error("Grammar '%s' should be a .fst2 file\n");
   }
   remove_extension(grammar,elg_file);
   strcat(elg_file,".elg");
   if (compile_elag_grammar(grammar,elg_file)==-1) {
     error("An error occured while compiling %s\n",grammar);
     return 1;
   }
   u_printf("Elag grammar is compiled into %s.\n",elg_file);
}
return 0;
}


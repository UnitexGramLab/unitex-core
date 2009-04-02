 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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


#include "Copyright.h"
#include "Error.h"
#include "File.h"
#include "Fst2TxtAsRoutine.h"
#include "LocateConstants.h"
#include "getopt.h"


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Fst2Txt [OPTIONS] <fst2>\n"
         "\n"
         "  <fst2>: the grammar to be applied to the text\n"
         "\n"
         "OPTIONS:\n"
         "  -t TXT/--text=TXT: the unicode text file to be parsed\n"
         "  -a ALPH/--alphabet=ALPH: the alphabet file\n"
         "  -s/--start_on_space: enables morphological use of space\n"
         "  -x/--dont_start_on_space: disables morphological use of space (default)\n"
         "  -c/--char_by_char: uses char by char tokenization; useful for languages like Thai\n"
         "  -w/--word_by_word: uses word by word tokenization (default)\n"
         "\n"
         "Output options:\n"
         "  -M/--merge (default)\n"
         "  -R/--replace\n"
         "\n"
         "  -h/--help: this help\n"
         "\n"
         "Applies a grammar to a text. The text file is modified.\n");
}



int main_Fst2Txt(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":t:a:MRcwsxh";
const struct option lopts[]= {
      {"text",required_argument,NULL,'t'},
      {"alphabet",required_argument,NULL,'a'},
      {"merge",no_argument,NULL,'M'},
      {"replace",no_argument,NULL,'R'},
      {"char_by_char",no_argument,NULL,'c'},
      {"word_by_word",no_argument,NULL,'w'},
      {"start_on_space",no_argument,NULL,'s'},
      {"dont_start_on_space",no_argument,NULL,'x'},      
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
struct fst2txt_parameters* p=new_fst2txt_parameters();
int val,index=-1;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 't': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty text file name\n");
             }
             p->text_file=strdup(optarg);
             break;
   case 'a': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file name\n");
             }
             p->alphabet_file=strdup(optarg);
             break;
   case 'M': p->output_policy=MERGE_OUTPUTS; break;
   case 'R': p->output_policy=REPLACE_OUTPUTS; break;
   case 'c': p->tokenization_policy=CHAR_BY_CHAR_TOKENIZATION; break;
   case 'w': p->tokenization_policy=WORD_BY_WORD_TOKENIZATION; break;
   case 's': p->space_policy=START_WITH_SPACE; break;
   case 'x': p->space_policy=DONT_START_WITH_SPACE; break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",optopt); 
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",optopt); 
             else fatal_error("Invalid option --%s\n",optarg);
             break;
   }
   index=-1;
}

if (optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

if (p->text_file==NULL) {
   fatal_error("You must specify the text file\n");
}

if (p->alphabet_file==NULL) {
   fatal_error("You must specify the alphabet file\n");
}

char tmp[FILENAME_MAX];
remove_extension(p->text_file,tmp);
strcat(tmp,".tmp");
p->temp_file=strdup(tmp);
p->fst_file=strdup(argv[optind]);

int result=main_fst2txt(p);
free_fst2txt_parameters(p);
return result;
}

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

#include <math.h>
#include <limits.h>
#include "Unicode.h"
#include "Copyright.h"
#include "Tfst.h"
#include "String_hash.h"
#include "Error.h"
#include "SingleGraph.h"
#include "UnitexGetOpt.h"
#include "Evamb.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Evamb =
         "Usage: Evamb [OPTIONS] <tfst>\n"
         "\n"
         "  <tfst>: text automaton file\n"
         "\n"
         "OPTIONS:\n"
         "  -o OUT/--output=OUT: optional output file\n"
         "  -s N/--sentence=N: sentence number\n"
         "  -h/--help: this help\n"
         "\n"
         "Prints the average lexical ambiguity rate of the whole text automaton, or of the\n"
         "sentence specified by <#sentence>. This value represents the average number of\n"
         "hypothesis per word of the sentence. Note that the result won't be the same\n"
         "whether the automaton tags are imploded or not.\n"
         "The text automaton is not modified.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Evamb);
}


const char* optstring_Evamb=":s:ho:k:q:";
const struct option_TS lopts_Evamb[]= {
      {"sentence",required_argument_TS,NULL,'s'},
      {"output",optional_argument_TS,NULL,'o'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_Evamb(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

int val,index=-1;
int sentence_number=-1;
const char* outfilename=NULL;
char output_name_buffer[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
UnitexGetOpt options;

while (EOF!=(val=options.parse_long(argc,argv,optstring_Evamb,lopts_Evamb,&index))) {
   switch(val) {
   case 's':   {  char c_foo;
                  if (1!=sscanf(options.vars()->optarg,"%d%c",&sentence_number,&c_foo) || sentence_number<=0) {
                    /* foo is used to check that the sentence number is not like "45gjh" */
                    error("Invalid sentence number: %s\n",options.vars()->optarg);
                    return USAGE_ERROR_CODE;
                  }
                }
                break;
      case 'o': if (options.vars()->optarg[0]=='\0') {
                   error("You must specify a non empty output file name\n");
                   return USAGE_ERROR_CODE;
                }
                strcpy(output_name_buffer,options.vars()->optarg);
                outfilename=output_name_buffer;
                break;
      case 'k': if (options.vars()->optarg[0]=='\0') {
                  error("Empty input_encoding argument\n");
                  return USAGE_ERROR_CODE;
                }
                decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
                break;
      case 'q': if (options.vars()->optarg[0]=='\0') {
                  error("Empty output_encoding argument\n");
                  return USAGE_ERROR_CODE;
                }
                decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
                break;
   case 'h': usage(); 
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Evamb[index].name);
             return USAGE_ERROR_CODE;                         
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

if (options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

u_printf("Loading '%s'...\n",argv[options.vars()->optind]);
Tfst* tfst=open_text_automaton(&vec,argv[options.vars()->optind]);
if (tfst==NULL) {
   error("Unable to load '%s'\n",argv[options.vars()->optind]);
   return DEFAULT_ERROR_CODE;
}
if (sentence_number>tfst->N) {
   error("Invalid sentence number %d: should be in [1;%d]\n",sentence_number,tfst->N);
   close_text_automaton(tfst);
   return DEFAULT_ERROR_CODE;
}
U_FILE* outfile = (outfilename == NULL) ? U_STDOUT : u_fopen(&vec,outfilename,U_WRITE);
if (outfile==NULL) {
    error("Cannot create file %s\n",outfilename);
    close_text_automaton(tfst);
    return DEFAULT_ERROR_CODE;
}
if (sentence_number==-1) {
   /* If we have to evaluate the ambiguity rate of the whole automaton */
   double lognp_total=0.0;
   double lmoy_total=0.0;
   double maxlogamb=0.0;
   double minlogamb=(double)INT_MAX;
   /* This is the number of bad automata in the text .fst2 */
   int n_bad_automata=0;
   int maxambno=-1;
   int minambno=-1;
   for (sentence_number=1;sentence_number<=tfst->N;sentence_number++) {
      load_sentence(tfst,sentence_number);
      SingleGraph graph=tfst->automaton;
      if (graph->number_of_states==0 || graph->states[0]->outgoing_transitions==NULL) {
         n_bad_automata++;
         error("Sentence %d: empty automaton\n",sentence_number);
      } else {
         /* log(number of paths) */
         double lognp;
         /* minimum/maximum path length */
         int lmin,lmax;
         /* Approximation of the sentence length */
         double lmoy;
         /* log(ambiguity rate) */
         double logamb;
         lognp=evaluate_ambiguity(graph,&lmin,&lmax);
         lmoy=(double)(lmin+lmax)/2.0;
         logamb=lognp/lmoy;
         if (maxlogamb<logamb) {
            maxlogamb=logamb;
            maxambno=sentence_number;
         }
         if (minlogamb>logamb) {
            minlogamb=logamb;
            minambno=sentence_number;
         }
         u_printf("Sentence %d            \r",sentence_number);
         lognp_total=lognp_total+lognp;
         lmoy_total=lmoy_total+lmoy;
      }
   }
   if (n_bad_automata>=tfst->N) {
      error("No stats to print because no non-empty sentence automata were found.\n");
   } else {
      u_fprintf(outfile,"%d/%d sentence%s taken into account\n",tfst->N-n_bad_automata,tfst->N,(tfst->N>1)?"s":"");
      u_fprintf(outfile,"Average ambiguity rate=%.3f\n",exp(lognp_total/lmoy_total));
      u_fprintf(outfile,"Minimum ambiguity rate=%.3f (sentence %d)\n",exp(minlogamb),minambno);
      u_fprintf(outfile,"Maximum ambiguity rate=%.3f (sentence %d)\n",exp(maxlogamb),maxambno);
   }
} else {
   /* If we have to evaluate the ambiguity rate of a single sentence automaton */
   load_sentence(tfst,sentence_number);
   SingleGraph graph=tfst->automaton;
   if (graph->number_of_states==0) {
      error("Sentence %d: empty automaton\n",sentence_number);
   } else {
      int min;
      int max;
      double lognp=evaluate_ambiguity(graph,&min,&max);
      double lmoy=(double)(min+max)/2.0;
      u_fprintf(outfile,"Sentence %d: ambiguity rate=%.3f\n",sentence_number,exp(lognp/lmoy));
   }
}

if (outfile!=U_STDOUT) {
  u_fclose(outfile);
}

close_text_automaton(tfst);
return SUCCESS_RETURN_CODE;
}

} // namespace unitex

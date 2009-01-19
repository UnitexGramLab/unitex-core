 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Fst2.h"
#include "String_hash.h"
#include "IOBuffer.h"
#include "Error.h"
#include "SingleGraph.h"
#include "getopt.h"


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Evamb [OPTIONS] <txtauto>\n"
         "\n"
         "  <txtauto>: text automaton FST2 file\n"
         "\n"
         "OPTIONS:\n"
         "  -s N/--sentence=N: sentence number\n"
         "  -h/--help: this help\n"
         "\n"
         "Prints the average lexical ambiguity rate of the whole text automaton, or of the\n"
         "sentence specified by <#sentence>. This value represents the average number of\n"
         "hypothesis per word of the sentence. Note that the result won't be the same\n"
         "whether the automaton tags are imploded or not.\n"
         "The text automaton is not modified.\n");
}


int main(int argc,char* argv[]) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":s:h";
const struct option lopts[]= {
      {"sentence",required_argument,NULL,'s'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int val,index=-1;
int sentence_number=-1;
char foo;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 's': if (1!=sscanf(optarg,"%d%c",&sentence_number,&foo) || sentence_number<=0) {
                /* foo is used to check that the sentence number is not like "45gjh" */
                fatal_error("Invalid sentence number: %s\n",optarg);
             }
             break;
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

u_printf("Loading '%s'...\n",argv[optind]);
Fst2* fst2=load_fst2(argv[optind],0);
if (fst2==NULL) {
   fatal_error("Unable to load '%s' fst2\n",argv[optind]);
}
if (sentence_number>fst2->number_of_graphs) {
   fatal_error("Invalid sentence number %d: should be in [1;%d]\n",sentence_number,fst2->number_of_graphs);
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
   for (sentence_number=1;sentence_number<=fst2->number_of_graphs;sentence_number++) {
      SingleGraph graph=get_subgraph(fst2,sentence_number);
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
      free_SingleGraph(graph);
   }
   if (n_bad_automata>=fst2->number_of_graphs) {
      error("No stats to print because no non-empty sentence automata were found.\n");
   } else {
      u_printf("%d/%d sentence%s taken into account\n",fst2->number_of_graphs-n_bad_automata,fst2->number_of_graphs,(fst2->number_of_graphs>1)?"s":"");
      u_printf("Average ambiguity rate=%.3f\n",exp(lognp_total/lmoy_total));
      u_printf("Minimum ambiguity rate=%.3f (sentence %d)\n",exp(minlogamb),minambno);
      u_printf("Maximum ambiguity rate=%.3f (sentence %d)\n",exp(maxlogamb),maxambno);
   }
} else {
   /* If we have to evaluate the ambiguity rate of a single sentence automaton */
   SingleGraph graph=get_subgraph(fst2,sentence_number);
   if (graph->number_of_states==0) {
      error("Sentence %d: empty automaton\n",sentence_number);
   } else {
      int min;
      int max;
      double lognp=evaluate_ambiguity(graph,&min,&max);
      double lmoy=(double)(min+max)/2.0;
      u_printf("Sentence %d: ambiguity rate=%.3f\n",sentence_number,exp(lognp/lmoy));
   }
   free_SingleGraph(graph);
}
free_Fst2(fst2);
return 0;
}


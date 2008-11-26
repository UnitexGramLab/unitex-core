 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Unicode.h"
#include "String_hash.h"
#include "Fst2.h"
#include "File.h"
#include "Copyright.h"
#include "grf.h"
#include "IOBuffer.h"
#include "Error.h"
#include "Transitions.h"
#include "getopt.h"

// also defined in grf.cpp
static unichar epsilon[] = { '<', 'E', '>', 0 };


#ifndef MAX_PATH
#define MAX_PATH 2000
#endif

void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: MergeTextAutomaton <txtauto>\n"
         "\n"
         "  <txtauto>: text automaton to be rebuilt\n"
         "\n"
         "OPTIONS:\n"
         "  -h/--help: this help\n"
         "\n"
         "Rebuilds the text automaton taking into account the sentence graphs that have\n"
         "been manually modified. The text automaton is modified.\n");
}


void fst_header(Fst2 * A, FILE * f) {

  char buf[11];
  strcpy(buf, "0000000000");

  int i = 9;
  int n = A->number_of_graphs;

  while (n) {
    buf[i--] = (char)('0' + (n % 10));
    n = n / 10;
  }

  u_fprintf(f, "%s\n", buf);
}



void output_fst(Fst2 * A, int no, string_hash * hash, FILE * f) {

  int stateno = A->initial_states[no];

  for (int i = 0; i < A->number_of_states_per_graphs[no]; i++) {

    Fst2State state = A->states[stateno + i];

    if (is_final_state(state)) {
       u_fprintf(f,"t\n");
    } else {
      u_fprintf(f,": ");
      for (Transition * trans = state->transitions; trans; trans = trans->next) {
         u_fprintf(f, "%d %d ", get_value_index(A->tags[trans->tag_number]->input, hash), trans->state_number - stateno);
      }
      u_fputc('\n', f);
    }
  }
  u_fprintf(f,"f \n");
}



void output_grf_trans(grf_t * grf, int i, int * ids, string_hash * labels, FILE * f) {

  u_fputc(grf->boxes[i]->terminal ? 't' : ':', f); u_fputc(' ', f);

  for (grf_trans * trans = grf->boxes[i]->trans; trans; trans = trans->next) {

    if (u_strlen(grf->boxes[trans->to]->label)) {

      int l = get_value_index(grf->boxes[trans->to]->label, labels);
      u_fprintf(f, "%d %d ", l, ids[trans->to]);
    }

  }

  u_fputc('\n', f);
}



void output_grf(grf_t * grf, string_hash * labels, FILE * f) {


  bool EPSILON_IN_INIT = (u_strcmp(grf->boxes[GRF_BOX_INIT]->label, epsilon) == 0);



  /* compute boxes which will be equivalent in the corresponding minimal automaton */

  int* ids=(int*)malloc(sizeof(int)*grf->nb_boxes);

  grf_calc_ids(grf, ids);


  /* reorder the box by their ids */

  grf_reorder_by_tab(grf, ids);


  /* displace first (init) box at the end (if it isn't labeled by <E>) */

  if (! EPSILON_IN_INIT) {

    u_fprintf(f, ": %d %d \n", get_value_index(grf->boxes[GRF_BOX_INIT]->label, labels), grf->nb_boxes);

  } else { output_grf_trans(grf, GRF_BOX_INIT, ids, labels, f); }



  /* final box */

  u_fprintf(f,"t\n");


  int i = 2;

  while (i < grf->nb_boxes) {

    output_grf_trans(grf, i, ids, labels, f);

    int id = ids[i++];

    while ((i < grf->nb_boxes) && (id == ids[i])) { i++; }
  }

  if (! EPSILON_IN_INIT) { output_grf_trans(grf, GRF_BOX_INIT, ids, labels, f); }

  u_fprintf(f,"f \n");
  free(ids);
}



void output_labels(string_hash * hash, FILE * f) {
  for (int i = 0; i < hash->size; i++) { u_fprintf(f, "%%%S\n", hash->value[i]); }
  u_fprintf(f,"f\n");
}




int clean_grf(grf_t * grf) {

  if (! grf_is_acyclic(grf)) {
    error("grf isn't an acyclic graph.\n");
    return -1;
  }

  if (grf_check_labels(grf) == -1) {
    error("bad label in grf.\n");
    return -1;
  }

  grf_remove_epsilons(grf);
  grf_cleanup(grf);

  if (grf->nb_boxes == 0) {
    error("error: final state isn't accessible.\n");
    return -1;
  }

  return 0;
}



int main(int argc, char ** argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();  

if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":h";
const struct option lopts[]= {
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int val,index=-1;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
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

u_printf("Loading %s...\n",argv[optind]);
Fst2* A = load_fst2(argv[optind], 1);
if (A==NULL) {
   fatal_error("Unable to load %s automaton\n",argv[optind]);
}
char basedir[FILENAME_MAX];
get_path(argv[optind],basedir);
char outname[FILENAME_MAX];
sprintf(outname,"%s.new",argv[optind]);
FILE* out;
if ((out = u_fopen(outname, U_WRITE)) == NULL) {
   fatal_error("Unable to open %s for writting\n",outname);
}
string_hash * labels = new_string_hash();
get_value_index(epsilon, labels);
fst_header(A, out);
for (int i = 1; i <= A->number_of_graphs; i++) {
   if ((i % 100) == 0) {
      u_printf("%d/%d sentences rebuilt...\n", i, A->number_of_graphs);
   }
   char grfname[MAX_PATH];
   sprintf(grfname, "%ssentence%d.grf", basedir, i);
   grf_t * grf;
   FILE * f;
   grf = NULL;
   if ((f = u_fopen(grfname, U_READ)) != NULL) {
      if ((grf = grf_load(f)) == NULL) {
         error("Unable to load '%s'.\n", grfname);
      } else if (clean_grf(grf) == -1) {
         error("%s is a bad grf file.\n", grfname);
         grf_delete(grf);
         grf = NULL;
      }
      u_fclose(f);
   }
   u_fprintf(out, "-%d %S\n", i, A->graph_names[i]);
   if (grf) {
      output_grf(grf, labels, out);
      grf_delete(grf);
   } else {
      output_fst(A, i, labels, out);
   }
}
output_labels(labels,out);
u_printf("Text automaton rebuilt.\n");
u_fclose(out);

/* make a backup and replace old automaton with new */
sprintf(outname, "%s.bck",argv[optind]);
remove(outname);
rename(argv[optind], outname);
sprintf(outname, "%s.new",argv[optind]);
rename(outname,argv[optind]);
u_printf("\nYou can find a backup of the original \"%s\" file in \"%s.bck\".\n",argv[optind],argv[optind]);
return 0;
}



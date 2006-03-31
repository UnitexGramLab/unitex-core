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

#include "unicode.h"
#include "String_hash.h"
#include "Fst2.h"
#include "FileName.h"
#include "grf.h"



#ifndef MAX_PATH
#define MAX_PATH 2000
#endif


#define STATE_TERMINAL   (1)




void fst_header(Automate_fst2 * A, FILE * f) {

  char buf[11];
  strcpy(buf, "0000000000");

  int i = 9;
  int n = A->nombre_graphes;

  while (n) {
    buf[i--] = '0' + (n % 10);
    n = n / 10;
  }

  u_fprintf(f, "%s\n", buf);
}



void output_fst(Automate_fst2 * A, int no, string_hash * hash, FILE * f) {

  int stateno = A->debut_graphe_fst2[no];

  for (int i = 0; i < A->nombre_etats_par_grf[no]; i++) {

    etat_fst * state = A->etat[stateno + i];

    if (state->controle & STATE_TERMINAL) {

      u_fprints_char("t\n", f);

    } else {

      u_fputc(':', f); u_fputc(' ', f);

      for (transition_fst * trans = state->trans; trans; trans = trans->suivant) {
	u_fprintf(f, "%d %d ", get_hash_number(A->etiquette[trans->etiquette]->contenu, hash), trans->arr - stateno);
      }

      u_fputc('\n', f);
    }
  }

  u_fprints_char("f \n", f);
}



void output_grf_trans(grf_t * grf, int i, int * ids, string_hash * labels, FILE * f) {

  u_fputc(grf->boxes[i]->terminal ? 't' : ':', f); u_fputc(' ', f);

  for (grf_trans * trans = grf->boxes[i]->trans; trans; trans = trans->next) {

    if (u_strlen(grf->boxes[trans->to]->label)) {

      int l = get_hash_number(grf->boxes[trans->to]->label, labels);
      u_fprintf(f, "%d %d ", l, ids[trans->to]);
    }

  }

  u_fputc('\n', f);
}



void output_grf(grf_t * grf, string_hash * labels, FILE * f) {


  bool EPSILON_IN_INIT = (u_strcmp(grf->boxes[GRF_BOX_INIT]->label, epsilon) == 0);


  /* compute boxes which will be equivalent in the corresponding minimal automaton */

  int ids[grf->nb_boxes];

  grf_calc_ids(grf, ids);


  /* reorder the box by their ids */

  grf_reorder_by_tab(grf, ids);


  /* displace first (init) box at the end (if it isn't labeled by <E>) */

  if (! EPSILON_IN_INIT) {

    u_fprintf(f, ": %d %d \n", get_hash_number(grf->boxes[GRF_BOX_INIT]->label, labels), grf->nb_boxes);

  } else { output_grf_trans(grf, GRF_BOX_INIT, ids, labels, f); }



  /* final box */

  u_fprints_char("t\n", f);


  int i = 2;

  while (i < grf->nb_boxes) {

    output_grf_trans(grf, i, ids, labels, f);

    int id = ids[i++];

    while ((i < grf->nb_boxes) && (id == ids[i])) { i++; }
  }

  if (! EPSILON_IN_INIT) { output_grf_trans(grf, GRF_BOX_INIT, ids, labels, f); }

  u_fprints_char("f \n", f);
}



void output_labels(string_hash * hash, FILE * f) {
  for (int i = 0; i < hash->N; i++) { u_fprintf(f, "%%%S\n", hash->tab[i]); }
  u_fprints_char("f\n", f);
}




int clean_grf(grf_t * grf) {

  if (! grf_is_acyclic(grf)) {
    fprintf(stdout, "grf isn't an acyclic graph.\n");
    return -1;
  }

  if (grf_check_labels(grf) == -1) {
    fprintf(stderr, "bad label in grf.\n");
    return -1;
  }

  grf_remove_epsilons(grf);
  grf_cleanup(grf);

  if (grf->nb_boxes == 0) {
    fprintf(stderr, "error: final state isn't accessible.\n");
    return -1;
  }

  return 0;
}



int main(int argc, char ** argv) {


  if (argc != 2) {
    fprintf(stderr, "usage: %s <automaton>\n", *argv);
    exit(1);
  }
  argv++;


  printf("loading %s ... \n%s", *argv, CR);

  Automate_fst2 * A = load_fst2(*argv, 1);
  
  if (A == NULL) {
    fprintf(stderr, "unable to load %s automaton\n", *argv);
    exit(1);
  }

  printf("done.\n\n%s", CR);


  char * basedir = strdup(*argv);
  get_filename_path(*argv, basedir);


  char * outname = (char *) malloc(strlen(*argv) + 5);
  sprintf(outname, "%s.new", *argv);

  FILE * out;
  if ((out = u_fopen(outname, U_WRITE)) == NULL) {
    fprintf(stderr, "unable to open %s for writting\n", outname);
    exit(1);
  }


  string_hash * labels = new_string_hash();

  get_hash_number(epsilon, labels);

  fst_header(A, out);

  for (int i = 1; i <= A->nombre_graphes; i++) {

    if ((i % 100) == 0) {
      printf("%d/%d sentences\n%s", i, A->nombre_graphes, CR); 
    }

    char grfname[MAX_PATH];
    sprintf(grfname, "%ssentence%d.grf", basedir, i);

    grf_t * grf = NULL;
    FILE * f;

    grf = NULL;

    if ((f = u_fopen(grfname, U_READ)) != NULL) {

      if ((grf = grf_load(f)) == NULL) {

	fprintf(stderr, "unable to load '%s'.\n", grfname);

      } else if (clean_grf(grf) == -1) {

	fprintf(stderr, "%s is a bad grf file.\n", grfname);
	grf_delete(grf);
	grf = NULL;
      }

      fclose(f);
    }


    u_fprintf(out, "-%d %s\n", i, A->nom_graphe[i]);

    if (grf) {

      output_grf(grf, labels, out);
      grf_delete(grf);

    } else {

      output_fst(A, i, labels, out);
    }
  }

  output_labels(labels, out);

  printf("\nall is done.\n%s", CR);

  fclose(out);

  
  /* make a backup and replace old automaton with new */

  sprintf(outname, "%s.bck", *argv);
  rename(*argv, outname);

  sprintf(outname, "%s.new", *argv);
  rename(outname, *argv);

  printf("\nYou can find a backup of the original \"%s\" file in \"%s.bck\".\n%s", *argv, *argv, CR);
}

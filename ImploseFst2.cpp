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

#include "Copyright.h"
#include "utils.h"
#include "autalmot_old.h"
#include "fst2autalmot.h"
#include "implose.h"
#include "IOBuffer.h"


/* Dans implose.cpp

int compare_trans(tTransitions * t1, tTransitions * t2) {

  if (t1 == t2) { return 0; }

  if (!t1 || ! t2) { return t2 - t1; }

  if (t1->but != t2->but) { return t2->but - t1->but; }

  if (t1->etiq == t2->etiq) { return 0; }

  if (! t1->etiq || ! t2->etiq) { return t2->etiq - t1->etiq; }

  if (t1->etiq->sorteSymbole != t2->etiq->sorteSymbole) { return t2->etiq->sorteSymbole - t1->etiq->sorteSymbole; }

  int res;

  if ((res = u_strcmp(t1->etiq->flechie, t2->etiq->flechie)) != 0)     { return res; }
  if ((res = u_strcmp(t1->etiq->canonique, t2->etiq->canonique)) != 0) { return res; }

  unichar * p1 = t1->etiq->gramm;
  unichar * p2 = t2->etiq->gramm;

  while (*p1 && *p2) {
    if (*p1 != *p2) { return *p2 - *p1; }
    if (*p1 == ':') { return 0; }
    p1++; p2++;
  }

  return *p2 - *p1;
}


tTransitions * sort_trans(tTransitions * trans) {

  if (trans == NULL) { return NULL; }

  tTransitions * root  = trans;
  trans = trans->suivant;
  root->suivant = NULL;

  tTransitions * next;

  while (trans) {

    next = trans->suivant;
    trans->suivant = NULL;

    if (compare_trans(trans, root) <= 0) {

      trans->suivant = root;
      root = trans;

    } else {

      tTransitions * t;
      for (t = root; t->suivant && (compare_trans(trans, t->suivant) > 0); t = t->suivant);

      trans->suivant = t->suivant;
      t->suivant = trans;
    }

    trans = next;
  }

  return root;
}



void implode(tAutAlMot * A) {

  etat e;

  for (e = 0; e < A->nbEtats; e++) {

    A->etats[e] = sort_trans(A->etats[e]);

    for (tTransitions * t = A->etats[e]; t;) {

      while (compare_trans(t, t->suivant) == 0) {

	unichar * p = u_strchr(t->suivant->etiq->gramm, ':');

	if (p) { u_strcat(t->etiq->gramm, p); }

	tTransitions * suiv = t->suivant->suivant;
	transition_delete(t->suivant);
	t->suivant = suiv;
      }

      t = t->suivant;
    }
  }
}

*/

void usage() {
  printf("%s", COPYRIGHT);
  printf("Usage: ImploseFst2 <txtauto> -o <out>\n"
         "\n"
         "where :\n"
         " <txtauto>     : input text automaton FST2 file,\n"
         " <out>         : resulting text automaton\n"
         "\n"
         "\"Implose\" the specified text automaton by merging together lexical entries\n"
         "which differ only in their flexional features.\n"
         "The resulting text automaton is stored in <out>.\n\n");
}


int main(int argc, char ** argv) {

  setBufferMode();  

  debug("implosefst2\n");

  char * txtname = NULL, * outname = NULL;

  argv++, argc--;

  if (argc == 0) { usage(); return 0; }

  while (argc) {

    if (**argv != '-') {

      txtname = *argv;

    } else {

      if (strcmp(*argv, "-h") == 0) {

	usage();
        return 0;

      } else if (strcmp(*argv, "-o") == 0) {

	*argv++, argc--;
	if (argc == 0) { fatal_error("'-o' needs an additionnal argument\n"); }

	outname = *argv;
      }
    }

    *argv++, argc--;
  }

  if (txtname == NULL) { fatal_error("no text automaton specified\n"); }

  
  if (outname == NULL) {

    int len = strlen(txtname);
    outname = (char *) malloc((len + 10) * sizeof(char));

    strcpy(outname, txtname);

    if (strcmp(txtname + len - 5, ".fst2") == 0) {
      strcpy(outname + len - 5, "-imp.fst2");
    } else {
      strcat(outname, "-imp.fst2");
    }
  }


  printf("loading '%s'\n", txtname);

  list_aut_old * txtauto = load_text_automaton(txtname, false);

  if (txtauto == NULL) { fatal_error("unable to load '%s'\n", txtname); }


  printf("implosion ....\n");

  for (int i = 0; i < txtauto->nb_aut; i++) {
    //debug("%d/%d\n", i, txtauto->nb_aut);
    implose(txtauto->les_aut[i]); 
  }

  if (text_output_fst2_fname(txtauto, outname) == -1) {
    fatal_error("unable to implose fst in '%s'\n", outname);
  }
  list_aut_old_delete(txtauto);

  printf("done. '%s' implosed in '%s'.\n", txtname, outname);

  return 0;
}



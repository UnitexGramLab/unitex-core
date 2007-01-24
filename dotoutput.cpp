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

#include "list_aut.h"
#include "entrsort.h"
#include "utils.h"


static char * outputdir = "dotout";


int main(int argc, char ** argv) {

  if (argc < 2) { die("usage: %s <txtauto>\n", *argv); }

  argv++, argc--;

  tChargeurPhrases * chPhrases = constrChargeurPhrases(*argv) ;

  list_aut * txtauto = (list_aut *) malloc(sizeof(list_aut));
  if (txtauto==NULL) {
     fatal_error("Not enough memory in main of dotoutput\n");
  }

  error("trying chargePhrases for %s...\n", *argv);

  int nb = chargePhrases(chPhrases, txtauto);

  FILE * f;
  char buf[1025];

  for (int i = 0; i < nb; i++) {

    error(".");

    sprintf(buf, "%s/%s-%d.dot", outputdir, *argv, i);
    if ((f = fopen(buf, "w")) == NULL) {
       fatal_error("unable to open %s for writing\n", buf);
    }
    autalmot_dump_dot(txtauto->les_aut[i], f);
    fclose(f);
  }

  error(" done.\n");
  return 0;
}

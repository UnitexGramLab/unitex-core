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
#include "autalmot.h"
#include "list_aut.h"


void libereListAut(list_aut * liste) {
  int a ;
  free(liste->tailleLog) ;
  for (a = 0 ; a < liste->nb_aut ; a++)
    libereAutAlMot(liste->les_aut[a]) ;
  free(liste->les_aut) ;
}


void fusion(list_aut * liste1, list_aut * liste2) {

  /* Fusionne liste1 et liste2. Modifie liste1 et libere liste2. */
  /* Preconditions : aucun des deux pointeurs n'est nul ; les deux */
  /* champs tailleInit sont a zero ; liste1 -> nb_aut > 0. */

  int n ;

  liste1->les_aut = (tAutAlMot **) realloc(liste1->les_aut, (liste1->nb_aut + liste2->nb_aut) * sizeof(tAutAlMot *));

  for (n = 0 ; n < liste2 -> nb_aut ; n ++)
    liste1->les_aut[liste1->nb_aut++] = liste2->les_aut[n];

  free(liste2->les_aut);
  free(liste2);
}

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

/* evamble.cpp */
/* Date 	: aout2001 */
/* Auteur(s) 	: Eric Laporte et al. */
/* Objet 	:  evaluation de l'ambiguite lexicale */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "general.h"
#include "autalmot.h"
#include "list_aut.h"
#include "entrsort.h"
#include "evamble.h"
#include "decompte.h"


/* Postcondition : si on renvoie un resultat non nul, le nombre de
 *  phrases n'est pas nul.
 */

tChargeurPhrases * evamble(char * nomFich) {

  tChargeurPhrases * chPhrases;

  if (! nomFich)
    erreurInt("evamble") ;

  chPhrases = constrChargeurPhrases(nomFich) ;

  list_aut * phrases = chargePhrases(chPhrases);

  libereListAut(phrases);
  free(phrases);


  if (! chPhrases->nbPhrases) {
    fprintf(fErr, "Error: no sentences in file %s.\n", nomFich);
    fclose(fErr);
    exit(1);
  }

  printf("%s:\n%.1f units of lexical ambiguity per sentence.\n", nomFich, chPhrases->cumul/chPhrases->nbPhrases);
  printf("%d  sentences, %d void.\n", chPhrases->nbPhrases, chPhrases->nbPhrasesVides);


  return chPhrases ;
}


void compareAmbLex(tChargeurPhrases * avant, tChargeurPhrases * apres) {
/* Preconditions : si avant et apres ont un contenu non nul, le nombre de
	phrases n'est pas nul. */
if (! avant || ! apres)
   erreurInt("compareAmbLex") ;
if (avant -> nbPhrases != apres -> nbPhrases)
	fprintf(fErr, "Error: number of sentences different in %s and %s.\n",
		avant -> nomFich, apres -> nomFich) ;
else if (avant -> cumul == 0.0) {
	printf("No lexical ambiguity found in first file.\n") ;
	fprintf(fErr, "No lexical ambiguity found in first file.\n") ; }
else if (apres -> cumul/avant -> cumul > 0.01) {
	printf("Residual ambiguity in second file: %.0f %%.\n",
	   100.0 * apres -> cumul/avant -> cumul) ;
	fprintf(fErr, "Residual ambiguity in second file: %.0f %%.\n",
		100.0 * apres -> cumul/avant -> cumul) ; }
else {
	printf("Residual ambiguity in second file: %.1f %%.\n",
	   100.0 * apres -> cumul/avant -> cumul) ;
	fprintf(fErr, "Residual ambiguity in second file: %.1f %%.\n",
		100.0 * apres -> cumul/avant -> cumul) ; } }

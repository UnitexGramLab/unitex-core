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

/* entrsort.h */

#ifndef __entrsort_h
#define __entrsort_h

#include "list_aut.h"

#define maxAut 512  /* Nombre maximal d'automates */
#define ACYCLIQUE 0
#define AVECCYCLES 1
#define maxFormesFl 32

typedef struct chargeurPhrases {
  char * nomFich ;
  //  FILE * fic ;
  //  tSymbole limiteDePhrase ;
  double cumul ;
  int nbPhrases, nbPhrasesVides ;
} tChargeurPhrases;           /* Chargeur des phrases du texte */


tAutAlMot * chargeAutGramm(char * nom_fic) ;
tChargeurPhrases * constrChargeurPhrases(char * nom_fic_phrases) ;
void destrChargeurPhrases(tChargeurPhrases * chPhrases) ;
list_aut * chargePhrases(tChargeurPhrases * chPhrases);
//int chargePhrases_old(tChargeurPhrases * chPhrases, list_aut * phrases);
void sauvegTexte(FILE * f, list_aut * Laut, int premier) ;
void sauvegPhrase(char * Nom_Fic, tAutAlMot * aut, int Naut,
   float tailleInit) ;


int compte_flex(unichar * gram);
int classe(tSymbole * symb, char * nomFich, unichar * gram);

BOOL complet(unichar * gramm);

int suppress_limphrase(tAutAlMot * A);

#endif



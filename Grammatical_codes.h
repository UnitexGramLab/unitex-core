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

//---------------------------------------------------------------------------
#ifndef Grammatical_codesH
#define Grammatical_codesH
//---------------------------------------------------------------------------
#include "unicode.h"
#include "Flexional_codes.h"
#include "Facteurs_interdits.h"
#include "LocatePattern.h"


struct liste_code_flexion {
  Code_flexion code;
  int numero_pattern;
  struct facteurs_interdits* f;
  unichar* canonique;
  struct liste_code_flexion* suivant;
};

struct noeud_code_gramm {
  struct liste_code_flexion *liste;
  unichar* mot;
  struct liste_feuilles_code_gramm *l;
};

struct liste_feuilles_code_gramm {
  struct noeud_code_gramm *node;
  struct liste_feuilles_code_gramm *suivant;
};




struct noeud_code_gramm* nouveau_noeud_code_gramm();
void inserer_code_gramm(int,unichar*,unichar*);
void decouper_code_gramm(unichar*,unichar**,unichar**,struct facteurs_interdits*);
void ajouter_combinaisons_code_gramm(unichar**,Code_flexion,int,
                                     struct facteurs_interdits*,unichar*);
void creer_ensemble_code_gramm(int*,unichar**,unichar**,Code_flexion,
                               int,int,int,
                               struct facteurs_interdits*,unichar*);
void ajouter_element_code_gramm(unichar**,int,struct noeud_code_gramm*,
                                Code_flexion,int,
                                struct facteurs_interdits*,unichar*);
void get_numeros_pattern(struct dela_entry*,unsigned char *,int);

#endif

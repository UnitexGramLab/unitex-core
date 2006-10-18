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
#ifndef Flexional_codesH
#define Flexional_codesH
//---------------------------------------------------------------------------
#include "unicode.h"
#include "DELA.h"

#define MAX_FLEXIONAL_CODES_LENGTH 400

struct code_flexion {
  unichar s[MAX_FLEXIONAL_CODES_LENGTH];
};
typedef struct code_flexion* Code_flexion;



Code_flexion nouveau_code_flexion();
Code_flexion calculer_code_flexion(unichar**);
Code_flexion calculer_code_flexion(struct dela_entry*);
void ajouter_a_liste_code_flexion(struct noeud_code_gramm*,Code_flexion,
                                  int,struct facteurs_interdits*,
                                  unichar*);

#endif

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

//---------------------------------------------------------------------------
#ifndef Regular_expressionH
#define Regular_expressionH
//---------------------------------------------------------------------------

#include "Unicode.h"


#define N_ETATS_REG 10000

struct etat_reg {
  unichar *contenu;
  struct liste_transitions_reg *l;
  int nombre_trans;
};
typedef struct etat_reg* Etat_reg;

struct liste_transitions_reg {
  int numero;
  struct liste_transitions_reg *suivant;
};



int reg2grf(unichar*,char*);
Etat_reg nouvel_etat_reg(unichar*);
int reg_2_grf(unichar*,int*,int*,Etat_reg*,int*);
void relier_reg(int,int,Etat_reg*);
void sauver_etats_reg(FILE*,Etat_reg*,int);
#endif

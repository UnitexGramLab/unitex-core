 /*
  * Unitex
  *
  * Copyright (C) 2001-2005 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#ifndef Arbre_dicoH
#define Arbre_dicoH
//---------------------------------------------------------------------------

#include "unicode.h"
#include "String_hash.h"

struct liste_nbre {
       int n;
       struct liste_nbre* suivant;
};


struct arbre_dico {
       struct liste_nbre* arr;
       struct arbre_dico_trans* trans;
       int offset; // this value will be used to give to this node an adress in the .BIN file
       int n_trans;
       int hash_number;
};


struct arbre_dico_trans {
       unichar c;
       struct arbre_dico* noeud;
       struct arbre_dico_trans* suivant;
};


void free_arbre_dico(struct arbre_dico*);
void free_arbre_dico_non_rec(struct arbre_dico*);
void free_arbre_dico_trans(struct arbre_dico_trans*);
void inserer_entree(unichar*,unichar*,struct arbre_dico*,struct string_hash*);
struct arbre_dico* new_arbre_dico();

#endif

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
#ifndef Arbre_charH
#define Arbre_charH
//---------------------------------------------------------------------------
#include "unicode.h"
#include "Alphabet.h"

struct liste_nbre {
       int etiq;
       int arr;
       struct liste_nbre* suivant;
};


struct arbre_char {
       struct liste_nbre* arr;
       struct arbre_char_trans* trans;
};


struct arbre_char_trans {
       unichar c;
       struct arbre_char* noeud;
       struct arbre_char_trans* suivant;
};


void free_arbre_char(struct arbre_char*);
void free_arbre_char_trans(struct arbre_char_trans*);
void inserer_etiquette(unichar*,int,int,struct arbre_char*);
struct arbre_char* new_arbre_char();
struct liste_nbre* get_matching_etiquettes(unichar*,struct arbre_char*,Alphabet*,int);


#endif

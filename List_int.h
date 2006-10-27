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
#ifndef Liste_nombresH
#define Liste_nombresH
//---------------------------------------------------------------------------

struct liste_nombres {
   int n;
   struct liste_nombres* suivant;
};


struct liste_nombres* new_liste_nombres();
void free_liste_nombres(struct liste_nombres*);
struct liste_nombres* inserer_dans_liste_nombres(int,struct liste_nombres*);
int appartient_a_liste(int,struct liste_nombres*);
int are_equivalent_liste_nombres(struct liste_nombres*,struct liste_nombres*);

#endif

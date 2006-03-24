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

#include <stdlib.h>

#include "Liste_nombres.h"
//---------------------------------------------------------------------------



struct liste_nombres* new_liste_nombres() {
struct liste_nombres* l;
l=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
l->suivant=NULL;
return l;
}


void free_liste_nombres(struct liste_nombres* l) {
struct liste_nombres* tmp;
while (l!=NULL) {
  tmp=l;
  l=l->suivant;
  free(tmp);
}
}


struct liste_nombres* inserer_dans_liste_nombres(int n,struct liste_nombres* l) {
struct liste_nombres* tmp;
if (l==NULL) {
   tmp=new_liste_nombres();
   tmp->n=n;
   return tmp;
}
if (n==l->n) return l;
if (n<l->n) {
   tmp=new_liste_nombres();
   tmp->n=n;
   tmp->suivant=l;
   return tmp;
}
l->suivant=inserer_dans_liste_nombres(n,l->suivant);
return l;
}



int appartient_a_liste(int n,struct liste_nombres* l) {
while (l!=NULL) {
  if (l->n==n) return 1;
  l=l->suivant;
}
return 0;
}



int are_equivalent_liste_nombres(struct liste_nombres* a,struct liste_nombres* b) {
if (a==NULL) {
   if (b==NULL) return 1;
   else return 0;
} else if (b==NULL) {return 0;}
if (a->n!=b->n) return 0;
return are_equivalent_liste_nombres(a->suivant,b->suivant);
}

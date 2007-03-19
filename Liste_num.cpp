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

#include "Liste_num.h"
#include "TransductionVariables.h"
//---------------------------------------------------------------------------

struct liste_num* new_liste_num(int n,int sommet,unichar pile[],Variables* v) {
struct liste_num* l;
l=(struct liste_num*)malloc(sizeof(struct liste_num));
l->position=n;
l->suivant=NULL;
l->sommet=sommet;
u_strcpy(l->pile,pile);
l->variable_backup=create_variable_backup(v);
return l;
}


/* inserts an element to list_num l only if there is no element with
   same n (same end position of match) */
/* may be optimized: Locate with "Ignore outputs" does not need
   sommet nor pile */
struct liste_num* inserer_si_absent(int n,struct liste_num* l,int sommet,
                                    unichar* pile,Variables* v) {
if (l==NULL) return new_liste_num(n,sommet,pile,v);
if (l->position==n) {
  l->sommet=sommet;
  u_strcpy(l->pile,pile);
  free_variable_backup(l->variable_backup);
  l->variable_backup=create_variable_backup(v);
  return l;
}
l->suivant=inserer_si_absent(n,l->suivant,sommet,pile,v);
return l;
}


/* inserts an element to list_num l only if there is no element with
   same n _and_ same pile */
struct liste_num* inserer_si_different(int n,struct liste_num* l,int sommet,
                                       unichar* pile,Variables* v) {
if (l==NULL)
  return new_liste_num(n,sommet,pile,v);
if ((l->position==n)                       // length is the same
    && !(u_strcmp(l->pile,pile))) { // stack content, too
  // overwrite liste_num entry
  l->sommet=sommet;
  u_strcpy(l->pile,pile);
  free_variable_backup(l->variable_backup);
  l->variable_backup=create_variable_backup(v);
  return l;
}

l->suivant=inserer_si_different(n,l->suivant,sommet,pile,v);
return l;
}


void free_list_num(struct liste_num* l) {
struct liste_num* tmp;
while (l!=NULL) {
   tmp=l->suivant;
   free_variable_backup(l->variable_backup);
   free(l);
   l=tmp;
}
}

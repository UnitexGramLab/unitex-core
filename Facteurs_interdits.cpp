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
#include "Facteurs_interdits.h"
//---------------------------------------------------------------------------



struct facteurs_interdits* nouveaux_facteurs_interdits() {
struct facteurs_interdits *f;
int i;
f=(struct facteurs_interdits*)malloc(sizeof(struct facteurs_interdits));
f->nbre_facteurs=0;
for (i=0;i<10;i++)
  f->facteur[i]=NULL;
return f;
}



void liberer_facteurs_interdits(struct facteurs_interdits *f) {
int i;
if (f==NULL) return;
for (i=0;i<f->nbre_facteurs;i++)
  if (f->facteur[i]!=NULL) free(f->facteur[i]);
free(f);
}



void ajouter_facteur_interdit(struct facteurs_interdits *f,unichar* s) {
int i;
i=f->nbre_facteurs;
if (i==9) {
  return;
}
f->facteur[i]=(unichar*)malloc((u_strlen(s)+1)*sizeof(unichar));
u_strcpy(f->facteur[i],s);
f->nbre_facteurs++;
}

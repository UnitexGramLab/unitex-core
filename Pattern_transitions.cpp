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
#include "Pattern_transitions.h"
//---------------------------------------------------------------------------

struct pattern_transitions* tableau[500000];

struct liste_nombres* convertir_liste_en_tableau(struct liste_nombres *ptr,int i) {
int octet,bit;
struct liste_nombres* tmp;
if (ptr==NULL) return NULL;
octet=ptr->n/8;
bit=ptr->n%8;
tableau[i]->c[octet]=(unsigned char)(tableau[i]->c[octet]|(1<<bit));
tmp=ptr->suivant;
free(ptr);
return convertir_liste_en_tableau(tmp,i);
}



void convert_pattern_lists(struct string_hash* tok) {
int i,j;
int n_octets;
n_octets=100;
for (i=0;i<tok->N;i++) {
  if (tableau[i]->n) {
    tableau[i]->c=(unsigned char*)malloc(sizeof(unsigned char)*n_octets);
    for (j=0;j<n_octets;j++)
      tableau[i]->c[j]=0;
    tableau[i]->l=convertir_liste_en_tableau(tableau[i]->l,i);
  }
}
}



struct pattern_transitions* nouveau_pattern_transitions() {
struct pattern_transitions *ptr;
ptr=(struct pattern_transitions*)malloc(sizeof(struct pattern_transitions));
ptr->l=NULL;
ptr->c=NULL;
ptr->n=0;
return ptr;
}



void init_pattern_transitions(struct string_hash* tok) {
int i;
for (i=0;i<tok->N;i++)
  tableau[i]=nouveau_pattern_transitions();
}


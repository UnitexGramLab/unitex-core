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

#include "LiberationFst2.h"

//---------------------------------------------------------------------------


void liberer_transitions(Fst2Transition ptr) {
if (ptr==NULL) return;
liberer_transitions(ptr->next);
free(ptr);
}


void liberer_etat(Fst2State e) {
if (e==NULL) return;
liberer_transitions(e->transitions);
free(e);
}



void liberer_etiquette(Fst2Tag e) {
if (e==NULL) return;
if (e->input!=NULL) free(e->input);
if (e->inflected!=NULL) free(e->inflected);
if (e->lemma!=NULL) free(e->lemma);
if (e->codes!=NULL) free(e->codes);
if (e->output!=NULL) free(e->output);
free(e);
}

//
// libere les ressources utilisees pour le stockage du FST2
//
void liberer_graphe_fst2(int nombre_etats,int nombre_etiquettes,int *debut_graphe,Fst2State graphe[],Fst2Tag etiquette[]) {
int i;
for (i=0;i<nombre_etats;i++)
  liberer_etat(graphe[i]);
for (i=0;i<nombre_etiquettes;i++)
  liberer_etiquette(etiquette[i]);
free(debut_graphe);
}

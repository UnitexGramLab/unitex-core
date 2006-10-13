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
#ifndef Load_DLF_DLCH
#define Load_DLF_DLCH
//---------------------------------------------------------------------------
#include "unicode.h"
#include "DELA.h"

struct noeud_dlf_dlc {
   struct liste_chaines* liste;
   struct trans_dlf_dlc* trans;
};

struct trans_dlf_dlc {
   unichar c;
   struct noeud_dlf_dlc* arr;
   struct trans_dlf_dlc* suivant;
};

#warning TO BE DEPRECATED
/* This structure should be a list of struct dela_entry* */
struct liste_chaines {
   unichar* chaine;
   struct liste_chaines* suivant;
};


struct noeud_dlf_dlc* new_noeud_dlf_dlc();
void free_noeud_dlf_dlc(struct noeud_dlf_dlc*);
void free_trans_dlf_dlc(struct trans_dlf_dlc*);
void free_liste_chaines(struct liste_chaines*);
void load_dlf_dlc(char*,struct noeud_dlf_dlc*);

#endif


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
#ifndef Loading_dicH
#define Loading_dicH
//---------------------------------------------------------------------------

#include "unicode.h"
#include "DELA.h"
#include "Alphabet.h"
#include "String_hash.h"
#include "Text_tokens.h"
#include "List_int.h"
#include "LocatePattern.h"
#include "CompoundWordTree.h"
#include "LocateConstants.h"

struct noeud {
  unsigned char controle;
  // bit 0: est fin_mot
  // bit 1: est MOT
  // bit 2: est DIC
  // bit 4: est MAJ
  // bit 8: est MIN
  // bit 16: est PRE
  // bit 32: est CDIC (case of a compound tag like {aujourd'hui,.ADV})
  // bit 64: est SDIC
  unichar* formes_flechies;
  unichar lettre;
  int numero;	// le numero du mot dans l'index
  struct liste_feuilles* l;
};

struct liste_feuilles {
  struct noeud* node;
  struct liste_feuilles* suivant;
};



struct noeud* nouveau_noeud();
void load_dic_for_locate(char*,Alphabet*,struct string_hash*,int,int,int,int,struct DLC_tree_info*,int);
void ajouter_forme_flechie(unichar*,int,struct noeud*,unichar*);
void check_patterns_for_tag_tokens(Alphabet*,struct string_hash*,int,struct DLC_tree_info*,int,struct list_int**);
int est_un_mot_simple(unichar*,Alphabet*,int);

extern struct noeud *racine;

struct noeud* get_sous_noeud(struct noeud *n,unichar c,int creer);

#endif

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
#ifndef MatchesH
#define MatchesH
//---------------------------------------------------------------------------

#include "unicode.h"


#define LONGUEST_MATCHES 0
#define SHORTEST_MATCHES 1
#define ALL_MATCHES 2
#define IGNORE_TRANSDUCTIONS 0
#define MERGE_TRANSDUCTIONS 1
#define REPLACE_TRANSDUCTIONS 2
#define IGNORE_AMBIG_TRANSDUCTIONS 0
#define ALLOW_AMBIG_TRANSDUCTIONS 1

struct liste_matches {
  int debut;
  int fin;
  unichar* output;
  struct liste_matches *suivant;
};


extern struct liste_matches *liste_match;
extern int nombre_match;
extern int nombre_output;
extern int longueur_avant;
extern int longueur_apres;
extern int statut_match;
extern int transduction_mode;
extern int ambig_transduction_mode;
extern int SEARCH_LIMITATION;


void init_matches();
void  afficher_match_fst2(int,unichar*);
struct liste_matches* nouveau_match(unichar*);
void free_liste_matches(struct liste_matches*);
struct liste_matches* eliminer_shortest_match_fst2(struct liste_matches*,int,int*,unichar*);
struct liste_matches* ecrire_index_des_matches(struct liste_matches*,int,
                                               long int*,FILE*);
struct liste_matches* load_match_list(FILE*,int*);

#endif

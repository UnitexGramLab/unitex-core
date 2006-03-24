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

#ifndef __deter_h
#define __deter_h



/* liste chainee de symboles de l'alphabet d'un automate
 */

struct alphabet {
  tSymbole * etiquette;  /* partage avec l automate dans deter.c */
  struct alphabet * suiv;
};
typedef struct alphabet alphabet;


/* bi-alphabet
 * chaque symbole a sa forme d origine, puis une 
 * forme developpee en une liste chainee de symboles
 * disjoints dont l union correspond au symbole d origine.
 * Les bi-alphabets ne tiennent pas compte des transitions avec
 * but par defaut.
 */

struct tBiAlph {
   tSymbole * orig; /* symbole  d origine (partage avec l automate) */
   alphabet * devel;  /* developpement du symbole */
   struct tBiAlph * suiv;
};
typedef struct tBiAlph tBiAlph ;


/* liste chainee de transitions donnant acces au bi-alphabet
 */

struct tTransBiAlph {
  tBiAlph * etiq ;  /* acces partage a un element du bi-alphabet */
  etat but ;
  struct tTransBiAlph * suiv ;
};
typedef struct tTransBiAlph tTransBiAlph ;


/* liste chainee d'etats donnant eventuellement acces
 * aux listes de transitions et indirectement au bi-alphabet
 */

struct noeud {
  etat nom ;
  tTransBiAlph * trans ;
  struct noeud * suiv ;
};
typedef struct noeud noeud ;

tAutAlMot * determinisation(tAutAlMot * autEntree) ;
tAutAlMot * deterCompl(tAutAlMot * autEntree) ;
alphabet * CreerAlphabet(tAutAlMot * autom, alphabet * alpha) ;
tSymbole * posAlph(tSymbole * s, alphabet * a) ;
tBiAlph * posBiAlph(tSymbole * s, tBiAlph * biAlph) ;
void libereBiAlph(tBiAlph * a) ;

#endif

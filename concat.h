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

/* concat.h */
/* auteurs : pascal  hafid Jean-Marc*/

#ifndef __concat_h
#define __concat_h

#define etatInitial	1
#define etatsFinaux	0

typedef struct strMultiTrans {
   etat but ;   /* Etat but de la transition */
   /* (etat source s'il s'agit d'une liste de transitions entrantes) */
   alphabet * etiq ;         /* Etiquettes de la transition */
   struct strMultiTrans * suivant ;
      } tMultiTrans ;       /* Liste de transitions */

tAutAlMot * concatenation(tAutAlMot * entAut1,tAutAlMot * entAut2);
tAutAlMot * unionAut(tAutAlMot * aut1,tAutAlMot * aut2) ;
tAutAlMot * AjoutBoucle(tAutAlMot * aut1, int pos) ;

void autalmot_concat(tAutAlMot * res, tAutAlMot * a);

#endif

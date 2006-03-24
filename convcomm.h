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


/********************************************************/
/*  NOM : convcomm.h               */
/*  analyse de la ligne de commande          */
/*  Date  : juillet 1998               */
/*  Auteur : Eric Laporte         */

#ifndef _CONVCOM_H_
#define _CONVCOM_H_

#define maxOpt 16   /* nombre maximal d'options */
#define maxVar 8    /* nombre maximal de variantes d'une option, plus un */

struct convComm {
  int nbOpt ;      /* nombre d'options (en comptant le parametre principal) */
  /* s'il y a un parametre principal, il apparait comme option numero zero */
  char identif[maxOpt][maxVar] ; /* identificateurs des options */
                                 /* et leurs variantes */
  char parametre[maxOpt] ; /* dit si chaque option a un parametre : */
                            /* 0 = non, 1 = oui */
  char tiret ;    /* dit si les options sont introduites par des tirets : */
                   /* 'o' = obligatoire, 'f' = facultatif, 'i' = interdit */
  char separees ; /* dit si les options doivent etre separees */
                   /* par des blancs : 0 = non, 1 = oui. */
};
typedef struct convComm tConvComm ;

struct ligneComm {    /* ligne de commande */
   char activee[maxOpt] ; /* dit si les options sont activees (0 ou 1) */
   char * parametre[maxOpt] ; } ;
typedef struct ligneComm tLigneComm ;

void lireLigneComm(tConvComm * conventions, tLigneComm * ligne, int c, char * v[]) ;

#endif

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


/* Nom 		: compgr.h */
/* Date 	: juin 98 */
/* Auteur(s) 	: MAYER Laurent et al */

#ifndef _COMPGR_H_
#define _COMPGR_H_

#include "autalmot.h"


#define maxRegles 16
#define maxContraintes 8

#define NEXISTEPAS - 1

typedef struct {
  autalmot_t * G, * D;
} tContexte;

typedef struct {
  char * nom;
  autalmot_t * autLu;      /* Automate lu dans un fichier */
//  autalmot_t * compilee;
  int nbContextes;        /* nombre de contextes, au moins 1 */
  tContexte * contexte;   /* liste des contextes */  
} tRegle;


int compile_grammar(char * gram, char * output);
int compile_rules(char * rulesname, char * outname);

#endif

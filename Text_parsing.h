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
#ifndef Text_parsingH
#define Text_parsingH
//---------------------------------------------------------------------------

#include "Matches.h"
#include "Fst2.h"
#include "TransductionStack.h"
#include "String_hash.h"
#include "Liste_num.h"
#include "LocateConstants.h"
#include "Context.h"
#include "CompoundWordTree.h"

/* $CD$ begin */
#include "GF_lib.h"
/* $CD$ end   */



#define NBRE_ARR_MAX 50
#define TAILLE_PILE 500 /* the maximal size of recursive calls of the
                           function parcourir_opt =~ the maximal number
                           of tokens to be recognized in one match */
#define MAX_MATCHES_AT_TOKEN_POS 400 /* the maximal number of matches
                                        starting from one token : this
                                        value is critical in the case
                                        of bad designed grammars */
#define BUFFER_SIZE 1000000

#define MODE_MORPHO 0
#define MODE_NON_MORPHO 1

#define MAX_ERRORS 50  /* maximal number of errors before exiting:
                          needed to avoid overflow of error buffers in Java GUI */

extern int GESTION_DE_L_ESPACE;
extern int texte[BUFFER_SIZE];
extern int LENGTH;
extern int N_INT_ALLREADY_READ;
extern int origine_courante;
extern long int nombre_unites_reconnues;
extern int* debut_graphe;
extern struct string_hash* TOKENS;
extern Etiquette* ETIQUETTE;
extern int SENTENCE_DELIMITER_INDICE;
extern int STOP_MARKER_INDICE;
/* $CD$ begin */
#ifdef TRE_WCHAR
extern MasterGF_T*  masterGF;
extern IndexGF_T*   indexGF;
#endif
/* $CD$ end   */


void launch_locate(FILE*,Automate_fst2*,int,struct string_hash*,FILE*,int,long int,FILE*,
					struct DLC_tree_info*);
int dichotomie(int,int*,int);
int trouver_mot_compose_DIC(int,int,struct DLC_tree_info*);

#endif



 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef Text_parsingH
#define Text_parsingH

#include "Matches.h"
#include "Fst2.h"
#include "TransductionStack.h"
#include "String_hash.h"
#include "ParsingInfo.h"
#include "LocateConstants.h"
#include "CompoundWordTree.h"
#include "LocatePattern.h"
#include "MorphologicalFilters.h"


#define STACK_MAX 1000 /* The maximal size of recursive calls of the
                          function parcourir_opt =~ the maximal number
                          of tokens to be recognized in one match */
                         
#define MAX_MATCHES_AT_TOKEN_POS 400 /* The maximal number of matches
                                        starting from one token : this
                                        value is critical in the case
                                        of bad designed grammars */

/* Arbitrary value used as a limit. It is similar to 'MAX_MATCHES_AT_TOKEN_POS',
 * but it concerns each subgraph. */
#define MAX_MATCHES_PER_SUBGRAPH 50

#define MAX_ERRORS 50  /* Maximal number of errors before exiting:
                          needed to avoid overflow of error buffers in Java GUI */




void launch_locate(FILE*,FILE*,long int,FILE*,struct locate_parameters*);


#endif



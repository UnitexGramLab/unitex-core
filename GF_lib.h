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
#ifndef GF_libH
#define GF_libH
//---------------------------------------------------------------------------
#include "unicode.h"
#include "Alphabet.h"
#include "String_hash.h"
#include "AutomateFst2.h"

#ifndef DO_NOT_USE_TRE_LIBRARY
#include "regex.h"
#ifdef TRE_WCHAR
extern int HASH_FILTERS_DIM;

typedef struct MasterGFTab {
                                char*       options;
                                unichar*    content;
                                regex_t*    preg;
                            
                           } MasterGFTab_T;

                    
typedef struct MasterGF {
                            int             tabDim;
                            MasterGFTab_T*  tab;
                            
                        } MasterGF_T;
                        
typedef struct IndexGF {
                            int             rowDim;
                            int             colDim;
                            int             bitDim;
                            unsigned char** tab;
                       } IndexGF_T;
                       
                                                                       
MasterGF_T* CreateMasterGF  ( Automate_fst2* , Alphabet* );
void        FreeMasterGF    ( MasterGF_T*, int );

IndexGF_T*  CreateIndexGF   ( MasterGF_T*, struct string_hash* );
void        FreeIndexGF     ( IndexGF_T*, int );

int         MatchGF         ( unichar*, regex_t* );
int         MatchRawGF      ( MasterGF_T*, unichar*, int );
int         OptMatchGF      ( IndexGF_T*, int, int );                       
#endif
#endif
#endif


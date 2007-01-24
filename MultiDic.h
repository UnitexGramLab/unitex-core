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

//---------------------------------------------------------------------------
#ifndef MultiDicH
#define MultiDicH

#include "Fst2.h"
#include "FileName.h"

#define DELAF_BIN 0
#define DELAF_FST2 1

#define DELAF_HIGH_PRIORITY 1
#define DELAF_NORMAL_PRIORITY 2
#define DELAF_LOW_PRIORITY 3

#define FIRST_DIC_PARAMETER 3

struct dic {
   char type;
   char priority;
   Fst2* fst2;
   unsigned char* bin;
   struct INF_codes* INF;
};


struct multi_dic {
   struct dic** tab;
   int N;
};


struct multi_dic* init_multi_dic_from_parameters(char*,int);
void free_multi_dic(struct multi_dic*);

//---------------------------------------------------------------------------
#endif

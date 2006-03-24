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

#ifndef BinListH
#define BinListH

#include "DELA.h"

#define HIGH_PRIORITY_DICTIONARY 0
#define NORMAL_PRIORITY_DICTIONARY 1
#define LOW_PRIORITY_DICTIONARY 2


struct bin_dic {
   int priority;
   unsigned char* bin;
   struct INF_codes* inf;
};


struct bin_list {
   int n;
   struct bin_dic** tab;
};




struct bin_dic* load_dictionary(char*);
void free_dictionary(struct bin_dic*);

struct bin_list* load_dictionary_list(int,char**);
void free_dictionary_list(struct bin_list*);


#endif


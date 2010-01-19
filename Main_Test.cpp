 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "IOBuffer.h"
#include "Pattern.h"
#include "PatternTree.h"
#include "Unicode.h"
#include "String_hash.h"
#include "DELA.h"
#include "List_pointer.h"
#include "StringParsing.h"
#include "Copyright.h"
#include "Fst2.h"
#include "Error.h"
#include "Tfst.h"
#include "AbstractFst2Load.h"

/**
 * This program is designed for test purpose only.
 */
int main(int argc,char *argv[]) {
setBufferMode();

if (argc!=2) {
   u_printf("Usage: %s <fst2>\n",argv[0]);
   return 0;
}
Fst2* fst2=load_abstract_fst2(argv[1],0,NULL);
u_printf("%d graphs\n",fst2->number_of_graphs);
u_printf("%d states\n",fst2->number_of_states);
int n=0;
for (int i=0;i<fst2->number_of_states;i++) {
   Transition* t=fst2->states[i]->transitions;
   while (t!=NULL) {
      n++;
      t=t->next;
   }
}
u_printf("%d transitions\n",n);
return 0;
}





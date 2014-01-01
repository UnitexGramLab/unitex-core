/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "ElagDebug.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * A debug printing of a SingleGraph.
 */
void print_graph(SingleGraph g) {
u_printf("--------------------------------\n");
if (g==NULL) {
   u_printf("NULL graph\n");
   u_printf("--------------------------------\n");
   return;
}
for (int i=0;i<g->number_of_states;i++) {
   SingleGraphState s=g->states[i];
   if (is_initial_state(s)) {
      u_printf("-> ");
   } else {u_printf("   ");}
   if (is_final_state(s)) {
      u_printf("%d t ",i);
   } else {
      u_printf("%d : ",i);
   }
   u_printf("(def %d) \n\t",s->default_state);
   Transition* t=s->outgoing_transitions;
   while (t!=NULL) {
      symbols_dump((symbol_t*)t->label);
      u_printf(",%d ",t->state_number);
      t=t->next;
   }
   u_printf("\n\n");
}
u_printf("--------------------------------\n");
}


int DEBUG=0;

void set_debug() {
DEBUG=1;
}

void unset_debug() {
DEBUG=0;
}

int is_set_debug() {
return DEBUG;
}

} // namespace unitex

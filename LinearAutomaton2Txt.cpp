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

#include "LinearAutomaton2Txt.h"
#include "Error.h"

int isLinearAutomaton(Fst2* fst2) {
if (fst2==NULL) {
   fatal_error("NULL error in isLinearAutomaton\n");
}
Fst2Transition l;
Fst2State etat;

for (int sentence=1;sentence<fst2->number_of_graphs+1;sentence++) {
   int n=fst2->initial_states[sentence]+fst2->number_of_states_per_graphs[sentence];
   for (int i=fst2->initial_states[sentence];i<n;i++) {
      etat=fst2->states[i];
      l=etat->transitions;
      if (is_final_state(etat)) {
         if (l!=NULL) {
            // the final state must not have any outgoing transition
            return sentence;
         }
      } else {
         if (l==NULL || l->next!=NULL) {
            // if there is not exactly one transition in each state
            return sentence;
         }
      }
   }
}
return LINEAR_AUTOMATON;
}



int convertLinearAutomaton(Fst2* fst2,FILE* f) {
if (fst2==NULL) {
   fatal_error("NULL error in convertLinearAutomaton\n");
}
Fst2Transition l;
Fst2State etat;
for (int sentence=1;sentence<fst2->number_of_graphs+1;sentence++) {
   etat=fst2->states[fst2->initial_states[sentence]];
   l=etat->transitions;
   do {
      if (is_final_state(etat)) {
         if (l!=NULL) {
            // the final state must not have any outgoing transition
            return sentence;
         }
         if (sentence!=fst2->number_of_graphs) {
            // if this is not the last sentence, we put a sentence delimiter {S}
            u_fprints_char("{S}",f);
         }
         // then, in any case we print a carridge return
         u_fputc('\n',f);
         // and we set 'etat' to NULL in order to quit the loop
         etat=NULL;
      } else {
         if (l==NULL || l->next!=NULL) {
            // if there is not exactly one transition in each state
            return sentence;
         }
         unichar* etiq=fst2->tags[l->tag_number]->input;
         u_fprints(etiq,f);
         u_fprints_char(" ",f);
         etat=fst2->states[l->state_number];
         l=etat->transitions;
      }
   } while (etat!=NULL);
}
return LINEAR_AUTOMATON;
}



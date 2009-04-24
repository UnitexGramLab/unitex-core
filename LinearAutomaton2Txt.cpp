 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
#include "Transitions.h"


/**
 * This functions returns -1 if the given .fst2 is linear, i.e. if
 * all its sentence automata are linear. Otherwise, it returns the
 * number of the first non linear sentence automaton.
 */
int isLinearAutomaton(Tfst* tfst) {
if (tfst==NULL) {
   fatal_error("NULL error in isLinearAutomaton\n");
}
Transition* l;
SingleGraphState state;
for (int sentence=1;sentence<tfst->N;sentence++) {
   load_sentence(tfst,sentence);
   int n=tfst->automaton->number_of_states;
   for (int i=0;i<n;i++) {
      state=tfst->automaton->states[i];
      l=state->outgoing_transitions;
      if (is_final_state(state)) {
         if (l!=NULL) {
            /* The final state must not have any outgoing transition */
            return sentence;
         }
      } else {
         if (l==NULL || l->next!=NULL) {
            /* If there is not exactly one transition in each state */
            return sentence;
         }
      }
   }
}
return LINEAR_AUTOMATON;
}


/**
 * This function tries to convert the given .fst2 into a text file.
 * A prints one sentence per line, separating tokens with spaces.
 *
 * It returns -1 if the given .fst2 is linear, i.e. if
 * all its sentence automata are linear. Otherwise, it returns the
 * number of the first non linear sentence automaton.
 */
int convertLinearAutomaton(Tfst* tfst,U_FILE* f) {
if (tfst==NULL) {
   fatal_error("NULL error in convertLinearAutomaton\n");
}
Transition* l;
SingleGraphState state;
for (int sentence=1;sentence<tfst->N;sentence++) {
   load_sentence(tfst,sentence);
   state=tfst->automaton->states[0];
   l=state->outgoing_transitions;
   do {
      if (is_final_state(state)) {
         if (l!=NULL) {
            /* The final state must not have any outgoing transition */
            return sentence;
         }
         if (sentence!=tfst->N) {
            /* If this is not the last sentence, we put a sentence delimiter {S} */
            u_fprintf(f,"{S}");
         }
         /* Then, in any case we print a carridge return */
         u_fputc('\n',f);
         /* And we set 'state' to NULL in order to quit the loop */
         state=NULL;
      } else {
         if (l==NULL || l->next!=NULL) {
            /* If there is not exactly one transition in each state */
            return sentence;
         }
         TfstTag* tag=(TfstTag*)(tfst->tags->tab[l->tag_number]);
         u_fprintf(f,"%S ",tag->content);
         state=tfst->automaton->states[l->state_number];
         l=state->outgoing_transitions;
      }
   } while (state!=NULL);
}
return LINEAR_AUTOMATON;
}



 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "autalmot_old.h"
#include "utils.h"
#include "implose.h"
#include "DELA.h"
#include "List_pointer.h"
#include "String_hash.h"


/* return true if t1 and t2 are the same trans or
 * differs only in their morphological features
 */


bool quasi_same_trans(tTransitions * t1, tTransitions * t2) {

  if (t1 == t2) { return true; }

  if (!t1 || ! t2) { return false; }

  if (t1->but != t2->but) { return false; }

  if (t1->etiq == t2->etiq) { return true; }

  if (! t1->etiq || ! t2->etiq) { return false; }

  if (t1->etiq->sorteSymbole != t2->etiq->sorteSymbole) {
    return false;
  }

  if (u_strcmp(t1->etiq->flex->str, t2->etiq->flex->str) != 0)     { return false; }
  if (u_strcmp(t1->etiq->canonique, t2->etiq->canonique) != 0) { return false; }

  unichar * p1 = t1->etiq->gramm;
  unichar * p2 = t2->etiq->gramm;

  while (*p1 && *p2) {
    if (*p1 != *p2) { return false; }
    if (*p1 == ':') { return true; }
    p1++; p2++;
  }

  return *p2 == *p1; // (== 0)
}




/* total order over transitions
 */

int compare_trans(tTransitions * t1, tTransitions * t2) {

  if (t1 == t2) { return 0; }

  if (!t1 || ! t2) { return t2 - t1; }

  if (t1->but != t2->but) { return t2->but - t1->but; }

  if (t1->etiq == t2->etiq) { return 0; }

  if (! t1->etiq || ! t2->etiq) { return t2->etiq - t1->etiq; }

  if (t1->etiq->sorteSymbole != t2->etiq->sorteSymbole) {
    return t2->etiq->sorteSymbole - t1->etiq->sorteSymbole;
  }

  int res;

  if ((res = u_strcmp(t1->etiq->flex->str, t2->etiq->flex->str)) != 0)     { return res; }
  if ((res = u_strcmp(t1->etiq->canonique, t2->etiq->canonique)) != 0) { return res; }

  return u_strcmp(t1->etiq->gramm, t2->etiq->gramm);
}



/* sort transition list, remove duplicates
 */

tTransitions * sort_trans(tTransitions * trans) {

  if (trans == NULL) { return NULL; }

  int res;
  tTransitions * root  = trans;
  trans = trans->suivant;
  root->suivant = NULL;

  tTransitions * next;

  while (trans) {

    next = trans->suivant;
    trans->suivant = NULL;

    if ((res = compare_trans(trans, root)) <= 0) {

      if (res) {
        trans->suivant = root;
        root = trans;
      } else { // duplicate
        transition_delete(trans);
      }
    } else {

      tTransitions * t;
      for (t = root; t->suivant && ((res = compare_trans(trans, t->suivant)) > 0); t = t->suivant);

      if (res) {
        trans->suivant = t->suivant;
        t->suivant = trans;
      } else { // duplicate
        transition_delete(trans);
      }
    }

    trans = next;
  }

  return root;
}



void implose(tAutAlMot * A) {

  int e;

  for (e = 0; e < A->nbEtats; e++) {

    A->etats[e] = sort_trans(A->etats[e]);

    for (tTransitions * t = A->etats[e]; t;) {

      //while (compare_trans(t, t->suivant) == 0) {
      while (quasi_same_trans(t, t->suivant)) {

	unichar * p = u_strchr(t->suivant->etiq->gramm, ':');

        if (p) {
          if ((u_strlen(p)+u_strlen(t->etiq->gramm)) > maxGramm) {
            fatal_error("seq of gramm. codes too long: %S,%S.%S%S\n",
                t->etiq->flex->str, t->etiq->canonique, t->etiq->gramm, p);
          }
          u_strcat(t->etiq->gramm, p);
        }

	tTransitions * suiv = t->suivant->suivant;
	transition_delete(t->suivant);
	t->suivant = suiv;
      }

      t = t->suivant;
    }
  }
}



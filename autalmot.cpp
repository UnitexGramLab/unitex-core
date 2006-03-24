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

#include "utils.h"
#include "fst_file.h"
#include "autalmot.h"


transition_t * transition_new(int to, symbol_t * label, transition_t * next) {

  transition_t * trans = (transition_t *) xmalloc(sizeof(transition_t));

  trans->to    = to;
  trans->label = symbol_dup(label);
  trans->next  = next;

  return trans;
}


void transition_delete(transition_t * trans) {
  symbol_delete(trans->label);
  free(trans);
}


void transitions_delete(transition_t * trans) {
  while (transition_t * tmp = trans) {
    trans = tmp->next;
    transition_delete(tmp);
  }
}


void transitions_concat(transition_t ** t1, transition_t * t2) {
  while (*t1) { t1 = & (*t1)->next; }
  *t1 = t2;
}


inline transition_t * transition_dup(const transition_t * trans) {
  return transition_new(trans->to, trans->label, NULL);
}


transition_t * transitions_dup(const transition_t * trans) {

  transition_t res;
  res.next = NULL;

  for (transition_t * t1 = & res; trans; t1 = t1->next, trans = trans->next) { t1->next = transition_dup(trans); }

  return res.next;
}


/*
void autalmot_trans_in_delete(autalmot_t * A) {
  if (A->trans_in == NULL) { return; }

  for (int i = 0; i < A->nb_states; i++) { transitions_delete(A->trans_in[i]); }

  free(A->trans_in);

  A->trans_in = NULL;
}
*/


void transition_dump(transition_t * t, FILE * f) {
  fprintf(f, "("); symbols_dump(t->label); fprintf(f, ", %d)", t->to);
}

void transitions_dump(transition_t * t, FILE * f) {
  fprintf(f, "{ ");
  while (t) {
    transition_dump(t, f);
    fprintf(f, ", ");
    t = t->next;
  }
  fprintf(f, "}");
}


autalmot_t * autalmot_new(unichar * name, int size) {

  autalmot_t * A = (autalmot_t *) xmalloc(sizeof(autalmot_t));

  A->name = u_strdup(name);

  A->nbstates = 0;

  if (size < 1) { size = 1; }

  A->states = (state_t *) xmalloc(size * sizeof(state_t));

  for (int i = 0; i < size; i++) {
    A->states[i].flags = 0;
    A->states[i].trans = NULL;
    A->states[i].defto = -1;
  }

  A->size = size;

  A->initials   = NULL;
  A->nbinitials = 0;

  return A;
}


void autalmot_delete(autalmot_t * A) {
  
  free(A->name);

  for (int i = 0; i < A->nbstates; i++) { transitions_delete(A->states[i].trans); }
  free(A->states);

  free(A->initials);

  free(A);
}


autalmot_t * autalmot_dup(const autalmot_t * src) {

  autalmot_t * res = autalmot_new(src->name, src->nbstates);

  for (int q = 0; q < src->nbstates; q++) {
    autalmot_add_state(res, src->states[q].flags);
    res->states[q].trans = transitions_dup(src->states[q].trans);
    res->states[q].defto = src->states[q].defto;
  }

  if (src->nbinitials != res->nbinitials) {
    error("autalmot_dup: != initials\n");
    for (int i = 0; i < src->nbinitials; i++) {
      autalmot_set_initial(res, src->initials[i]);
    }
  }

  return res;
}


void autalmot_empty(autalmot_t * A) {

  free(A->name);
  A->name = NULL;

  for (int i = 0; i < A->nbstates; i++) { transitions_delete(A->states[i].trans); }
  A->nbstates   = 0;
  A->nbinitials = 0;
}


void autalmot_resize(autalmot_t * A, int size) {

  if (size == 0) { size = 1; }

  if (size < A->nbstates) { die("autalmot_resize: size(=%d) < nbstates(=%d)\n", size, A->nbstates); }

  A->states = (state_t *) xrealloc(A->states, size * sizeof(state_t));

  for (int i = A->size; i < size; i++) {
    A->states[i].flags = 0;
    A->states[i].trans = NULL;
    A->states[i].defto  = -1;
  }

  A->size = size;
}



int autalmot_add_state(autalmot_t * A, int flags) {

  if (A->nbstates >= A->size) { autalmot_resize(A, A->size * 2); }

  int res = A->nbstates;

  A->states[res].flags = flags;
  A->states[res].trans = NULL;
  A->states[res].defto = -1;

  A->nbstates++;

  if (flags & AUT_INITIAL) { autalmot_set_initial(A, res); }

  return res;
}



void autalmot_add_trans(autalmot_t * A, int from, symbol_t * label, int to) {


  if (label == SYMBOL_DEF) {
    if (A->states[from].defto != -1) { die("autalmot add trans: to much <def>\n"); }
    A->states[from].defto = to;
    return;
  }


  while (label) {

    if (label == SYMBOL_DEF) { die("autalmot_add_trans called with SYMB_DEF\n"); }

    A->states[from].trans = transition_new(to, label, A->states[from].trans);
    label = label->next;
  }
}




void autalmot_set_initial(autalmot_t * A, int q) {

  A->states[q].flags |= AUT_INITIAL;

  for (int i = 0; i < A->nbinitials; i++) { if (A->initials[i] == q) { return; } }

  A->nbinitials++;
  A->initials = (int *) xrealloc(A->initials, A->nbinitials * sizeof(int));

  A->initials[A->nbinitials - 1] = q;
}


void autalmot_unset_initial(autalmot_t * A, int q) {

  A->states[q].flags &= ~(AUT_INITIAL);

  for (int i = A->nbinitials - 1; i >= 0; i--) {
    if (A->initials[i] == q) {
      A->initials[i] = A->initials[--A->nbinitials];
    }
  }
}





void autalmot_dump(const autalmot_t * A, FILE * f) {

  i_fprintf(f, "[%S] -- nbstates = %d, %d initials : ", A->name, A->nbstates, A->nbinitials);

  for (int i = 0; i < A->nbinitials; i++) { fprintf(f, "%d, ", A->initials[i]); }
  endl(f);

  for (int q = 0; q < A->nbstates; q++) {

//    debug("q=%d\n", q);

    fprintf(f, "%d [%c%c]:, ", q, (A->states[q].flags & AUT_INITIAL) ? 'I' : ' ',
	    (A->states[q].flags & AUT_FINAL) ? 'F' : ' ');

    for (transition_t * t = A->states[q].trans; t; t = t->next) {
//      debug("trans=%d -- ", t); transition_dump(t); endl();
      fprintf(f, "("); symbol_dump(t->label, f); fprintf(f, ", %d) ", t->to);
    }

    if (A->states[q].defto != -1) { fprintf(f, "(<def>, %d) ", A->states[q].defto); }

    fprintf(f, "\n");
  }
}



void autalmot_dump_dot(const autalmot_t * A, FILE * f) {

  if (A == NULL) { die("dot_output: A is null\n"); }

  ustring_t * ustr = ustring_new();

  fprintf(f,
	  "# AutalMot output\n\n"
	  "digraph G {\n\n"
	  "  graph [ center = true, orientation = landscape, rankdir = LR ];\n"
	  "  node  [ shape  = circle ];\n");

  for (int q = 0; q < A->nbstates; q++) {

    fprintf(f, "\n  %d [ label=\"%d\" ", q, q);
    if (A->states[q].flags & AUT_TERMINAL) { fprintf(f, "shape=\"doublecircle\" "); }
    fprintf(f, "];\n");

    for (transition_t * t = A->states[q].trans; t; t = t->next) {
      symbol_to_str(t->label, ustr);
      i_fprintf(f, "  %d -> %d [ label=\"%S\" ];\n", q, t->to, ustr->str);
    }

    if (A->states[q].defto != -1) {
      i_fprintf(f, "  %d -> %d [ label=\"<def>\" ];\n", q, A->states[q].defto);
    }
  }

  fprintf(f, "}\n");

  ustring_delete(ustr);
}



void autalmot_dump_dot_fname(const autalmot_t * A, char * fname) {

  FILE * f = fopen(fname, "w");

  if (f == NULL) {
    error("autalmot_dot_output: cannot open '%s' for writing\n", fname);
    return;
  }

  autalmot_dump_dot(A, f);
  fclose(f);
}


void autalmot_output_fst2(const autalmot_t * A, char * name, int type) {

  fst_file_out_t * fstf = fst_file_out_open(name, type);

  if (fstf == NULL) { error("unable to open '%s'\n", name); return; }

  fst_file_write(fstf, A);
  fst_file_close(fstf);
}

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


#include "autalmot.h"
#include "symbol_op.h"
#include "stack.h"
#include "fst_file.h"
#include "utils.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"



static void add_limphrase(autalmot_t *);
void print_match(stack_type * stack, FILE * f);
static void locate(autalmot_t * sentence, int q1, autalmot_t * pattern, int q2,
                   int * nbm, stack_type * stack, FILE * f);
int autalmot_locate_pattern(autalmot_t * sentence, autalmot_t * pattern, FILE * f);




void usage() {
u_printf("%S",COPYRIGHT);   
u_printf("Usage: FstLocate <txt> -p pattern -l LANG\n");
}

int main(int argc, char ** argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

if (argc!=6) {
   usage();
   return 0;
}
char* txtname=NULL;
char* langname=NULL;
char* patternname=NULL;
argv++;
argc--;
while (argc) {
   if (**argv != '-') {
      txtname=*argv;
   } else {
      if (strcmp(*argv,"-h")==0) {
         usage();
         return 0;
      } else if (strcmp(*argv, "-p") == 0) {
         *argv++;
         argc--;
         if (argc==0) {
            fatal_error("-p needs an additionnal argument\n"); 
         }
         patternname=(*argv);
      } else if (strcmp(*argv, "-l")==0) {
         argv++;
         argc--;
         if (argc==0) {
            fatal_error("-l needs an additionnal argument\n");
         }
         langname=(*argv);
      } else {
         fatal_error("Unknown argument '%s'\n", *argv);
      }
   }
   *argv++;
   argc--;
}
if (txtname==NULL) {
   fatal_error("No text automaton specified\n");
}
if (langname==NULL) {
   fatal_error("no text language specified\n");
}
if (patternname==NULL) {
   fatal_error("no pattern specified\n");
}
u_printf("Loading %s langage definition ...\n", langname);
language_t* lang=language_load(langname);
set_current_language(lang);
fst_file_in_t* txtin=fst_file_in_open(txtname,FST_TEXT);
if (txtin==NULL) {
   fatal_error("Unable to load text '%s'\n", txtname);
}
u_printf("%d sentence(s) in %s\n", txtin->nbelems, txtname);
autalmot_t* pattern=load_grammar_automaton(patternname);
if (pattern==NULL) {
   fatal_error("Unable to load '%s' automaton\n", patternname);
}
autalmot_t* A;
int no=0;
int totalmatches=0;
while ((A=fst_file_autalmot_load_next(txtin)) != NULL) {
   if (no==4) break;
   u_printf("Sentence %d:\n",no+1);
   if (A->nbstates < 2) {
      error("Sentence %d is empty\n",no+1);
   } else {
      add_limphrase(A); 
      totalmatches=totalmatches+autalmot_locate_pattern(A,pattern,stdout);
   }
   autalmot_delete(A);
   no++;
}
u_printf("%d sentence%s processed -- %d matching sequence%s.\n",no,(no>1)?"s":"",totalmatches,(totalmatches>1)?"s":"");
return 0;
}


static void add_limphrase(autalmot_t * A) {

  static unichar S[] = { '{', 'S', '}', 0 };

  int idx = language_add_form(LANG, S);

  symbol_t * LIM = symbol_PUNC_new(LANG, idx);

  int initBis   = autalmot_add_state(A);
  int nouvFinal = autalmot_add_state(A, AUT_TERMINAL);

  A->states[initBis].trans  = A->states[A->initials[0]].trans;
  A->states[A->initials[0]].trans = NULL;

  A->states[initBis].flags = A->states[A->initials[0]].flags & ~(AUT_INITIAL);
  A->states[A->initials[0]].flags = AUT_INITIAL;

  autalmot_add_trans(A, A->initials[0], LIM, initBis);


  for (int q = 1; q < A->nbstates - 2; q++) {
    if (autalmot_is_final(A, q)) {
      autalmot_add_trans(A, q, LIM, nouvFinal);
      autalmot_unset_terminal(A, q);
    }
  }

  symbol_delete(LIM);
}

void print_match(stack_type * stack, FILE * f) {

  for (int i = 0; i < stack->head; i++) {
    symbol_dump((symbol_t *) stack->data[i], f); fprintf(f, " ");
  }
  fprintf(f, "\n");
}


static void locate(autalmot_t * sentence, int q1, autalmot_t * pattern, int q2,
                   int * nbm, stack_type * stack, FILE * f) {

  transition_t * t1, * t2;

  if (autalmot_is_final(pattern, q2)) { (*nbm)++; print_match(stack, f); }
  
  for (t1 = sentence->states[q1].trans; t1; t1 = t1->next) {
  
    for (t2 = pattern->states[q2].trans; t2; t2 = t2->next) {
    
      if (symbol_in_symbol(t1->label, t2->label)) {
        stack_push(stack, t1->label);
        locate(sentence, t1->to, pattern, t2->to, nbm, stack, f);
        stack_pop(stack);
      }
    }
  }
}



int autalmot_locate_pattern(autalmot_t * sentence, autalmot_t * pattern, FILE * f) {

  int nbmatches = 0;
  stack_type * stack = stack_new();

  for (int i = 0; i < sentence->nbstates; i++) {
    locate(sentence, i, pattern, 0, & nbmatches, stack, f); 
  }
  
  u_printf("%d matches\n", nbmatches);

  stack_delete(stack);
  
  return nbmatches;
}


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

/* compgr.cpp */

/* Date 	: juin 98 */
/* Auteur(s) 	: MAYER Laurent et al */
/*                Olivier Blanc 2002-2006 */
/* compilation des grammaires de levee d ambiguites */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "autalmot.h"
#include "fst_file.h"
#include "list_aut.h"
#include "compgr.h"
#include "utils.h"
#include "File.h"
#include "AutConcat.h"



/* maximum number of state for a grammar before we split it in several fst2 */

#define MAX_GRAM_SIZE   128



void split_elag_rule(elRule*) ;
static int count_constraints(Fst2Automaton * aut, int * contrainte) ;
static Fst2Automaton * compile_elag_rule(elRule * regle);
int get_sub_automaton(Fst2Automaton*,Fst2Automaton*,int,int,int);
int get_sub_automaton(Fst2Automaton*,SingleGraph,int,SymbolType,int*) ;
int get_left_constraint_part(Fst2Automaton*,SingleGraph,int,int,int*);
static Fst2Automaton * combinaison(elRule * regle, int ens, Fst2Automaton * AetoileR1, Fst2Automaton * R2Aetoile) ;




static inline void chomp(char * s) {
  while (*s) {
    if (*s == '\n' || *s == '\r' || *s == '#') {
      *s = 0;
    } else { s++; }
  }
}


static inline void strip_extension(char * s) {

  char * p = s + strlen(s) - 1;
  
  while (p > s) {
    if (*p == '.') { *p = 0; break; }
    if (*p == '/' || *p == '\\') { break; }
    p--;
  }
}


/**
 * Modify the given automaton so that it starts by a loop that can
 * recognize anything. If A is empty, it remains empty.
 */
void prefix_with_everything(Fst2Automaton* A) {
symbol_t* anything=new_symbol(LEXIC);
unichar tmp[4]={'<','.','>','\0'};
int size=A->symbols->size;
int index=get_value_index(tmp,A->symbols,INSERT_IF_NEEDED,anything);
struct list_int* list=get_initial_states(A->automaton);
while (list!=NULL) {
   add_outgoing_transition(A->automaton->states[list->n],index,list->n);
   list=list->next;
}
free_list_int(list);
if (index!=size) {
   /* If the symbol was already there, we can free it */
   free_symbol(anything);
}
}


/**
 * Modify the given automaton so that it ends by a loop that can
 * recognize anything. If A is empty, it remains empty.
 */
void suffix_with_everything(Fst2Automaton* A) {
symbol_t* anything=new_symbol(LEXIC);
unichar tmp[4]={'<','.','>','\0'};
int size=A->symbols->size;
int index=get_value_index(tmp,A->symbols,INSERT_IF_NEEDED,anything);
/* WARNING commentaire à virer
 * for (int i=0;i<A->automaton->number_of_states;i++) {
   if (is_final(A->automaton->states[i])) {

      // on detruit les transitions sortantes 

      transitions_delete(A->states[i].trans);
      A->states[i].trans = NULL;

      // remplace par un boucle 
      add_transition(A, i, UNIV, i);
    }
  }*/
if (index!=size) {
   /* If the symbol was already there, we can free it */
   free_symbol(anything);
}
}




/**
 * Allocates, initializes and returns a new Elag rule from the given .fst2.
 * Returns NULL in case of error at loading the rule.
 */
elRule* new_elRule(char* fst2) {
elRule* rule=(elRule*)malloc(sizeof(elRule));
if (rule==NULL) {
   fatal_error("Not enough memory in new_elRule\n");
}
rule->automaton=NULL;
rule->nbContexts=0;
rule->contexts=NULL;
rule->name=strdup(fst2);
if ((rule->automaton=load_elag_grammar_automaton(rule->name))==NULL) {
   error("Cannot load '%s' automaton.\n", fst2);
   free(rule->name);
   free(rule);
   return NULL;
}
split_elag_rule(rule);
return rule;
}



static void deleteRegle(elRule * regle) {

  free(regle->name);

  for (int i = 0; i < regle->nbContexts; i++) {
    free_Fst2Automaton(regle->contexts[i].right);
    free_Fst2Automaton(regle->contexts[i].left);
  }

  free(regle->contexts);

  free(regle);
}


/**
 * 
 */
Fst2Automaton* compile_elag_rule(elRule* rule) {
int p,ens;
u_printf("Compiling %s... (%d context%s)\n",rule->name,rule->nbContexts,(rule->nbContexts>1)?"s":"");
for (int c=0;c<rule->nbContexts;c++) {
   determinize(rule->contexts[c].left->automaton);
   trim(rule->contexts[c].left->automaton);
   determinize(rule->contexts[c].right->automaton);
   trim(rule->contexts[c].right->automaton);
   /*autalmot_determinize(rule->contexts[c].left);
   autalmot_emonde(rule->contexts[c].left);
   autalmot_determinize(rule->contexts[c].right);
   autalmot_emonde(rule->contexts[c].right);*/
}
/* We build A*.R1 */
prefix_with_everything(rule->contexts[0].left);
minimize(rule->contexts[0].left->automaton,1);
Fst2Automaton* AstarR1=rule->contexts[0].left;
/* and R2.A* */
suffix_with_everything(rule->contexts[0].right);
minimize(rule->contexts[0].right->automaton,1);

  Fst2Automaton * R2Aetoile = rule->contexts[0].right;


  p = (int) pow(2, rule->nbContexts - 1);

  Fst2Automaton * res = new_Fst2Automaton();

  for (ens = 0 ; ens < p ; ens++) {

    Fst2Automaton * a1 = combinaison(rule, ens, AstarR1, R2Aetoile);

    res = autalmot_union(res, a1);

    printtime(autalmot_determinize(res));
    printtime(autalmot_minimize(res));
  }

  error("compileRegle: out of combis (%d states)\n", res->nbstates);

  error("compl\n");
  printtime(autalmot_complementation(res));

  error("emonde\n");
  printtime(autalmot_emonde(res));

  if (res->nbstates == 0) { error("grammar %s forbids everything.\n", rule->name); }

  u_printf("grammar %s compiled. (%d states)\n", rule->name, res->nbstates);

  return res;
}



/**
 * Compiles the given .fst2 grammar into the given .elg file.
 * Returns 0 in case of success; -1 otherwise.
 */
int compile_elag_grammar(char* grammar,char* elg_file) {
elRule* rule=new_elRule(grammar);
if (rule==NULL) {
   error("Unable to read grammar '%s'\n",grammar);
   return -1;
}
Fst2Automaton* A=compile_elag_rule(rule);
if (A==NULL) {
   fatal_error("Unable to compile rule '%s'\n",grammar);
}
error("after compile\n");
deleteRegle(rule);
error("after delete\n");
save_automaton(A, elg_file,FST_GRAMMAR);
free_Fst2Automaton(A);
error("endofcompile\n");
return 0;
}


/* Lit les regles de levee d'ambiguites et les compile sous la forme d'automates. 
 * nomFichNomRegles est le fichier des noms des grammaires. 
 * nomFichNomGramm est le fichier de la composition des grammaires compilées.
 */


int compile_rules(char * rulesname, char * outname) {

  u_printf("Compilation of %s\n", rulesname);

  FILE * f = NULL;
  FILE * frules = fopen(rulesname, "r");
  if (frules == NULL) { fatal_error("cannot open file '%s'\n", rulesname); }

  FILE * out = fopen(outname, "w");
  if (out == NULL) { fatal_error("cannot open file '%s'\n", outname); }

  char fstoutname[FILENAME_MAX]; // le nom du fichier qui contient l'automate resultat (fst2)

  int nbregles = 0;
  char buf[FILENAME_MAX];

  time_t debut = time(0);

  Fst2Automaton * res = NULL, * A;
  int fstno = 0;
  Ustring * ustr = new_Ustring();




  while (fgets(buf, FILENAME_MAX, frules)) {

    chomp(buf);
    if (*buf == 0) { continue; }

    u_printf("\n\n%s ...\n", buf);

    strip_extension(buf);
    strcat(buf, ".elg");

    if ((f = fopen(buf, "r")) == NULL) { // .elg doesn't exist, making one

      strip_extension(buf);
      u_printf("precompiling %s.fst2\n", buf);

      strcat(buf, ".fst2");
      
      elRule * regle = new_elRule(buf);
      if (regle == NULL) { fatal_error("unable to read grammar '%s'\n", buf); }
      
      if ((A = compile_elag_rule(regle)) == NULL) { fatal_error("unable to compile rule '%s'\n", buf); }

      deleteRegle(regle);

      /* saving result in name.elg  ? */
      /*
      strip_extension(buf);
      strcat(buf, ".elg");
      autalmot_output_fst2(A, buf, FST_GRAMMAR);
      */
    } else {
      fclose(f);
      u_printf("using already exiting %s\n", buf);
      A = load_elag_grammar_automaton(buf);      
      if (A == NULL) { fatal_error("unable to load '%s'\n", buf); }
    }


    if (A->nbstates == 0) { error("grammar %s forbids everything!\n", buf); }
 
    error("regroupe\n");

    printtime(if (res) {

      Fst2Automaton * tmp = res;
      res = autalmot_intersection(tmp, A);
      free_Fst2Automaton(tmp);
      free_Fst2Automaton(A);
      autalmot_emonde(res);

    } else { res = A; });

    error("regroupe done (nbstates = %d)\n", res->nbstates);

    fprintf(out, "\t%s\n", buf);
    nbregles++;


    if (res->nbstates > MAX_GRAM_SIZE) {

      autalmot_minimize(res, 1);

      sprintf(fstoutname, "%s-%d.elg", outname, fstno++);
      fprintf(out, "<%s>\n", fstoutname);

      u_printf("splitting big grammar in '%s' (%d states)\n", fstoutname, res->nbstates);

      u_sprintf(ustr, "%s: compiled elag grammar", fstoutname);
      free(res->name);
      res->name = u_strdup(ustr->str);

      save_automaton(res, fstoutname, FST_GRAMMAR);

      free_Fst2Automaton(res);
      res = NULL;
    }
  }


  
  if (res) {

    sprintf(fstoutname, "%s-%d.elg", outname, fstno++);
    fprintf(out, "<%s>\n", fstoutname);

    u_printf("outputing grammar in '%s'(%d states)\n", fstoutname, res->nbstates);

    autalmot_minimize(res, 1);

    u_sprintf(ustr, "%s: compiled elag grammar", fstoutname);
    free(res->name);
    res->name = u_strdup(ustr->str);

    save_automaton(res, fstoutname, FST_GRAMMAR);

    free_Fst2Automaton(res);
  }

  time_t fin = time(0);

  fclose(frules);
  fclose(out);

  free_Ustring(ustr);

  u_printf("\ndone.\nElapsed time: %.0f s.\n", difftime(fin, debut));

  u_printf("\n%d rule%s from %s compiled in %s (%d automat%s).\n",
         nbregles, (nbregles > 1) ? "s" : "", rulesname, outname, fstno, (fstno > 1) ? "a" : "on");
  return 0;
}


/**
 * This function builds and returns an automaton for pattern
 * matching of the rule's context.
 */
Fst2Automaton* make_locate_automaton(elRule* rule) {
Fst2Automaton* res=new_Fst2Automaton(rule->contexts[0].left->name);
res->symbols=rule->contexts[0].left->symbols;
res->automaton=clone(rule->contexts[0].left->automaton);
/* We concatenate the left and right contexts */
concat(res->automaton,rule->contexts[0].right->automaton,res->symbols);
/* Then we add loops with ignorable POS on each state */
language_t* lang=get_current_language();
struct list_int* list=NULL;
for (int i=0;i<lang->POSs->size;i++) {
   POS_t* PoS=(POS_t*)lang->POSs->value[i];
   if (PoS->ignorable) {
      /* If we have a POS that can be ignored, we build
       * the corresponding tag like <XXX> and we add it to
       * the symbols of the grammar */
      unichar temp[1024];
      u_sprintf(temp,"<%S>",PoS->name);
      int size=res->symbols->size;
      symbol_t* s=new_symbol_POS(PoS);
      int n=get_value_index(temp,res->symbols,INSERT_IF_NEEDED,s);
      if (n!=size) {
         /* If the symbol was already there, we free it */
         free_symbols(s);
      }
      list=sorted_insert(n,list);
   }
}
for (int q=1;q<res->automaton->number_of_states;q++) {
   struct list_int* l=list;
   while (l!=NULL) {
      add_outgoing_transition(res->automaton->states[q],l->n,q);
      l=l->next;
   }
}
free_list_int(list);
return res;
}



/**
 * This function analyzes the given Elag rule automaton to find
 * where the rule and constraint parts are. As a side effect, it builds
 * a fst2 grammar ("foo.fst2" => "foo-conc.fst2") that can be used by
 * the Locate program to match the <!> .... <!> .... <!> part of the rule.
 */
void split_elag_rule(elRule* rule) {
int c;
/* This array contains the numbers of the states that are pointed to by
 * middle '<=>' of the constraints */
int constraints[ELAG_MAX_CONSTRAINTS];
int nbConstraints=count_constraints(rule->automaton,constraints);
/* +1 because we have to count the <!> .... <!> .... <!> part of the rule */
rule->nbContexts=nbConstraints+1;
rule->contexts=(elContext*)malloc(rule->nbContexts*sizeof(elContext));
if (rule->contexts==NULL) {
   fatal_error("Not enough memory in split_elag_rule\n");
}
for (c=0;c<rule->nbContexts;c++) {
   rule->contexts[c].left=NULL;
   rule->contexts[c].right=NULL;
}
int endR1=ELAG_UNDEFINED;
int endR2=ELAG_UNDEFINED;
int endC2=ELAG_UNDEFINED;
for (Transition* t=rule->automaton->automaton->states[0]->outgoing_transitions;t!=NULL;t=t->next) {
   symbol_t* symbol=(symbol_t*)rule->automaton->symbols->value[t->tag_number];
   switch (symbol->type) {
      /* We split the unique <!> .... <!> .... <!> part */
      case EXCLAM:
         if (rule->contexts[0].left!=NULL) {
            fatal_error("Too much '<!>' tags\n",rule->name);
         }
         rule->contexts[0].left=new_Fst2Automaton();
         /* We look for the end of the first part of the rule */
         endR1=get_sub_automaton(rule->automaton,rule->contexts[0].left,t->state_number,0,EXCLAM);
         rule->contexts[0].right=new_Fst2Automaton();
         endR2=get_sub_automaton(rule->automaton,rule->contexts[0].right,endR1,0,EXCLAM);
         if (endR1==ELAG_UNDEFINED || endR2==ELAG_UNDEFINED
             || !is_final_state(rule->automaton->automaton->states[endR2])) {
            fatal_error("split_elag_rule: %s: parse error in <!> part\n",rule->name);
         }
         break;

      /* We split the nbConstraints <=> .... <=> .... <=> parts */
      case EQUAL:
         if (rule->contexts[1].left!=NULL) {
            fatal_error("Non deterministic .fst2 file\n");
         }
         for (c=0;c<nbConstraints;c++) {
            rule->contexts[c+1].left=new_Fst2Automaton();
            get_sub_automaton(rule->automaton,rule->contexts[c+1].left,t->state_number,1,constraints[c]);
            rule->contexts[c+1].right=new_Fst2Automaton();
            endC2=get_sub_automaton(rule->automaton,rule->contexts[c+1].right,constraints[c],0,EQUAL);
            if (endC2==ELAG_UNDEFINED || !is_final_state(rule->automaton->automaton->states[endC2])) {
               fatal_error("split_elag_rule: %s: parse error in <=> part\n",rule->name);
            }
         }
         break;
         
      default: fatal_error("Left delimitor '<!>' or '<=>' missing\n");
   }
}
free_Fst2Automaton(rule->automaton);
rule->automaton=NULL;
if (rule->contexts[0].left==NULL) {
   fatal_error("In grammar '%s': symbol '<!>' not found.\n",rule->name);
}
char buf[FILENAME_MAX];
remove_extension(rule->name,buf);
strcat(buf,"-conc.fst2");
/* We create the.fst2 to be used by Locate */
Fst2Automaton* locate=make_locate_automaton(rule);
save_automaton(locate,buf,FST_LOCATE);
free_Fst2Automaton(locate);
}


/**
 * An Elag constraints is of the form: <=> .... <=> .... <=>
 * This function looks for states that are pointed to by middle '<=>' transitions.
 * It places their numbers into 'constraints' and it returns the size of this
 * array, i.e. the number of constraints expressed by this Elag rule.
 */
int count_constraints(Fst2Automaton* aut,int* constraints) {
int source=0;
int e;
Transition* t;
int c;
int nbConstraints=0;
symbol_t* symbol;
SingleGraph automaton=aut->automaton;
for (t=automaton->states[0]->outgoing_transitions;t!=NULL && source==0;t=t->next) {
   symbol=(symbol_t*)aut->symbols->value[t->tag_number];
   if (symbol->type==EQUAL) {
      if (t->state_number==0) {
         fatal_error("Illegal cycle in grammar\n");
      }
      source=t->tag_number;
   }
}
if (source==0) {
   /* If there are no contraints */
   return 0;
}
/* We look for '<=>' transitions, but only from the state 1, because
 * from state 0, we would take into account all the '<=>' that begin rules. */
for (e=1;e<automaton->number_of_states;e++) {
   for (t=automaton->states[e]->outgoing_transitions;t!=NULL;t=t->next) {
      symbol=(symbol_t*)aut->symbols->value[t->tag_number];
      if (t->state_number!=source && symbol->type==EQUAL && !is_final_state(automaton->states[t->state_number])) {
         /* We don't take into account '<=>' transitions that go to final states because
          * they are not middle '<=>' transitions. */
         for (c=0;c<nbConstraints;c++) {
            if (constraints[c]==t->state_number) {
               /* We stop if the constraint is already in the list */
               break;
            }
         }
         if (c==nbConstraints) {
	         if (++nbConstraints>=ELAG_MAX_CONSTRAINTS) {
               fatal_error("Too many constraints with same condition\n");
            }
	         constraints[c]=t->state_number;
	      }
      }
   }
}
if (nbConstraints==0) {
   fatal_error("Middle delimitor '<=>' not found\n");
}
return nbConstraints;
}


/**
 * This function copies into 'aut_dest' the sub-automaton of 'aut_src' that
 * starts at the state #start and that ends at:
 * 
 * - the state #z pointed by transitions, if left_constraint_part=1;
 * - the state pointed by transitions tagged with the 'z' symbol otherwise.
 * 
 * Note that these transitions are not copied. The function returns the number of 
 * the ending state.
 * 
 * 'aut_dest' is supposed to have been allocated and to be empty.
 */
int get_sub_automaton(Fst2Automaton* aut_src,Fst2Automaton* aut_dest,int start,
                      int left_constraint_part,int z) {
/* 'aut_dest' and 'aut_src' share the same symbols */
aut_dest->symbols=aut_src->symbols;
SingleGraph automaton=aut_dest->automaton;
/* We create the initial state of the sub-automaton */
SingleGraphState state=add_state(automaton);
set_initial_state(state);
/* We use this array to renumber states */
int* renumber=(int*)malloc(aut_src->automaton->number_of_states*sizeof(int));
if (renumber==NULL) {
   fatal_error("Not enough memory in get_sub_automaton\n");
}
for (int e=0;e<aut_src->automaton->number_of_states;e++) {
   renumber[e]=ELAG_UNDEFINED;
}
int end=ELAG_UNDEFINED;
renumber[start]=0;
if (left_constraint_part) {
   end=get_left_constraint_part(aut_src,automaton,start,z,renumber);
} else {
   end=get_sub_automaton(aut_src,automaton,start,(SymbolType)z,renumber);
}
free(renumber);
if (left_constraint_part) {
   if (!end) {
      /* This should never happen */
      fatal_error("Internal error in get_sub_automaton\n");
   }
} else if (end==ELAG_UNDEFINED) {
   /* If we have not found the delimitor */
   fatal_error("In grammar: middle or end delimitor missing\n");
}
return end;
}


/**
 * This function copies into 'aut_dest' the sub-automaton of 'aut_src' that
 * starts at the state #current_state and that ends at the state #z pointed
 * by transitions tagged with the given symbol. Note that these transitions are not copied.
 * The function returns z if such a state is found, ELAG_UNDEFINED otherwise.
 * The 'renumber' array is updated each time a new state is copied into 'aut_dest'.
 */
int get_sub_automaton(Fst2Automaton* aut_src,SingleGraph aut_dest,int current_state,SymbolType delim,int* renumber) {
int f;
int end=ELAG_UNDEFINED;
SingleGraph src=aut_src->automaton;
for (Transition* t=src->states[current_state]->outgoing_transitions;t!=NULL;t=t->next) {
   symbol_t* symbol=(symbol_t*)aut_src->symbols->value[t->tag_number];
   if (symbol->type==delim) {
      /* If we have found a transition tagged by the delimitor  */
      if (end!=ELAG_UNDEFINED && end!=t->state_number) {
         /* For a given rule part, all the delimitors all supposed to
          * point on the same state */
         fatal_error("get_sub_automaton: too much '<%c>' delimitors in rule\n",delim);
      }
      end=t->state_number;
      /* We set final the corresponding state in 'aut_dest' */
      set_final_state(aut_dest->states[renumber[current_state]]);
    } else {
      /* If we have a normal transition, we just copy it */
      if (renumber[t->state_number]==ELAG_UNDEFINED) {
         /* If we have to create a new state */
         renumber[t->state_number]=aut_dest->number_of_states;
         add_state(aut_dest);
         SingleGraphState state=aut_dest->states[renumber[current_state]];
         add_outgoing_transition(state,t->tag_number,renumber[t->state_number]);
         /* We copy recursively this part of 'aut_src' that we don't yet know */
         f=get_sub_automaton(aut_src,aut_dest,t->state_number,delim,renumber);
         if (f!=ELAG_UNDEFINED) {
            if (end!=ELAG_UNDEFINED && f!=end) {
               fatal_error("get_sub_automaton: too much '<%c>' delimitors in rule\n",delim);
            }
            end=f;
         }
      } else {
         /* If the state already exists, we just add a transition, because
          * there is no need to explore again the state. */
         SingleGraphState state=aut_dest->states[renumber[current_state]];
         add_outgoing_transition(state,t->tag_number,renumber[t->state_number]);
      }
   }
}
return end;
}


/**
 * This function copies into 'aut_dest' the sub-automaton of 'aut_src' that
 * starts at the state #current_state and that ends at the state #final.
 * Note that <=> transitions leading to the state #final are not copied.
 * The function returns 1 if the state #final is found, 0 otherwise.
 * The 'renumber' array is updated each time a new state is copied into 'aut_dest'.
 */
int get_left_constraint_part(Fst2Automaton* aut_src,SingleGraph aut_dest,int current_state,
                             int final,int* renumber) {
int found=0;
for (Transition* t=aut_src->automaton->states[current_state]->outgoing_transitions;t!=NULL;t=t->next) {
   symbol_t* symbol=(symbol_t*)aut_src->symbols->value[t->tag_number];
   if (symbol->type==EQUAL) {
      /* If we find a <=> transition */
      if (t->state_number==final && !is_final_state(aut_dest->states[renumber[current_state]])) {
         /* If we find the final state for the first time */
         found=1;
         set_final_state(aut_dest->states[renumber[current_state]]);
      }
   } else {
      /* If we have a normal transition */
      if (renumber[t->state_number]==ELAG_UNDEFINED) {
         /* If we have to create a new state */
         renumber[t->state_number]=aut_dest->number_of_states;
         add_state(aut_dest);
         SingleGraphState state=aut_dest->states[renumber[current_state]];
         add_outgoing_transition(state,t->tag_number,renumber[t->state_number]);
         if (get_left_constraint_part(aut_src,aut_dest,t->state_number,final,renumber)) {
            found=1;
         }
      } else {
         SingleGraphState state=aut_dest->states[renumber[current_state]];
         add_outgoing_transition(state,t->tag_number,renumber[t->state_number]);
      }
   }
}
  /* Si toutes les transitions sortant de e entrent dans des etats deja
   * explores auparavant, on peut avoir une activation de suivreCont qui
   * ne rencontre pas arrivee. Dans ce cas, trouve == false.
   */

  return found;
}






/* Parcours de l'ensemble des contraintes. Chaque contrainte est
 * representee par un bit dans la representation binaire de ens.
 * Postcondition : l'automate resultat est deterministe et complet.
 * Il peut ne pas avoir d'etats.
 * 
 * retourne:  (A*.R1 \ (U_{i in ens} (A*.Ci,1))) (R2.A* \ (U_{i not in ens} (Ci,2.A*)))
 */


static Fst2Automaton * combinaison(elRule * regle, int ens, Fst2Automaton * AetoileR1, Fst2Automaton * R2Aetoile) {

  // if (ens > 0) { die("combinaison: enough!\n"); }

  error("\ncombinaison(%d)\n", ens);

  Fst2Automaton * a1 = new_Fst2Automaton();
  Fst2Automaton * a2 = new_Fst2Automaton();

  int c, dpc; /* dpc = pow(2, c - 1) */

  for (c = 1, dpc = 1; c < regle->nbContexts; c++, dpc = dpc << 1) {

    if (dpc & ens) {    /* le c-ieme bit de ens vaut 1 */

      a1 = autalmot_union(a1, autalmot_dup(regle->contexts[c].left));

      autalmot_determinize(a1);
      autalmot_minimize(a1);

    } else {    /* le c-ieme bit de ens vaut 0 */

      a2 = autalmot_union(a2, autalmot_dup(regle->contexts[c].right));

      autalmot_determinize(a2);
      autalmot_minimize(a2);
    }
  }

  error("\nI\n");

  //  debug("U_{i in E} (Ci,1):\n"); autalmot_dump(a1);
  //  debug("U_{i not in E} (Ci,2):\n"); autalmot_dump(a2);

  /* L(a1) = U_{i in I} R1_i
   * L(a2) = U_{i not in I} R2_i */

  printtime(prefix_with_everything(a1));
  
  printtime(autalmot_determinize(a1));

  printtime(autalmot_minimize(a1));

  printtime(autalmot_complementation(a1));

  printtime(autalmot_emonde(a1));

  Fst2Automaton * tmp = a1;
  printtime(a1 = autalmot_intersection(a1, AetoileR1));

  printtime(free_Fst2Automaton(tmp));

  printtime(autalmot_emonde(a1));

  printtime(autalmot_minimize(a1));  /* emonder a3 ? */

  /* L(a1) =  (A*.R1 \ (U_{i in ens} (A*.Ci,1))) */

  error("\nII\n");

  printtime(suffix_with_everything(a2));

  printtime(autalmot_determinize(a2));

  printtime(autalmot_minimize(a2));
    
  printtime(autalmot_complementation(a2));

  printtime(autalmot_emonde(a2));

  tmp = a2;
  printtime(a2 = autalmot_intersection(a2, R2Aetoile));
  free_Fst2Automaton(tmp);


  /* autalmot_determinize(a2) ; */

  printtime(autalmot_emonde(a2));
  printtime(autalmot_minimize(a2));

  /* L(a2) == (R2.A* \ (U_{i not in ens} (Ci,2.A*))) */

  error("\nIII\n");

  concat(a1->automaton,a2->automaton,a1->symbols);

  free_Fst2Automaton(a2);

  printtime(autalmot_emonde(a1));

  printtime(autalmot_determinize(a1));

  printtime(autalmot_emonde(a1));  /* utile quand tous les etats sont inutiles */

  printtime(autalmot_minimize(a1));

  error("end of combi(%d)\n\n", ens);

//  autalmot_dump(a1);

  return a1;
}
